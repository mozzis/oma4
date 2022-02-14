/* -----------------------------------------------------------------------
/
/  plotbox.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/PLOTBOX.H 1     6/18/99 1:53a Maynard $
 * 
 *    Rev 1.0   07 Jan 1992 11:51:24   cole
 * Initial revision.
 * 
*/

#ifndef PLOTBOX_INCLUDED
#define PLOTBOX_INCLUDED

#ifndef CGIBIND_INCLUDED
   #define CGIBIND_INCLUDED
   #include <cgibind.h>
#endif

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

enum { LEGEND_SIZE = 30 };

struct axis_data
{
   char    legend[ LEGEND_SIZE ];  /* preallocated */
   CXY     axis_end_offset;
   CXY     axis_zero;
   float   max_value;
   float   min_value;
   float   original_max_value;
   float   original_min_value;
   float   inv_range;
   BOOLEAN ascending;
   USHORT  units;                   /* same as in methdr's */
};
  
/* typedef struct axis_data AXISDATA; */

// Add data structure for automatic peak labels.

struct curve_peak_label
{
   float curve_number;
   float threshold_value;
   BOOLEAN  enabled;
};

/* typedef struct curve_peak_label CURVE_PEAK_LABEL; */ 

// Store peak label info for at most 10 curves.
#define MAX_LABELLED_CURVES 10

struct plot_peak_label
{
   CURVE_PEAK_LABEL curve_peaks[ MAX_LABELLED_CURVES ];
   BOOLEAN label_peaks;  // label peaks iff TRUE
};

/* typedef struct plot_peak_label PLOT_PEAK_LABEL; */ 

// values for PLOTBOX.z_position
typedef enum { NOSIDE, RIGHTSIDE, LEFTSIDE } ZPosition;

typedef enum {OVERLAPPED_CURVES= 0, HIDDENLINE_CURVES, HIDDENSURF_CURVES,
              FALSE_COLOR } styles;

#define TITLE_SIZE 28

struct pplot_flags {
  unsigned int live_fast : 1;
  unsigned int loop_colors : 2;
} ;

typedef struct pplot_flags PPLOT_FLAGS;

struct plot_box
{
   PPLOT_FLAGS       flags;
   char              title[TITLE_SIZE+2];
   float             xscale;
   float             yscale;
   CRECT             fullarea;
   CRECT             plotarea;
   AXISDATA          x;
   AXISDATA          y;
   AXISDATA          z;
   int               xz_percent;
   int               yz_percent;
   ZPosition         z_position;
   int               background_color;
   CCOLOR            box_color;
   CCOLOR            text_color;
   CCOLOR            grid_color;
   CLINETYPE         grid_line_type;
   CCOLOR            plot_color;
   CLINETYPE         plot_line_type;
   enum styles       style;
   PLOT_PEAK_LABEL   plot_peak_labels;
   int               colorKeyWidth;
};

/* typedef struct plot_box PLOTBOX; */ 

// enum { OVERLAPPED_CURVES = 0, HIDDENLINE_CURVES, HIDDENSURF_CURVES, FALSE_COLOR };

#define MAXPLOTS 9
#define CAL_PLOTBOX              (MAXPLOTS - 1)

extern PLOTBOX far Plots[];          /* plot areas */

extern PLOTBOX * ActivePlot;

extern CRECT DisplayGraphArea;  /* screen graphing window */

#define NUM_LINE_COLORS 8
extern CCOLOR LineColors[];

int percent(int,int);

void scale_axis(AXISDATA*);

// return the margin size and the axis angle for the given plotbox and axis.
// margin is in VDC for label side margin.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void axisMarginAngle(PLOTBOX const * plot, char which_one, int * margin,
                      double * angle);

// uses InitialMethod for method header info
void InitializePlots(CRECT * GraphArea);

// copies to InitialMethod
void CopyPlotToMethod(void);

// functions implemented in plot.asm
int GssPosX(PLOTBOX *, FLOAT, int);
int GssPosY(PLOTBOX *, FLOAT, int);
int FactorPoint(float Value, float Factor, float BasePt, long Offset,
                 int ascends);
void CalcOffsetForZ(PLOTBOX *plot, FLOAT ZValue, PLONG pXOffset,
                     PLONG pYOffset);
BOOLEAN CalcClipRect(PLOTBOX *, FLOAT, CRECT *);
  
CXY gss_position(PLOTBOX*,float,float,float);

// set up axis min, max, magnitudes, exponent for original min and max
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void initAxisToOriginal(AXISDATA * axis);

// return an array of plotbox outline points and a point count for it
// offset moves the points towards the center (if > 0) or away from
// the center (if < 0). offset is a VDC value for adding/subtracting
// to the x and y coordinates of the outline[] points. Offset is in
// DEVICE COORDINATES (!!!), +1 means move one device pixel inside the
// outline.  outline[] is returned in VDC coordinates.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void plotboxOutline(PLOTBOX * plot, CXY outline[], short * pointCount,
                     int offset);

// draw outline of plotbox, including all 3 axes
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void drawPlotboxOutline(PLOTBOX * plotbox);

void set_plotbox_size(PLOTBOX *);

// Set the drawing sizes for the axis lines
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SizeAxis(PLOTBOX *plot, char which_one, AXISDATA * *axis,
               int *axis_length);

// Tell whether live curve drawing is "Fast" in this plotbox
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN LiveDrawFast(PLOTBOX * plot);
#endif  //  PLOTBOX_INCLUDED
