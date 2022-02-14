/* -----------------------------------------------------------------------
/
/  doplot.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/doplot.c_v   1.11   12 Mar 1992 13:27:40   maynard  $
/  $Log:   J:/logfiles/oma4000/main/doplot.c_v  $
*/

#include <math.h>        // log10()
#include <string.h>
#include <stdlib.h>      // itoa()

#include "doplot.h"
#include "symbol.h"      // symbol()
#include "lineplot.h"    // startLinePlot()
#include "tempdata.h"    // ReadTempCurvehdr()
#include "di_util.h"     // ConvertUnits()
#include "device.h"      // deviceHandle()
#include "fcolor.h"      // false_color_draw_plotbox()
#include "forms.h"       // erase_mouse_cursor(), replace_mouse_cursor()
#include "calib.h"       // ApplyCalibrationToX()
#include "multi.h"       // WindowPlotAssignment
#include "omameth.h"     // InitialMethod
#include "plotbox.h"
#include "crvheadr.h"
#include "points.h"

// use the following for GRAPHICAL scan setup
//#include "scanset.h"     // inScanSetup()
//#include "scangraf.h"    // scanGrafXAxisAdjust()

/* -----------------------------------------------------------------------
/
/   doplot.c
/
/  Written by: TLB      Version 1.00       9-18 March       1988
/
/
/  doplot.c is a collection of routines to draw a cartesian plot, using
/  GSS (Graphic Software Systems) graphics interface drivers.
/  The caller creates a PLOTBOX structure in their program (see plot.h),
/  initializes the necessary variables, then calls create_plotbox() to
/  display the plot area on the screen or other output device.  Then,
/  depending on the application, the caller can either pass two complete
/  x and y arrays to array_plot(), to draw the entire graph at once, or
/  repeatedly call plot_point() with single xy pairs.  Note that if the
/  second method is used, set_first_plot_point() must be called with
/  the first xy pair. Graphs using hidden lines are supported with the
/  use of array_hplot(). Parameters are virtually the same as for the
/  function array_plot(). Also the GSS equivalent of v_pline() is supported
/  for hidden lines (hidden_v_pline()). Other functions necessary for the
/  use of hidden lines are hplot_alloc() and hplot_init.
/  These routines are designed to allow multiple plotboxes to be used at
/  the same time; all data specific to any one plotbox is kept in the PLOTBOX
/  structure for that plotbox. These are the variables that must be
/  initialised before calling create_plotbox():
/                         (assume - PLOTBOX   userplot)
/
/  userplot.x.legend;            char * to text for x axis
/  userplot.x.min_value;         float initial min value for x axis
/  userplot.x.max_value;         float initial max value for x axis
/  userplot.y.legend;            char * to text for y axis
/  userplot.y.min_value;         float initial min value for y axis
/  userplot.y.max_value;         float initial max value for u axis
/
/        these two xy positions define the size of the plot box
/  userplot.fullarea[0].x;       int lower left x position of plot box
/  userplot.fullarea[0].y;       int lower left y position of plot box
/  userplot.fullarea.ur.x;       int upper right x position of plot box
/  userplot.fullarea.ur.y;       int upper right y position of plot box
/  userplot.background_color;    int color of plotbox background
/  userplot.box_color;           int color of box around plot region, ticks
/  userplot.text_color;          int color of text in plotbox
/  userplot.grid_color;          int color of grid lines (set to 0 for no grid)
/  userplot.grid_line_type;      int line type for grid
/  userplot.plot_color;          int color of plotted data
/  userplot.plot_line_type;      int line type for lines between plotted points
/ ----------------------------------------------------------------------- */

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

CRECT full_screen = {{-32768,32767}, {-32768,32767}};

