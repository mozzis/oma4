/* -----------------------------------------------------------------------
/
/  fcolor.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/fcolor.c_v   0.18   12 Jan 1992 11:05:40   cole  $
/  $Log:   J:/logfiles/oma4000/main/fcolor.c_v  $
*/

#include <math.h>
#include <string.h>    // strstr()
#include <malloc.h>
#include <stdlib.h>

#include "fcolor.h"
#include "device.h"
#include "points.h"
#include "doplot.h"    // create_plotbox(), gss_position()
#include "symbol.h"    // symbol()
#include "di_util.h"   // StripExp()
#include "calib.h"     // ApplyCalibrationToX
#include "forms.h"     // GREEN and all the other colors
#include "omameth.h"   // InitialMethod
#include "crvheadr.h"  // FLOATTYPE
#include "plotbox.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

#define MAX_TEXT_LEN 15
#define NUM_EGA_COLORS 16
#define MAX_FALSE_COLORS 256
#define NUM_GRAYS 64

// if palette is limited, can't redefine colors, so reorder existing
// colors in order from lowest range to highest range of y-values
static int false_colors[NUM_EGA_COLORS] = {
    BLACK,
    WHITE,  /* appears grey */
    BROWN,
    RED,
    BRT_RED,
    BRT_ORANGE,
    BRT_YELLOW,
    GREEN,
    BRT_GREEN,
    CYAN,
    BLUE,
    BRT_BLUE,
    BRT_CYAN,
    PURPLE,
    BRT_PURPLE,
    BRT_WHITE
};

#define TMPBUFPOINTS  512
PRIVATE SHORT NumFalseColors=16;
PRIVATE SHORT MinFalseColor=0;
PRIVATE SHORT MaxFalseColor=16;
PRIVATE SHORT *pixels = NULL;         // array of color values for drawing
PRIVATE USHORT pixelCount = 0;         // number of points in Pixels array
PRIVATE SHORT tempBuf[TMPBUFPOINTS];    // in case malloc() fails

// define the lower limit y-value corresponding to each of the false colors
static float range_lower_limit[MAX_FALSE_COLORS];

// size in y val's of a single color range
static float rangeSize;

// the plotbox being used for the false color plot
static PLOTBOX * pPlot;

// the window being used for the false color plot.
static SHORT window_FC;

static SHORT PhysXDist;
static SHORT PhysYDist;

static SHORT MaxPixColor = 0;

// find the range number corresponding to a y value
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int _pascal false_color_range(float y_val)
{
  int retval;

  retval = (int)(((y_val - range_lower_limit[0]) * rangeSize)+0.5F);

  retval += MinFalseColor;

  if (retval < MinFalseColor)
    retval = MinFalseColor;

  if (retval > MaxFalseColor)
    retval = MaxFalseColor;

  if (NumFalseColors <= NUM_EGA_COLORS)
    return false_colors[retval];
  else
    return retval;
}

// determine text strings for use in the color key and return the width
// of the color key.  If max_text_len > 0, it is a default value to be
// used instead of computing a value.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void color_key_sizes(
                        char limits_text[NUM_EGA_COLORS][MAX_TEXT_LEN],
                        int * char_height, int * color_height,
                        int * color_width)
{
  SHORT i, text_len, max_text_len = 0;
  FLOAT j;

  j = (FLOAT)(NumFalseColors / NUM_EGA_COLORS);

  // compute all the range strings and find the length of the longest one
  for(i = 0; i < NUM_EGA_COLORS; i ++)
    {
    sprintf(limits_text[i], "%10.4g", range_lower_limit[(SHORT)((FLOAT)i * j)]);
    // delete leading + and 0's from the exponent and the number
    text_len = condense_float_string(limits_text[i]);
    if (text_len == 0)
      text_len = sprintf(limits_text[i], "0.00");
    if(text_len > max_text_len)
      max_text_len = text_len;
    }
  max_text_len += 1;  // one extra character space

  *color_height = (pPlot->plotarea.ur.y - pPlot->plotarea.ll.y) / NUM_EGA_COLORS;
  // use same formula for character height as draw_axis() in doplot.c
  *char_height = percent(pPlot->plotarea.ll.y - pPlot->fullarea.ll.y, 30);
  // don't make characters bigger than the color box
  if(*char_height > (*color_height))
    *char_height = *color_height;

  * color_width = (int)(*char_height * max_text_len * pPlot->xscale);
  // make 2 device pixels wider
  * color_width = adjustXbyDCOffset(* color_width, +2);
}

