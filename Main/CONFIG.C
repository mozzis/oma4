
/*  Copyright (c) 1990,  EG&G Instruments Inc.                    */
/*   config.c - 20Feb1990 Morris Maynard                          */
/*              contains data and code for config form            */
/*              of oma4000 software                               */
/*
 * $Header:   J:/logfiles/oma4000/main/config.c_v   1.9   07 Jul 1992 17:09:40   maynard  $
 * $Log:   J:/logfiles/oma4000/main/config.c_v  $
 *
*/
/******************************************************************/

#include <stdlib.h>
#include <string.h>
#include <bios.h>

#include "config.h"
#include "formtabs.h"  // FormTable[]
#include "helpindx.h"
#include "gpibcom.h"
#include "points.h"    // GetDAtaPoint
#include "ksindex.h"
#include "coolstat.h"
#include "oma4000.h"
#include "formwind.h"
#include "live.h"
#include "syserror.h"  // ERROR_ALLOC_MEM
#include "omaerror.h"
#include "omameth.h"   // InitialMethod
#include "oma4driv.h" 
#include "detsetup.h"  // DET_SETUP
#include "tempdata.h"
#include "filestuf.h"
#include "curvedir.h"  // MainCurveDir
#include "curvbufr.h"
#include "forms.h"
#include "cursor.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

static WINDOW * MessageWindow;

DET_SETUP * pdet_setups_0 = &(det_setups[0]);
DET_SETUP * pdet_setups_1 = &(det_setups[1]);
DET_SETUP * pdet_setups_2 = &(det_setups[2]);
DET_SETUP * pdet_setups_3 = &(det_setups[3]);
DET_SETUP * pdet_setups_4 = &(det_setups[4]);
DET_SETUP * pdet_setups_5 = &(det_setups[5]);
DET_SETUP * pdet_setups_6 = &(det_setups[6]);
DET_SETUP * pdet_setups_7 = &(det_setups[7]);

#define SELECT_LENGTH 10
static char select_text[ SELECT_LENGTH + 1 ]       = "<-Selected";
static char erase_select_text[ SELECT_LENGTH + 1 ] = "          ";

enum {
   DET_1_LABEL,    DET_2_LABEL,    DET_3_LABEL,    DET_4_LABEL,
   DET_5_LABEL,    DET_6_LABEL,    DET_7_LABEL,    DET_8_LABEL,
   ADDR_LABEL,     ACQ_LABEL,      ROMVER_LABEL,   PROGVER_LABEL,
   FREQ_LABEL,     PIA_RLABEL,     PIA_WLABEL,     PORT_ADDR_1,
   PORT_ADDR_2,    PORT_ADDR_3,
   PORT_ADDR_4,    PORT_ADDR_5,    PORT_ADDR_6,    PORT_ADDR_7,
   PORT_ADDR_8,    ACQ_STATUS,     SYSOPTS,        SYSOPTSGO,
   CONFIG_RUN,     CONFIG_FREQ,
   PIA_READ_0,
   PIA_READ_1,
   PIA_READ_2,
   PIA_READ_3,
   PIA_READ_4,
   PIA_READ_5,
   PIA_READ_6,
   PIA_READ_7,

   PIA_WRITE_0,
   PIA_WRITE_1,
   PIA_WRITE_2,
   PIA_WRITE_3,
   PIA_WRITE_4,
   PIA_WRITE_5,
   PIA_WRITE_6,
   PIA_WRITE_7,

   ROMVER,         PROGVER
};

static SHORT dummy= 0;
static SHORT detector_field_index = PORT_ADDR_1;
SHORT readsys_index = 0;
static USHORT config_status = 0;
static SHORT pia_input[8] = {0,0,0,0,0,0,0,0};
static SHORT pia_output[8] = {0,0,0,0,0,0,0,0};

static SHORT dummySelect;
static KEY_IDLE_CALLS keyIdle_config = { NULL, NULL };

static FLOAT ProgramVer;

static enum DATAREGISTRY_ACCESS { DGROUP_DO_STRINGS = 1, DGROUP_CODE,
                                  DGROUP_TOGGLES,        DGROUP_CONFIG
};