/* -----------------------------------------------------------------------
/  void format_clean_tick_value(value, string)
/
/  function:   formats a tick value to mark the ticks on the axis
/              which has no trailing zeros and, if the tick value is
/              less than one, no zero before the decimal point.
/              If the tick value is zero, the string simply is set
/              to "0" (which avoids feeding 0.0 to the code which
/              strips off the zeroes).
/  requires:   (float) value - the tick value
/              (char *) string - the string to put the clean tick value into.
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
PRIVATE void format_clean_tick_value(double value, char * string)
{
  if (value == 0.0)
    strcpy(string, "0");
  else
    {
    char tick_str[20];
    int  len;

    sprintf(tick_str, "%.3f", value);
    len = strlen(tick_str);
    while (len-- > 0)                   /* strip off trailing zeroes */
      {
      if (tick_str[len] == '.')
        {
        tick_str[len] = '\0';
        break;
        }
      else if (tick_str[len] == '0')
        {
        tick_str[len] = '\0';
        }
      else
        break;
      }
    if (tick_str[0] == '0')             /* remove leading zero (only if */
      strcpy(string, &tick_str[1]);     /* 0.nnn or -0. */
    else if (tick_str[0] == '-' && tick_str[1] == '0')
      {
      tick_str[1] = '-';
      strcpy(string, &tick_str[1]);
      }
    else
      strcpy(string, tick_str);

    /* limit to 6 chars */
    if (strlen(string) > 6)
      {
      char * temp = strchr(string, '.');
      if (temp)
        *temp = '\0';
      }
    }
}

