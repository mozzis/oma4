/* -----------------------------------------------------------------------
/
/  autopeak.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/autopeak.c_v   0.17   30 Mar 1992 10:05:36   maynard  $
/  $Log:   J:/logfiles/oma4000/main/autopeak.c_v  $
*/

// file autopeak.c  RAC  11-MAY-90

#include <string.h>
#include <float.h>

#include "autopeak.h"
#include "points.h"
#include "device.h"
#include "doplot.h"
#include "cursor.h"
#include "macrecor.h"
#include "forms.h"
#include "di_util.h"   // StripExp()
#include "calib.h"     // add_calibration_point()
#include "multi.h"
#include "baslnsub.h"  // baslnsub_active()
#include "omameth.h"   // InitialMethod
#include "omaerror.h"
#include "curvdraw.h"  // plot_curves()
#include "plotbox.h"
#include "crvheadr.h"
#include "curvedir.h"  // MainCurveDir
#include "omaform.h"   // isFormGraphWindow()
#include "symbol.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

static float threshold;  // current threshold value

// Only this module can change auto_peak_mode.  Other modules have access to
// its value via is_auto_peak but cannot change it.
static BOOLEAN auto_peak_mode = FALSE;
BOOLEAN const * const is_auto_peak = & auto_peak_mode;

// index into ...curve_peaks[] of the current entry to use
static int curve_index;

// enable/disable the current curve entry for peak labels.
// set up a new threshold if enabled.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void auto_peak_curve_enable(BOOLEAN on_off)
{
  PLOTBOX * pPlot = & Plots[ActiveWindow];

  if((curve_index < 0) || (curve_index >= MAX_LABELLED_CURVES))
    return;

  pPlot->plot_peak_labels.curve_peaks[curve_index].enabled = on_off;
  if(on_off)
    {
    pPlot->plot_peak_labels.curve_peaks[curve_index].threshold_value =
      threshold;

    if(isKeyStrokeRecordOn())
      {
      CHAR Buf[80];
      sprintf(Buf, "SET_PEAK_THRESHOLD(%d, %f, %f);\n", curve_index,
        pPlot->plot_peak_labels.curve_peaks[curve_index].curve_number,
        threshold);
      MacRecordString(Buf);
      }
    }
}

// Find the index in curve_peak_mode[] which contains the given z_value
// and is enabled.  Return its index or -1 if not found.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int find_curve_index(float z_value, PLOTBOX * the_plotbox)
{
   int i;

   for(i = 0; i < MAX_LABELLED_CURVES; i ++) {
      if(   (the_plotbox->plot_peak_labels.curve_peaks[ i ].enabled)
          && (the_plotbox->plot_peak_labels.curve_peaks[ i ].curve_number
                                                               == z_value))
          return i;
   }
   return -1;
}

// If thresh is not within the bounds of the plot box, fix it.
// Otherwise, leave it alone.  If it needs fixing and middle is TRUE, then
// set threshold to the middle of the plot box.  If it needs fixing and
// middle is FALSE, then set it to either the top or the bottom of the
// plot box.  Return the fixed value.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE float fix_threshold(SHORT the_window, BOOLEAN middle, float thresh)
{
   SHORT PlotIndex       = WindowPlotAssignment[the_window];
   PLOTBOX * the_plotbox = & Plots[PlotIndex];
   float thresh_lowlim   = the_plotbox->y.min_value;
   float thresh_highlim  = the_plotbox->y.max_value;

   if(thresh_highlim < thresh_lowlim)
   {
      float temp = thresh_highlim;

      thresh_highlim = thresh_lowlim;
      thresh_lowlim  = temp;
   }
   
   if(thresh >= thresh_lowlim) {
      if(thresh <= thresh_highlim)
         return thresh;
      else
         if(! middle)
            return thresh_highlim;
   }
   else
      if(! middle)
         return thresh_lowlim;

   // out of bounds and middle
   return (thresh_lowlim + thresh_highlim) * 0.5F;
}

