/* -----------------------------------------------------------------------
/
/  points.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/points.c_v   1.4   13 Jan 1992 15:35:02   cole  $
/  $Log:   J:/logfiles/oma4000/main/points.c_v  $
/
*/

#include <math.h>
#include <limits.h>
#include <string.h>

#include "points.h"
#include "tempdata.h"
#include "di_util.h"
#include "calib.h"
#include "curvbufr.h"
#include "omameth.h"   // InitialMethod
#include "crventry.h"
#include "curvedir.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

GET_POINT_FUNC * get_point = GetDataPoint;

GET_POINT_FUNC * WhichGetFunction(void)
{
  return get_point;
}

BOOLEAN SetGetFunction(GET_POINT_FUNC * GetFunc)
{
  if (GetFunc)
    get_point = GetFunc;
  else
    get_point = GetDataPoint;

  return (FALSE);
}

/***************************************************************************/
/*                                                                         */
/*  Function: Get an X and Y data point pair.  May be read from any buffer */
/*                                                                         */
/*  Variables: CurveDir - Input. Curve directory for curve entry           */
/*             EntryIndex - Input. Entry for curve block                   */
/*             CurveIndex - Input. File curve number to change.            */
/*             DataPoint - Input. data point number 0....                  */
/*             pX - Output. location to put X value into                   */
/*             pY - Output. location to put Y value into                   */
/*             DataType - see OMA35.h, FLOATTYPE, SHORTTYPE, etc.          */
/*             PrefBuf - Input.  Preferred buffer for loading the curve    */
/*                       portion into.  May not actually be used.          */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_READ - temp file seek error                           */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: CvBuf contents may be changed.                           */
/*                                                                         */
/***************************************************************************/

/* (Implementation in assembler file vdata.asm) */

/* ERR_OMA GetDataPoint(CURVEDIR *CurveDir, SHORT EntryIndex,      */
/*                             USHORT CurveIndex, USHORT DataPoint,        */
/*                             PVOID X, PVOID Y, CHAR DataType,            */
/*                             SHORT *PrefBuf)                              */
/*                                                                         */
  
/***************************************************************************/
/*                                                                         */
/*  Function: Set an X and Y data point pair.  May be read from any buffer */
/*                                                                         */
/*  Variables: CurveDir - Input. Curve directory for curve entry           */
/*             EntryIndex - Input. Entry for curve block                   */
/*             CurveIndex - Input. File curve number to change.            */
/*             DataPoint - Input. data point number 0....                  */
/*             X - Input. X value, do nothing if NULL                      */
/*             Y - Input. Y value, do nothing if NULL                      */
/*             DataType - see OMA35.h, FLOATTYPE, SHORTTYPE, etc.          */
/*             PrefBuf - Input.  Preferred buffer for loading the curve    */
/*                       portion into.  May not actually be used.          */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_READ - temp file read error                              */
/*             ERROR_WRITE - temp file write error                             */
/*             ERROR_SEEK - temp file seek error                              */
/*             ERROR_NONE - OK                                                  */
/*                                                                         */
/*  Side effects: CvBuf contents may be changed.                           */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA SetDataPoint(CURVEDIR *CurveDir, SHORT EntryIndex,
                            USHORT CurveIndex, USHORT DataPoint,
                            PFLOAT X, PVOID Y, CHAR DataType,
                            SHORT *PrefBuf)
{
  USHORT i;
  ERR_OMA err;
  USHORT FirstPoint;
  BOOLEAN Found = FALSE;
  PVOID TmpPtr;
  USHORT DataSz;

  
  /* see if header is already in a buffer */
  for (i=0; (i<BUFNUM) && (! Found); i++)
    {
    if (CvBuf[i].ActiveDir)
      DataSz = CvBuf[i].Curvehdr.DataType & 0x0F;

    if ((CvBuf[i].ActiveDir == CurveDir) &&          /* same dir */
      (CvBuf[i].Entry == EntryIndex))                /* same entry */
      {
      /* same curve or cvhdr points to mem curve set */
      if ((CvBuf[i].CurveIndex == CurveIndex) ||     
          ((CurveDir->Entries[EntryIndex].EntryType == OMA4MEMDATA) &&
           (CvBuf[i].Curvehdr.CurveCount + CvBuf[i].CurveIndex >= CurveIndex)))
        {
        if ((DataPoint >= CvBuf[i].BufferOffset) &&
          (DataPoint < (CvBuf[i].BufferOffset + ((BUFLEN/2)/DataSz))))

          {  /* data point is already in buffer */
          Found = TRUE;
          *PrefBuf = i;
          FirstPoint = CvBuf[i].BufferOffset;
          }
        }
      }
    }

  if (! Found)
    {
    /* not already in buffer so read it in */
    FirstPoint = DataPoint;
    if (err = LoadCurveBuffer(CurveDir, EntryIndex, CurveIndex, &FirstPoint,
                              PrefBuf))
      return err;
    DataSz = CvBuf[*PrefBuf].Curvehdr.DataType & 0x0F;

    /************************&&&&&&&&&&&&&&&&&&&&&&&&&&&&******************/
    //      print_tempbuf(&(CvBuf[PrefBuf]));
    /************************&&&&&&&&&&&&&&&&&&&&&&&&&&&&******************/
    }

  if(Y)
    {
    TmpPtr = (PVOID)((ULONG) CvBuf[*PrefBuf].BufPtr
      + ((ULONG) (DataPoint - FirstPoint) * (ULONG) DataSz));

    if (DataType == (CHAR) CvBuf[*PrefBuf].Curvehdr.DataType)
      memmove(TmpPtr, Y, DataSz);
    else
      ConvertTypes(Y, DataType, TmpPtr, CvBuf[*PrefBuf].Curvehdr.DataType);

    CvBuf[*PrefBuf].status = CVBUF_DIRTY;
    }

  // X is always FLOAT, never have to convert it.
  if(X)
    {
    TmpPtr = (PVOID)((ULONG) CvBuf[*PrefBuf].BufPtr + (BUFLEN / 2) +
      ((ULONG) (DataPoint - FirstPoint) * (ULONG) sizeof(FLOAT)));
    memmove(TmpPtr, X, sizeof(FLOAT));

    // X data NOT stored on oma4000 board.
    if(CurveDir->Entries[EntryIndex].EntryType != OMA4MEMDATA)
      CvBuf[*PrefBuf].status = CVBUF_DIRTY;
    }
  return ERROR_NONE;
}