// draw the axis title
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void draw_axis_legend(PLOTBOX * plot, AXISDATA * axis,
                               int legendExponent,
                               char which_one, double axis_angle,
                               int tick_line_length, int char_height,
                               int axis_length)
{
  char text[ 80 ];
  int  textlen;
  char expstr[ 20 ];
  int  explen;
  int  i;
  CHORALIGN  x_align = CTX_Center;
  CVERTALIGN y_align;
  // dTick is one half of the axis range
  float dTick = 0.5F/ axis->inv_range;
  CXY textpt;
  CXY rel_exp_pt;
  double cos_axis_angle = cos(axis_angle);
  double sin_axis_angle = sin(axis_angle);
  CCOLOR SelColor;
  float XAxisScale;
  float YAxisScale;

  strcpy(text, axis->legend);
  textlen = strlen(text);
  if(legendExponent)
    {
    strcat(text, " x 10");
    textlen = strlen(text);
    itoa(legendExponent, expstr, 10);
    explen = strlen(expstr);
    for (i=textlen; i<(textlen + explen); i++)
      text[i] = ' ';    /* save space for exponent */

    text[i] = '\0';  /* put in terminating null */
    }

  switch (which_one)
    {
    case 'X':
      XAxisScale = (float) axis_length * axis->inv_range;
      textpt.x = axis->axis_zero.x + (int) (dTick * XAxisScale);

      textpt.y = axis->axis_zero.y - tick_line_length;
      /* include space for the tick value */
      textpt.y -= (tick_line_length >> 1) + char_height;

      /* put it in the middle of the remaining space */
      textpt.y = ((textpt.y - plot->fullarea.ll.y) >> 1)+plot->fullarea.ll.y;

      if(legendExponent)
        {
        rel_exp_pt.x = (int)((double)(textlen * char_height) * plot->xscale);
        rel_exp_pt.y = (int) ((double)char_height * plot->yscale * 0.4);
        }
      y_align = CTX_Center;  // for later text alignment purposes
    break;

    case 'Y':
      YAxisScale = (float) axis_length * axis->inv_range;
      textpt.y = axis->axis_zero.y + (int) (dTick * YAxisScale);
      if(legendExponent)
        {
        rel_exp_pt.y = (int)((double)(textlen * char_height) * plot->xscale);
        rel_exp_pt.x = (int) ((double)char_height * plot->yscale * 0.3);
        }

      if (plot->z_position != LEFTSIDE)
        {
        textpt.x = axis->axis_zero.x - tick_line_length;
        /* include space for the tick value */
        textpt.x -= (tick_line_length >> 1) + char_height;

        /* put it in the middle of the remaining space */
        textpt.x = ((textpt.x-plot->fullarea.ll.x) >> 1)+plot->fullarea.ll.x;

        if(legendExponent)
          rel_exp_pt.x = -rel_exp_pt.x;
        y_align = CTX_Center;
        }
      else
        {
        textpt.x = axis->axis_zero.x + tick_line_length;
        /* include space for the tick value */
        textpt.x += (tick_line_length >> 1) + char_height;

        /* put it in the middle of the remaining space */
        textpt.x = plot->fullarea.ur.x-((plot->fullarea.ur.x-textpt.x) >> 1);

        if(legendExponent)
          rel_exp_pt.y = -rel_exp_pt.y;
        }
      y_align = CTX_Bottom;  // for later text alignment purposes
    break;
    case 'Z':
      {
      int zy_size = 0;
      int zx_size = 0;

      if(plot->style != FALSE_COLOR)
        {
        zy_size = (int)(((LONG)(plot->plotarea.ur.y - plot->plotarea.ll.y) *
                        (LONG) plot->yz_percent) / 100L);
        zx_size = (int)(((LONG)(plot->plotarea.ur.x - plot->plotarea.ll.x) *
                        (LONG) plot->xz_percent) / 100L);
        }

      XAxisScale = (float) zx_size * axis->inv_range;
      YAxisScale = (float) zy_size * axis->inv_range;

      textpt.y = axis->axis_zero.y + (int) (dTick * YAxisScale) -
      (int) (cos_axis_angle * (double) tick_line_length);
      /* space for tick value */
      textpt.y -= (int) (cos_axis_angle *
      (double) (tick_line_length) + (1.5 * (double) char_height));

      if(legendExponent)
        {
        rel_exp_pt.y = (int) (sin_axis_angle * (double) (textlen *
                        char_height) * plot->xscale);
        rel_exp_pt.x = (int) (cos_axis_angle * (double) (textlen *
                        char_height) * plot->xscale);
        }

      if (plot->z_position == RIGHTSIDE)
        {
        textpt.x = axis->axis_zero.x + (int) (dTick * XAxisScale) +
                   (int) (sin_axis_angle * (double) tick_line_length);
         /* space for tick value */
        textpt.x += (int) (sin_axis_angle *
                   (double) (tick_line_length) + (1.4 * (double)char_height));
        if(legendExponent)
          {
          rel_exp_pt.y += (int) (cos_axis_angle * (double) char_height *
                            plot->yscale * 0.4);
          rel_exp_pt.x -= (int) (sin_axis_angle * (double) char_height *
                            plot->yscale * 0.4);
          }
        }
      else
        {
        textpt.x = axis->axis_zero.x - (int) (dTick * XAxisScale) -
                      (int) (sin_axis_angle * (double) tick_line_length);
         /* space for tick value */
        textpt.x += ((int) (sin_axis_angle * (double) tick_line_length -
                     (2.3 * (double) char_height)));
        rel_exp_pt.y += (int) (cos_axis_angle * (double) char_height *
                         plot->yscale * 0.4);
        rel_exp_pt.x -= (int) (sin_axis_angle * (double) char_height *
                         plot->yscale * 0.4);
        }
      y_align = CTX_Top;  // for later text alignment purposes
      }
    break;
    }

  AlignText((FLOAT) ((double) char_height * plot->xscale),
            (FLOAT) ((double) char_height * plot->yscale),
             x_align, y_align, text, & textpt, (FLOAT) axis_angle);
  CSetLineColor(deviceHandle(), plot->text_color, &SelColor);
  symbol(& textpt, text, (float) axis_angle,
          (float) ((double) char_height * plot->xscale),
           (float) ((double) char_height * plot->yscale));

  /* print the exponent */
  if(legendExponent)
    {
    textpt.x += rel_exp_pt.x;
    textpt.y += rel_exp_pt.y;

    char_height = (char_height * 3) / 4;

    AlignText((FLOAT) ((double) char_height * plot->xscale),
               (FLOAT) ((double) char_height * plot->yscale),
               CTX_Left, CTX_Bottom, expstr, &textpt, (FLOAT) axis_angle);
    symbol(& textpt, expstr, (float) axis_angle,
            (float) ((double) char_height * plot->xscale),
            (float) ((double) char_height * plot->yscale));
    }
}

