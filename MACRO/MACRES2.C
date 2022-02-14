/* -----------------------------------------------------------------------
/
/  macres2.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/macro/macres2.c_v   0.28   06 Jul 1992 12:51:22   maynard  $
/  $Log:   J:/logfiles/oma4000/macro/macres2.c_v  $
/
*/

#include <string.h>
#include <conio.h>
#include <math.h>

#include "macres2.h"
#include "macruntm.h"
#include "macnres.h"
#include "macrores.h"
#include "tempdata.h"
#include "points.h"
#include "curvedir.h"
#include "curvbufr.h"
#include "filestuf.h"
#include "mathops.h"
#include "curvdraw.h"
#include "fkeyfunc.h"
#include "cursor.h"
#include "di_util.h" 
#include "oma4000.h" 
#include "crventry.h"
#include "omaform.h"
#include "live.h"
#include "formwind.h"
#include "runforms.h"
#include "pltsetup.h"
#include "mdatstak.h"
#include "syserror.h"  // ERROR_OPEN
#include "omaerror.h"
#include "handy.h"
#include "omameth.h"
#include "plotbox.h"
#include "crvheadr.h"
#include "forms.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DeleteTempMathFiles()
{
   SHORT i;

   /* delete any temporary curve blocks that are present */
   for(i = MainCurveDir.BlkCount - 1; i >= 0; i--)
      if(strncmp(MainCurveDir.Entries[i].name, TempEntryName, 4) == 0)
         DelTempFileBlk(&MainCurveDir, i);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void LoadMethod(void)
{
   ERR_OMA err = ERROR_NONE;
   CHAR *FileName;
   FILE *hFile;
   LPMETHDR NewMethod;

   if (! PopScalarFromDataStack(&FileName, POINTER_TO | TYPE_STRING))
   {
      error(ERROR_BAD_PARAM_TYPE, 1);
      SetErrorFlag();
      return;
   }

   if(! (hFile = fopen(FileName, "rb"))) {
      error(ERROR_OPEN, FileName);
      err = ERROR_OPEN;
   }
   else
   {
      err = MethdrRead(hFile, FileName, & NewMethod);
      fclose(hFile);
   }

   if (err)
      SetErrorFlag();
   else
   {
      DeAllocMethdr(InitialMethod);
      InitialMethod = NewMethod;
      InitializePlotSetupFields(& DisplayGraphArea);

      /* initialize scan setup */
      MethodToDetInfo(InitialMethod);

      // get rid of copied detector data from the method
      DeAllocMetDetInfo(InitialMethod);
   }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SaveMethod(void)
{
  ERR_OMA err = ERROR_NONE;
  CHAR *FileName;
  FILE *hFile;


  if ((PopFromDataStack(&FileName,
     POINTER_TO | TYPE_STRING) & ~POINTER_TO) != TYPE_STRING)
    {
    error(ERROR_BAD_PARAM_TYPE, 1);
    SetErrorFlag();
    return;
    }

  if(! (hFile = fopen(FileName, "w+b"))) {
    error(ERROR_OPEN, FileName);
    err = ERROR_OPEN;
    }
  else
    {
    /* copy the plotbox values to InitailMethod */
    CopyPlotToMethod();

    /* copy the scan values to the method */
    err = DetInfoToMethod(& InitialMethod);

    if(! err)
       MethdrWrite(hFile, FileName, InitialMethod);

    fclose(hFile);
    // get rid of copied detector data from the method
    DeAllocMetDetInfo(InitialMethod);
    }

  if(err)
    SetErrorFlag();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DelCurveSet(void)
{
  CURVE_REF *pCurve;
  SHORT Type = PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO);

  if ((Type != TYPE_CURVE) || (pCurve->ReferenceType != CLASS_CURVESET))
    {
    error(ERROR_BAD_PARAM_TYPE, 1);
    SetErrorFlag();
    }
  else if(DelTempFileBlk(&MainCurveDir, pCurve->CurveSetIndex))
    SetErrorFlag();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DelCurve(void)
{
  CURVE_REF *pCurve;
  SHORT Type = PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO);

  if ((Type != TYPE_CURVE) || (pCurve->ReferenceType != CLASS_CURVE))
    {
    error(ERROR_BAD_PARAM_TYPE, 1);
    SetErrorFlag();
    }
  else if(DelMultiTempCurve(&MainCurveDir, pCurve->CurveSetIndex,
                            pCurve->CurveIndex, 1))
    SetErrorFlag();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Does the work for MacCreateCurveSet, can be called from elsewhere too
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA CreateCurveSet(char *Name, char *Path, char *Desc, int NIndex)
{
  CHAR PathBuf[FNAME_LENGTH];
  CHAR NameBuf[FNAME_LENGTH];
  SHORT OldIndex;
  USHORT DestinationStartCurve = 0;
  time_t Time;
  TM *TimeStruct;
  ERR_OMA err;
  
  /* make sure of string size */
  if (strlen(Name) > DOSFILESIZE)
    Name[DOSFILESIZE] = '\0';

  if (strlen(Path) > DOSPATHSIZE)
    Path[DOSPATHSIZE] = '\0';

  if (strlen(Desc) >= DESCRIPTION_LENGTH)
    Desc[DESCRIPTION_LENGTH - 1] = '\0';

  // test path and file name for correctness
  if (ParseFileName(PathBuf, Path) != 1)
    return error(ERROR_BAD_DIRNAME, PathBuf);

  strcat(PathBuf, "\\");
  strcat(PathBuf, Name);
  if (ParsePathAndName(PathBuf, NameBuf, PathBuf) != 2)
    return error(ERROR_BAD_FILENAME, NameBuf);

  OldIndex = 0;
  do
    {
    OldIndex = SearchNextNameAndCurveNum(NameBuf, &MainCurveDir, OldIndex,
                                         DestinationStartCurve);
    if (OldIndex != -1)
      {
      if (strcmpi(PathBuf, MainCurveDir.Entries[OldIndex].path) != 0)
        OldIndex = -1;
      else
        {
        // try another starting index
        DestinationStartCurve =
          MainCurveDir.Entries[OldIndex].StartIndex +
          MainCurveDir.Entries[OldIndex].count + 1;
        OldIndex = 0;
        }
      }
    }
  while (OldIndex != -1);

  time(&Time);
  TimeStruct = localtime(&Time);

  /* put the curve block entry into the current directory */
  err = InsertCurveBlkInDir(NameBuf, PathBuf, Desc, DestinationStartCurve,
                            0L, 0, TimeStruct, &MainCurveDir,
                            NIndex, OMA4DATA);
  if (err)
    return err;

  if ((USHORT) NIndex == (MainCurveDir.BlkCount - 1))
    MainCurveDir.Entries[NIndex].TmpOffset = TempFileSz;
  else
    MainCurveDir.Entries[NIndex].TmpOffset =
      MainCurveDir.Entries[NIndex+1].TmpOffset;

  return ERROR_NONE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacCreateCurveSet(void)
{
  PCHAR Name, Path, Desc;
  SHORT NewIndex;
  SHORT BadParam = 0;
  USHORT DestinationStartCurve = 0;

  if (! PopScalarFromDataStack(&NewIndex, TYPE_INTEGER))
    BadParam = 4;
  else if (! PopScalarFromDataStack(&Desc, POINTER_TO | TYPE_STRING))
    BadParam = 3;
  else if (! PopScalarFromDataStack(&Path, POINTER_TO | TYPE_STRING))
    BadParam = 2;
  else if (! PopScalarFromDataStack(&Name, POINTER_TO | TYPE_STRING))
    BadParam = 1;

  if (BadParam)
    {
    error(ERROR_BAD_PARAM_TYPE, BadParam);
    SetErrorFlag();
    return;
    }

  if (CreateCurveSet(Name, Path, Desc, NewIndex))
    SetErrorFlag();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void GCurveSetIndex(void)
{
  PCHAR Name, Path;
  USHORT CurveNumber;
  SHORT BadParam = 0;
  SHORT DirIndex;

  if (! PopScalarFromDataStack(&CurveNumber, TYPE_WORD))
    BadParam = 3;
  else if (! PopScalarFromDataStack(&Path, POINTER_TO | TYPE_STRING))
    BadParam = 2;
  else if (! PopScalarFromDataStack(&Name, POINTER_TO | TYPE_STRING))
    BadParam = 1;

  if (BadParam)
    {
    error(ERROR_BAD_PARAM_TYPE, BadParam);
    SetErrorFlag();
    return;
    }
   
  DirIndex = GetCurveSetIndex(Name, Path, CurveNumber);

  PushToDataStack(&DirIndex, TYPE_INTEGER, FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void GActiveWindow(void)
{
  USHORT Window;

  Window = (ActiveWindow + 1);
  PushToDataStack(&Window, TYPE_INTEGER, FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void GActivePlotSetup(void)  
{
  USHORT PlotIndex;

  PlotIndex = (ActiveWindow + 1);
  PushToDataStack(&PlotIndex, TYPE_INTEGER, FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SWindowStyle(void)  
{
  int TempWS;

  if (! PopScalarFromDataStack(&TempWS, TYPE_BYTE))
    {
    error(ERROR_BAD_PARAM_TYPE, 1);
    SetErrorFlag();
    }
  else            // limit to byte, make it zero based.
    {
    window_style = ((TempWS & 0xFF) - 1);

    if (window_style < 0)
      window_style = 0;
    else if (window_style > 9)
      window_style = 9;
    }

  InitPlotBox(ActivePlot);
  // if plotarea is showing and in correct mode     
  if (((active_locus != LOCUS_FORMS) && (active_locus != LOCUS_POPUP)) ||
      ((isCurrentFormMacroForm()) && (isPrevFormMenu1() ||
      isPrevFormGraphWindow())))
    PutUpPlotBoxes();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SAccum(void)
{
  SHORT BadParam = 0;

  if (! PopScalarFromDataStack(&BackGroundActive, TYPE_BOOLEAN))
    BadParam = 1;

  if (BadParam)
    {
    error(ERROR_BAD_PARAM_TYPE, BadParam);
    SetErrorFlag();
    return;
    }

  if (BackGroundActive)
    FKeyItems[1].Text = GoAccumBackStr;
  else
    FKeyItems[1].Text = GoAccumStr;

  FKeyItems[1].TextLen = (char) 0;
  FKeyItems[1].Control &= ~ MENUITEM_INACTIVE;
  DoAccum = TRUE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SLive(void)
{
  SHORT BadParam = 0;

  if (! PopScalarFromDataStack(&BackGroundActive, TYPE_BOOLEAN))
    BadParam = 1;

  if (BadParam)
    {
    error(ERROR_BAD_PARAM_TYPE, BadParam);
    SetErrorFlag();
    return;
    }

  if (BackGroundActive)
    FKeyItems[1].Text = GoLiveBackStr;
  else
    FKeyItems[1].Text = GoLiveStr;

  FKeyItems[1].TextLen = (char) 0;
  FKeyItems[1].Control &= ~ MENUITEM_INACTIVE;
  DoAccum = FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SFrameCapture(void)
{
  BOOLEAN TempBool;

  if (! PopScalarFromDataStack(&TempBool, TYPE_BOOLEAN))
    {
    error(ERROR_BAD_PARAM_TYPE, 1);
    SetErrorFlag();
    }
  else
    SaveLiveFrame = TempBool;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CSCount(void)
{
  PushToDataStack(&(MainCurveDir.BlkCount), TYPE_INTEGER, FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DCCount()
{
  SHORT i;

  CursorStatus[ActiveWindow].TotalCurves = 0;
  for (i=0; i<(SHORT) MainCurveDir.BlkCount; i++)
  {
     if (MainCurveDir.Entries[i].DisplayWindow & (1 << ActiveWindow))
        CursorStatus[ActiveWindow].TotalCurves +=
        MainCurveDir.Entries[i].count;
  }

  PushToDataStack(&(CursorStatus[ActiveWindow].TotalCurves), TYPE_WORD,
  FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MCCount(void)
{
  PushToDataStack(&(MainCurveDir.CurveCount), TYPE_WORD, FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DOUBLE DoBinMath(LPCURVEDIR pCurveDir, PSHORT pEntryIndex,
                 PUSHORT StartCurves, PUSHORT NumCurves,
                 PDOUBLE ConstantVal, SHORT Operator,
                 BOOLEAN KbChk)
{
  SHORT Key, prefBuf0 = 0, prefBuf1 = 1, prefBuf2 = 2;
  USHORT i, j, k, CurveCount[3], CurveIndex[3], EntryIndex[3], UnitCount,
         RepCount; /* the number of curve to do before recycling */
  LONG ly[2];
  FLOAT x[2], floatTemp;
  DOUBLE y[2], RoundNum[2], fTemp;
  BOOLEAN XValWarning = FALSE, Switched = FALSE;
  ERR_OMA err;
  CURVEHDR Curvehdr;
  LPCURVE_ENTRY pEntry[3], TempEntry;

  for (i=0; i<3; i++)
    {
    if (pEntryIndex[i] != -1)
      pEntry[i] = &(pCurveDir->Entries[pEntryIndex[i]]);
    else
      {
      pEntry[i] = NULL;
      x[i] = (FLOAT) 0.0;
      if (i!=2)
        y[i] = ConstantVal[i];
      }

    CurveCount[i] = NumCurves[i];
    EntryIndex[i] = pEntryIndex[i];
    }

  /* put the block with the largest number of curves in the operand 1 spot */
  if ((CurveCount[0] < CurveCount[1]) ||
    ((pEntryIndex[0] == -1) && (pEntryIndex[1] != -1)))
    {
    TempEntry = pEntry[0];
    pEntry[0] = pEntry[1];
    pEntry[1] = TempEntry;
    CurveIndex[0] = EntryIndex[0];
    EntryIndex[0] = EntryIndex[1];
    EntryIndex[1] = CurveIndex[0];
    CurveIndex[0] = CurveCount[0];
    CurveCount[0] = CurveCount[1];
    CurveCount[1] = CurveIndex[0];
    CurveIndex[0] = StartCurves[1];
    CurveIndex[1] = StartCurves[0];
    fTemp = y[0];
    y[0] = y[1];
    y[1] = fTemp;

    floatTemp = x[ 0 ];
    x[0] = x[1];
    x[ 1 ] = floatTemp;

    Switched = TRUE;
    }
  else
    {
    CurveIndex[0] = StartCurves[0];
    CurveIndex[1] = StartCurves[1];
    }

  CurveIndex[2] = StartCurves[2];

  switch (Operator)
    {
    /* unary operations */
    case LOG:
    case LN:
    case TRUNC:
    case ROUND:
    case BITNOT:
    case SIN:
    case COS:
    case TAN:
    case ASIN:
    case ACOS:
    case ATAN:
    case ABS:
      UnitCount = 1;
      RepCount = CurveCount[0];
    break;

    /* binary operations */
    default:
      UnitCount = CurveCount[1];
      RepCount = CurveCount[0] / CurveCount[1];
    }

  for (i=0; i<RepCount; i++)
    {
    for (j=0; j<UnitCount; j++)
      {
      if (EntryIndex[2] != -1)
        {
        if (err = ReadTempCurvehdr(pCurveDir, EntryIndex[2],
          CurveIndex[2], &Curvehdr))
          return y[0];
        }
      else
        Curvehdr.pointnum = 1;
      for (k=0; k<Curvehdr.pointnum; k++)
        {
        if (EntryIndex[0] != -1)
          {
          if(GetDataPoint(pCurveDir, EntryIndex[0], CurveIndex[0], k,
            &(x[0]), &(y[0]), DOUBLETYPE, &prefBuf0))
            return y[0];
          }

        if (pEntry[1] == NULL)
          x[1] = x[0];
        else
          {
          if(GetDataPoint(pCurveDir, EntryIndex[1], CurveIndex[1], k,
            &(x[1]), &(y[1]), DOUBLETYPE, &prefBuf1))
            return y[0];
          }

        if (Switched)
          {
          fTemp = y[0];
          y[0] = y[1];
          y[1] = fTemp;
          }

        RoundNum[0] = 0.5;
        RoundNum[1] = 0.5;
        if (y[0] < 0.0)
          RoundNum[0] = -0.5;
        if (y[1] < 0.0)
          RoundNum[1] = -0.5;
        switch (Operator)       // use fTemp as the resultant
          {
          case PLUS:
            fTemp = y[0] + y[1];
          break;

          case SUB:
            fTemp = y[0] - y[1];
          break;

          case MULTIPLY:
            fTemp = y[0] * y[1];
          break;

          case DIVIDE:
            if (y[1] != 0.0)
              fTemp = y[0] / y[1];
            else
              fTemp = MaxErrorVal;
          break;

          case ABSORB:
            if (y[1] != 0.0)
              {
              fTemp = y[0] / y[1];
              if (fTemp > 0.0)
                fTemp = -log10(fTemp);
              else
                fTemp = MinErrorVal;
              }
            else
              fTemp = MaxErrorVal;
          break;

          case LOG:
            if (y[0] > 0.0)
              fTemp = log10(y[0]);
            else
              fTemp = MinErrorVal;
          break;

          case LN:
            if (y[0] > 0.0)
              fTemp = log(y[0]);
            else
              fTemp = MinErrorVal;
          break;

          case INTDIV:
            ly[0] = (LONG) (y[0] + RoundNum[0]);
            ly[1] = (LONG) (y[1] + RoundNum[1]);
            if (ly[1] != 0L)
              fTemp = (DOUBLE) (ly[0] / ly[1]);
            else
              fTemp = (DOUBLE) 0x7FFFFFFF;
          break;

          case MOD:
            ly[0] = (LONG) (y[0] + RoundNum[0]);
            ly[1] = (LONG) (y[1] + RoundNum[1]);
            if (ly[1] != 0L)
              fTemp = (DOUBLE) (ly[0] % ly[1]);
            else
              fTemp = 0.0;
          break;

          case TRUNC:
            fTemp = (DOUBLE) ((LONG)y[0]);
          break;

          case ROUND:
            fTemp = (DOUBLE) ((LONG)(y[0] + RoundNum[0]));
          break;

          case ABS:
            fTemp = fabs(y[0]);
          break;

          case AND:
            ly[0] = (LONG) (y[0] + RoundNum[0]);
            ly[1] = (LONG) (y[1] + RoundNum[1]);
            fTemp = (DOUBLE) (ly[0] & ly[1]);
          break;

          case OR:
            ly[0] = (LONG) (y[0] + RoundNum[0]);
            ly[1] = (LONG) (y[1] + RoundNum[1]);
            fTemp = (DOUBLE) (ly[0] | ly[1]);
          break;

          case XOR:
            ly[0] = (LONG) (y[0] + RoundNum[0]);
            ly[1] = (LONG) (y[1] + RoundNum[1]);
            fTemp = (DOUBLE) (ly[0] ^ ly[1]);
          break;

          case BITNOT:
            ly[0] = (LONG) (y[0] + RoundNum[0]);
            fTemp = (DOUBLE) (~ly[0]);
          break;

          case SHL:
            ly[0] = (LONG) (y[0] + RoundNum[0]);
            ly[1] = (LONG) (y[1] + RoundNum[1]);
            fTemp = (DOUBLE) (ly[0] << ly[1]);
          break;

          case SHR:
            ly[0] = (LONG) (y[0] + RoundNum[0]);
            ly[1] = (LONG) (y[1] + RoundNum[1]);
            fTemp = (DOUBLE) (ly[0] >> ly[1]);
          break;

          case EXP:
            fTemp = pow(y[0], y[1]);
          break;

          case SIN:      /* angle in radians */
            fTemp = sin(y[0]);
          break;

          case COS:      /* angle in radians */
            fTemp = cos(y[0]);
          break;

          case TAN:      /* angle in radians */
            fTemp = tan(y[0]);
          break;

          case ASIN:      /* angle in radians */
            fTemp = asin(y[0]);
          break;

          case ACOS:      /* angle in radians */
            fTemp = acos(y[0]);
          break;

          case ATAN:      /* angle in radians */
            fTemp = atan(y[0]);
          break;

          case ATAN2:      /* angle in radians */
            fTemp = atan2(y[0], y[1]);
          break;

          case EQUALTO:
            fTemp = (DOUBLE) (y[0] == y[1]);
          break;

          case NOTEQUALTO:
            fTemp = (DOUBLE) (y[0] != y[1]);
          break;

          case LESSTHAN:
            fTemp = (DOUBLE) (y[0] < y[1]);
          break;

          case GREATERTHAN:
            fTemp = (DOUBLE) (y[0] > y[1]);
          break;

          case LESSTHANEQ:
            fTemp = (DOUBLE) (y[0] <= y[1]);
          break;

          case GREATERTHANEQ:
            fTemp = (DOUBLE) (y[0] >= y[1]);
          break;
          }

        if(!InLiveLoop)
          {
          if(((x[1] - x[0]) > (float)MAX_FLOAT_DIFF) && (!XValWarning))
            {
            /* put up warning for different X's */
            char * DiffXPrompt[] =
              { "Different X values were detected",
                "Continue?", NULL };

            XValWarning = TRUE;
            if(yes_no_choice_window(DiffXPrompt, 1, COLORS_MESSAGE) != YES)
              return fTemp;
            }
          }

        if (EntryIndex[2] != -1)
          {
          if(SetDataPoint(pCurveDir, EntryIndex[2], CurveIndex[2], k,
            &(x[0]), &fTemp, DOUBLETYPE, &prefBuf2))
            return fTemp;
          }
        if (Switched)      // switch back
          {
          fTemp = y[0];
          y[0] = y[1];
          y[1] = fTemp;
          }
        }
      CurveIndex[0]++;
      CurveIndex[1]++;
      CurveIndex[2]++;

      /* check for user escape */
      if (KbChk)
        {
        if (kbhit())
          {
          Key = getch();
          if (Key == ESCAPE)
            return fTemp;
          }
        }
      }
    CurveIndex[1] = StartCurves[1];
    }
  return fTemp;
}

/****************************************************************************/
/* Do smoothing or derivative.  Smoothing uses a 3 point binomial filter.   */
/* Ref: Marchand, P. and Marnet, L., Rev. Sci. Instrum. 54 (8), Aug 1983,   */
/* "Binomial smoothing filter: A way to avoid some pitfalls of least-squares*/
/* polynomial smoothing"                                                    */
/* Source and destination curves may be different or the same.  Block is an */
/* array of integers; Block[0] is the CurveDir index of the source data,    */
/* Block[2] is the CurveDir index of the destination data.  Block[1] is not */
/* used here.  In the same way Start gives the starting curve numbers, and  */
/* Count gives the curve count (only the source count is used: RunMath makes*/
/* sure that source and dest have the same count).  If Degree is 0, then a  */
/* derivative is done as a special case of the smoothing algorithm, else    */
/* the smooth is repeated for Degree times, yielding the effect of 3, 6,    */
/* or 12 point smooths, etc.                                                */
/****************************************************************************/

BOOLEAN DoBinom(OP_BLOCK *Src, OP_BLOCK *Dst, int Degree)
{
  SHORT SrcBuf = 0, DestBuf = 1;
  USHORT i,                      /* Current source curve # */
         j,                      /* Current point # */
         k,                      /* Current destination curve # */
        SrcSet = Src->BlkIndex, DstSet = Dst->BlkIndex;
  FLOAT LastX, ThisX, NextX;
  DOUBLE LastY, ThisY, NextY, NewY;
  CURVEDIR * pDir = &MainCurveDir;
  CURVEHDR CHeader;

  k = Dst->Start;
  for (i = Src->Start; i < Src->Count; i++, k++)
    {
    if (ReadTempCurvehdr(pDir, SrcSet, i, &CHeader))
      return(TRUE);

    if (GetDataPoint(pDir, SrcSet, i, 0, &LastX, &LastY, DOUBLETYPE, &SrcBuf))
      return(TRUE);

    if (!Degree)  /* Do Derivative */
      {
      for (j = 0; j < CHeader.pointnum-1; j++)
        {
        if (GetDataPoint(pDir, SrcSet, i, j, &ThisX, &ThisY, DOUBLETYPE, &SrcBuf))
          return TRUE;
        if (GetDataPoint(pDir, SrcSet, i, j+1, &NextX, &NextY, DOUBLETYPE, &SrcBuf))
        return TRUE;
        NewY = NextY - LastY;
        if (SetDataPoint(pDir, DstSet, k, j, &ThisX, &NewY, DOUBLETYPE, &DestBuf))
          return TRUE;
        LastY = ThisY;
        LastX = ThisX;
        }
      if (GetDataPoint(pDir, SrcSet, i, j, &ThisX, &ThisY, DOUBLETYPE, &SrcBuf))
        return TRUE;
      NewY = ThisY - LastY;
      if (SetDataPoint(pDir, DstSet, k, j, &ThisX, &NewY, DOUBLETYPE, &DestBuf))
        return TRUE;
      }
    else  /* Do Smooth */
      {                        /* for best performance: */
      SHORT  TmpDegree = Degree;
      USHORT OldSrc = SrcSet,  /* On 1st pass, src and dst sets should use */
             OldCrv = i;       /* different curve buffers. On next passes, */
                               /* src and dst are same, so use same buffer */

      while (TmpDegree--)
        {
        ThisY = LastY;
        for (j = 0; j < CHeader.pointnum-1; j++)
          {
          if (GetDataPoint(pDir,SrcSet,i,j+1,&NextX,&NextY,DOUBLETYPE, &SrcBuf))
            return TRUE;
          NewY = (ThisY + NextY) / 2.0;
          if (SetDataPoint(pDir, DstSet, k, j, &ThisX, &NewY, DOUBLETYPE, &DestBuf))
            return TRUE;
          ThisX = NextX;
          ThisY = NextY;
          }
        do
          {
          if (GetDataPoint(pDir,SrcSet,i,j-1,&NextX,&NextY,DOUBLETYPE, &SrcBuf))
            return TRUE;
          if (GetDataPoint(pDir,SrcSet,i,j,&ThisX,&ThisY,DOUBLETYPE, &SrcBuf))
            return TRUE;
          NewY = (NextY + ThisY) / 2.0;
          if (SetDataPoint(pDir, DstSet, k, j, &ThisX, &NewY, DOUBLETYPE, &DestBuf))
            return TRUE;
          }
          while (--j);
        if (SetDataPoint(pDir, DstSet, k, j, &LastX, &LastY, DOUBLETYPE, &DestBuf))
          return TRUE;
        SrcSet = DstSet;  /* so subsequent passes use already smoothed data */
        i = k;
        SrcBuf = DestBuf;  /* so LoadCurveBuf won't have to load next pass */
        }
      SrcBuf = 0;
      DestBuf = 1;
      SrcSet = OldSrc;
      i = OldCrv;
      }
    }
  return FALSE;
}

/****************************************************************/
void MacDoSmoothOrDeriv(void)
{
  SHORT Degree, CurveCount, BadParam = 0;
  CURVE_REF StartCurve,
            *pCurve;
  OP_BLOCK Src;

  if (!PopScalarFromDataStack(&Degree, TYPE_WORD))
    BadParam = 3;
  else if (! PopScalarFromDataStack(&CurveCount, TYPE_WORD))
    BadParam = 2;
  else
    {
    SHORT Type = PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO);
    
    StartCurve = * pCurve;
    if ((Type != TYPE_CURVE))
      BadParam = 1;
    }
  
  if (BadParam)
    {
    MacBadParam(BadParam);
    return;
    }

  Src.BlkIndex = StartCurve.CurveSetIndex;
  Src.Start = StartCurve.CurveIndex;
  Src.Count = CurveCount;
  if (DoBinom(&Src, &Src, Degree))
    SetErrorFlag();
}

/* Calculate the slope and intercept of a line connecting the */
/* start and end of the designated curve area */

static struct {
  FLOAT Slope;
  FLOAT Icpt;
  } LineEqu;

ERR_OMA FindLineEqu(SHORT CvSet, USHORT Curve,
                    SHORT Point1, SHORT Points, USHORT *prefBuf)
{
  FLOAT X;
  DOUBLE BaseY1, BaseY2;

  if (GetDataPoint(&MainCurveDir, CvSet, Curve,
                   Point1, &X, &BaseY1, DOUBLETYPE, prefBuf) ||
      GetDataPoint(&MainCurveDir, CvSet, Curve,
                   Point1+Points-1, &X, &BaseY2, DOUBLETYPE, prefBuf))
      return -1;

    LineEqu.Slope = (float)(BaseY2 - BaseY1) / (Points);
    LineEqu.Icpt = -LineEqu.Slope*(Point1+Points)+BaseY2;

  return 0;
}

/*************************************************************************/
/* Calculate Average, Std. Deviation, and Area or Volume of the selected */
/* part of the source curve.                                             */
/*                                                                       */
/*************************************************************************/
void DoStats(OP_BLOCK *Src, int FirstPoint, int PointCount, STAT_STRUCT *Stats)
{
  SHORT j, CvSet = Src->BlkIndex, prefBuf = 0;
  USHORT i;
  ULONG Temp;
  FLOAT X, Slope, Icpt;
  DOUBLE Y;
  ERR_OMA err = ERROR_NONE;

  clearAllCurveBufs();
  Stats->Avg = Stats->Dev = (double)0;
  // first get the Average
  for (i = Src->Start; i < Src->Start + Src->Count; i++)
    {
    /* Get slope and intercept of line to use to correct for offset */
    if (FindLineEqu(CvSet, i, FirstPoint, PointCount, &prefBuf))
      return;
    
    Slope = LineEqu.Slope;
    Icpt  = LineEqu.Icpt;

    for (j = FirstPoint; j < FirstPoint + PointCount; j++)
      {
      err = GetDataPoint(&MainCurveDir, CvSet, i, j, &X, &Y, DOUBLETYPE, &prefBuf);
      if (err)
        return;
      
      if (Stats->DoCArea)
        Y = Y - ((Slope*j)+Icpt); // Correct for baseline;

      Stats->Avg += Y;
      }
    }
  Stats->Area = Stats->Avg;

  Temp = (ULONG)((ULONG)Src->Count * (ULONG)PointCount);
  if (Stats->Avg != 0)
    Stats->Avg /= (double)Temp;
  else
    Stats->Avg = 1;

  // now calculate the Deviation
  for (i = Src->Start; i < Src->Start + Src->Count; i++)
    {
    if (FindLineEqu(CvSet, i, FirstPoint, PointCount, &prefBuf))
      return;
    
    Slope = LineEqu.Slope;
    Icpt  = LineEqu.Icpt;

    for (j = FirstPoint; j < FirstPoint + PointCount; j++)
      {
      err = GetDataPoint(&MainCurveDir, CvSet, i, j, &X, &Y, DOUBLETYPE, &prefBuf);
      if (err)
        return;

      if (Stats->DoCArea)
        Y = Y - ((Slope*j)+Icpt); // Correct for baseline;

      Stats->Dev += (Y - Stats->Avg) * (Y - Stats->Avg);
      }
    }
  if (Stats->Dev != 0)
    Stats->Dev /= (double)(Temp - 1);
  else
    Stats->Dev = 1;
  Stats->Dev = sqrt(Stats->Dev);
}