/***************************************************************************/
/*                                                                         */
/*  function:  Put a new point in newX, newY corresponding to PointIndex,  */
/*             but modify newX for calibration and unit conversion.        */
/*             This is a helper function for GetTempCurvePointIndex()      */
/*             which follows.                                              */
/*                                                                         */
/*  variables: pCurveDir - Input. Curve directory for curve entry          */
/*             EntryIndex - Input. Entry for curve block                   */
/*             CurveIndex - Input. File curve number to change.            */
/*             newX - Output. Closest X value in XUnits.                   */
/*             YVal - Output. Y value at the closest X value.              */
/*             XUnits - expected X units                                   */
/*             PointIndex - Input. index to look at.                       */
/*             PrefBuf - Input.  Preferred buffer for loading the curve    */
/*                       portion into.  May not actually be used.          */
/*             pCurveHdr - Input. curve header of pCurveDir                */
/*                                                                         */
/*  returns: error value                                                   */
/*                                                                         */
/***************************************************************************/

PRIVATE ERR_OMA GetCalConvertPoint(CURVEDIR *pCurveDir,
                                          USHORT EntryIndex,
                                          USHORT CurveIndex,
                                          USHORT PointIndex,
                                          CURVEHDR *pCurvehdr,
                                          PFLOAT newX, PFLOAT newY,
                                          UCHAR XUnits,
                                          SHORT *PrefBuf)
{
  DOUBLE xTemp;
  ERR_OMA err;

  err = GetDataPoint(pCurveDir, EntryIndex, CurveIndex, PointIndex,
    newX, newY, FLOATTYPE, PrefBuf);
  if(err) return err;
  if(XUnits == pCurvehdr -> XData.XUnits) return err;

  if(pCurvehdr -> XData.XUnits == COUNTS)
    {
    * newX = (FLOAT) PointIndex;
    if(InitialMethod->CalibUnits[0] != COUNTS)
      {
      * newX = ApplyCalibrationToX(InitialMethod->CalibCoeff[0], * newX);
      err = ConvertUnits(XUnits, &xTemp, InitialMethod->CalibUnits[0],
                   (DOUBLE)*newX, (DOUBLE)InitialMethod->Excitation);
      if (err)
        return err;

      *newX = (FLOAT) xTemp;
      }
    }
  else
    {
    err = ConvertUnits(XUnits, &xTemp, pCurvehdr->XData.XUnits,
                 (DOUBLE)*newX, (DOUBLE)InitialMethod->Excitation);
    if (err)
      return err;

    * newX = (FLOAT) xTemp;
    }
  return err;
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Find the point index corresponding to a specific X value.   */
/*             and return the X and Y values at that point.                */
/*                                                                         */
/*  variables: pCurveDir - Input. Curve directory for curve entry          */
/*             EntryIndex - Input. Entry for curve block                   */
/*             CurveIndex - Input. File curve number to change.            */
/*             XVal - Input. X value to look for in XUnits.                */
/*                    Output. Closest X value in XUnits.                   */
/*             YVal - Output. Y value at the closest X value.              */
/*             XUnits - expected X units                                   */
/*             PointIndex - Input. Approximate index to look at.           */
/*                          Output. Index of closest X value               */
/*             PrefBuf - Input.  Preferred buffer for loading the curve    */
/*                       portion into.  May not actually be used.          */
/*                                                                         */
/*  returns: error value                                                   */
/*                                                                         */
/***************************************************************************/

ERR_OMA GetTempCurvePointIndex(CURVEDIR *pCurveDir,
                                      USHORT EntryIndex,
                                      USHORT CurveIndex,
                                      PFLOAT XVal, PFLOAT YVal,
                                      UCHAR XUnits,
                                      PUSHORT PointIndex,
                                      SHORT PrefBuf)
{
  FLOAT XTarget = * XVal;
  FLOAT X1, X2, Y1, Y2;
  CURVEHDR Curvehdr;
  ERR_OMA err;
  USHORT index1 = * PointIndex;
  USHORT index2 = index1 + 1;
  USHORT indexMax;
  USHORT incrementFlag = 0;
  USHORT decrementFlag = 0;

  if(err = ReadTempCurvehdr(pCurveDir, EntryIndex, CurveIndex, &Curvehdr))
    return err;

  indexMax = Curvehdr.pointnum - 1;

  if(XUnits == COUNTS)
    {
    // Round the X value
    // if XVal is not positive it will be set to 0 later

    // guard against a floating point error
    if (*XVal > (FLOAT) USHRT_MAX)
      *PointIndex = USHRT_MAX;
    else if (*XVal < (FLOAT) 0)
      *PointIndex = 0;
    else
      *PointIndex = (USHORT) (*XVal + 0.5F);

    if (*PointIndex > Curvehdr.pointnum)
      *PointIndex = Curvehdr.pointnum - 1;
    else if (*PointIndex > 0x8000)
      *PointIndex = 0;

    // get the Y value
    err = GetDataPoint(pCurveDir, EntryIndex, CurveIndex, *PointIndex,
                       XVal, YVal, FLOATTYPE, &PrefBuf);
    // reset the X value to the point index
    *XVal = (FLOAT) *PointIndex;
    return err;    // return the value of X and Y at that data point
    }

  if(index2 > indexMax)
    {
    index2 = indexMax;
    index1 = index2 - 1;
    }
  err = GetCalConvertPoint(pCurveDir, EntryIndex, CurveIndex, index1,
                           &Curvehdr, &X1, &Y1, XUnits, &PrefBuf);
  if(err) return err;

  err = GetCalConvertPoint(pCurveDir, EntryIndex, CurveIndex,
                           index2, &Curvehdr, &X2, &Y2, XUnits, &PrefBuf);
  if(err) return err;

  // increment or decrement but don't reverse direction, no infinite loops
  // in case calibration is not monotonic.
  for(;;)
    {
    if (((X1 >= XTarget) && (X2 <= XTarget)) ||
       ((X1 <= XTarget) && (X2 >= XTarget)))
      break;

    if(((X1 < XTarget) && (X1 < X2)) || ((X1 > XTarget) && (X1 > X2)))
      {
      if(index2 >= indexMax) break;
      if(decrementFlag) break;
      incrementFlag = 1;
      index1 = index2 ++;
      X1 = X2;
      Y1 = Y2;
      err = GetCalConvertPoint(pCurveDir, EntryIndex, CurveIndex,
                               index2, &Curvehdr, &X2, &Y2,
                               XUnits, &PrefBuf);
      if(err) break;
      }
    else
      {
      if(index1 <= 0) break;
      if(incrementFlag) break;
      decrementFlag = 1;
      index2 = index1 --;
      X2 = X1;
      Y2 = Y1;
      err = GetCalConvertPoint(pCurveDir, EntryIndex, CurveIndex,
                               index1, &Curvehdr, &X1, &Y1,
                               XUnits, &PrefBuf);
      if(err) break;
      }
    }

  /* return the closest point */
  if (fabs(XTarget - X1) > fabs(XTarget - X2))
    {
    * XVal = X2;
    * YVal = Y2;
    * PointIndex = index2;
    }
  else
    {
    * XVal = X1;
    * YVal = Y1;
    * PointIndex = index1;
    }
  return err;
}