// Return the value of the exponent to use for scaling. All tick
// value numeric strings should be scaled by the proper power or ten when
// drawing the axis. axisEnd1 and axisEnd2 are the two endpoints of the
// axis in any order. Returned value will be a multiple of three.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int scientificExponent(double axisEnd1, double axisEnd2)
{
   int exponent;
   axisEnd1 = fabs(axisEnd1);
   axisEnd2 = fabs(axisEnd2);

   // make axisEnd1 the largest absolute value end point.
   if(axisEnd1 < axisEnd2) axisEnd1 = axisEnd2;

   // "cheat" by one order of magnitude so that values such as 1024 don't
   // require an exponent. Up to 9999 only requires 4 digits.
   if(axisEnd1 >= 1.0 && axisEnd1 < 10000.0) return 0;

   // Don't try to take the log of zero.
   if(axisEnd1 == 0.0) return 0;

   exponent = (int) floor(log10(axisEnd1)); // exponent of axisEnd1

   // return a multiple of three
   return exponent - (exponent % 3);
}

// find a rounded "nice" number approximately equal to x.
// ASSUME x is not zero !!!    REFERENCE : "Graphics Gems" page 657 ff.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE double nicenum(double x)
{
   int    exp  = (int) floor(log10(x));    // exponent of x
   double base = pow(10.0, (double) exp);  // base value of x, power of 10
   double f    = x / base;                 // between 1 and 10

   if(f < 1.5) return        base;

   if(f < 3.0) return  2.0 * base;

   if(f < 7.0) return  5.0 * base;

   return 10.0 * base;
}

// Compute tight labels from min and max (and NTICKS). Return the tick
// values in tickVal[] and the number of ticks in tickCount. Tight labels
// have the min and max at each end with "nice" values in between.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void tightLabel(double tickVal[], double min, double max,
                         int * tickCount)
{
   enum { NTICK = 5 };    // desired number of tick marks
   double d;              // tick mark spacing
   double x;
   double start;
   double end;

   if(max == min)
    {
    * tickCount = 1;
    tickVal[ 0 ] = min;
    return;
    }

   d = nicenum(fabs(max - min) / (NTICK - 1));

   * tickCount = 0;

   tickVal[ (* tickCount) ++ ] = min;

   if(min > max)
   {
      start = max;
      end   = min;
   }
   else
   {
      start = min;
      end   = max;
   }

   for(x = (floor(start / d) + 1) * d; x < end; x += d)
   {
      // don't let a tick value get too close to the min or the max
      if(fabs(x - start) < d * 0.67)
         continue;
      if(fabs(x - end) < d * 0.67)
         continue;

      tickVal[ (* tickCount) ++ ] = x;
   }

   tickVal[ (* tickCount) ++ ] = max;
}

