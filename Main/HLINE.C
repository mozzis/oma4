/* -----------------------------------------------------------------------
/
/  hline.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/*
/  $Header:   J:/logfiles/oma4000/main/hline.c_v   1.9   13 Jan 1992 13:41:36   cole  $
/  $Log:   J:/logfiles/oma4000/main/hline.c_v  $
*/

#include <math.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "hline.h"
#include "doplot.h"
#include "tempdata.h"
#include "points.h"
#include "di_util.h"
#include "calib.h"
#include "omameth.h"    // InitialMethod
#include "syserror.h"   // ERROR_ALLOC_MEM
#include "curvdraw.h"
#include "device.h"
#include "handy.h"
#include "omaerror.h"
#include "plotbox.h"
#include "crvheadr.h"

// value that is sure to be <= the screen's physical minimum
#define SCR_MIN -0x7FFF

// value that is sure to be >= the screen's physical maximum
#define SCR_MAX 0x7FFF

// -----  start of horizons functions -----------------------
// Two sets of "horizons" are maintained for doing hidden line plots.
// The min and max horizon arrays are allocated on the heap.  Each
// stores an array of SHORT x, y data.

typedef enum { MINHOR, MAXHOR } Horizon;

// min and max horizon arrays are allocated on the heap.
// 4 pointers to array of SHORT
static PSHORT horizon[2][2] = { 0, 0, 0, 0 };
// [0][0] -- min horizon x values -- MINHOR
// [0][1] -- min horizon y values
// [1][0] -- max horizon x values -- MAXHOR
// [1][1] -- max horizon y values

static void getHorizonPoint(Horizon maxmin, USHORT j, PSHORT xVal,
                             PSHORT yVal)
{
   * xVal = * (horizon[ maxmin ][ 0 ] + j);
   * yVal = * (horizon[ maxmin ][ 1 ] + j);
}

static void putHorizonPoint(USHORT maxmin, USHORT j, SHORT xVal,
                             SHORT yVal)
{
   * (horizon[ maxmin ][ 0 ] + j) = xVal;
   * (horizon[ maxmin ][ 1 ] + j) = yVal;
}

static ERR_OMA horizon_init(USHORT numPoints)
{
   USHORT i;

   // allocate all 4 arrays in one block
   horizon[ 0 ][ 0 ] = malloc(numPoints * sizeof(SHORT) * 4);
   if(! horizon[ 0 ][ 0 ]) {
      return error(ERROR_ALLOC_MEM);
   }

   horizon[ 0 ][ 1 ] = horizon[ 0 ][ 0 ] + numPoints;
   horizon[ 1 ][ 0 ] = horizon[ 0 ][ 1 ] + numPoints;
   horizon[ 1 ][ 1 ] = horizon[ 1 ][ 0 ] + numPoints;

   for (i = 0; i < numPoints; i++)
   {
      putHorizonPoint(MINHOR, i, SCR_MAX, SCR_MAX);
      putHorizonPoint(MAXHOR, i, SCR_MIN, SCR_MIN);
   }
   return ERROR_NONE;
}

static void end_horizon()
{
   free(horizon[ 0 ][ 0 ]);
}

// ------- end of horizons functions --------------------------------------

// -------- local functions ----------------------------------------------
/* ---------------------------------------------------------------------
/  function:   Tests a line to see if it is above the horizon. The original
/              Horizon is pointed to by the X portion of the data in
/              HorEntryIndex curves MAXHOR and MINHOR.  The new horizon is
/              placed in the Y data portion of the curve.
/
/  requires:   (CXY *) ending_phy - returns the location where line
/                           is first under horizon.
/              (CXY *) starting_phy - returns the location where line
/                           comes out above the horizon.
/              (int) x1 - first x coordinate. VDC units.
/              (int) y1 - first y coordinate. VDC units.
/              (int) x1 - second x coordinate. VDC units.
/              (int) y2 - second y coordinate. VDC units.
/              Crossover - Output. TRUE if the line crossed the horizon or
/                          stayed inside it.
/              LastInitialX - last point for which the horizon was not set.
/                             allow this function to reset the new horizon
/                             at this point and not use it as a guideline
/              XMax - input. Maximum X value.
/              p_ClipMin - physical clip rectangle x limits
/              p_ClipMax
/
/              Upper - TRUE if working on upper horizon.
/
/  returns:
/           ERROR_READ - temp file seek error
/           ERROR_WRITE - temp file write error
/           ERROR_SEEK - temp file seek error
/           ERROR_NONE - OK
/
/  NOTE:  If the test_hline procedure returns Crossover = FALSE, the curve
/         stayed outside the horizon and starting_phy and ending_phy are
/         undefined. If Crossover is returned FALSE it is because the line
/         at some point falls inside the horizon.  ending_phy is set to
/         this point. starting_phy shows where the line comes back into sight
/         after disappearing behind the horizon. If the line disappears and
/         does not come out from beneath the horizon starting_phy.x=-1
/         and starting_phy.y = -1.
/ ----------------------------------------------------------------------- */

