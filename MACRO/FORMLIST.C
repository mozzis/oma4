/* -----------------------------------------------------------------------
/
/  formlist.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/macro/formlist.c_v   0.19   06 Jul 1992 12:50:00   maynard  $
/  $Log:   J:/logfiles/oma4000/macro/formlist.c_v  $
*/

#include <stdio.h>
#include <string.h>

#include "primtype.h"
#include "ksindex.h"

CHAR MacFormFileName[]  = "oma4000.frm";
CHAR MacFieldFileName[] = "oma4000.fld";

// N.B. !!!!  In order for LookupFormFieldMenu() to find a name in one of
// the following tables, the first 4 characters must be "FORM" for a form,
// or "MENU" for a menu, or the first 3 characters must be "FLD" for a field.

/* form names must be in same order as in ksindex.h */
PCHAR FormNames[] = {

  /* 00 */ "FORM_NONE",
  /* 01 */ "FORM_DA",
  /* 02 */ "FORM_PLOT",
  /* 03 */ "FORM_FORMAT",
  /* 04 */ "FORM_GET_METHOD",
  /* 05 */ "FORM_LIVE_DISK",
  /* 06 */ "FORM_LIVE",
  /* 07 */ "FORM_ACCUM",
  /* 08 */ "FORM_KEYSTROKE",
  /* 09 */ "FORM_BACKGROUND",
  /* 10 */ "FORM_MATH",
  /* 11 */ "FORM_STAT",
  /* 12 */ "FORM_FILE_DIR",
  /* 13 */ "FORM_FD_CURVE",
  /* 14 */ "FORM_FD_FILE",
  /* 15 */ "FORM_FD_CURVE_SET",
  /* 16 */ "FORM_XCAL",
  /* 17 */ "FORM_XCAL_SCROLL",
  /* 18 */ "FORM_XCAL_POINT",
  /* 19 */ "FORM_YCAL",
  /* 20 */ "FORM_YKNOTS",
  /* 21 */ "FORM_SCAN",          // for GRAPHICAL scan set, eliminate
  /* 22 */ "FORM_SPECIAL",       // all except FORM_SCAN (up to CONFIG)
  /* 23 */ "FORM_SLICE",         
  /* 24 */ "FORM_SLICE_SCROLL",
  /* 25 */ "FORM_TRACK",
  /* 26 */ "FORM_TRACK_SCROLL",
  /* 27 */ "FORM_CONFIG",
  /* 28 */ "FORM_BASELINE",
  /* 29 */ "FORM_BL_KNOTS",
  /* 30 */ "FORM_CURSOR_GOTO",
  /* 31 */ "FORM_TAG_CURVES",
  /* 32 */ "FORM_SAVE_TAGGED",
  /* 33 */ "FORM_SPECTROGRAPH",
  /* 33 */ "FORM_GETOFFSETPOINT",
  /* 34 */ "FORM_RAPDA",
  /* 35 */ "FORM_REGIONS",
       
   // menu names

   "MENU_FILES",          "MENU_PLOT",           "MENU_RUN",
   "MENU_MATH",           "MENU_CALIB",          "MENU_MAIN",
} ;


