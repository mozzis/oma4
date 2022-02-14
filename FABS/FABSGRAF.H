/* fabsgraf.h */

#ifndef PRIMTYPE_INCLUDED
  #include "primtype.h"
#endif

#define LN_TRIGWAIT    1
#define LN_PRESS_KEY   2
#define LN_FRAMETIME   3
#define LN_DETID       4
#define LN_TIMER       6

/* output graphics text centered at coordinates x, y - like printf */

int xy_printf(int x, int y, int *oldlen, char * format, ...);

/* output centered graphics text at line "line" - like printf */

int at_printf(int line, char * format, ...);

/* erase a text line */

void at_erase(int line);

/* initialize graphics system - returns non-zero on failure */

int InitGraf(void);

/* intialize the plotting system - memory alloc, axes, etc. */

int InitPlot(unsigned int points);

/* scale the graphics display */

void ScaleDisplay(float *Result, int points, float *YMin, float *YMax);

/* draw data on the screen. */

int plot_Data(float *Result, int points);

/* shut down the graphics system */

int DeInitGraf(void);

