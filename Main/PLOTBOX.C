/* -----------------------------------------------------------------------
/
/  plotbox.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/plotbox.c_v   1.3   06 Jul 1992 10:35:38   maynard  $
/  $Log:   J:/logfiles/oma4000/main/plotbox.c_v  $
/
*/

#include <string.h>
#include <math.h>     // sqrt()
#include <limits.h>

#include "plotbox.h"
#include "omameth.h"  // LPMETHDR
#include "cursor.h"   // ActivePlotIndex
#include "pltsetup.h" // autoscale_x
#include "multi.h"    // WindowPlotAssignment
#include "device.h"   // deviceHandle()
#include "handy.h"    // PI
#include "forms.h"    // Colors
// use the following only for GRAPHICAL scan setup
//#include "scanset.h"  // inScanSetup()

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

PLOTBOX Plots[MAXPLOTS];
/* plot areas, up to 9, # 9 is reserved */
/* for split screen displays:
  X axis calibration
  Y axis calibration
  Baseline subtract
  Scan setup
  Math Menu
  Stats Menu
  Spectrograph setup
*/

PLOTBOX * ActivePlot = Plots;

CRECT DisplayGraphArea;  /* screen graphing window */

CCOLOR LineColors[] = { BRT_WHITE, 
                        BLUE,      
                        BRT_YELLOW,
                        RED,       
                        WHITE,     
                        BRT_BLUE,  
                        BRT_ORANGE,
                        BRT_RED }; 

/* -----------------------------------------------------------------------
/
/  int percent(full, percentage)
/
/  function:   used to find a percentage of an int value.
/  requires:   (int) full - the amount the percentage is taken from
/              (int) percentage - the percent value; usually 0 - 100
/  returns:    (int) 'percentage' % of 'full'
/  side effects:
/
/ ----------------------------------------------------------------------- */
int percent(int full, int percentage)
{
   return (int) ((long) full * (long) percentage / 100L);
}

// Set up axis values based on axis->min and axis->max. End points of the
// axis will be exactly what the user entered (or what autoscale produces).
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void scale_axis(AXISDATA * axis)
{
   double axisRange = fabs(axis->max_value - axis->min_value);
   
   if(axisRange < 1e-12)            /* want to know if max & min equal, */
      axisRange = 1.0;              /* but if axisRange < range of float */
                                     /* disaster occurs unless '<' used */

   axis->inv_range = (float) (1.0 / axisRange);
   
   if(axis->max_value < axis->min_value)
      axis->ascending = FALSE;
   else
      axis->ascending = TRUE;
}

// return the margin size and the axis angle for the given plotbox and axis.
// margin is in VDC for label side margin.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void axisMarginAngle(PLOTBOX const * plot, char which_one, int * margin,
                     double * angle)
{
   switch(which_one)
   {
      case 'X' :
         * angle  = 0.0;
         * margin = plot->plotarea.ll.y - plot->fullarea.ll.y; // bottom
         break;

      case 'Y' :
         if(plot->z_position != LEFTSIDE)
         {
            * angle  = PI * 0.5;
            * margin = plot->plotarea.ll.x - plot->fullarea.ll.x; // left
         }
         else
         {  * angle  = PI * 1.5;
            * margin = plot->fullarea.ur.x - plot->plotarea.ur.x; // right
         }
         break;

      case 'Z' :
         if((plot->xz_percent == 0) || (plot->style == FALSE_COLOR))
            * angle = PI * 0.5;
         else if(plot->yz_percent == 0)
            * angle = 0.0;
         else
          {
          double temp1 = (double)plot->yz_percent *
                        (double)(plot->fullarea.ur.y - plot->fullarea.ll.y);

          double temp2 = (double) plot->xz_percent *
                        (double)(plot->fullarea.ur.x - plot->fullarea.ll.x);

          if (!temp2) temp2 = 0.5;

          * angle = atan((temp1) / (temp2));

          }
         if(plot->z_position == RIGHTSIDE)
            * margin = plot->fullarea.ur.x - plot->plotarea.ur.x; // right
         else
         {
            * margin = plot->plotarea.ll.x - plot->fullarea.ll.x; // left
            * angle = - * angle;
         }

         if(* angle < PI * 0.25)
            * margin = plot->plotarea.ll.y - plot->fullarea.ll.y; // bottom

         break;
   }
}