// erase and/or draw a threshold line in a window.  If new_thresh is NULL,
// then just erase the current threshold line if it is visible.  Otherwise,
// erase the old threshold line if it is visible and then draw a new one.
// Return the threshold value corresponding to curve_index.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float auto_peak_draw_threshold(SHORT the_window, float const * new_thresh)
{
   // TRUE iff visible in the window
   static BOOLEAN threshold_visible = FALSE;
   static CXY thresh_line[ 2 ];     // end points of the threshold line

   SHORT     PlotIndex   = WindowPlotAssignment[the_window];
   PLOTBOX * the_plotbox = &Plots[PlotIndex];
   float     z_val       = CursorStatus[PlotIndex].Z;
   CDVHANDLE the_handle  = deviceHandle();
   CRECT     ClipRect;
   CRECT     OldClipRect;
   CCOLOR    SelColor;
   CPIXOPS   SelMode;

   // set the clipping rectangle and save the current one
   if(CalcClipRect(the_plotbox, z_val, &ClipRect))
      return the_plotbox->plot_peak_labels.
                                 curve_peaks[ curve_index ].threshold_value;
   
   CInqClipRectangle(the_handle, & OldClipRect);   
   CSetClipRectangle(the_handle, ClipRect);

   // change writing mode to exclusive OR, ignore error return values.
   CSetWritingMode(the_handle, CdXORs, &SelMode); // exclusive or mode

   // set the drawing color, ignore errors
   CSetLineColor(the_handle, the_plotbox->plot_color, &SelColor);

   if(threshold_visible)                 // erase the old threshold line
    {
      CPolyline(the_handle, 2, thresh_line);
      threshold_visible = FALSE;
    }
   if(new_thresh)                           // draw a new threshold line
    {
      // max or min if out of bounds
      threshold = fix_threshold(the_window, FALSE, * new_thresh);
      the_plotbox->plot_peak_labels.curve_peaks[ curve_index ].
                                                threshold_value = threshold;
      thresh_line[0] = gss_position(the_plotbox, the_plotbox->x.min_value,
                                     threshold, z_val);
      thresh_line[1] = gss_position(the_plotbox, the_plotbox->x.max_value,
                                     threshold, z_val);

      CPolyline(the_handle, 2, thresh_line);

      threshold_visible = TRUE;
   }
   // restore the writing mode, ignore error return value.
   CSetWritingMode(the_handle, CReplace, &SelMode);  // replace mode  

   // restore the clipping rectangle
   CSetClipRectangle(the_handle, OldClipRect);

   return threshold;
}

// Determine the proper x value for the peak label and then draw text of the
// x value above the peak maximum point.  If necessary, move the label text
// down so that it does not get clipped.
// Return the peak point in x_pix, x_cal.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void label_peak(CURVEDIR *pCurveDir,
                        SHORT entry_index,
                        SHORT curve_index,
                        float peak_sum,
                        CXY max_position,
                        USHORT first_cursor,
                        CURVEHDR const * PCurveHdr,
                        PLOTBOX const * pPlot,
                        double * x_pix,
                        double * x_cal)
{
  CHAR  text[12];
  SHORT prefBuf = 0;
  FLOAT cXsz, cYsz, 
        x_peak_cal;
  USHORT cursor = first_cursor;
  ERR_OMA err;

  {
  FLOAT previous_sum = 0.0F,
        current_sum = 0.0F,
        target_sum = peak_sum * 0.5F,
        current_x, current_y, delta;

  for(;;)
    {
    GetDataPoint(pCurveDir, entry_index, curve_index, cursor,
                 &current_x, &current_y, FLOATTYPE, &prefBuf);
    current_sum += current_y;
    if(current_sum >= target_sum)
      break;
    cursor ++, previous_sum = current_sum;
    }
  // ASSERT : previous_sum < target_sum.  current_sum >= target_sum.
  // cursor is at (current_x,current_y) point corresponding to current_sum.

  delta = (target_sum - previous_sum) / current_y;
  x_peak_cal = current_x - 0.5F + delta;
  }

  // adjust x_peak_cal to a calibrated x value
  if(pPlot->x.units != COUNTS)
    {
    double calibrated_x;

    if(PCurveHdr->XData.XUnits == COUNTS)
      {
      x_peak_cal = ApplyCalibrationToX(InitialMethod->CalibCoeff[0],
                                       (FLOAT) cursor);

      err = ConvertUnits((UCHAR) pPlot->x.units,
                   &calibrated_x, InitialMethod->CalibUnits[0],
                   (DOUBLE)x_peak_cal,
                   (DOUBLE)InitialMethod->Excitation);
      }
    else
      {
      err = ConvertUnits((UCHAR)pPlot->x.units,
                   &calibrated_x, PCurveHdr->XData.XUnits,
                   (double) x_peak_cal, (double) InitialMethod->Excitation);
      }
    if (err)
      return;

    x_peak_cal = (float) calibrated_x;
 
    * x_pix = cursor, * x_cal = x_peak_cal;

    if (pPlot->x.max_value >= pPlot->x.min_value)
      {
      if (x_peak_cal > pPlot->x.max_value || x_peak_cal < pPlot->x.min_value)
        return;
      }
    else
      {
      if (x_peak_cal > pPlot->x.min_value || x_peak_cal < pPlot->x.max_value)
        return;
      }
    }
  else
    {
    *x_pix = cursor, *x_cal = x_peak_cal;

    if (pPlot->x.max_value >= pPlot->x.min_value)
      {
      if (*x_pix > pPlot->x.max_value || *x_pix < pPlot->x.min_value)
        return;
      }
    else
      if (*x_pix > pPlot->x.min_value || *x_pix < pPlot->x.max_value)
        return;
    }

  sprintf(text, "%10.6g", x_peak_cal);
  condense_float_string(text);

  /* base character size on X axis margin size, which tracks plot box */
  /* size reasonable well */
  {
  SHORT Margin;
  double Angle;
  
  axisMarginAngle(pPlot, 'X', &Margin, &Angle);
  cXsz = cYsz = Margin * 0.35F;
  cXsz *= pPlot->xscale;
  cYsz *= pPlot->yscale;
  }

  // add a third of a character to center (right-left) above the curve peak
  max_position.x += (cXsz / 3.0F);

  // add half a character to move above the peak just a little
  max_position.y += (cYsz / 2.0F);

  {
  SHORT cliprect_top, extent_top;

  // move down so that the top of the text is below the clip rectangle top
  cliprect_top = pPlot->plotarea.ur.y;
  extent_top = (SHORT)(cYsz * (FLOAT)strlen(text) * 0.5F) + max_position.y;
  if(extent_top > cliprect_top)
    max_position.y -= extent_top - cliprect_top;
  }

  {
  FLOAT angle = (float)(PI * 0.5F);
  SHORT dummy;

  CSetLineColor(deviceHandle(), BRT_YELLOW, &dummy);
  symbol(&max_position, text, angle, cXsz, cYsz);
  }
}