/* -----------------------------------------------------------------------
/   void draw_axis(PLOTBOX *plot, char which_one)
/
/   function:   Displays the tick marks, tick values, text label,
/               and the scale magnitude for the axis specified.
/               Does not draw the axis line itself.
/   requires:
/               (PLOTBOX *) plot - the structure describing
/                                  the current plotting region, or "plotbox".
/               char which_one - which axis is being plotted, 'X', 'Y', 'Z'
/ ----------------------------------------------------------------------- */
PRIVATE void draw_axis(PLOTBOX *plot, char which_one)
{
   char        text[80];
   int         char_height,i;
   int         legendExponent;
   CHORALIGN   x_align;
   CVERTALIGN  y_align;
   double      axis_angle,sin_axis_angle,cos_axis_angle;
   double      scale_to_exponent;
   int         tick_line_length, tick_value_max_len;
   int         axis_length;
   int         legend_length;
   int         Margin;
   AXISDATA *  axis;
   CXY         tickline[2];
   CXY         textpt;
   CCOLOR      SelColor;
   CLINETYPE   SelLType;
   double      tickVal[ 15 ];  // where the ticks should go
   int         tickCount;      // the number of tick values in tickVal

   SizeAxis(plot, which_one, & axis, & axis_length);
   axisMarginAngle(plot, which_one, & Margin, & axis_angle);

   switch (which_one)
   {
      case 'X':
         char_height=percent(Margin,30);
         y_align = CTX_Top;
         break;

      case 'Y':
         char_height=percent(Margin,24);
         y_align = CTX_Bottom;
         break;

      case 'Z':
         char_height = percent(Margin, 25);
         y_align = CTX_Top;
         break;
   }

   legendExponent = scientificExponent(axis->min_value, axis->max_value);
   scale_to_exponent = pow(10.0, - legendExponent);

   // determine where the tick marks belong and how many there are
   tightLabel(tickVal, axis->min_value, axis->max_value, & tickCount);

   /* set up tick lines so that they reflect the size of shortest screen */
   /* axis */
   tick_line_length = Margin / 6;

   /* check for tick value length space assuming a maximum of 6 chars plus */
   /* a half space per tick */
   if (which_one == 'X')
      tick_value_max_len =
           (int) ((double) ((tickCount - 1) * 6.7 * char_height) *
           plot->xscale);
   else
      tick_value_max_len =    /* one of the ending strings is centered */
         (int) ((((double) (tickCount - 2) * 6.7) + 3.0) *
         (double) char_height * plot->xscale);

   if (tick_value_max_len > axis_length)
      char_height = (int) ((double) char_height * (double) axis_length /
         (double) tick_value_max_len);

   /* limit the character width based on the legend length */
   legend_length = strlen(axis->legend) * (int) ((double) char_height *
                           plot->xscale);
   if (legend_length > axis_length)
      char_height = (int) ((double) char_height * (double)axis_length /
                    (double) legend_length);

   // plot tickmarks and tick values
   CSetLineType(deviceHandle(), CLN_Solid, &SelLType);
   CSetLineColor(deviceHandle(), plot->text_color, &SelColor);

   for(i = 0; i < tickCount; i ++)
   {
      switch (which_one)
      { /* beginning of switch */

         case 'X' :
            tickline[0] = tickline[1] = gss_position(plot,
                                                      (float) tickVal[i],
                                                      plot->y.min_value,
                                                      plot->z.min_value);
            tickline[1].y -= tick_line_length;
            break;

         case 'Y':
         {
            float xIntercept    = plot->x.min_value;
            int   tickEndOffset = - tick_line_length;

            if(plot->z_position == LEFTSIDE)
            {
               xIntercept    = plot->x.max_value;
               tickEndOffset = tick_line_length;
            }

            tickline[0] = tickline[1] = gss_position(plot, xIntercept,
                                                      (float) tickVal[i],
                                                      plot->z.min_value);
            if(plot->z_position == LEFTSIDE)
               if(plot->x.max_value == plot->x.min_value)
               {
                  tickline[0].x += plot->x.axis_end_offset.x;
                  tickline[1].x += plot->x.axis_end_offset.x;
               }

            tickline[1].x += tickEndOffset;

            break;
         }

         // ASSUME: draw_axis('Z') is not called unless there really is a
         //         z-axis to draw.
         case 'Z':
         {
            float xIntercept = plot->x.min_value;

            sin_axis_angle = sin(axis_angle);
            cos_axis_angle = cos(axis_angle);

            if(plot->z_position == RIGHTSIDE)
               xIntercept = plot->x.max_value;

            tickline[0] = tickline[1] = gss_position(plot, xIntercept,
                                                      plot->y.min_value,
                                                      (float) tickVal[i]);
            if(plot->z_position == RIGHTSIDE)
               if(plot->x.max_value == plot->x.min_value)
               {
                  tickline[0].x += plot->x.axis_end_offset.x;
                  tickline[1].x += plot->x.axis_end_offset.x;
               }

            tickline[1].x += (int) (sin_axis_angle
                                     * (double) tick_line_length);
            tickline[1].y -= (int) (cos_axis_angle
                                     * (double) tick_line_length);
            break;
         }
      }

      CPolyline(deviceHandle(), 2, tickline);

      textpt = tickline[1];
      switch (which_one)
      {
         case 'X':
            textpt.y -= tick_line_length/2;
            break;

         case 'Y':
            if (plot->z_position != LEFTSIDE)
               textpt.x -= tick_line_length/2;
            else
               textpt.x += tick_line_length/2;

            break;

         case 'Z':
            textpt.x += (int) (sin_axis_angle * 0.5
                                * (double) tick_line_length);
            textpt.y -= (int) (cos_axis_angle * 0.5
                                * (double) tick_line_length);
            break;
      }

      if (i == 0)
      {
         switch (which_one)
         {
            case 'Y':
               if (plot->z_position == LEFTSIDE)
               {
                  x_align = CTX_Right;
                  break;
               }
            case 'X':
               x_align = CTX_Left;
               break;
            case 'Z':
               if (plot->z_position == RIGHTSIDE)
                  x_align = CTX_Left;
               else
                  x_align = CTX_Right;
               break;
         }
      }
      else
      {
         if(i == (tickCount - 1))
         {
            if ((which_one == 'X') && (plot->z_position != LEFTSIDE))
               x_align = CTX_Right;
         }
         else
            x_align = CTX_Center;
      }

      format_clean_tick_value(tickVal[ i ] * scale_to_exponent, text);

      AlignText((FLOAT) ((double) char_height * plot->xscale),
                 (FLOAT) ((double) char_height * plot->yscale),
                 x_align, y_align, text,
                 &textpt, (FLOAT) axis_angle);

      symbol(& textpt, text, (float) axis_angle,
              (float) ((double) char_height * plot->xscale),
              (float) ((double) char_height * plot->yscale));
   }

   /* plot the axis legend in the center of the axis */
   draw_axis_legend(plot, axis, legendExponent, which_one, axis_angle,
                     tick_line_length, char_height, axis_length);
}