static void test_hline(CXY * ending_phy, CXY * starting_phy, SHORT x1,
                       SHORT y1, SHORT x2, SHORT y2,
                       BOOLEAN * Crossover, SHORT * LastInitialX,
                       SHORT XMax, SHORT p_ClipMin, SHORT p_ClipMax,
                       BOOLEAN Upper)
{
  SHORT xinc, yinc, dx, dy, dsum,steps,
        OldPt, NewPt, HorCurveIndex,
        *IndepChange, *DepChange, *IndepPos, *DepPos, *IndepInc, *DepInc;
  CXY   last_good;

  if (Upper)
    HorCurveIndex = MAXHOR;
  else
    HorCurveIndex = MINHOR;

  /* assume starting inside horizon until shown otherwise */
  starting_phy->x = -1;
  starting_phy->y = -1;

  /* make second, third and fourth quadrant vectors act like first */
  if (x1<x2) xinc = 1; else xinc = -1;
  if (y1<y2) yinc = 1; else yinc = -1;

  /* check for over limits */
  if (((x1 <= p_ClipMin) && (x2 <= p_ClipMin)) ||
    ((x1 >= p_ClipMax) && (x2 >= p_ClipMax)))
    {
    ending_phy->x = x1;
    ending_phy->y = y1;
    return;
    }

  /* change in X  and Y for this vector */
  dx = abs (x1-x2);
  dy = abs (y1-y2);

  getHorizonPoint(HorCurveIndex, x1, & OldPt, & NewPt);

  /* check for unset horizons, leading edge of plot will be plotted */
  /* as the top surface. */
  if (! Upper && ((OldPt == SCR_MAX) ||
    ((*LastInitialX == x1) && (y1 < OldPt))))
    {
    OldPt = y1;
    putHorizonPoint(MINHOR, x1, OldPt, y1);
    *LastInitialX = x1;
    }

  /* if outside the horizon - good */
  if (((y1 > OldPt) && Upper) || ((y1 < OldPt) && ! Upper))
    {
    last_good.x = x1;    // Track last good pt.  necessary because we
    last_good.y = y1;    // want to return the last good pt. and not the
    *Crossover = FALSE;  // first bad point.
    }
  else
    {  /* if inside the horizon */
    ending_phy->x = x1;
    ending_phy->y = y1;
    *Crossover = TRUE;
    }

  /* For an explanation of this algorithm (Octantal Digital Differential */
  /* Analyzer) see "The C User's Journal", 8/89, p.82 */

  /* if the X vector component changes faster than the Y draw the line by */
  /* incrementing the X position (X independent, Y dependent), otherwise */
  /* draw by incrementing the Y position (Y independent, X dependent)  */

  if (dx > dy)
    {
    IndepChange = &dx;
    DepChange = &dy;
    IndepPos = &x1;
    DepPos = &y1;
    IndepInc = &xinc;
    DepInc = &yinc;
    }
  else
    {
    IndepChange = &dy;
    DepChange = &dx;
    IndepPos = &y1;
    DepPos = &x1;
    IndepInc = &yinc;
    DepInc = &xinc;
    }

  dsum = *IndepChange >> 1;

  for (steps=1; steps <= *IndepChange; steps++)
    {
    /* when dsum (running error) gets big enough, the dependent X or Y */
    /* value is incremented, and the running error is restarted */
    dsum += *DepChange;
    if (dsum >= *IndepChange)
      {
      dsum -= *IndepChange;
      *DepPos += *DepInc;
      }

    /* go to the next independent pixel value */
    *IndepPos += *IndepInc;

    if (x1 < 0)
      {
      y1 = (SHORT)(((FLOAT)(y2-y1) / (FLOAT)(x2-x1)) * (FLOAT)(-x1)) + y1;
      x1 = 0;
      }
    else if (x1 > XMax)
      {
      y1 = (SHORT)(((FLOAT)(y2-y1) / (FLOAT)(x2-x1)) *
                   (FLOAT)(XMax - x1)) + y1;
      x1 = XMax;
      }

    if (x2 < 0)
      {
      y2 = (SHORT)(((FLOAT)(y2-y1) / (FLOAT)(x2-x1)) * (FLOAT)(-x2)) + y2;
      x2 = 0;
      }
    else if (x2 > XMax)
      {
      y2 = (SHORT)(((FLOAT)(y2-y1) / (FLOAT)(x2-x1)) *
        (FLOAT)(XMax - x2)) + y2;
      x2 = XMax;
      }

    /* get horizon value at new X value */
    getHorizonPoint(HorCurveIndex, x1, & OldPt, &NewPt);

    /* check for unset horizons, leading edge of plot will be plotted */
    /* as the top surface. */
    if (! Upper && ((OldPt == SCR_MAX) ||
      ((*LastInitialX == x1) && (y1 < OldPt))))
      {
      OldPt = y1;
      putHorizonPoint(MINHOR, x1, OldPt, y1);
      *LastInitialX = x1;
      }

    /* if outside the horizon - good */
    if (((y1 > OldPt) && Upper) || ((y1 < OldPt) && ! Upper))
      {
      last_good.x = x1;  /* this point is now the last good point */
      last_good.y = y1;

      /* if outside this curve's horizon reset the new horizon */
      if (((y1 > NewPt) && Upper) ||  ((y1 < NewPt) && ! Upper))
        putHorizonPoint(HorCurveIndex, x1, OldPt, y1);

      /* if the previous point was inside the horizon, return this */
      /* point as the start, finish will be less */
      if (*Crossover)
        {
        starting_phy->x = x1;
        starting_phy->y = y1;
        return;
        }
      }
    else    /* else inside or equal to horizon */
      {
      /* check to see if this is the first to go inside the horizon */
      if (! *Crossover)
        {
        ending_phy->x = last_good.x;
        ending_phy->y = last_good.y;
        *Crossover = TRUE;
        }
      }
    }  /* for steps */
}
/* ---------------------------------------------------------------------
/  function:   Mimics the GSS function v_pline by calling a line plotting
/              routine which uses the Bresenham algorithm.
/  requires:
/              (int) index - number of points to plot.
/              (CXY *) plotlines - pointer to xy pairs.
/              p_min - lower left corner of plotting area in pixels
/              p_max - upper right corner of plotting area in pixels
/              v_max - upper right corner of plotting area in GSS virtual
/                      units
/              LastInitialX - Input and Output. Last point which had no
/                             horizon set for it.  Signals incomplete horizon
/                             decision for this X column.
/              p_ClipMin - physical clip rectangle x limits
/              p_ClipMax
/
/              OverCheck - True if plotting upper surface, false for lower
/                          surface
/ ----------------------------------------------------------------------- */
static void hidden_v_pline(int index, CXY *plotlines,
                           CXY *p_min, CXY *p_max,
                           CXY *v_max, SHORT *LastInitialX,
                           SHORT p_ClipMin, SHORT p_ClipMax,
                           BOOLEAN OverCheck)
{
  SHORT count, beginning_index, PixelNum, x1, y1, x2, y2, i;
  FLOAT XVirToPhys, XPhysToVir,
        YVirToPhys, YPhysToVir,  fTemp;
  BOOLEAN Crossover;
  CXY    phy_cord[2], ending_cord, starting_cord, TempPt1, TempPt2;

  PixelNum = p_max->x - p_min->x + 1;
  XVirToPhys = (FLOAT) p_max->x / (FLOAT) v_max->x;
  YVirToPhys = (FLOAT) p_max->y / (FLOAT) v_max->y;
  XPhysToVir = (FLOAT) 1.0 / XVirToPhys;
  YPhysToVir = (FLOAT) 1.0 / YVirToPhys;

  count = 0;
  beginning_index = 0;       /* initialize variables */
  TempPt1 = plotlines[beginning_index];

  phy_cord[1].x = (int) (((float) plotlines[0].x * XVirToPhys) + 0.5);
  phy_cord[1].y = (int) (((float) plotlines[0].y * YVirToPhys) + 0.5);
  for (i=0; i<(index-1); i++)
    {
    /* last line is now new line beginning */
    phy_cord[0].x = phy_cord[1].x;
    phy_cord[0].y = phy_cord[1].y;

    if (*LastInitialX == -1)
      *LastInitialX = phy_cord[0].x;

    /* convert from virtual coordinates to physical */
    phy_cord[1].x = (int)(((float) plotlines[i+1].x * XVirToPhys) + 0.5);
    phy_cord[1].y = (int)(((float) plotlines[i+1].y * YVirToPhys) + 0.5);

    //      fprintf(stdprn, "i %u put at %d,%d\n", i,
    //               plotlines[i].x, plotlines[i].y);

    // adjust for boundary crossing
    x1 = phy_cord[0].x - p_min->x;
    x2 = phy_cord[1].x - p_min->x;
    y1 = phy_cord[0].y;
    y2 = phy_cord[1].y;

    if ((x1 < 0) && (x2 > 0))
      {
      y1 += (SHORT) (((FLOAT) (y2-y1) / (FLOAT) (x2-x1)) * (FLOAT) (-x1));
      x1 = 0;
      }
    else if ((x1 > PixelNum-1) && (x2 < PixelNum-1))
      {
      y1 += (SHORT) (((FLOAT) (y2-y1) / (FLOAT) (x2-x1)) *
        (FLOAT) (PixelNum - 1 - x1));
      x1 = PixelNum-1;
      }

    if ((x2 < 0) && (x1 > 0))
      {
      y2 += (SHORT) (((FLOAT) (y2-y1) / (FLOAT) (x2-x1)) * (FLOAT) (-x2));
      x2 = 0;
      }
    else if ((x2 > PixelNum-1) && (x1 < PixelNum-1))
      {
      y2 += (SHORT) (((FLOAT) (y2-y1) / (FLOAT) (x2-x1)) *
        (FLOAT) (PixelNum - 1 - x2));
      x2 = PixelNum-1;
      }

    /* send the X values offset from the window min X to save */
    /* storage space */
    test_hline(& ending_cord, & starting_cord, x1, y1, x2, y2,
               & Crossover, LastInitialX, PixelNum - 1, p_ClipMin,
               p_ClipMax, OverCheck);

    /* get the real physical X values for starting and ending space */
    ending_cord.x += p_min->x;
    if (starting_cord.x != -1)
      starting_cord.x += p_min->x;

    if (! Crossover)
      {
      if (count == 0)
        {
        plotlines[beginning_index] = TempPt1;
        beginning_index = i;
        TempPt1 = plotlines[beginning_index];
        count++;
        }
      count++;
      }
    else            /* some points (or all) of line fall inside horizon */
      {
      TempPt2 = plotlines[i+1];
      if ((ending_cord.x != phy_cord[0].x) ||
        (ending_cord.y != phy_cord[0].y))
        {              /* beginning is not inside horizon */
        if (count == 0)
          {
          beginning_index = i;
          count = 1;
          }
        fTemp = ((float) ending_cord.x * XPhysToVir) + 0.5;
        if (fTemp > (FLOAT) INT_MAX)
          fTemp = (FLOAT) INT_MAX;
        if (fTemp < (FLOAT) INT_MIN)
          fTemp = (FLOAT) INT_MIN;
        plotlines[i+1].x = (int)fTemp;

        fTemp = ((float) ending_cord.y * YPhysToVir) + 0.5;
        if (fTemp > (FLOAT) INT_MAX)
          fTemp = (FLOAT) INT_MAX;
        if (fTemp < (FLOAT) INT_MIN)
          fTemp = (FLOAT) INT_MIN;
        plotlines[i+1].y = (int) fTemp;

        count++;
        }
      if (count > 1)
        CPolyline(deviceHandle(), count, &(plotlines[beginning_index]));

      plotlines[i+1] = TempPt2;

      /* if line didn't end inside horizon */
      if (starting_cord.x != -1)
        {
        plotlines[beginning_index] = TempPt1;
        beginning_index = i;
        count = 1;
        phy_cord[1] = starting_cord;
        TempPt1 = plotlines[beginning_index];

        fTemp = ((float) starting_cord.x * XPhysToVir) + 0.5;
        if (fTemp > (FLOAT) INT_MAX)
          fTemp = (FLOAT) INT_MAX;
        if (fTemp < (FLOAT) INT_MIN)
          fTemp = (FLOAT) INT_MIN;
        plotlines[i].x = (int)fTemp;

        fTemp = ((float) starting_cord.y * YPhysToVir) + 0.5;
        if (fTemp > (FLOAT) INT_MAX)
          fTemp = (FLOAT) INT_MAX;
        if (fTemp < (FLOAT) INT_MIN)
          fTemp = (FLOAT) INT_MIN;
        plotlines[i].y = (int) fTemp;

        i--;
        }
      else
        {  /* line went inside horizon */
        count = 0;
        }
      }  /* else was crossover */
    }  /* for end */

  if (count > 1)
    CPolyline(deviceHandle(), count, &(plotlines[beginning_index]));

  plotlines[beginning_index] = TempPt1;

  return;
}
// --------------- end of local functions -------------------------------
/* -----------------------------------------------------------------------
/       NOTE: maximumx is the number of maximum data points
/             along the x-axis.(datamaxx-dataminx)
/
/  (i.e. if data on x-axis ranges from 430 to 590 then
/   maximumx sould have a value of 590-430+1=161.)
/
/-----------------------------------------------------------------------*/
/* -----------------------------------------------------------------------
/  ERR_OMA hplot_init (SHORT MaximumX);
/
/  function: initialize min and max horizons.
/            The X variable contains the temporary horizon (formerly
/            called hormin and hormax) and the Y variables contain the last
/            horizon ((formerly called minhor and maxhor) for each virtual screen
/            point.
/
/  requires:
/            (SHORT) MaximumX - number of data points to initialize(ALL).
/  returns:
/             ERROR_ALLOC_MEM
/             FALSE - OK
/
/-----------------------------------------------------------------------*/
ERR_OMA hplot_init(SHORT MaximumX)
{
   return horizon_init(MaximumX);
}

