/* -----------------------------------------------------------------------
/
/  macnres2.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/macro/macnres2.c_v   0.35   06 Jul 1992 12:50:32   maynard  $
/  $Log:   J:/logfiles/oma4000/macro/macnres2.c_v  $
*/

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>

#include "macnres2.h"
#include "macnres.h"
#include "macrecor.h"
#include "macrores.h"
#include "forms.h"
#include "formwind.h"
#include "macruntm.h"
#include "tempdata.h"
#include "points.h"
#include "mathops.h"
#include "device.h"
#include "filestuf.h"
#include "di_util.h"
#include "macrofrm.h"  // CommandOutput[]
#include "mdatstak.h"
#include "macres2.h"
#include "macnres3.h"
#include "syserror.h"  // ERROR_ALLOC_MEM
#include "omaerror.h"
#include "crventry.h"
#include "curvbufr.h"
#include "curvedir.h"  // MainCurveDir
#include "omaform.h"   // setCurrentFormToMacroForm()
#include "live.h"      // LastLiveEntryName

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

static SHORT ConStart = 0;
static SHORT CurConPos = 0;
#define MAXCONLEN (79)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT MakeNewTempCurveSet(SHORT SrcEntryIndex, USHORT SrcStartIndex,
                          USHORT Count)
{
  SHORT NewIndex;
  static CURVEHDR TmpCurvehdr;
  SHORT TempNumber = 0;

  /* make a new temporary curve block */
  do
    {
    /* strip off previous extensions and add a new one */
    itoa(TempNumber++, &(TempEntryName[4]), 10);
    NewIndex = FindSpecialEntry(TempEntryName);
    }
  while(NewIndex != -1);

  /* make a unique temporary entry */
  if(CreateTempFileBlk(&MainCurveDir, &NewIndex, TempEntryName,
                       "", "", 0, 0L, 0, NULL, OMA4DATA, 0))
    {
    SetErrorFlag();
    return -1;
    }

  /* CreateTempFileBlk makes TempEntryName upper case, restore to */
  /* lower case */
  strcpy(MainCurveDir.Entries[NewIndex].name, TempEntryName);

  /* if the largest operand uses the temp block, reuse its space */
  if (SrcEntryIndex != NewIndex)
    {
    // This done to ensure that temp curve set is of type FLOATTYPE
    if (ReadTempCurvehdr(&MainCurveDir, SrcEntryIndex, SrcStartIndex,
                         &TmpCurvehdr) ||
        create_new_curve(FALSE, Count, TmpCurvehdr.pointnum,
                         NewIndex, 0, 0.0))
      {
      DelTempFileBlk(&MainCurveDir, NewIndex);
      SetErrorFlag();
      return NOT_FOUND;
      }
    }
  return NewIndex;
}