/* -----------------------------------------------------------------------
/
/  normal_draw_plotbox(plot)
/
/  function:   draws the plotbox which is pointed to by "plot"
/  requires:   (PLOTBOX *) plot - the structure describing
/              the current plotbox.
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
void normal_draw_plotbox(PLOTBOX * plot)
{
   CCOLOR SelColor;
   CINTERIORFILL SelFStyle;
   AXISDATA * axisDummy;
   int axisLengthDummy;

   /* erase full area of plotbox (fill in with background color) */

   CSetFillColor(deviceHandle(), plot->background_color, &SelColor);
   CSetFillInterior(deviceHandle(), CSolidFill, &SelFStyle);
   CBar(deviceHandle(), plot->fullarea);
   /* outline the plotting area (left and lower lines are x,y axis) */
   CSetFillColor(deviceHandle(), plot->box_color, &SelColor);

   CSetBgColor(deviceHandle(), plot->background_color, &SelColor);

   SizeAxis(plot, 'X', & axisDummy, & axisLengthDummy);
   SizeAxis(plot, 'Y', & axisDummy, & axisLengthDummy);
   SizeAxis(plot, 'Z', & axisDummy, & axisLengthDummy);

// use the following for GRAPHICAL scan setup
//   scanGrafXAxisAdjust(& plot->x.min_value, & plot->x.max_value);

   draw_axis(plot,'X');    /* draw axis x*/
   draw_axis(plot,'Y');    /* draw axis y*/
   // only draw z-axis if not a false color plot
   if((plot->z_position) && (plot -> style != FALSE_COLOR))
      draw_axis(plot,'Z');    /* draw axis z*/
   else
   {
      plot->z.axis_end_offset.x = 0;
      plot->z.axis_end_offset.y = 0;
   }

