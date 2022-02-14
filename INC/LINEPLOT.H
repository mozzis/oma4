/* -----------------------------------------------------------------------
/
/  lineplot.h
/                  some routines for accumulating points and drawing lines
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/LINEPLOT.H 1     6/18/99 1:52a Maynard $
 
 *    Rev 1.0   07 Jan 1992 11:51:02   cole
 * Initial revision.
*/

#ifndef LINEPLOT_INCLUDED
#define LINEPLOT_INCLUDED

// This function must be called first, numPoints is the anticipated number
// of points to be drawn.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void startLinePlot(unsigned numPoints);

// x and y in VDC.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void addPoint(int x, int y);

// draw a line through all the points added in so far and then clear the
// list of points
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void drawLinePlot(void);

// draw any left over points and deallocate storage.  This function MUST be
// called when done with drawing lines in order to free up the heap.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void endLinePlot(void);

void setPreDrawTask(void ((*Task)(void)));

#endif   // LINEPLOT_INCLUDED
