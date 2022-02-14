/* -----------------------------------------------------------------------
/
/  omaform1.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/omaform1.c_v   0.32   06 Jul 1992 10:34:40   maynard  $
/  $Log:   J:/logfiles/oma4000/main/omaform1.c_v  $
/
*/

#include <io.h>
#include <string.h>

#include "omaform.h"
#include "formtabs.h"
#include "curvdraw.h"
#include "multi.h"
#include "change.h"
#include "helpindx.h"
#include "omameth.h"
#include "cursor.h"
#include "ksindex.h"
#include "livedisk.h"
#include "macrecor.h"
#include "macruntm.h"
#include "di_util.h"
#include "fileform.h"
#include "filestuf.h"  // fileIdOMA4...
#include "barmenu.h"
#include "graphops.h"
#include "basepath.h"
#include "filemsg.h"
#include "omaerror.h"
#include "forms.h"
#include "plotbox.h"
#include "syserror.h"  // ERROR_OPEN
#include "pltsetup.h"
#include "oma4driv.h"
#include "detsetup.h"  // WriteSys();
#include "coolstat.h"
#include "spgraph.h"   // ActonSpectrograph(), Reset1235()
#include "mathform.h" // min_count, max_count

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

PRIVATE unsigned int min_det_addr = 0x100;
PRIVATE unsigned int max_det_addr = 0xF00;

PRIVATE DATA_LIMIT DataLimits[] = {
   { 0, 0, 0 },
   { NULL, NULL, 0 },
   { &min_det_addr, &max_det_addr, 0 },
   { &min_count, &max_count, 0 }
};

DATA_LIMIT * DataLimitRegistry = &DataLimits[0];

PRIVATE char gmh_filename[FNAME_LENGTH] = { "" };
PRIVATE char gmh_description[DESCRIPTION_LENGTH] = { "" };
PRIVATE int  gmh_operation = 0;

// for CursorGoToForm
PRIVATE BOOLEAN CursorGO;
PRIVATE FLOAT CursorGoToX = (FLOAT) 0;
PRIVATE FLOAT CursorGoToZ = (FLOAT) 0;

PRIVATE char * ch_output_options[] = {
   "OMA 1460",
   "OMA 1470",
   "Single column",
   "Single column with X",
   "Multicolumn",
   "Multicolumn with X",
   "HiDRIS (TCL-32)",
   "TIFF-G (8-bit)",
};

PRIVATE char * getmethodhdr_options[] =
{
   "Load Full Method from File",
   "Save Method to File",
   "Load Detector Setups Only",
   "Load Plot Setups Only",
};

PRIVATE char * KeyStrokeModeOptions[] = { "Record", "Play Back" };

static int dummy_select;

unsigned int default_form_attributes =
(FORMATTR_NO_ENTER_VECTOR | FORMATTR_FIRST_CHAR_ERASE | FORMATTR_VISIBLE);

// PRIVATE functions
PRIVATE int     getmethodhdr_execute(void);
PRIVATE int     methodFormExit(void);
PRIVATE int     savedescription(void);
PRIVATE BOOLEAN getdescription(void);
PRIVATE BOOLEAN CursorGoToInit();
PRIVATE BOOLEAN CursorGoToExit();
PRIVATE int     CursorGoTo(void * field_data, char * field_string);
PRIVATE int VerifyMethodFileName(void);

enum { DGROUP_DO_STRINGS = 1, DGROUP_TOGGLES, DGROUP_CODE,
       DGROUP_GENERAL,        DGROUP_CHANGE
};