// use InitialMethd for method header info
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void InitializePlots(CRECT * GraphArea)
{
   SHORT i;
   LPMETHDR pMethdr = InitialMethod;

   /* position lower left of plot box */
   DisplayGraphArea.ll = GraphArea->ll;  
   /* set upper right of plot box */
   /* (this sizes the screen graphing window) */
   DisplayGraphArea.ur = GraphArea->ur;  

   window_style = pMethdr->PlotWindowIndex;
   ActiveWindow = pMethdr->ActivePlotSetup;
   ActivePlot = &Plots[ActiveWindow];

   for (i = 0; i < MAXPLOTS-1; i++) /* doesn't do CAL_PLOTBOX */
   {
      WindowPlotAssignment[i] = (UCHAR) pMethdr->WindowPlotSetups[i];
      strncpy(Plots[i].title, pMethdr->PlotInfo[i].Title, TITLE_SIZE);
      strncpy(Plots[i].x.legend, pMethdr->PlotInfo[i].XLegend, LEGEND_SIZE);
      strncpy(Plots[i].y.legend, pMethdr->PlotInfo[i].YLegend, LEGEND_SIZE);
      strncpy(Plots[i].z.legend, pMethdr->PlotInfo[i].ZLegend, LEGEND_SIZE);
      memcpy(&Plots[i].flags, &pMethdr->PlotInfo[i].flags, sizeof(PPLOT_FLAGS));
      Plots[i].x.original_min_value = pMethdr->PlotInfo[i].XMin;
      Plots[i].x.original_max_value = pMethdr->PlotInfo[i].XMax;
      Plots[i].x.min_value = pMethdr->PlotInfo[i].XMin;
      Plots[i].x.max_value = pMethdr->PlotInfo[i].XMax;
      Plots[i].y.original_min_value = pMethdr->PlotInfo[i].YMin;
      Plots[i].y.original_max_value = pMethdr->PlotInfo[i].YMax;
      Plots[i].y.min_value = pMethdr->PlotInfo[i].YMin;
      Plots[i].y.max_value = pMethdr->PlotInfo[i].YMax;
      Plots[i].z.original_min_value = pMethdr->PlotInfo[i].ZMin;
      Plots[i].z.original_max_value = pMethdr->PlotInfo[i].ZMax;
      Plots[i].z.min_value = pMethdr->PlotInfo[i].ZMin;
      Plots[i].z.max_value = pMethdr->PlotInfo[i].ZMax;

      Plots[i].x.ascending = pMethdr->PlotInfo[i].XAscending;
      Plots[i].y.ascending = pMethdr->PlotInfo[i].YAscending;
      Plots[i].z.ascending = pMethdr->PlotInfo[i].ZAscending;

      Plots[i].x.units = pMethdr->PlotInfo[i].XUnits;
      Plots[i].y.units = pMethdr->PlotInfo[i].YUnits;
      Plots[i].z.units = pMethdr->PlotInfo[i].ZUnits;

      Plots[i].xz_percent = pMethdr->PlotInfo[i].XZPercent;
      Plots[i].yz_percent = pMethdr->PlotInfo[i].YZPercent;
      Plots[i].z_position = pMethdr->PlotInfo[i].ZPosition;

      Plots[i].style = pMethdr->PlotInfo[i].Style;
      Plots[i].plot_peak_labels.label_peaks =
        pMethdr->PlotInfo[i].PlotPeakLabels;
   }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CopyPlotToMethod(void)
{
   USHORT   i;
   LPMETHDR pMethdr = InitialMethod;

   pMethdr->ActivePlotSetup = ActiveWindow;

   for (i=0; i<8; i++)
   {
      pMethdr->WindowPlotSetups[i] = WindowPlotAssignment[i];
      strncpy(pMethdr->PlotInfo[i].Title, Plots[i].title, TITLE_SIZE);
      strncpy(pMethdr->PlotInfo[i].XLegend, Plots[i].x.legend, LEGEND_SIZE);
      strncpy(pMethdr->PlotInfo[i].YLegend, Plots[i].y.legend, LEGEND_SIZE);
      strncpy(pMethdr->PlotInfo[i].ZLegend, Plots[i].z.legend, LEGEND_SIZE);

      memcpy(&pMethdr->PlotInfo[i].flags, &Plots[i].flags, sizeof(PPLOT_FLAGS));
      pMethdr->PlotInfo[i].XMin = Plots[i].x.min_value;
      pMethdr->PlotInfo[i].XMax = Plots[i].x.max_value;
      pMethdr->PlotInfo[i].YMin = Plots[i].y.min_value;
      pMethdr->PlotInfo[i].YMax = Plots[i].y.max_value;
      pMethdr->PlotInfo[i].ZMin = Plots[i].z.min_value;
      pMethdr->PlotInfo[i].ZMax = Plots[i].z.max_value;

      pMethdr->PlotInfo[i].XAscending = Plots[i].x.ascending;
      pMethdr->PlotInfo[i].YAscending = Plots[i].y.ascending;
      pMethdr->PlotInfo[i].ZAscending = Plots[i].z.ascending;

      pMethdr->PlotInfo[i].XUnits = Plots[i].x.units;
      pMethdr->PlotInfo[i].YUnits = Plots[i].y.units;
      pMethdr->PlotInfo[i].ZUnits = Plots[i].z.units;

      pMethdr->PlotInfo[i].XZPercent = Plots[i].xz_percent;
      pMethdr->PlotInfo[i].YZPercent = Plots[i].yz_percent;
      pMethdr->PlotInfo[i].ZPosition = Plots[i].z_position;

      pMethdr->PlotInfo[i].Style = Plots[i].style;
      pMethdr->PlotInfo[i].PlotPeakLabels =
         Plots[i].plot_peak_labels.label_peaks;
   }

   pMethdr->AutoScaleX = autoscale_x;
   pMethdr->AutoScaleY = autoscale_y;
   pMethdr->AutoScaleZ = autoscale_z;

   pMethdr->PlotWindowIndex = window_style;
   return;
}

/* -----------------------------------------------------------------------
/
/  int GssPosX(PLOTBOX *plot, FLOAT xvalue, int XOffset)
/
/  function:   translates the input X value into an X  coordinate
/              position on the output device, in VDC units.  The position
/              is relative to the minimum position for the axis, and
/              scaled to the length of the axis.  Note that the
/              .magnitude value used to scale the axis to a limited
/              range is also applied here to the input data (see
/              scale_axis() above).
/  requires:   (PLOTBOX *) plot - a pointer to the plotbox structure
/              (FLOAT) xvalue - the raw, unscaled input value of x-coord.
/              (int) XOffset - this curves X offset from the left side of
/                              the plotbox
/  returns:    (int) - the output X position in VDC units
/
/ ----------------------------------------------------------------------- */

//int GssPosX(PLOTBOX *plot, FLOAT xvalue, int XOffset)
//{
//   FLOAT XFraction;
//   long XPos;
//   FLOAT XLen;
//
//   xvalue *= plot->x.magnitude_factor;
//   if (plot->x.ascending)
//      XFraction = (xvalue - plot->x.min_value) * plot->x.inv_range;
//   else
//      XFraction = (plot->x.min_value - xvalue) * plot->x.inv_range;
//
//   XLen = XFraction * (FLOAT) plot->x.axis_end_offset.x;
//
//   /* check for gross wraparound */
//   if (XLen > (FLOAT) INT_MAX)
//      return (int) INT_MAX;
//   if (XLen < (FLOAT) INT_MIN)
//      return (int) INT_MIN;
//
//   XPos = (long) XLen + (long) plot->x.axis_zero.x + (long) XOffset;
//
//   if (plot->z_position == LEFTSIDE)
//      XPos += LINE_WIDTH;
//
//   /* check for wraparound */
//   if (XPos > INT_MAX)
//      return INT_MAX;
//   if (XPos < INT_MIN)
//      return INT_MIN;
//
//   return (int) XPos;
//
//}

/* -----------------------------------------------------------------------
/
/  int GssPosY(PLOTBOX *plot, FLOAT yvalue, int YOffset)
/
/  function:   translates the input Y value into a y coordinate
/              position on the output device, in VDC units.  The position
/              is relative to the minimum position for the axis, and
/              scaled to the length of the axis.  Note that the
/              .magnitude value used to scale the axis to a limited
/              range is also applied here to the input data (see
/              scale_axis() above).
/  requires:   (PLOTBOX *) plot - a pointer to the plotbox structure
/              (FLOAT) yvalue - the raw, unscaled input value of x-coord.
/              (int) YOffset - this curves Y offset from the bottom of
/                              the plotbox
/  returns:    (int) - the output Y position in VDC units
/
/ ----------------------------------------------------------------------- */

//int GssPosY(PLOTBOX *plot, FLOAT yvalue, int YOffset)
//{
//   FLOAT YFraction, YLen;
//   long TempY;
//
//   yvalue *= plot->y.magnitude_factor;
//
//   if (plot->y.ascending)
//      YFraction = (yvalue - plot->y.min_value) * plot->y.inv_range;
//   else
//      YFraction = (plot->y.min_value - yvalue) * plot->y.inv_range;
//
//
//   YLen = YFraction * (FLOAT) plot->y.axis_end_offset.y;
//   /* check for gross wraparound */
//   if (YLen > (FLOAT) INT_MAX)
//      return (int) INT_MAX;
//   if (YLen < (FLOAT) INT_MIN)
//      return (int) INT_MIN;
//
//   TempY = (long) YLen + (long) plot->y.axis_zero.y + (long) YOffset;
//
//   TempY += LINE_WIDTH;
//   /* check for wraparound */
//   if (TempY > INT_MAX)
//      return INT_MAX;
//   if (TempY < INT_MIN)
//      return INT_MIN;
//
//   return (int) TempY;
//}

/* -----------------------------------------------------------------------
/
/  CXY gss_position(PLOTBOX *plot, float xvalue,float yvalue, float zvalue)
/
/  function:   translates the input values into an x and y coordinate
/              position on the output device, in VDC units.  The position
/              is relative to the minimum position for the axis, and
/              scaled to the length of the axis.  Note that the
/              .magnitude value used to scale the axis to a limited
/              range is also applied here to the input data (see
/              scale_axis() above).
/  requires:   (PLOTBOX *) plot - a pointer to the plotbox structure
/              (float) xvalue - the raw, unscaled input value of x-coord.
/              (float) yvalue - the raw, unscaled input value of x-coord.
/              (float) zvalue - the raw, unscaled input value of x-coord.
/                               (usually the plot's line number (overlay))
/  returns:    (CXY) pos  - the output position in VDC units
/                     pos.x - coordinate x
/                     pos.y - coordinate y
/
/ ----------------------------------------------------------------------- */
CXY gss_position(PLOTBOX *plot, float xvalue, float yvalue, float zvalue)
{
   LONG XOffset = 0L, YOffset = 0L;
   CXY pos;

   CalcOffsetForZ(plot, zvalue, &XOffset, &YOffset);

   pos.x = GssPosX(plot, xvalue, (int) XOffset);
   pos.y = GssPosY(plot, yvalue, (int) YOffset);

   return pos;
}

void CalcOffsetForZ(PLOTBOX *plot, FLOAT ZValue, PLONG pXOffset,
                    PLONG pYOffset)
{
  register float temp1, temp2;
  long ltemp1, ltemp2;

   /* calculate the relative magnitude of the ZValue along the Z axis in */
   /* measurement units */
  if (plot->z.min_value == ZValue)
    {
    ltemp1 = ltemp2 = 0L;
    }
  else
    {
    if (plot->z.ascending)
      temp1 =  ZValue - plot->z.min_value;
    else
      temp1 = plot->z.min_value - ZValue;

    /* convert Z distance to virtual plotting units to get X */
    /* and Y offset */
  
    temp1 *= plot->z.inv_range;
    temp2 = temp1;
    temp2 *= plot->z.axis_end_offset.y;
    temp1 *= plot->z.axis_end_offset.x;
 
    ltemp1 = (long)temp1;
    ltemp2 = (long)temp2;

    if (ltemp1 < INT_MIN) ltemp1 = INT_MIN;
    if (ltemp1 > INT_MAX) ltemp1 = INT_MAX;

    if (ltemp2 < INT_MIN) ltemp2 = INT_MIN;
    if (ltemp2 > INT_MAX) ltemp2 = INT_MAX;
    }  
  *pXOffset = ltemp1;
  *pYOffset = ltemp2;
}

/* -----------------------------------------------------------------------
/
/  BOOLEAN CalcClipRect(Plot, ZValue, ClipRect)
/
/  function:   Calculates a cliprectangle for the given plotbox and Z value
/
/  requires:   (PLOTBOX *) Plot - the current plotbox structure.
/              (FLOAT) ZValue - the z coordinate (graph layer number)
/              (CRECT *) ClipRect - pointer to clip rectangle structure
/                                      to be filled
/
/  returns:    FALSE if OK
/              TRUE if Clip rectangle has no volume
/
/ ----------------------------------------------------------------------- */

BOOLEAN CalcClipRect(PLOTBOX * Plot, float ZValue, CRECT *ClipRect)
{
   LONG XOffset;
   LONG YOffset;

   ClipRect->ll = Plot->x.axis_zero;
   ClipRect->ur.x = Plot->x.axis_end_offset.x + Plot->x.axis_zero.x;
   ClipRect->ur.y = Plot->y.axis_end_offset.y + Plot->y.axis_zero.y;

   if(Plot->style != FALSE_COLOR)
   {
      if((ZValue < Plot->z.min_value) && (ZValue < Plot->z.max_value))
         return TRUE;
      if((ZValue > Plot->z.min_value) && (ZValue > Plot->z.max_value))
         return TRUE;
   }
   else
   {
      if((ZValue < Plot->y.min_value) && (ZValue < Plot->y.max_value))
         return TRUE;
      if((ZValue > Plot->y.min_value) && (ZValue > Plot->y.max_value))
         return TRUE;
   }

   CalcOffsetForZ(Plot, ZValue, & XOffset, & YOffset);
      
   ClipRect->ll.x += XOffset;
   ClipRect->ur.x += XOffset;
   ClipRect->ll.y += YOffset;
   ClipRect->ur.y += YOffset;

   return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void initAxisToOriginal(AXISDATA * axis)
{
   axis->min_value = axis->original_min_value;
   axis->max_value = axis->original_max_value;
}

// values for maxMins arg of gssPlotPosition
enum { MINMINMIN, MINMINMAX, MINMAXMIN, MINMAXMAX, MAXMINMIN, MAXMINMAX,
       MAXMAXMIN, MAXMAXMAX };

// bit mask for extracting x,y, or z axis MIN or MAX from above enum
// bit = 0 iff MIN, bit = 1 iff MAX
enum { XMAXMIN = 4, YMAXMIN = 2, ZMAXMIN = 1 };

// Determine gss position with correction for min and max axis values
// being identical. Only for x,y,z at axis end points.
// This is a helper function for drawPlotboxOutline().
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE CXY gssPlotPosition(PLOTBOX * plotbox, int maxMins)
{
   float xVal = plotbox->x.min_value;
   float yVal = plotbox->y.min_value;
   float zVal = plotbox->z.min_value;
   CXY position;

   if(maxMins & XMAXMIN)
      xVal = plotbox->x.max_value;

   if(maxMins & YMAXMIN)
      yVal = plotbox->y.max_value;

   if(maxMins & ZMAXMIN)
      zVal = plotbox->z.max_value;

   if(plotbox->z_position == NOSIDE)
      zVal = (float) 0.0;
   
   position = gss_position(plotbox, xVal, yVal, zVal);

   if(plotbox->x.min_value == plotbox->x.max_value)
      if(maxMins & XMAXMIN)
         position.x += plotbox->x.axis_end_offset.x;

   if(plotbox->y.min_value == plotbox->y.max_value)
      if(maxMins & YMAXMIN)
         position.y += plotbox->y.axis_end_offset.y;

   if(plotbox->z.min_value == plotbox->z.max_value)
      if(maxMins & ZMAXMIN)
      {
         position.x += plotbox->z.axis_end_offset.x;
         position.y += plotbox->z.axis_end_offset.y;
      }

   return position;
}

// Return an array of points and a point count corresponding to the
// plot box outline (includes the axis lines). A maximum of 7 points
// will be provided, the first point and last point are identical.
// REQUIRES that outline[] be big enough to hold seven points.
// Each point returned in outline[] will be modified by offset, where
// offset > 0 moves the point toward the center of the plotbox and
// offset < 0 moves the point away from the center of the plotbox.
// offset is in DEVICE COORDINATES !!!  +1 means move one device pixel
// towards the center.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void plotboxOutline(PLOTBOX * plot, CXY outline[], short * pointCount,
                     int offset)
{
   switch(plot->z_position)
   {
      case NOSIDE :
         outline[0] = gssPlotPosition(plot, MINMAXMIN);
         outline[1] = gssPlotPosition(plot, MAXMAXMIN);
         outline[2] = gssPlotPosition(plot, MAXMINMIN);
         outline[3] = gssPlotPosition(plot, MINMINMIN);
         * pointCount = 5;
         if(offset)
         {
            outline[0] = movePointByDCOffset(outline[0], +offset, -offset);
            outline[1] = movePointByDCOffset(outline[1], -offset, -offset);
            outline[2] = movePointByDCOffset(outline[2], -offset, +offset);
            outline[3] = movePointByDCOffset(outline[3], +offset, +offset);
         }
         outline[4] = outline[0];
         break;

      case RIGHTSIDE :
         outline[0] = gssPlotPosition(plot, MINMAXMIN);
         outline[1] = gssPlotPosition(plot, MINMAXMAX);
         outline[2] = gssPlotPosition(plot, MAXMAXMAX);
         outline[3] = gssPlotPosition(plot, MAXMINMAX);
         outline[4] = gssPlotPosition(plot, MAXMINMIN);
         outline[5] = gssPlotPosition(plot, MINMINMIN);
         * pointCount = 7;
         if(offset)
         {
            outline[0] = movePointByDCOffset(outline[0], +offset, 0      );
            outline[1] = movePointByDCOffset(outline[1], 0,       -offset);
            outline[2] = movePointByDCOffset(outline[2], -offset, -offset);
            outline[3] = movePointByDCOffset(outline[3], -offset, 0      );
            outline[4] = movePointByDCOffset(outline[4], 0,       +offset);
            outline[5] = movePointByDCOffset(outline[5], +offset, +offset);
         }
         outline[6] = outline[0];
         break;

      case LEFTSIDE :
         outline[0] = gssPlotPosition(plot, MAXMAXMIN);
         outline[1] = gssPlotPosition(plot, MAXMAXMAX);
         outline[2] = gssPlotPosition(plot, MINMAXMAX);
         outline[3] = gssPlotPosition(plot, MINMINMAX);
         outline[4] = gssPlotPosition(plot, MINMINMIN);
         outline[5] = gssPlotPosition(plot, MAXMINMIN);
         * pointCount = 7;
         if(offset)
         {
            outline[0] = movePointByDCOffset(outline[0], -offset, 0      );
            outline[1] = movePointByDCOffset(outline[1], 0,       -offset);
            outline[2] = movePointByDCOffset(outline[2], +offset, -offset);
            outline[3] = movePointByDCOffset(outline[3], +offset, 0      );
            outline[4] = movePointByDCOffset(outline[4], 0,       +offset);
            outline[5] = movePointByDCOffset(outline[5], -offset, +offset);
         }
         outline[6] = outline[0];
         break;
   }
}

// draw the outline of the plot box, including the x,y, and z axis lines.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void drawPlotboxOutline(PLOTBOX * plot)
{
   CXY drawset[7];
   SHORT pointCount;
   CCOLOR SelColor;
   CLINEREPR OldRepr;
   CDVHANDLE ActiveHandle = deviceHandle();

   plotboxOutline(plot, drawset, &pointCount, 0);
   CInqLineRepr(ActiveHandle, &OldRepr);
   CSetLineColor(ActiveHandle, plot->box_color, &SelColor);
   CPolyline(ActiveHandle, pointCount, drawset);
   CSetLineColor(ActiveHandle, OldRepr.Color, &SelColor);
}

/* -----------------------------------------------------------------------
/
/  void set_plotbox_size(plot)
/
/  function:   Set size of plotbox structure (15% of fullarea)
/  requires:   (PLOTBOX *) plot - the structure describing
/              the current plotbox.
/  returns:    (void)
/  side effects: Alters contents of plotbox variables .plotarea[].
/
/ ----------------------------------------------------------------------- */
void set_plotbox_size(PLOTBOX * plot)
{
   int fullwidth  = plot->fullarea.ur.x - plot->fullarea.ll.x;
   int fullheight = plot->fullarea.ur.y - plot->fullarea.ll.y;

   /* lower left is plot 0,0;  upper right is upper right of fullarea */
   if (plot->z_position != LEFTSIDE)
   {
      plot->plotarea.ll.x = (plot->fullarea.ll.x + percent(fullwidth, 8));
      plot->plotarea.ur.x = plot->fullarea.ur.x - percent(fullwidth, 5);
   }
   else
   {
      plot->plotarea.ll.x = plot->fullarea.ll.x + percent(fullwidth, 5);
      plot->plotarea.ur.x = plot->fullarea.ur.x - percent(fullwidth, 8);
   }
   // no right margin if false color plot, but move left 2 pixels to stay
   // out of the color key when plotting.
   if(plot->style == FALSE_COLOR)
      plot->plotarea.ur.x += adjustXbyDCOffset(0, -2);

   // use the following only for GRAPHICAL scan setup
//   // if in scan setup, ensure enough room for the group size tick mark
//   // labels
//   if(* inScanSetup)
//      plot->plotarea.ur.x += adjustXbyDCOffset(0, -8);

   plot->plotarea.ll.y = plot->fullarea.ll.y + percent(fullheight, 10);
   plot->plotarea.ur.y = plot->fullarea.ur.y - percent(fullheight, 8);

   /* find 0,0 location for plots */
   plot->x.axis_zero.x = plot->plotarea.ll.x;
   plot->y.axis_zero.y = plot->plotarea.ll.y;

   plot->x.axis_end_offset.x = plot->plotarea.ur.x - plot->x.axis_zero.x;
   plot->y.axis_end_offset.y = plot->plotarea.ur.y - plot->y.axis_zero.y;
}

/* -----------------------------------------------------------------------
/  void SizeAxis(PLOTBOX *plot, char which_one, AXISDATA * *axis,
/                 int *axis_length)
/
/   function:   Sets the drawing sizes for the axis lines
/   requires:
/               (PLOTBOX *) plot - the structure describing
/                                  the current plotting region, or "plotbox".
/               char which_one - which axis is being plotted, 'X', 'Y', 'Z'
/
/               (AXISDATA * *)axis - returns pointer to proper axis
/
/               (int *) axis_length - length of axis vector in VDC units
/
/   returns:      (void)
/ ----------------------------------------------------------------------- */
void SizeAxis(PLOTBOX *plot, char which_one, AXISDATA * *axis,
               int *axis_length)
{
   int xplot_size,yplot_size,zy_size,zx_size;
   int xaxis_size, yaxis_size;

   /* calculate best angle and char height based on Height space */
   /* will need to check length space later */
   switch(which_one)
   { /* beginning of switch */
      case 'X':
         *axis=&(plot->x);
         break;

      case 'Y':
         *axis=&(plot->y);
         break;

      case 'Z':
         *axis=&(plot->z);
         break;
   }

   xplot_size = plot->plotarea.ur.x - plot->plotarea.ll.x;
   yplot_size = plot->plotarea.ur.y - plot->plotarea.ll.y;

   if (plot->z_position != NOSIDE)
   {
      if(plot->style == FALSE_COLOR) {
         zy_size = 0;
         zx_size = 0;
      }
      else {
         zy_size = (int) (((LONG) yplot_size * (LONG) plot->yz_percent) /
                   100L);
         zx_size = (int) (((LONG) xplot_size * (LONG) plot->xz_percent) /
                   100L);
      }
      xaxis_size = xplot_size - zx_size;
      yaxis_size = yplot_size - zy_size;
   }
   else
   {
      xaxis_size = xplot_size;
      yaxis_size = yplot_size;
   }

   /* plot the line for the axis */
   switch(which_one)
   {
      case 'X':
         if (plot->z_position != LEFTSIDE)
         {     /* keep it to the left */
            (* axis)->axis_zero = plot->plotarea.ll;
            (*axis)->axis_end_offset.x = xaxis_size + plot->plotarea.ll.x;
            (*axis)->axis_end_offset.y = plot->plotarea.ll.y;
         }
         else
         {     /* keep it to the right */
            (*axis)->axis_zero.x = plot->plotarea.ur.x - xaxis_size;
            (*axis)->axis_zero.y = plot->plotarea.ll.y;
            (*axis)->axis_end_offset.x = plot->plotarea.ur.x;
            (*axis)->axis_end_offset.y = plot->plotarea.ll.y;
         }
         *axis_length = xaxis_size;
         break;

      case 'Y':
         /* check to see which side to put Y axis on */
         (*axis)->axis_zero.y = plot->plotarea.ll.y;
         (*axis)->axis_end_offset.y = yaxis_size + plot->plotarea.ll.y;
         if (plot->z_position != LEFTSIDE)
         {  /* put it on the left hand side */
            (*axis)->axis_zero.x = plot->plotarea.ll.x;
            (*axis)->axis_end_offset.x = plot->plotarea.ll.x;
         }
         else
         {  /* put it on the right hand side */
            (*axis)->axis_zero.x = plot->plotarea.ur.x;
            (*axis)->axis_end_offset.x = plot->plotarea.ur.x;
         }
         *axis_length = yaxis_size;
         break;
      case 'Z':
         (*axis)->axis_zero.y = plot->plotarea.ll.y;
         if (plot->z_position == RIGHTSIDE)
         {     /* put z axis on the right hand side */
            (*axis)->axis_zero.x = xaxis_size + plot->plotarea.ll.x;
            /* end of Z axis at plotarea.ur.x for any angle <= PI/2 */
            (*axis)->axis_end_offset.x = (*axis)->axis_zero.x + zx_size;
            (*axis)->axis_end_offset.y = (*axis)->axis_zero.y + zy_size;
         }
         else
         {        /* put it on the left hand side */
            (*axis)->axis_zero.x = plot->plotarea.ur.x - xaxis_size;
            /* end of Z axis at plotarea.ll.x for any angle > PI/2 and < PI*/
            (*axis)->axis_end_offset.x = (*axis)->axis_zero.x - zx_size;
            (*axis)->axis_end_offset.y = (*axis)->axis_zero.y + zy_size;
         }
         *axis_length = (int) sqrt(((double)zx_size * (double)zx_size) +
         ((double)zy_size * (double)zy_size));
         break;
   }

   (*axis)->axis_end_offset.x -= (*axis)->axis_zero.x;
   (*axis)->axis_end_offset.y -= (*axis)->axis_zero.y;
}

BOOLEAN LiveDrawFast(PLOTBOX * plot)
{
  return(plot->flags.live_fast == 0);
}

  