// find all the peaks and label them.  Label is put at the location where
// the max y value was found.  If gathering calibration points, send each
// new peak point to the calib module, ie the set of peaks found will be the
// new set of calibration points.  Only send points for the curve that the
// graphics cursor is on AND when not in auto peak mode.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void find_peaks(CURVEDIR *pDir, USHORT Entry,
                        USHORT Curve, SHORT the_window,
                        PLOTBOX * pPlot, CURVEHDR const * PCurveHdr,
                        FLOAT z_value)
{
  SHORT  PlotIndex = WindowPlotAssignment[the_window],
         prefBuf = 0;

  USHORT cursor, first_cursor;      /* starting point in the peak region */
  USHORT const num_points = PCurveHdr->pointnum;

  FLOAT  current_sum = 0.0F,        // sum of y's in the peak region
         x_val, y_val, max_y;       // maximum y value in the peak region
  // location of max y in the peak region in GSS coordinates
  CXY    xofmaxy;
  DOUBLE x_pix, x_cal;              // actual peak point values
  BOOLEAN last_above = FALSE,       // TRUE iff previous point above threshld
          no_peaks = TRUE;          // no peak sent to calib yet
  ERR_OMA err;

  for(cursor = 0; cursor < num_points; cursor ++)
    {
    GetDataPoint(pDir, Entry, Curve, cursor, &x_val, &y_val, FLOATTYPE, &prefBuf);
    // adjust x_val for calibration
    if(pPlot->x.units != COUNTS)
      {
      double calibrated_x;

      if(PCurveHdr->XData.XUnits == COUNTS)
        {
        x_val = ApplyCalibrationToX(InitialMethod->CalibCoeff[0],
                                   (float)cursor);
        err = ConvertUnits((UCHAR) pPlot->x.units,
                     &calibrated_x, InitialMethod->CalibUnits[0],
                     (DOUBLE)x_val,
                     (DOUBLE)InitialMethod->Excitation);
        }
      else
        {
        err = ConvertUnits((UCHAR) pPlot->x.units,
                     &calibrated_x, PCurveHdr->XData.XUnits,
                     (DOUBLE) x_val,
                     (DOUBLE) InitialMethod->Excitation);
        }
      if (err)
        return;

      x_val = (float)calibrated_x;
      }
    if(last_above)
      {
      // previous point above threshold
      if(y_val > threshold)
        {
        current_sum += y_val;
        if(y_val > max_y)
          {
          max_y = y_val;
          xofmaxy = gss_position(pPlot, x_val, y_val, z_value);
          }
        }
      else
        {
        // end of peak region
        label_peak(pDir, Entry, Curve, current_sum, xofmaxy, first_cursor,
                   PCurveHdr, pPlot, &x_pix, &x_cal);
        if(gathering_calib_points &&
          (CursorStatus[PlotIndex].Z == z_value) &&
           (!auto_peak_mode))
          {
          add_calibration_point(x_pix, x_cal, no_peaks);
          no_peaks = FALSE;
          }
        last_above = FALSE;
        }
      }
    else
      {  // previous point NOT above threshold
      if(y_val > threshold)
        {
        // start of a new peak region
        max_y = y_val;
        xofmaxy = gss_position(pPlot, x_val, y_val, z_value);
        last_above = TRUE;
        current_sum = y_val;
        first_cursor = cursor;
        }
      }
    }
  // if the end point was above threshold, label the last peak
  if(last_above)
    {
    label_peak(pDir, Entry, Curve, current_sum, xofmaxy, first_cursor,
               PCurveHdr, pPlot, & x_pix, & x_cal);
    if(gathering_calib_points && (CursorStatus[PlotIndex].Z == z_value) &&
       (!auto_peak_mode))
      add_calibration_point(x_pix, x_cal, no_peaks);
    }
}