// Field names must be in same order as in ksindex.h
PCHAR FieldNames[] = {

// DA_FormFields

   "FLD_MEMS",
   "FLD_START_MEM",
   "FLD_DA_MODE",
   "FLD_DA_NAME",
   "FLD_SCANS",
   "FLD_IGN_SCANS",
   "FLD_PREP",
   "FLD_ET",

   "FLD_TSOURCE",
   "FLD_RS_IMODE",
   "FLD_FPWIDTH",
   "FLD_FPDELAY",
   "FLD_DELINC",
   "FLD_DELRANGE",
   "FLD_TCOUNT",
   "FLD_AUDIO",
   "FLD_TTHRESH",
   "FLD_TVOLTS",            

   "FLD_DET_TEMP",
   "FLD_COOL_ONOFF",
   "FLD_PIX_TIME_OPTS",
   "FLD_SFT_TIME_OPTS",
   "FLD_64_SCOMP",
   "FLD_KPCLEAN_SEL",

   "FLD_64_SYNC_OPTS",
   "FLD_OPEN_SYNC",
   "FLD_CLOSE_OPTS",
   "FLD_SHUTTER_OPTS",

// PlotSetupFormFields

   "FLD_PLOT_TITLE",        "FLD_ACTIVE_WIN",          "FLD_X_LBL",
   "FLD_X_MIN",             "FLD_X_MAX",               "FLD_X_UNITS",
   "FLD_Y_LBL",             "FLD_Y_MIN",               "FLD_Y_MAX",
   "FLD_Y_UNITS",           "FLD_Z_LBL",               "FLD_Z_MIN",
   "FLD_Z_MAX",             "FLD_Z_UNITS",             "FLD_Z_SIDE_OPTS",
   "FLD_Z_PER_X",           "FLD_Z_PER_Y",             "FLD_PLOT_METHOD",
   "FLD_AUTOSCALE_X",       "FLD_AUTOSCALE_Y",         "FLD_AUTOSCALE_Z",
   "FLD_WIN_STYLE",         "FLD_PK_LABEL",            "FLD_LPLOT_MODE",
   "FLD_PCOLOR_MODE",       "FLD_ACTIVE_PLOT",

// chformat_formFields

   "FLD_XLATE_IN_NAME",     "FLD_XLATE_START",         "FLD_XLATE_COUNT",
   "FLD_XLATE_OUT_NAME",    "FLD_XLATE_OPTS",          "FLD_XLATE_GO",

// getmethodhdrFields

   "FLD_GM_OUT_NAME",       "FLD_GM_DESC",             "FLD_GM_OPTS",
   "FLD_GM_GO",

// LIVE_TO_DISK_FORMFields

   "FLD_LD_OUT_NAME",       "FLD_LD_REPS",             "FLD_LD_OPTS",
   "FLD_LD_GO",

// LiveFormFields

   "FLD_LIVE_FILTER",       "FLD_LIVE_REF_CURVES",
   "FLD_LIVE_OUT_NAME",     "FLD_LIVE_OPTS",

// AccumFormFields

   "FLD_ACCUM_OUT_NAME",

// KeyStrokeFormFields

   "FLD_KS_NAME",           "FLD_KS_OPTS",             "FLD_KS_DELAY",
   "FLD_KS_GO",

// BackGroundFields

   "FLD_BGRND_NAME",        "FLD_BGRND_START",         "FLD_BGRND_COUNT",
   "FLD_BGRND_GO",

// MathFormFields

   "FLD_OP1_NAME",          "FLD_OP1_START",           "FLD_OP1_COUNT",
   "FLD_OPERATOR",          "FLD_OP2_NAME",            "FLD_OP2_START",
   "FLD_OP2_COUNT",         "FLD_RES_NAME",            "FLD_RES_START",
   "FLD_RES_COUNT",         "FLD_MATH_GO",

// StatFormFields

   "FLD_SOURCE",            "FLD_CRVSTART",            "FLD_CRVCOUNT", 
   "FLD_PNTSTART",          "FLD_PNTCOUNT",            "FLD_GOCALC",

// scrlltst ControlFields

   "FLD_FD_FILESPEC",

// scrlltst CurveScrollFields

   "FLD_FD_CURVE_CHOICE",

// scrlltst FileScrollFields

   "FLD_FD_FILE_CHOICE",

// scrlltst UserFormFields

   "FLD_FD_NAME",           "FLD_FD_DESC",             "FLD_FD_OPTS",
   "FLD_FD_GO",             "FLD_FD_SRC_START",        "FLD_FD_SRC_COUNT",
   "FLD_FD_DST_INDEX",      "FLD_FD_DST_START",        "FLD_FD_WIN_1",
   "FLD_FD_WIN_2",          "FLD_FD_WIN_3",            "FLD_FD_WIN_4",
   "FLD_FD_WIN_5",          "FLD_FD_WIN_6",            "FLD_FD_WIN_7",
   "FLD_FD_WIN_8",

// X_CalibrationFormFields

   "FLD_XCAL_UNITS",        "FLD_XCAL_EXCIT",          "FLD_XCAL_DEGREE",
   "FLD_XCAL_SCOPE",        "FLD_XCAL_DEL_PT",         "FLD_XCAL_ACTION",
   "FLD_XCAL_GO",           "FLD_XCAL_PARAM_1",        "FLD_XCAL_PARAM_2",
   "FLD_XCAL_PARAM_3",      "FLD_XCAL_PARAM_4",

// X_CalScrollFormFields

   "FLD_XCAL_X_VAL",

// GetCalibPointFormFields

   "FLD_XCAL_GP_VAL",

// YCalibrationFormFields

   "FLD_LAMP_DATA",
   "FLD_LAMP_UNITS",
   "FLD_LAMP_ACT",
   "FLD_LAMP_CURVE",
   "FLD_CORR_CURVE",
   "FLD_CORR_CREATE",
   "FLD_REAL_CURVE",
   "FLD_REAL_APPLY",
   "FLD_ADDKNOT",
   "FLD_DELKNOT",

// YKnot Scroll form fields
  
   "FLD_XKNOT_SCR",
   "FLD_YKNOT_SCR",

// ScanSetupFormFields

// use the following only for GRAPHICAL scan setup
//   "FLD_SLICES",            "FLD_SLICE_MODE",          "FLD_TRACKS",
//   "FLD_TRACK_MODE",        "FLD_SHIFT_MODE",          "FLD_ACTIVE_X",
//   "FLD_ACTIVE_Y",          "FLD_ANTI_BLOOM",          "FLD_OUTPUT_REG",

// ScanSetupFormFields

   "FLD_SLICES",            "FLD_SLICE_MODE",            "FLD_TRACKS",
   "FLD_TRACK_MODE",
  
// SpecialsFormFields (scan setup)
   "FLD_ANTI_BLOOM",
   "FLD_OUTPUT_REG",
   "FLD_STREAKMODE",
   "FLD_STREAKROWS",
   "FLD_PRESCAN",   
   "FLD_SHIFT_MODE",
   "FLD_ACTIVE_X",
   "FLD_ACTIVE_Y",

// SliceForm

   "FLD_X0",                "FLD_DELTA_X",

// SliceScrollFormFields

   "FLD_SCROLL_X0",         "FLD_SCROLL_DELTA_X",

// TrackForm

   "FLD_Y0",                "FLD_DELTA_Y",

// TrackScrollFormFields

   "FLD_SCROLL_Y0",         "FLD_SCROLL_DELTA_Y",

// ================================ end of scan setup form fields ==========
   
// ConfigForm

   "FLD_C_ADDR_1",          "FLD_C_ADDR_2",            "FLD_C_ADDR_3",
   "FLD_C_ADDR_4",          "FLD_C_ADDR_5",            "FLD_C_ADDR_6",
   "FLD_C_ADDR_7",          "FLD_C_ADDR_8",            "FLD_C_READSYS_OPTS",
   "FLD_C_GO",              "FLD_C_RUN",               "FLD_C_LINE",

   "FLD_PIA_READ_0",
   "FLD_PIA_READ_1",
   "FLD_PIA_READ_2",
   "FLD_PIA_READ_3",
   "FLD_PIA_READ_4",
   "FLD_PIA_READ_5",
   "FLD_PIA_READ_6",
   "FLD_PIA_READ_7",
  
   "FLD_PIA_WRITE_0",
   "FLD_PIA_WRITE_1",
   "FLD_PIA_WRITE_2",
   "FLD_PIA_WRITE_3",
   "FLD_PIA_WRITE_4",
   "FLD_PIA_WRITE_5",
   "FLD_PIA_WRITE_6",
   "FLD_PIA_WRITE_7",

// baseline_sub_fields

   "FLD_BL_ACTION",         "FLD_BL_Z",                "FLD_BL_THRESHOLD",
   "FLD_BL_NAME",           "FLD_BL_SHOW_CURVE",       "FLD_BL_SHOW_BL",
   "FLD_BL_SHOW_SUM",       "FLD_BL_SHOW_PEAKS",       "FLD_BL_SHOW_ONE",
   "FLD_BL_DEL_KNOT",
  
// knots_form_fields

   "FLD_BL_KNOT_YVAL",

// CursorGoToFormFields

   "FLD_CGT_X",             "FLD_CGT_Z",               "FLD_CGT_GO",

// tag curve form fields

   "FLD_TC_ZSTART",         "FLD_TC_DELTA",            "FLD_TC_GO",

// save tagged curve form fields

   "FLD_ST_NAME",           "FLD_ST_GO",

// spectrograph form fields

   "FLD_SPEC_SELECT",       
   "FLD_SPEC_RESET",
   "FLD_SPEC_GRATING",      
   "FLD_SPEC_GETLAM",         
   "FLD_SPEC_OFFPXL",         
   "FLD_SPEC_OFFWVL",         
   "FLD_SPEC_SETOFFSET",
   "FLD_SPEC_GO",
   "FLD_SPEC_DISP",         

// rapda form fields 
   "FLD_REGIONS",           "FLD_BASET",               "FLD_ARRSIZE",
   
   "FLD_REGX0",             "FLD_REGSIZE",             "FLD_REGET",
} ;