// Draw the false color key to the right of the plotted curves.
// range_text[][] contains the text strings to be written in for each color
// in the color key.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void draw_color_key(char range_text[NUM_EGA_COLORS][MAX_TEXT_LEN],
                            int char_height, int color_height)
{
  SHORT i;
  FLOAT j;
  CRECT outline;
  CXY text_point;
  CXY BorderLine[5];
  CCOLOR SelColor;
  CINTERIORFILL SelStyle;

  j = (FLOAT)(NumFalseColors / NUM_EGA_COLORS);

  // fill in color bars in the color key area, also range value
  // set up for solid fill
  CSetFillInterior(deviceHandle(), CSolidFill, &SelStyle);
  outline.ll.x = pPlot->fullarea.ur.x;
  outline.ll.y = pPlot->plotarea.ll.y;
  outline.ur.x = pPlot->fullarea.ur.x + pPlot->colorKeyWidth;

  if(outline.ur.x > DisplayGraphArea.ur.x || outline.ur.x < 0)
    outline.ur.x = DisplayGraphArea.ur.x;

  for(i = 0; i < NUM_EGA_COLORS; i ++)
    {
    CSetFillColor(deviceHandle(),
                  false_color_range(range_lower_limit[(SHORT)((FLOAT)i*j+0.5F)]),
                  &SelColor);
    outline.ur.y = outline.ll.y + color_height;
    CBar(deviceHandle(), outline);

    // print the range value on top of the color
    CSetLineColor(deviceHandle(), i < 7 ? BRT_WHITE : BLACK, &SelColor);
    text_point = outline.ll;
    text_point.x += char_height / 2;
    text_point.y += (color_height - char_height) / 2;
    // move up one device pixel
    text_point.y = adjustYbyDCOffset(text_point.y, +1);

    symbol(& text_point, range_text[i], 0.0F,
      char_height * pPlot->xscale,
      char_height * pPlot->yscale);

    outline.ll.y = outline.ur.y;  // move up to start next color
    }


  // outline the color key area in white
  BorderLine[0].x = outline.ll.x;
  BorderLine[0].y = pPlot->plotarea.ll.y;
  BorderLine[1].x = outline.ur.x;
  BorderLine[1].y = BorderLine[0].y;
  BorderLine[2].x = BorderLine[1].x;
  BorderLine[2].y = pPlot->plotarea.ur.y;
  BorderLine[3].y = BorderLine[2].y;
  BorderLine[3].x = BorderLine[0].x;
  BorderLine[4] = BorderLine[0];

  CSetLineColor(deviceHandle(), BRT_BLUE, &SelColor);
  CPolyline(deviceHandle(), 5, BorderLine);
}

// Determine the numerical lower limits for each of the false colors and
// put the values into range_limits[].
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void set_limits(SHORT DisplayWindow)
{
  float min = Plots[DisplayWindow].z.min_value;
  float max = Plots[DisplayWindow].z.max_value;
  SHORT i;

  // set up y-value boundaries for each color range
  rangeSize = (max - min) / NumFalseColors;
  for (i = 0; i < NumFalseColors; i++)
	  range_lower_limit[i] = min + i * rangeSize;

  /* invert rangeSize so false_color_range() does multiply not divide */

  if (rangeSize > (float)MAX_FLOAT_DIFF)
    rangeSize = 1.0F / rangeSize;
  else
    rangeSize = 1.0F;
}

// Set up the plot box for false color plot.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void false_color_start(PLOTBOX * plot, SHORT window)
{
  pPlot = plot;  // all fcolor functions will use this plot box
  window_FC   = window;
  NumFalseColors = deviceColors();
  NumFalseColors = min(MAX_FALSE_COLORS, NumFalseColors);
  init_colors(deviceHandle(), NumFalseColors);
  
  MaxFalseColor = NumFalseColors;

  if (NumFalseColors > NUM_EGA_COLORS)
    MinFalseColor = NUM_EGA_COLORS;

  else
    MinFalseColor = 0;

  // set up false color boundary values
  set_limits(window);
}