static DATA DO_STRING_Reg[] = {

   /* 0  */ { "Detector #1", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  /* (0:1) */
   /* 1  */ { "Detector #2", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  /* (0:1) */
   /* 2  */ { "Detector #3", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  /* (0:2) */
   /* 3  */ { "Detector #4", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  /* (0:3) */
   /* 4  */ { "Detector #5", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  /* (0:4) */
   /* 5  */ { "Detector #6", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  /* (0:5) */
   /* 6  */ { "Detector #7", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  /* (0:6) */
   /* 7  */ { "Detector #8", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  /* (0:7) */
   /* 8  */ { "Port Addr.", 0, DATATYP_STRING, DATAATTR_PTR, 0 },   /* (0:8) */
   /* 9  */ { "PIA Input Bits", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 10 */ { "PIA Output Bits", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 11 */ { "Acquisition", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  /* (0:10) */
   /* 12 */ { "Software Version", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 13 */ { " Go ", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 14 */ { "Broadcast Run", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  /* (0:13) */
   /* 15 */ { "Monitor Version", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  /* (0:14) */
   /* 16 */ { "Line Frequency", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  /* (0:15) */

};

// private function declarations

PRIVATE void display_selected_indicator(char * text, unsigned char attribute);
PRIVATE SHORT form_set_freq(void);
PRIVATE SHORT update_PIA_out(void);
PRIVATE SHORT select_detector_field(void);
PRIVATE SHORT select_detector(void);
PRIVATE BOOLEAN init_config(void);
PRIVATE BOOLEAN exit_config(void);
PRIVATE SHORT broadcast_run(void);

static EXEC_DATA CODE_Reg[] = {    // took out overlay calls
   /* 0 */ { select_detector_field, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 1 */ { select_detector, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 2 */ { init_config, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 3 */ { form_set_freq, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 4 */ { broadcast_run, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 5 */ { exit_config, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 6 */ { update_PIA_out, 0, DATATYP_CODE, DATAATTR_PTR, 0 },

};

static char * readsys_options[] = { { "Read Data From Detector" } };

static char * acquisition_options[] = {
   /* 0 */ { "Inactive" },
   /* 1 */ { "Active  " },
};

static char * line_freq_options[] = {
   /* 0 */ { "50 Hz" },
   /* 1 */ { "60 Hz" },
};

static char * in_bits_options[] = {
  /* 0 */ { "0" },
  /* 1 */ { "1" },
};

static char * out_bits_options[] = {
  /* 0 */ { "1" },
  /* 1 */ { "0" },
};

static DATA TOGGLES_Reg[] = {
   /* 0  */ { line_freq_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
   /* 1  */ { acquisition_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
   /* 2  */ { readsys_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
   /* 3  */ { in_bits_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
   /* 4  */ { out_bits_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
};

static DATA CONFIG_Reg[] = {

   /* 0*/ { &pdet_setups_0, STRUCTOFFSET(DET_SETUP, det_addr), DATATYP_INT,
            DATAATTR_PTR_PTR_OFFS, 0 },

   /* 1*/ { &pdet_setups_1, STRUCTOFFSET(DET_SETUP, det_addr), DATATYP_INT,
            DATAATTR_PTR_PTR_OFFS, 0 },

   /* 2*/ { &pdet_setups_2, STRUCTOFFSET(DET_SETUP, det_addr), DATATYP_INT,
            DATAATTR_PTR_PTR_OFFS, 0 },

   /* 3*/ { &pdet_setups_3, STRUCTOFFSET(DET_SETUP, det_addr), DATATYP_INT,
            DATAATTR_PTR_PTR_OFFS, 0 },

   /* 4*/ { &pdet_setups_4, STRUCTOFFSET(DET_SETUP, det_addr), DATATYP_INT,
            DATAATTR_PTR_PTR_OFFS, 0 },

   /* 5*/ { &pdet_setups_5, STRUCTOFFSET(DET_SETUP, det_addr), DATATYP_INT,
            DATAATTR_PTR_PTR_OFFS, 0 },

   /* 6*/ { &pdet_setups_6, STRUCTOFFSET(DET_SETUP, det_addr), DATATYP_INT,
            DATAATTR_PTR_PTR_OFFS, 0 },

   /* 7*/ { &pdet_setups_7, STRUCTOFFSET(DET_SETUP, det_addr), DATATYP_INT,
            DATAATTR_PTR_PTR_OFFS, 0 },

   /* 8*/ { &(((char *)(VersionString))[0]), 0, DATATYP_STRING, DATAATTR_PTR },

   /* 9*/ { &config_status, 0, DATATYP_INT, DATAATTR_PTR, 0 },

   /*10*/ { &readsys_index, DATATYP_INT, DATAATTR_PTR, 0 },

   /*11*/ { &dummySelect, 0, DATATYP_INT, DATAATTR_PTR, 0 },

   /*12*/ { &det_setup, STRUCTOFFSET(DET_SETUP, line_freq_index),
            DATATYP_INT, DATAATTR_PTR_PTR_OFFS, 0 },

   /*13*/ { &dummy, DATATYP_INT, DATAATTR_PTR, 0 }, /* for select field "RUN" */

   /*14*/ { &det_setup, STRUCTOFFSET(DET_SETUP, Version), DATATYP_FLOAT,
            DATAATTR_PTR_PTR_OFFS, 0 },

   /*15*/ { &(pia_input[0]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*16*/ { &(pia_input[1]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*17*/ { &(pia_input[2]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*18*/ { &(pia_input[3]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*19*/ { &(pia_input[4]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*20*/ { &(pia_input[5]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*21*/ { &(pia_input[6]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*22*/ { &(pia_input[7]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*23*/ { &(pia_output[0]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*24*/ { &(pia_output[1]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*25*/ { &(pia_output[2]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*26*/ { &(pia_output[3]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*27*/ { &(pia_output[4]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*28*/ { &(pia_output[5]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*29*/ { &(pia_output[6]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /*30*/ { &(pia_output[7]), 0, DATATYP_INT, DATAATTR_PTR, 0 },
};

static FIELD ConfigForm_Fields[] = {

   label_field(DET_1_LABEL,
   DGROUP_DO_STRINGS, 0,           /* Detector  1 */
   4, 4, 11),

   label_field(DET_2_LABEL,
   DGROUP_DO_STRINGS, 1,           /* Detector  2 */
   6, 4, 11),

   label_field(DET_3_LABEL,
   DGROUP_DO_STRINGS, 2,           /* Detector  3 */
   8, 4, 11),

   label_field(DET_4_LABEL,
   DGROUP_DO_STRINGS, 3,           /* Detector  4 */
   10, 4, 11),

   label_field(DET_5_LABEL,
   DGROUP_DO_STRINGS, 4,           /* Detector  5 */
   12, 4, 11),

   label_field(DET_6_LABEL,
   DGROUP_DO_STRINGS, 5,           /* Detector  6 */
   14, 4, 11),

   label_field(DET_7_LABEL,
   DGROUP_DO_STRINGS, 6,           /* Detector  7 */
   16, 4, 11),

   label_field(DET_8_LABEL,
   DGROUP_DO_STRINGS, 7,           /* Detector  8 */
   18, 4, 11),

   label_field(ADDR_LABEL,
   DGROUP_DO_STRINGS, 8,           /* GPIB Addr." */
   2, 16, 10),

   label_field(ACQ_LABEL,
   DGROUP_DO_STRINGS, 11,          /* "Acquisition" */
   5, 40, 11),

   label_field(ROMVER_LABEL,
   DGROUP_DO_STRINGS, 15,          /* "Monitor Version" */
   17, 40, 18),

   label_field(PROGVER_LABEL,
   DGROUP_DO_STRINGS, 12,          /* "Software Version" */
   18, 40, 16),

   label_field(FREQ_LABEL,
   DGROUP_DO_STRINGS, 16,          /* "Line Frequency" */
   11, 40, 14),

   label_field(PIA_RLABEL,
    DGROUP_DO_STRINGS, 9,          /* "PIA Input Bits" */
    13, 40, 14),

   label_field(PIA_WLABEL,
    DGROUP_DO_STRINGS, 10,         /* "PIA Output Bits" */
    15, 40, 15),

   field_set(PORT_ADDR_1,
   FLDTYP_HEX_INT,
   FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_C_ADDR_1,
   CONFIG_HBASE + 0,
   DGROUP_CONFIG, 0,               /* GPIB Addr 1 */
   0, 0,
   DGROUP_CODE, 0,
   3, 3,
   4, 19, 4,
   EXIT, PORT_ADDR_1, PORT_ADDR_8, PORT_ADDR_2,
   SYSOPTSGO, SYSOPTS, PORT_ADDR_8, SYSOPTS),

   field_set(PORT_ADDR_2,
   FLDTYP_HEX_INT,
   FLDATTR_REV_VID | FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_C_ADDR_2,
   CONFIG_HBASE + 4,
   DGROUP_CONFIG, 1,               /* GPIB Addr 2 */
   0, 0,
   DGROUP_CODE, 0,
   3, 3,
   6, 19, 4,
   EXIT, PORT_ADDR_2, PORT_ADDR_1, PORT_ADDR_3,
   SYSOPTSGO, SYSOPTS, CONFIG_FREQ, PORT_ADDR_3),

   field_set(PORT_ADDR_3,
   FLDTYP_HEX_INT,
   FLDATTR_REV_VID | FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_C_ADDR_3,
   CONFIG_HBASE + 5,
   DGROUP_CONFIG, 2,               /* GPIB Addr 3 */
   0, 0,
   DGROUP_CODE, 0,
   0, 0,
   8, 19, 4,
   EXIT, PORT_ADDR_3, PORT_ADDR_2, PORT_ADDR_4,
   CONFIG_RUN, CONFIG_RUN, PORT_ADDR_2, PORT_ADDR_4),

   field_set(PORT_ADDR_4,
   FLDTYP_HEX_INT,
   FLDATTR_REV_VID | FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_C_ADDR_4,
   CONFIG_HBASE + 6,
   DGROUP_CONFIG, 3,               /* GPIB Addr 4 */
   0, 0,
   DGROUP_CODE, 0,
   0, 0,
   10, 19, 4,
   EXIT, PORT_ADDR_4, PORT_ADDR_3, PORT_ADDR_5,
   CONFIG_FREQ, CONFIG_FREQ, PORT_ADDR_3, PORT_ADDR_5),

   field_set(PORT_ADDR_5,
   FLDTYP_HEX_INT,
   FLDATTR_REV_VID | FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_C_ADDR_5,
   CONFIG_HBASE + 7,
   DGROUP_CONFIG, 4,               /* GPIB Addr 5 */
   0, 0,
   DGROUP_CODE, 0,
   0, 0,
   12, 19, 4,
   EXIT, PORT_ADDR_5, PORT_ADDR_4, PORT_ADDR_6,
   PORT_ADDR_5, PORT_ADDR_5, PORT_ADDR_4, PORT_ADDR_6),

   field_set(PORT_ADDR_6,
   FLDTYP_HEX_INT,
   FLDATTR_REV_VID | FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_C_ADDR_6,
   CONFIG_HBASE + 8,
   DGROUP_CONFIG, 5,               /* GPIB Addr 6 */
   0, 0,
   DGROUP_CODE, 0,
   0, 0,
   14, 19, 4,
   EXIT, PORT_ADDR_6, PORT_ADDR_5, PORT_ADDR_7,
   PORT_ADDR_6, PORT_ADDR_6, PORT_ADDR_5, PORT_ADDR_7),

   field_set(PORT_ADDR_7,
   FLDTYP_HEX_INT,
   FLDATTR_REV_VID | FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_C_ADDR_7,
   CONFIG_HBASE + 9,
   DGROUP_CONFIG, 6,               /* GPIB Addr 7 */
   0, 0,
   DGROUP_CODE, 0,
   0, 0,
   16, 19, 4,
   EXIT, PORT_ADDR_7, PORT_ADDR_6, PORT_ADDR_8,
   PORT_ADDR_7, PORT_ADDR_7, PORT_ADDR_6, PORT_ADDR_8),

   field_set(PORT_ADDR_8,
   FLDTYP_HEX_INT,
   FLDATTR_REV_VID | FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_C_ADDR_8,
   CONFIG_HBASE + 10,
   DGROUP_CONFIG, 7,               /* GPIB Addr 8 */
   0, 0,
   DGROUP_CODE, 0,
   0, 0,
   18, 19, 4,
   EXIT, PORT_ADDR_8, PORT_ADDR_7, PORT_ADDR_1,
   PORT_ADDR_8, PORT_ADDR_8, PORT_ADDR_7, PORT_ADDR_1),

   field_set(ACQ_STATUS,
   FLDTYP_TOGGLE,
   FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_CONFIG, 9,
   DGROUP_TOGGLES, 1,              /* Acquisition status */
   0, 0,
   0, 2,
   5, 52, 8,
   SYSOPTS, ACQ_STATUS, ACQ_STATUS, ACQ_STATUS,
   ACQ_STATUS, ACQ_STATUS, ACQ_STATUS, ACQ_STATUS),

   field_set(SYSOPTS,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_C_READSYS_OPTS,
   CONFIG_HBASE + 1,
   DGROUP_CONFIG, 10,
   DGROUP_TOGGLES, 2,              /* readsys options */
   0, 0,
   0, 1,
   7, 40, 24,
   EXIT, SYSOPTS, PIA_WRITE_0, CONFIG_RUN,
   PORT_ADDR_2, SYSOPTSGO, PORT_ADDR_1, SYSOPTSGO),

   field_set(SYSOPTSGO,
   FLDTYP_SELECT,
   FLDATTR_REV_VID,
   KSI_C_GO,
   CONFIG_HBASE + 13,
   DGROUP_CONFIG, 11,              /* dummySelect */
   DGROUP_DO_STRINGS, 13,          /* GO */
   DGROUP_CODE, 1,                 /* do action */
   0, 0,
   7, 67, 4,
   EXIT, SYSOPTSGO, CONFIG_FREQ, CONFIG_RUN,
   SYSOPTS, PORT_ADDR_2, SYSOPTS, CONFIG_RUN),

   field_set(CONFIG_RUN,
   FLDTYP_SELECT,
   FLDATTR_REV_VID,
   KSI_C_RUN,
   CONFIG_HBASE + 11,
   DGROUP_CONFIG, 13,              /* dummy */
   DGROUP_DO_STRINGS, 14,          /* "Broadcast RUN" */
   DGROUP_CODE, 4,
   0, 0,
   9, 40, 13,
   EXIT, CONFIG_RUN, SYSOPTS, CONFIG_FREQ,
   PORT_ADDR_3, PORT_ADDR_3, SYSOPTSGO, CONFIG_FREQ),

   field_set(CONFIG_FREQ,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_C_LINE,
   CONFIG_HBASE + 12,
   DGROUP_CONFIG, 12,              /* Line Freq options */
   DGROUP_TOGGLES, 0,              /* line_freq_options */
   DGROUP_CODE, 3,                 /* set_freq */
   0, 2,
   11, 55, 6,
   EXIT, CONFIG_FREQ, CONFIG_RUN, PIA_READ_0,
   PORT_ADDR_4, PORT_ADDR_4, CONFIG_RUN, PORT_ADDR_4),

   field_set(PIA_READ_0,
   FLDTYP_TOGGLE,
   FLDATTR_DISPLAY_ONLY,
   KSI_PIA_READ_0,
   CONFIG_HBASE + 32,
   DGROUP_CONFIG, 15,              /* pia_input */
   DGROUP_TOGGLES, 3,
   0, 0,                           /* code routine */
   0, 2,
   13, 56, 1,
   PIA_READ_1, PIA_READ_0, CONFIG_FREQ, PIA_WRITE_0,
   CONFIG_FREQ, PIA_READ_1, CONFIG_FREQ, PIA_READ_1),

   field_set(PIA_READ_1,
   FLDTYP_TOGGLE,
   FLDATTR_DISPLAY_ONLY,
   KSI_PIA_READ_1,
   CONFIG_HBASE + 32,
   DGROUP_CONFIG, 16,              /* pia_input */
   DGROUP_TOGGLES, 3,
   0, 0,                           /* code routine */
   0, 2,
   13, 58, 1,
   PIA_READ_2, PIA_READ_1, CONFIG_FREQ, PIA_WRITE_0,
   PIA_READ_0, PIA_READ_2, PIA_READ_0, PIA_READ_2),

   field_set(PIA_READ_2,
   FLDTYP_TOGGLE,
   FLDATTR_DISPLAY_ONLY,
   KSI_PIA_READ_2,
   CONFIG_HBASE + 32,
   DGROUP_CONFIG, 17,              /* pia_input */
   DGROUP_TOGGLES, 3,
   0, 0,                           /* code routine */
   0, 2,
   13, 60, 1,
   PIA_READ_3, PIA_READ_2, CONFIG_FREQ, PIA_WRITE_0,
   PIA_READ_1, PIA_READ_3, PIA_READ_1, PIA_READ_3),

   field_set(PIA_READ_3,
   FLDTYP_TOGGLE,
   FLDATTR_DISPLAY_ONLY,
   KSI_PIA_READ_3,
   CONFIG_HBASE + 32,
   DGROUP_CONFIG, 18,              /* pia_input */
   DGROUP_TOGGLES, 3,
   0, 0,                           /* code routine */
   0, 2,
   13, 62, 1,
   PIA_READ_4, PIA_READ_3, CONFIG_FREQ, PIA_WRITE_0,
   PIA_READ_2, PIA_READ_4, PIA_READ_2, PIA_READ_4),

   field_set(PIA_READ_4,
   FLDTYP_TOGGLE,
   FLDATTR_DISPLAY_ONLY,
   KSI_PIA_READ_4,
   CONFIG_HBASE + 32,
   DGROUP_CONFIG, 19,              /* pia_input */
   DGROUP_TOGGLES, 3,
   0, 0,                           /* code routine */
   0, 2,
   13, 64, 1,
   PIA_READ_5, PIA_READ_4, CONFIG_FREQ, PIA_WRITE_0,
   PIA_READ_3, PIA_READ_5, PIA_READ_3, PIA_READ_5),

   field_set(PIA_READ_5,
   FLDTYP_TOGGLE,
   FLDATTR_DISPLAY_ONLY,
   KSI_PIA_READ_5,
   CONFIG_HBASE + 32,
   DGROUP_CONFIG, 20,              /* pia_input */
   DGROUP_TOGGLES, 3,
   0, 0,                           /* code routine */
   0, 2,
   13, 66, 1,
   PIA_READ_6, PIA_READ_5, CONFIG_FREQ, PIA_WRITE_0,
   PIA_READ_4, PIA_READ_6, PIA_READ_4, PIA_READ_6),

   field_set(PIA_READ_6,
   FLDTYP_TOGGLE,
   FLDATTR_DISPLAY_ONLY,
   KSI_PIA_READ_6,
   CONFIG_HBASE + 32,
   DGROUP_CONFIG, 21,              /* pia_input */
   DGROUP_TOGGLES, 3,
   0, 0,                           /* code routine */
   0, 2,
   13, 68, 1,
   PIA_READ_7, PIA_READ_6, CONFIG_FREQ, PIA_WRITE_0,
   PIA_READ_5, PIA_READ_7, PIA_READ_5, PIA_READ_7),

   field_set(PIA_READ_7,
   FLDTYP_TOGGLE,
   FLDATTR_DISPLAY_ONLY,
   KSI_PIA_READ_7,
   CONFIG_HBASE + 32,
   DGROUP_CONFIG, 22,              /* pia_input */
   DGROUP_TOGGLES, 3,
   0, 0,                           /* code routine */
   0, 2,
   13, 70, 1,
   PIA_WRITE_0, PIA_READ_7, CONFIG_FREQ, PIA_WRITE_0,
   PIA_READ_6, PIA_WRITE_0, PIA_READ_6, PIA_WRITE_0),

   field_set(PIA_WRITE_0,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_PIA_WRITE_0,
   CONFIG_HBASE + 33,
   DGROUP_CONFIG, 23,              /* pia_output */
   DGROUP_TOGGLES, 4,
   DGROUP_CODE, 6,                 /* code routine */
   0, 2,
   15, 56, 1,
   EXIT, PIA_WRITE_0, CONFIG_FREQ, SYSOPTS,
   PORT_ADDR_6, PIA_WRITE_1, PORT_ADDR_6, PIA_WRITE_1),

   field_set(PIA_WRITE_1,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_PIA_WRITE_1,
   CONFIG_HBASE + 33,
   DGROUP_CONFIG, 24,              /* pia_output */
   DGROUP_TOGGLES, 4,
   DGROUP_CODE, 6,                 /* code routine */
   0, 2,
   15, 58, 1,
   EXIT, PIA_WRITE_1, CONFIG_FREQ, SYSOPTS,
   PIA_WRITE_0, PIA_WRITE_2, PIA_WRITE_0, PIA_WRITE_2),

   field_set(PIA_WRITE_2,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_PIA_WRITE_2,
   CONFIG_HBASE + 33,
   DGROUP_CONFIG, 25,              /* pia_output */
   DGROUP_TOGGLES, 4,
   DGROUP_CODE, 6,                 /* code routine */
   0, 2,
   15, 60, 1,
   EXIT, PIA_WRITE_2, CONFIG_FREQ, SYSOPTS,
   PIA_WRITE_1, PIA_WRITE_3, PIA_WRITE_1, PIA_WRITE_3),

   field_set(PIA_WRITE_3,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_PIA_WRITE_3,
   CONFIG_HBASE + 33,
   DGROUP_CONFIG, 26,              /* pia_output */
   DGROUP_TOGGLES, 4,
   DGROUP_CODE, 6,                 /* code routine */
   0, 2,
   15, 62, 1,
   EXIT, PIA_WRITE_3, CONFIG_FREQ, SYSOPTS,
   PIA_WRITE_2, PIA_WRITE_4, PIA_WRITE_2, PIA_WRITE_4),

   field_set(PIA_WRITE_4,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_PIA_WRITE_4,
   CONFIG_HBASE + 33,
   DGROUP_CONFIG, 27,              /* pia_output */
   DGROUP_TOGGLES, 4,
   DGROUP_CODE, 6,                 /* code routine */
   0, 2,
   15, 64, 1,
   EXIT, PIA_WRITE_4, CONFIG_FREQ, SYSOPTS,
   PIA_WRITE_3, PIA_WRITE_5, PIA_WRITE_3, PIA_WRITE_5),

   field_set(PIA_WRITE_5,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_PIA_WRITE_5,
   CONFIG_HBASE + 33,
   DGROUP_CONFIG, 28,              /* pia_output */
   DGROUP_TOGGLES, 4,
   DGROUP_CODE, 6,                 /* code routine */
   0, 2,
   15, 66, 1,
   EXIT, PIA_WRITE_5, CONFIG_FREQ, SYSOPTS,
   PIA_WRITE_4, PIA_WRITE_6, PIA_WRITE_4, PIA_WRITE_6),

   field_set(PIA_WRITE_6,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_PIA_WRITE_6,
   CONFIG_HBASE + 33,
   DGROUP_CONFIG, 29,              /* pia_output */
   DGROUP_TOGGLES, 4,
   DGROUP_CODE, 6,                 /* code routine */
   0, 2,
   15, 68, 1,
   EXIT, PIA_WRITE_6, CONFIG_FREQ, SYSOPTS,
   PIA_WRITE_5, PIA_WRITE_7, PIA_WRITE_5, PIA_WRITE_7),

   field_set(PIA_WRITE_7,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_PIA_WRITE_7,
   CONFIG_HBASE + 33,
   DGROUP_CONFIG, 30,              /* pia_output */
   DGROUP_TOGGLES, 4,
   DGROUP_CODE, 6,                 /* code routine */
   0, 2,
   15, 70, 1,
   EXIT, PIA_WRITE_7, CONFIG_FREQ, SYSOPTS,
   PIA_WRITE_6, PORT_ADDR_6, PIA_WRITE_6, PORT_ADDR_6),

   field_set(ROMVER,
   FLDTYP_STD_FLOAT,
   FLDATTR_DISPLAY_ONLY | FLDATTR_RJ,
   KSI_NO_INDEX,
   0,
   DGROUP_CONFIG, 14,              /* ROM version */
   0, 0,
   0, 0,
   3, 0,
   17, 59, 5,
   PROGVER, ROMVER, ROMVER, ROMVER,
   ROMVER, ROMVER, ROMVER, ROMVER),

   field_set(PROGVER,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY | FLDATTR_RJ,
   KSI_NO_INDEX,
   0,
   DGROUP_CONFIG, 8,               /* ProgramVer */
   0, 0,
   0, 0,
   5, 0,       /* 4, 0 */
   18, 59, 7,  /* 18, 59, 6 */
   PROGVER, PROGVER, PROGVER, PROGVER,
   PROGVER, PROGVER, PROGVER, PROGVER)
};

static FORM ConfigForm = {
   0,
   0,
   FORMATTR_BORDER | FORMATTR_EXIT_RESTORE | FORMATTR_VISIBLE |
   FORMATTR_FULLSCREEN,
   0, 0, 0,
   2, 0, 21, 80,
   0, 0,
   { DGROUP_CODE, 2 },
   { DGROUP_CODE, 5},
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(ConfigForm_Fields) / sizeof(ConfigForm_Fields[ 0 ]),
   ConfigForm_Fields,
   KSI_CONFIG_FORM,
   0, DO_STRING_Reg, (DATA *)CODE_Reg, TOGGLES_Reg, CONFIG_Reg, 0
};


PRIVATE SHORT form_set_freq(void)
{
  /* it's already set */
  return(FIELD_VALIDATE_SUCCESS);
}

/*********************************************************************/
/*   display_selected_indicator -                                    */
/*        this routine takes a text string, which is either the      */
/*        string "<-Selected" or an equivalent number of spaces,     */
/*        and displays to the screen with the specified attribute.   */
/*        The screen row and column are computed using the global    */
/*        detector_field_index to access the config_form_fields      */
/*        array, whence we find the screen row and column of the     */
/*        current detector entry.  No assumptions are made here      */
/*        about the relative position of each detector field on      */
/*        the form.                                                  */
/*                                                                   */
/*********************************************************************/

PRIVATE void display_selected_indicator(char * text, unsigned char attribute)
{
   SHORT row = (SHORT) (Current.Form->row +
                    Current.Form->fields[detector_field_index].row +
                    Current.Form->display_row_offset);

   SHORT column = (SHORT) (Current.Form->column +
                       Current.Form->fields[detector_field_index].column);

   column += (1 + Current.Form->fields[detector_field_index].width);

   display_string(text, SELECT_LENGTH, row, column, attribute);
}


/*********************************************************************/
/*   select_detector_field -                                         */
/*        this routine is called whenever the user presses Enter     */
/*        on a GPIB address field.  At that point the old "Selected" */
/*        box must be erased, and a new one drawn at the current     */
/*        field position.                                            */
/*        The globals detector_index and det_setup must also be     */
/*        updated.  Note that to do this, I assume that the detector */
/*        address fields are in order and contigous in the config    */
/*        form - normally not a good assumption, but because of the  */
/*        way this form is coded, the form editor will not be able   */
/*        to modify it, so all changes will be done "by hand"...     */
/*                                                                   */
/*********************************************************************/

PRIVATE SHORT select_detector_field(void)
{
   unsigned char attribute;

   if (det_setups[Current.Form->field_index - PORT_ADDR_1].det_addr == 0)
      return(FIELD_VALIDATE_SUCCESS);

   /* erase old text box */

   attribute = set_attributes(Current.FormColorSet->shaded.foreground,
                              Current.FormColorSet->shaded.background);
   display_selected_indicator(erase_select_text, attribute);

   /* reset pointers */

   detector_field_index = Current.Form->field_index;
   detector_index = detector_field_index - PORT_ADDR_1;
   det_setup = &(det_setups[detector_index]);

   /* draw old text box */

   attribute = set_attributes(Current.FormColorSet->reverse.foreground,
                              Current.FormColorSet->reverse.background);
   display_selected_indicator(select_text, attribute);

   return(FIELD_VALIDATE_SUCCESS);
}

/********************************************************************/
/*  select_detector -                                               */
/*       this routine is called whenever the user presses Enter     */
/*       on the SYSOPTS field. The action selected by that field    */
/*       can be: Read a 1461 setup into a SETUP structure,          */
/*               Send the contents of a SETUP structure to a 1461   */
/*               Read the contents of a file into a SETUP           */
/*               Write the contents of a SETUP to a file            */
/*               Get data from a previous ACCUM                     */
/*       If there is no detector at the selected address, the       */
/*       action is not done.  Also, no attempt is made to access    */
/*       a GPIB device at address 0.                                */
/*                                                                  */
/********************************************************************/

static enum SYSOPTS_OPTS { GET_DET_DATA };

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE SHORT select_detector(void)
{
  ERR_OMA err = ERROR_NONE;
  float real_detector;

  GetParam(DC_THERE, &real_detector);

  disableCoolStat();

  if (!real_detector && (init_detector(det_setup->det_addr) == 0))
      SetParam(DC_THERE, (float) FALSE);

  put_up_message_window(BusyWorking, COLORS_MESSAGE, &MessageWindow);

  if (readsys_index == GET_DET_DATA)
    err = read_detector_data();

  enableCoolStat();

  if (MessageWindow != NULL)
    release_message_window(MessageWindow);

  return(err ? FIELD_VALIDATE_WARNING : FIELD_VALIDATE_SUCCESS);
}

/********************************************************************/
/*  broadcast_run -                                                 */
/*       this routine is called whenever the user presses Enter     */
/*       on a the Broadcast Run Field. All detectors with an        */
/*       address appearing in their window (as opposed to the       */
/*       message "None") are addressed to LISTEN.  Then, the        */
/*       RUN command is sent over the bus, so all units receive     */
/*       it simultaneously.                                         */
/*                                                                  */
/********************************************************************/
PRIVATE SHORT broadcast_run(void)
{
   SHORT i, err = ERROR_NONE;
   DET_SETUP * det_save = det_setup;

   for (i = 0; i < 8; i++)
   {
      if (det_setups[i].det_addr != 0)
        {
        det_setup = &(det_setups[i]);
        err |= start_OMA_DA(0);
        }
   }
   det_setup = det_save;
   err |= SetupAcqCurveBlk(det_setup->points, det_setup->tracks *
                                                det_setup->memories);
   UpdateCursorMode(CURSORMODE_ACTIVE);
   return(err ? FIELD_VALIDATE_WARNING : FIELD_VALIDATE_SUCCESS);
}

/**********************************************************************/
/*                                                                    */
/*   read_detector_data - 04Apr1990 Morris Maynard                    */
/*                        reads the data from the selected detector   */
/*                        into a curvedir entry named after the       */
/*                        detector index. May be called by the config */
/*                        select menu field or as the result of       */
/*                        attempting to send a command to the 1461    */
/*                        with OUTPUT_READY active (cf. GPIBCOM.C)    */
/*                                                                    */
/*   I noticed in Feb 1992 this was changed so it did nothing         */
/*   with the recieved data.  Not too cool.                           */
/*                                                                    */
/*   restored to operation April 1992                                 */
/*                                                                    */
/**********************************************************************/
ERR_OMA read_detector_data(void)
{
  SHORT TransferBlkIndex, LiveIndex, prefBuf = 0;
  USHORT curves;
  ERR_OMA err;
  static USHORT data_index = 0;
  static char NewName[14];
  static char NewDesc[DESCRIPTION_LENGTH];
  CURVEHDR far * TCurvehdr;
  float real_detector;

  GetParam(DC_THERE, &real_detector);

  if (!real_detector)
    return(ERROR_FAKEDETECTOR);

  err = SetupAcqCurveBlk(det_setup->points, det_setup->tracks *
                                            det_setup->memories);

  LiveIndex = SearchCurveBlkDir(LastLiveEntryName, "", 0, &MainCurveDir);

  if (err || LiveIndex == -1)
    {
    err = error(ERROR_NO_LIVE_DATA);
    }
  else
    {
    float X, Y;

    sprintf(NewName, "ACCUM%-1.1d.DAT", data_index);
    sprintf(NewDesc, "Data from detector #%d", detector_index);

    curves = MainCurveDir.Entries[LiveIndex].count;

    GetDataPoint(&MainCurveDir, LiveIndex, 0, 0, &X, &Y, FLOATTYPE, &prefBuf);
    TransferBlkIndex = MainCurveDir.BlkCount;    // Append to curve directory

    TCurvehdr = calloc(1, sizeof(CURVEHDR));
    if (TCurvehdr == NULL)
      {
      err = error(ERROR_ALLOC_MEM);
      }
    else
      {
      if (!(err = ReadTempCurvehdr(&MainCurveDir, LiveIndex, 0,TCurvehdr)))
        {
        TCurvehdr->MemData = FALSE;
        TCurvehdr->CurveCount = 1;

        err = CreateTempFileBlk(&MainCurveDir,     /* as always */
                                &TransferBlkIndex, /* append to directory */
                                NewName,           /* new name */
                                "",                /* no path */
                                NewDesc,           /* new description */
                                0, 0L,             /* no index or offset */
                                0,                 /* no curves yet */
                                TCurvehdr,
                                OMA4DATA, 0);

        if (! err)
          {
          err = InsertMultiTempCurve(&MainCurveDir,
                                    LiveIndex, 0,
                                    TransferBlkIndex, 0, curves);
          }
        }
      }
    free(TCurvehdr);
    }
  return err;
  }

PRIVATE USHORT collect_bits(SHORT * array)
{
  USHORT i, bits = 0;

  for (i = 0; i <= 7; i++)
    bits |= ((array[7-i] != 0) << i);
  return(bits);
}

PRIVATE void distrib_bits(USHORT bits, SHORT * array)
{
  USHORT i;

  for (i = 0; i <= 7; i++)
    array[7-i] = ((bits & (1 << i)) != 0);
}

PRIVATE SHORT update_PIA_out(void)
{
  USHORT pia_bits = collect_bits(pia_output);
  set_PIA_Out(pia_bits);
  return(FIELD_VALIDATE_SUCCESS);
}

// register config form in formTable[]
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void registerConfigForm(void)
{
   FormTable[KSI_CONFIG_FORM] = &ConfigForm;
}

static unsigned char call_prev_idle_handler(void)
{
   if(keyboard_idle -> prev_handler) {

      KEY_IDLE_CALLS * pTemp = keyboard_idle;
      UCHAR cTemp;

      keyboard_idle = keyboard_idle -> prev_handler;
      cTemp = (* (keyboard_idle -> current_handler)) ();
      keyboard_idle = pTemp;
      return cTemp;
   }
   else
      return NIL;
}

PRIVATE void display_active_status(void)
{
  USHORT temp_status;
  USHORT pia_in_status;

  get_DaActive((SHORT *)&temp_status);
  if (config_status != temp_status)
    {
    CHAR * str = acquisition_options[temp_status];
    
    config_status = temp_status;
    cheat_display(&ConfigForm, ACQ_STATUS, str);
    }

  get_PIA_In(&temp_status);
  pia_in_status = collect_bits(pia_input);

  if (pia_in_status != temp_status)
    {
    SHORT i;
    CHAR * bit_str;

    distrib_bits(temp_status, pia_input);
    for (i = PIA_READ_0; i<= PIA_READ_7; i++)
      {
      bit_str = in_bits_options[pia_input[i-PIA_READ_0]];
      cheat_display(&ConfigForm, i, bit_str);
      }
    }
}

static unsigned char update_active_status(void)
{
#define WAIT_INTERVAL 200

  static long next_showing = 0L;
  long now;

  now = clock();

  if(next_showing <= now)
    {
    next_showing = now + WAIT_INTERVAL;

    display_active_status();
    }
  return call_prev_idle_handler();
}

/*********************************************************************/
/*   first_display -                                                 */
/*        when config form is first started, the init routine        */
/*        init_config is called.  That routine makes sure that       */
/*        first_display will run the next time the keyboard is       */
/*        idle.  First_display draws the "Selected" box next         */
/*        to the correct detector entry on the form.  This has to    */
/*        be done after the form is drawn, so doing as part of       */
/*        init_config won't work.  Then the keyboard idle routine    */
/*        is changed to the routine which updates the Locked         */
/*        and Active fields based on 1461 serial poll bits.          */
// 4/19/91 RAC. update cooler locked removed from this form, the
//   first_display() function removes itself from the keyboard idle
//   loop instead.
/*                                                                   */
/*********************************************************************/

PRIVATE unsigned char first_display(void)
{
  unsigned char  attribute =
                 set_attributes(Current.FormColorSet->reverse.foreground,
                                Current.FormColorSet->reverse.background);

  display_selected_indicator(select_text, attribute);

  /* replace first_display() in the keyboard idle loop w update_active */

  keyIdle_config.current_handler = update_active_status;

  return((unsigned char)FALSE);
}

/********************************************************************/
/*  init_config -                                                   */
/*       when config form is first started, the init routine        */
/*       init_config is called.  Its entire purpose is to see       */
/*       that first_display is done soon after the form is drawn,   */
/*       so the selected detector will be displayed properly.       */
/*       It does this by using the keyboard idle function - when    */
/*       the form system is waiting for input, the idle function    */
/*       is called.  See the note on first_display for more info.   */
/*                                                                  */
/********************************************************************/

PRIVATE BOOLEAN init_config(void)
{
   SHORT bits;

   get_DaActive(&config_status);
   get_PIA_In(&bits);
   distrib_bits(bits, pia_input);
   get_PIA_Out(&bits);
   distrib_bits(bits, pia_output);
   keyIdle_config.current_handler = first_display;
   keyIdle_config.prev_handler = keyboard_idle;
   keyboard_idle = &keyIdle_config;

   return FALSE;
}

PRIVATE BOOLEAN exit_config(void)
{
  if(keyIdle_config.current_handler)
    {
    keyboard_idle = keyIdle_config.prev_handler;
    keyIdle_config.prev_handler = NULL;
    keyIdle_config.current_handler = NULL;
    }
  return FALSE;
}



