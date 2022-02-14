/* -----------------------------------------------------------------------
/
/  pltsetup.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/pltsetup.c_v   1.6   06 Jul 1992 10:35:50   maynard  $
/  $Log:   J:/logfiles/oma4000/main/pltsetup.c_v  $
/
*/

#include <string.h>

#include "pltsetup.h"
#include "ksindex.h"
#include "helpindx.h"
#include "cursor.h"
#include "curvdraw.h"
#include "device.h"
#include "doplot.h"
#include "multi.h"
#include "formtabs.h"
#include "calib.h"     // WaveLengthOptions[]
#include "omaform.h"   // COLORS_DEFAULT
#include "omameth.h"   // LPMETHDR
#include "omaerror.h"
#include "crvheadr.h"
#include "plotbox.h"
#include "tagcurve.h"  // ExpandedOnTagged
#include "forms.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

PRIVATE char   plot_title[TITLE_SIZE];
PRIVATE char X_axis_label[LEGEND_SIZE];
PRIVATE char Y_axis_label[LEGEND_SIZE];
PRIVATE char Z_axis_label[LEGEND_SIZE];

PRIVATE double min_X;
PRIVATE double max_X;
PRIVATE double min_Y;
PRIVATE double max_Y;
PRIVATE double min_Z;
PRIVATE double max_Z;

PRIVATE int X_units;
PRIVATE int Y_units;
PRIVATE int Z_units;
PRIVATE int z_orientation; /* LEFTSIDE, RIGHTSIDE, or NOSIDE */
PRIVATE int z_percent_x;
PRIVATE int z_percent_y;
PRIVATE int plot_method;
PRIVATE unsigned int live_mode;

int autoscale_x; 
int autoscale_y; 
int autoscale_z;
int window_style;

PRIVATE int vary_line_color;
PRIVATE int vary_line_type;
PRIVATE int zero_tick_mark;
PRIVATE int connect_the_dots;

PRIVATE int WindowSelect = 0;
PRIVATE int plot_label_peaks;  // TRUE/FALSE   0 or 1.

PRIVATE SHORT OldActiveWindow = 0;

PRIVATE int PlotSetupExit(int Dummy);
PRIVATE int VerifyWinLayout(void);
PRIVATE int VerifyActivePlot(void);
PRIVATE BOOLEAN PlotSetupFormInit(void);