// set up range limits for each of the false colors evenly spaced from
// y_min to y_max based on the min and max of all curves in a plot box.
// Modify and draw the plot box but not the curves.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void false_color_init(SHORT DisplayWindow, PLOTBOX * plot)
{
   // color key sizes and text
   char limits_text[NUM_EGA_COLORS][MAX_TEXT_LEN];
   int char_height;
   int color_height;

   false_color_start(plot, DisplayWindow);

   // compute color key width
   color_key_sizes(limits_text, &char_height, &color_height,
                   &plot->colorKeyWidth);
}

// Draw a plotbox only, but correct size and axes for a false color plot.
// Clear the full plotbox area to the background color.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void false_color_draw_plotbox(PLOTBOX * plot)
{
   // color key sizes and text
   char limits_text[NUM_EGA_COLORS][MAX_TEXT_LEN];
   int char_height;
   int color_height;
   CCOLOR SelColor;
   CINTERIORFILL SelStyle;

   // erase full area of plotbox (fill in with background color)
   // copied from draw_plotbox() in doplot.c

   CSetFillColor(deviceHandle(), plot->background_color, &SelColor);
   CSetFillInterior(deviceHandle(), CSolidFill, &SelStyle);

   plot->fullarea.ur.x += plot->colorKeyWidth;

   CBar(deviceHandle(), plot -> fullarea);
 
   false_color_start(plot, window_FC);
   
   color_key_sizes(limits_text, & char_height, & color_height,
                    & plot->colorKeyWidth);

   plot->fullarea.ur.x -= plot->colorKeyWidth;

   // same as create_plotbox in doplot.c but use normal_draw_plotbox()
   set_plotbox_size(plot);
   scale_axis(& plot -> x);
   scale_axis(& plot -> y);
   scale_axis(& plot -> z);
   normal_draw_plotbox(plot);
   
   draw_color_key(limits_text, char_height, color_height);
}

// Allocate buffer for color values translated from data values
// For speed, buffer will grow to fit longest curve used in session
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void allocPixelBuffer(USHORT *points)
{
  if (pixelCount < *points)           /* make buffer grow */
    {
    if (pixels != NULL)              /* discard previous buffer if any */
      free(pixels);
                                    /* try to allocate from heap */
    pixels = malloc(*points * sizeof(SHORT));    
    if (pixels == NULL)             /* if failure, */
      {
      pixels = &tempBuf[0];         /* put points in tmp buf */
      if (*points > TMPBUFPOINTS)    /* since no way to report error here, */
        {
        pixelCount = TMPBUFPOINTS;  /* only plot points which will fit */
        *points = pixelCount;
        }
      else
        pixelCount = *points;
      }
    else
      pixelCount = *points;
    }
}

