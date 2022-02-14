/* -----------------------------------------------------------------------
/
/  BASLNSUB.C
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/baslnsub.c_v   0.21   06 Jul 1992 10:24:24   maynard  $
/  $Log:   J:/logfiles/oma4000/main/baslnsub.c_v  $
/
*/

#include <string.h>       // strupr(), strcpy()

#include "baslnsub.h"
#include "tempdata.h"
#include "points.h"
#include "doplot.h"       // create_plotbox()
#include "fkeyfunc.h"     // FKeyItems[]
#include "graphops.h"     // GraphOps()
#include "spline-3.h"     // point_interp(), etc
#include "helpindx.h"     // BASLNSUB_HBASE,  help file index
#include "cursor.h"       // ActiveWindow, CursorStatus -- globals
#include "lineplot.h"     // startLinePlot()
#include "curvdraw.h"     // Replot()
#include "device.h"
#include "omaform.h"      // GraphWindow
#include "di_util.h"      // ParsePathAndName()
#include "curvedir.h"     // SearchNextNamePath()
#include "omazoom.h"      // cursor_loc
#include "ksindex.h"    
#include "macrecor.h"   
#include "multi.h"      
#include "plotbox.h"
#include "syserror.h"     // ERROR_ALLOC_MEM
#include "omaerror.h"
#include "formtabs.h"
#include "crventry.h"
#include "crvheadr.h"
#include "forms.h"
#include "splitfrm.h"

#ifdef STATIC
   #define PRIVATE static
#else
   #define PRIVATE
#endif

#define KNOT_MARKER_HEIGHT 400     // number is from testing on VGA -
                                   // actually selects 356

#define SCROLL_FORM_ACTIVE_FIELD 5  // focusable field in scroll form

// current values define the curve to act upon or plot
PRIVATE SHORT EntryIndex;
PRIVATE SHORT CurveIndex;

PRIVATE SHORT OldActiveWindow;
PRIVATE CRECT OldGraphArea;

// TRUE iff in baseline subtract mode
PRIVATE BOOLEAN baseline_subtract_mode = FALSE;

// Use MAXPLOTS - 1 for plotting and cursor stuff
PRIVATE const int BLS_plotbox = CAL_PLOTBOX;     

// x-y values for a knot and a point
PRIVATE int total_knots;
PRIVATE float X_knot;
PRIVATE float Y_knot;
PRIVATE float Z_param;

// for specifying what to draw in each of the z-axis positions of the
// baseline subtract plot box.  Each is a one bit flag.
enum { data_curve = 1, baseline = 2, base_with_peaks = 4,
       zero_with_peaks = 8 }  curve_kinds;

// value for determining peaks, a point must be this much higher than the
// baseline curve in order to plot as a peak
PRIVATE float Threshold = (float) 50.0;  // arbitrary first value

PRIVATE int do_baseline_op(void * field_data, char * field_string);
PRIVATE int do_select_knot(void * field_data, char * field_string);
PRIVATE int delete_scroll_knot(void * field_data, char * field_string);
PRIVATE BOOLEAN knots_form_init(int index);
PRIVATE int baseline_redraw(void * field_data, char * field_string);
PRIVATE int new_draw_one(void * field_data, char * field_string);
PRIVATE int BS_save_curves(void * field_data, char * field_string);

PRIVATE ERR_OMA get_baseline_data(CURVEDIR *, SHORT, USHORT, USHORT,
                                         float *, float *, CHAR, SHORT*);
PRIVATE ERR_OMA get_base_peak_data(CURVEDIR *, SHORT, USHORT, USHORT,
                                          float *, float *, CHAR, SHORT*);
PRIVATE ERR_OMA get_zero_peak_data(CURVEDIR *, SHORT, USHORT, USHORT,
                                          float *, float *, CHAR, SHORT*);
BOOLEAN baseline_form_init(int dummy_not_used);
BOOLEAN baseline_form_exit(void);

// order MUST match exactly with enum type curve_kinds
static GET_POINT_FUNC * data_func[] = { GetDataPoint,
                                      get_baseline_data,
                                      get_base_peak_data,
                                      get_zero_peak_data
                                    };

// toggle field, enumerate baseline operations
enum {
     clear_baseline,
     auto_gen_baseline,
     increase_baseline,
     decrease_baseline
} baseline_op;

PRIVATE char * baseline_action[] =
{ "Clear baseline",
  "Autogenerate from Z",
  "Increase by Z",
  "Decrease by Z"
};

// toggle control for baseline operations
PRIVATE enum baseline_op baseline_op_flag = auto_gen_baseline;

PRIVATE BOOLEAN draw_one_only = FALSE; // redraw all curves or only one
PRIVATE float draw_one_curvenum;  // the curve to draw if only one

// disable/enable for displaying different curves
enum { disabled = 0, enabled = 1 } show_state;

// disable/enable symbols for displaying different curves
PRIVATE char * show_enable[] = { " ", "X" };

// toggle controls for display options
PRIVATE enum show_state show_curve      = enabled;
PRIVATE enum show_state show_baseline   = enabled;
PRIVATE enum show_state show_base_peaks = disabled;
PRIVATE enum show_state show_zero_peaks = disabled;

// dummy variables for select fields
PRIVATE int add_knot_dummy = 0;
PRIVATE int delete_knot_dummy = 0;

// mode flag for baslnsub_plot(),  either redraw curves OR add curves to
// directory
enum { display_redraw, directory_add } baslnsub_plot_mode;

PRIVATE enum baslnsub_plot_mode plot_flag = display_redraw;

// file specifier for saving curves to the directory
PRIVATE char file_spec[DOSFILESIZE + DOSPATHSIZE + 1] = "";

// startup flag for adding curves
PRIVATE BOOLEAN first_add_flag;
PRIVATE struct save_area_info * SavedArea;           

// fields are organized in four groups by column
#define COL_A 2
#define COL_B 25
#define COL_C 40
#define COL_D 52

// fields and form for baseline calculations
// fields are ordered top to bottom, left to right