BOUNDS FormBounds[] =
{
   {KSI_NO_INDEX, KSI_NO_INDEX},              // NONE
   {KSI_MEMS, KSI_SHUTTER_OPTS},              // DA_FormFields
   {KSI_PLOT_TITLE, KSI_ACTIVE_PLOT},         // PlotSetupFormFields
   {KSI_XLATE_IN_NAME, KSI_XLATE_GO},         // chformat_formFields
   {KSI_GM_OUT_NAME, KSI_GM_GO},              // getmethodhdrFields
   {KSI_LD_OUT_NAME, KSI_LD_GO},              // LIVE_TO_DISK_FORMFields
   {KSI_LVFILT, KSI_LIVE_OPTS},               // LiveFormFields
   {KSI_ACCUM_OUT_NAME, KSI_ACCUM_OUT_NAME},  // AccumFormFields
   {KSI_KS_NAME, KSI_KS_DELAY},               // KeyStrokeFormFields
   {KSI_BGRND_NAME, KSI_BGRND_GO},            // BackGroundFields
   {KSI_MATH_OP1_NAME, KSI_MATH_GO},          // MathFormFields
   {KSI_STAT_SOURCE, KSI_STAT_GO},            // StatFormFields
   {KSI_FD_FILESPEC, KSI_FD_FILESPEC},        // scrlltst ControlFields
   {KSI_FD_CURVE_CHOICE, KSI_FD_CURVE_CHOICE},// scrlltst CurveScrollFields
   {KSI_FD_FILE_CHOICE, KSI_FD_FILE_CHOICE},  // scrlltst FileScrollFields
   {KSI_FD_FILE_NAME, KSI_FD_WINDOW_8},       // scrlltst UserFormFields
   {KSI_XCAL_UNITS, KSI_XCAL_PARAM_4},        // X_CalibrationFormFields
   {KSI_XCAL_X_VAL, KSI_XCAL_X_VAL},          // X_CalScrollFormFields
   {KSI_XCAL_GP_VAL, KSI_XCAL_GP_VAL},        // GetCalibPointFormFields
   {KSI_YCAL_LAMP_DATA, KSI_YCAL_DELKNOT},    // Y_CalibrationFormFields
   {KSI_YCAL_XKNOT_SCR, KSI_YCAL_YKNOT_SCR},  // Y_KnotFormFields
   {KSI_SLICES, KSI_TRACK_MODE},              // ScanSetupFormFields
   {KSI_ANTIBLOOM, KSI_ACTIVEY},              // SpecialsFormFields
   {KSI_X0, KSI_DELTA_X},                     // SliceForm
   {KSI_SCROLL_X0, KSI_SCROLL_DELTA_X},       // SliceScrollFormFields
   {KSI_Y0, KSI_DELTA_Y},                     // TrackForm
   {KSI_SCROLL_Y0, KSI_SCROLL_DELTA_Y},       // TrackScrollFormFields
   {KSI_C_ADDR_1, KSI_PIA_WRITE_7 },          // ConfigForm
   {KSI_BL_ACTION, KSI_BL_DEL_KNOT},          // baseline_sub_fields
   {KSI_BL_KNOT_Y_VAL, KSI_BL_KNOT_Y_VAL},    // knots_form_fields
   {KSI_CGT_X, KSI_CGT_GO},                   // CursorGoToFormFields
   {KSI_TC_ZSTART, KSI_TC_GO},                // TagCurveGroupFormFields
   {KSI_ST_FILE_NAME, KSI_ST_GO},             // SaveTaggedFormFields
   {KSI_SPEC_SELECT, KSI_SPEC_DISP},          // Spectrograph form fields
   {KSI_SPGRAPH_GP_VAL, KSI_SPGRAPH_GP_VAL},  // GetOffsetPoint form fields
   {KSI_REGIONS, KSI_ARRSIZE},                // rapda form fields
   {KSI_REG_X0, KSI_REGET},                   // region form fields
};

