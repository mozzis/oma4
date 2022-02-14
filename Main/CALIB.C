/* -----------------------------------------------------------------------
/
/  CALIB.C
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/calib.c_v   0.36   06 Jul 1992 10:24:54   maynard  $
/  $Log:   J:/logfiles/oma4000/main/calib.c_v  $
/
*/

#include <string.h>
#include <math.h>

#include "calib.h"
#include "spgraph.h"   // ResetOffset
#include "forms.h"
#include "tempdata.h"
#include "points.h"
#include "helpindx.h"
#include "curvdraw.h"  // Replot()
#include "device.h"
#include "cursor.h"
#include "graphops.h"
#include "ksindex.h"
#include "multi.h"
#include "oma4000.h"
#include "omaform.h"    // GraphWindow
#include "formwind.h"
#include "syserror.h"  // ERROR_ALLOC_MEM
#include "omaerror.h"
#include "formtabs.h"
#include "crventry.h"
#include "omameth.h"   // InitialMethod
#include "autoscal.h"  // AutoScalePlotBox()
#include "plotbox.h"
#include "curvedir.h"  // MainCurveDir
#include "crvheadr.h"
#include "curvbufr.h"
#include "splitfrm.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

/* must be one greater than maximum degree of fit (cubic is max here) */
#define     CAL_ROWS           4
#define     CAL_COLUMNS        (CAL_ROWS + 1)

// maximum number of points to calibrate to
#define     MAX_CAL_POINTS    15
#define     MAX_COEFFICIENTS  4

#define     CAL_DEGREE_LINEAR          0
#define     CAL_DEGREE_SQUARE          1
#define     CAL_DEGREE_CUBIC           2

#define     CAL_APPLY_TO_ONE_CURVE     0
#define     CAL_APPLY_TO_ALL_CURVES    1
#define     CAL_APPLY_TO_PIXELS        2

#define     CAL_ACTION_CALIBRATE       0
#define     CAL_ACTION_REUSE           1
#define     CAL_ACTION_EXTRACT         2
#define     CAL_ACTION_UNCALIBRATE     3

// PRIVATE functions
PRIVATE int delete_point(void * field_data, char * field_string);
PRIVATE int do_calibration(void * field_data, char * field_string);
PRIVATE int check_calib_unit(void * field_data, char * field_string);
PRIVATE BOOLEAN init_calib_scroll(unsigned int index);
PRIVATE BOOLEAN exit_calib_scroll(unsigned int index);
PRIVATE BOOLEAN GetCalibPointFormInit(void);
PRIVATE BOOLEAN GetCalibPointFormExit(void);
PRIVATE BOOLEAN X_CalibrationFormInit(void);
PRIVATE BOOLEAN X_CalibrationFormExit(void);

//  spgraph (1235) uses this so allow
//  visibility:  PRIVATE int get_new_points(void);

// used for marking as a special case in macro playback
BOOLEAN InCalibForm = FALSE;     

static SHORT OldActiveWindow;    
static CRECT OldGraphArea = { {-1, -1}, {-1, -1} };       
static SHORT OldLocus;

static USHORT     CalibPointNum = 0;   
static USHORT     degree_of_fit = 2;      

static CalibPoint   CalibrationPoints[ MAX_CAL_POINTS ];
// for spgraph.c M1235
// static CalibPoint * CurrentCalibrationPoint = &CalibrationPoints[0];
CalibPoint * CurrentCalibrationPoint = &CalibrationPoints[0];

static DOUBLE       calibrate_unit_value = 0.0;

/* summing matrix for least squares */
double coefmtrx[ CAL_ROWS ] [ CAL_COLUMNS ];

BOOLEAN gathering_calib_points = FALSE;

static int CalibActionFlag = CAL_ACTION_CALIBRATE;
static int CalibUnitsFlag;
static int CalibDegreeFlag = CAL_DEGREE_LINEAR;

static int CalibApplyFlag = CAL_APPLY_TO_PIXELS;

DOUBLE ExcitationWaveLength;

static int cal_dummy_select = FALSE;

double CalCoeff[MAX_COEFFICIENTS];   

static struct save_area_info * SavedArea = NULL;
static BOOLEAN NoAction;

// note corresponding entries in convert.asm and also in
//     WaveLengthOptions[] in calib.c

UCHAR WaveLengthUnitTable[] =
{
  /*  0 */  COUNTS,
  /*  1 */  NM,
  /*  2 */  UM,
  /*  3 */  MM,
  /*  4 */  CM,
  /*  5 */  METER,
  /*  6 */  WAVENUMBER,
  /*  7 */  ANGSTROM,
  /*  8 */  RSHIFT,
  /*  9 */  EV,
  /* 10 */  JOULE,
  /* 11 */  ERG,
  /* 12 */  HZ,
  /* 13 */  ADJ_NM,
  /* 14 */  SECOND,
  /* 15 */  FSEC,
  /* 16 */  PSEC,
  /* 17 */  NSEC,
  /* 18 */  USEC,
  /* 19 */  MSEC,
  /* 20 */  0xFF,  /* place holder for unknown value */
};

const int szWlUnitTbl = sizeof(WaveLengthUnitTable) /
                        sizeof(WaveLengthUnitTable[0]) - 1;

/* ------------------------------------------------------------------ */

// N.B. This table should match up with the data units definitions in
// omatyp.h --  COUNTS, ANGSTROM, NM, UM, etc.

char * WaveLengthOptions[] = {
    /*  0 */  "Pixels",
    /*  1 */  "Nanometer",
    /*  2 */  "Micrometer",
    /*  3 */  "Millimeter",
    /*  4 */  "Centimeter",
    /*  5 */  "Meter",
    /*  6 */  "Wavenumber",
    /*  7 */  "Angstrom",
    /*  8 */  "Raman Shift",
    /*  9 */  "eV",
    /* 10 */  "Joule",
    /* 11 */  "Erg",
    /* 12 */  "Hz",
    /* 13 */  "<Nanometer>",
    /* 14 */  "Second",
    /* 15 */  "Femtosecond",
    /* 16 */  "Picosecond",
    /* 17 */  "Nanosecond",
    /* 18 */  "Microsecond",
    /* 19 */  "Millisecond",
    /* 20 */  "Unknown",
};

static char * CalibDegreeOptions[] = { "Linear", "Square", "Cubic" };

static char * CalibApplyOptions[] = {
         "curve at cursor", "all curves in window", "all pixel conversions"
};
   
static char * CalibActionOptions[] = {
   "Perform New Calibration",    "Apply Current Calibration",
   "Extract Coefficients from Calibrated Curve",     "Uncalibrate"
};

enum { DGROUP_DO_STRINGS = 1, DGROUP_FORMS,   DGROUP_TOGGLES,
       DGROUP_CODE,           DGROUP_GENERAL
};