/************************************************************************/
/* find the closest point corresponding to a given wavelength           */
/* this is done elsewhere in the program (e.g. GetTempCurvePointIndex)  */
/* but much less efficiently                                            */
/************************************************************************/
SHORT _pascal XtoPtIndx(CURVEDIR *pDir, USHORT Blk, USHORT Crv, FLOAT Wlen,
                        USHORT points, UCHAR DUnits)
{
  SHORT IncVal, SearchPixels, LeftPxl, NewLeft, Buf;
  ERR_OMA err;
  FLOAT Y, tmp1, tmp2, *SysCal = InitialMethod->CalibCoeff[0];

  points -= 1;

  /* If wavelength increases with pixel value, IncVal = 1; 0 otherwise */
  IncVal = SysCal[1] > (SysCal[2] + SysCal[3]);
  LeftPxl = 0;

  SearchPixels = 2 * points;  /* twice as many as are really there.*/

  err = GetDataPoint(pDir, Blk, Crv, LeftPxl, &tmp1, &Y, FLOATTYPE, &Buf);
  if (err)
    return 0;

  if (DUnits == COUNTS)
    tmp1 = ApplyCalibrationToX(SysCal, tmp1);

  /* search till region is 1 or 0 pixels wide, or is outside wavelength */
  while ((SearchPixels > 1) && ((IncVal) ? (tmp1 < Wlen) : (tmp1 > Wlen)))
    {
    SearchPixels = (SearchPixels) / 2;  /* divide region in half. */
    NewLeft = LeftPxl + SearchPixels;   /* NewLeft = center pixel */

    /* tmp2 = center wavelength */
    err = GetDataPoint(pDir, Blk, Crv, NewLeft, &tmp2, &Y, FLOATTYPE, &Buf);
    if (err)
      return 0;

    if (DUnits == COUNTS)
      tmp2 = ApplyCalibrationToX(SysCal, tmp2);

   /* see which half is valid; if it's the right half, reset left edge.*/
    if ((IncVal) ? (tmp2 <= Wlen) : (tmp2 >= Wlen))
      LeftPxl = NewLeft;
    err= GetDataPoint(pDir, Blk, Crv, LeftPxl, &tmp1, &Y, FLOATTYPE, &Buf);
    if (err)
      return 0;
    if (DUnits == COUNTS)
      tmp1 = ApplyCalibrationToX(SysCal, tmp1);
    }

  /* difference between next and given value */
  err= GetDataPoint(pDir, Blk, Crv, LeftPxl+1, &tmp1, &Y, FLOATTYPE, &Buf);
  if (err)
    return 0;
  if (DUnits == COUNTS)
    tmp1 = ApplyCalibrationToX(SysCal, tmp1);
  tmp1 -= Wlen;

  /* difference between this and given value */ 

  err= GetDataPoint(pDir, Blk, Crv, LeftPxl, &tmp2, &Y, FLOATTYPE, &Buf);
  if (err)
    return 0;
  if (DUnits == COUNTS)
    tmp2 = ApplyCalibrationToX(SysCal, tmp2);
  tmp2 = Wlen - tmp2;

  /* if closer, increment left pixel */
  if ((IncVal) ? (tmp1 < tmp2) : (tmp1 > tmp2))
    LeftPxl++;
  
  return (LeftPxl);
}

