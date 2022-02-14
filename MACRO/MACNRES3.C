/* -----------------------------------------------------------------------
/
/  macnres3.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/macro/macnres3.c_v   0.24   06 Jul 1992 12:50:46   maynard  $
/  $Log:   J:/logfiles/oma4000/macro/macnres3.c_v  $
/
*/

#include <string.h>
#include <stdlib.h>

#include "macnres3.h"
#include "points.h"
#include "tempdata.h"
#include "omaerror.h"
#include "syserror.h"
#include "oma4driv.h"
#include "detsetup.h"
#include "mdatstak.h"
#include "macrofrm.h"   // pDefaultName
#include "macnres.h"
#include "macres2.h"
#include "macnres2.h"
#include "crventry.h"
#include "crvheadr.h"
#include "curvedir.h"   // MainCurveDir
#include "filestuf.h"
#include "fileform.h"
#include "di_util.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AssignToCurveSetHdr(CURVE_REF lCurve)
{
  LPCURVE_ENTRY pEntry;
  ERR_OMA err = ERROR_NONE;
  PCHAR String;
  USHORT i;

  if (lCurve.CurveSetIndex > MainCurveDir.BlkCount)
    {
    error(ERROR_BAD_CURVE_BLOCK, lCurve.CurveSetIndex);
    SetErrorFlag();
    return;
    }

  pEntry = &(MainCurveDir.Entries[lCurve.CurveSetIndex]);

  switch (lCurve.Point_Or_HeaderItem)
    {
    case CURVE_START_INDEX:
      if (! PopScalarFromDataStack(&(pEntry->StartIndex), TYPE_WORD))
        err = error(ERROR_BAD_ASSN_RTYPE);
    break;

    case CURVE_NAME:

      /* if the curve set being changed is lastlive, don't really */
      /* rename it; instead, copy it to a new curve set with the */
      /* target name */

      if (! PopScalarFromDataStack(&String, POINTER_TO | TYPE_STRING))
        err = error(ERROR_BAD_ASSN_RTYPE);
      else
        {
        if (is_special_name(pEntry->name))
          {
          err = InsertCurveBlkInDir(String, "", pEntry->descrip,
                                    0, pEntry->StartOffset, 0,
                                    &pEntry->time, &MainCurveDir,
                                    MainCurveDir.BlkCount, OMA4DATA);
          if (!err)
            {
            i = GetCurveSetIndex(String, "", 0);
            MainCurveDir.Entries[i].TmpOffset = TempFileSz;
            err = InsertMultiTempCurve(&MainCurveDir, lCurve.CurveSetIndex,
                                      0, i, 0, pEntry->count);
            }
          }
        else
          {
          char Buffer[81];
          if (ParseFileName(Buffer, String) != 2) /* if name is illegal */
            error(ERROR_BAD_FILENAME, String);
          else
            {
            }
            strncpy(pEntry->name, String, DOSFILESIZE);
            pEntry->name[DOSFILESIZE] = '\0';
          }
        }
    break;

    case CURVE_PATH:
      if (! PopScalarFromDataStack(&String, POINTER_TO | TYPE_STRING))
        err = error(ERROR_BAD_ASSN_RTYPE);
      else
        {
        strncpy(pEntry->path, String, DOSPATHSIZE);
        pEntry->path[DOSPATHSIZE] = '\0';
        }
    break;

    case CURVE_DESC:
      if (! PopScalarFromDataStack(&String, POINTER_TO | TYPE_STRING))
        err = error(ERROR_BAD_ASSN_RTYPE);
      else
        {
        strncpy(pEntry -> descrip, String, DESCRIPTION_LENGTH);
        pEntry -> descrip[ DESCRIPTION_LENGTH - 1 ] = '\0';
        }
    break;

    case CURVE_DISPLAY:
      if (! PopScalarFromDataStack(&pEntry->DisplayWindow, TYPE_WORD))
        err = error(ERROR_BAD_ASSN_RTYPE);
    break;

    default:
      err = error(ERROR_READ_ONLY_LTYPE);
    break;
    }

  if(err)
    SetErrorFlag();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AssignToCurveHdr(CURVE_REF lCurve)
{
  CURVEHDR Curvehdr;
  USHORT PointNum;
  ERR_OMA err = ERROR_NONE;
  BOOLEAN RightSide = TRUE;
  DOUBLE FillVal = 0;

  if(ReadTempCurvehdr(&MainCurveDir, lCurve.CurveSetIndex,
    lCurve.CurveIndex, &Curvehdr))
    {
    SetErrorFlag();
    return;
    }

  switch (lCurve.Point_Or_HeaderItem)
    {
    case CURVE_POINT_COUNT:
      if (! PopScalarFromDataStack(&PointNum, TYPE_WORD))
        err = ERROR_BAD_ASSN_RTYPE;

      if (PointNum > Curvehdr.pointnum)
        {
        PushToDataStack(&lCurve, TYPE_CURVE, FALSE);
        PushToDataStack(&PointNum, TYPE_WORD, FALSE);
        PushToDataStack(&RightSide, TYPE_BOOLEAN, FALSE);
        PushToDataStack(&FillVal, TYPE_REAL, FALSE);
        MacChangeCurveSize();
        }
      else
        {
        if(ChangeCurveSize(&MainCurveDir, lCurve.CurveSetIndex,
          lCurve.CurveIndex, PointNum,
          (CHAR) Curvehdr.DataType))
          {
          SetErrorFlag();
          return;
          }
        }
    break;

    case CURVE_TIME:
      if (! PopScalarFromDataStack(&(Curvehdr.time), FLOATTYPE))
        err = ERROR_BAD_ASSN_RTYPE;
    break;

    case CURVE_YMIN:
      if (! PopScalarFromDataStack(&(Curvehdr.Ymin), FLOATTYPE))
        err = ERROR_BAD_ASSN_RTYPE;
    break;

    case CURVE_YMAX:
      if (! PopScalarFromDataStack(&(Curvehdr.Ymax), FLOATTYPE))
        err = ERROR_BAD_ASSN_RTYPE;
    break;

    case CURVE_XMIN:
      if (! PopScalarFromDataStack(&(Curvehdr.Xmin), FLOATTYPE))
        err = ERROR_BAD_ASSN_RTYPE;
    break;

    case CURVE_XMAX:
      if (! PopScalarFromDataStack(&(Curvehdr.Xmax), FLOATTYPE))
        err = ERROR_BAD_ASSN_RTYPE;
    break;

    case CURVE_FRAME:
      if (! PopScalarFromDataStack(&(Curvehdr.Frame), USHORTTYPE))
        err = ERROR_BAD_ASSN_RTYPE;
    break;

    case CURVE_TRACK:
      if (! PopScalarFromDataStack(&(Curvehdr.Track), USHORTTYPE))
        err = ERROR_BAD_ASSN_RTYPE;
    break;

    case CURVE_XUNITS:
      if (! PopScalarFromDataStack(&(Curvehdr.XData.XUnits), UCHARTYPE))
        err = ERROR_BAD_ASSN_RTYPE;
    break;

    case CURVE_YUNITS:
      if (! PopScalarFromDataStack(&(Curvehdr.YUnits), UCHARTYPE))
        err = ERROR_BAD_ASSN_RTYPE;
    break;

    case CURVE_SCMP:
      if (! PopScalarFromDataStack(&(Curvehdr.scomp), ULONGTYPE))
        err = ERROR_BAD_ASSN_RTYPE;
    break;
    }

  // points already written
  if ((lCurve.Point_Or_HeaderItem != CURVE_POINT_COUNT) && ! err)
    {
    if(WriteTempCurvehdr(&MainCurveDir, lCurve.CurveSetIndex,
      lCurve.CurveIndex, &Curvehdr))
      {
      SetErrorFlag();
      return;
      }
    }

  if(err)
    {
    error(err);
    SetErrorFlag();
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AssignToPoint(CURVE_REF lCurve)
{
  DOUBLE Y, Value;
  FLOAT X;
  SHORT prefBuf = -1;

  ERR_OMA err = ERROR_NONE;
  CURVEHDR Curvehdr;
  BOOLEAN Changed = FALSE;

  /* use doubles as intermediate values */
  if (! PopScalarFromDataStack(&Value, TYPE_REAL))
    {
    error(ERROR_BAD_ASSN_RTYPE);
    SetErrorFlag();
    return;
    }

  err = ReadTempCurvehdr(&MainCurveDir, lCurve.CurveSetIndex,
                         lCurve.CurveIndex, &Curvehdr);

  /* get the X and Y values for the data point */
  if (! err)
    {
    err = GetDataPoint(&MainCurveDir, lCurve.CurveSetIndex,
      lCurve.CurveIndex, lCurve.PointIndex, &X, &Y,
      (CHAR) TYPE_REAL, &prefBuf);
    }

  if (err)
    {
    SetErrorFlag();
    return;
    }

  if (lCurve.Point_Or_HeaderItem != CURVE_POINTX)
    {
    Y = Value;
    if ((FLOAT)Y < Curvehdr.Ymin)
      {
      Curvehdr.Ymin = (FLOAT) Y;
      Changed = TRUE;
      }
    if ((FLOAT)Y > Curvehdr.Ymax)
      {
      Curvehdr.Ymax = (FLOAT)Y;
      Changed = TRUE;
      }
    }
  else
    {
    X = (FLOAT) Value;
    if(X < Curvehdr.Xmin)
      {
      Curvehdr.Xmin = X;
      Changed = TRUE;
      }
    if(X > Curvehdr.Xmax)
      {
      Curvehdr.Xmax = X;
      Changed = TRUE;
      }
    }
  if (Changed)
    {
    err = WriteTempCurvehdr(&MainCurveDir, lCurve.CurveSetIndex,
      lCurve.CurveIndex, &Curvehdr);
    }

  /* Set the X and Y values for the data point */
  if (! err)
    {
    err = SetDataPoint(&MainCurveDir, lCurve.CurveSetIndex,
      lCurve.CurveIndex, lCurve.PointIndex, &X, &Y,
      (CHAR) TYPE_REAL, &prefBuf);
    }
  if(err)
     SetErrorFlag();
}

/**********************************************************************/
/* makes a new empty curve set; returns directory index or -1 if fail */
/**********************************************************************/
USHORT MakeNewCurveSet(void)
{
  CHAR DefaultNameStr[_MAX_FNAME];
  static SHORT DefaultNameCount = 0; // never decremented or reset ??
  USHORT returnIndex;

  sprintf(DefaultNameStr, "%s%d", pDefaultName, DefaultNameCount);
  DefaultNameCount++;

  returnIndex = MainCurveDir.BlkCount;

  if(CreateCurveSet(DefaultNameStr, "", "Auto allocated curve set", returnIndex))
    returnIndex = -1;

  return returnIndex;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AssignToCurveClass()
{
  USHORT         i, j, lCurveCount, rCurveCount, DelCount, NewCount,
                 SrcCurveIndex, DstCurveIndex;
  SHORT          rType = (DataStackPeek(0) & ~(POINTER_TO)), prefBuf = -1;
  CURVE_REF     *pCurve, lCurve, rCurve;
  DOUBLE         dTemp;
  CURVEHDR       Curvehdr;
  BOOLEAN        Scalar, TempFile = FALSE, MakeNewSet = FALSE;
  LPCURVE_ENTRY  LpEntry, RpEntry;
  CHAR *         strptr;

  /* check for a possible multiple value assignment */
  if(rType == TYPE_CURVE)
    {
    PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO);
    rCurve = * pCurve;
    Scalar = FALSE;
    }
  else
    {
    if (rType != TYPE_STRING)
      PopScalarFromDataStack(&dTemp, TYPE_REAL);
    else
      PopFromDataStack(&strptr, (POINTER_TO | TYPE_STRING));
    Scalar = TRUE;
    }

  PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO);
  lCurve = * pCurve;

  // automatically generate a new curve set if trying to assign to a  
  // curve set that doesn't exist

  if (lCurve.CurveSetIndex >= MainCurveDir.BlkCount)
    MakeNewSet = TRUE;
  else if (strncmp(MainCurveDir.Entries[lCurve.CurveSetIndex].name,
    TempEntryName, 4) == 0)
    MakeNewSet = TRUE;

  if (MakeNewSet)
    lCurve.CurveSetIndex = MakeNewCurveSet();

  if (lCurve.CurveSetIndex == -1)
    {
    SetErrorFlag();
    return;
    }

  LpEntry = &(MainCurveDir.Entries[lCurve.CurveSetIndex]);
  /* set up rep counters */
  lCurveCount=1;

  switch (lCurve.ReferenceType)
    {
    case CLASS_CURVESET:
      lCurveCount = LpEntry->count;
      lCurve.CurveIndex = LpEntry->StartIndex;
      TempFile = TRUE;             /* may have made a temp storage file */
    break;

    case CLASS_CURVE:
      lCurveCount = 1;
      TempFile = TRUE;             /* may have made a temp storage file */
    break;

    case CLASS_CURVESETHDR:
      if (Scalar)
        if (rType != TYPE_STRING)
          PushToDataStack(&dTemp, TYPE_REAL, FALSE);
        else
          PushToDataStack(strptr, TYPE_STRING, TRUE);
      else
        PushToDataStack(&rCurve, TYPE_CURVE, FALSE);
      AssignToCurveSetHdr(lCurve);
      return;

    case CLASS_CURVEHDR:
      if (Scalar)
        PushToDataStack(&dTemp, TYPE_REAL, FALSE);
      else
        PushToDataStack(&rCurve, TYPE_CURVE, FALSE);
      AssignToCurveHdr(lCurve);
    return;

    case CLASS_POINT:
      if (Scalar)
        PushToDataStack(&dTemp, TYPE_REAL, FALSE);
      else
        PushToDataStack(&rCurve, TYPE_CURVE, FALSE);
      AssignToPoint(lCurve);
      return;
    }

  /* lCurve.ReferenceType == CLASS_CURVE or CLASS_CURVESET */
  /* check for a possible multiple value assignment */

  if (rType == TYPE_CURVE)
    {
    RpEntry = &(MainCurveDir.Entries[rCurve.CurveSetIndex]);

    if (lCurve.ReferenceType == CLASS_CURVESET)
      {
      DelCount = 0;
      NewCount = 0;

      /* check to see if doing a complete replacement */
      if (rCurve.ReferenceType == CLASS_CURVESET)
        {
        SHORT Dummy;

        if (CheckCurveBlkOverLap(LpEntry->path, LpEntry->name,
            LpEntry->StartIndex,
            LpEntry->StartIndex + RpEntry->count - 1, &Dummy) ==
          SPLITRANGE)
          {
          error(ERROR_NOT_UNIQUE_CURVE);
          SetErrorFlag();
          return;
          }
        DelCount = LpEntry->count;
        NewCount = RpEntry->count;

        rCurveCount = RpEntry->count;
        SrcCurveIndex = RpEntry->StartIndex;
        }
      else if (rCurve.ReferenceType == CLASS_CURVE)
        {
        DelCount = LpEntry->count;
        if (DelCount == 0)
          NewCount = 1;
        else
          NewCount = DelCount;

        rCurveCount = 1;
        SrcCurveIndex = rCurve.CurveIndex;
        }
      else  // one of the header items.  Get a scalar value later.
        PushToDataStack(&rCurve, TYPE_CURVE, FALSE);

      DstCurveIndex = LpEntry->StartIndex;
      }  /* if lCurve was a CURVESET */
    else  /* lCurve must be a CURVE */
      {
      if (rCurve.ReferenceType == CLASS_CURVESET)
        {
        error(ERROR_BAD_ASSN_RTYPE);
        SetErrorFlag(); /* if rCurve was a set, can't assign */
        return;
        }
      else if (rCurve.ReferenceType == CLASS_CURVE)
        {
        SrcCurveIndex = rCurve.CurveIndex;
        DstCurveIndex = lCurve.CurveIndex;

        // assign curve to curve, only works for equal lengths
        if(ReplaceCurve(& MainCurveDir, rCurve.CurveSetIndex,
                        SrcCurveIndex,  lCurve.CurveSetIndex,
                        DstCurveIndex))
          {
          DelCount = 0;
          NewCount = 0;
          }
        else
          {
          DelCount = 1;
          NewCount = 1;
          rCurveCount = 1;
          }
        }
      else  // one of the header items.  Get a scalar value later.
        PushToDataStack(&rCurve, TYPE_CURVE, FALSE);
      }

    /* copy the new curve multiple times */
    /* or the curve set once */
    i = 0;
    while (i < NewCount)
      {
      if (LpEntry->EntryType != OMA4MEMDATA)
        {
        if(InsertMultiTempCurve(& MainCurveDir, rCurve.CurveSetIndex,
          SrcCurveIndex, lCurve.CurveSetIndex,
          DstCurveIndex, rCurveCount))
          {
          SetErrorFlag();
          return;
          }
        }
      else
        {
        unsigned long k =  LpEntry->TmpOffset;
        GetTempCurveOffset(DstCurveIndex - RpEntry->StartIndex, &k);

        if (TempFileWrite(hTempFile,
                          "TempFile",
                          k,
                          &MainCurveDir,
                          rCurve.CurveSetIndex,
                          SrcCurveIndex,
                          DstCurveIndex,
                          rCurveCount,
                          OMA4MEMDATA))
          {
          SetErrorFlag();
          return;
          }
        DelCount -= rCurveCount;

        }
      i += rCurveCount;
      DstCurveIndex += rCurveCount;
      }

    /************************************************************/
    if (DelCount)
      {
      /* delete the old curves */
      if(DelMultiTempCurve(& MainCurveDir, lCurve.CurveSetIndex,
        LpEntry->StartIndex + NewCount, DelCount))
        {
        SetErrorFlag();
        return;
        }
      // temporary files are deleted by DeleteTempMathFiles() which
      // is called by AssignTo() after this function returns.
      }
    /************************************************************************/
    }
  else  /* right side was not a TYPE_CURVE, do scalar assign */
    {
    for (i=LpEntry->StartIndex; i < LpEntry->StartIndex + lCurveCount; i++)
      {
      /* create destination for scalar assign if it does not exist */
      /* so cs[0].[4] := 2.1 works even if cs[0] has only 4 curves */
      /* but cs[0].[5] := 2.1 would not work since new curve is not */
      /* contiguous with existing ones (error caught by InsertTempCurve) */

      if (LpEntry->count <= (i - LpEntry->StartIndex))
        {
        SHORT points;

        get_Points(&points);
        if(create_new_curve(FALSE, 1, points, lCurve.CurveSetIndex,i, 0.0))
          {
          SetErrorFlag();
          return;
          }
        }

      if(ReadTempCurvehdr(& MainCurveDir, lCurve.CurveSetIndex, i, &Curvehdr))
        {
        SetErrorFlag();
        return;
        }

      if (MakeNewSet)
        Curvehdr.DataType = FLOATTYPE; /* may work since float&lints */
                                       /* are both 4 bytes long */

      if(WriteTempCurvehdr(& MainCurveDir, lCurve.CurveSetIndex, i,
        & Curvehdr))
        {
        SetErrorFlag();
        return;
        }

      for (j=0; j<Curvehdr.pointnum; j++)
        {
        FLOAT X;
        FLOAT Y;

        /* get the X values for the data point */
        if(GetDataPoint(& MainCurveDir, lCurve.CurveSetIndex, i, j,
          & X, &Y, (CHAR) TYPE_REAL, &prefBuf))
          {
          SetErrorFlag();
          return;
          }

        /* replace Y value */
        if(SetDataPoint(&MainCurveDir, lCurve.CurveSetIndex, i,
          j, &X, &dTemp, (CHAR) TYPE_REAL, &prefBuf))
          {
          SetErrorFlag();
          return;
          }
        } /* for j, point loop */
      }  /* for i, Curve loop */
    } /* type was curvset */
}

