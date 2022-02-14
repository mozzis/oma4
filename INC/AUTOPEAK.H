// file autopeak.h       RAC  May 11, 1990
/*
/  $Header: /OMA4000/Include/AUTOPEAK.H 1     6/18/99 1:52a Maynard $
*/

#ifndef AUTOPEAK_INCLUDED
#define AUTOPEAK_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

// TRUE iff in auto peak mode.
extern BOOLEAN const * const is_auto_peak;

// switch to auto peak mode, determine first threshold.
void auto_peak_start(SHORT the_window);
                     
// Erase old threshold line and draw a new one at new_thresh.
// If new_thresh is NULL, just erase the current threshold line.
// Returns the new threshold value.
float auto_peak_draw_threshold(SHORT the_window, float const * new_thresh);

// get out of auto peak mode
void auto_peak_terminate(SHORT the_window);

// plot only the curve corresponding to the current cursor Z value in the
// currently active window.  EntryIndex and FileCurveNumber define the
// location of the curve data in MainCurveDir.  TCurveHdr is the curve header
// for the curve data.
void auto_peak_plot(float Z, USHORT EntryIndex, USHORT FileCurveNumber,
                     CURVEHDR * TCurveHdr);

// enable/disable peak labels for the current curve
void auto_peak_curve_enable(BOOLEAN on_off);

// draw the peaks of a curve above the curve, rotated 90 degrees, but only
// if peak labelling is enabled for the curve and not a false color plot.
void draw_peaks(LPCURVEDIR pCurveDir, USHORT EntryIndex, USHORT CurveIndex,
                 SHORT the_window, CURVEHDR const * PCurveHdr,
                 FLOAT z_value);

void SetPeakThreshold(SHORT CurveIndex, FLOAT CurveNumber,
                       FLOAT ThresholdVal);

void MacDrawPeaks(void);
                     
#endif  // AUTOPEAK_INCLUDED