// clean up by deallocating the horizon arrays
void hplot_end()
{
 end_horizon();
}

static ERR_OMA HPlotCleanUp(CRECT *OldClipRect,
                                    CXY * plotlines,
                                    ERR_OMA err)
{
  if (plotlines)
    free(plotlines);
  if(OldClipRect)
    CSetClipRectangle(deviceHandle(), *OldClipRect);
  return err;
}

/* -----------------------------------------------------------------------
/ ERR_OMA array_hplot(PLOTBOX*plot,
/                             FLOAT zvalue, CURVEDIR *pCurveDir,
/                             SHORT EntryIndex, USHORT CurveIndex,
/                             CXY *p_min, CXY *p_max,
/                             CXY *v_max)
/
/  function: Plots a single curve at one time.  The curve is
/       supplied as a virtual data curve entry, with a one-to-one
/       correspondence between x data and y data indices yielding
/       the correct xy pair.
/  requires: plot - the current plotbox structure.
/            zvalue - The value for this curve on the Z axis.
/            pCurveDir - The curve directory for the plotted curve
/            EntryIndex - Index for the plotted curves entry block
/            CurveIndex - Index of the curve into the entry block
/            p_min - lower left corner of plotting area in pixels
/            p_max - upper right corner of plotting area in pixels
/            v_max - upper right corner of plotting area in GSS virtual units
/
/  returns:
/           ERROR_READ - temp file seek error
/           ERROR_WRITE - temp file write error
/           ERROR_SEEK - temp file seek error
/           ERROR_NONE - OK
/ ----------------------------------------------------------------------- */
ERR_OMA array_hplot(PLOTBOX*plot, FLOAT zvalue, CURVEDIR *pCurveDir,
                           SHORT Entry, USHORT Curve,
                           CXY *p_min, CXY *p_max, CXY *v_max,
                           CCOLOR underColor)
{
  SHORT  LastInitialX, NewPt, OldPt,
         prefBuf = -1, p_ClipMin, p_ClipMax, j;
  USHORT point_count, i, index;
  LONG   XOffset, YOffset;
  FLOAT  X, Y, XBasePt, XFactor, YBasePt, YFactor,
         XVirToPhys, YVirToPhys;
  DOUBLE temp;
  CURVEHDR Curvehdr, TmpCvHdr;
  ERR_OMA err;
  CXY * plotlines;
  CRECT ClipRect, OldClipRect;
  CLINETYPE SelType;
  CCOLOR SelColor;
  GET_POINT_FUNC * get_point = WhichGetFunction();

  index = 0;

  XVirToPhys = (FLOAT)((double)screen.LastXY.x / (double)screen.LastVDCXY.x);
  YVirToPhys = (FLOAT)((double)screen.LastXY.y / (double)screen.LastVDCXY.y);

  /* get the number of data points */
  if (err = ReadTempCurvehdr(pCurveDir, Entry, Curve, &Curvehdr))
    return err;

  /* allocate a decent sized buffer, retry with smaller buffer if */
  /* not enough room. Try at 1/32nd of data space to start */
  point_count = (SEGSIZE / (ULONG) (sizeof (CXY))) >> 5;
  point_count = min(Curvehdr.pointnum, point_count);
  /* point_count is the number of data points which can fit into */
  /* the local buffer */
  while (((plotlines = malloc(point_count * sizeof(CXY))) == NULL) &&
    (point_count != 0))
    point_count /= 2;

  /* check for total screwup */
  if (point_count == 0)
    return error(ERROR_ALLOC_MEM);

  /* set clip rect */
  if (CalcClipRect(plot, zvalue, &ClipRect))
    {
    return HPlotCleanUp(NULL, plotlines, ERROR_NONE);
    }

  p_ClipMin = (int)((FLOAT) ClipRect.ll.x * XVirToPhys) - p_min->x;
  p_ClipMax = (int)((FLOAT) ClipRect.ur.x * XVirToPhys) - p_min->x;

  CInqClipRectangle(deviceHandle(), &OldClipRect);
  CSetClipRectangle(deviceHandle(), ClipRect);

  /* Need to keep track of which lower horizon index was not initialized. */
  /* When curve segments are connected, the next point, which may have the */
  /* same X value will not assume that the lower horizon value there is */
  /* final.  Otherwise the effect would be seen on steep sections where */
  /* the horizon was not previously initialized */
  /* Need only one LastInitialX value if assume that curve groups are */
  /* all drawn in same direction */
  /* set to -1 to show that there is no present value */
  LastInitialX = -1;

  XBasePt = plot->x.min_value;

  XFactor = plot->x.inv_range * (FLOAT) plot->x.axis_end_offset.x;

  YBasePt = plot->y.min_value;

  YFactor = plot->y.inv_range * (FLOAT) plot->y.axis_end_offset.y;

  CalcOffsetForZ(plot, zvalue, &XOffset, &YOffset);

  XOffset += plot->x.axis_zero.x;
  YOffset += plot->y.axis_zero.y;

  if (plot->x.units != COUNTS)
    {
    /* get the number of data points */
    if (err = ReadTempCurvehdr(pCurveDir, Entry, Curve, &TmpCvHdr))
      return HPlotCleanUp(&OldClipRect, plotlines, err);
    }
  CSetLineType(deviceHandle(), plot->plot_line_type, &SelType);

  for (i = 0; i < Curvehdr.pointnum; i++)
    {
    if (err = (*get_point)(pCurveDir, Entry, Curve, i, &X, &Y, FLOATTYPE, &prefBuf))
      {
      return HPlotCleanUp(&OldClipRect, plotlines, err);
      }
    if (plot->x.units != COUNTS)
      {
      if (TmpCvHdr.XData.XUnits == COUNTS)
        {
        // changed meaning of plotting in pixel units
        X = (FLOAT) i;
        if (InitialMethod->CalibUnits[0] != COUNTS)
          {
          X = ApplyCalibrationToX(InitialMethod->CalibCoeff[ 0 ], X);
          err = ConvertUnits((UCHAR) plot->x.units, &temp,
                        InitialMethod->CalibUnits[0], (double) X,
                        (double) InitialMethod->Excitation);
          if (err)
            return HPlotCleanUp(&OldClipRect, plotlines, err);

          X = (float) temp;                                                    /* JRL */
          }
        }
      else
        {
        err = ConvertUnits((UCHAR) plot->x.units, &temp,
          TmpCvHdr.XData.XUnits, (double)X,
          (double)InitialMethod->Excitation);

        if (err)
          return HPlotCleanUp(&OldClipRect, plotlines, err);

        X = (float) temp;
        }
      }
    else    // changed meaning of plotting in pixel units
      X = (FLOAT) i;

    // change from X and Y experimental values to virtual screen points

    /* Fill up the array of GSS values for X */
    if (plot->x.ascending)
      X = (X - XBasePt) * XFactor;
    else
      X = (XBasePt - X) * XFactor;

    X += (FLOAT) XOffset;

    /* check for gross wraparound */
    if (X > (FLOAT) INT_MAX)
      X = (FLOAT) INT_MAX;
    if (X < (FLOAT) INT_MIN)
      X = (FLOAT) INT_MIN;

    plotlines[index].x = (SHORT) X;

    if (plot->y.ascending)
      Y = (Y - YBasePt) * YFactor;
    else
      Y = (YBasePt - Y) * YFactor;

    Y += (FLOAT) YOffset;
    /* check for gross wraparound */
    if (Y > (FLOAT) INT_MAX)
      Y = (FLOAT) INT_MAX;
    if (Y < (FLOAT) INT_MIN)
      Y = (FLOAT) INT_MIN;

    plotlines[index].y = (SHORT) Y;

    index++;

    /* if buffer is full, plot it */
    if (index == point_count)
      {
      /* do the lines above the horizon */
      if (!plot->flags.loop_colors)
        CSetLineColor(deviceHandle(), plot->plot_color, &SelColor);
      else
        {
        int color_index;
        if (plot->flags.loop_colors == 1)
          color_index = ((int)zvalue+1) % 5 ? 0 : 6;
        else
          color_index = (int)zvalue % 7;
        CSetLineColor(deviceHandle(), LineColors[color_index], &SelColor);
        }
      hidden_v_pline(index, plotlines, p_min, p_max, v_max,
        & LastInitialX, p_ClipMin, p_ClipMax, TRUE);

      if (plot->style == HIDDENSURF_CURVES)
        {
        CSetLineColor(deviceHandle(), underColor, &SelColor);
        /* do the under surface lines */
        hidden_v_pline(index, plotlines, p_min, p_max, v_max,
          & LastInitialX, p_ClipMin, p_ClipMax, FALSE);
        }

      /* set up new buffer using last plotted point as new start */
      index--;
      plotlines[0].x = plotlines[index].x;
      plotlines[0].y = plotlines[index].y;
      index = 1;
      }
    }

  /* plot remaining points in buffer */
  if (index > 1)
    {
    /* do the lines above the horizon */
    CSetLineColor(deviceHandle(), plot->plot_color, &SelColor);
    hidden_v_pline(index, plotlines, p_min, p_max, v_max, & LastInitialX,
      p_ClipMin, p_ClipMax, TRUE);

    /* do the under surface lines */
    CSetLineColor(deviceHandle(), underColor, &SelColor);
    hidden_v_pline(index, plotlines, p_min, p_max, v_max, & LastInitialX,
      p_ClipMin, p_ClipMax, FALSE);
    }

  HPlotCleanUp(&OldClipRect, plotlines, err);

  for (j=0; j <= (p_max->x - p_min->x); j++)   // MLM
    {
    /* reset the upper horizon */
    getHorizonPoint(MAXHOR, j, & OldPt, & NewPt);

    if (OldPt < NewPt)
      /* update the horizon and reset the temp horizon */
      putHorizonPoint(MAXHOR, j, NewPt, SCR_MIN);
    else
      /* update the horizon and reset the temp horizon */
      putHorizonPoint(MAXHOR, j, OldPt, SCR_MIN);

    /* reset the lower horizon */
    getHorizonPoint(MINHOR, j, & OldPt, & NewPt);

    if (OldPt > NewPt)
      /* update the horizon and reset the temp horizon */
      putHorizonPoint(MINHOR, j, NewPt, SCR_MAX);
    else
      /* update the horizon and reset the temp horizon */
      putHorizonPoint(MINHOR, j, OldPt, SCR_MAX);
    }
  return ERROR_NONE;
}
