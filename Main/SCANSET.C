/**********************************************************************/
/*                                                                    */
/*       scanset.c                                                    */
/*       written by MLM 1/22/90                                       */
/*       forms and routines for scan setup of 1461                    */
/*       used by oma4000 software                                     */
/*                                                                    */
/*  Copyright (c) 1990,  EG&G Instruments Inc.                        */
/*
/  $Header:   J:/logfiles/oma4000/main/scanset.c_v   0.28   07 Jul 1992 17:11:02   maynard  $
/  $Log:   J:/logfiles/oma4000/main/scanset.c_v  $
*/
/**********************************************************************/

#include "scanset.h"
#include "omaform.h"
#include "oma4driv.h"
#include "detsetup.h"
#include "helpindx.h"
#include "ksindex.h"
#include "formtabs.h"
#include "omaerror.h"
#include "driverrs.h"
#include "forms.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

// private functions ------------------------------------------------------

PRIVATE BOOLEAN ScanSetFormInit(void);

// tells form system when to stop looking for more virtual rows in the
// point scrolling form 
PRIVATE BOOLEAN validate_point_index(int index);

// tells form system when to stop looking for more virtual rows in the
// track scrolling form
PRIVATE BOOLEAN validate_track_index(int index);

// tells form system whether to draw the scrolling (for non-uniform) or
// non-scrolling (for uniform) version of the point menu.                                            */
PRIVATE unsigned char point_form_entry_select(FORM_CONTEXT * dummy);

// tells form system whether to draw the scrolling (for random) or
// non-scrolling (for contiguous) version of the track menu.                                            */
PRIVATE unsigned char track_form_entry_select(FORM_CONTEXT * dummy);

// called to update the form when certain toggle fields are selected
PRIVATE void refresh_form(void);

// set detector ActiveX value from det_setup -> ActiveX
PRIVATE int form_set_ActiveX(void);

// set detector ActiveY value from det_setup -> ActiveY
PRIVATE int form_set_ActiveY(void);

PRIVATE int form_set_X0(void);
PRIVATE int form_set_Y0(void);

PRIVATE int form_set_DeltaX(void);
PRIVATE int form_set_DeltaY(void);

PRIVATE int form_set_CurrentTrack(void);
PRIVATE int form_set_CurrentPoint(void);

PRIVATE int form_set_pointmode(void);
PRIVATE int form_set_trackmode(void);

PRIVATE int form_set_shiftmode(void);
PRIVATE int form_set_StreakMode(void);
PRIVATE int form_set_ExpRows(void);
PRIVATE int form_set_Prescan(void);

// set the total number of points in a frame.
PRIVATE int form_set_points(void);

// set the total number of tracks in a frame.
PRIVATE int form_set_tracks(void);

// set the detector's anti bloom value
PRIVATE int form_set_antiBloom(void);

// set the output register to A or B
PRIVATE int form_set_output_reg(void);

// ------------------------------------------------------------------------

PRIVATE int form_ActiveX;
PRIVATE int form_ActiveY;

PRIVATE int form_DeltaX;
PRIVATE int form_DeltaY;

PRIVATE int form_X0;
PRIVATE int form_Y0;

PRIVATE int form_trackmode;
PRIVATE int form_pointmode;

PRIVATE int form_CurrPoint;
PRIVATE int form_CurrTrack;

PRIVATE int form_points;
PRIVATE int form_tracks;

PRIVATE float frame_time;
PRIVATE float pretrig_time;

PRIVATE int form_StreakMode;
PRIVATE int form_StreakSize;
PRIVATE int form_Prescan;

// toggle field index [0,10]  0%, 10%, 20%, ... 90%, 100%
PRIVATE int form_antiBloom;

// toggle field index [0,1] A or B output register
PRIVATE int form_outputReg;

// toggle field index [0, 3] for shift mode
PRIVATE int form_shiftmode;

static enum { DGROUP_DO_STRINGS = 1, DGROUP_FORMS,    DGROUP_TOGGLES,
              DGROUP_CODE,           DGROUP_SCANSET
};

