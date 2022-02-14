/* -----------------------------------------------------------------------
/
/  multi.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/MULTI.H 1     6/18/99 1:53a Maynard $
 * 
*/

#ifndef MULTI_INCLUDED
#define MULTI_INCLUDED

#ifndef CGIBIND_INCLUDED
   #define CGIBIND_INCLUDED
   #include <cgibind.h>
#endif

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern int fullscreen_count[];
extern UCHAR WindowPlotAssignment[];
extern CRECT PlotWindows[];

int multiplot_setup(CRECT *Windows, CRECT *area, int window_style);

// return the VDC coordinates of where to put a window
CRECT plotWindowArea(SHORT window);

// void multiplot_plot(int count);
#endif   // MULTI_INCLUDED