enum DATAREGISTRY_ACCESS
{
  DGROUP_DO_STRINGS = 1, DGROUP_TOGGLE, DGROUP_CODE, DGROUP_DATA, DGROUP_FORM
};

// registries for the forms

static DATA DO_STRING_Reg[] =
{
   { "baseline",      0, DATATYP_STRING, DATAATTR_PTR, 0 },  //  0
   { "Threshold",     0, DATATYP_STRING, DATAATTR_PTR, 0 },  //  1
   { "KNOTS :",       0, DATATYP_STRING, DATAATTR_PTR, 0 },  //  2
   { "DISPLAY",       0, DATATYP_STRING, DATAATTR_PTR, 0 },  //  3
   { "Z",             0, DATATYP_STRING, DATAATTR_PTR, 0 },  //  4
   { "base w/peaks",  0, DATATYP_STRING, DATAATTR_PTR, 0 },  //  5
   { "zero w/peaks",  0, DATATYP_STRING, DATAATTR_PTR, 0 },  //  6
   { "Delete",        0, DATATYP_STRING, DATAATTR_PTR, 0 },  //  7
   { "Select",        0, DATATYP_STRING, DATAATTR_PTR, 0 },  //  8
   { "X      Y",      0, DATATYP_STRING, DATAATTR_PTR, 0 },  //  9
   { "curve",         0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 10
   { "one Z only",    0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 11
   { "Save to :",     0, DATATYP_STRING, DATAATTR_PTR, 0 }   // 12
};

// toggle stuff for toggle registry
static DATA TOGGLE_Reg[] =
{
   { show_enable,     0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },     // 0
   { baseline_action, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 }      // 1
};

// declare form now, definition later
PRIVATE FORM total_form;
PRIVATE FORM knots_form;

static EXEC_DATA CODE_Reg[] =
{
/* 0*/ { CAST_CHR2INT baseline_form_init,  0, DATATYP_CODE, DATAATTR_PTR, 0 },     
/* 1*/ { CAST_CHR2INT do_baseline_op,      0, DATATYP_CODE, DATAATTR_PTR, 0 },     
/* 2*/ { CAST_CHR2INT do_select_knot,      0, DATATYP_CODE, DATAATTR_PTR, 0 },     
/* 3*/ { CAST_CHR2INT delete_scroll_knot,  0, DATATYP_CODE, DATAATTR_PTR, 0 },     
/* 4*/ { CAST_CHR2INT knots_form_init,     0, DATATYP_CODE, DATAATTR_PTR, 0 },     
//scroll_... are declared in forms.h
/* 5*/ { CAST_CHR2INT scroll_entry_field,  0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 6*/ { CAST_CHR2INT scroll_up_field,     0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 7*/ { CAST_CHR2INT scroll_down_field,   0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 8*/ { CAST_CHR2INT refresh_scroll_only, 0, DATATYP_CODE, DATAATTR_PTR, 0 },     
/* 9*/ { CAST_CHR2INT baseline_redraw,     0, DATATYP_CODE, DATAATTR_PTR, 0 },     
/*10*/ { CAST_CHR2INT new_draw_one,        0, DATATYP_CODE, DATAATTR_PTR, 0 },     
/*11*/ { CAST_CHR2INT BS_save_curves,      0, DATATYP_CODE, DATAATTR_PTR, 0 },  
/*12*/ { baseline_form_exit,  0, DATATYP_CODE, DATAATTR_PTR, 0 },     
};

static DATA DATA_Reg[] = {
   { & Threshold,        0, DATATYP_FLOAT,  DATAATTR_PTR, 0 },  //  0
   { & total_knots,      0, DATATYP_INT,    DATAATTR_PTR, 0 },  //  1
   { & X_knot,           0, DATATYP_FLOAT,  DATAATTR_PTR, 0 },  //  2
   { & Y_knot,           0, DATATYP_FLOAT,  DATAATTR_PTR, 0 },  //  3
   { & add_knot_dummy,   0, DATATYP_INT,    DATAATTR_PTR, 0 },  //  4
   { & delete_knot_dummy,0, DATATYP_INT,    DATAATTR_PTR, 0 },  //  5
   { & baseline_op_flag, 0, DATATYP_INT,    DATAATTR_PTR, 0 },  //  6
   { & Z_param,          0, DATATYP_FLOAT,  DATAATTR_PTR, 0 },  //  7
   { & show_baseline,    0, DATATYP_INT,    DATAATTR_PTR, 0 },  //  8
   { & show_base_peaks,  0, DATATYP_INT,    DATAATTR_PTR, 0 },  //  9
   { & show_zero_peaks,  0, DATATYP_INT,    DATAATTR_PTR, 0 },  // 10
   { & show_curve,       0, DATATYP_INT,    DATAATTR_PTR, 0 },  // 11
   { & draw_one_only,    0, DATATYP_INT,    DATAATTR_PTR, 0 },  // 12
   { file_spec,          0, DATATYP_STRING, DATAATTR_PTR, 0 }   // 13
};

static DATA FORM_Reg[] = {

   { &knots_form,  0, DATATYP_VOID, DATAATTR_PTR, 0 },     // 1
   { &total_form,  0, DATATYP_VOID, DATAATTR_PTR, 0 },     // 10
    };

// names for the form fields
enum baslnsub_field_names
{  FLD_BLS_ACTION = 0,
   LBL_Z,
   FLD_ZVAL,
   LBL_THRESH,
   FLD_THRESH,
   LBL_SAVE,
   FLD_SAVE_FILE,
   LBL_DISPLAY,
   LBL_CURVE,
   FLD_CRVTGL,
   LBL_BASELINE,
   FLD_BLTGL,
   LBL_PEAKS,
   FLD_PEAKTGL,
   LBL_ZPEAKS,
   FLD_ZPTGL,
   LBL_ONEZ,
   FLD_ONEZTGL,
   FLD_KNOTFM,
   LBL_KNOTS,
   FLD_ADDKNOT,
   FLD_DELKNOT,
   LBL_X_Y,
   FLD_KNOTSC
};

PRIVATE FIELD baseline_sub_fields[] =
{
  field_set(FLD_BLS_ACTION,       /* 0 */
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_BL_ACTION,                   
    BASLNSUB_HBASE + 0,
    DGROUP_DATA,   6,             /* baseline_op_flag */
    DGROUP_TOGGLE, 1,             /* baseline_action[] */
    DGROUP_CODE,   1,             /* do_baseline_op() */
    0, 4,                         /* 4 option toggle field */
    1, COL_A, 20,
    FORM_EXIT_UP, FLD_BLS_ACTION, FLD_SAVE_FILE, FLD_ZVAL,
    FLD_KNOTSC, FLD_CRVTGL, FLD_KNOTSC, FLD_ZVAL),

  label_field(LBL_Z,              /* 1 */
    DGROUP_DO_STRINGS, 4,
    3, COL_A + 8, 1),

  field_set(FLD_ZVAL,             /* 2 */
    FLDTYP_STD_FLOAT,
    FLDATTR_REV_VID,
    KSI_BL_Z,                   
    BASLNSUB_HBASE + 1,
    DGROUP_DATA, 7,
    0, 0,
    0, 0,
    5, 0,
    3, COL_A + 10, 9,
    FORM_EXIT_UP, FLD_ZVAL, FLD_BLS_ACTION, FLD_THRESH,
    FLD_KNOTSC, FLD_CRVTGL, FLD_BLS_ACTION, FLD_CRVTGL),

  label_field(LBL_THRESH,         /* 3 */
    DGROUP_DO_STRINGS, 1,
    5, COL_A, 9),

  field_set(FLD_THRESH,           /* 4 */
    FLDTYP_STD_FLOAT,
    FLDATTR_REV_VID,
    KSI_BL_THRESHOLD,                   
    BASLNSUB_HBASE + 2,
    DGROUP_DATA, 0,
    0, 0,
    0, 0,
    5, 0,
    5, COL_A + 10, 9,
    FORM_EXIT_UP, FLD_THRESH, FLD_ZVAL, FLD_SAVE_FILE,
    FLD_KNOTSC, FLD_PEAKTGL, FLD_KNOTSC, FLD_PEAKTGL),

  label_field(LBL_SAVE,           /* 5 */
    DGROUP_DO_STRINGS, 12,
    7, COL_A, 9),

  field_set(FLD_SAVE_FILE,        /* 6 */
    FLDTYP_STRING,
    FLDATTR_REV_VID,
    KSI_BL_NAME,                   
    BASLNSUB_HBASE + 3,
    DGROUP_DATA, 13,
    0, 0,                         /* file_spec */
    DGROUP_CODE, 11,              /* BS_save_curves() to directory */
    DOSFILESIZE + DOSPATHSIZE + 1, 0,
    7, COL_A + 10, 37,
    FORM_EXIT_UP, FLD_SAVE_FILE, FLD_THRESH, FLD_BLS_ACTION,
    FLD_KNOTSC, FLD_ONEZTGL, FLD_THRESH, FLD_ONEZTGL),

  label_field(LBL_DISPLAY,        /* 7 */
    DGROUP_DO_STRINGS, 3,
    1, COL_B + 4, 7),

  label_field(LBL_CURVE,          /* 8 */
    DGROUP_DO_STRINGS, 10,
    2, COL_B + 4, 8),

  field_set(FLD_CRVTGL,           /* 9 */
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_BL_SHOW_CURVE,                   
    BASLNSUB_HBASE + 4,
    DGROUP_DATA, 11,              /* show_curve */
    DGROUP_TOGGLE, 0,             /* show_enable[] */
    DGROUP_CODE, 9,               /* baseline_redraw() */
    0, 2,                         /* 2 option toggle field */
    2, COL_B + 13, 1,
    FORM_EXIT_UP, FLD_CRVTGL, FLD_ONEZTGL, FLD_BLTGL,
    FLD_ZVAL, FLD_ADDKNOT, FLD_ZVAL, FLD_BLTGL),

  label_field(LBL_BASELINE,       /* 10 */
      DGROUP_DO_STRINGS, 0,
      3, COL_B + 4, 8),

  field_set(FLD_BLTGL,            /* 11 */
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_BL_SHOW_BL,                   
    BASLNSUB_HBASE + 5,
    DGROUP_DATA, 8,               /* show_baseline */
    DGROUP_TOGGLE, 0,             /* show_enable[] */
    DGROUP_CODE, 11,              /* baseline_redraw() */
    0, 2,                         /* 2 option toggle field */
    3, COL_B + 13, 1,
    FORM_EXIT_UP, FLD_BLTGL, FLD_CRVTGL, FLD_PEAKTGL,
    FLD_ZVAL, FLD_ADDKNOT, FLD_CRVTGL, FLD_ADDKNOT),

  label_field(LBL_PEAKS,          /* 12 */
    DGROUP_DO_STRINGS, 5,
    4, COL_B, 12),

  field_set(FLD_PEAKTGL,
    FLDTYP_TOGGLE,                /* 13 */
    FLDATTR_REV_VID,
    KSI_BL_SHOW_SUM,                   
    BASLNSUB_HBASE + 6,
    DGROUP_DATA, 9,               /* show_base_peaks */
    DGROUP_TOGGLE, 0,             /* show_enable[] */
    DGROUP_CODE, 11,              /* baseline_redraw() */
    0, 2,                         /* 2 option toggle field */
    4, COL_B + 13, 1,
    FORM_EXIT_UP, FLD_PEAKTGL, FLD_BLTGL, FLD_ZPTGL,
    FLD_THRESH, FLD_DELKNOT, FLD_THRESH, FLD_ZPTGL),

  label_field(LBL_ZPEAKS,         /* 14 */
    DGROUP_DO_STRINGS, 6,
    5, COL_B, 12),

  field_set(FLD_ZPTGL,            /* 15 */
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_BL_SHOW_PEAKS,                   
    BASLNSUB_HBASE + 7,
    DGROUP_DATA, 10,              /* show_zero_peaks */
    DGROUP_TOGGLE, 0,             /* show_enable[] */
    DGROUP_CODE, 11,              /* baseline_redraw() */
    0, 2,                         /* 2 option toggle field */
    5, COL_B + 13, 1,
    FORM_EXIT_UP, FLD_ZPTGL, FLD_PEAKTGL, FLD_ONEZTGL,
    FLD_THRESH, FLD_DELKNOT, FLD_PEAKTGL, FLD_DELKNOT),

  label_field(LBL_ONEZ,           /* 16 */
    DGROUP_DO_STRINGS, 11,
    6, COL_B, 10),

  field_set(FLD_ONEZTGL,          /* 17 */
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_BL_SHOW_ONE,                   
    BASLNSUB_HBASE + 8,
    DGROUP_DATA, 12,              /* draw_one_only */
    DGROUP_TOGGLE, 0,             /* show_enable[] */
    DGROUP_CODE, 10,              /* new_draw_one() */
    0, 2,                         /* 2 option toggle field */
    6, COL_B + 13, 1,
    FORM_EXIT_UP, FLD_ONEZTGL, FLD_ZPTGL, FLD_SAVE_FILE,
    FLD_THRESH, FLD_DELKNOT, FLD_ZPTGL, FLD_DELKNOT),

  field_set(FLD_KNOTFM,           /* 18 */
    FLDTYP_FORM,
    FLDATTR_NONE,
    KSI_NO_INDEX,
    BASLNSUB_HBASE, /* + ?? */
    DGROUP_FORM, 1,           /* total knots form */
    0, 0,
    0, 0,
    0, 0,
    1, COL_C, 3,
    FLD_ADDKNOT, FLD_ADDKNOT, FLD_KNOTFM, FLD_KNOTFM,
    FLD_KNOTFM, FLD_KNOTFM, FLD_KNOTFM, FLD_KNOTFM),

  label_field(LBL_KNOTS,          /* 19 */
    DGROUP_DO_STRINGS, 2,
    1, COL_C + 4, 7),

  field_set(FLD_ADDKNOT,          /* 20 */
    FLDTYP_SELECT,
    FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
    KSI_NO_INDEX,                 /* graph enter function will do this */
    BASLNSUB_HBASE + 9,
    DGROUP_DATA, 4,               /* add_knot_dummy */
    DGROUP_DO_STRINGS, 8,         /* "Select" */
    DGROUP_CODE, 2,               /* do_select_knot() */
    1, 0,                         /* match value, dummy */
    3, COL_C + 4, 6,
    FORM_EXIT_UP, FLD_ADDKNOT, FLD_DELKNOT, FLD_DELKNOT,
    FLD_CRVTGL, FLD_KNOTSC, FLD_CRVTGL, FLD_KNOTSC),

  field_set(FLD_DELKNOT,          /* 21 */
    FLDTYP_SELECT,
    FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
    KSI_BL_DEL_KNOT,                   
    BASLNSUB_HBASE + 10,
    DGROUP_DATA, 5,               /* delete_knot_dummy */
    DGROUP_DO_STRINGS, 7,         /* "Delete" */
    DGROUP_CODE, 3,               /* delete_scroll_knot() */
    1, 0,                         /* match value, dummy */
    5, COL_C + 4, 6,
    FORM_EXIT_UP, FLD_DELKNOT, FLD_ADDKNOT, FLD_ADDKNOT,
    FLD_ZPTGL, FLD_KNOTSC, FLD_ZPTGL, FLD_KNOTSC),

  label_field(LBL_X_Y,            /* 22 */
    DGROUP_DO_STRINGS, 9,         /* "X      Y" */
    1, COL_D + 4, 8),

  field_set(FLD_KNOTSC,           /* 23 */
    FLDTYP_FORM,
    FLDATTR_NONE,
    KSI_NO_INDEX,
    0, 
    DGROUP_FORM, 0,               /* scrolling knot form */
    0, 0,
    0, 0,
    0, 0,
    2, COL_D, 1,
    FORM_EXIT_UP, FLD_KNOTSC, FLD_DELKNOT, FLD_BLS_ACTION,
    FLD_ADDKNOT, FLD_ZVAL, FLD_DELKNOT, FLD_BLS_ACTION)

}; /* end of baseline_sub_fields[] initialization */

PRIVATE FORM BaselnForm = {                   
   0, 0, FORMATTR_BORDER | FORMATTR_FIRST_CHAR_ERASE | FORMATTR_VISIBLE |
         FORMATTR_FULLWIDTH,
   0, 0, 0, 2, 0, 9, 80,
   0, 0,
   { DGROUP_CODE, 0 },       // init_function : baseline_form_init()
   { DGROUP_CODE, 12 },      // exit_function   baseline_form_exit()
   COLORS_DEFAULT, 0, 0, 0, 0,
   sizeof(baseline_sub_fields) / sizeof(FIELD),
   baseline_sub_fields, KSI_BASLN_FORM,
   0, DO_STRING_Reg, TOGGLE_Reg, (DATA *)CODE_Reg, DATA_Reg, FORM_Reg
};

// scrolling form for displaying knot X,Y locations

enum knots_form_field_names
{ FLD_KNOT_ENTRY,
  FLD_REF_ONLY,
  FLD_KNOT_UP,
  FLD_KNOT_DN,
  FLD_XKNOT_SCR,
  FLD_YKNOT_SCR
};

PRIVATE FIELD knots_form_fields[] = {

  // 0  logic field for scrolling form entry
  field_set(FLD_KNOT_ENTRY,
    FLDTYP_LOGIC,
    FLDATTR_NONE,
    KSI_NO_INDEX,
    0, 
    DGROUP_CODE, 5,     //  scroll_entry_field() from forms.h
    0, 0, 0, 0, 0, 0,
    1, 0, 1,
    FORM_EXIT_UP, FLD_REF_ONLY, FLD_KNOT_ENTRY, FLD_KNOT_ENTRY,
    FLD_KNOT_ENTRY, FLD_REF_ONLY, FLD_KNOT_ENTRY, FLD_KNOT_ENTRY),
 
    // 1  logic field for checking refresh only, bail out if only refreshing
  field_set(FLD_REF_ONLY,
    FLDTYP_LOGIC, FLDATTR_NONE, KSI_NO_INDEX, 0, 
    DGROUP_CODE, 8,      // refresh_scroll_only()
    0, 0,
    0, 0,
    0, 0,
    1, 2, 1,
    FLD_YKNOT_SCR, FORM_EXIT_UP, FLD_YKNOT_SCR, FLD_YKNOT_SCR,
    FLD_YKNOT_SCR, FLD_YKNOT_SCR, FLD_YKNOT_SCR, FLD_YKNOT_SCR),

  // 2  logic field for scrolling up
  field_set(FLD_KNOT_UP,
    FLDTYP_LOGIC,
    FLDATTR_NONE,
    KSI_NO_INDEX,
    0, 
    DGROUP_CODE, 6,          // scroll_up_field() from forms.h
    0, 0,
    0, 0,
    0, 0,
    1, 4, 1,
    FORM_EXIT_UP, FLD_YKNOT_SCR, FLD_KNOT_UP, FLD_YKNOT_SCR,
    FLD_KNOT_ENTRY, FLD_KNOT_DN, FLD_REF_ONLY, FLD_KNOT_DN),
  
  // 3  logic field for scrolling down
  field_set(FLD_KNOT_DN,
    FLDTYP_LOGIC,
    FLDATTR_NONE,
    KSI_NO_INDEX,
    0, 
    DGROUP_CODE, 7,          // scroll_down_field() from forms.h
    0, 0,
    0, 0,
    0, 0,
    1, 6, 1,
    FORM_EXIT_UP, FLD_YKNOT_SCR, FLD_KNOT_DN, FLD_YKNOT_SCR,
    FLD_KNOT_ENTRY, FLD_YKNOT_SCR, FLD_KNOT_UP, FLD_YKNOT_SCR),

  // 4  The float X-value of each knot
  field_set(FLD_XKNOT_SCR,
    FLDTYP_STD_FLOAT,
    FLDATTR_RJ | FLDATTR_DISPLAY_ONLY,
    KSI_NO_INDEX,
    0, 
    DGROUP_DATA, 2,          // float X_knot
    0, 0,
    0, 0,
    2, 0,
    0, 0, 7,
    FLD_YKNOT_SCR, FLD_YKNOT_SCR, FLD_KNOT_UP, FLD_KNOT_DN,
    FLD_XKNOT_SCR, FLD_XKNOT_SCR, FLD_XKNOT_SCR, FLD_XKNOT_SCR),

  // 5  The float Y value of each knot
  field_set(FLD_YKNOT_SCR,
    FLDTYP_STD_FLOAT,
    FLDATTR_REV_VID,
    KSI_BL_KNOT_Y_VAL,
    0, 
    DGROUP_DATA, 3,          // float Y_knot
    0, 0,
    0, 0,
    5, 0,
    0, 9, 10,
    FORM_EXIT_UP, FLD_YKNOT_SCR, FLD_KNOT_UP, FLD_KNOT_DN,
    FORM_EXIT_UP, FORM_EXIT_DN, FORM_EXIT_UP, FORM_EXIT_DN)
};

PRIVATE FORM knots_form = {
   0, 0, FORMATTR_SCROLLING | FORMATTR_FIRST_CHAR_ERASE | FORMATTR_VISIBLE,
   0, 0, 0,
//   4, COL_D, 6, 20,
   2, COL_D, 6, 20,
   0, 0,
   { DGROUP_CODE, 4 },         // knots_form_init()
   { 0, 0 }, COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(knots_form_fields) / sizeof(FIELD),
   knots_form_fields,
   KSI_KNOTS_FORM,
   0, DO_STRING_Reg, TOGGLE_Reg, (DATA *)CODE_Reg, DATA_Reg, 0
};

PRIVATE FIELD total_form_fields[] = {
  // 0
  // total knots integer, display only, row 0, column 0, length 3, RJ
  do_field_set(FLDTYP_UNS_INT,
    FLDATTR_DISPLAY_ONLY | FLDATTR_RJ,
    0, DGROUP_DATA,
    1, 0, 0, 0, 0, 0, 0,
  0, 0, 3)
};

PRIVATE FORM total_form = {
   0, 0, FORMATTR_FIRST_CHAR_ERASE | FORMATTR_VISIBLE,
   0, 0, 0,
   1, COL_C, 1, 4,
   0, 0,
   { 0, 0 },         // form_init() -- none needed
   { 0, 0 }, COLORS_DEFAULT,
   0, 0, 0, 0,
   1,
   total_form_fields,
   KSI_NO_INDEX,               
   0, DO_STRING_Reg, TOGGLE_Reg, (DATA *)CODE_Reg, DATA_Reg, 0
};

// return the value of Threshold
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float baseline_threshold(void) { return Threshold; }

// Redraw the baseline plot box. the whole thing : axes, labels, and all
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int baseline_redraw(void * field_data, char * field_string)
{
   erase_mouse_cursor();               
   MouseCursorEnable(FALSE);
   create_plotbox(& Plots[BLS_plotbox]);
   plot_curves(&MainCurveDir, & Plots[BLS_plotbox], BLS_plotbox);

   MouseCursorEnable(TRUE);          

   if (active_locus == LOCUS_APPLICATION)    
      CDisplayCursor(deviceHandle(), cursor_loc);

   return FIELD_VALIDATE_SUCCESS;
}

// Set draw_one_curvenum to the new Z value (Z_param) of the single curve
// to be drawn and then redraw the display.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int new_draw_one(void * field_data, char * field_string)
{
   if(draw_one_only)
      draw_one_curvenum = Z_param;
   baseline_redraw(NULL, NULL);
   return FIELD_VALIDATE_SUCCESS;
}

// set up curve index and entry index to point to the given curve in
// the window at location curve_num. Return number of points in the curve.
// Return zero if there is an error.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE USHORT index_setup(SHORT curve_num)
{
   SHORT Curve_Znum;
   CURVEHDR Curve_header;
   USHORT UTDisplayCurve;

   if(FindFirstPlotBlock(& MainCurveDir, & EntryIndex, & CurveIndex,
                         & UTDisplayCurve, BLS_plotbox)) 
      Curve_Znum = 0;
   else
      return 0;

   while(Curve_Znum < curve_num)
      if(FindNextPlotCurve(& MainCurveDir, & EntryIndex, & CurveIndex,
                           & UTDisplayCurve, BLS_plotbox)) 
         Curve_Znum ++;
      else
         break;

   if(ReadTempCurvehdr(& MainCurveDir, EntryIndex, CurveIndex,
                       & Curve_header))
      return 0; // error

   return Curve_header.pointnum;
}

// return the set of curves that the user has specified to be drawn.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE enum curve_kinds user_draw_curves(void)
{
   enum curve_kinds ret_val = 0;            // show nothing

   if(show_curve == enabled) ret_val |= data_curve;

   if(current_knots() != 0) {              // is there a baseline to show ?
      if(show_baseline   == enabled) ret_val |= baseline;
      if(show_base_peaks == enabled) ret_val |= base_with_peaks;
      if(show_zero_peaks == enabled) ret_val |= zero_with_peaks;
   }
   return ret_val;
}

// let autoscale routine find out how to get data points
// in case plot will be done with zero with peaks
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GET_POINT_FUNC * WhichAutoscaleGetFunction(void)
{
  if (baslnsub_active() && (user_draw_curves() & zero_with_peaks))
    return get_zero_peak_data;
  else
    return WhichGetFunction();
}

// get_data() function (can be used as an argument to auto_baseline())
// obtain the x,y values of a point.  Uses module globals EntryIndex
// and CurveIndex.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int get_data(int index, float * X, float * Y)
{
  static int prefBuf = -1;

  return GetDataPoint(& MainCurveDir, EntryIndex, CurveIndex, index,
                        X, Y, FLOATTYPE, &prefBuf);
}

// return TRUE iff spline knots are being manually specified by the user
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN baslnsub_active(void) { return baseline_subtract_mode; }

// Make sure the graphics cursor still works properly after autoscale
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void baslnAutoscaleCursorAdjust(void)
{
   dCXY new_point;

   MoveCursorUp(0);          
   MoveCursorDown(0);        
   new_point = current_zoom_point();
   CursorStatus[ActiveWindow].X = new_point.x;

   if(Plots[ActiveWindow].style == FALSE_COLOR)
      CursorStatus[ActiveWindow].Z = new_point.y;
   else
      CursorStatus[ActiveWindow].Y = new_point.y;
}

// invoke graphics mode to allow user to specify manual knots.
// this is a select field function from the "Select Knots" field
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int do_select_knot(void * field_data, char * field_string)
{
   SwitchToGraphMode(); // same as F10 key to go to graphics mode
   return FIELD_VALIDATE_SUCCESS;
}

// Invoked as a form initialization function.  Initialize form variables.
// Returns zero iff error, else returns one.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN baseline_form_init(int dummy_not_used)
{
  InitSplitForm(&BaselnForm, &baseline_subtract_mode, NULL);

  FKeyItems[1].Control |= MENUITEM_INACTIVE; 
  ShowFKeys(& FKey);

  Z_param = CursorStatus[ActiveWindow].Z;
  total_knots = current_knots();

  return FALSE;               
}

// put form and plot box on screen, interract with user to do baseline
// subtract function, restore screen when done.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN baseline_form_exit(void)
{
   ExitSplitForm(&baseline_subtract_mode);
   FKeyItems[1].Control &= ~MENUITEM_INACTIVE; 
   ShowFKeys(& FKey);
   return FALSE;
}

// plot a curve at z axis location Z of the baseline subtract plot box.
// EntryIndex and FileCurveNumber define the location of the curve data
// in MainCurveDir.  pcurve_header points to the header for the curve data.
// get_point is a function which obtains the data to be plotted.
// This function is based on array_plot() in the doplot module.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE ERR_OMA generic_plot(float Z, USHORT Entry, USHORT Curve,
                                    USHORT pointNum)
{
  return array_plot(&Plots[BLS_plotbox], &MainCurveDir, Entry, Curve, 0,
                    pointNum, Z);
}

// add a new curve to the curve directory specified by file_spec.
// Find out if file_spec is already int the user's curve directory.
// If it is, append the curve to it.  If it is not, instantiate a new curve
// directory entry and put the new curve in it.
// get_point() can be used to retrieve y-values for the curve being added.
// EntryIndex, FileCurveNumber, and pcurve_header all correspond to a curve
// currently in the user's directory to use as a model for the new curve.
// Use same x-axis but different y values.  Update Ymin and Ymax in the
// curve header of the new curve.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE ERR_OMA generic_add(BOOLEAN new_curve, USHORT Entry,
                                   USHORT Curve, CURVEHDR * pcurve_header)
{
  ERR_OMA err;
  SHORT dest_index, prefBuf1 = 0, prefBuf2 = 2;
  CHAR Name[DOSFILESIZE + 1],
       Path[DOSPATHSIZE + 1];
  CURVEDIR * pDir = &MainCurveDir;
  USHORT i;
  float X, Y;
  GET_POINT_FUNC * get_point = WhichGetFunction();

  // number of curves in source curve entry
  static USHORT source_curves;
  static USHORT curves_processed = 0;

  if (strlen(file_spec) == 0)
    return ERROR_NONE;

  // never process more than source_curves curves, avoid inifinite
  // loop which would otherwise occur if appending curves to the source
  if(first_add_flag)
    {
    first_add_flag = FALSE;
    source_curves = pDir->Entries[Entry].count;
    curves_processed = 0;
    }

  if(new_curve)
    // start processing a new curve
    curves_processed++;
  if(curves_processed > source_curves)
    return ERROR_NONE;

  // expand the file block name and
  // check to see that it is not a directory
  if(ParsePathAndName(Path, Name, file_spec) != 2)
    return error(ERROR_BAD_FILENAME, file_spec);

  strcpy(file_spec, Path);
  strcat(file_spec, Name);

  // if file_spec is already in user's directory
  dest_index = SearchNextNamePath(Name, Path, pDir, 0);
  if(dest_index >= 0)
    {
    // file_spec is already in the curve directory and
    // pDir->Entries[dest_index] is the first block
    // with matching name and path.  Append a new curve.
    if(err = InsertTempCurve(pDir, Entry, Curve, dest_index,
                            pDir->Entries[dest_index].count))
      return err;
    }
  else
    { // file_spec is not already in the curve directory
    // create a new entry and set up to append to it

    BOOLEAN Temp = pcurve_header->MemData;
    pcurve_header->MemData = FALSE;
    if(err = CreateTempFileBlk(pDir, &dest_index, Name, Path, "", 0, 0L, 1,
                               pcurve_header, OMA4DATA, 0))
      return err;
    pcurve_header->MemData = Temp;
    }
  // set the y data values, ymin, ymax
  pcurve_header -> Ymin = (FLOAT)MAXFLOAT;
  pcurve_header -> Ymax = (FLOAT)MINFLOAT;
  for(i = 0; i < pcurve_header -> pointnum; i ++)
    {
    if(err = (* get_point)(pDir, Entry, Curve, i, &X, &Y, FLOATTYPE, &prefBuf1))
      return err;

    if(err = SetDataPoint(pDir, dest_index, pDir->Entries[dest_index].count-1,
                          i, &X, &Y, FLOATTYPE, &prefBuf2))
      return err;

    if(Y < pcurve_header -> Ymin)
      pcurve_header -> Ymin = Y;
    if(Y > pcurve_header -> Ymax)
      pcurve_header -> Ymax = Y;
    }
  return WriteTempCurvehdr(pDir, dest_index, pDir->Entries[dest_index].count,
                           pcurve_header);
}

// get data function for use by generic_plot().  If the curve data is
// within Threshold of the baseline, plot the baseline.  Otherwise, plot
// the curve data.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE ERR_OMA get_base_peak_data(CURVEDIR *CurveDir,
                                          SHORT EntryIndex,
                                          USHORT CurveIndex , USHORT index,
                                          float * x_val, float * y_val,
                                          CHAR DataType, SHORT *PrefBuf)
{
   ERR_OMA err = GetDataPoint(CurveDir, EntryIndex, CurveIndex,
                                      index, x_val, y_val, DataType,
                                      PrefBuf);

   // first put the data point of the real curve in x_val, y_val
   if(! err) {

      // put the y value of the baseline in YS
      float YS = point_interp(* x_val);

      // test for a peak
      if (* y_val < (YS + Threshold))
         * y_val = YS;
   }
   return err;
}

// get data function for use by generic_plot().  If the curve data is
// within Threshold of the baseline, plot zero.  Otherwise, plot
// the curve data less the baseline.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE ERR_OMA get_zero_peak_data(CURVEDIR *CurveDir,
                                          SHORT EntryIndex,
                                          USHORT CurveIndex , USHORT index,
                                          float * x_val, float * y_val,
                                          CHAR DataType, SHORT *PrefBuf)
{

   // first put the data point of the real curve in x_val, y_val
   ERR_OMA err  = GetDataPoint(CurveDir, EntryIndex, CurveIndex,
                                      index, x_val, y_val, DataType,
                                      PrefBuf);

   if(! err) {

      // put the y value of the baseline in YS
      float YS = point_interp(* x_val);

      // test for a peak
      if (* y_val < (YS + Threshold))
         * y_val = (float) 0.0;
      else
         * y_val -= YS;
   }
   return err;
}

// get data function for use by generic_plot().  Return the value of the
// basline curve in x_val, y_val.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE ERR_OMA get_baseline_data(CURVEDIR *CurveDir,
                                         SHORT EntryIndex,
                                         USHORT CurveIndex , USHORT index,
                                         float * x_val, float * y_val,
                                         CHAR DataType, SHORT *PrefBuf)
{
   // first put the data point of the real curve in x_val, y_val
   ERR_OMA err = GetDataPoint(CurveDir, EntryIndex, CurveIndex,
                                     index, x_val, y_val, DataType,
                                     PrefBuf);

   if(! err)
      // put the y value of the baseline in y_val
      * y_val = point_interp(* x_val);

   return err;
}

