/* -----------------------------------------------------------------------
/
/  multi.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/multi.c_v   0.11   13 Jan 1992 14:17:06   cole  $
/  $Log:   J:/logfiles/oma4000/main/multi.c_v  $
*/

#include <bios.h>
#include <dos.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "multi.h"
#include "plotbox.h"  // MAXPLOTS

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

PRIVATE int fullscreen_index[] = {0,1,3,5,8,11,14,18,22,28};
int fullscreen_count[] = {1,2,2,3,3,3,4,4,6,8};

// plot assignment for the 9 windows
UCHAR WindowPlotAssignment[MAXPLOTS] = {0,1,2,3,4,5,6,7,8};

CRECT PlotWindows[MAXPLOTS];

PRIVATE CRECT fullscreen_table[] =
{  {0,0,1024,1024},            /* 0 */

   {0,0,1024,512},             /* 1 */
   {0,512,1024,1024},

   {0,0,512,1024},             /* 3 */
   {512,0,1024,1024},

   {0,0,1024,341},             /* 5 */
   {0,341,1024,683},
   {0,683,1024,1024},

   {0,0,1024,512},             /* 8 */
   {0,512,512,1024},
   {512,512,1024,1024},

   {0,0,512,1024},             /* 11 */
   {512,0,1024,512},
   {512,512,1024,1024},

   {0,0,512,512},              /* 14 */
   {0,512,512,1024},
   {512,0,1024,512},
   {512,512,1024,1024},

   {0,0,1024,256},             /* 18 */
   {0,256,1024,512},
   {0,512,1024,768},
   {0,768,1024,1024},

   {0,0,1024,167},             /* 22 */
   {0,167,1024,341},
   {0,341,1024,512},
   {0,512,1024,683},
   {0,683,1024,880},
   {0,880,1024,1024},

   {0,0,1024,128},             /* 28 */
   {0,128,1024,256},
   {0,256,1024,384},
   {0,384,1024,512},
   {0,512,1024,640},
   {0,640,1024,768},
   {0,768,1024,896},
   {0,896,1024,1024}
};

/****************************************************************************/
/*                                                                          */
/* requires: PLOTBOX* - pointer to the first of an array of plotboxes.      */
/*           CRECT * - pointer to an array of two cartesian points in       */
/*                     VDC units. The first specifies the lower left-hand   */
/*                      coordinate of the screen and the second specifies   */
/*                      the upper right-hand coordinate of the screen.      */
/*           int - specifies the screen format (0..10).                     */
/*                                                                          */
/****************************************************************************/
int multiplot_setup(CRECT *Windows, CRECT * area, int scr_form)
{
  register int i;
  int  count,index;
  long range_x,range_y;
  CXY value;

  count = fullscreen_count[scr_form];
  index = fullscreen_index[scr_form];
  range_x = area->ur.x - area->ll.x;
  range_y = area->ur.y - area->ll.y;
  for (i=0; i<count; i++)
    {
    value.x = area->ll.x + (int)
      ((range_x * (long) fullscreen_table[i + index].ll.x) >> 10);

    value.y = area->ll.y + (int)
      ((range_y * (long) fullscreen_table[i + index].ll.y) >> 10);

    Windows[i].ll = value ;

    value.x = area->ll.x + (int)
      ((range_x * (long) fullscreen_table[i + index].ur.x) >> 10);

    value.y = area->ll.y + (int)
      ((range_y * (long) fullscreen_table[i + index].ur.y) >> 10);

    Windows[i].ur = value ;
    }
  return 0;
}

CRECT plotWindowArea(SHORT windowNumber)
{
  return PlotWindows[windowNumber];
}