// Find and draw all the peak values, but only if peak labelling is enabled
// for the curve and not if a false color plot.
// If in autopeak mode, only label the curve corresponding to the current
// curve.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void draw_peaks(CURVEDIR *pDir, USHORT Entry, USHORT Curve,
                SHORT the_window, CURVEHDR const * PCurveHdr,
                FLOAT z_value)
{
   SHORT PlotIndex = WindowPlotAssignment[the_window];
   PLOTBOX * pPlot = &Plots[PlotIndex];
   SHORT Zindex    = find_curve_index(z_value, pPlot),
         SelAngle;
   CRECT ClipRect,
         OldClipRect;
   CCOLOR SelColor;

   if(Zindex < 0)
      return;
   if(pPlot->style == FALSE_COLOR)
      return;
   if(auto_peak_mode && (z_value != CursorStatus[PlotIndex].Z))
      return;

   // Set the clipping rectangle and save the current one.
   // Use the entire plotarea for showing peak labels.
   ClipRect.ll = pPlot->plotarea.ll;
   ClipRect.ur = pPlot->plotarea.ur;
   
   CInqClipRectangle(deviceHandle(), &OldClipRect); 
   CSetClipRectangle(deviceHandle(), ClipRect);

   threshold = pPlot->plot_peak_labels.curve_peaks[Zindex].threshold_value;

   // rotate text 90 degrees, ignore errors
   CSetGTextRotation(deviceHandle(), 900, &SelAngle);
   // set the graphics text color to plot_color, ignore errors
   CSetGTextColor(deviceHandle(), pPlot->plot_color, &SelColor);
   CSetBgColor(deviceHandle(), pPlot->background_color, &SelColor);

   // find and label all the peaks
   find_peaks(pDir, Entry, Curve, the_window, pPlot, PCurveHdr, z_value);

   // put the rotation back to zero for normal text, ignore errors
   CSetGTextRotation(deviceHandle(), 0, &SelAngle);
   // restore the clipping rectangle
   CSetClipRectangle(deviceHandle(), OldClipRect);
}

// Find the index in curve_peak_mode[] which contains the given z_value and
// is enabled. If there is no such index, set up an entry in the first empty
// curve_peak_mode[] location a threshold of MAX_FLT.  If all the entries
// are full and there is no match, return -1.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int set_curve_index(float z_value, PLOTBOX * pPlot)
{
  int i = find_curve_index(z_value, pPlot);

  if(i >= 0)
    return i;

  // no enabled peak threshold for this curve
  for(i = 0; i < MAX_LABELLED_CURVES; i ++)
    {
    if(! pPlot->plot_peak_labels.curve_peaks[ i ].enabled)
      {
      // curve_peaks[ i ].enabled is set TRUE when the user exits
      // autopeak mode with the enter key.  If the user exits autopeak
      // mode with the escape key, then curve_peaks[ i ].enabled will
      // remain false
      pPlot->plot_peak_labels.curve_peaks[i].curve_number = z_value;
      pPlot->plot_peak_labels.curve_peaks[i].threshold_value= (float)FLT_MAX;
      return i;
      }
    }
  return -1;  // curve_peak_mode[] is full, no more room
}

