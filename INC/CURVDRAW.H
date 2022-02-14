/* -----------------------------------------------------------------------
/
/  curvdraw.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/CURVDRAW.H 1     6/18/99 1:52a Maynard $
*/

#ifndef CURVDRAW_INCLUDED
#define CURVDRAW_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

#define  X_RAMP      0     /* y data, x is increased by regular steps */
#define  Y_RAMP      1     /* y ramp not yet supported... */
#define  XY_DATA     2     /* parametric plots, x vs. y */
  
enum { POINT_BATCH_SIZE = 100 };
 
#define DRAW_ALL              1
#define DRAW_TAGGED_ONLY      2
#define DRAW_TAGGED_AS_NORMAL 4
#define DRAW_PEAK_LABELS      8

// returns TRUE if abnormal termination.
BOOLEAN plot_curves_labels(LPCURVEDIR pCurveDir, PLOTBOX *pPlotBox,
                            SHORT DisplayWindow, SHORT drawSpecifier);

// returns TRUE if abnormal termination
BOOLEAN plot_curves(LPCURVEDIR, PLOTBOX *, SHORT);

// replot curves only without redrawing the entire plot box, axis labels and
// tick marks remain unchanged
void ReplotCurvesOnly(SHORT Window);

void Replot(SHORT);

int PutUpPlotBoxes(void);

USHORT DupDisplayWindow(LPCURVEDIR pCurveDir, USHORT DstWindow,
                         USHORT SrcWindow);

// draw one curve in a window using curve_color.
// zVal specifies which curve to draw.
// returns TRUE if abnormal termination
BOOLEAN draw_one_curve(USHORT window, float zVal, SHORT drawSpecifier);

BOOLEAN FindFirstPlotBlock(LPCURVEDIR, PSHORT, PUSHORT, PUSHORT, SHORT);
BOOLEAN FindLastPlotBlock(LPCURVEDIR, PSHORT, PUSHORT, PUSHORT, SHORT);
BOOLEAN FindNextPlotCurve(LPCURVEDIR, PSHORT, PUSHORT, PUSHORT, SHORT);
BOOLEAN FindPrevPlotCurve(LPCURVEDIR, PSHORT, PUSHORT, PUSHORT, SHORT);

#endif   // CURVDRAW_INCLUDED