static DATA DO_STRING_Reg[] = {
/* 0  */ { "Macro File Name", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 1  */ { "Mode", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 2  */ { "Delay", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 3  */ { "Description", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 4  */ { "New Cursor Position", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 5  */ { "X=", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 6  */ { "Z=", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 7  */ { "Filename", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 8  */ { "Action", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 9  */ { " Go ", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 10 */ { "Count", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 11 */ { "sec.", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 12 */ { "Starting Curve", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 13 */ { "Input Curve Set", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 14 */ { "Output File", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 15 */ { "Output Format", 0, DATATYP_STRING, DATAATTR_PTR, 0 },

};

static DATA TOGGLES_Reg[] = {
/* 0 */ { KeyStrokeModeOptions, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
/* 1 */ { getmethodhdr_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
/* 2 */ { ch_output_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
};

static EXEC_DATA CODE_Reg[] = {
/* 0  */ { getdescription, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 1  */ { savedescription, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 2  */ { getmethodhdr_execute, 0, DATATYP_CODE, DATAATTR_PTR, 0},
/* 3  */ { ChangeVerifyCurveBlk, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 4  */ { VerifyFileName, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 5  */ { CursorGoToInit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 6  */ { CAST_CHR2INT CursorGoTo, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 7  */ { CursorGoToExit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 8  */ { StartKeyStroke, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 9  */ { CAST_CHR2INT change_execute, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 10 */ { methodFormExit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 11 */ { VerifyMethodFileName, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
};

/* PDA setup always sends contents of same string buffer, */
/* CCD scan setup code is now in CODE_Reg in scanset.c */

static DATA GENERAL_Reg[] = {
/* 0  */ { &CursorGoToZ,  0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
/* 1  */ { &dummy_select, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 2  */ { &LiveDiskMode, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 3  */ { &CursorGoToX,  0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
/* 4  */ { &PlayBackDelay,  0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
/* 5  */ { &gmh_operation, 0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 6  */ { gmh_description, 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 7  */ { &KeyStrokeMode,  0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 8  */ { MacRecordFileName,  0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 9  */ { gmh_filename, 0, DATATYP_STRING, DATAATTR_PTR, 0 },
};

static DATA CHANGE_Reg[] = {
   { ch_input_fname, 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   { &from_field, 0, DATATYP_INT, DATAATTR_PTR, 0 },
   { &count_field, 0, DATATYP_INT, DATAATTR_PTR, 0 },
   { ch_output_fname, 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   { &ch_output_format, 0, DATATYP_INT, DATAATTR_PTR, 0 }
};

static FIELD chformat_formFields[] = {
   { FLDTYP_STRING, FLDATTR_DISPLAY_ONLY, KSI_NO_INDEX,
      0, { DGROUP_DO_STRINGS, 13 }, {0, 0}, {0, 0}, {0, 0},
   5, 2, 15, { 1, 0, 0, 0, 0, 0, 0, 0 } },
   { FLDTYP_STRING, FLDATTR_DISPLAY_ONLY, KSI_NO_INDEX,
      0, { DGROUP_DO_STRINGS, 12 }, {0, 0}, {0, 0}, {0, 0},
   5, 46, 14, { 1, 0, 0, 0, 0, 0, 0, 0 } },
   { FLDTYP_STRING, FLDATTR_DISPLAY_ONLY, KSI_NO_INDEX,
      0, { DGROUP_DO_STRINGS, 10 }, {0, 0}, {0, 0}, {0, 0},
   5, 67, 5, { 1, 0, 0, 0, 0, 0, 0, 0 } },
   { FLDTYP_STRING, FLDATTR_DISPLAY_ONLY, KSI_NO_INDEX,
      0, { DGROUP_DO_STRINGS, 14 },   // "Output File"
      {0, 0}, {0, 0}, {0, 0},
   9, 2, 11, { 1, 0, 0, 0, 0, 0, 0, 0 } },
   { FLDTYP_STRING, FLDATTR_DISPLAY_ONLY, KSI_NO_INDEX,
      0, { DGROUP_DO_STRINGS, 15 },     // "Output Format"
      {0, 0}, {0, 0}, {0, 6},
   14, 14, 13, { 1, 0, 0, 0, 0, 0, 0, 0 } },
   // file name
   { FLDTYP_STRING, FLDATTR_REV_VID, KSI_XLATE_IN_NAME,
      CHFORMAT_HBASE + 0,
      { DGROUP_CHANGE, 0 }, {0, 0},
      {DGROUP_CODE, 3 },        // ChangeVerifyCurveBlk
      {40, 0},
   5, 18, 26, { -6, 0, 5, 3, 0, 0, 5, 1 } },
   // StartCurve
   { FLDTYP_UNS_INT, FLDATTR_REV_VID, KSI_XLATE_START,
      CHFORMAT_HBASE + 1,
      { DGROUP_CHANGE, 1 }, {0, 0}, {0, 0}, {0, 0},
   5, 61, 5, { -7, 0, 4, 2, -1, 1, -1, 1 } },
   // CurveCount
   { FLDTYP_UNS_INT, FLDATTR_REV_VID, KSI_XLATE_COUNT,
      CHFORMAT_HBASE + 2,
      { DGROUP_CHANGE, 2 }, {0, 0}, {0, 0}, {0, 0},
   5, 73, 5, { -8, 0, 3, 1, -1, 0, -1, 1 } },
   // Output file name
   { FLDTYP_STRING, FLDATTR_REV_VID, KSI_XLATE_OUT_NAME,
      CHFORMAT_HBASE + 3,
      { DGROUP_CHANGE, 3 }, {0, 0},
      {DGROUP_CODE, 4 }, // VerifyFileName
      {40, 0},
   9, 14, 64, { -9, 0, -3, 1, 0, 0, -1, 1 } },
   // format
   { FLDTYP_TOGGLE, FLDATTR_REV_VID, KSI_XLATE_FORMAT_OPTS,
      CHFORMAT_HBASE + 4,
      { DGROUP_CHANGE, 4 },
      {DGROUP_TOGGLES, 2 },  // ch_output_options
      {0, 0},
      {0, 8},
     14, 28, 26,
     { -10, 0, -1, 1, 0, 0, -1, 1 } },
   // Go
   { FLDTYP_SELECT, FLDATTR_REV_VID, KSI_XLATE_GO,
      CHFORMAT_HBASE + 5,
      { DGROUP_GENERAL, 1 },      // dummy_select
      { DGROUP_DO_STRINGS, 9 },  // " Go "
      {DGROUP_CODE, 9 },      // change_execute
      {0, 0},                                                /* (0:3) */
   17, 28, 4, { -11, 0, -1, -5, 0, 0, -1, -5 } }
};

FORM  chformat_form = {
   0, 0, FORMATTR_BORDER | FORMATTR_EXIT_RESTORE | FORMATTR_VISIBLE |
         FORMATTR_FULLSCREEN,
    0, 0, 0, 2, 0, 21, 80, 0, 0,
   { 0, 0 }, { 0, 0 }, COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(chformat_formFields) / sizeof(chformat_formFields[ 0 ]),
   chformat_formFields, KSI_FORMAT_FORM,
   0, DO_STRING_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, GENERAL_Reg, CHANGE_Reg
};

static enum { LABEL_FNAME,
              LABEL_DESC,
              LABEL_ACT,
              FLD_FILENAME,
              FLD_DESC,
              FLD_ACT,
              FLD_DOIT
              };

FIELD getmethodhdrFields[] = {
   
  label_field(LABEL_FNAME,         
  DGROUP_DO_STRINGS, 7,    /* "Filename" */
  5, 9, 4),

  label_field(LABEL_DESC,         
  DGROUP_DO_STRINGS, 3,    /* "Description" */
     8, 2, 11),

  label_field(LABEL_ACT,         
  DGROUP_DO_STRINGS, 8,    /* "Action" */
  13, 7, 6),

  field_set(FLD_FILENAME,
   FLDTYP_STRING,
   FLDATTR_REV_VID,
   KSI_GM_OUT_NAME,
   GETMETHODHDR_HBASE + 0,
   DGROUP_GENERAL, 9 ,     /* gmh_filename */
   0, 0,
   DGROUP_CODE, 11,        /* VerifyFileName */
   FNAME_LENGTH-1, 0,
   5, 14, 64,
   EXIT, FLD_FILENAME, FLD_DOIT, FLD_DESC,
   FLD_FILENAME, FLD_FILENAME, FLD_DOIT, FLD_DESC),
   
  field_set(FLD_DESC,
   FLDTYP_STRING,
   FLDATTR_REV_VID,
   KSI_GM_DESC,
   GETMETHODHDR_HBASE + 1,
   DGROUP_GENERAL, 6,      /* gmh_description */
   0, 0,
   DGROUP_CODE, 1,         /* savedescription */
   DESCRIPTION_LENGTH-1, 0,
   8, 14, 64,
   EXIT, FLD_DESC, FLD_FILENAME, FLD_ACT,
   FLD_DESC, FLD_DESC, FLD_FILENAME, FLD_ACT),
   
  field_set(FLD_ACT,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_GM_OPTS,
   GETMETHODHDR_HBASE + 3,
   DGROUP_GENERAL, 5,      /* gmh_operation */
   DGROUP_TOGGLES, 1,      /* getmethodhdr_options */
   0, 0,
   0, 4,
   13, 14, 26,
   EXIT, FLD_ACT, FLD_DESC, FLD_DOIT,
   FLD_DESC, FLD_DESC, FLD_DESC, FLD_DOIT),
   
  field_set(FLD_DOIT,
   FLDTYP_SELECT,
   FLDATTR_REV_VID,
   KSI_GM_GO,
   GETMETHODHDR_HBASE + 2,
   DGROUP_GENERAL, 1,      /* dummy_select */
   DGROUP_DO_STRINGS, 9,   /* " Go " */
   DGROUP_CODE, 2,         /* getmethodhdr_execute */
   0, 0,                   
   16, 14, 4,
   EXIT, FLD_DOIT, FLD_ACT, FLD_FILENAME,
   FLD_DOIT, FLD_DOIT, FLD_ACT, FLD_FILENAME),
};

FORM getmethodhdrForm = {
   0, 0, FORMATTR_BORDER | FORMATTR_FULLSCREEN |
         FORMATTR_VISIBLE,
    0, 0, 0, 2, 0, 21, 80, 0, 0,
   { DGROUP_CODE, 0 },          // getdescription
   { DGROUP_CODE, 10 },         // getdescription
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(getmethodhdrFields) / sizeof(getmethodhdrFields[ 0 ]),
   getmethodhdrFields, KSI_GET_METHOD_FORM,
   0, DO_STRING_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, GENERAL_Reg, CHANGE_Reg
};

enum { KS_FNAME_LABEL, KS_MODE_LABEL, KS_DELAY_LABEL, KS_SEC_LABEL,
       KS_FNAME,       KS_MODE,       KS_DELAY,       KS_GO
};

static FIELD KeyStrokeFormFields[] = {
   field_set(KS_FNAME_LABEL,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_DO_STRINGS, 0,       /* "Macro File Name" */
   0, 0,
   0, 0,
   0, 0,                           /* (0:0) */
   5, 3, 15,
   KS_MODE_LABEL, KS_FNAME_LABEL, KS_FNAME_LABEL,
   KS_FNAME_LABEL, KS_FNAME_LABEL, KS_FNAME_LABEL,
   KS_FNAME_LABEL, KS_FNAME_LABEL),

   field_set(KS_MODE_LABEL,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_DO_STRINGS, 1,   /* "Mode" */
   0, 0,
   0, 0,
   0, 0,                           /* (0:1) */
   9, 20, 4,
   KS_DELAY_LABEL, KS_MODE_LABEL, KS_MODE_LABEL, KS_MODE_LABEL,
   KS_MODE_LABEL, KS_MODE_LABEL, KS_MODE_LABEL, KS_MODE_LABEL),

   field_set(KS_DELAY_LABEL,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_DO_STRINGS, 2,  /* "Delay" */
   0, 0,
   0, 0,
   0, 0,                           /* (0:2) */
   9, 40, 5,
   KS_SEC_LABEL, KS_DELAY_LABEL, KS_DELAY_LABEL, KS_DELAY_LABEL,
   KS_DELAY_LABEL, KS_DELAY_LABEL, KS_DELAY_LABEL, KS_DELAY_LABEL),

   field_set(KS_SEC_LABEL,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_DO_STRINGS, 11,           /* "sec." */
   0, 0,
   0, 0,
   0, 0,                           /* (0:3) */
   9, 54, 4,
   KS_FNAME, KS_SEC_LABEL, KS_SEC_LABEL, KS_SEC_LABEL,
   KS_SEC_LABEL, KS_SEC_LABEL, KS_SEC_LABEL, KS_SEC_LABEL),

   field_set(KS_FNAME,
   FLDTYP_STRING, FLDATTR_REV_VID | FORMATTR_FIRST_CHAR_ERASE,
   KSI_KS_NAME, KEYSTROKE_HBASE,
   DGROUP_GENERAL, 8,              /* MacRecordFileName */
   0, 0,
   DGROUP_CODE, 4,           /* VerifyFileName */
   FNAME_LENGTH - 1, 0,
   5, 19, 57,
   EXIT, KS_FNAME, KS_GO, KS_MODE, KS_FNAME,
   KS_FNAME, KS_GO, KS_MODE),

   field_set(KS_MODE,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID | FORMATTR_FIRST_CHAR_ERASE,
   KSI_KS_OPTS,
   KEYSTROKE_HBASE + 1,
   DGROUP_GENERAL, 7,    /* KeyStrokeMode */
   DGROUP_TOGGLES, 0,    /* KeyStrokeModeOptions */
   0, 0,
   0, 2,                           /* (0:5) */
   9, 25, 9,
   EXIT, KS_MODE, KS_FNAME, KS_GO, KS_FNAME, KS_DELAY,
   KS_FNAME, KS_DELAY),

   field_set(KS_DELAY,
   FLDTYP_SCL_FLOAT,
   FLDATTR_RJ | FLDATTR_REV_VID | FORMATTR_FIRST_CHAR_ERASE,
   KSI_KS_DELAY,
   KEYSTROKE_HBASE + 2,
   DGROUP_GENERAL, 4,     /* PlayBackDelay */
   0, 0,
   0, 0,
   4, 0,                           /* (0:6) */
   9, 45, 8,
   EXIT, KS_DELAY, KS_FNAME, KS_GO, KS_MODE, KS_DELAY,
   KS_MODE, KS_GO),

   field_set(KS_GO,
   FLDTYP_SELECT,
   FLDATTR_REV_VID,
   KSI_KS_GO,
   KEYSTROKE_HBASE + 3,
   DGROUP_GENERAL, 1,           /* dummy_select */
   DGROUP_DO_STRINGS, 9,        /* " Go " */
   DGROUP_CODE, 8,              /* StartKeyStroke */
   0, 0,                                                 /* (0:7) */
   13, 36, 4,
   EXIT, KS_GO, KS_MODE, KS_FNAME, KS_GO, KS_GO,
   KS_DELAY, KS_FNAME),

};

FORM KeyStrokeForm = {
   0, 0, FORMATTR_BORDER | FORMATTR_FULLSCREEN | FORMATTR_EXIT_RESTORE |
    FORMATTR_VISIBLE,
   0, 0, 0, 2, 0, 21, 80, 0, 0,
   { 0, 0 }, { 0, 0 }, COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(KeyStrokeFormFields) / sizeof(KeyStrokeFormFields[ 0 ]),
   KeyStrokeFormFields, KSI_KEYSTROKE_FORM,
   0, DO_STRING_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, GENERAL_Reg, CHANGE_Reg
};

enum { LABEL_CGT, LABEL_CGX, LABEL_CGZ,
       CGT_X, CGT_Z, CGT_GO };

FIELD CursorGoToFormFields[] =
{
   label_field(LABEL_CGT,               /* "New Cursor Position" */
   DGROUP_DO_STRINGS, 4,
   1, 8, 19),
   
   label_field(LABEL_CGX,               /* "X=" */
   DGROUP_DO_STRINGS, 5,
   3, 1, 2),
   
   label_field(CGT_Z,                   /* "Z=" */
   DGROUP_DO_STRINGS, 6,                
   3, 18, 2),
  
   field_set(CGT_X,
   FLDTYP_STD_FLOAT,
   FLDATTR_RJ | FLDATTR_REV_VID,
   KSI_CGT_X,
   CURSOR_GOTO_HBASE,
   DGROUP_GENERAL, 3,                   /* CursorGoToX */
   0, 0,
   0, 0,
   4, 0,
   3, 4, 12,
   EXIT, CGT_Z, CGT_GO, CGT_GO,
   CGT_X, CGT_Z, CGT_GO, CGT_Z),
   
   field_set(CGT_Z,
   FLDTYP_STD_FLOAT,
   FLDATTR_RJ | FLDATTR_REV_VID,
   KSI_CGT_Z,
   CURSOR_GOTO_HBASE + 1,
   DGROUP_GENERAL, 0,                   /* CursorGoToZ */
   0, 0,
   0, 0,
   4, 0,
   3, 21, 12,
   EXIT, CGT_GO, CGT_GO, CGT_GO,
   CGT_Z, CGT_Z, CGT_Z, CGT_GO),

   field_set(CGT_GO,
   FLDTYP_SELECT,
   FLDATTR_REV_VID,
   KSI_CGT_GO,
   CURSOR_GOTO_HBASE + 2,
   DGROUP_GENERAL, 1,                   /* dummy_select */
   DGROUP_DO_STRINGS, 9,                /* " Go " */
   DGROUP_CODE, 6,                      /* CursorGoTo */
   0, 0,
   5, 15, 4,
   EXIT, EXIT, CGT_X, CGT_X,
   CGT_GO, CGT_GO, CGT_Z, CGT_Z),
};

FORM    CursorGoToForm = {
   0, 0, FORMATTR_EXIT_RESTORE | FORMATTR_BORDER | FORMATTR_VISIBLE, 
   0, 0, 0, 8, 21, 7, 35, 0, 0,
   { DGROUP_CODE, 5 },    /* CursorGoToInit */
   { DGROUP_CODE, 7 },   /* CursorGoToExit */
   COLORS_DEFAULT,
    0, 0, 0, 0,
   sizeof(CursorGoToFormFields) / sizeof(CursorGoToFormFields[0]),
   CursorGoToFormFields, KSI_CURSOR_GOTO_FORM,
   0, DO_STRING_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, GENERAL_Reg, CHANGE_Reg
};

COLOR_SET ColorSets[ MAX_COLOR_SETS ] = {
   { { BRT_WHITE, BLUE },
     { BRT_WHITE, CYAN },
     { BRT_RED, BLUE },
     { WHITE, BLUE } },

   { { BLACK, BLUE },
     { BRT_BLUE, BLACK },
     { BLACK, BLACK },
     { BLACK, BLACK } },

   { { BRT_WHITE, RED },
     { BLACK, RED },
     { BLACK, RED },
     { BLACK, RED } },

   { { BRT_WHITE, BLUE },
     { BRT_WHITE, CYAN },
     { BRT_RED, BLUE },
     { WHITE, BLUE } }
};

FORM  GraphWindow = {
   0, 0, 0, 0, 0, 0, 2, 0, 21, 80, 0, 0,
   { 0, 0 }, { 0, 0 }, COLORS_DEFAULT,
   0, 0, 0, 0, 0, NULL, KSI_NO_INDEX,
   0, DO_STRING_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, GENERAL_Reg, CHANGE_Reg
};

PRIVATE FORM  Menu1 = {
   0, 0, 0, 0, 0, 0, 0, 0, 1, 80, 0, 0,
   { 0, 0 }, { 0, 0 }, COLORS_MENU,
   0, 0, 0, 0, 0, NULL, KSI_NO_INDEX,
   0, DO_STRING_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, GENERAL_Reg, CHANGE_Reg
};

//FORM  Menu2 = {
//   0, 0, 0, 0, 0, 0, 1, 0, 1, 80, 0, 0,
//   { 0, 0 }, { 0, 0 }, COLORS_MENU,
//   0, 0, 0, 0, 0, NULL, KSI_NO_INDEX,
//   0, DO_STRING_Reg, TOGGLES_Reg, CODE_Reg, GENERAL_Reg, CHANGE_Reg
//};

/* function key display on the bottom of the screen will look somewhat */
/* like the bar menus */
CHAR *MenuModeStr =  "F10 Menus";
CHAR *TagExpandStr = "F6 Tag Expand";
CHAR *TagShrinkStr = "F6 Tag Shrink";
CHAR *TagSaveStr =   "F5 Copy Tagged";

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN isFormGraphWindow(FORM * theForm)
{
  return theForm == &GraphWindow;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN isPrevFormGraphWindow()
{
  if (Current.PreviousStackedContext)
    return Current.PreviousStackedContext->Form == &GraphWindow;
  else
    return(FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN isFormMenu1(FORM * theForm)
{
  return theForm == & Menu1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN isPrevFormMenu1()
{
  if (Current.PreviousStackedContext)
    return (Current.PreviousStackedContext->Form == & Menu1);
  else
    return(FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setCurrentFormToMenu1(void)
{
  Current.Form = &Menu1;
  Current.Form->status = FORMSTAT_ACTIVE_FIELD;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setCurrentFormToGraphWindow(void)
{
  Current.Form = &GraphWindow;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setCurrentFormToMacroForm(void)
{
  Current.Form = NorecFormTable[ NOREC_MACRO_FORM ];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN isCurrentFormMacroForm(void)
{
   return Current.Form == NorecFormTable[ NOREC_MACRO_FORM ];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN TempChangeCursorType(SaveAreaInfo ** SavedArea)
{
  if ((active_locus == LOCUS_MENUS) || ((active_locus == LOCUS_FORMS) &&
    !((isCurrentFormMacroForm()) && (isPrevFormGraphWindow()))))
    {
    erase_cursor();
    SetGraphCursorType(CursorType);
    if (SavedArea)
      *SavedArea = save_screen_area(1, 0, 1, screen_columns);
    return TRUE;
    }
  else if(isCurrentFormMacroForm())
    {
    erase_cursor();
    SetGraphCursorType(CursorType);
    }
  return FALSE;
}

void TempRestoreCursorType(BOOLEAN RemGCursor, SaveAreaInfo ** SavedArea)
{
  if (RemGCursor)
    {
    RemoveGraphCursor();
    set_cursor_type(TextCursor);
    if (SavedArea)
      restore_screen_area(*SavedArea);
    }
  else if(isCurrentFormMacroForm())
    set_cursor_type(TextCursor);
}

// Verify that a string may be a proper file name
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int VerifyFileName(void)
{
  CHAR NameBuf[81];

  /* expand the file block name and check to see if it is not a directory */
  if (ParseFileName(NameBuf, Current.FieldDataPtr) == 2)
    {
    strupr(NameBuf);
    strcpy(Current.FieldDataPtr, NameBuf);
    return(FIELD_VALIDATE_SUCCESS);
    }
  else
    return strlen(Current.FieldDataPtr);
}

// verify that the current field string is a good non-reserved file name 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int VerifyWritableFileName(void) 
{
   CHAR SrcPath[DOSPATHSIZE + 1], SrcName[DOSFILESIZE + 1];
   int ReturnVal = VerifyFileName();

   if (ReturnVal == FIELD_VALIDATE_SUCCESS)
   {
      // check for special file names
      ParsePathAndName(SrcPath, SrcName, Current.FieldDataPtr);

      // Do not allow writing TO one of the special file names.
      // Renaming FROM a special file is OK.
      if(is_special_name(SrcName))
      {
         error(ERROR_NO_SPECIAL_OVERWRITE, SrcName);
         strcpy(Current.FieldDataPtr, ""); // blank the field
         string_to_field_string(Current.FieldDataPtr);
         ReturnVal = 0;
      }
   }
   return ReturnVal;
}

// Verify that a string may be a proper file name including the special
// names that may be in the curve directory
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int specialVerifyFileName(void)
{
   if(is_special_name(Current.FieldDataPtr))
      return FIELD_VALIDATE_SUCCESS;

   return VerifyFileName();
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int VerifyMethodFileName(void)
{
  CHAR NameBuf[81];
  CHAR FileTypeID[FTIDLEN];
  FILE * mFile;

  /* expand the file block name and check to see if it is not a directory */
  if (ParseFileName(NameBuf, Current.FieldDataPtr) == 2)
    {
    strupr(NameBuf);
    strcpy(Current.FieldDataPtr, NameBuf);
    }
  else
    return strlen(Current.FieldDataPtr);

  /* access checks to see if file exists */

  if (access(NameBuf, 0) && !MacroRunProgram)
    {
    /* show message if file does not exist and in interactive mode */
    form_help_from_file(GETMETHODHDR_HBASE + 4);
    return(FIELD_VALIDATE_SUCCESS);
    }

  mFile = fopen(NameBuf, "rb");

  if (!mFile)
    return(error(ERROR_OPEN, NameBuf));

  if (fread(FileTypeID, FTIDLEN, 1, mFile) != 1)
    return(error(ERROR_READ, NameBuf));

  /* must be either a method or data file */
  if (stricmp(FileTypeID, fidDataOMA4) && stricmp(FileTypeID, fidMethodOMA4))
    return error(ERROR_IMPROPER_FILETYPE, NameBuf);

  if (fseek(mFile, offsetof(METHDR, Description), SEEK_SET))
    return error(ERROR_SEEK, NameBuf);

  if (fread(gmh_description, DESCRIPTION_LENGTH, 1, mFile) != 1)
    return(error(ERROR_READ, NameBuf));

  fclose(mFile);

  display_random_field(&getmethodhdrForm, FLD_DESC);
  return(FIELD_VALIDATE_SUCCESS);
}

// Prepare for special graph (with form on screen at the same time) to
// exit to the form.  Returns result of set_attributes() call.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UCHAR specialGrafPrepareForExit(void)
{
   UCHAR attribute =
             set_attributes(MenuFocus.MenuColorSet -> regular.foreground,
                            MenuFocus.MenuColorSet -> regular.background);

   RemoveGraphCursor();
   Current.Form->exit_key_code = (UCHAR) GraphExitKey;
   active_locus = LOCUS_FORMS;

   if ((Current.Form->status != FORMSTAT_EXIT_ALL_FORMS) && 
        (Current.Form->status != FORMSTAT_EXIT_TO_MENU1) &&
        (Current.Form->status != FORMSTAT_EXIT_TO_MENU2))
      Current.Form->status = FORMSTAT_ACTIVE_FIELD;

   if (Current.Form->attrib & FORMATTR_OVERSTRIKE)
      set_cursor_type(CURSOR_TYPE_OVERSTRIKE);
   else
      set_cursor_type(CURSOR_TYPE_NORMAL);

   return attribute;
}

// Return from a special graph (with form on screen at the same time) to
// the on screen form.
// Mouse handler may have already popped the context, so check it.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void specialGrafReturnToForm(void)
{
   if(Current.Form == & GraphWindow)      
   {
      UCHAR OldStatus = Current.Form->status;

      pop_form_context();
      Current.Form->status = OldStatus;
   }
}

// return TRUE iff a plot is showing on screen at the same time as the form.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN isPlotShowingForm(int macFormIndex)
{
   switch(macFormIndex) {
      case KSI_XCAL_FORM :
      case KSI_XCAL_SCROLL_FORM :
      case KSI_BASLN_FORM :
      case KSI_KNOTS_FORM :
      case KSI_SPGRAPH_FORM :
      case KSI_MATH_FORM:
      case KSI_STAT_FORM:
// only put in KSI_SCAN_SETUP if graphical scan setup form is being used.
//      case KSI_SCAN_SETUP :
         return TRUE;
   }
   return FALSE;
}

// return TRUE if the plot area is showing
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN plotAreaShowing(void)
{
  if((active_locus != LOCUS_FORMS) && (active_locus != LOCUS_POPUP))
    return TRUE;

  if(isPlotShowingForm(Current.Form->MacFormIndex))
    return TRUE;

  if(isCurrentFormMacroForm())
    {

    if(isPrevFormMenu1() || isPrevFormGraphWindow())
      return TRUE;

    if (Current.PreviousStackedContext)
      return isPlotShowingForm(Current.PreviousStackedContext->Form->MacFormIndex);
    }
  return FALSE;
}

/****************************************************************************/
PRIVATE int methodFormExit(void)
{
  multiplot_setup(PlotWindows, &DisplayGraphArea, window_style);
  PutUpPlotBoxes();
  return 0;
}

/****************************************************************************/
PRIVATE int getmethodhdr_execute(void)
{
  FILE* fptr;
  int   overwrite_reply;
  ERR_OMA err;
  static char* filemode[] =
    {
    "rb",       /* attribute for Load Full Method Header */
    "wb",       /* attribute for "Save Method Header" */
    "rb",       /* attribute for Load detector */
    "rb",       /* attribute for Load Plot */
    };

  if (gmh_operation == METHOD_SAVE)
    {
    if ((fptr = fopen(gmh_filename,"r")) != NULL)
      {
      char * overwrite_prompt[] = { "Output file already exists",
                                    "Overwrite existing file ?",
                                    NULL };
      fclose(fptr);
      overwrite_reply = yes_no_choice_window(overwrite_prompt, 0,
                                             COLORS_MESSAGE);
      if (overwrite_reply != YES)
        {
        char * get_new_file[] = { "New output filename requested",
                                   NULL };

        message_window(get_new_file,COLORS_ERROR);
        return FIELD_VALIDATE_SUCCESS;
        }
      }
    }
  /* get real output file */
  if ((fptr = fopen(gmh_filename,filemode[gmh_operation])) == NULL)
    {
    return error(ERROR_OPEN, gmh_filename);
    }
  if (gmh_operation == METHOD_SAVE) /* save */
    {
    /* copy the plotbox values to InitialMethod */
    CopyPlotToMethod();

    /* copy the scan values to the method */
    err = DetInfoToMethod(&InitialMethod);

    if (! err)
      err = MethdrWrite(fptr, gmh_filename, InitialMethod);

    // get rid of copied detector data from the method
    DeAllocMetDetInfo(InitialMethod);
    }
  else   /* load */
    {
    DeAllocMethdr(InitialMethod);
    if(! (err = MethdrRead(fptr, gmh_filename, & InitialMethod)))
      /* Don't use info if read was bad */
      {
      if ((gmh_operation == METHOD_LOAD_FULL) ||
          (gmh_operation == METHOD_LOAD_PLOT))
        {
        InitializePlotSetupFields(&DisplayGraphArea);
        if(ActonSpectrograph())       /* make sure it's ok to reset.*/
          Reset1235();
        }

      /* initialize scan setup */
      if ((gmh_operation == METHOD_LOAD_FULL) ||
          (gmh_operation == METHOD_LOAD_DET))
        {
        MethodToDetInfo(InitialMethod);
        WriteSys();
        }

      // get rid of copied detector data from the method
      DeAllocMetDetInfo(InitialMethod);
      }
    }

  fclose(fptr);
  return FIELD_VALIDATE_SUCCESS;
}

/****************************************************************************/
PRIVATE int savedescription(void)
{
   memcpy(InitialMethod->Description,gmh_description,DESCRIPTION_LENGTH);
   return FIELD_VALIDATE_SUCCESS;
}
/****************************************************************************/
PRIVATE BOOLEAN getdescription(void)
{
   memcpy(gmh_description,InitialMethod->Description,DESCRIPTION_LENGTH);
   return FALSE;
}

//    Initialization routine for Cursor Goto form
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN CursorGoToInit()
{
   CursorGoToForm.field_index = 0;     // always start out in X field
   CursorGoToX = CursorStatus[ActiveWindow].X;
   CursorGoToZ = CursorStatus[ActiveWindow].Z;
   CursorGO = FALSE;
   return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN CursorGoToExit()
{
   SetGraphCursorType(CursorType);
   if(CursorGO && JumpCursor(CursorGoToX, CursorGoToZ))
     return TRUE;

   return FALSE;
}

//    GO field action for Cursor Goto form
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int CursorGoTo(void * field_data, char * field_string)
{
   CursorGO = TRUE;
   return FIELD_VALIDATE_SUCCESS;
}

// -----------------------------------------------------------------------
void form_help_from_file(int help_index)
{
   file_message_window( base_path(HELP_FILE), help_index,
                         MAX_MESSAGE_ROWS, COLORS_MESSAGE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void registerOmaform1Forms()
{
   FormTable[KSI_FORMAT_FORM] = &chformat_form;
   FormTable[KSI_GET_METHOD_FORM] = &getmethodhdrForm;
   FormTable[KSI_KEYSTROKE_FORM] = &KeyStrokeForm;
   FormTable[KSI_CURSOR_GOTO_FORM] = &CursorGoToForm;
}