int main(void)
{
  FILE *hForm;
  FILE *hField;

  SHORT i;
  SHORT StrLen;
  SHORT FillLen;
  SHORT IntBuf;

  if ((hForm = fopen(MacFormFileName, "wb")) == NULL)
    {
    printf("open error, file : %s\n", MacFormFileName);
    return 1;
    }

  for (i=0; i<NUM_FORMS + NUM_MENUS; i++)
    {
    StrLen = strlen(FormNames[i]);
    FillLen = FORM_NAME_SIZE - StrLen;
    if (fwrite(FormNames[i], StrLen, 1, hForm) != 1)
      {
      printf("write error, file : %s\n", MacFormFileName);
      return 1;
      }

    IntBuf = 0;
    while (FillLen > 0)
      {
      if (fwrite(&IntBuf, 1, 1, hForm) != 1)
        {
        printf("write error, file : %s\n", MacFormFileName);
        return 1;
        }
      FillLen--;
      }
    }
  fclose(hForm);

  if ((hField = fopen(MacFieldFileName, "wb")) == NULL)
    {
    printf("open error, file : %s\n", MacFieldFileName);
    return 1;
    }

  /* write out the number of forms */
  IntBuf = NUM_FORMS;
  if (fwrite(&IntBuf, sizeof(int), 1, hField) != 1)
    {
    printf("write error A, file : %s\n", MacFieldFileName);
    return 1;
    }

  /* write out the field boundaries for each form */
  for (i=0; i<NUM_FORMS; i++)
    {
    if (fwrite(&FormBounds[i], sizeof(int), 2, hField) != 2)
      {
      printf("write error B %d, file : %s\n", i, MacFieldFileName);
      return 1;
      }
    }

  for (i=0; i<NUM_FIELDS; i++)
    {
    StrLen = strlen(FieldNames[i]);
    FillLen = FORM_NAME_SIZE - StrLen;
    if (fwrite(FieldNames[i], StrLen, 1, hField) != 1)
      {
      printf("write error C %d, file : %s\n", i, MacFieldFileName);
      return 1;
      }

    IntBuf = 0;
    while (FillLen > 0)
      {
      if (fwrite(&IntBuf, 1, 1, hField) != 1)
        {
        printf("write error E %d, file : %s\n", i, MacFieldFileName);
        return 1;
        }
      FillLen--;
      }
    }
  fclose(hField);
  return 0;
}