void minmax_curve(unsigned SetIndex, unsigned CurveIndex,
                  unsigned Count, float *min, float *max)
{
  USHORT i;
  SHORT prefBuf = -1;
  float Y, X, locmin = (float)MAXFLOAT, locmax = (float)MINFLOAT;
  
  for (i = 0;i < Count; i++)
    {
    if (GetDataPoint(&MainCurveDir, SetIndex, CurveIndex, i, &X, &Y,
                     FLOATTYPE, &prefBuf))
      {
      SetErrorFlag();
      return;
      }
    if (Y < locmin) locmin = Y;
    if (Y > locmax) locmax = Y;
    }
  if (min) *min = locmin;
  if (max) *max = locmax;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void OperateOnCurves(SHORT lType, SHORT rType, SHORT Operator,
                     SHORT OperandCount)
{
  SHORT CopyIndex,
        EntryIndex[3], i, Types[3],
        prefBuf[3] = {0, 1, 2};
  USHORT NumCurves[3], StartCurves[3];
  LONG lTemp[2];
  FLOAT fTemp[2], X;
  DOUBLE Y, dTemp[2];
  PCHAR String[2];
  CURVEHDR Curvehdr;
  LPCURVE_ENTRY pEntries[3];
  CURVE_REF Curve[3];

  for (i=0; i<3; i++)
    {
    EntryIndex[i] = -1;
    NumCurves[i] = 1;
    StartCurves[i] = 0;
    }

  Types[0] = lType;
  Types[1] = rType;

  for (i= OperandCount - 1; i >= 0; i--)
    {
    if (Types[i] == TYPE_CURVE)
      {
      CURVE_REF * pCurve;

      PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO);
      Curve[ i ] = * pCurve;
      if (Curve[i].CurveSetIndex == NOT_FOUND ||
          Curve[i].CurveSetIndex >= MainCurveDir.BlkCount)
        {
        error(ERROR_BAD_CURVE_BLOCK, Curve[i].CurveSetIndex);
        SetErrorFlag();
        return;
        }

      EntryIndex[i] = Curve[i].CurveSetIndex;
      pEntries[i] = &(MainCurveDir.Entries[EntryIndex[i]]);
      if (Curve[i].ReferenceType == CLASS_CURVESET)
        {
        Curve[i].CurveIndex = pEntries[i]->StartIndex;
        NumCurves[i] = pEntries[i]->count;
        }
      else if (Curve[i].ReferenceType != CLASS_CURVE)
        {
        if (Curve[i].CurveIndex > pEntries[i]->count)
          {
          error(ERROR_CURVE_NUM, Curve[i].CurveIndex);
          SetErrorFlag();
          return;
          }
        EntryIndex[i] = -1;
        if (Curve[i].ReferenceType == CLASS_CURVEHDR)
          {
          if(ReadTempCurvehdr(& MainCurveDir, Curve[i].CurveSetIndex,
            Curve[i].CurveIndex, & Curvehdr))
            {
            SetErrorFlag();
            return;
            }
          }
        switch (Curve[i].Point_Or_HeaderItem)
          {
          case CURVE_POINT_COUNT:
            Types[i] = TYPE_LONG_INTEGER;
            lTemp[i] = (LONG) Curvehdr.pointnum;
          break;

          case CURVE_TIME:
            Types[i] = TYPE_REAL4;
            fTemp[i] = Curvehdr.time;
          break;

          case CURVE_YMIN:
            Types[i] = TYPE_REAL4;
            minmax_curve(Curve[i].CurveSetIndex,
              Curve[i].CurveIndex,
              Curvehdr.pointnum, &(fTemp[i]), NULL);
            Curvehdr.Ymin = fTemp[i];
          break;

          case CURVE_YMAX:
            Types[i] = TYPE_REAL4;
            minmax_curve(Curve[i].CurveSetIndex,
              Curve[i].CurveIndex,
              Curvehdr.pointnum, NULL, &(fTemp[i]));
            Curvehdr.Ymax = fTemp[i];
          break;

          case CURVE_XMIN:
            Types[i] = TYPE_REAL4;
            fTemp[i] = Curvehdr.Xmin;
          break;

          case CURVE_XMAX:
            Types[i] = TYPE_REAL4;
            fTemp[i] = Curvehdr.Xmax;
          break;

          case CURVE_FRAME:
            Types[i] = TYPE_LONG_INTEGER;
            lTemp[i] = (ULONG) Curvehdr.Frame;
          break;

          case CURVE_TRACK:
            Types[i] = TYPE_LONG_INTEGER;
            lTemp[i] = (ULONG) Curvehdr.Track;
          break;

          case CURVE_XUNITS:
            Types[i] = TYPE_LONG_INTEGER;
            lTemp[i] = (LONG) Curvehdr.XData.XUnits;
          break;

          case CURVE_YUNITS:
            Types[i] = TYPE_LONG_INTEGER;
            lTemp[i] = (LONG) Curvehdr.YUnits;
          break;

          case CURVE_SCMP:
            Types[i] = TYPE_LONG_INTEGER;
            lTemp[i] = Curvehdr.scomp;
          break;

          case CURVE_START_INDEX:
            Types[i] = TYPE_LONG_INTEGER;
            lTemp[i] = (LONG) pEntries[i]->StartIndex;
          break;

          case CURVE_COUNT:
            Types[i] = TYPE_LONG_INTEGER;
            lTemp[i] = (LONG) pEntries[i]->count;
          break;

          case CURVE_NAME:
            Types[i] = TYPE_STRING;
            String[i] = pEntries[i]->name;
          break;

          case CURVE_PATH:
            Types[i] = TYPE_STRING;
            String[i] = pEntries[i]->path;
          break;

          case CURVE_DESC:
            Types[i] = TYPE_STRING;
            String[i] = pEntries[i]->descrip;
          break;

          case CURVE_DISPLAY:
            Types[i] = TYPE_LONG_INTEGER;
            lTemp[i] = (LONG) pEntries[i]->DisplayWindow;
          break;

          case CURVE_POINTY:
          case CURVE_POINTX:
            Types[i] = TYPE_REAL;
            if(GetDataPoint(& MainCurveDir, Curve[i].CurveSetIndex,
              Curve[i].CurveIndex, Curve[i].PointIndex,
              &X, &Y, (CHAR) TYPE_REAL, &prefBuf[i]))
              {
              SetErrorFlag();
              return;
              }

            if (Curve[i].Point_Or_HeaderItem != CURVE_POINTX)
              dTemp[i] = Y;
            else
              dTemp[i] = (DOUBLE) X;
          break;
          }
        }  /* scalar CLASS from TYPE_CURVE */

      StartCurves[i] = Curve[i].CurveIndex;
      }     /* if type curve */
    else
      {
      if (Types[i] == TYPE_STRING)
        PopScalarFromDataStack(&(String[i]), POINTER_TO | TYPE_STRING);
      else if (Types[i] == TYPE_REAL)
        PopScalarFromDataStack(&(dTemp[i]), TYPE_REAL);
      else if (Types[i] == TYPE_REAL4)
        PopScalarFromDataStack(&(fTemp[i]), TYPE_REAL4);
      else
        PopScalarFromDataStack(&(lTemp[i]), TYPE_LONG_INTEGER);
      }
    }        /* for operand loop */

  /* check for two scalars */
  if ((EntryIndex[0] == EntryIndex[1]) && (EntryIndex[0] == -1))
    {
    for (i=0; i<OperandCount; i++)
      {
      if (Types[i] == TYPE_STRING)
        PushToDataStack(&(String[i]), POINTER_TO | TYPE_STRING, FALSE);
      else if (Types[i] == TYPE_REAL)
        PushToDataStack(&(dTemp[i]), TYPE_REAL, FALSE);
      else if (Types[i] == TYPE_REAL4)
        PushToDataStack(&(fTemp[i]), TYPE_REAL4, FALSE);
      else
        PushToDataStack(&(lTemp[i]), TYPE_LONG_INTEGER, FALSE);
      }
    OperateOnScalars(Operator, OperandCount);
    return;
    }

  /***************************/
  /* have at least one curve */
  /***************************/

  /* find the number of curves in the result */
  if ((NumCurves[0] >= NumCurves[1]) && (EntryIndex[0] != -1))
    {
    NumCurves[2] = NumCurves[0];
    CopyIndex = 0;
    }
  else
    {
    NumCurves[2] = NumCurves[1];
    CopyIndex = 1;
    }

  EntryIndex[2] = MakeNewTempCurveSet(Curve[CopyIndex].CurveSetIndex,
    Curve[CopyIndex].CurveIndex,
    NumCurves[2]);
  if (EntryIndex[2] == -1)
    return;

  /* set up result for later return */
  /* class is set to the largest one */
  if (((EntryIndex[0] != -1) &&
    (Curve[0].ReferenceType == CLASS_CURVESET)) ||
    ((EntryIndex[1] != -1) &&
    (Curve[1].ReferenceType == CLASS_CURVESET)))
    Curve[2].ReferenceType = CLASS_CURVESET;
  else
    Curve[2].ReferenceType = CLASS_CURVE;

  Curve[2].CurveSetIndex = EntryIndex[2];
  pEntries[2] = &(MainCurveDir.Entries[EntryIndex[2]]);
  Curve[2].CurveIndex = 0;
  Curve[2].PointIndex = 0;           // this one doesn't really matter
  Curve[2].Point_Or_HeaderItem = 0;  // this one doesn't really matter

  for (i=0; i<OperandCount; i++)
    {
    if (EntryIndex[i] == -1)
      {
      switch (Types[i])
        {
        case TYPE_LONG_INTEGER:
          dTemp[i] = (DOUBLE) lTemp[i];
        break;
        case TYPE_STRING:
          dTemp[i] = (DOUBLE) ((LONG) String[i]);
        break;
        }
      }
    }

  DoBinMath(&MainCurveDir, EntryIndex, StartCurves, NumCurves, dTemp,
    Operator, FALSE);

  PushToDataStack(&(Curve[2]), TYPE_CURVE, FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void OperateOnScalars(SHORT Operator, SHORT OperandCount)
{
  DOUBLE Values[3];
  SHORT EntryIndex[3], i;
  USHORT NumCurves[3], StartCurves[3];

  if (OperandCount == 2)
    PopFromDataStack(&Values[1], TYPE_REAL);
  PopFromDataStack(&Values[0], TYPE_REAL);

  for (i=0; i<3; i++)
    {
    EntryIndex[i] = -1;
    NumCurves[i] = 1;
    StartCurves[i] = 0;
    }

  Values[2] = DoBinMath(NULL, EntryIndex, StartCurves, NumCurves,
                        Values, Operator, FALSE);
  PushToDataStack(&Values[2], TYPE_REAL, FALSE);
}

/***********************************************************************/
void OperateOnStrings(SHORT Operator, SHORT OperandCount)
{
  PCHAR String[2];
  PUCHAR pMaxStrLen;
  UCHAR i;
  SHORT StrLen1;
  SHORT Compare;

  if (OperandCount == 2)
    PopFromDataStack(&String[1], POINTER_TO | TYPE_STRING);
  PopFromDataStack(&String[0], POINTER_TO | TYPE_STRING);

  Compare = strcmp(String[0], String[1]);

  switch (Operator)
    {
    case PLUS:
      pMaxStrLen = (PUCHAR) ((ULONG) String[0] - 1L);
      StrLen1 = strlen(String[0]);
      i=0;
      do
        {
        String[0][StrLen1 + i] = String[1][i];
        i++;
        }
      while (((UCHAR) (StrLen1 + i) < *pMaxStrLen) &&
        (String[1][i-1] != '\0'));

      String[0][*pMaxStrLen] = '\0';

      PushToDataStack(String[0], POINTER_TO | TYPE_STRING, FALSE);
      return;

    case EQUALTO:
      Compare = !Compare;
    break;

    case NOTEQUALTO:
    break;

    case LESSTHAN:
      if (Compare < 0)
        Compare = TRUE;
      else
        Compare = FALSE;
    break;

    case GREATERTHAN:
      if (Compare > 0)
        Compare = TRUE;
      else
        Compare = FALSE;
    break;

    case LESSTHANEQ:
      if (Compare <= 0)
        Compare = TRUE;
      else
        Compare = FALSE;
    break;

    case GREATERTHANEQ:
      if (Compare >= 0)
        Compare = TRUE;
      else
        Compare = FALSE;
    break;
    }

  PushToDataStack(&Compare, TYPE_BOOLEAN, FALSE);
}

/***********************************************************************/
void RunOperator(SHORT Operator)
{
   SHORT rType, lType;
   SHORT lStackPos;

   switch (Operator)
   {
      case PLUS:
      case SUB:
      case MULTIPLY:
      case DIVIDE:
      case ABSORB:
      case INTDIV:
      case MOD:
      case AND:
      case OR:
      case XOR:
      case EXP:
      case ATAN2:
      case EQUALTO:
      case NOTEQUALTO:
      case LESSTHAN:
      case GREATERTHAN:
      case LESSTHANEQ:
      case GREATERTHANEQ:
         lStackPos = 1;
         break;

      case LOG:
      case LN:
      case TRUNC:
      case ROUND:
      case ABS:
      case BITNOT:
      case SHL:
      case SHR:
      case SIN:
      case COS:
      case TAN:
      case ATAN:
      case ASIN:
      case ACOS:
         lStackPos = 0;
         break;
   }
   rType = DataStackPeek(0) & 0x7F;
   lType = DataStackPeek(lStackPos) & 0x7F;
   if ((rType == TYPE_CURVE) || (lType == TYPE_CURVE))
      OperateOnCurves(lType, rType, Operator, lStackPos + 1);
   else if ((rType == TYPE_STRING) || (lType == TYPE_STRING))
      OperateOnStrings(Operator, lStackPos + 1);
   else
      OperateOnScalars(Operator, lStackPos + 1);
}

/***********************************************************************/
void AssignTo(void)
{
  int lType, PopType;
  PVOID pResult, pTemp;
  PCHAR String[2];
  FLOAT fTemp;
  DOUBLE dTemp;
  LONG lTemp;
  BOOLEAN lPtr;

  lType = DataStackPeek(1);
  lPtr = lType & POINTER_TO;
  lType &= ~POINTER_TO;

  if (lType == TYPE_CURVE)
    AssignToCurveClass();
  else
    {
    PopType = DataStackPeek(0) & ~POINTER_TO;

    /* handle assignment of byte array or string to string or byte array */

    if (lType == TYPE_STRING ||
        lType == TYPE_BYTE && lPtr && PopType == TYPE_STRING)
      {
      USHORT MaxStrLen;
      
      PopFromDataStack(&String[1], POINTER_TO | TYPE_STRING);
      PopFromDataStack(&String[0], POINTER_TO | TYPE_STRING);

      MaxStrLen = min(strlen(String[1])+1, DEFAULT_STRING_LEN);

      memcpy(String[0], String[1], MaxStrLen);
      }
    else
      {
      switch (lType)
        {
        case TYPE_INTEGER:
        case TYPE_LONG_INTEGER:
        case TYPE_WORD:
        case TYPE_BYTE:
        case TYPE_FILE:
        case TYPE_TEXTFILE:
          PopType =  TYPE_LONG_INTEGER;
          pTemp = &lTemp;
        break;

        case TYPE_REAL4:
          PopType =  TYPE_REAL4;
          pTemp = &fTemp;
        break;

        default:
          PopType = TYPE_REAL;
          pTemp = &dTemp;
        break;

        }
      if (!PopScalarFromDataStack(pTemp, PopType))
        {
        error(ERROR_BAD_ASSN_RTYPE);
        SetErrorFlag();
        }

      PopFromDataStack(&pResult, POINTER_TO);

      ConvertTypes(pTemp, PopType, pResult, lType);
      }
    }
  DeleteTempMathFiles();
}

/***********************************************************************/
void Assign(void)
{
  CHAR *FileName;
  FILE **hFile;
  SHORT Text;
  CHAR *AccessType = "at+";
  SHORT FileType;

  FileType = DataStackPeek(1) & 0x7F;
  Text = (FileType == TYPE_TEXTFILE);

  if (! PopScalarFromDataStack(&FileName, POINTER_TO | TYPE_STRING))
    {
    error(ERROR_BAD_PARAM_TYPE, 2);
    SetErrorFlag();
    return;
    }

  if ((FileType != TYPE_FILE) && ! Text)
    {
    error(ERROR_BAD_PARAM_TYPE, 1);
    SetErrorFlag();
    return;
    }

  if (Text)
    {
    PopFromDataStack(&hFile, TYPE_TEXTFILE | POINTER_TO);
    AccessType[1] = 't';
    }
  else
    {
    PopFromDataStack(&hFile, TYPE_FILE | POINTER_TO);
    AccessType[1] = 'b';
    }

  AccessType[0] = 'a';
  *hFile = fopen(FileName, AccessType);

  /* test for a non-existing file or a bad open */
  if (*hFile == NULL)
    {
    /* try to open a new file */
    AccessType[0] = 'w';
    *hFile = fopen(FileName, AccessType);
    }

  if (*hFile == NULL)
    {
    error(ERROR_OPEN, FileName);
    SetErrorFlag();
    }
}

/***********************************************************************/

static BOOLEAN move_old_data(LPCURVE_ENTRY pEntry, USHORT OldPointNum,
                      USHORT NewPointNum, USHORT CurveIndex, USHORT DataType)

{
  USHORT DataSz;
  ULONG SrcOffset, DstOffset, MoveLen;

  SrcOffset = pEntry->TmpOffset;

  if(GetTempCurveOffset(CurveIndex - pEntry->StartIndex, &SrcOffset))
    return TRUE;

  DataSz = DataType & 0x0F;
  SrcOffset += sizeof(CURVEHDR);

  if (NewPointNum > OldPointNum)  /* if growing, not truncating */
    {
    DstOffset = SrcOffset + ((NewPointNum - OldPointNum) * DataSz);
    }
  else  /* if truncating */
    {
    USHORT Temp = NewPointNum;
    NewPointNum = OldPointNum;
    OldPointNum = Temp;

    DstOffset = SrcOffset;
    SrcOffset = DstOffset + ((NewPointNum - OldPointNum) * DataSz);
    }
  MoveLen = OldPointNum * DataSz;

  /* move Y Points up (or down) */
  if(MoveFileBlock(hTempFile, TempFileBuf, DstOffset,
    hTempFile, TempFileBuf, SrcOffset, MoveLen))
    
    return TRUE;

  SrcOffset += NewPointNum * DataSz;
  DstOffset += NewPointNum * DataSz;
  MoveLen = OldPointNum * sizeof(FLOAT);

  /* move X Points up (or down) */
  if(MoveFileBlock(hTempFile, TempFileBuf, DstOffset,
    hTempFile, TempFileBuf, SrcOffset, MoveLen))
  
    return TRUE;

  return FALSE;
}

/***********************************************************************/
void MacChangeCurveSize(void)
{
  SHORT BadParam = 0, prefBuf = -1;
  USHORT i, DataType, CrvIdx, CrvSet, NewPointNum, OldPointNum, DiffPoint[2];
  FLOAT XInc, X[2];
  DOUBLE Y, FillVal;
  BOOLEAN RightSide;
  CURVE_REF Curve;
  CURVEHDR Curvehdr;
  LPCURVE_ENTRY pEntry;

  if (! PopScalarFromDataStack(&FillVal, TYPE_REAL))
    BadParam = 4;
  else if (! PopScalarFromDataStack(&RightSide, TYPE_BOOLEAN))
    BadParam = 3;
  else if (! PopScalarFromDataStack(&NewPointNum, TYPE_WORD))
    BadParam = 2;
  else
    {
    CURVE_REF *pCurve;
    SHORT Type = PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO);

    Curve = *pCurve;
    if ((Type != TYPE_CURVE) || (Curve.ReferenceType != CLASS_CURVE))
      BadParam = 1;
    }

  if (BadParam)
    {
    MacBadParam(BadParam);
    return;
    }
  
  CrvSet = Curve.CurveSetIndex;
  CrvIdx = Curve.CurveIndex;
  pEntry = &(MainCurveDir.Entries[CrvSet]);

  if(ReadTempCurvehdr(&MainCurveDir, CrvSet, CrvIdx, &Curvehdr))
    {
    SetErrorFlag();
    return;
    }

  if (Curvehdr.MemData) // live data
    {  
    error(ERROR_IMPROPER_FILETYPE, LastLiveEntryName);
    SetErrorFlag();
    return;
    }

  OldPointNum = Curvehdr.pointnum;
  DataType = Curvehdr.DataType;

  /* To fill in X and Y values, X will increment using the difference */
  /* between the first or last 2 original X values, depending on RightSide */
  if (RightSide)
    {
    DiffPoint[0] = OldPointNum - 2;
    DiffPoint[1] = OldPointNum - 1;
    }
  else
    {
    DiffPoint[0] = 0;
    DiffPoint[1] = 1;
    }

  /* If truncating left side, move data down first */

  if ((!RightSide) && (OldPointNum > NewPointNum))
    {
    if (move_old_data(pEntry, OldPointNum, NewPointNum, CrvIdx, DataType))
      {
      SetErrorFlag();
      return;
      }
    }

  if(GetDataPoint(&MainCurveDir, CrvSet, CrvIdx,
    DiffPoint[0], &X[0], &Y, DOUBLETYPE, &prefBuf) ||

    GetDataPoint(&MainCurveDir, CrvSet, CrvIdx,
      DiffPoint[1], &X[1], &Y, DOUBLETYPE, &prefBuf) ||

    ChangeCurveSize(&MainCurveDir, CrvSet, CrvIdx, NewPointNum, (CHAR)DataType))
    {
    SetErrorFlag();
    return;
    }

  /* if curve was shrunk, all done */
  if (OldPointNum >= NewPointNum)
    return;

  /* move the X and Y values */
  if (!RightSide)
    {
    if (move_old_data(pEntry, OldPointNum, NewPointNum, CrvIdx, DataType))
      {
      SetErrorFlag();
      return;
      }
    NewPointNum -= OldPointNum;
    OldPointNum = 0;
    }

  XInc = X[1] - X[0];

  if (! RightSide)
    X[1] = X[0] - ((NewPointNum+1) * XInc);

  for(i = OldPointNum; i< NewPointNum; i++)
    {
    X[1] += XInc;
    if (X[1] < Curvehdr.Xmin)  Curvehdr.Xmin = X[1];
    if (X[1] > Curvehdr.Xmax)  Curvehdr.Xmax = X[1];
    if (SetDataPoint(&MainCurveDir, CrvSet, CrvIdx, i, &X[1],
                     &FillVal, (CHAR)DOUBLETYPE, &prefBuf))
      {
      SetErrorFlag();
      return;
      }
    }

  /* Reread curve header to get changes made by ChangeCurveSize */
  /* First save changes this routine will need to make          */

  X[0] = Curvehdr.Xmin;
  X[1] = Curvehdr.Xmax;

  if(ReadTempCurvehdr(&MainCurveDir, CrvSet, CrvIdx, &Curvehdr))
    {
    SetErrorFlag();
    return;
    }

  Curvehdr.Xmin = X[0];
  Curvehdr.Xmax = X[1];

  if(WriteTempCurvehdr(&MainCurveDir, CrvSet, CrvIdx, &Curvehdr))
    SetErrorFlag();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void SkipToNumber(FILE * source)
{
  char     test;

  do
    {
    test = (char) getc(source);
    }
  while ((!isdigit(test)) && (test != '-')  && (test != '+')
          && (test != '\n') && (test != EOF));

  ungetc(test, source);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int GetConInput(void)
{
  CHAR Chr[2];
  SHORT CursorPos = CurConPos;
  SHORT BufPos = 0;
  BOOLEAN KeyOK;
  SHORT Temp;

  Chr[1] = 0;
  do
    {
    set_cursor(screen_rows - 1, CursorPos);  /* display text cursor */

    Chr[0] = get_FORM_key_input();

    Temp = Chr[0];
    Temp &= 0xFF;
    KeyOK = FALSE;
    switch (Temp)
      {
      case KEY_LEFT:
        if (CursorPos > CurConPos)
          CursorPos -= 1;
      break;
      case KEY_RIGHT:
        if (CursorPos < (SHORT) strlen(CommandOutput))
          CursorPos += 1;
      break;
      case KEY_BACKSPACE:
        if (CursorPos > CurConPos)
          CursorPos -= 1;         // follow into KEY_DELETE
        else
      break;
      case KEY_DELETE:
        if (CursorPos < (SHORT) strlen(CommandOutput))
          {
          memmove(&(CommandOutput[CursorPos]),&(CommandOutput[CursorPos+1]),
                   strlen(CommandOutput) - CursorPos);
          BufPos--;
          }
      break;

    case KEY_HOME:
      CursorPos = CurConPos;
    break;

    case KEY_ENTER:
      Chr[0] = CR;
    case KEY_END:
      CursorPos = strlen(CommandOutput);
    break;

    default:
      if (isascii(Temp))
        KeyOK = TRUE;
      }

    push_form_context();

    if (KeyOK)
      {
      if (CursorPos == (SHORT) strlen(CommandOutput))
        strncat(CommandOutput, Chr, MAXCONLEN - strlen(CommandOutput));
      else  // insert at current location
        {
        memmove(&(CommandOutput[CursorPos + 1]), &(CommandOutput[CursorPos]),
                strlen(CommandOutput) - CursorPos);
        CommandOutput[MAXCONLEN] = '\0';
        CommandOutput[CursorPos] = Chr[0];
        }
      if (CursorPos < MAXCONLEN)
        CursorPos++;
      }

    if (MacPlayBack) /* if in keystroke "menu" mode */
      {
      displayMacroForm(); /* make form visible */
      }
    setCurrentFormToMacroForm();
    erase_cursor();
    CommandOutput[MAXCONLEN] = '\0';
    Current.Form->field_index = 2;
    init_field();
    format_and_display_field(FALSE);
    pop_form_context();
    }
  while (Chr[0] != CR);
  return 1;
}

/* scan past leading spaces and tabs to numeric values, return position */
/* of next non-numeric character.  Returns 0 if no numeric characters */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE SHORT FindNumericEnd(PCHAR String, BOOLEAN GetFloat)
{
  SHORT i;
  BOOLEAN Exp = !GetFloat;
  BOOLEAN Decimal = !GetFloat;  // integer values will not use '.' or 'e'
  BOOLEAN Sign = FALSE;
  BOOLEAN StrOK = TRUE;
  SHORT StrLen;

  i=0;
  while (isspace((SHORT)String[i]))
    i++;

  StrLen = strlen(String);
  /* check for nonnumeric characters */
  for ( ;i<StrLen && StrOK; i++)
    {
    /* check for non-numeric, or repeat '.' or 'e' */
    if (!isdigit(String[i]))
      {
      if (String[i] == '.')
        {
        Sign = TRUE;  /* don't want a sign after decimal point */
        if (Decimal)
          StrOK = FALSE;
        else
          Decimal = TRUE;
        }
      else if ((String[i] & 0xDF) == 'E')
        {
        Sign = FALSE; /* can do another sign */
        if (Exp)
          StrOK = FALSE;
        else
          Exp = TRUE;
        }
      else if ((String[i] == '-') || (String[i] == '+'))
        {
        if (Sign)
          StrOK = FALSE;
        else
          Sign = TRUE;
        }
      else /* unrecognized character */
        StrOK = FALSE;
      } /* non space or non_numeric */
    }
  if (i!= StrLen)
    i--;
  return i;
}

/* scan the string for one variable at a time.  Take only the variable */
/* requested and remember the current location on the string */
/* returns TRUE if successful */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN con_scanf(PCHAR FormatString, PVOID pVar)
{
  PCHAR pTmpString;
  SHORT NumEndPos;
  CHAR tChr;
  BOOLEAN IsString = FALSE;

  pTmpString = &CommandOutput[CurConPos];

  if (*pTmpString == '\0')                    // check for null string
    return FALSE;

  /* parse to correct type, as given by ReadProc */
  switch (FormatString[2])
    {
    case 'd':
      NumEndPos = FindNumericEnd(pTmpString, FALSE);
    break;

    case 'u':
      NumEndPos = FindNumericEnd(pTmpString, FALSE);
    break;

    case 'l':
    switch (FormatString[3])
      {
      case 'g':
        NumEndPos = FindNumericEnd(pTmpString, TRUE);
      break;

      case 'd':
        NumEndPos = FindNumericEnd(pTmpString, FALSE);
      break;
      }
    break;
    default:
       /* must be a string */
      sscanf(&(FormatString[1]), "%d", &NumEndPos);
      IsString = TRUE;
    break;
    }


  tChr = pTmpString[NumEndPos];
  pTmpString[NumEndPos] = '\0';
  if (IsString)
    {
    strncpy(pVar, pTmpString, NumEndPos);
    ((CHAR *)pVar)[NumEndPos] = '\0';
    }
  else
    sscanf(pTmpString, FormatString, pVar);
  pTmpString[NumEndPos] = tChr;
  CurConPos += NumEndPos;
  return TRUE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacReadChar(void)
{
  unsigned inch = 0;

  inch = get_FORM_key_input();

  PushToDataStack((unsigned *) &inch, TYPE_WORD, FALSE);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReadProc(BOOLEAN ToEOLN)
{
  SHORT FileType = TYPE_TEXTFILE;
  FILE *hFile = stdin;
  SHORT Type;
  SHORT BadParam = 0;
  SHORT VarNum, GivenVarNum;
  ERR_OMA err = ERROR_NONE;
  CHAR FormatStr[8];
  PUCHAR pMaxStrLen;
  SHORT i, Size;
  CURVE_REF Curve;
  BOOLEAN CurveType;

  UCHAR bTmp;
  USHORT wTmp;
  SHORT iTmp;
  LONG liTmp;
  DOUBLE dTmp;
  FLOAT fTmp;
  PVOID pTmp;
  BOOLEAN FilVarGiven = FALSE;
  BOOLEAN GotInput = FALSE;

  if (! PopScalarFromDataStack(&GivenVarNum, TYPE_INTEGER))
    {
    error(ERROR_BAD_PARAM_TYPE, 0);
    SetErrorFlag();
    return;
    }

  VarNum = GivenVarNum;

  if (GivenVarNum)  /* empty call */
    {
    FileType = (DataStackPeek(GivenVarNum - 1) & ~POINTER_TO);
    if ((FileType == TYPE_FILE) || (FileType == TYPE_TEXTFILE))
      {
      IndexDataStack((GivenVarNum - 1), &hFile, TYPE_FILE);
      VarNum--;
      FilVarGiven = TRUE;
      }
    }              // if any variables

  GotInput = FALSE;

  for (i=VarNum-1; i>=0; i--)
    {
    if ((Type = (DataStackPeek(i) & ~POINTER_TO)) == TYPE_CURVE)
      {
      CURVE_REF * pCurve;

      if (! ScalarDataStackPeek(i, &Type))
        {
        if (FilVarGiven)
          BadParam = VarNum - i + 1;
        else
          BadParam = VarNum - i;

        error(ERROR_BAD_PARAM_TYPE, BadParam);
        SetErrorFlag();
        return;
        }
      IndexDataStack(i, &pCurve, TYPE_CURVE | POINTER_TO);
      Curve = * pCurve;
      CurveType = TRUE;
      }
    else
      {
      IndexDataStack(i, &pTmp, TYPE_DONT_CARE | POINTER_TO);
      CurveType = FALSE;
      }

    switch (Type)
      {
      case TYPE_BYTE:
        if (CurveType)
          pTmp = &bTmp;
        if (FileType == TYPE_FILE)
          Size = sizeof(BYTE);
        else
          {
          strcpy(FormatStr, " %d");
          wTmp = 0;                 // default value
          }
      break;

      case TYPE_WORD:
        if (CurveType)
          pTmp = &wTmp;
        if (FileType == TYPE_FILE)
          Size = sizeof(USHORT);
        else
          {
          strcpy(FormatStr, " %u");
          *((PUSHORT)pTmp) = 0;         // default value
          }
      break;
      case TYPE_INTEGER:
        if (CurveType)
          pTmp = &iTmp;
        if (FileType == TYPE_FILE)
          Size = sizeof(SHORT); 
        else
          {
          strcpy(FormatStr, " %d");
          *((PSHORT)pTmp) = 0;          // default value
          }
      break;

      case TYPE_LONG_INTEGER:
        if (CurveType)
          pTmp = &liTmp;
        if (FileType == TYPE_FILE)
          Size = sizeof(LONG);
        else
          {
          strcpy(FormatStr, " %ld");
          *((PLONG)pTmp) = 0;           // default value
          }
      break;
      case TYPE_FILE:
      case TYPE_TEXTFILE:
        if (FileType == TYPE_FILE)
          Size = sizeof(PVOID);
        else
          {
          strcpy(FormatStr, " %ld");
          *((PULONG)pTmp) = 0;          // default value
          }
      break;

      case TYPE_STRING:
        Size = *pBufLen;
        pMaxStrLen = (PUCHAR) ((ULONG) pTmp - 1L);
        if (*pMaxStrLen > (UCHAR)*pBufLen)
          Size = *pBufLen;
        else
          Size = (SHORT) *pMaxStrLen;
        if (CurveType)
          {
          pTmp = malloc(Size);
          if (pTmp == NULL)
            {
            error(ERROR_ALLOC_MEM);
            SetErrorFlag();
            return;
            }
          }
        if (FileType != TYPE_FILE)
          {
          sprintf(FormatStr, "%%%ds", Size);
          *(PCHAR)pTmp = '\0';          // default value
          }
      break;

      case TYPE_REAL4:
        if (CurveType)
          pTmp = &fTmp;
        if (FileType == TYPE_FILE)
          Size = sizeof(FLOAT);
        else
          strcpy(FormatStr, " %g");
        *(PFLOAT)pTmp = (FLOAT)0.0;            // default value
      break;
      case TYPE_REAL:
        if (CurveType)
          pTmp = &dTmp;
        if (FileType == TYPE_FILE)
          Size = sizeof(DOUBLE);
        else
          strcpy(FormatStr, " %lg");
        *(PDOUBLE)pTmp = 0.0;            // default value
      break;
      }

    if (FileType == TYPE_FILE)
      {
      if (fread(pTmp, Size, 1, hFile) != 1)
        {
        /* check to see if real error or just end of file */
        if (! feof(hFile))
          err = ERROR_MAC_READ_FAIL;
        }
      }
    else
      {
      if (Type != TYPE_STRING)
        {
        if (hFile != stdin)
          SkipToNumber(hFile);
        }

      if (Type != TYPE_BYTE)
        {
        if (hFile == stdin)
          {
          if (!con_scanf(FormatStr, pTmp))
            {
            /* ran out of leftover input string */
            GetConInput();
            // one more try, use default value if still null string
            con_scanf(FormatStr, pTmp);
            GotInput = TRUE;
            }
          }
        else if (fscanf(hFile, FormatStr, pTmp) == 0)
          err = ERROR_MAC_READ_FAIL;
        }
      else
        {
        if (hFile == stdin)
          {
          wTmp = 0;
          if (! con_scanf(FormatStr, &wTmp))
            {
            /* ran out of leftover input string */
            GetConInput();
            // one more try, use default value if still null string
            con_scanf(FormatStr, &wTmp);
            GotInput = TRUE;
            }
          }
        else if (fscanf(hFile, FormatStr, &wTmp) == 0)
          err = ERROR_MAC_READ_FAIL;
        *(BYTE *)pTmp = (BYTE) wTmp;
        }
      }

    if (CurveType)
      {
      PushToDataStack(&Curve, TYPE_CURVE, FALSE);
      PushToDataStack(pTmp, Type, FALSE);
      AssignToCurveClass();
      if (Type == TYPE_STRING)
        free(pTmp);
      }
    }  // for VarNum loop

  if (! err)
    {
    if (ToEOLN)
      {
      if (FilVarGiven)
        {
        do
          {
          if (fread(&wTmp, 1, 1, hFile) != 1)
            {
            /* check to see if real error or just end of file */
            if (! feof(hFile))
              err = error(ERROR_MAC_READ_FAIL);
            else
              wTmp = 0x0A;
            }
          }
        while (((wTmp & 0xFF) != 0x0A) && (!err));
        }
      else
        {
        /* make sure that a line was read on this go around */
        if (!GotInput && (CurConPos == ConStart))
          GetConInput();

        push_form_context();
        if (MacPlayBack) /* if in keystroke "menu" mode */
          {
          displayMacroForm(); /* make form visible */
          }
        setCurrentFormToMacroForm();
        CommandOutput[0] = '\0';          /* just erase line */
        Current.Form->field_index = 2;
        init_field();
        format_and_display_field(FALSE);
        pop_form_context();
        CurConPos = 0;
        ConStart = CurConPos;
        }
      }
    }
  RemoveItemsFromDataStack(GivenVarNum);

  if (err)
    SetErrorFlag();
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void WriteProc(BOOLEAN ToEOLN)
{
  SHORT FileType = TYPE_TEXTFILE;
  FILE *hFile = stdout;
  SHORT Type;
  SHORT BadParam = 0;
  SHORT VarNum, GivenVarNum;
  ERR_OMA err = ERROR_NONE;
  static CHAR OutputStr[151];
  static CHAR FormatStr[151];
  SHORT Size, i;
  BOOLEAN FilVarGiven = FALSE;

  UCHAR bTmp;
  USHORT wTmp;
  SHORT iTmp;
  LONG liTmp;
  DOUBLE dTmp;
  PVOID pTmp;
  FLOAT fTmp;

  if (! PopScalarFromDataStack(&GivenVarNum, TYPE_INTEGER))
    {
    error(ERROR_BAD_PARAM_TYPE, 0);
    SetErrorFlag();
    return;
    }

  VarNum = GivenVarNum;

  if (GivenVarNum)  /* check for empty call */
    {
    FileType = (DataStackPeek(GivenVarNum - 1) & ~POINTER_TO);
    if ((FileType == TYPE_FILE) || (FileType == TYPE_TEXTFILE))
      {
      IndexDataStack((GivenVarNum - 1), &hFile, TYPE_TEXTFILE);
      FilVarGiven = TRUE;
      VarNum--;
      }
    }

  if ((*pBufLen > MAXCONLEN) && (hFile == stdout))
    *pBufLen = MAXCONLEN;

  for (i=VarNum-1; i>=0; i--)
    {
    if (! ScalarDataStackPeek(i, &Type))
      {
      if (FilVarGiven)
        BadParam = VarNum - i + 1;
      else
        BadParam = VarNum - i;

      error(ERROR_BAD_PARAM_TYPE, BadParam);
      SetErrorFlag();
      return;
      }

    switch (Type)
      {
      case TYPE_BYTE:
        Size = sizeof(BYTE);
        pTmp = &bTmp;
        ScalarIndexDataStack(i, pTmp, Type);
        if (FileType != TYPE_FILE)
          {
          sprintf(FormatStr, "%%%dc", 2);
          sprintf(FormatStr, "%%c");
          sprintf(OutputStr, FormatStr, bTmp);
          }
      break;

      case TYPE_WORD:
        pTmp = &wTmp;
        ScalarIndexDataStack(i, pTmp, Type);
        if (FileType != TYPE_FILE)
          {
          if (*pBufLen == MAXCONLEN)
            Size = 6;
          else
            Size = *pBufLen;
          sprintf(FormatStr, "%%%du", abs(Size));
          sprintf(OutputStr, FormatStr, wTmp);
          StripExp(OutputStr, abs(*pBufLen), *pBufLen < 0);
          }
        else
          Size = sizeof(USHORT);
      break;

      case TYPE_INTEGER:
        pTmp = &iTmp;
        ScalarIndexDataStack(i, pTmp, Type);
        if (FileType != TYPE_FILE)
          {
          if (*pBufLen == MAXCONLEN)
            Size = 7;
          else
            Size = *pBufLen;
          sprintf(FormatStr, "%%%dd", abs(Size));
          sprintf(OutputStr, FormatStr, iTmp);
          StripExp(OutputStr, abs(*pBufLen), *pBufLen < 0);
          }
        else
          Size = sizeof(SHORT);
      break;

      case TYPE_LONG_INTEGER:
        pTmp = &liTmp;
        ScalarIndexDataStack(i, pTmp, Type);
        if (FileType != TYPE_FILE)
          {
          if (*pBufLen == MAXCONLEN)
            Size = 11;
          else
            Size = *pBufLen;
          sprintf(FormatStr, "%%%dld", abs(Size));
          sprintf(OutputStr, FormatStr, liTmp);
          StripExp(OutputStr, abs(*pBufLen), *pBufLen < 0);
          }
        else
          Size = sizeof(LONG);
      break;
      case TYPE_FILE:
      case TYPE_TEXTFILE:
        ScalarIndexDataStack(i, &pTmp, Type);
        if (FileType != TYPE_FILE)
          {
          if (*pBufLen == MAXCONLEN)
            Size = 10;
          else
            Size = *pBufLen;
          sprintf(FormatStr, "%%%dlu", Size);
          sprintf(OutputStr, FormatStr, (ULONG) pTmp);
          }
        else
          Size = sizeof(PVOID);
      break;

      case TYPE_STRING:
        ScalarIndexDataStack(i, &pTmp, Type | POINTER_TO);
        Size = strlen(pTmp);
        if (! FilVarGiven)
          {
          if (Size > MAXCONLEN)
            Size = MAXCONLEN;
          }
        if (FileType != TYPE_FILE)
          {
          sprintf(FormatStr, "%%%ds", Size);
          sprintf(OutputStr, FormatStr, pTmp);
          }
      break;

      case TYPE_REAL4:
        pTmp = &fTmp;
        ScalarIndexDataStack(i, pTmp, Type);
        if (FileType != TYPE_FILE)
          {
          gcvt(fTmp, 16, FormatStr);
          StripExp(FormatStr, abs(*pBufLen), *pBufLen < 0);
          strcpy(OutputStr, FormatStr);
          }
        else
          Size = sizeof(FLOAT);
      break;

      case TYPE_REAL:
        pTmp = &dTmp;
        ScalarIndexDataStack(i, pTmp, Type);
        if (FileType != TYPE_FILE)
          {
          gcvt(dTmp, 16, FormatStr);
          StripExp(FormatStr, abs(*pBufLen), *pBufLen < 0);
          strcpy(OutputStr, FormatStr);
          }
        else
          Size = sizeof(DOUBLE);
      break;
      }

    if (FileType == TYPE_FILE)
      {
      if (fwrite(pTmp, Size, 1, hFile) != 1)
        err = ERROR_MAC_WRITE_FAIL;
      }
    else
      {
      if (FilVarGiven)
        {
        if (Type != TYPE_BYTE)
          Size = strlen(OutputStr);

        if (fwrite(OutputStr, Size, 1, hFile) == 0)
          err = ERROR_MAC_WRITE_FAIL;
        }
      else
        {
        push_form_context();
        if (MacPlayBack) /* if in keystroke "menu" mode */
          {
          displayMacroForm(); /* make form visible */
          }
        setCurrentFormToMacroForm();
        strncpy(&CommandOutput[CurConPos], OutputStr, MAXCONLEN - CurConPos);
        CommandOutput[MAXCONLEN] = '\0';
        Current.Form->field_index = 2;
#ifdef MACTEST
        printf("%s", CommandOutput);
#else
        init_field();
        format_and_display_field(FALSE);
#endif
        pop_form_context();
        CurConPos = strlen(CommandOutput);
        ConStart = CurConPos;
        }
      }

    }  // for VarNum loop

  RemoveItemsFromDataStack(GivenVarNum);

  if (! err)
    {
    if (ToEOLN)
      {
      if (FilVarGiven)
        {
        if (fprintf(hFile, "\n") == 0)
          err = ERROR_MAC_WRITE_FAIL;
        }
      else       /* just erase line if empty call, else just reset start */
        {
        CommandOutput[0] = '\0';
        if (VarNum == 0)
          {
          push_form_context();
          if (MacPlayBack) /* if in keystroke "menu" mode */
            {
            displayMacroForm(); /* make form visible */
            }
          setCurrentFormToMacroForm();
          Current.Form->field_index = 2;
#ifdef MACTEST
          printf("\n");
#else
          init_field();
          format_and_display_field(FALSE);
#endif
          pop_form_context();
          }
        CurConPos = 0;
        ConStart = CurConPos;
        }
      }
    }

  if (err)
    {
    error(err);
    SetErrorFlag();
    }
}

/************************************************************************/
void GetCommandPrompt(void)
{
  PushToDataStack(CommandInput, TYPE_STRING | POINTER_TO, TRUE);
}

/************************************************************************/
void SetCommandPrompt(void)
{
  char * PromptString;

  if((PopFromDataStack(&PromptString, TYPE_STRING | POINTER_TO)) != TYPE_STRING)
    {
    error(ERROR_BAD_PARAM_TYPE, 1);
    SetErrorFlag();
    return;
    }

  push_form_context();
  if (MacPlayBack) /* if in keystroke "menu" mode */
    {
    displayMacroForm(); /* make form visible */
    }

  setCurrentFormToMacroForm();
  strncpy(CommandInput, PromptString, MAXCONLEN);
  CommandInput[MAXCONLEN] = '\0';
  Current.Form->field_index = 3;
  init_field();
  format_and_display_field(FALSE);
  pop_form_context();
}