// This function is invoked from Graphops() when the user presses the enter
// key.  Add a knot at the cursor position and then redraw the graph,
// the scrolling knot form, and the knot total.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void add_graphics_cursor_knot()
{
   unsigned int new_index;
   float XX;   // dummy
   float YY;   // dummy

   if (pKSRecord != NULL)
   {
      if (*pKSRecord)
      {
         CHAR Buf[80];

         sprintf(Buf, "ADD_KNOT(%.7g, %.7g);\n",
                  CursorStatus[ActiveWindow].X,
                  CursorStatus[ActiveWindow].Y);
         MacRecordString(Buf);
      }
   }

   add_knot(CursorStatus[ActiveWindow].X, CursorStatus[ActiveWindow].Y);

   total_knots = current_knots();

   new_index = closest_knot(CursorStatus[ActiveWindow].X, & XX, & YY);

   display_random_field(& total_form, 0);

   redraw_scroll_form(&knots_form, new_index, SCROLL_FORM_ACTIVE_FIELD);

   baseline_redraw(NULL, NULL);   // show the new baseline by redrawing
}

// select field function to delete a knot based on the scrolling knot form
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int delete_scroll_knot(void * field_data, char * field_string)
{
   if (total_knots == 0)     
      return 0;
   delete_knot_index(knots_form.virtual_row_index);
   total_knots = current_knots();
   draw_form();                      // show new values in form fields
   baseline_redraw(NULL, NULL);   // redraw the graph with new knot set
   return FIELD_VALIDATE_SUCCESS;
}