PRIVATE DATA DO_String_Reg[] = {

/* 0  */ { "PeakLabel", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 1  */ { "X", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 2  */ { "Y", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 3  */ { "Z", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 4  */ { "Window", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
// /* 5  */ { "Active Setup", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 5  */ { "Active Window", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 6  */ { "Units", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 7  */ { "Plot Title", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 8  */ { "Label", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 9  */ { "Minimum", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 10 */ { "Maximum", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 11 */ { "X Axis", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 12 */ { "Y Axis", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 13 */ { "Z Axis", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 14 */ { "Orientation", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 15 */ { "% of X", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 16 */ { "% of Y", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 17 */ { "Plot Format", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 18 */ { "Autoscale", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 19 */ { "Graph Window Layout", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 20 */ { "Draw Live: ", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 21 */ { "Line Color: ", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
};

PRIVATE char * plot_type_options[] = {
   "Overlapped",
   "Hidden Line",
   "Hidden Surface",
   "False Color",
   "Contour",
   "Mesh",
};

PRIVATE char * IntensityOptions[] = {"Intensity"};

PRIVATE char * WindowStyleOptions[] = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"
};

PRIVATE char * yesno_options[] = { "No", "Yes" };

PRIVATE char * z_orient_options[] = { "No Z Axis", "Right", "Left" };

PRIVATE char * live_plot_options[] = {
   "As Fast As Possible",
   "Wait for New Data",
};

PRIVATE char * plot_color_options[] = {
   "All lines White",
   "Mark 5th line",
   "Change Line Colors",
};

PRIVATE DATA pltTOGGLES_Reg[] = {

/* 0 */ { plot_type_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
/* 1 */ { IntensityOptions, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
/* 2 */ { WindowStyleOptions, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
/* 3 */ { WaveLengthOptions, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
/* 4 */ { yesno_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
/* 5 */ { z_orient_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
/* 6 */ { live_plot_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
/* 6 */ { plot_color_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
};

PRIVATE EXEC_DATA pltCODE_Reg[] = {

/* 0 */ { VerifyWinLayout, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 1 */ { VerifyActivePlot, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 2 */ { PlotSetupFormInit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 3 */ { CAST_CHR2INT PlotSetupExit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },

};

PRIVATE DATA DATA_Reg[] = {
/* 0  */ { plot_title, 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 1  */ { X_axis_label, 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 2  */ { &min_X, 0, DATATYP_DOUBLE_FLOAT, DATAATTR_PTR, 0 },
/* 3  */ { &max_X, 0, DATATYP_DOUBLE_FLOAT, DATAATTR_PTR, 0 },
/* 4  */ { Y_axis_label, 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 5  */ { &min_Y, 0, DATATYP_DOUBLE_FLOAT, DATAATTR_PTR, 0 },
/* 6  */ { &max_Y, 0, DATATYP_DOUBLE_FLOAT, DATAATTR_PTR, 0 },
/* 7  */ { Z_axis_label, 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 8  */ { &min_Z, 0, DATATYP_DOUBLE_FLOAT, DATAATTR_PTR, 0 },
/* 9  */ { &max_Z, 0, DATATYP_DOUBLE_FLOAT, DATAATTR_PTR, 0 },
/* 10 */ { &z_orientation, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 11 */ { &z_percent_x, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 12 */ { &z_percent_y, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 13 */ { &plot_method, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 14 */ { &WindowSelect, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 15 */ { &autoscale_x, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 16 */ { &live_mode, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 17 */ { &vary_line_color, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 18 */ { &window_style, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 19 not used */ { &zero_tick_mark, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 20 not used */ { &connect_the_dots, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 21 */ { &X_units, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 22 */ { &Y_units, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 23 */ { &Z_units, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 24 */ { NULL, /* &ActivePlotIndex, */ 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 25 */ { &plot_label_peaks, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 26 */ { &autoscale_y, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 27 */ { &autoscale_z, 0, DATATYP_INT, DATAATTR_PTR, 0 },

};

enum { DGROUP_DO_STRINGS = 1, DGROUP_TOGGLES, DGROUP_CODE, DGROUP_PLOT };

  enum { LAB_PTITLE,
         LAB_ACWIN,
         LAB_LABEL,
         LAB_MINI,
         LAB_MAXI,
         LAB_UNITS,
         LAB_XAXIS,
         LAB_YAXIS,
         LAB_ZAXIS,
         LAB_ORIENT,
         LAB_PERCX,
         LAB_PERCY,
         LAB_PFORM,
         LAB_LAYOUT,
         LAB_ASCALE,
         LAB_ASCALEX,
         LAB_ASCALEY,
         LAB_ASCALEZ,
         LAB_PEAKLAB,
         LAB_LIVEMODE,
         LAB_PCOLOR,

         F_PTITLE,
         F_WACTIV,
         F_XLABEL,
         F_XMIN,
         F_XMAX,
         F_XUNIT,
         F_YLABEL,
         F_YMIN,
         F_YMAX,
         F_YUNIT,
         F_ZLABEL,
         F_ZMIN,
         F_ZMAX,
         F_ZUNIT,
         F_ZORIENT,
         F_ZPERCX,
         F_ZPERCY,
         F_PMETH,
         F_PAUTOX,
         F_PAUTOY,
         F_PAUTOZ,
         F_WSTYL,
         F_PKLABEL,
         F_LPMODE,
         F_PCOLOR,
        };

PRIVATE FIELD PlotSetupFormFields[] = {

   // plot title
  label_field(LAB_PTITLE,
      DGROUP_DO_STRINGS, 7,
      1, 2, 10),

   /* active window */
  label_field(LAB_ACWIN,
      DGROUP_DO_STRINGS, 5,
      1, 43, 13),

   /* label */
  label_field(LAB_LABEL,
      DGROUP_DO_STRINGS, 8,
      3, 17, 5),

   /* minimum */
  label_field(LAB_MINI,
      DGROUP_DO_STRINGS, 9,
      3, 37, 7),

   /* maximum */
  label_field(LAB_MAXI,
      DGROUP_DO_STRINGS, 10,
      3, 48, 7),

   /* units */
  label_field(LAB_UNITS,
      DGROUP_DO_STRINGS, 6,
      3, 59, 5),

   /* X axis */
  label_field(LAB_XAXIS,
      DGROUP_DO_STRINGS, 11,
      4, 2, 6),

   /* Y axis */
  label_field(LAB_YAXIS,
      DGROUP_DO_STRINGS, 12,
      6, 2, 6),

   /* Z axis */
  label_field(LAB_ZAXIS,
      DGROUP_DO_STRINGS, 13,
      8, 2, 6),

   /* Orientation */
  label_field(LAB_ORIENT,
      DGROUP_DO_STRINGS, 14,
      9, 13, 11),

   /* percent of X */
  label_field(LAB_PERCX,
      DGROUP_DO_STRINGS, 15,
      9, 39, 6),

   /* percent of Y */
  label_field(LAB_PERCY,
      DGROUP_DO_STRINGS, 16,
      9, 50, 6),

   /* plot format */
  label_field(LAB_PFORM,
      DGROUP_DO_STRINGS, 17,
      11, 2, 11),

   /* graph window layout */
  label_field(LAB_LAYOUT,
      DGROUP_DO_STRINGS, 19,
      13, 2, 19),

   /* autoscale*/
  label_field(LAB_ASCALE,
      DGROUP_DO_STRINGS, 18,
      11, 37, 9),

   /* autoscale X*/
  label_field(LAB_ASCALEX,
      DGROUP_DO_STRINGS, 1,
      10, 48, 1), 

   /* autoscale Y */
  label_field(LAB_ASCALEY,
      DGROUP_DO_STRINGS, 2,
      10, 53, 1),

   /* autoscale Z */
  label_field(LAB_ASCALEZ,
      DGROUP_DO_STRINGS, 3,
      10, 58, 1),

   // peak label enable
  label_field(LAB_PEAKLAB,
      DGROUP_DO_STRINGS, 0,
      13, 37, 9),

   // live plot mode
  label_field(LAB_LIVEMODE,
      DGROUP_DO_STRINGS, 20,
      14, 37, 11),

   // line plot color
  label_field(LAB_PCOLOR,
      DGROUP_DO_STRINGS, 21,
      15, 36, 12),

   /* plot title */
   field_set(F_PTITLE,
     FLDTYP_STRING,
     FLDATTR_REV_VID,
     KSI_PLOT_TITLE,
     PLOTSETUP_HBASE + 0,
     DGROUP_PLOT, 0,
     0, 0,
     0, 0,
     25, 0,                                   // 20
     1, 14, 25,
     EXIT, F_PTITLE, F_WSTYL, F_XLABEL,
     F_PTITLE, F_WACTIV, F_LPMODE, F_WACTIV),

   /* Active window toggle */
   field_set(F_WACTIV,
     FLDTYP_TOGGLE,
     FLDATTR_REV_VID,
     KSI_ACTIVE_WINDOW,
     PLOTSETUP_HBASE + 28,
     DGROUP_PLOT, 14,           // WindowSelect
     DGROUP_TOGGLES, 2 ,        // WindowStyleOptions
     DGROUP_CODE, 1 ,           // VerifyActivePlot
     0, 1,                                                 // 22
     1, 57, 1,
     EXIT, F_WACTIV, F_LPMODE, F_XLABEL,
     F_PTITLE, F_WACTIV, F_PTITLE, F_XLABEL),

   /* X axis label */
   field_set(F_XLABEL,
     FLDTYP_STRING,
     FLDATTR_REV_VID,
     KSI_X_LABEL,
     PLOTSETUP_HBASE + 1,
     DGROUP_PLOT, 1,
     0, 0,
     0, 0,
     25, 0,            // 23
     4, 10, 24,
     EXIT, F_XLABEL, F_PTITLE, F_YLABEL,
     F_XLABEL, F_XUNIT, F_WACTIV, F_XMIN),

   /* min X */
   field_set(F_XMIN,
     FLDTYP_STD_FLOAT,
     FLDATTR_REV_VID,
     KSI_X_MIN,
     PLOTSETUP_HBASE + 2,
     DGROUP_PLOT, 2,
     0, 0,
     0, 0,
     4, 0,                                  // 24
     4, 36, 9,
     EXIT, F_XMIN, F_PTITLE, F_YMIN,
     F_XLABEL, 2, F_XLABEL, F_XMAX),

   /* max X */
   field_set(F_XMAX,
     FLDTYP_STD_FLOAT,
     FLDATTR_REV_VID,
     KSI_X_MAX,
     PLOTSETUP_HBASE + 3,
     DGROUP_PLOT, 3,
     0, 0,
     0, 0,
     4, 0,                                   // 25
     4, 47, 9,
     EXIT, F_XMAX, F_PTITLE, F_YMAX,
     F_XMIN, F_XUNIT, F_XMIN, F_XUNIT),

   /* X units */
   field_set(F_XUNIT,
     FLDTYP_TOGGLE,
     FLDATTR_REV_VID,
     KSI_X_UNITS,
     PLOTSETUP_HBASE + 4,
     DGROUP_PLOT, 21,
     DGROUP_TOGGLES, 3 ,       // WaveLengthOptions
     0, 0,
     0, 15,                                          // 26
     4, 58, 12,
     EXIT, F_XUNIT, F_PTITLE, F_YUNIT,
     F_XMAX, F_XUNIT, F_XMAX, F_YLABEL),

   /* Y axis label */
   field_set(F_YLABEL,
     FLDTYP_STRING,
     FLDATTR_REV_VID,
     KSI_Y_LABEL,
     PLOTSETUP_HBASE + 17,
     DGROUP_PLOT, 4,
     0, 0,
     0, 0,
     25, 0,                                  // 27
     6, 10, 24,
     EXIT, F_YLABEL, F_XLABEL, F_ZLABEL,
     F_YLABEL, F_YMIN, F_XUNIT, F_YMIN),

   /* min Y */
   field_set(F_YMIN,
     FLDTYP_STD_FLOAT,
     FLDATTR_REV_VID,
     KSI_Y_MIN,
     PLOTSETUP_HBASE + 18,
     DGROUP_PLOT, 5,
     0, 0,
     0, 0,
     4, 0,                                    // 28
     6, 36, 9,
     EXIT, F_YMIN, F_XMIN, F_ZMIN,
     F_YLABEL, F_YMAX, F_YLABEL, F_YMAX),

   /* max Y */
   field_set(F_YMAX,
     FLDTYP_STD_FLOAT,
     FLDATTR_REV_VID,
     KSI_Y_MAX,
     PLOTSETUP_HBASE + 19,
     DGROUP_PLOT, 6,
     0, 0,
     0, 0,
     4, 0,                                   // 29
     6, 47, 9,
     EXIT, F_YMAX, F_XMAX, F_ZMAX,
     F_YMIN, F_YUNIT, F_YMIN, F_YUNIT),

   /* Y units */
   field_set(F_YUNIT,
     FLDTYP_TOGGLE,
     FLDATTR_REV_VID,
     KSI_Y_UNITS,
     PLOTSETUP_HBASE + 20,
     DGROUP_PLOT, 22,
     DGROUP_TOGGLES, 1 ,     // IntensityOptions
     0, 0,
     0, 1,                                         // 30
     6, 58, 12,
     EXIT, F_YUNIT, F_XUNIT, F_ZUNIT,
     F_YMAX, F_YUNIT, F_YMAX, F_ZLABEL),

   /* Z axis label */
   field_set(F_ZLABEL,
     FLDTYP_STRING,
     FLDATTR_REV_VID,
     KSI_Z_LABEL,
     PLOTSETUP_HBASE + 21,
     DGROUP_PLOT, 7,
     0, 0,
     0, 0,
     25, 0,                                 // 31
     8, 10, 24,
     EXIT, F_ZLABEL, F_YLABEL, F_ZORIENT,
     F_ZLABEL, F_ZMIN, F_YUNIT, F_ZMIN),

   /* min Z */
   field_set(F_ZMIN,
     FLDTYP_STD_FLOAT,
     FLDATTR_REV_VID,
     KSI_Z_MIN,
     PLOTSETUP_HBASE + 22,
     DGROUP_PLOT, 8,
     0, 0,
     0, 0,
     4, 0,                                   // 32
     8, 36, 9,
     EXIT, F_ZMIN, F_YMIN, F_ZPERCX,
     F_ZLABEL, F_ZMAX, F_ZLABEL, F_ZMAX),

   /* max Z */
   field_set(F_ZMAX,
     FLDTYP_STD_FLOAT,
     FLDATTR_REV_VID,
     KSI_Z_MAX,
     PLOTSETUP_HBASE + 23,
     DGROUP_PLOT, 9,
     0, 0,
     0, 0,
     4, 0,                                   // 33
     8, 47, 9,
     EXIT, F_ZMAX, F_YMAX, F_ZPERCY,
     F_ZMIN, F_ZUNIT, F_ZMIN, F_ZUNIT),

   /* Z units */
   field_set(F_ZUNIT,
     FLDTYP_TOGGLE,
     FLDATTR_REV_VID,
     KSI_Z_UNITS,
     PLOTSETUP_HBASE + 24,
     DGROUP_PLOT, 23,
     DGROUP_TOGGLES, 3 ,     // WaveLengthOptions
     0, 0,
     0, 1,                                   // 34
     8, 58, 12,
     EXIT, F_ZUNIT, F_YUNIT, F_ZORIENT,
     F_ZMAX, F_ZUNIT, F_ZMAX, F_ZORIENT),

   /* Z orientation */
   field_set(F_ZORIENT,
     FLDTYP_TOGGLE,
     FLDATTR_REV_VID,
     KSI_Z_SIDE_OPTS,
     PLOTSETUP_HBASE + 5,
     DGROUP_PLOT, 10,
     DGROUP_TOGGLES, 5 ,     // z_orient_options
     0, 0,
     0, 3,                                  // 35
     9, 25, 9,
     EXIT, F_ZORIENT, F_ZLABEL, F_PMETH,
     F_ZORIENT, F_ZPERCX, F_ZUNIT, F_ZPERCX),

   /* Z percent X */
   field_set(F_ZPERCX,
     FLDTYP_UNS_INT,
     FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
     KSI_Z_PER_X,
     PLOTSETUP_HBASE + 6,
     DGROUP_PLOT, 11,
     0, 0,
     0, 0,
     0, 0,                                 // 36
     9, 36, 3,
     EXIT, F_ZPERCX, F_ZMIN, F_PAUTOX,
     F_ZORIENT, F_ZPERCY, F_ZORIENT, F_ZPERCY),

   /* Z percent Y */
   field_set(F_ZPERCY,
     FLDTYP_UNS_INT,
     FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
     KSI_Z_PER_Y,
     PLOTSETUP_HBASE + 7,
     DGROUP_PLOT, 12,
     0, 0,
     0, 0,
     0, 0,                     // 37
     9, 47, 3,
     EXIT, F_ZPERCY, F_ZMAX, F_PAUTOX,
     -2, F_ZPERCY, F_ZPERCX, F_PMETH),

   /* plot_method */
   field_set(F_PMETH,
     FLDTYP_TOGGLE,
     FLDATTR_REV_VID,
     KSI_PLOT_METHOD,
     PLOTSETUP_HBASE + 8,
     DGROUP_PLOT, 13,
     DGROUP_TOGGLES, 0,       // plot_type_options
     0, 0,
     0, 4,                     // 38
     11, 14, 14,
     EXIT, F_PMETH, F_ZORIENT, F_WSTYL,
     F_PMETH, F_PAUTOZ, F_ZPERCY, F_PAUTOX),

   /* auto scale x*/
   field_set(F_PAUTOX,
     FLDTYP_TOGGLE,
     FLDATTR_REV_VID,
     KSI_AUTOSCALE_X,
     PLOTSETUP_HBASE + 11,
     DGROUP_PLOT, 15,
     DGROUP_TOGGLES, 4 ,     // yesno_options
     0, 0,
     0, 2,                     // 39
     11, 47, 3,
     EXIT, F_PAUTOX, F_ZPERCY, F_PKLABEL,
     F_PMETH, F_PAUTOY, F_PMETH, F_PAUTOY),

   /* auto scale y*/
   field_set(F_PAUTOY,
     FLDTYP_TOGGLE,
     FLDATTR_REV_VID,
     KSI_AUTOSCALE_Y,
     PLOTSETUP_HBASE + 26,
     DGROUP_PLOT, 26,
     DGROUP_TOGGLES, 4 ,     // yesno_options
     0, 0,
     0, 2,                     // 40
     11, 52, 3,
     EXIT, F_PAUTOY, F_ZMAX, F_PKLABEL,
     F_PAUTOX, F_PAUTOZ, F_PAUTOX, F_PAUTOZ),

   /* auto scale z*/
   field_set(F_PAUTOZ,
     FLDTYP_TOGGLE,
     FLDATTR_REV_VID,
     KSI_AUTOSCALE_Z,
     PLOTSETUP_HBASE + 27,
     DGROUP_PLOT, 27,
     DGROUP_TOGGLES, 4 ,     // yesno_options
     0, 0,
     0, 2,                     // 41
     11, 57, 3,
     EXIT, F_PAUTOZ, F_ZUNIT, F_PKLABEL,
     F_PAUTOY, F_PAUTOZ, F_PAUTOY, F_WSTYL),

   /* window style */
   field_set(F_WSTYL,
     FLDTYP_TOGGLE,
     FLDATTR_REV_VID,
     KSI_WINDOW_STYLE,
     PLOTSETUP_HBASE + 9,
     DGROUP_PLOT, 18,
     DGROUP_TOGGLES, 2 ,   // WindowStyleOptions
     DGROUP_CODE, 0 ,      // VerifyWinLayout
     0, 10,                    // 42
     13, 22, 2,
     EXIT, F_WSTYL, F_PMETH, F_PTITLE,
     F_WSTYL, F_PKLABEL, F_PAUTOZ, F_PKLABEL),

   /*peak label */ 
   field_set(F_PKLABEL,
     FLDTYP_TOGGLE,
     FLDATTR_REV_VID,
     KSI_PK_LABEL,             // 43
     PLOTSETUP_HBASE + 25,
     DGROUP_PLOT, 25,         // plot_label_peaks,  BOOLEAN
     DGROUP_TOGGLES, 4,       // yesno_options
     0, 0,
     0, 2,
     13, 49, 3,
     EXIT, F_PKLABEL, F_PAUTOX, F_LPMODE,
     F_WSTYL, F_PKLABEL, F_WSTYL, F_LPMODE),   
    
    /* live plot mode */
   field_set(F_LPMODE,
     FLDTYP_TOGGLE,
     FLDATTR_REV_VID,
     KSI_LPLOT_MODE,
     PLOTSETUP_HBASE + 29,
     DGROUP_PLOT, 16,
     DGROUP_TOGGLES, 6,
     0, 0,
     0, 2,
     14, 49, 19,
     EXIT, F_LPMODE, F_PKLABEL, F_PCOLOR,
     F_WSTYL, F_LPMODE, F_PKLABEL, F_PCOLOR),
    
    /* line plot color mode */
   field_set(F_PCOLOR,
     FLDTYP_TOGGLE,
     FLDATTR_REV_VID,
     KSI_PCOLOR_MODE, 
     PLOTSETUP_HBASE + 30,
     DGROUP_PLOT, 17,
     DGROUP_TOGGLES, 7,
     0, 0,
     0, 3,
     15, 49, 19,
     EXIT, F_PCOLOR, F_LPMODE, F_WACTIV,
     F_WSTYL, F_PCOLOR, F_LPMODE, F_PTITLE)   
};     

PRIVATE FORM  PlotSetupForm = {
   0, 0, FORMATTR_BORDER | FORMATTR_VISIBLE | FORMATTR_FULLSCREEN,
   0, 0, 0, 2, 0, 21, 80, 0, 0,
   { DGROUP_CODE, 2 },    // PlotSetupFormInit
   { DGROUP_CODE, 3 },    // PlotSetupExit
   COLORS_DEFAULT, 0, 0, 0, 0,
   sizeof(PlotSetupFormFields) / sizeof(PlotSetupFormFields[ 0 ]),
   PlotSetupFormFields, KSI_PLOT_FORM,
   0, DO_String_Reg, pltTOGGLES_Reg, (DATA *)pltCODE_Reg, DATA_Reg, 0 };


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int PlotSetupExit(int Dummy)
{
  PLOTBOX * pPlotBox;                  /* !!! */
  BOOLEAN false_color_change;          /* !!! */

  pPlotBox = &Plots[ActiveWindow];      /* !!! */
  false_color_change =
    ((pPlotBox->style == FALSE_COLOR) != (plot_method == FALSE_COLOR));

  // if going into false color, modify the z-axis max value so that the
  // last curve will show on the false color plot
  if(false_color_change)
    {
    if(plot_method == FALSE_COLOR) // going into false color
      {
      if(max_Z >= min_Z)
        max_Z ++;
      else
        min_Z ++;
      }
    else         // coming out of false color, restore the z-axis max value
      if (max_Y >= min_Y)
      max_Y --; // axis swap is still in effect !!
    else
      min_Y --;
    }

  InitPlotBox(pPlotBox);

  // if entering or leaving false color plot mode, swap Y and Z axes.
  if(false_color_change)
    {
    AXISDATA temp = pPlotBox->z;

    pPlotBox->z = pPlotBox->y;
    pPlotBox->y = temp;
    }

  PutUpPlotBoxes();                  
  return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int SetFormFromPlotBox(int Dummy)   
{
  PLOTBOX *pPlotBox;
  SHORT i;

  WindowSelect = ActiveWindow;

  pPlotBox = &Plots[ActiveWindow];

  live_mode = (pPlotBox->flags.live_fast != 0);
  vary_line_color = pPlotBox->flags.loop_colors;
  min_X = pPlotBox->x.min_value;
  max_X = pPlotBox->x.max_value;
  min_Y = pPlotBox->y.min_value;
  max_Y = pPlotBox->y.max_value;
  min_Z = pPlotBox->z.min_value;
  max_Z = pPlotBox->z.max_value;
  strcpy(plot_title, pPlotBox->title);
  strcpy(X_axis_label, pPlotBox->x.legend);
  strcpy(Y_axis_label, pPlotBox->y.legend);
  strcpy(Z_axis_label, pPlotBox->z.legend);

  /* change so X can be calibrated in time */
//  for (i=0; i<WLEN_UNITNUM; i++)
  for (i=0; i < TIME_DIS_UNITNUM; i++)
    {
    if (WaveLengthUnitTable[i] == (UCHAR) pPlotBox->x.units)
      {
      X_units = i;
      break;
      }
    }

  Y_units = COUNTS;

  for (i=0; i < TIME_DIS_UNITNUM; i++)
    {
    if (WaveLengthUnitTable[i] == (UCHAR) pPlotBox->z.units)
      {
      Z_units = i;
      break;
      }
    }

  z_percent_x = pPlotBox->xz_percent;
  z_percent_y = pPlotBox->yz_percent;
  z_orientation = pPlotBox->z_position;

  plot_method = pPlotBox->style;

  plot_label_peaks = pPlotBox -> plot_peak_labels.label_peaks;

  PlotSetupFormFields[F_WACTIV].specific.tglfld.total_items =
         (UCHAR) fullscreen_count[window_style];

  return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN PlotSetupFormInit()
{
 
  OldActiveWindow = ActiveWindow;
  SetFormFromPlotBox(0);

  return FALSE;
}


void InitPlotColors(PLOTBOX * pPlotBox)
{
  pPlotBox->background_color = BLACK;
  pPlotBox->box_color = BRT_BLUE;
  pPlotBox->text_color = BRT_WHITE;
  pPlotBox->grid_color = RED;
  pPlotBox->grid_line_type = CLN_Dotted;
  pPlotBox->plot_color = BRT_WHITE;
  pPlotBox->plot_line_type = CLN_Solid;
}


/* -----------------------------------------------------------------------
/
/  void InitPlotBox(PLOTBOX *pPlotBox)
/
/  requires:
/            pPlotBox - pointer to active plotbox structure
/  returns:    (void)
/
/ ----------------------------------------------------------------------- */
void InitPlotBox(PLOTBOX * pPlotBox)
{
  static UCHAR IntensityUnitTable[] = { COUNTS };

  if (! ExpandedOnTagged)
    multiplot_setup(PlotWindows, &DisplayGraphArea, window_style);
  else  // Window style 0 plots one graph at full screen
    multiplot_setup(&PlotWindows[ActiveWindow], &DisplayGraphArea, 0);

  pPlotBox->flags.live_fast = live_mode;
  pPlotBox->flags.loop_colors = vary_line_color;
  pPlotBox->x.min_value = pPlotBox->x.original_min_value = (FLOAT) min_X;
  pPlotBox->x.max_value = pPlotBox->x.original_max_value = (FLOAT) max_X;
  if (pPlotBox->x.min_value > pPlotBox->x.max_value)
    pPlotBox->x.ascending = FALSE;
  else
    pPlotBox->x.ascending = TRUE;

  pPlotBox->y.min_value = pPlotBox->y.original_min_value = (FLOAT) min_Y;
  pPlotBox->y.max_value = pPlotBox->y.original_max_value = (FLOAT) max_Y;
  if (pPlotBox->y.min_value > pPlotBox->y.max_value)
    pPlotBox->y.ascending = FALSE;
  else
    pPlotBox->y.ascending = TRUE;

  pPlotBox->z.min_value = pPlotBox->z.original_min_value = (FLOAT) min_Z;
  pPlotBox->z.max_value = pPlotBox->z.original_max_value = (FLOAT) max_Z;
  if (pPlotBox->z.min_value > pPlotBox->z.max_value)
    pPlotBox->z.ascending = FALSE;
  else
    pPlotBox->z.ascending = TRUE;

  InitPlotColors(pPlotBox);

  strcpy(pPlotBox->title, plot_title);
  strcpy(pPlotBox->x.legend, X_axis_label);
  strcpy(pPlotBox->y.legend, Y_axis_label);
  strcpy(pPlotBox->z.legend, Z_axis_label);

  pPlotBox->x.units = WaveLengthUnitTable[X_units];
  pPlotBox->y.units = IntensityUnitTable[Y_units];
  pPlotBox->z.units = WaveLengthUnitTable[Z_units];

  pPlotBox->xz_percent = z_percent_x;
  pPlotBox->yz_percent = z_percent_y;
  pPlotBox->z_position = z_orientation;

  pPlotBox->style = plot_method;
  pPlotBox->plot_peak_labels.label_peaks = plot_label_peaks;

  scale_axis(&(pPlotBox->x));
  scale_axis(&(pPlotBox->y));
  scale_axis(&(pPlotBox->z));
}

// always uses InitialMethod
// -----------------------------------------------------------------------
void InitializePlotSetupFields(CRECT *GraphArea)
{
  SHORT i;
  LPMETHDR pMethdr = InitialMethod;

  InitializePlots(GraphArea);

  live_mode = (ActivePlot->flags.live_fast != 0);
  vary_line_color = ActivePlot->flags.loop_colors;

  strncpy(plot_title, ActivePlot->title, TITLE_SIZE);
  strncpy(X_axis_label, ActivePlot->x.legend, LEGEND_SIZE);
  strncpy(Y_axis_label, ActivePlot->y.legend, LEGEND_SIZE);
  strncpy(Z_axis_label, ActivePlot->z.legend, LEGEND_SIZE);

  min_X = ActivePlot->x.original_min_value;
  min_Y = ActivePlot->y.original_min_value;
  min_Z = ActivePlot->z.original_min_value;

  max_X = ActivePlot->x.original_max_value;
  max_Y = ActivePlot->y.original_max_value;
  max_Z = ActivePlot->z.original_max_value;

  z_orientation = ActivePlot->z_position;
  z_percent_x = ActivePlot->xz_percent;
  z_percent_y = ActivePlot->yz_percent;

  /* change so X can be calibrated in time */
//  for (i=0; i<WLEN_UNITNUM; i++)
  for (i=0; i < TIME_DIS_UNITNUM; i++)
    {
    if (WaveLengthUnitTable[i] == (UCHAR) ActivePlot->x.units)
      {
      X_units = i;
      break;
      }
    }

  Y_units = 0;

  for (i=0; i < TIME_DIS_UNITNUM; i++)
    {
    if (WaveLengthUnitTable[i] == (UCHAR) ActivePlot->z.units)
      {
      Z_units = i;
      break;
      }
    }

  plot_method = ActivePlot->style;
  autoscale_x = pMethdr->AutoScaleX;
  autoscale_y = pMethdr->AutoScaleY;
  autoscale_z = pMethdr->AutoScaleZ;
  vary_line_color = FALSE;
  vary_line_type = FALSE;

  window_style = pMethdr->PlotWindowIndex;
  PlotSetupFormFields[F_WACTIV].specific.tglfld.total_items =
    (UCHAR) fullscreen_count[window_style];
  zero_tick_mark = FALSE;
  connect_the_dots = TRUE;
  plot_label_peaks = ActivePlot->plot_peak_labels.label_peaks;
}

//-------------------------------------------------------------------------
//  function: make sure that the active window is available in this setup.
//  requires:   (void)
//  returns:    (void)
//  side effects:
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int VerifyWinLayout(void)
{
   if (ActiveWindow >= fullscreen_count[window_style])   
   {
      error(ERROR_WINDOW_NOT_VISIBLE, ActiveWindow);
      return 0;
   }
   else
   {
      // change the number of allowed windows
      PlotSetupFormFields[F_WACTIV].specific.tglfld.total_items =
         (UCHAR) fullscreen_count[window_style];
      return FIELD_VALIDATE_SUCCESS;
   }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int VerifyActivePlot(void)
{
   int OldIndex;

    // Allow changeable window select
   ActiveWindow = WindowSelect;     /* !!! */

   ActivePlot = &Plots[ActiveWindow];
   InitPlotBox(&Plots[OldActiveWindow]); /* ??? Old ??? */
   SetFormFromPlotBox(0);
   OldIndex = Current.Form->field_index;
   draw_form();
   Current.Form->field_index = OldIndex;
   format_and_display_field(FALSE);
   OldActiveWindow = ActiveWindow;

   return(FIELD_VALIDATE_SUCCESS);
}

// Assume min and max values for each plotbox axis have been set up.  Update
// the necessary data structures to accomplish an autoscale.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void scalePlotbox(PLOTBOX * pPlotBox)
{
   BOOLEAN falseColor = (pPlotBox->style == FALSE_COLOR);

   if (autoscale_x)                 
   {
      scale_axis(&(pPlotBox->x));
      min_X = pPlotBox->x.min_value;
      max_X = pPlotBox->x.max_value;
   }

   if (autoscale_y)
      if(! falseColor)
        {
         scale_axis(&(pPlotBox->y));
         min_Y = pPlotBox->y.min_value;
         max_Y = pPlotBox->y.max_value;
      }
      else
        {
         scale_axis(&(pPlotBox->z));
         min_Y = pPlotBox->z.min_value;
         max_Y = pPlotBox->z.max_value;
      }

   if (autoscale_z)
      if(! falseColor)
        {
         scale_axis(&(pPlotBox->z));
         min_Z = pPlotBox->z.min_value;
         max_Z = pPlotBox->z.max_value;
      }
      else
        {
         scale_axis(&(pPlotBox->y));
         min_Z = pPlotBox->y.min_value;
         max_Z = pPlotBox->y.max_value;
      }
}

// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void registerPlotSetupForm(void)
{
   FormTable[KSI_PLOT_FORM] = &PlotSetupForm;
}