PRIVATE DATA DO_STRING_Reg[] = {
   /* 0 */  { "#Points",         0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 1 */  { "Points",          0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 2 */  { "#Tracks",         0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 3 */  { "Tracks",          0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 4 */  { "Point",           0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 5 */  { "Starts",          0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 6 */  { "at Column",       0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 7 */  { "Columns",         0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 8 */  { "in Point",        0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 9 */  { "Track",           0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 10 */ { "Starts",          0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 11 */ { "at Row",          0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 12 */ { "Rows",            0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 13 */ { "in Track",        0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 14 */ { "Tracks",          0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 15 */ { "All",             0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 16 */ { "Array Size",      0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 17 */ { "X",               0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 18 */ { "Anti-Bloom",      0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 19 */ { "%",               0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 20 */ { "Shift Register",  0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 21 */ { "Frame Time",      0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 22 */ { "s.",              0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 23 */ { "Shift Reg.",      0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 24 */ { "Streak Mode",     0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 25 */ { "Rows Exposed",    0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 26 */ { "Pretrig Tracks",  0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 27 */ { "Pretrig Time",    0, DATATYP_STRING, DATAATTR_PTR, 0 },
};

PRIVATE FORM SliceForm;
PRIVATE FORM SliceScrollForm;
PRIVATE FORM TrackForm;
PRIVATE FORM TrackScrollForm;
PRIVATE FORM SpecialsForm;
               
PRIVATE DATA FORMS_Reg[] = {
   { &SliceForm,       0, DATATYP_VOID, DATAATTR_PTR, 0 },
   { &SliceScrollForm, 0, DATATYP_VOID, DATAATTR_PTR, 0 },
   { &TrackForm,       0, DATATYP_VOID, DATAATTR_PTR, 0 },
   { &TrackScrollForm, 0, DATATYP_VOID, DATAATTR_PTR, 0 },
   { &SpecialsForm,    0, DATATYP_VOID, DATAATTR_PTR, 0 }
};

PRIVATE char * pointTrack_mode_options[] = { "Uniform", "Random" };

PRIVATE char * shift_mode_options[] = {
   "º", "Á"
};

PRIVATE char * StreakModeOptions[] = { "Off", "On", "Dual" };

PRIVATE char * antiBloomOptions[] = {
   "  0", " 10", " 20", " 30", " 40",
   " 50", " 60", " 70", " 80", " 90", "100"
};

PRIVATE char * outputOptions[] = { "A  ", "B  ", "Dual" }; // output register

PRIVATE DATA TOGGLES_Reg[] = {
  { shift_mode_options,      0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 }, //0 
  { pointTrack_mode_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 }, //1
  { antiBloomOptions,        0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 }, //2
  { outputOptions,           0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 }, //3
  { StreakModeOptions,       0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 }, //4
};

PRIVATE DATA SCAN_SET_Reg[] = {

/* 0 */   { &form_ActiveX,   0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 1 */   { &form_ActiveY,   0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 2 */   { &form_DeltaX,    0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 3 */   { &form_DeltaY,    0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 4 */   { &form_X0,        0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 5 */   { &form_Y0,        0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 6 */   { &form_trackmode, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 7 */   { &form_pointmode, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 8  */  { &form_CurrPoint, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 9  */  { &form_CurrTrack, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 10 */  { &form_points,    0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 11 */  { &form_tracks,    0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 12 */  { &form_shiftmode, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 13 */  { &form_antiBloom, 0, DATATYP_INT, DATAATTR_PTR, 0 },        
/* 14 */  { &form_outputReg, 0, DATATYP_INT, DATAATTR_PTR, 0 },        
/* 15 */  { &frame_time,     0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
/* 16 */  { &form_StreakMode,0, DATATYP_INT, DATAATTR_PTR, 0 },        
/* 17 */  { &form_StreakSize,0, DATATYP_INT, DATAATTR_PTR, 0 },        
/* 18 */  { &form_Prescan,   0, DATATYP_INT, DATAATTR_PTR, 0 },        
/* 19 */  { &pretrig_time,   0, DATATYP_FLOAT, DATAATTR_PTR, 0 },

};

PRIVATE EXEC_DATA CODE_Reg[] = {
 /* 0 */ { CAST_CHR2INT scroll_entry_field, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 1 */ { CAST_CHR2INT scroll_up_field,    0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 2 */ { CAST_CHR2INT scroll_down_field,  0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 3 */ { CAST_CHR2INT validate_point_index, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 4 */ { CAST_CHR2INT validate_track_index, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 5 */ { CAST_CHR2INT point_form_entry_select, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 6 */ { CAST_CHR2INT track_form_entry_select, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 7 */ { form_set_ActiveX,        0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 8 */ { form_set_ActiveY,        0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 9 */ { form_set_X0,             0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 10*/ { form_set_Y0,             0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 11*/ { form_set_DeltaX,         0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 12*/ { form_set_DeltaY,         0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 13*/ { form_set_pointmode,      0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 14*/ { form_set_trackmode,      0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 15*/ { form_set_shiftmode,      0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 16*/ { form_set_points,         0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 17*/ { form_set_tracks,         0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 18*/ { ScanSetFormInit,         0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 19*/ { form_set_antiBloom,      0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 20*/ { form_set_output_reg,     0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 21*/ { form_set_CurrentPoint,   0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 22*/ { form_set_CurrentTrack,   0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 23*/ { form_set_StreakMode,     0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 24*/ { form_set_ExpRows,        0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 25*/ { form_set_Prescan,        0, DATATYP_CODE, DATAATTR_PTR, 0 },
};

// field names

static enum {
   NMR_SLC_LABEL,
   SLCS_LABEL,
   NMR_TRK_LABEL,
   TRKS_LABEL,
   SLC_NMR_LABEL,
   X0_LABEL_1,
   X0_LABEL_2,
   DX_LABEL_1,
   DX_LABEL_2,
   TRK_NMR_LABEL,
   Y0_LABEL_1,
   Y0_LABEL_2,
   DY_LABEL_1,
   DY_LABEL_2,
   FTIME_LABEL,
   FTIME_SECS,
   PTTIME_LABEL,
   PTTIME_SECS,
   FR_TIME,
   PT_TIME,
   NMR_SLCS,
   SLC_MODE,
   NMR_TRKS,
   TRK_MODE,
   SLC_LOGIC,
   SLC_FRM,
   SLC_SCR_FORM,
   SPEC_FORM,
   TRK_LOGIC,
   TRK_FRM,
   TRK_SCR_FORM   
};
   
PRIVATE FIELD ScanSetupFormFields[] = {

   label_field(NMR_SLC_LABEL,
   DGROUP_DO_STRINGS, 0,     /*  "#Points" */
   1, 2, 7),

   label_field(SLCS_LABEL,
   DGROUP_DO_STRINGS, 1,     /*  "Points" */
   1, 16, 6),

   label_field(NMR_TRK_LABEL,
   DGROUP_DO_STRINGS, 2,     /*  "#Tracks" */
   1, 49, 7),

   label_field(TRKS_LABEL,
   DGROUP_DO_STRINGS, 3,     /*  "Tracks" */
   1, 63, 6),

   label_field(SLC_NMR_LABEL,
   DGROUP_DO_STRINGS, 4,     /*  "Point" */
   4, 2, 5),

   label_field(X0_LABEL_1,
   DGROUP_DO_STRINGS, 5,     /*  "Starts" */
   3, 11, 5),

   label_field(X0_LABEL_2,
   DGROUP_DO_STRINGS, 6,     /*  "at Column" */
   4, 9, 9),

   label_field(DX_LABEL_1,
   DGROUP_DO_STRINGS, 7,     /*  "Columns" */
   3, 19, 7),

   label_field(DX_LABEL_2,
   DGROUP_DO_STRINGS, 8,     /*  "in Point" */
   4, 19, 8),

   label_field(TRK_NMR_LABEL,
   DGROUP_DO_STRINGS, 9,     /*  "Track"  */
   4, 54, 6),

   label_field(Y0_LABEL_1,
   DGROUP_DO_STRINGS, 10,    /*  "Starts" */
   3, 62, 6),

   label_field(Y0_LABEL_2,
   DGROUP_DO_STRINGS, 11,    /*  "at Row" */
   4, 62, 6),

   label_field(DY_LABEL_1,
   DGROUP_DO_STRINGS, 12,    /*  "Rows" */
   3, 72, 4),

   label_field(DY_LABEL_2,
   DGROUP_DO_STRINGS, 13,    /*  "in Track" */
   4, 70, 8),

   label_field(FTIME_LABEL,
    DGROUP_DO_STRINGS, 21,   /*  "Frame Time" */
    3, 30, 10),
                           
   label_field(FTIME_SECS,   
    DGROUP_DO_STRINGS, 22,   /*  "s." */
    3, 50, 2),
                           
   label_field(PTTIME_LABEL,
    DGROUP_DO_STRINGS, 27,   /*  "Pretrig Time" */
    4, 30, 12),
                           
   label_field(PTTIME_SECS,  
    DGROUP_DO_STRINGS, 22,   /*  "s." */
    4, 50, 2),
                           
   field_set(FR_TIME,
   FLDTYP_SCL_FLOAT,
   FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR | FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_SCANSET, 15,                  /*  frame time */
   0, 0,
   0, 0,
   2, 0,
   3, 41, 9,
   PT_TIME, PT_TIME, FR_TIME, FR_TIME,
   FR_TIME, FR_TIME, FR_TIME, FR_TIME),

   field_set(PT_TIME,
   FLDTYP_SCL_FLOAT,
   FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR | FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_SCANSET, 19,                  /*  pretrig time */
   0, 0,
   0, 0,
   1, 0,
   4, 43, 7,
   NMR_SLCS, NMR_SLCS, PT_TIME, PT_TIME,
   PT_TIME, PT_TIME, PT_TIME, PT_TIME),

   /*  end of display only fields */

   field_set(NMR_SLCS,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID,
   KSI_SLICES,
   SCAN_HBASE + 0,
   DGROUP_SCANSET, 10,
   0, 0,
   DGROUP_CODE, 16,
   0, 0,                /*  #Points * */
   1, 10, 4,
   EXIT, NMR_SLCS, SLC_LOGIC, SLC_LOGIC,
   TRK_MODE, SLC_MODE, TRK_MODE, SLC_MODE),

   field_set(SLC_MODE,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_SLICE_MODE,
   SCAN_HBASE + 1,
   DGROUP_SCANSET, 7,
   DGROUP_TOGGLES, 1,  /*  pointTrack_mode_options */
   DGROUP_CODE, 13,    /*  Slicemode */
   0, 2,
   1, 23, 7,
   EXIT, SLC_MODE, SLC_LOGIC, SLC_LOGIC,
   NMR_SLCS, NMR_TRKS, NMR_SLCS, NMR_TRKS),

   field_set(NMR_TRKS,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID,
   KSI_TRACKS,
   SCAN_HBASE + 2,
   DGROUP_SCANSET, 11,   /*  form_tracks */
   0, 0,
   DGROUP_CODE, 17,
   0, 0,
   1, 57, 4,
   EXIT, NMR_TRKS, TRK_LOGIC, TRK_LOGIC,
   SLC_MODE, TRK_MODE, SLC_MODE, TRK_MODE),

   field_set(TRK_MODE,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_TRACK_MODE,
   SCAN_HBASE + 3,
   DGROUP_SCANSET, 6, 
   DGROUP_TOGGLES, 1,    /*  pointTrack_mode_options */
   DGROUP_CODE, 14,      /*  Trackmode */
   0, 2,
   1, 70, 7,
   EXIT, TRK_MODE, TRK_LOGIC, TRK_LOGIC,
   NMR_TRKS, NMR_SLCS, NMR_TRKS, SLC_LOGIC),

   field_set(SLC_LOGIC,
   FLDTYP_LOGIC,
   FLDATTR_DRAW_PERMITTED,
   KSI_NO_INDEX,
   0,
   DGROUP_CODE, 5,      /*  point_form_entry_select */
   0, 0, 0, 0, 0, 0,    /*  SliceLogic */
   4, 33, 1,
   /*  expect either 0 or 1 from point_form_entry_select() */
   SLC_FRM, SLC_SCR_FORM, SPEC_FORM, SPEC_FORM,
   SPEC_FORM, SPEC_FORM, SPEC_FORM, SPEC_FORM),

   field_set(SLC_FRM,
   FLDTYP_FORM,
   FLDATTR_GET_DRAW_PERMISSION,
   KSI_NO_INDEX,
   0,
   DGROUP_FORMS, 0,   /*  SliceForm */
   0, 0, 0, 0, 0, 0,
   4, 33, 1,
   EXIT, SPEC_FORM, NMR_SLCS, SPEC_FORM,
   SPEC_FORM, SPEC_FORM, TRK_LOGIC, SPEC_FORM),

   field_set(SLC_SCR_FORM,
   FLDTYP_FORM,
   FLDATTR_GET_DRAW_PERMISSION,
   KSI_NO_INDEX,
   0,
   DGROUP_FORMS, 1,   /*  SliceScroll */
   0, 0, 0, 0, 0, 0,
   4, 35, 1,
   EXIT, SPEC_FORM, SPEC_FORM, SPEC_FORM,
   SPEC_FORM, SPEC_FORM, TRK_LOGIC, SPEC_FORM),

   field_set(SPEC_FORM,
   FLDTYP_FORM,
   FLDATTR_DRAW_PERMITTED,
   KSI_NO_INDEX,
   SCAN_HBASE + 9,
   DGROUP_FORMS, 4,   /*  SpecialsForm */
   0, 0, 0, 0, 0, 0,
   6, 37, 1,
   EXIT, TRK_FRM, NMR_TRKS, NMR_SLCS,
   SLC_LOGIC, TRK_LOGIC, SLC_LOGIC, TRK_LOGIC),

  field_set(TRK_LOGIC,
   FLDTYP_LOGIC,
   FLDATTR_DRAW_PERMITTED,
   KSI_NO_INDEX,
   0,
   DGROUP_CODE, 6,     /*  track_form_entry_select */
   0, 0, 0, 0, 0, 0,   /*  TrackLogic */
   4, 36, 1,
   /*  expect either 0 or 1 from track_form_entry_select() */
   TRK_FRM, TRK_SCR_FORM, NMR_SLCS, NMR_SLCS,
   NMR_SLCS, NMR_SLCS, NMR_SLCS, NMR_SLCS),

   field_set(TRK_FRM,
   FLDTYP_FORM,
   FLDATTR_GET_DRAW_PERMISSION,
   KSI_NO_INDEX,
   SCAN_HBASE + 9,
   DGROUP_FORMS, 2,   /*  TrackForm */
   0, 0, 0, 0, 0, 0,
   6, 37, 1,
   EXIT, TRK_FRM, NMR_TRKS, NMR_TRKS,
   SLC_LOGIC, SLC_LOGIC, SPEC_FORM, NMR_SLCS),

   field_set(TRK_SCR_FORM,
   FLDTYP_FORM,
   FLDATTR_GET_DRAW_PERMISSION,
   KSI_NO_INDEX,
   SCAN_HBASE + 10,
   DGROUP_FORMS, 3,
   0, 0, 0, 0, 0, 0,          /*  TrackScroll */
   6, 39, 1,
   EXIT, TRK_SCR_FORM, NMR_TRKS, NMR_TRKS,
   SLC_LOGIC, SLC_LOGIC, SPEC_FORM, NMR_SLCS),
};

PRIVATE FORM  ScanSetupForm = {
   0, 0, FORMATTR_BORDER | FORMATTR_EXIT_RESTORE | FORMATTR_VISIBLE |
         FORMATTR_FULLSCREEN,
   0, 0, 0,
   2, 0, 21, 80,
   0, 0,
   { DGROUP_CODE, 18 },
   { 0, 0 },
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(ScanSetupFormFields) / sizeof(FIELD),
   ScanSetupFormFields,
   KSI_SCAN_SETUP,
   0, DO_STRING_Reg, FORMS_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, SCAN_SET_Reg
};

enum { BLOOM_LABEL,
       BLOOM_VAL_LABEL,
       OUTPUT_REG_LABEL,
       STREAK_LABEL,
       STREAK_SIZE_LABEL,
       PRESCAN_LABEL,
       SHIFT_LABEL,
       SERIAL_LABEL,
       ARRAY_LABEL,
       X_LABEL,
       BLOOM_VAL,
       OUTPUT_REG,
       FLD_STREAK,
       STREAK_ROWS,
       FLD_PRESCAN,
       SHIFTMODE,
       ACTVX,
       ACTVY
     };

PRIVATE FIELD SpecialsFields[] = {
   
   label_field(BLOOM_LABEL,
   DGROUP_DO_STRINGS, 18,    /*  "Anti-Bloom" */
   1, 1, 10),

   label_field(BLOOM_VAL_LABEL,
   DGROUP_DO_STRINGS, 19,    /*  "%" */
   1, 20, 1),

   label_field(OUTPUT_REG_LABEL,
   DGROUP_DO_STRINGS, 20,    /*  "Shift Register" */
   3, 1, 14),
   
   label_field(STREAK_LABEL,
   DGROUP_DO_STRINGS, 24,    /*  "Streak Mode" */
   5, 1, 11),

   label_field(STREAK_SIZE_LABEL,
   DGROUP_DO_STRINGS, 25,    /*  "Rows Exposed" */
   6, 1, 12),

   label_field(PRESCAN_LABEL,
   DGROUP_DO_STRINGS, 26,    /*  "Pretrig Tracks" */
   7, 1, 14),

   label_field(SHIFT_LABEL,
   DGROUP_DO_STRINGS, 14,    /*  "Tracks" */
   9, 1, 6),

   label_field(SERIAL_LABEL,
   DGROUP_DO_STRINGS, 23,    /*  "Shift Reg." */
   9, 11, 10),

   label_field(ARRAY_LABEL,
   DGROUP_DO_STRINGS, 16,    /*  "Array Size" */
   11, 1, 10),

   label_field(X_LABEL,
   DGROUP_DO_STRINGS, 17,    /*  "X" */
   11, 17, 1),

   field_set(BLOOM_VAL,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_ANTIBLOOM,
   SCAN_HBASE + 7,
   DGROUP_SCANSET, 13,       /*  form_antiBloom */
   DGROUP_TOGGLES, 2,        /*  antiBloomOptions */
   DGROUP_CODE, 19,          /*  form_set_antiBloom */
   0, 11,
   1, 16, 3,
   EXIT, BLOOM_VAL, EXIT, OUTPUT_REG,
   EXIT, EXIT_DN, EXIT, EXIT_DN),

   field_set(OUTPUT_REG,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_OUT_REG,
   SCAN_HBASE + 8,
   DGROUP_SCANSET, 14,       /*  form_outputReg */
   DGROUP_TOGGLES, 3,        /*  outputOptions   (A or B) */
   DGROUP_CODE, 20,          /*  form_set_output_reg */
   0, 3,
   3, 16, 4,
   EXIT, OUTPUT_REG, BLOOM_VAL, FLD_STREAK,
   EXIT, EXIT_DN, EXIT, EXIT_DN),
   
   field_set(FLD_STREAK,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_STREAKMODE,
   SCAN_HBASE + 9,
   DGROUP_SCANSET, 16,  /*  form_StreakMode */
   DGROUP_TOGGLES, 4,   /*  StreakModeOptionss */
   DGROUP_CODE, 23,     /*  form_set_StreakMode(); */
   0, 3,
   5, 16, 4,
   EXIT, FLD_STREAK, OUTPUT_REG, STREAK_ROWS,
   EXIT, EXIT_DN, EXIT, EXIT_DN),

   field_set(STREAK_ROWS,
   FLDTYP_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_STREAKROWS,
   SCAN_HBASE + 10,
   DGROUP_SCANSET, 17,  /*  form_StreakSize */
   0, 0,
   DGROUP_CODE, 24,     /*  form_set_ExpRows(); */
   0, 0,
   6, 16, 4,
   EXIT, STREAK_ROWS, FLD_STREAK, FLD_PRESCAN,
   EXIT, EXIT_DN, EXIT, EXIT_DN),

   field_set(FLD_PRESCAN,
   FLDTYP_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_PRESCAN,
   SCAN_HBASE + 13,
   DGROUP_SCANSET, 18,  /*  form_Prescan */
   0, 0,
   DGROUP_CODE, 25,     /*  form_set_Prescan(); */
   0, 0,
   7, 16, 4,
   EXIT, FLD_PRESCAN, STREAK_ROWS, SHIFTMODE,
   EXIT, EXIT_DN, EXIT, EXIT_DN),

   field_set(SHIFTMODE,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_SHIFT_MODE,
   SCAN_HBASE + 4,
   DGROUP_SCANSET, 12,  /*  shiftModeIndex */
   DGROUP_TOGGLES, 0,   /*  shift_mode_options */
   DGROUP_CODE, 15,     /*  Shiftmode */
   0, 2,
   9, 8, 1,
   EXIT, SHIFTMODE, STREAK_ROWS, ACTVX,
   EXIT, EXIT_DN, EXIT, EXIT_DN),

   field_set(ACTVX,
   FLDTYP_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_ACTIVEY,
   SCAN_HBASE + 6,
   DGROUP_SCANSET, 0,   /*  det_setup.ActiveY */
   0, 0,
   DGROUP_CODE, 7,      /*  form_set_ActiveY */
   0, 0,
   11, 12, 4,
   EXIT, ACTVX, SHIFTMODE, EXIT_DN,
   EXIT, EXIT_DN, EXIT, ACTVY),

   field_set(ACTVY,
   FLDTYP_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_ACTIVEY,
   SCAN_HBASE + 6,
   DGROUP_SCANSET, 1,   /*  det_setup.ActiveY */
   0, 0,
   DGROUP_CODE, 8,      /*  form_set_ActiveY */
   0, 0,
   11, 19, 4,
   EXIT, ACTVY, SHIFTMODE, EXIT_DN,
   EXIT, EXIT_DN, ACTVX, EXIT_DN),
};

PRIVATE FORM  SpecialsForm = {
   0, 0, FORMATTR_BORDER | FORMATTR_VISIBLE,
   0, 0, 0,
   6, 28, 13, 24,
   0, 0,
   { 0, 0 },
   { 0, 0 },
   COLORS_MENU,
   0, 0, 0, 0,
   sizeof(SpecialsFields) / sizeof(FIELD),
   SpecialsFields,
   KSI_SPECIALS,
   0, DO_STRING_Reg, FORMS_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, SCAN_SET_Reg
};

static enum { ALL_SLCS_LABEL, X0_ALL, DX_ALL };
  
PRIVATE FIELD SliceFormFields[] = {

   field_set(ALL_SLCS_LABEL,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_DO_STRINGS, 15,   /*  "All" */
   0, 0, 0, 0, 4, 0,
   1, 2, 4,
   X0_ALL, X0_ALL, ALL_SLCS_LABEL, ALL_SLCS_LABEL,
   ALL_SLCS_LABEL, ALL_SLCS_LABEL, ALL_SLCS_LABEL, ALL_SLCS_LABEL),

   field_set(X0_ALL,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_X0,
   SCAN_HBASE + 11,   /* X0 */
   DGROUP_SCANSET, 4,
   0, 0,
   DGROUP_CODE, 9,
   0, 0,
   1, 10, 4,
   EXIT, X0_ALL, EXIT, EXIT_DN,
   EXIT, DX_ALL, EXIT, DX_ALL),

   field_set(DX_ALL,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_DELTA_X,
   SCAN_HBASE + 12,   /* DeltaX */
   DGROUP_SCANSET, 2,
   0, 0,
   DGROUP_CODE, 11,
   0, 0,
   1, 19, 4,
   EXIT_DN, DX_ALL, EXIT, EXIT_DN,
   X0_ALL, EXIT_DN, X0_ALL, EXIT_DN),
};

PRIVATE FORM SliceForm = {
   0, 0, FORMATTR_VISIBLE | FORMATTR_BORDER,
   0, 0, 0,
   6, 1, 3, 26,
   0, 0,
   { 0, 0 }, { 0, 0 }, COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(SliceFormFields) / sizeof(SliceFormFields[ 0 ]),
   SliceFormFields,
   KSI_SLICE_FORM,
   0, DO_STRING_Reg, FORMS_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, SCAN_SET_Reg
};

static enum { SLC_INIT_LOGIC, SLC_UP_LOGIC, SLC_DN_LOGIC, SLC_NMR, SLC_X0,
              SLC_DX
};
  
PRIVATE FIELD SliceScrollFormFields[] = {

   field_set(SLC_INIT_LOGIC,
   FLDTYP_LOGIC,
   FLDATTR_NONE,
   KSI_NO_INDEX,
   0,
   DGROUP_CODE, 0,
   0, 0, 0, 0, 0, 0,
   0, 1, 1,
   EXIT, SLC_NMR, SLC_INIT_LOGIC, SLC_INIT_LOGIC,
   SLC_INIT_LOGIC, SLC_INIT_LOGIC, SLC_INIT_LOGIC, SLC_INIT_LOGIC),

   field_set(SLC_UP_LOGIC,
   FLDTYP_LOGIC,
   FLDATTR_NONE,
   KSI_NO_INDEX,
   0,
   DGROUP_CODE, 1,
   0, 0, 0, 0, 0, 0,
   0, 3, 1,
   EXIT, SLC_NMR, SLC_UP_LOGIC, SLC_UP_LOGIC,
   SLC_UP_LOGIC, SLC_UP_LOGIC, SLC_UP_LOGIC, SLC_UP_LOGIC),

   field_set(SLC_DN_LOGIC,
   FLDTYP_LOGIC,
   FLDATTR_NONE,
   KSI_NO_INDEX,
   0,
   DGROUP_CODE, 2,
   0, 0, 0, 0, 0, 0,
   0, 5, 1,
   EXIT, SLC_NMR, SLC_DN_LOGIC, SLC_DN_LOGIC,
   SLC_DN_LOGIC, SLC_DN_LOGIC, SLC_DN_LOGIC, SLC_DN_LOGIC),

   field_set(SLC_NMR,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   SCAN_HBASE + 13,
   DGROUP_SCANSET, 8, /* form_current_point */
   0, 0,
   DGROUP_CODE, 21,
   0, 0,
   0, 1, 4,
   SLC_X0, SLC_X0, SLC_NMR, SLC_NMR,
   SLC_NMR, SLC_NMR, SLC_NMR, SLC_NMR),

   field_set(SLC_X0,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_SCROLL_X0,
   SCAN_HBASE + 14,
   DGROUP_SCANSET, 4, 
   0, 0,
   DGROUP_CODE, 9,
   0, 0,            /*  X0 */
   0, 10, 4,
   EXIT, SLC_X0, SLC_UP_LOGIC, SLC_DN_LOGIC,
   SLC_NMR, SLC_DX, EXIT, SLC_DX),

   field_set(SLC_DX,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_SCROLL_DELTA_X,
   SCAN_HBASE + 15,
   DGROUP_SCANSET, 2, 
   0, 0,
   DGROUP_CODE, 11,
   0, 0,           /*  DeltaX */
   0, 19, 4,
   EXIT, SLC_DX, SLC_UP_LOGIC, SLC_DN_LOGIC,
   SLC_X0, EXIT_DN, SLC_X0, EXIT_DN),
};

PRIVATE FORM SliceScrollForm = {
   /*  fill in FORMATTR_SCROLLING only when this form is to be displayed, */
   /*  otherwise mouse may choose wrong form */
   0, 0, FORMATTR_VISIBLE | FORMATTR_BORDER, 
   0, 0, 0,
   6, 1, 12, 26,
   0, 0,
   { DGROUP_CODE, 3 },
   { 0, 0 },
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(SliceScrollFormFields) / sizeof(SliceScrollFormFields[ 0 ]),
   SliceScrollFormFields,
   KSI_SLICE_SCROLL_FORM,
   0, DO_STRING_Reg, FORMS_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, SCAN_SET_Reg
};

static enum { ALL_TRKS_LABEL, Y0_ALL, DY_ALL };
  
PRIVATE FIELD TrackFormFields[] = {

   field_set(ALL_TRKS_LABEL,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_DO_STRINGS, 15,   /*  "All" */
   0, 0, 0, 0, 4, 0,
   1, 2, 4,
   Y0_ALL, Y0_ALL, EXIT, Y0_ALL,
   EXIT, Y0_ALL, EXIT, Y0_ALL),

   field_set(Y0_ALL,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_Y0,
   SCAN_HBASE + 16,
   DGROUP_SCANSET, 5,
   0, 0,
   DGROUP_CODE, 10,
   0, 0,      /*  Y0 */
   1, 10, 4,
   EXIT, Y0_ALL, EXIT, EXIT_DN,
   Y0_ALL, DY_ALL, EXIT, DY_ALL),

   field_set(DY_ALL,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_DELTA_Y,
   SCAN_HBASE + 17,
   DGROUP_SCANSET, 3,
   0, 0,
   DGROUP_CODE, 12,
   0, 0,      /*  DeltaY */
   1, 19, 4,
   EXIT, DY_ALL, EXIT, EXIT_DN,
   Y0_ALL, DY_ALL, Y0_ALL, EXIT_DN)
};

PRIVATE FORM TrackForm = {
   0, 0,
   FORMATTR_VISIBLE | FORMATTR_BORDER, 
   0, 0, 0,
   6, 53, 3, 25,
   0, 0,
   { 0, 0 }, { 0, 0 }, COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(TrackFormFields) / sizeof(TrackFormFields[ 0 ]),
   TrackFormFields,
   KSI_TRACK_FORM,
   0, DO_STRING_Reg, FORMS_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, SCAN_SET_Reg
};

static enum { TRK_INIT_LOGIC, TRK_UP_LOGIC, TRK_DN_LOGIC, TRK_NMR, TRK_Y0,
              TRK_DY
};

PRIVATE FIELD TrackScrollFormFields[] = {

   field_set(TRK_INIT_LOGIC,
   FLDTYP_LOGIC,
   FLDATTR_NONE,
   KSI_NO_INDEX,
   0,
   DGROUP_CODE, 0,
   0, 0, 0, 0, 0, 0,
   0, 1, 1,
   EXIT, TRK_NMR, TRK_INIT_LOGIC, TRK_INIT_LOGIC,
   TRK_INIT_LOGIC, TRK_INIT_LOGIC, TRK_INIT_LOGIC, TRK_INIT_LOGIC),

   field_set(TRK_UP_LOGIC,
   FLDTYP_LOGIC,
   FLDATTR_NONE,
   KSI_NO_INDEX,
   0,
   DGROUP_CODE, 1,
   0, 0, 0, 0, 0, 0,
   0, 3, 1,
   EXIT, TRK_NMR, TRK_UP_LOGIC, TRK_UP_LOGIC,
   TRK_UP_LOGIC, TRK_UP_LOGIC, TRK_UP_LOGIC, TRK_UP_LOGIC),

   field_set(TRK_DN_LOGIC,
   FLDTYP_LOGIC,
   FLDATTR_NONE,
   KSI_NO_INDEX,
   0,
   DGROUP_CODE, 2,
   0, 0, 0, 0, 0, 0,
   0, 5, 1,
   EXIT, TRK_NMR, TRK_DN_LOGIC, TRK_DN_LOGIC,
   EXIT, TRK_DN_LOGIC, EXIT, EXIT_DN),

   field_set(TRK_NMR,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   SCAN_HBASE + 18,
   DGROUP_SCANSET, 9, 
   0, 0,
   DGROUP_CODE, 22,
   0, 0,               /*  Track# */
   0, 1, 4,
   TRK_Y0, TRK_Y0, TRK_NMR, TRK_NMR,
   TRK_NMR, TRK_NMR, TRK_NMR, TRK_NMR),

   field_set(TRK_Y0,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_SCROLL_Y0,
   SCAN_HBASE + 19,
   DGROUP_SCANSET, 5, 
   0, 0,
   DGROUP_CODE, 10,
   0, 0,             /*  Y0 */
   0, 10, 4,
   EXIT, TRK_Y0, TRK_UP_LOGIC, TRK_DN_LOGIC,
   TRK_NMR, TRK_DY, EXIT, TRK_DY),

   field_set(TRK_DY,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_SCROLL_DELTA_Y,
   SCAN_HBASE + 20,
   DGROUP_SCANSET, 3, 
   0, 0,
   DGROUP_CODE, 12,
   0, 0,              /*  DeltaY */
   0, 19, 4,
   EXIT, TRK_DY, TRK_UP_LOGIC, TRK_DN_LOGIC,
   TRK_Y0, EXIT_DN, TRK_Y0, EXIT_DN),
};

PRIVATE FORM TrackScrollForm = {
   /*  fill in FORMATTR_SCROLLING only when this form is to be displayed, */
   /*  otherwise mouse may choose wrong form */
   0, 0,
   FORMATTR_VISIBLE | FORMATTR_BORDER,
   0, 0, 0,
   6, 53, 12, 25,
   0, 0,
   { DGROUP_CODE, 4 }, {0, 0},
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(TrackScrollFormFields) / sizeof(TrackScrollFormFields[ 0 ]),
   TrackScrollFormFields,
   KSI_TRACK_SCROLL_FORM,
   0, DO_STRING_Reg, FORMS_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, SCAN_SET_Reg
};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
PRIVATE BOOLEAN ScanSetFormInit(void)
{
  unsigned det_type;

  get_CurrentPoint(&form_CurrPoint);
  get_CurrentTrack(&form_CurrTrack);

  get_ActiveX(&form_ActiveX);
  get_ActiveY(&form_ActiveY);

  get_DeltaX(&form_DeltaX);
  get_DeltaY(&form_DeltaY);

  get_X0(&form_X0);
  get_Y0(&form_Y0);

  get_TrackMode(&form_trackmode);
  get_PointMode(&form_pointmode);

  get_ShiftMode(&form_shiftmode);

  get_Points(&form_points);
  get_Tracks(&form_tracks);

  get_OutputReg(&form_outputReg);

  get_StreakMode(&form_StreakMode);
  get_Prescan(&form_Prescan);
  
  get_ExposedRows(&form_StreakSize);
  if (form_StreakSize == 0)
    form_StreakSize++;

  get_DetectorType(&det_type);

  if (det_type != TSM_CCD)   /* only allow Dual if split mode */
    {
    unsigned char reg_opts = 2;

    if (det_type == EEV_CCD) /* only allow B if Thomson */
      reg_opts = 1;
    SpecialsFields[OUTPUT_REG].specific.tglfld.total_items = reg_opts;
    if (form_outputReg > reg_opts - 1)
      {
      form_outputReg = reg_opts - 1;
      set_OutputReg(form_outputReg);
      }
    }
  else
    SpecialsFields[OUTPUT_REG].specific.tglfld.total_items = 3;

  get_FrameTime(&frame_time);
  get_PrescanTime(&pretrig_time);

  /*  round off to nearest 10 percent */
  get_AntiBloom(&form_antiBloom);
  form_antiBloom = (form_antiBloom + 5) / 10;
  
  if(form_pointmode)
     SliceScrollForm.attrib |= FORMATTR_SCROLLING;
  else
     SliceScrollForm.attrib &= ~ FORMATTR_SCROLLING;

  if(form_trackmode)
     TrackScrollForm.attrib |= FORMATTR_SCROLLING;
  else
     TrackScrollForm.attrib &= ~ FORMATTR_SCROLLING;

  return FALSE;
}

PRIVATE int form_set_CurrentPoint(void)
{
  int err = errorCheckDetDriver(set_CurrentPoint(form_CurrPoint));

  get_X0(&form_X0);
  get_DeltaX(&form_DeltaX);
  return err;
}

PRIVATE int form_set_CurrentTrack(void)
{
  int err = errorCheckDetDriver(set_CurrentTrack(form_CurrTrack));

  get_Y0(&form_Y0);
  get_DeltaY(&form_DeltaY);
  return err;
}

PRIVATE int form_set_StreakMode(void)
{
  int err = errorCheckDetDriver(set_StreakMode(form_StreakMode));

  get_StreakMode(&form_StreakMode);
  return err;
}

PRIVATE int form_set_ExpRows(void)
{
  SHORT err;
  static SHORT HelpCount = 3;
  
  if (!form_StreakSize)
    return FIELD_VALIDATE_WARNING;

  /* Help message #3122 tells the user to enable streak mode first */

  if (!form_StreakMode)
    {
    form_help_from_file(SCAN_HBASE + 22);
    return FIELD_VALIDATE_SUCCESS;
    }

  if(push_form_context())
    Current.Form = &ScanSetupForm;
  err = set_ExposedRows(form_StreakSize);
  get_ExposedRows(&form_StreakSize);
  display_random_field(&ScanSetupForm, STREAK_ROWS);

  form_trackmode = 0;
  form_set_trackmode();            /* set uniform tracks */
  form_Y0 = 1;
  form_set_Y0();
  form_DeltaY = form_StreakSize;
  form_set_DeltaY();
  form_tracks = (form_ActiveY/form_StreakSize);
  form_set_tracks();
  pop_form_context();

  /* Help message #3122 reminds the user to check the other menu settings */
  
  if (!err && HelpCount-- > 0)
    form_help_from_file(SCAN_HBASE + 21);

  return errorCheckDetDriver(err);
}

PRIVATE void update_frame_time(void)
{
  get_FrameTime(&frame_time);
  display_random_field(&ScanSetupForm, FR_TIME);
  get_PrescanTime(&pretrig_time);
  display_random_field(&ScanSetupForm, PT_TIME);
}

PRIVATE int form_set_Prescan(void)
{
  SHORT temp;
  
  if (!form_StreakMode)
    return FIELD_VALIDATE_SUCCESS;

  temp = set_Prescan(form_Prescan);
  update_frame_time();
  return errorCheckDetDriver(temp);
}

PRIVATE BOOLEAN validate_point_index(int index)
{
  if (index < form_points)
    {
    errorCheckDetDriver(set_CurrentPoint(form_CurrPoint = index + 1));
    get_X0(&form_X0);
    get_DeltaX(&form_DeltaX);
    }
  return (index >= form_points);
}

PRIVATE BOOLEAN validate_track_index(int index)
{
  if (index < form_tracks)
    {
    errorCheckDetDriver(set_CurrentTrack(form_CurrTrack = index + 1));
    get_Y0(&form_Y0);
    get_DeltaY(&form_DeltaY);
    }
  return (index >= form_tracks);
}

PRIVATE unsigned char point_form_entry_select(FORM_CONTEXT * dummy)
{
  int mode; 
  get_PointMode(&mode);
  return ((unsigned char) mode);
}

PRIVATE unsigned char track_form_entry_select(FORM_CONTEXT * dummy)
{
  int mode; 
  get_TrackMode(&mode);
  return ((unsigned char) mode);
}

PRIVATE void refresh_form()
{
  int pointmode;
  int trackmode;

  get_PointMode(&pointmode);
  get_TrackMode(&trackmode);
   if(pointmode)
      SliceScrollForm.attrib |= FORMATTR_VISIBLE | FORMATTR_SCROLLING;
   else
      SliceScrollForm.attrib &= ~ FORMATTR_VISIBLE & ~FORMATTR_SCROLLING;

   if(trackmode)
      TrackScrollForm.attrib |= FORMATTR_VISIBLE | FORMATTR_SCROLLING;
   else
      TrackScrollForm.attrib &= ~ FORMATTR_VISIBLE & ~FORMATTR_SCROLLING;

   get_FrameTime(&frame_time);
   get_PrescanTime(&pretrig_time);
   draw_form();
}

PRIVATE int form_set_X0(void)
{
   int temp = set_X0(form_X0);
   update_frame_time();
   return errorCheckDetDriver(temp);
}

PRIVATE int form_set_Y0(void)
{
   int temp = set_Y0(form_Y0);
   update_frame_time();
   return errorCheckDetDriver(temp);
}

PRIVATE int form_set_DeltaX(void)
{
   int temp = set_DeltaX(form_DeltaX);
   update_frame_time();
   return errorCheckDetDriver(temp);
}

PRIVATE int form_set_DeltaY(void)
{
  int temp = set_DeltaY(form_DeltaY);
  update_frame_time();
  return errorCheckDetDriver(temp);
}

PRIVATE int form_set_ActiveX(void)
{
   int temp = errorCheckDetDriver(set_ActiveX(form_ActiveX));

   refresh_form();
   return temp;
}

PRIVATE int form_set_ActiveY(void)
{
   int temp = errorCheckDetDriver(set_ActiveY(form_ActiveY));

   refresh_form();
   return temp;
}

PRIVATE int form_set_pointmode(void)
{
   int temp = errorCheckDetDriver(set_PointMode(form_pointmode));

   get_Points(&form_points);

   if (form_pointmode)
    {
    ScanSetupFormFields[SLC_SCR_FORM].attrib |= FLDATTR_DRAW_PERMITTED;
    ScanSetupFormFields[SLC_FRM].attrib &= ~FLDATTR_DRAW_PERMITTED;
    }
  else
    {
    ScanSetupFormFields[SLC_SCR_FORM].attrib &= ~FLDATTR_DRAW_PERMITTED;
    ScanSetupFormFields[SLC_FRM].attrib |= FLDATTR_DRAW_PERMITTED;
    }
   refresh_form();
   return temp;
}

PRIVATE int form_set_trackmode(void)
{
   int temp = errorCheckDetDriver(set_TrackMode(form_trackmode));

   get_Tracks(&form_tracks);
   if (form_trackmode)
    {
    ScanSetupFormFields[TRK_SCR_FORM].attrib |= FLDATTR_DRAW_PERMITTED;
    ScanSetupFormFields[TRK_FRM].attrib &= ~FLDATTR_DRAW_PERMITTED;
    }
  else
    {
    ScanSetupFormFields[TRK_SCR_FORM].attrib &= ~FLDATTR_DRAW_PERMITTED;
    ScanSetupFormFields[TRK_FRM].attrib |= FLDATTR_DRAW_PERMITTED;
    }
   refresh_form();
   return temp;
}

PRIVATE int form_set_shiftmode(void)
{
  int temp = set_ShiftMode(form_shiftmode);
  update_frame_time();
  return errorCheckDetDriver(temp);
}

PRIVATE int form_set_points(void)
{
   int temp = errorCheckDetDriver(set_Points(form_points));

   refresh_form();
   return temp;
}

PRIVATE int form_set_tracks(void)
{
   int temp = errorCheckDetDriver(set_Tracks(form_tracks));
   get_Tracks(&form_tracks);
   get_Y0(&form_Y0);

   refresh_form();
   return temp;
}

/*  enter key response to anti bloom value toggle field */
PRIVATE int form_set_antiBloom(void)
{
  int temp =  set_AntiBloom(10 * form_antiBloom);
  update_frame_time();
  return errorCheckDetDriver(temp);
}

/*  enter key response to ouput register toggle field */
PRIVATE int form_set_output_reg(void)
{
  int temp = set_OutputReg(form_outputReg);
  update_frame_time();

  return errorCheckDetDriver(temp);
}

/*  init FormTable[] with form addresses */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void registerScansetForms(void)
{
   FormTable[KSI_SCAN_SETUP] = &ScanSetupForm;
   FormTable[KSI_SPECIALS] = &SpecialsForm;
   FormTable[KSI_SLICE_FORM] = &SliceForm;
   FormTable[KSI_SLICE_SCROLL_FORM] = &SliceScrollForm;
   FormTable[KSI_TRACK_FORM] = &TrackForm;
   FormTable[KSI_TRACK_SCROLL_FORM] = &TrackScrollForm;
}
