/* -----------------------------------------------------------------------
/
/  lineplot.c
/                  some routines for accumulating points and drawing lines
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/lineplot.c_v   1.0   07 Jan 1992 11:56:00   cole  $
/  $Log:   J:/logfiles/oma4000/main/lineplot.c_v  $
 * 
 *    Rev 1.0   07 Jan 1992 11:56:00   cole
 * Initial revision.
 * 
 * 
*/

#include <stddef.h>  // NULL
#include <malloc.h>

#ifndef CGIBIND_INCLUDED
   #define CGIBIND_INCLUDED
   #include <cgibind.h>
#endif

#include "lineplot.h"
#include "device.h"    // deviceHandle()

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

PRIVATE CXY * plotlines = NULL;    // array of points for plotting
PRIVATE USHORT plotSize;           // dimension of the plotlines array
PRIVATE USHORT pointIndex;         // number of points in the plotlines array

enum { BUFPOINTS = 8 };
PRIVATE CXY tempBuf[BUFPOINTS];    // in case malloc() fails
PRIVATE BOOLEAN plotlinesMalloc = FALSE;

void (*DoBeforeDraw)(void) = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setPreDrawTask(void ((*Task)(void)))
{
  DoBeforeDraw = Task;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void startLinePlot(USHORT numPoints)
{
  if(plotlines && plotlinesMalloc)
    free(plotlines);

  plotlines = NULL;

  plotlinesMalloc = TRUE;  // assume malloc() will succeed()

  if (numPoints > 2048) numPoints = 2048;

  for(plotSize = numPoints;; plotSize /= 2)
    {
    if(plotSize <= BUFPOINTS)
      {
      plotlines = tempBuf;
      plotSize  = BUFPOINTS;
      plotlinesMalloc = FALSE;
      break;
      }
    if(plotlines = (CXY *) malloc(plotSize * sizeof(CXY)));
    break;
    }
  pointIndex = 0;  // there are no points yet
}

// x and y in VDC.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void addPoint(int x, int y)
{
  if(pointIndex >= plotSize)
    {
    drawLinePlot();
    plotlines[0] = plotlines[ plotSize - 1 ];
    pointIndex = 1;
    }
  plotlines[ pointIndex ].x = x;
  plotlines[ pointIndex ++ ].y = y;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void drawLinePlot(void)
{
  if(pointIndex < 1)
    return;

  if (DoBeforeDraw)
    {
    (*DoBeforeDraw)();
    DoBeforeDraw = NULL;
    }

  if(pointIndex == 1)     // be sure to draw a dot if only one point
    {
    plotlines[1] = plotlines[0];
    pointIndex = 2;
    }

  CPolyline(deviceHandle(), pointIndex, plotlines);
  pointIndex = 0;
}

// draw any left over points and then deallocate the array
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void endLinePlot(void)
{
  drawLinePlot();

  if(plotlines && plotlinesMalloc)
    free(plotlines);

  DoBeforeDraw = NULL;
  plotlines = NULL;
}