// use the following for GRAPHICAL scan setup
//   if(* inScanSetup)
//      drawGroupsizeTicks();

   if (plot->title != NULL)
   {
      int TopMargin = plot->fullarea.ur.y - plot->plotarea.ur.y;
      int char_height=percent(TopMargin, 50);
      CXY textpt;

      textpt.x = ((plot->fullarea.ur.x - plot->fullarea.ll.x) >> 1)
                  + plot->fullarea.ll.x;

      textpt.y = (TopMargin >> 1) + plot->plotarea.ur.y;

      AlignText((FLOAT) ((double) char_height * plot->xscale),
                 (FLOAT) ((double) char_height * plot->yscale),
                 CTX_Center, CTX_Center,
                 plot->title, &textpt, 0.0F);

      CSetLineColor(deviceHandle(), plot->text_color, &SelColor);
      symbol(& textpt, plot->title, 0.0F,
              (float) ((double) char_height * plot->xscale),
              (float) ((double) char_height * plot->yscale));
   }

   drawPlotboxOutline(plot);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void draw_plotbox(PLOTBOX * plot)
{
   erase_mouse_cursor();

   plot->xscale = 0.7F;
   plot->yscale = 1.0F;

   if(plot->style == FALSE_COLOR)
      false_color_draw_plotbox(plot);
   else
      normal_draw_plotbox(plot);

   replace_mouse_cursor();
}

/* -----------------------------------------------------------------------
/
/  create_plotbox(plot)
/
/  function:   This is the first plot function called.  To use these
/              plotting functions, a "PLOTBOX" structure is created
/              and initialized with a few key parameters, then
/              create_plotbox() is called to calculate the remaining
/              parameters and display the plotting region, or "plotbox".
/  requires:   (PLOTBOX *) plot - the structure describing
/              the current plotbox.
/  returns:    (void)
/
/ ----------------------------------------------------------------------- */
void create_plotbox(PLOTBOX * plot)
{
   set_plotbox_size(plot);

   scale_axis(&plot->x);
   scale_axis(&plot->y);
   scale_axis(&plot->z);

   draw_plotbox(plot);
}


PRIVATE ERR_OMA APlotCleanUp(CRECT OldClipRect, ERR_OMA err)
{
  endLinePlot();
  CSetClipRectangle(deviceHandle(), OldClipRect);
  return err;
}