// verify function for saving displayed curves to file_spec
// Set plot_flag to directory_add so that baslnsub_plot() will add curves
// to the directory instead of plotting them when it is called. Restore
// plot flag to display_redraw for normal redrawing when done.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int BS_save_curves(void * field_data, char * field_string)
{
   if (strlen(file_spec) == 0)
    return baseline_redraw(field_data, field_string);

   erase_mouse_cursor();               
   MouseCursorEnable(FALSE);         
   plot_flag = directory_add;
   first_add_flag = TRUE;  // indicate first time to generic_add()

   // cause baslnsub_plot() to be called for each curve
   plot_curves(& MainCurveDir, & Plots[BLS_plotbox], BLS_plotbox);
   MouseCursorEnable(TRUE);         
   if (active_locus == LOCUS_APPLICATION)       
      CDisplayCursor(deviceHandle(), cursor_loc);

   plot_flag = display_redraw;
   return FIELD_VALIDATE_SUCCESS;
}

// toggle field function for baseline operations for the baseline form.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int do_baseline_op(void * field_data, char * field_string)
{
   USHORT point_num;
   USHORT dest = (USHORT) Z_param;  // float to USHORT

   switch(baseline_op_flag) {
      case clear_baseline :
         delete_all_knots();
         break;
      case auto_gen_baseline :
         point_num = index_setup(dest);
         auto_baseline(point_num, get_data);
         break;
      case increase_baseline :
         move_spline(Z_param);
         break;
      case decrease_baseline :
         move_spline(- Z_param);
         break;
   }
   total_knots = current_knots();
   draw_form();
   baseline_redraw(NULL, NULL);
   return FIELD_VALIDATE_SUCCESS;
}