static DATA DO_STRING_Reg[] = {
  /* 0  */ { "Xp Units",        0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 1  */ { "Pixel",           0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 2  */ { "Xp Unit Value",   0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 3  */ { "Degree of Fit",   0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 4  */ { "Apply to ",       0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 5  */ { "2",               0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 6  */ { "3",               0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 7  */ { "Xp =",            0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 8  */ { "+",               0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 9  */ { "P +",             0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 10 */ { "P",               0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 11 */ { "Select Points",   0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 12 */ { "Delete Point",    0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 13 */ { "value",           0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 14 */ { "Excitation (nm)", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
  /* 15 */ { " Go ",            0, DATATYP_STRING, DATAATTR_PTR, 0 },
};

static FORM X_CalScrollForm;

static DATA Forms_Reg[] = {
   { &X_CalScrollForm, 0, DATATYP_VOID, DATAATTR_PTR, 0 },
};

static DATA Toggles_Reg[] = {
   /* 0 */  { WaveLengthOptions, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
   /* 1 */  { CalibDegreeOptions, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
   /* 2 */  { CalibApplyOptions, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
   /* 3 */  { CalibActionOptions, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
};

static EXEC_DATA CODE_Reg[] = {
   /* 0  */  { CAST_CHR2INT scroll_entry_field, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 1  */  { CAST_CHR2INT scroll_up_field, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 2  */  { CAST_CHR2INT scroll_down_field, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 3  */  { CAST_CHR2INT delete_point, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 4  */  { CAST_CHR2INT do_calibration, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 5  */  { CAST_CHR2INT refresh_scroll_only, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 6  */  { CAST_CHR2INT init_calib_scroll, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 7  */  { CAST_CHR2INT exit_calib_scroll, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 8  */  { GetCalibPointFormInit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 9  */  { GetCalibPointFormExit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 10 */  { X_CalibrationFormInit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 11 */  { X_CalibrationFormExit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 12 */  { get_new_points, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 13  */ { CAST_CHR2INT check_calib_unit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
};

static DATA GENERAL_Reg[] = {
/* 0  */  { &calibrate_unit_value, 0, DATATYP_DOUBLE_FLOAT, DATAATTR_PTR, 0 },
/* 1  */  { &CalibUnitsFlag, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 2  */  { &CalibDegreeFlag, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 3  */  { &CalibApplyFlag, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 4  */  { &CalibActionFlag, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 5  */  { &CurrentCalibrationPoint, STRUCTOFFSET(CalibPoint, x_value),
                  DATATYP_DOUBLE_FLOAT, DATAATTR_PTR_PTR_OFFS, 0 },
/* 6  */  { &CurrentCalibrationPoint,
                  STRUCTOFFSET(CalibPoint, calibration_value),
                  DATATYP_DOUBLE_FLOAT, DATAATTR_PTR_PTR_OFFS, 0 },
/* 7  */  { &CalCoeff[0], 0, DATATYP_DOUBLE_FLOAT, DATAATTR_PTR, 0 },
/* 8  */  { &CalCoeff[1], 0, DATATYP_DOUBLE_FLOAT, DATAATTR_PTR, 0 },
/* 9  */  { &CalCoeff[2], 0, DATATYP_DOUBLE_FLOAT, DATAATTR_PTR, 0 },
/* 10 */  { &CalCoeff[3], 0, DATATYP_DOUBLE_FLOAT, DATAATTR_PTR, 0 },
/* 11 */  { &cal_dummy_select, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 12 */  { &ExcitationWaveLength, 0, DATATYP_DOUBLE_FLOAT, DATAATTR_PTR, 0 },
};

   /* calibration units */
   /* laser line for Raman Shift units */
   /* CalibPoint Scroll form */
   /* calibration degree */
   // get point
   /* calibration scope */
   // delete point
   /* perform, use current, extract from curve and axis */
   /* Go */
   /* first parameter in equation */
   /* second parameter in equation */
   /* third parameter in equation */
   /* fourth parameter in equation */


static enum {
   LBL_XUNITS,   // (0) "Xp Units"
   LBL_EXCIT,    // (1) "Excitation (nm)"
   LBL_PIXEL,    // (2) "Pixel"
   LBL_UNVAL,    // (3) "Xp Unit Value"
   LBL_DEGREE,   // (4) "Degree of Fit"
   LBL_APPLY,    // (5) "Apply to "
   LBL_A2SQ,     // (6) "2"
   LBL_A3CU,     // (7) "3"
   LBL_XPEQ,     // (8) "Xp ="
   LBL_A0PLUS,   // (9) "+"
   LBL_A1PLUS,   // (10) "P +"
   LBL_A2PLUS,   // (11) "P +"
   LBL_A3PLUS,   // (12) "P"

   FLD_XUNITS,   /* (13) calibration units */
   FLD_EXCIT,    /* (14) laser line for Raman Shift units */
   FLD_CALSCFM,  /* (15) CalibPoint Scroll form */
   FLD_DEGREE,   /* (16) calibration degree */
   FLD_GETPT,    /* (17) get point */
   FLD_CALSCOPE, /* (18) calibration scope */
   FLD_DELPT,    /* (19) delete point */
   FLD_CALACT,   /* (20) perform, use current, extract from curve and axis */
   FLD_CALGO,    /* (21) Go */
   FLD_A0,       /* (22) first parameter in equation */
   FLD_A1,       /* (23) second parameter in equation */
   FLD_A2,       /* (24) third parameter in equation */
   FLD_A3,       /* (25) fourth parameter in equation */
};

static FIELD X_CalibrationFormFields[] = {

   label_field(LBL_XUNITS,
   DGROUP_DO_STRINGS, 0,    // "Xp Units"
   1, 2, 8),

   label_field(LBL_EXCIT,
   DGROUP_DO_STRINGS, 14,   // "Excitation (nm)"
   1, 23, 15),

   label_field(LBL_PIXEL,
   DGROUP_DO_STRINGS, 1,    // "Pixel"
   1, 56, 8),

   label_field(LBL_UNVAL,
   DGROUP_DO_STRINGS, 2,    // "Xp Unit Value"
   1, 65, 13),

   label_field(LBL_DEGREE,
   DGROUP_DO_STRINGS, 3,    // "Degree of Fit"
   3, 2, 13),

   label_field(LBL_APPLY,
   DGROUP_DO_STRINGS, 4,    // "Apply to "
   5, 2, 8),

   label_field(LBL_A2SQ,
   DGROUP_DO_STRINGS, 5,    // "2"
   8, 42, 1),

   label_field(LBL_A3CU,
   DGROUP_DO_STRINGS, 6,    // "3"
   8, 55, 1),

   label_field(LBL_XPEQ,
   DGROUP_DO_STRINGS, 7,    // "Xp ="
   9, 2, 4),

   label_field(LBL_A0PLUS,
   DGROUP_DO_STRINGS, 8,    // "+"
   9, 17, 1),

   label_field(LBL_A1PLUS,
   DGROUP_DO_STRINGS, 9,    // "P +"
   9, 28, 3),

   label_field(LBL_A2PLUS,
   DGROUP_DO_STRINGS, 9,    // "P +"
   9, 41, 3),

   label_field(LBL_A3PLUS,
   DGROUP_DO_STRINGS, 10,   // "P"
   9, 54, 1),

  field_set(FLD_XUNITS,     /* calibration units */
  FLDTYP_TOGGLE,
  FLDATTR_REV_VID,
  KSI_XCAL_UNITS,
  X_CALIBRATIONFIELD_HBASE + 0,
  DGROUP_GENERAL, 1,        // CalibUnitsFlag
  DGROUP_TOGGLES, 0,        // WaveLengthOptions
  DGROUP_CODE, 13,          // check_calib_unit
  0, 15,
  1, 11, 11,
  EXIT, FLD_XUNITS, FLD_A0, FLD_DEGREE,
  FLD_XUNITS, FLD_DEGREE, FLD_A2, FLD_EXCIT),

  field_set(FLD_EXCIT,      /* laser line for Raman Shift units */
  FLDTYP_STD_FLOAT,
  FLDATTR_REV_VID,
  KSI_XCAL_EXCIT,
  X_CALIBRATIONFIELD_HBASE + 1,
  DGROUP_GENERAL, 12,       // ExcitationWaveLength
  0, 0,
  0, 0,
  5, 0,
  1, 39, 8,
  EXIT, FLD_EXCIT, FLD_A0, FLD_DEGREE,
  FLD_XUNITS, FLD_CALSCFM, FLD_XUNITS, FLD_DEGREE),

  field_set(FLD_CALSCFM,    /* CalibPoint Scroll form */
  FLDTYP_FORM,
  FLDATTR_NONE,
  KSI_NO_INDEX,
  X_CALIBRATIONFIELD_HBASE + 2,
  DGROUP_FORMS, 0,
  0, 0,
  0, 0,
  0, 0,
  2, 59, 1,
  EXIT, FLD_CALSCFM, FLD_XUNITS, FLD_CALACT,
  FLD_GETPT, FLD_GETPT, FLD_GETPT, FLD_DEGREE),

  field_set(FLD_DEGREE,     /* calibration degree */
  FLDTYP_TOGGLE,
  FLDATTR_REV_VID,
  KSI_XCAL_DEGREE,
  X_CALIBRATIONFIELD_HBASE + 3,
  DGROUP_GENERAL, 2,        // CalibDegreeFlag
  DGROUP_TOGGLES, 1,        // CalibDegreeOptions
  0, 0,
  0, 3,
  3, 16, 6,
  EXIT, FLD_DEGREE, FLD_XUNITS, FLD_CALSCOPE,
  FLD_DEGREE, FLD_GETPT, FLD_EXCIT, FLD_GETPT),

  field_set(FLD_GETPT,      /* get point */
  FLDTYP_SELECT,
  FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
  KSI_NO_INDEX,             // handled by graph entry function
  X_CALIBRATIONFIELD_HBASE + 4,
  DGROUP_GENERAL, 11,       // cal_dummy_select
  DGROUP_DO_STRINGS, 11,    // "Select Points"
  DGROUP_CODE, 12,          // get_new_points
  1, 5,
  3, 42, 13,
  EXIT, FLD_GETPT, FLD_EXCIT, FLD_DELPT,
  FLD_XUNITS, FLD_CALSCFM, FLD_DEGREE, FLD_CALSCFM),

  field_set(FLD_CALSCOPE,   /* calibration scope */
  FLDTYP_TOGGLE,
  FLDATTR_REV_VID,
  KSI_XCAL_SCOPE,
  X_CALIBRATIONFIELD_HBASE + 5,
  DGROUP_GENERAL, 3,        // CalibApplyFlag
  DGROUP_TOGGLES, 2,        // CalibApplyOptions
  0, 0,
  3, 3,
  5, 11, 21,
  EXIT, FLD_CALSCOPE, FLD_DEGREE, FLD_CALACT,
  FLD_CALSCOPE, FLD_GETPT, FLD_DEGREE, FLD_DELPT),

  field_set(FLD_DELPT,      /* delete point */
  FLDTYP_SELECT,
  FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
  KSI_XCAL_DEL_POINT,
  X_CALIBRATIONFIELD_HBASE + 6,
  DGROUP_GENERAL, 11,       // cal_dummy_select
  DGROUP_DO_STRINGS, 12,    // "Delete Point"
  DGROUP_CODE, 3 ,          // delete_point
  2, 5,
  5, 42, 12,
  EXIT, FLD_DELPT, FLD_GETPT, FLD_CALGO,
  FLD_XUNITS, FLD_CALSCFM, FLD_CALSCOPE, FLD_CALSCFM),

   
  field_set(FLD_CALACT,     /* select calib action */
  FLDTYP_TOGGLE,
  FLDATTR_REV_VID,
  KSI_XCAL_ACTION,
  X_CALIBRATIONFIELD_HBASE + 7,
  DGROUP_GENERAL, 4,        // CalibActionFlag
  DGROUP_TOGGLES, 3,        // CalibActionOptions
  0, 0,
  3, 4,
  7, 2, 42,
  EXIT, FLD_CALACT, FLD_CALSCOPE, FLD_A0,
  FLD_CALACT, FLD_CALGO, FLD_CALSCFM, FLD_CALGO),

  field_set(FLD_CALGO,      /* Go */
  FLDTYP_SELECT,
  FLDATTR_REV_VID,
  KSI_XCAL_GO,
  USERFORM_HBASE + 3,
  DGROUP_GENERAL, 11,       // cal_dummy_select
  DGROUP_DO_STRINGS, 15,    // " Go "
  DGROUP_CODE, 4 ,          // do_calibration
  0, 0,
  7, 49, 4,
  EXIT, FLD_CALGO, FLD_DELPT, FLD_A3,
  FLD_CALACT, FLD_CALGO, FLD_CALACT, FLD_A0),

  field_set(FLD_A0,         /* first parameter in equation */
  FLDTYP_STD_FLOAT,
  FLDATTR_RJ | FLDATTR_REV_VID,
  KSI_XCAL_PARAM_1,
  X_CALIBRATIONFIELD_HBASE + 8,
  DGROUP_GENERAL, 7,        // CalCoeff[0]
  0, 0,
  0, 0,
  9, 0,
  9, 7, 8,
  EXIT, FLD_A0, FLD_CALACT, FLD_XUNITS,
  FLD_CALGO, FLD_A1, FLD_CALGO, FLD_A1),

  field_set(FLD_A1,         /* second parameter in equation */
  FLDTYP_STD_FLOAT,
  FLDATTR_RJ | FLDATTR_REV_VID,
  KSI_XCAL_PARAM_2,
  X_CALIBRATIONFIELD_HBASE + 9,
  DGROUP_GENERAL, 8,        // CalCoeff[1]
  0, 0,
  0, 0,
  9, 0,
  9, 18, 8,
  EXIT, FLD_A1, FLD_CALACT, FLD_A2,
  FLD_A0, FLD_A2, FLD_A0, FLD_XUNITS),

  field_set(FLD_A2,         /* third parameter in equation */
  FLDTYP_STD_FLOAT,
  FLDATTR_RJ | FLDATTR_REV_VID,
  KSI_XCAL_PARAM_3,
  X_CALIBRATIONFIELD_HBASE + 10,
  DGROUP_GENERAL, 9,        // CalCoeff[2]
  0, 0,
  0, 0,
  9, 0,
  9, 31, 8,
  EXIT, FLD_A2, FLD_CALACT, FLD_XUNITS,
  FLD_A1, FLD_A3, FLD_A1, FLD_A3),

  field_set(FLD_A3,         /* fourth parameter in equation */
  FLDTYP_STD_FLOAT,
  FLDATTR_RJ | FLDATTR_REV_VID,
  KSI_XCAL_PARAM_4,
  X_CALIBRATIONFIELD_HBASE + 11,
  DGROUP_GENERAL, 10,       // CalCoeff[3]
  0, 0,
  0, 0,
  9, 0,
  9, 44, 8,
  EXIT, FLD_A3, FLD_CALACT, FLD_XUNITS,
  FLD_A0, FLD_A2, FLD_A2, FLD_A0),
};

static FORM X_CalibrationForm = {
   0, 0,
   FORMATTR_BORDER | FORMATTR_FIRST_CHAR_ERASE | FORMATTR_VISIBLE |
   FORMATTR_FULLWIDTH,
   0, 0, 0,
   2, 0, 11, 80,
   0, 0,
   {DGROUP_CODE, 10},  // X_CalibrationFormInit
   {DGROUP_CODE, 11},  // X_CalibrationFormExit
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(X_CalibrationFormFields) / sizeof(X_CalibrationFormFields[0]) ,
   X_CalibrationFormFields, KSI_XCAL_FORM,
   0, DO_STRING_Reg, Forms_Reg, Toggles_Reg, (DATA *)CODE_Reg, GENERAL_Reg
};

static FIELD X_CalScrollFormFields[] = {

   { FLDTYP_LOGIC,
     FLDATTR_NONE,
     KSI_NO_INDEX,
     X_CALSCROLLFIELD_HBASE + 0,
     { DGROUP_CODE, 0 },
     {0, 0},
     {0, 0},
     {0, 0},             /* (1:0) */
     1, 0, 1,
     { -1, 1, 0, 0, 0, 1, 0, 0 } },

   { FLDTYP_LOGIC,
     FLDATTR_NONE,
     KSI_NO_INDEX,
     X_CALSCROLLFIELD_HBASE + 1,
     { DGROUP_CODE, 5 },            // refresh_scroll_only
     {0, 0},
     {0, 0},
     {0, 0},                                /* (1:1) */
     1, 2, 1,
     { 4, -2, 0, 0, 0, 0, 0, 0 } },

   { FLDTYP_LOGIC,
     FLDATTR_NONE,
     KSI_NO_INDEX,
     X_CALSCROLLFIELD_HBASE + 2,
     { DGROUP_CODE, 1 },
     {0, 0},
     {0, 0},
     {0, 0},            /* (1:2) */
     1, 4, 1,
     { -3, 3, 0, 3, -2, 1, -1, 1 } },

   { FLDTYP_LOGIC,
     FLDATTR_NONE,
     KSI_NO_INDEX,
     X_CALSCROLLFIELD_HBASE + 3,
     { DGROUP_CODE, 2 },
     {0, 0},
     {0, 0},
     {0, 0},            /* (1:3) */
     1, 6, 1,
     { -4, 2, 0, 2, -3, 2, -1, 2 } },

   { FLDTYP_STD_FLOAT,
     FLDATTR_RJ | FLDATTR_DISPLAY_ONLY,
     KSI_NO_INDEX,
     X_CALSCROLLFIELD_HBASE + 4,
     { DGROUP_GENERAL, 5 },    // CurrentCalibrationPoint.x_value
     {0, 0},
     {0, 0},
     {3, 0},                                 /* (1:4) */
     0, 0, 7,
     { -5, 0, -2, -1, 0, 0, 0, 0 } },

   { FLDTYP_STD_FLOAT,
     FLDATTR_REV_VID,
     KSI_XCAL_X_VAL,
     X_CALSCROLLFIELD_HBASE + 6,
     { DGROUP_GENERAL, 6 },   // CurrentCalibrationPoint.calibration_value 
     {0, 0},
     {0, 0},
     {5, 0},                                  /* (1:5) */
     0, 9, 10,
     { -6, 0, -3, -2, 0, 0, -6, -6 } }
};

static FORM X_CalScrollForm = {
   0, 0,
   FORMATTR_SCROLLING | FORMATTR_FIRST_CHAR_ERASE | FORMATTR_VISIBLE,
   0, 0, 0,
   2, 57, 8, 20,
   0, -1,                     // initialized to -1 for no points for M1235.
   {DGROUP_CODE, 6},          // init_calib_scroll
   {DGROUP_CODE, 7},          // exit_calib_scroll
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(X_CalScrollFormFields) / sizeof(X_CalScrollFormFields[ 0 ]),
   X_CalScrollFormFields, KSI_XCAL_SCROLL_FORM,
   0, DO_STRING_Reg, Forms_Reg, Toggles_Reg, (DATA *)CODE_Reg, GENERAL_Reg
};


static FIELD GetCalibPointFormFields[] = {
   { FLDTYP_TOGGLE,
     FLDATTR_RJ | FLDATTR_DISPLAY_ONLY,
     KSI_NO_INDEX,
     0,
     { DGROUP_GENERAL, 1 },  // CalibUnitsFlag
     {DGROUP_TOGGLES, 0 },   // WaveLengthOptions
     {0, 0},
     {0, 13},                                          /* (2:0) */
     1, 1, 14,
     { 1, 0, 0, 0, 0, 0, 0, 0 } },
   { FLDTYP_STRING,
     FLDATTR_DISPLAY_ONLY,
     KSI_NO_INDEX,
     0,
     { DGROUP_DO_STRINGS, 13 },      // "value"
     {0, 0},
     {0, 0},
     {0, 0},                                 /* (2:1) */
     1, 16, 5,
     { 1, 0, 0, 0, 0, 0, 0, 0 } },
   { FLDTYP_STD_FLOAT,
     FLDATTR_REV_VID,
     KSI_XCAL_GP_VAL,
     GET_CALIB_PT_FIELD_HBASE + 0,
     { DGROUP_GENERAL, 0 },    // calibrate_unit_value
     {0, 0},
     {0, 0},
     {5, 0},                                  /* (2:2) */
     1, 22, 10,
     { -3, -3, 0, 0, 0, 0, 0, 0 } }
};

static FORM GetCalibPointForm = {
   0, 0, FORMATTR_EXIT_RESTORE | FORMATTR_BORDER |
    FORMATTR_FIRST_CHAR_ERASE | FORMATTR_VISIBLE,
   0, 0, 0, 6, 2, 3, 34, 0, 0,
   { DGROUP_CODE, 8 },     // GetCalibPointFormInit
   { DGROUP_CODE, 9 },     // GetCalibPointFormExit
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(GetCalibPointFormFields) / sizeof(GetCalibPointFormFields[0]),
   GetCalibPointFormFields, KSI_XCAL_POINT_FORM,
   0, DO_STRING_Reg, Forms_Reg, Toggles_Reg, (DATA *)CODE_Reg, GENERAL_Reg
};

#define  SCROLL_FORM_ACTIVE_FIELD   5

// ------------------------------------------------------------------

PRIVATE BOOLEAN init_calib_scroll(unsigned int index)   
{
   if ((index < CalibPointNum) && (CalibPointNum != 0))
   {
      CurrentCalibrationPoint = &CalibrationPoints[index];
      return(FALSE);
   }
   else
      return(TRUE);
}


/* !!!!!!!!!!!!!!!!!!! link this to exit function of scroll form */
PRIVATE BOOLEAN exit_calib_scroll(unsigned int index)
{
   if (CalibPointNum)         
      show_active_field(Current.Form, SCROLL_FORM_ACTIVE_FIELD);
   return(FALSE);

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
USHORT insert_point_in_calib_list(USHORT index,
                                  double x_value, double calib_value)
{
  USHORT         i;

  x_value = (double) DataPtToChnl((int)(x_value+0.5));
  if (CalibPointNum == 0)
    {
    CalibrationPoints[0].x_value = x_value;
    CalibrationPoints[0].calibration_value = calib_value;
    CalibPointNum++;

    return(0);
    }
  else if (CalibPointNum < MAX_CAL_POINTS)
    {
    if (index < CalibPointNum-1)
      {
      for (i=CalibPointNum; i > index; i--)
        CalibrationPoints[ i ] = CalibrationPoints[ i - 1 ];
      }
    CalibrationPoints[index].x_value = x_value;
    CalibrationPoints[index].calibration_value = calib_value;
    CalibPointNum++;

    return(index);
    }
  else
    error(ERROR_CAL_TOO_MANY_POINTS, MAX_CAL_POINTS);

  return(-1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE unsigned int delete_point_from_calib_list(unsigned int index)
{
  USHORT   i;

  if (CalibPointNum > 0)
    {
    CalibPointNum--;

    if (index < CalibPointNum)
      {
      for (i=index; i < CalibPointNum; i++)
        CalibrationPoints[ i ] = CalibrationPoints[ i + 1 ];
      }
    else
      {
      if (index != 0)
        index--;
      }

    return(index);
    }
  else
    return(-1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN insert_point_in_scroll_form(double x_value, double calibration_value)
{
   unsigned int new_index;

   new_index = insert_point_in_calib_list(CalibPointNum,
                                          x_value, calibration_value);

   if (new_index != -1)
      redraw_scroll_form(&X_CalScrollForm, new_index,
                          SCROLL_FORM_ACTIVE_FIELD);

   return(new_index != -1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN delete_point_from_scroll_form(void)
{
   unsigned int delete_point_index;
   unsigned int new_index;

   delete_point_index = index_of_scroll_form(&X_CalScrollForm);

   new_index = delete_point_from_calib_list(delete_point_index);

   if (new_index != -1)
   {
      if (CalibPointNum == 0)
         redraw_scroll_form(&X_CalScrollForm, new_index, -1);
      else
         redraw_scroll_form(&X_CalScrollForm, new_index, SCROLL_FORM_ACTIVE_FIELD);
   }
   return(new_index != -1);
}

/* !!!!!!!!!!!!!!!!!!!!!!!
This function will be called from graphops.c,
when the user presses the RETURN key...
!!!!!!!!!!!!!!!!!!!!!!!
*/

PRIVATE BOOLEAN GetCalibPointFormInit(void)
{
   OldLocus = active_locus;         

   if(! popupWindowBegin()) // allocate PopupWindow
      return TRUE;    // error

   calibrate_unit_value = (double) CursorStatus[ActiveWindow].X;

   // fakeout the forms system so that the mouse handler
   // will require this form to be taken of first.
   active_locus = LOCUS_POPUP;

   popupWindowSetup(GetCalibPointForm.row, GetCalibPointForm.column,
                    GetCalibPointForm.size_in_rows,
                    GetCalibPointForm.size_in_columns);
   return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN GetCalibPointFormExit(void)
{
  popupWindowEnd();
  
  if (Current.Form->exit_key_code != KEY_ESCAPE)
     insert_point_in_scroll_form(
                          (DOUBLE) CursorStatus[ActiveWindow].PointIndex,
                          calibrate_unit_value);

  SetGraphCursorType(CursorType);            
  active_locus = OldLocus;

  return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void get_point_for_calibration(void)
{
  run_form(&GetCalibPointForm, &default_form_attributes, FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void GetNewPointsInit(void)       
{
  erase_cursor();
  gathering_calib_points = TRUE;
  if (push_form_context())
    setCurrentFormToGraphWindow();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int GetNewPointsExit(void)       
{
 gathering_calib_points = FALSE;

 cal_dummy_select = FALSE;
 
 erase_screen_area(1, 0, 1, screen_columns, specialGrafPrepareForExit(), FALSE);

 specialGrafReturnToForm();
  
 return(FIELD_VALIDATE_SUCCESS);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int get_new_points(void)
{
  GetNewPointsInit();     

  GraphOps();

  return GetNewPointsExit();     
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int delete_point(void * field_data, char * field_string) 
{
   cal_dummy_select = FALSE;

   if (delete_point_from_scroll_form())
   {
      return(FIELD_VALIDATE_SUCCESS);
   }
   else
   {
      /* error(?); */
      return(0);
   }
}

// sort the calibration points by x_value
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SortCalibPoints(BOOLEAN Ascending)
{
   USHORT i, j;
   CalibPoint CPTemp;
   BOOLEAN Switch;

   for (i=1; i<CalibPointNum; i++)
   {
      
      /* check for equivalent points */
      if (CalibrationPoints[i].x_value == CalibrationPoints[i-1].x_value)
      {
         for (j=i-1; j<CalibPointNum-1; j++)
         {
            CalibrationPoints[j].x_value = CalibrationPoints[j+1].x_value;
         }
         CalibPointNum --;
         i--;
         Switch = FALSE;
      }
      else if (Ascending &&
         (CalibrationPoints[i].x_value < CalibrationPoints[i-1].x_value))
         Switch = TRUE;
      else if (! Ascending &&
         (CalibrationPoints[i].x_value > CalibrationPoints[i-1].x_value))
         Switch = TRUE;
      else
         Switch = FALSE;

      if (Switch)
      {
         CPTemp = CalibrationPoints[i-1];
         CalibrationPoints[i-1] = CalibrationPoints[i];
         CalibrationPoints[i] = CPTemp;
         if (i!=1)
            i-=2;
      }
   }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN verify_list_order(void)
{
   USHORT i;
   double LastCalVal;
   BOOLEAN Ascending;

   /* sort by pixel value */
   SortCalibPoints(Plots[CAL_PLOTBOX].x.ascending);

   LastCalVal = CalibrationPoints[1].calibration_value;

   if (CalibrationPoints[0].calibration_value < LastCalVal)
      Ascending = TRUE;
   else
      Ascending = FALSE;

   /* check for ascending or descending calibration values */
   for (i=2; i<CalibPointNum; i++)  // 4/11/90
   {
      if (Ascending &&
         (CalibrationPoints[i].calibration_value < LastCalVal))
         return FALSE;
      else if (! Ascending &&
         (CalibrationPoints[i].calibration_value > LastCalVal))
         return FALSE;
      LastCalVal = CalibrationPoints[i].calibration_value;   
   }

   return(TRUE);  /* for now */
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void initialize_matrix_from_points(void)
{
   USHORT      point;      
   USHORT      i;
   USHORT      j;

   for (i=0; i < CAL_ROWS; i++)
   {
      for (j = 0; j < CAL_COLUMNS; j ++)
         coefmtrx[i][j] = (double) 0.0;
   }

   for (point = 0; point < CalibPointNum; point++)
   {
      for (i=0; i<degree_of_fit; i++)
      {
         for (j=0; j<degree_of_fit; j++)
         {
            if ((i+j) == 0)                       
               coefmtrx[i][j]++;
            // x_value is type double  RAC  August 24, 1990
            else if (CalibrationPoints[point].x_value != 0.0)
               coefmtrx[i][j] +=
               // pow((double) CalibrationPoints[point].x_value,
               pow(CalibrationPoints[point].x_value, (double) (i+j));
         }
         if (i==0)
            coefmtrx[i][degree_of_fit] +=
            CalibrationPoints[point].calibration_value;
         // else if (CalibrationPoints[point].x_value != 0)
         else if (CalibrationPoints[point].x_value != 0.0)
            coefmtrx[i][degree_of_fit] +=
            // (pow((double) CalibrationPoints[point].x_value, (double) i)
            (pow(CalibrationPoints[point].x_value, (double) i)
            * CalibrationPoints[point].calibration_value);
      }
   }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void least_squares_fit()
{
   USHORT i, j;         
   USHORT pivot;        // row/column to pivot around.  

   initialize_matrix_from_points();

   /* don't try to solve if not enough entries to be legitimate.*/
   if ((USHORT)coefmtrx[0][0] < degree_of_fit)  /* an invalid condition.*/
   {
      error(ERROR_CAL_TOO_FEW_POINTS, (int)coefmtrx[0][0]);
      return;
   }
   else
   {
      /* perform Gauss Jordan elimination on a system of simultaneous
      * equations represented by the coefficients in coefmtrx. */

      /* actual number of values entered.*/
      for (pivot = 0; pivot < degree_of_fit; pivot++)
      {
         /* if pivotal coefficient = 0, it's over; zero remaining results.*/
         if (! coefmtrx[pivot][pivot])
         {
            for (i = pivot; i < degree_of_fit; i++)
               coefmtrx[i][degree_of_fit]=0.;
            break;                  /* and stop trying to normalize!*/
         }
         else                       /* if not, keep going! */
         {
            /* The heart: divide each entry in the pivotal row by the pivotal
            * coefficient; then for each j'th row - except the pivotal row -
            * multiply the pivotal row by the coef. in the pivotal column of the
            * j'th row, subtract the result from the pivotal row and replace
            * the j'th row with the final result.  This causes the pivotal
            * coefficient to be 1, and makes all other coefficients in the pivotal
            * column 0 - i.e., the unknown represented by the pivotal column drops
            * out of all equations except the one represented by the pivotal row.
            * Since the values of 1 and 0 are understood, the routine does not
            * actually store them in the matrix.
            * A modification is to check each row for the max pivot coefficient
            * to reduce the math roundoff errors. I didn't find much difference.*/

            /* loop over columns after the pivot column.*/
            for (i = pivot + 1; i < CAL_COLUMNS; i++)
            {
               /* normalize the pivot row */
               if (coefmtrx[pivot][pivot] == 0.0)
                  coefmtrx[pivot][i] = MAXFLOAT;
               else if (coefmtrx[pivot][pivot] >= MAXFLOAT)
                  coefmtrx[pivot][i] = 0.0;
               else if (coefmtrx[pivot][pivot] <= MINFLOAT)
                  coefmtrx[pivot][i] = 0.0;
               else
                  coefmtrx[pivot][i] /= coefmtrx[pivot][pivot];

               /* loop over all rows in that column
               * except for the pivot row */
               for (j = 0; j < degree_of_fit; j++)
               {
                  if (j != pivot) /* subtract the product from each row */
                  {
                     coefmtrx[j][i] -=
                     (coefmtrx[pivot][i] * coefmtrx[j][pivot]);
                  }

                  if (coefmtrx[j][i] > MAXFLOAT)
                     coefmtrx[j][i] = MAXFLOAT;
                  else if (coefmtrx[j][i] < MINFLOAT)
                     coefmtrx[j][i] = MINFLOAT;
               }
            }
         }
      }

      /* use 4 until constant defined for max coefficients */
      for (i = 0; i < MAX_COEFFICIENTS; i++)    
         CalCoeff[i] = 0.0;

      for (i = 0; i < degree_of_fit; i++) /* store the results .*/
      {
         CalCoeff[i] = coefmtrx[i][degree_of_fit];
      }

   }
}

/* a helper function for CheckUniqueY(), test the calibration polynomial */
/* in a small range around xZero to determine monotonicity. */
/* Only interested in integers in the range [ 0 ... pointNum ]. */
/* Return TRUE iff strictly monotonic. */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN isMonotonic(float xZero, PFLOAT CalibCoeff, USHORT pointNum)
{
   enum { UNKNOWN, RISING, FALLING } direction = UNKNOWN;
   float hiLimit;        /* last point to check */
   float fPrev;
   float fCurrent;

   /* outside [ 0 ... pointNum ] ? */
   if((xZero < 0.0F) || (xZero >= pointNum))
      return TRUE;

   xZero = (float) floor(xZero);
   if(xZero >= 1.0F) xZero --; /* start at 1 less than floor */
   hiLimit = xZero + 4.0F;  /* 4 points is plenty to check */
   while(hiLimit >= pointNum)    /* go no farther than numPoints - 1 */
      hiLimit --;
   if(hiLimit - xZero < 1.0F)
      return TRUE;  /* at most one point in range */
   
   fPrev = ApplyCalibrationToX(CalibCoeff, xZero);
   for(; xZero <= hiLimit; xZero ++) {
      fCurrent = ApplyCalibrationToX(CalibCoeff, xZero);
      if(fPrev < fCurrent) {
         if(direction == FALLING)
            return FALSE;
         direction = RISING;
      }
      else if(fPrev > fCurrent) {
         if(direction == RISING)
            return FALSE;
         direction = FALLING;
      }
      else
         return FALSE;   /* identical values */
      fPrev = fCurrent;
   }
   return TRUE;
}

/* check for unique Y values over the range of points */   
/* return TRUE if Y values are NOT unique. */
/* new version, RAC 6/17/91 */
/* assume a cubic polynomial with coefficients in CalibCoeff[]. */
/* check the range 0 to PointNum only. */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN CheckUniqueY(PFLOAT CalibCoeff, USHORT pointNum)
{
  /* x value where the first derivative of the calibration polynomial */
  /* is zero.  There are at most 2 for a cubic polynomial. */
  float firstZero;   
  float secondZero;
  
  if(CalibCoeff[3] == 0.0F)
     if(CalibCoeff[2] == 0.0F)
        /* linear has unique Y's unless the slope is 0.0 */
        return CalibCoeff[1] == 0.0F;            
     else {
        /* quadratic polynomial has only one point to check */
        firstZero = (-0.5F) * CalibCoeff[1] / CalibCoeff[2];
        return ! isMonotonic(firstZero, CalibCoeff, pointNum);
     }
  else {
     double rootNumber = CalibCoeff[2] * CalibCoeff[2] -
                        3.0 * CalibCoeff[3] * CalibCoeff[1];
     if(rootNumber < 0.0)
        return FALSE;  /* There is no point with first derivative zero */

     rootNumber = sqrt(rootNumber);
     firstZero  = (-CalibCoeff[2] + rootNumber) / CalibCoeff[3] * 3.0;
     secondZero = (-CalibCoeff[2] + rootNumber) / CalibCoeff[3] * 3.0;
  }
  /* there are two points to check where first derivative is zero */
  if(isMonotonic(firstZero, CalibCoeff, pointNum))
     return ! isMonotonic(secondZero, CalibCoeff, pointNum);

  return TRUE; /* not monotonic at firstZero */
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FLOAT _pascal ApplyCalibrationToX(PFLOAT CalibCoeff, FLOAT X)
{
   DOUBLE ReturnVal = (DOUBLE)DataPtToChnl((int)X);
 
   ReturnVal = CalibCoeff[0] + ReturnVal * (CalibCoeff[1]  +
               (ReturnVal *
               (CalibCoeff[2] + (CalibCoeff[3] * ReturnVal))));
  
   if(ReturnVal < 0 && ReturnVal < MINFLOAT)
     ReturnVal = MINFLOAT;
   else if(ReturnVal > MAXFLOAT)
     ReturnVal = MAXFLOAT;

   return (FLOAT)ReturnVal;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA ApplyCalibrationToCurve(CURVEDIR *pCurveDir,
                                       SHORT EntryIndex,
                                       USHORT CurveIndex,
                                       PFLOAT CalibCoeff,
                                       CHAR CalibUnits)
{
  FLOAT X, Y;
  USHORT i;
  SHORT prefBuf = 0;
  CURVEHDR Curvehdr;
  ERR_OMA err;

  if (err = ReadTempCurvehdr(pCurveDir, EntryIndex, CurveIndex, &Curvehdr))
    return err;

  /* check for unique Y values */
  if (CheckUniqueY(CalibCoeff, Curvehdr.pointnum))
    return error(ERROR_CAL_NOT_UNIQUE_Y);

  Curvehdr.Xmin = (FLOAT)MAXFLOAT;
  Curvehdr.Xmax = (FLOAT)MINFLOAT;

  if (Curvehdr.MemData)
    {
    GenXData(Curvehdr.pointnum, CalibCoeff);
    GetXMinMax(&Curvehdr.Xmin, &Curvehdr.Xmax);
    }
  else
    {
    for (i=0; i<Curvehdr.pointnum; i++)
      {
      // get X value in curve units
      if (err = GetDataPoint(pCurveDir, EntryIndex, CurveIndex,
                            i, &X, &Y, FLOATTYPE, &prefBuf))
        return err;

      X = ApplyCalibrationToX(CalibCoeff, (FLOAT)i);

      // set X value in curve units
      if (err = SetDataPoint(pCurveDir, EntryIndex, CurveIndex,
                            i, &X, NULL, FLOATTYPE, &prefBuf))
        return err;

      if (X < Curvehdr.Xmin)
        Curvehdr.Xmin = X;
      if (X > Curvehdr.Xmax)
        Curvehdr.Xmax = X;
      }
    }

  /* change the curve units and write the new curve header out */
  Curvehdr.XData.XUnits = CalibUnits;

  return WriteTempCurvehdr(pCurveDir, EntryIndex, CurveIndex, &Curvehdr);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA ExtractCalPointsFromCurve(CURVEDIR *pCurveDir,
                                         SHORT EntryIndex,
                                         USHORT CurveIndex)
{
   FLOAT i, PointInterval;
   FLOAT X, Y;
   SHORT j, prefBuf = 0;
   CURVEHDR Curvehdr;
   USHORT Index;
   ERR_OMA err;

   if (err = ReadTempCurvehdr(pCurveDir, EntryIndex, CurveIndex,
      &Curvehdr))
      return err;

   if (Curvehdr.pointnum <= MAX_CAL_POINTS)
      PointInterval = (FLOAT) 1;
   else
      PointInterval = (FLOAT) Curvehdr.pointnum / (FLOAT) (MAX_CAL_POINTS+1);

   Index = 0;
   CalibPointNum = 0;
   for (i=(FLOAT)0; (i < (FLOAT) Curvehdr.pointnum) &&
      (Index < MAX_CAL_POINTS); i+=PointInterval)
   {
      // get X value in curve units
      if (err = GetDataPoint(pCurveDir, EntryIndex, CurveIndex,
         (USHORT) i, &X, &Y, FLOATTYPE, &prefBuf))
         return err;
      insert_point_in_calib_list(Index++, (DOUBLE) i, (DOUBLE) X);

   }

  for (j=0; i < szWlUnitTbl; i++)
   {
      if (WaveLengthUnitTable[j] == Curvehdr.XData.XUnits)
      {
         CalibUnitsFlag = j;
         break;
      }
   }

   return ERROR_NONE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE ERR_OMA UnCalibrateCurve(CURVEDIR *pCurveDir,
                                        SHORT EntryIndex,
                                        USHORT CurveIndex)
{
   FLOAT i, X, Y;
   SHORT prefBuf = 0;
   CURVEHDR Curvehdr;
   ERR_OMA err;

   if (err = ReadTempCurvehdr(pCurveDir, EntryIndex, CurveIndex, &Curvehdr))
      return err;

   for (i=(FLOAT)0; i < (FLOAT) Curvehdr.pointnum; i++)
   {
      // get Y value
      if (err = GetDataPoint(pCurveDir, EntryIndex, CurveIndex,
         (USHORT) i, &X, &Y, FLOATTYPE, &prefBuf))
         return err;

      // set X value
      if (err = SetDataPoint(pCurveDir, EntryIndex, CurveIndex,
         (USHORT) i, &i, NULL, FLOATTYPE, &prefBuf))
         return err;
   }

   Curvehdr.Xmin = 0.0F;
   Curvehdr.Xmax = (FLOAT) (Curvehdr.pointnum - 1);
   Curvehdr.XData.XUnits = COUNTS;

   if (err = WriteTempCurvehdr(pCurveDir, EntryIndex, CurveIndex, &Curvehdr))
      return err;

   return ERROR_NONE;
}

PRIVATE int check_calib_unit(void * field_data, char * field_string)
{
  if (WaveLengthUnitTable[*((USHORT *)field_data)] == ADJ_NM)
    {
    int j;

    for (j = 0; j < szWlUnitTbl; j++)
      {
      if (WaveLengthUnitTable[j] == NM)
        {
        CalibUnitsFlag = j;
        break;
        }
      }
    if (field_string) /* field_string==NULL means called from init_form */
      display_random_field(&X_CalibrationForm, FLD_XUNITS);
    }
 return(FIELD_VALIDATE_SUCCESS);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int do_calibration(void * field_data, char * field_string)  
{
  SHORT EntryIndex, ReturnVal, prefBuf = 0;
  USHORT i, CurveIndex, UTDisplayCurve;
  UCHAR CalUnits;
  FLOAT CalibCoeff[MAX_COEFFICIENTS];
  BOOLEAN Found;
  ERR_OMA err = ERROR_NONE;
  WINDOW * MessageWindow;

  NoAction = FALSE;

  put_up_message_window(BusyWorking, COLORS_MESSAGE, &MessageWindow);

  EntryIndex = CursorStatus[ActiveWindow].EntryIndex;

  if ((EntryIndex == -1) ||
    ! (MainCurveDir.Entries[EntryIndex].DisplayWindow & (1<<CAL_PLOTBOX)))
    {
    Found = FindFirstPlotBlock(&MainCurveDir, &EntryIndex,
      &CurveIndex, &UTDisplayCurve, ActiveWindow);
    if (! Found && (CalibApplyFlag != CAL_APPLY_TO_PIXELS))
      {
      if (MessageWindow != NULL)
        release_message_window(MessageWindow);
      return error(ERROR_BAD_CURVE_BLOCK, EntryIndex);
      }
    }

  CurveIndex = CursorStatus[ActiveWindow].FileCurveNumber;

  CalUnits = WaveLengthUnitTable[CalibUnitsFlag];

  switch (CalibActionFlag)
    {
    case CAL_ACTION_UNCALIBRATE:
      if (EntryIndex == -1)
        error(err = ERROR_BAD_CURVE_BLOCK, EntryIndex);
      else if (CalibApplyFlag == CAL_APPLY_TO_ONE_CURVE)
        err = UnCalibrateCurve(&MainCurveDir, EntryIndex, CurveIndex);
      else if (CalibApplyFlag == CAL_APPLY_TO_ALL_CURVES)
        {
        Found = FindFirstPlotBlock(&MainCurveDir, &EntryIndex,
          &CurveIndex, &UTDisplayCurve, ActiveWindow);

        while (Found && (! err))
          {
          err = UnCalibrateCurve(&MainCurveDir, EntryIndex, CurveIndex);
          if (! err)
            Found = FindNextPlotCurve(&MainCurveDir, &EntryIndex,
              &CurveIndex, &UTDisplayCurve, ActiveWindow);
          }
        }
      else if (CalibApplyFlag == CAL_APPLY_TO_PIXELS)
        {
        InitialMethod->CalibCoeff[0][0] = (FLOAT) 0.0;
        InitialMethod->CalibCoeff[0][1] = (FLOAT) 1.0;
        InitialMethod->CalibCoeff[0][2] = (FLOAT) 0.0;
        InitialMethod->CalibCoeff[0][3] = (FLOAT) 0.0;
        CalCoeff[0] = 0.0;
        CalCoeff[1] = 1.0;
        CalCoeff[2] = 0.0;
        CalCoeff[3] = 0.0;
        }

      if (err)
        {
        ReturnVal = FIELD_VALIDATE_WARNING;
        }
      else
        {
        CalibUnitsFlag = 0;
        InitialMethod->CalibUnits[0] = COUNTS;
        }
    break;
    case CAL_ACTION_EXTRACT:
      if (EntryIndex == -1)
        {
        error(err = ERROR_BAD_CURVE_BLOCK, EntryIndex);
        ReturnVal = FIELD_VALIDATE_WARNING;
        }
      else if (err = ExtractCalPointsFromCurve(&MainCurveDir, EntryIndex,
        CurveIndex))
        {
        ReturnVal = FIELD_VALIDATE_WARNING;
        }
      /* fall through to calibration */

    case CAL_ACTION_CALIBRATE:
      {
      degree_of_fit = (CalibDegreeFlag + 2);

      /* don't allow calibration to pixel units */
      if ((CalibActionFlag == CAL_ACTION_CALIBRATE) && (CalUnits == COUNTS))
        {
        error(err = ERROR_CAL_PIXEL_UNITS);
        ReturnVal = FIELD_VALIDATE_WARNING;
        break;
        }
      if (verify_list_order())
        {
        if (degree_of_fit <= CalibPointNum)
          {
          least_squares_fit();
          for (i=0; i<MAX_COEFFICIENTS; i++)
            InitialMethod->CalibCoeff[0][i] = (FLOAT)CalCoeff[i];
          ResetOffset();
          for (i=0; i<MAX_COEFFICIENTS; i++)
            CalCoeff[i] = (double)InitialMethod->CalibCoeff[0][i];
          ReturnVal = FIELD_VALIDATE_SUCCESS;
          }
        else
          {
          error(err = ERROR_CAL_TOO_FEW_POINTS, degree_of_fit);
          ReturnVal = FIELD_VALIDATE_WARNING;
          }
        }
      else
        {
        error(err = ERROR_CAL_NOT_MONOTONIC);
        ReturnVal = FIELD_VALIDATE_WARNING;
        }
      }
    break;
    case CAL_ACTION_REUSE:
      {
      /* don't allow calibration to pixel units */
      if ((CalibActionFlag == CAL_ACTION_CALIBRATE) && (CalUnits == COUNTS))
        {
        error(err = ERROR_CAL_PIXEL_UNITS);
        ReturnVal = FIELD_VALIDATE_WARNING;
        }
      else
        ReturnVal = FIELD_VALIDATE_SUCCESS;
      }
    break;
    }

  if ((ReturnVal == FIELD_VALIDATE_SUCCESS) &&
    (CalibActionFlag != CAL_ACTION_EXTRACT) &&
    (CalibActionFlag != CAL_ACTION_UNCALIBRATE))
    {
    for (i=0; i<MAX_COEFFICIENTS; i++)
      CalibCoeff[i] = (FLOAT) CalCoeff[i];

    switch (CalibApplyFlag)
      {
      case CAL_APPLY_TO_ONE_CURVE:
        if (EntryIndex != -1)
          {
          EntryIndex =
            CursorStatus[ActiveWindow].EntryIndex;
          CurveIndex =
            CursorStatus[ActiveWindow].FileCurveNumber;
          if (ApplyCalibrationToCurve(&MainCurveDir,
            EntryIndex,
            CurveIndex,
            CalibCoeff,
            CalUnits))
            {
            ReturnVal = FIELD_VALIDATE_WARNING;
            }
          }
      break;

      /* apply to pixels will also replace X values of all curves on screen */
      case CAL_APPLY_TO_PIXELS:
      case CAL_APPLY_TO_ALL_CURVES:
        Found = FindFirstPlotBlock(&MainCurveDir, &EntryIndex,
                                  &CurveIndex, &UTDisplayCurve, ActiveWindow);
        while (Found && (err == ERROR_NONE))
          {
          if (err = ApplyCalibrationToCurve(&MainCurveDir, EntryIndex,
                                            CurveIndex, CalibCoeff, CalUnits))
            {
            ReturnVal = FIELD_VALIDATE_WARNING;
            }
          Found = FindNextPlotCurve(&MainCurveDir, &EntryIndex,
            &CurveIndex, &UTDisplayCurve, ActiveWindow);
          }

        if (CalibApplyFlag == CAL_APPLY_TO_PIXELS)
          {
          for (i = 0; i < MAX_COEFFICIENTS; i++) /* store the results */
            InitialMethod->CalibCoeff[0][i] = (FLOAT) CalCoeff[i];

          InitialMethod->CalibUnits[0] = CalUnits;
          if (CalUnits == RSHIFT)
            InitialMethod->Excitation = (FLOAT) ExcitationWaveLength;
          }
      break;
      }
    }

  MessageWindow = release_message_window(MessageWindow);

  if ((CalibActionFlag != CAL_ACTION_EXTRACT) &&
    (ReturnVal != FIELD_VALIDATE_WARNING))
    {
    strcpy(Plots[CAL_PLOTBOX].x.legend, WaveLengthOptions[CalibUnitsFlag]);
    Plots[CAL_PLOTBOX].x.units = CalUnits;
    if (AutoScalePlotBox(&MainCurveDir, &Plots[CAL_PLOTBOX], CAL_PLOTBOX))
      return FIELD_VALIDATE_WARNING;

    erase_mouse_cursor();

    if ((CalUnits != COUNTS) && Found) /* MLM Don't get data if no curve */
      {
      if (GetDataPoint(&MainCurveDir,
        CursorStatus[ActiveWindow].EntryIndex,
        CursorStatus[ActiveWindow].FileCurveNumber,
        CursorStatus[ActiveWindow].PointIndex,
        &CursorStatus[ActiveWindow].X,
        &CursorStatus[ActiveWindow].Y,
        FLOATTYPE, &prefBuf))
        {
        return FIELD_VALIDATE_WARNING;
        }
      }
    else
      CursorStatus[ActiveWindow].X =
      (FLOAT)CursorStatus[ActiveWindow].PointIndex;

    Replot(ActiveWindow);
    RemoveGraphCursor();
    }
  draw_form_fields();

  if (CalibPointNum)
    redraw_scroll_form(&X_CalScrollForm, 0, SCROLL_FORM_ACTIVE_FIELD);

  replace_mouse_cursor();

  return ReturnVal;
}

/* Main module, called from menu. */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN X_CalibrationFormInit(void)
{
  SHORT i;

  NoAction = TRUE;  // will be reset if GO is pressed

  for (i=0; i<MAX_COEFFICIENTS; i++)
    CalCoeff[i] = InitialMethod->CalibCoeff[0][i];

  ExcitationWaveLength = (FLOAT) InitialMethod->Excitation;

  CalibUnitsFlag = 0;
  for (i=0; i < szWlUnitTbl; i++)
    {
    if (WaveLengthUnitTable[i] == InitialMethod->CalibUnits[0])
      {
      CalibUnitsFlag = i;
      break;
      }
    }

  check_calib_unit(&CalibUnitsFlag, NULL);

  strcpy(Plots[CAL_PLOTBOX].x.legend, WaveLengthOptions[CalibUnitsFlag]);
  // Plot in currently selected calibration units
  Plots[CAL_PLOTBOX].x.units = WaveLengthUnitTable[CalibUnitsFlag];
  
  return InitSplitForm(&X_CalibrationForm, &InCalibForm, get_new_points);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN X_CalibrationFormExit(void)
{
  
  InitialMethod->Excitation = (FLOAT)ExcitationWaveLength;

  if (ExitSplitForm(&InCalibForm))
    return TRUE;

  return FALSE;
}

// New function for use by autopeak module.  RAC August 24, 1990.
// Add a calibration point and redisplay the scrolling form of cal points.
// is_first TRUE means delete all previous calibration points and then
// add the new one.  is_first FALSE means just add a new point.
// Change the clipping rectangle to full screen so that redrawing the
// scrolling form will be visible on the screen.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void add_calibration_point(double x_pix, double x_cal, BOOLEAN is_first)
{
   CRECT OldClipRect;            
   CRECT ScreenArea;

   ScreenArea.ll.x = 0;
   ScreenArea.ll.y = 0;
   ScreenArea.ur.x = screen.LastVDCXY.x;
   ScreenArea.ll.y = screen.LastVDCXY.y;

   CInqClipRectangle(screen_handle, &OldClipRect); 
   CSetClipRectangle(screen_handle, ScreenArea);

   if(is_first)
      CalibPointNum = 0;
//   x_pix = (double)DataPtToChnl ((int)(x_pix+0.5)); /* this already done? */
   insert_point_in_scroll_form(x_pix, x_cal);

   CSetClipRectangle(screen_handle, OldClipRect);  
}

ERR_OMA CalibUnitError(UCHAR SrcUnit, UCHAR DstUnit)
{
  int i, j;

  for (i = 0; i < szWlUnitTbl; i++)
    {
    if (WaveLengthUnitTable[i] == SrcUnit)
      break;
    }
 
  for (j = 0; j < szWlUnitTbl; j++)
    {
    if (WaveLengthUnitTable[j] == DstUnit)
      break;
    }
 
  return error(ERROR_CONVERT_UNITS, WaveLengthOptions[i], WaveLengthOptions[j]);
}

// init FormTable[]
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void registerXCalibForms(void)
{
   FormTable[KSI_XCAL_FORM] =        &X_CalibrationForm;
   FormTable[KSI_XCAL_SCROLL_FORM] = &X_CalScrollForm;
   FormTable[KSI_XCAL_POINT_FORM] =  &GetCalibPointForm;
}