/* -----------------------------------------------------------------------
/  function:   Plots a series of xy points at one time.  These are
/              supplied in an array of x values and an array of
/              y values, with a one-to-one correspondence between
/              x array and y array indices yielding the correct xy pair.
/              A count is supplied to indicate how many xy pairs
/              are in the arrays.
/
/  requires:   (PLOTBOX *) Plot - the current plotbox structure.
/              CURVEDIR *pCurveDir - data curve directory
/              SHORT EntryIndex - the curve entry number
/              SHORT CurveIndex - Curve number in entry
/              USHORT StartPoint - starting data point
/              USHORT Count - the number of xy pairs
/              FLOAT ZValue - the z coordinate (graph layer number)
/
/  returns:    ERROR_ALLOC_MEM - unsuccessful allocation to create an array of
/                         cartesian points, used to hold the x,y values
/                         for the GSS display device.
/              ERROR_NONE
/
/  side effects:  requires free memory on the heap (4 bytes for each
/              x,y pair).
/
/ ----------------------------------------------------------------------- */
ERR_OMA array_plot(PLOTBOX *Plot, CURVEDIR *pCurveDir,
                          SHORT EntryIndex, SHORT CurveIndex,
                          USHORT StartPoint, USHORT Count,
                          FLOAT ZValue)
{
  CRECT          ClipRect, OldClipRect;
  USHORT         i;
  FLOAT          x, y;
  double         temp;
  ERR_OMA err;
  LONG XOffset,  YOffset;
  FLOAT XBasePt, XFactor;
  FLOAT YBasePt, YFactor;
  CLINETYPE      SelType;
  CCOLOR         SelColor;
  UCHAR          curveHdr_XData_XUnits;
  SHORT          xVal, yVal, prefBuf = 0;
  GET_POINT_FUNC * get_point = WhichGetFunction();

  /* set clip rect */
  /* if CalcClip returns TRUE, Zvalue won't fit in rectangle */
  if (CalcClipRect(Plot, ZValue, &ClipRect))
    return ERROR_NONE;

  startLinePlot(Count);

  CInqClipRectangle(deviceHandle(), &OldClipRect);
  CSetClipRectangle(deviceHandle(), ClipRect);

  XBasePt = Plot->x.min_value;

  XFactor = Plot->x.inv_range * (float) Plot->x.axis_end_offset.x;

  YBasePt = Plot->y.min_value;

  YFactor = Plot->y.inv_range * (float) Plot->y.axis_end_offset.y;

  CalcOffsetForZ(Plot, ZValue, &XOffset, &YOffset);

  XOffset += Plot->x.axis_zero.x;
  YOffset += Plot->y.axis_zero.y;

  CSetLineType(deviceHandle(), Plot->plot_line_type, &SelType);
  if (!Plot->flags.loop_colors)
    CSetLineColor(deviceHandle(), Plot->plot_color, &SelColor);
  else
    {
    int color_index;
    if (Plot->flags.loop_colors == 1)
      color_index = ((int)ZValue+1) % 5 ? 0 : 6;
    else
      color_index = (int)ZValue % 7;
    CSetLineColor(deviceHandle(), LineColors[color_index], &SelColor);
    }

  if (Plot->x.units != COUNTS)
    {
    CURVEHDR temp_curve_header;

    ReadTempCurvehdr(pCurveDir, EntryIndex, CurveIndex, &temp_curve_header);
    curveHdr_XData_XUnits = temp_curve_header.XData.XUnits;
    }

  for (i = StartPoint; i < (StartPoint + Count); i++)
    {
    if (err = (*get_point)(pCurveDir, EntryIndex, CurveIndex,
               i, &x, &y, FLOATTYPE, &prefBuf))
      {
      return APlotCleanUp(OldClipRect, err);
      }

    if (Plot->x.units != COUNTS)
      {
      if (curveHdr_XData_XUnits == COUNTS)
        {
        x = (FLOAT) i;
        if (InitialMethod->CalibUnits[0] != COUNTS)
          {
          x = ApplyCalibrationToX(InitialMethod->CalibCoeff[0], x);
          err = ConvertUnits((UCHAR) Plot->x.units, &temp,
                       InitialMethod->CalibUnits[0], (double) x,
                       (double)InitialMethod->Excitation);
          if (err)
            return APlotCleanUp(OldClipRect, err);

          x = (float) temp; 
          }
        }
      else
        {
        err = ConvertUnits((UCHAR) Plot->x.units, &temp,
          curveHdr_XData_XUnits, (double) x,
          (double)InitialMethod->Excitation);

        if (err)
          return APlotCleanUp(OldClipRect, err);

        x = (float) temp;
        }
      }
    else
      x = (FLOAT) i;

    xVal = FactorPoint(x, XFactor, XBasePt, XOffset, Plot->x.ascending);
    yVal = FactorPoint(y, YFactor, YBasePt, YOffset, Plot->y.ascending);

    addPoint(xVal, yVal);  // another point to be plotted
    }

  return APlotCleanUp(OldClipRect, ERROR_NONE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ResizePlotForWindow(SHORT theWindow)
{
   SHORT      iDummy  ;
   AXISDATA * AxisDummy;
   PLOTBOX  * pPlot = & Plots[ WindowPlotAssignment[theWindow] ];

   pPlot->fullarea = plotWindowArea(theWindow);

   set_plotbox_size(pPlot);

   // if false color, make allowance for color key, plot area is smaller
   if(pPlot->style == FALSE_COLOR) {
      false_color_plotbox_adjust(theWindow, pPlot);
      set_plotbox_size(pPlot);
   }

   SizeAxis(pPlot, 'X', &AxisDummy, &iDummy);
   SizeAxis(pPlot, 'Y', &AxisDummy, &iDummy);

   if (pPlot->z_position)
      SizeAxis(pPlot, 'Z', &AxisDummy, &iDummy);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void UpdatePlotscreenString(SHORT Row, SHORT Column, PCHAR String)
  {
  CXY TextPt;
  CCOLOR SelColor;

  CSetATextColor(screen_handle, BLACK, &SelColor);
  CSetBgColor(screen_handle, BRT_YELLOW, &SelColor);

  TextPt.x = column_to_x(Column);
  TextPt.y = row_to_y(Row);
  CSetATextPosition(screen_handle, TextPt, &TextPt);
  CAText(screen_handle, String, &TextPt);
  }


