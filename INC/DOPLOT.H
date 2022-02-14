/* -----------------------------------------------------------------------
/
/   doplot.h
/
/  Copyright (c) 1989,  EG&G Princeton Applied Research
/
/  Written by: DAI      Version 1.00         16 October     1989
/  Worked on:
/
/  doplot.h function prototypes for plotting.
/
/  $Header: /OMA4000/Include/DOPLOT.H 1     6/18/99 1:52a Maynard $
/
/ ----------------------------------------------------------------------- */
  
#ifndef DOPLOT_INCLUDED
#define DOPLOT_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif


void draw_plotbox(PLOTBOX *);

// for use by false color only, bypass false color check in draw_plotbox()
void normal_draw_plotbox(PLOTBOX *);

void create_plotbox(PLOTBOX *);

ERR_OMA array_plot(PLOTBOX *, LPCURVEDIR, SHORT, SHORT,
                           USHORT, USHORT, FLOAT);

// set up the plot box corresponding to ActiveWindow
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ResizePlotForWindow(SHORT ActiveWindow);

#endif  //  DOPLOT_INCLUDED

void UpdatePlotscreenString(SHORT Row, SHORT Column, PCHAR String);