// put a marker at each knot position for curve Z in the plot box
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void draw_knot_markers(float Z)
{
  int i;
  CXY knot_point;
  CMARKERREPR MarkerRep, OldMarkerRep;

  // change the color and type of marker
  CInqMarkerRepr(screen_handle, &OldMarkerRep);

  MarkerRep.Type = CMK_Star;
  MarkerRep.Color = BRT_GREEN;
  MarkerRep.Height = KNOT_MARKER_HEIGHT;
  CSetMarkerRepr(screen_handle, &MarkerRep);

  for(i = 0; i < total_knots; i ++)
    {
    knot_point = gss_position(&Plots[BLS_plotbox], knot_x_val(i),
                              knot_y_val(i), Z);

    CPolymarker(screen_handle, 1, &knot_point);
    }
  // replace old marker style
  CSetMarkerRepr(screen_handle, &OldMarkerRep);       
}

// If plot_flag is display_redraw, then do the following :
//    plot special baseline curves for curve Z in the baseline subtract plot
//    box.  Set up a clipping rectangle at the start and restore it at the
//    end.  EntryIndex and FileCurveNumber define the location of the curve
//    data in MainCurveDir.  TCurveHdr is the curve header for the curve
//    data.
// If plot_flag is directory_add, don't do any plotting stuff and call
// generic_add() instead of generic_plot()
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA baslnsub_plot(float Z, USHORT Entry,
                             USHORT Curve, CURVEHDR * TCurveHdr)
{
  ERR_OMA err = ERROR_NONE;
  enum curve_kinds draw_curves;
  CRECT ClipRect;
  CRECT OldClipRect;
  int i;
  int j;
  BOOLEAN new_curve; // signal generic_add each new curve

  // if only drawing one curve but not this one, return
  if(draw_one_only && (Z != draw_one_curvenum))
    return ERROR_NONE;

  // set the clipping rectangle and save the current one
  if(plot_flag == display_redraw)
    {
    if(CalcClipRect(&Plots[BLS_plotbox], Z, &ClipRect))
      return ERROR_NONE;
    CInqClipRectangle(deviceHandle(), &OldClipRect);
    CSetClipRectangle(deviceHandle(), ClipRect);
    }
  draw_curves = user_draw_curves();
  new_curve = TRUE;
  // zero_with_peaks is the last literal in enum type curve_kinds
  for(i = 0, j = 1; j <= zero_with_peaks; i ++, j <<= 1)
    {
    if(draw_curves & j)
      {
      switch(plot_flag)
        {
        case display_redraw :
          SetGetFunction(data_func[i]);
          err = generic_plot(Z, Entry, Curve, TCurveHdr->pointnum);
          SetGetFunction(NULL);
        break;
        case directory_add :
          SetGetFunction(data_func[i]);
          err = generic_add(new_curve, Entry, Curve, TCurveHdr);
          SetGetFunction(NULL);
        break;
        }
      new_curve = FALSE;
      if(err)
        break;
      }
    }

  if(plot_flag == display_redraw)
    {
    // put up knot markers only if the baseline curve is being displayed
    if(draw_curves & baseline)
      draw_knot_markers(Z);
    CSetClipRectangle(deviceHandle(), OldClipRect);
    }
  return err;
}

// form init function for scrolling knots_form
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN knots_form_init(int index)
{
   if((index >= current_knots()) || !current_knots) 
      return TRUE; // out of bounds

   X_knot = knot_x_val(index);
   Y_knot = knot_y_val(index);
   return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void macDoAddKnot(double xVal, double yVal)
{
   if(baslnsub_active())
   {
      CursorStatus[ActiveWindow].X = (FLOAT) xVal;    
      CursorStatus[ActiveWindow].Y = (FLOAT) yVal;

      if(isCurrentFormMacroForm())
      {
         erase_cursor();
         SetGraphCursorType(CursorType);
      }
      add_graphics_cursor_knot();
    
      if(isCurrentFormMacroForm())
         set_cursor_type(TextCursor);
   }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void registerBaslnsubForms(void)
{
   FormTable[KSI_BASLN_FORM] = &BaselnForm;
   FormTable[KSI_KNOTS_FORM] = &knots_form;
}