// If the current threshold value is outside the plot box, set it to the
// middle of the plot box.  Draw a threshold line and turn on auto_peak mode.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void auto_peak_start(SHORT the_window)
{
   SHORT PlotIndex = WindowPlotAssignment[the_window];
   PLOTBOX * pPlot = & Plots[PlotIndex];

   curve_index = set_curve_index(CursorStatus[PlotIndex].Z, pPlot);
   if(curve_index < 0)
    {
      error(ERROR_CURVE_PEAKS_FULL);
      return;
   }

   auto_peak_mode = TRUE;

   // Show the first threshold line using the current threshold value.
   // Use midpoint if outside of plot box.
   threshold =
    fix_threshold(the_window, TRUE,
                  pPlot->plot_peak_labels.curve_peaks[curve_index].threshold_value);
   CursorStatus[PlotIndex].Y = threshold;

   // redraw the entire plot box, axes and all
   erase_mouse_cursor();
   RemoveGraphCursor();
   MouseCursorEnable(FALSE);
   create_plotbox(& Plots[PlotIndex]);
   plot_curves(& MainCurveDir, & Plots[PlotIndex], the_window);
   MouseCursorEnable(TRUE);
   auto_peak_draw_threshold(the_window, & threshold);
}

// Get out of auto peak mode.  Enable peak labelling for the plot box
// temporarily so that labels will be drawn by plot_curves().
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void auto_peak_terminate(SHORT the_window)
{
   SHORT PlotIndex = WindowPlotAssignment[the_window];
   BOOLEAN label_save = Plots[PlotIndex].plot_peak_labels.label_peaks;

   if (pKSRecord != NULL)
   {
      if (*pKSRecord)
         MacRecordString("DRAW_PEAKS();\n");
   }

   // erase threshold line if it's visible
   auto_peak_draw_threshold(the_window, NULL);
   auto_peak_mode = FALSE;
   // redraw the entire plot box, axes and all
   erase_mouse_cursor();
   RemoveGraphCursor();
   MouseCursorEnable(FALSE);

   create_plotbox(& Plots[ PlotIndex ]);
   
   // temporarily enable peak labelling for the plot box.  Peak labels are
   // always displayed on exit from autopeak mode.
   Plots[PlotIndex].plot_peak_labels.label_peaks = TRUE;

   plot_curves(& MainCurveDir, & Plots[ PlotIndex ], the_window);
   MouseCursorEnable(TRUE);

   // restore the old label_peaks value to the plot box.
   Plots[PlotIndex].plot_peak_labels.label_peaks = label_save;

   // Put the graphics cursor back onto the curve unless in baseline
   // subtract mode.
   if(baslnsub_active())
      return;

   // Get the closest point with the X and Y values.
   if(GetTempCurvePointIndex(& MainCurveDir,
                                 CursorStatus[PlotIndex].EntryIndex,
                                 CursorStatus[PlotIndex].FileCurveNumber,
                                 &CursorStatus[PlotIndex].X,
                                 &CursorStatus[PlotIndex].Y,
                                 (UCHAR) Plots[PlotIndex].x.units,
                                 & CursorStatus[PlotIndex].PointIndex,
                                 -1)
    )
      return;

   UpdateCursorXStat(the_window);
   UpdateCursorYStat(the_window);
   SetCursorPos(the_window, CursorStatus[PlotIndex].X,
                            CursorStatus[PlotIndex].Y,
                            CursorStatus[PlotIndex].Z);
}

// plot only one curve so that user can see to set the threshold easily.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void auto_peak_plot(float Z, USHORT EntryIndex, USHORT FileCurveNumber,
                     CURVEHDR * TCurveHdr)
{
   if(Z == CursorStatus[ActiveWindow].Z)
      array_plot(&Plots[ActiveWindow], &MainCurveDir, EntryIndex,
                  FileCurveNumber, 0, TCurveHdr->pointnum, Z);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SetPeakThreshold(SHORT CurveIndex, FLOAT CurveNumber, FLOAT ThresholdVal)
{
  curve_index = CurveIndex;
  threshold = ThresholdVal;
  Plots[ActiveWindow].plot_peak_labels.
           curve_peaks[curve_index].curve_number = CurveNumber;
  Plots[ActiveWindow].plot_peak_labels.curve_peaks[ curve_index ].enabled =
     TRUE;
  Plots[ActiveWindow].plot_peak_labels.
                    curve_peaks[ curve_index ].threshold_value = threshold;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacDrawPeaks(void)
{
  SaveAreaInfo *SavedArea;

  if(plotAreaShowing())
    {
    BOOLEAN RemGCursor = TempChangeCursorType(&SavedArea);

    auto_peak_terminate(ActiveWindow);

    TempRestoreCursorType(RemGCursor, &SavedArea);
    }
}
