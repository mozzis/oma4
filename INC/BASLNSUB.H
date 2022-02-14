/* -------------------------------------------------------------------------
/  FILE : baslnsub.h 
/  
/  form and plot for baseline subtract
/
/  $Header: /OMA4000/Include/BASLNSUB.H 1     6/18/99 1:52a Maynard $
/  $Log: /OMA4000/Include/BASLNSUB.H $
 * 
 * 1     6/18/99 1:52a Maynard
/ --------------------------------------------------------------------------
*/
  
#ifndef BASLNSUB_INCLUDED
#define BASLNSUB_INCLUDED
  
#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

#ifndef POINTS_INCLUDED
  #include "points.h"
#endif
  
// return TRUE iff baseline subtract is active
BOOLEAN baslnsub_active(void);
  
// plot special baseline curves for curve Z in the baseline subtract plot
// box.  EntryIndex and FileCurveNumber define the location of the curve
// data in MainCurveDir.  TCurveHdr is the curve header for the curve data.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA baslnsub_plot(float Z, USHORT EntryIndex,
                            USHORT FileCurveNumber, CURVEHDR * TCurveHdr);

GET_POINT_FUNC * WhichAutoscaleGetFunction(void);

// required for cursor adjustment after autoscale, only call if in baseline
// subtract mode.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void baslnAutoscaleCursorAdjust(void);
  
void add_graphics_cursor_knot(void);
  
void add_graphics_cursor_knot_init(void);
  
void macDoAddKnot(double xVal, double yVal);

// init FormTable[] with & baseline_sub_form and & knots_form
void registerBaslnsubForms(void);

#endif   // BASLNSUB_INCLUDED