// False color plot for a single curve in a plot box.  The beginning and
// end data points are matched to the beginning and end wavelength value
// of the plotbox, but for speed, the intervening data points are not
// matched pixel for pixel with the correct wavelength.  The cursor
// readout of wavelength will still be correct, however.
// Add plotTagged argument.  RAC Oct 8, 1990
// Tagged curves are plotted in bright green if plotTagged is TRUE.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void false_color_array_plot(float z_value, CURVEDIR *pDir,
                            USHORT Blk, USHORT Crv,
                            USHORT Points, UCHAR DXUnits,
                            BOOLEAN plotTagged)
{
  SHORT i, prefBuf = 0;
  static SHORT StartPt, EndPt, OldPoints = 0xFFFF;
  FLOAT x, y, ZMin, ZMax;
  static FLOAT PlotXMin = -1.0F, PlotXMax = -1.0F;
  CRECT ClipRect, OldClipRect, plot_region;
  UCHAR PXUnits = (UCHAR)pPlot->x.units;
  BOOLEAN XInvert = FALSE;
  GET_POINT_FUNC * get_point = WhichGetFunction();

  allocPixelBuffer(&Points);

  ZMin = pPlot->y.min_value;
  ZMax = pPlot->y.max_value;
  /* don't bother plotting if off scale */
  if((z_value < ZMin && z_value < ZMax) || (z_value > ZMin && z_value > ZMax))
    return;

  MaxPixColor = 0;

  /* set the clipping rectangle and save the current one */
  if(CalcClipRect(pPlot, z_value, &ClipRect))
    return;

  ClipRect.ll.y = adjustYbyDCOffset(ClipRect.ll.y, 1);
  ClipRect.ll.x = adjustXbyDCOffset(ClipRect.ll.x, 1);
  ClipRect.ur.y = adjustYbyDCOffset(ClipRect.ur.y, -1);
  ClipRect.ur.x = adjustXbyDCOffset(ClipRect.ur.x, -1);

  CInqClipRectangle(deviceHandle(), &OldClipRect);
  CSetClipRectangle(deviceHandle(), ClipRect);

  /* find the starting and ending points to compute color values for */
  /* only done if either the plotbox size or the number of points is */
  /* changed.                                                        */

  if (PlotXMin == pPlot->x.min_value && PlotXMax == pPlot->x.max_value &&
      OldPoints == (SHORT)Points)
      ;       /* no change, don't do it */
  else
    {
    if (PXUnits != COUNTS)
      {
      StartPt = XtoPtIndx(pDir, Blk, Crv, pPlot->x.min_value, Points, DXUnits);
      EndPt   = XtoPtIndx(pDir, Blk, Crv, pPlot->x.max_value, Points, DXUnits);
      }
    else
      {
      StartPt = (USHORT)pPlot->x.min_value;
      EndPt   = (USHORT)pPlot->x.max_value;
      }
  
    if (StartPt > EndPt)
      {
      USHORT Temp = StartPt;
      StartPt = EndPt;
      EndPt = Temp;
      }

    if (EndPt > (SHORT)Points)
      {
      EndPt = Points-1;
      }

    if (StartPt < 0)
      StartPt = 0;

    if (StartPt + Points - 1 < EndPt)
      EndPt = StartPt + Points - 1;

    if (StartPt + Points - 1 > EndPt)
      Points = EndPt - StartPt;

    OldPoints = (SHORT)Points,
      PlotXMin = pPlot->x.min_value, PlotXMax = pPlot->x.max_value;
    }

    {
    /* define the region GSS will fill using the computed color values */
    FLOAT XMin = StartPt, XMax = EndPt;

    if (PXUnits != COUNTS)
      {
      XMin = ApplyCalibrationToX(InitialMethod->CalibCoeff[0], XMin);
      XMax = ApplyCalibrationToX(InitialMethod->CalibCoeff[0], XMax);
      }
    plot_region.ll = gss_position(pPlot, XMin, z_value, 0.0F);
    plot_region.ur = gss_position(pPlot, XMax, z_value+1.0F, 0.0F);
    }

  /* handle reversed X axis plotting */

  if (plot_region.ur.x < plot_region.ll.x)
    {
    int tX = plot_region.ll.x;
    plot_region.ll.x = plot_region.ur.x;
    plot_region.ur.x = tX;
    XInvert = TRUE;
    }

  /* handle reversed Y axis plotting */

  if (plot_region.ur.y < plot_region.ll.y)
    {
    int tY = plot_region.ll.y;
    plot_region.ll.y = plot_region.ur.y;
    plot_region.ur.y = tY;
    }

  for (i = StartPt; i <= EndPt; i++)
    {
    if ((*get_point)(pDir, Blk, Crv, i, &x, &y, FLOATTYPE, &prefBuf))
      {
      break;
      }

    /* for speed, don't compensate for plotting offset */
    /* (pixel buffer is big enough to hold whole curve)*/

    if (!XInvert)
      pixels[i] = false_color_range(y);
    else
      pixels[(EndPt-i)+StartPt] = false_color_range(y);
    }

    CCellArray(deviceHandle(),
      plot_region,
      Points,              /* total len of a row in color index array */
      Points,              /* #elements / row of CIA (norm = total length */
      1,                   /* #rows in CIA */
      CReplace,            /* pixel operation to be performed */
      &pixels[StartPt]);   /* color index array (CIA) */

    if (plotTagged)
      {
      CXY box[5];

      box[0].x = plot_region.ll.x;
      box[0].y = plot_region.ll.y;

      box[1].x = plot_region.ll.x;
      box[1].y = plot_region.ur.y;

      box[2].x = plot_region.ur.x;
      box[2].y = plot_region.ur.y;

      box[3].x = plot_region.ur.x;
      box[3].y = plot_region.ll.y;

      box[4].x = plot_region.ll.x;
      box[4].y = plot_region.ll.y;

      CPolyline(deviceHandle(), 4, &box[0]);
      }

  /* restore the clipping rectangle */
  CSetClipRectangle(deviceHandle(), OldClipRect);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void false_color_plotbox_adjust(SHORT theWindow, PLOTBOX * pPlotbox)
{
  char tempLimitText[NUM_EGA_COLORS][MAX_TEXT_LEN];
  int charHeight;
  int colorHeight;
  
  set_limits(theWindow);
  pPlot = pPlotbox;
  color_key_sizes(tempLimitText, &charHeight, &colorHeight,
                  &pPlot->colorKeyWidth);

  pPlot->fullarea.ur.x -= pPlot->colorKeyWidth;
}
