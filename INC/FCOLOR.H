/* -----------------------------------------------------------------------
/
/  fcolor.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/FCOLOR.H 1     6/18/99 1:52a Maynard $
*/
  
#ifndef FCOLOR_INCLUDED
#define FCOLOR_INCLUDED
  
#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif
  
// set up for false color plot, call once for each plot box before calling
// false_color_array_plot()
void false_color_init(SHORT DisplayWindow, PLOTBOX * plot);
  
// false color plot into a plotbox. Tagged curves are drawn in bright green
// if plotTagged is TRUE.
void false_color_array_plot(float z_value, LPCURVEDIR pCurveDir,
                             USHORT EntryIndex, USHORT CurveIndex,
                             USHORT total_points, UCHAR xDataUnits,
                             BOOLEAN plotTagged);

// draw a plotbox only, but correct size and axes for a false color plot
void false_color_draw_plotbox(PLOTBOX * plot);
  

// adjust plotbox setup for false color, make room for color key
void false_color_plotbox_adjust(SHORT theWindow, PLOTBOX * pPlot);

#endif  // FCOLOR_INCLUDED
