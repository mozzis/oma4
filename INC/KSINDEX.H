/* -----------------------------------------------------------------------
/
/  KSINDEX.H
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/KSINDEX.H 1     6/18/99 1:52a Maynard $
/
*/
  
#ifndef KSINDEX_INCLUDED
#define KSINDEX_INCLUDED

typedef struct
{
   int Lower;
   int Higher;
} BOUNDS;
  
#define FORM_NAME_SIZE     20       // maximum command name size
  
/***************************************************************************/
/*                                                                         */
/*                         FORM Indices                                    */
/*	 MUST agree with the order of fields in formlist.c!							*/
/*                                                                         */
/***************************************************************************/

enum {
  /* 00 */  KSI_NO_INDEX = 0,
  /* 01 */  KSI_DA_FORM = 1,
  /* 02 */  KSI_PLOT_FORM,
  /* 03 */  KSI_FORMAT_FORM,
  /* 04 */  KSI_GET_METHOD_FORM,
  /* 05 */  KSI_LIVE_DISK_FORM,
  /* 06 */  KSI_LIVE_FORM,
  /* 07 */  KSI_ACCUM_FORM,
  /* 08 */  KSI_KEYSTROKE_FORM,
  /* 09 */  KSI_BACKGROUND_FORM,
  /* 10 */  KSI_MATH_FORM,
  /* 11 */  KSI_STAT_FORM,
  /* 12 */  KSI_FILE_DIR_FORM,
  /* 13 */  KSI_FD_CURVE_SCROLL,
  /* 14 */  KSI_FD_FILE_SCROLL,
  /* 15 */  KSI_FD_USER,
  /* 16 */  KSI_XCAL_FORM,
  /* 17 */  KSI_XCAL_SCROLL_FORM,
  /* 18 */  KSI_XCAL_POINT_FORM,
  /* 19 */  KSI_YCAL_FORM,
  /* 20 */  KSI_YKNOTS_FORM,
  /* 21 */  KSI_SCAN_SETUP,
  /* 22 */  KSI_SPECIALS,
  /* 23 */  KSI_SLICE_FORM,
  /* 24 */  KSI_SLICE_SCROLL_FORM,
  /* 25 */  KSI_TRACK_FORM,
  /* 26 */  KSI_TRACK_SCROLL_FORM,
  /* 27 */  KSI_CONFIG_FORM,
  /* 28 */  KSI_BASLN_FORM,
  /* 29 */  KSI_KNOTS_FORM,
  /* 30 */  KSI_CURSOR_GOTO_FORM,
  /* 31 */  KSI_TAG_CURVES_FORM,
  /* 32 */  KSI_SAVE_TAGGED_FORM,
  /* 33 */  KSI_SPGRAPH_FORM,
  /* 34 */  KSI_SPGRAPH_POINT_FORM,
  /* 35 */  KSI_RAPDA_FORM,
  /* 36 */  KSI_REGSCROLL_FORM,
  /* 37 */  NUM_FORMS,               // = KSI_REGSCROLL_FORM,
  /* menus */
  /* 38 */  KSI_FILES_MENU,
  /* 39 */  KSI_PLOT_MENU, 
  /* 40 */  KSI_RUN_MENU,  
  /* 41 */  KSI_MATH_MENU, 
  /* 42 */  KSI_CALIB_MENU,
  /* 43 */  KSI_MAIN_MENU, 
};
  
#define NUM_MENUS             KSI_MAIN_MENU - NUM_FORMS

#define KSI_FKEY_HOT          KSI_MAIN_MENU + 1
#define KSI_ALT_FKEY_HOT      KSI_MAIN_MENU + 2
#define KSI_ALT_KEY_HOT       KSI_MAIN_MENU + 3
#define KSI_SPC_KEY_HOT       KSI_MAIN_MENU + 4
#define KSI_CONTROL_FKEY_HOT  KSI_MAIN_MENU + 5
#define KSI_SHIFT_FKEY_HOT    KSI_MAIN_MENU + 6
#define KSI_CTRL_KEY_HOT      KSI_MAIN_MENU + 7
  
/***************************************************************************/
//                         FIELD Indices                                    /
//	 MUST agree with the order of fields in formlist.c!
/***************************************************************************/
  
enum { 
  /* 001 */ KSI_MEMS = 1,           // RunSetup fields
  /* 002 */ KSI_START_MEM,
  /* 003 */ KSI_DA_MODE,
  /* 004 */ KSI_DA_NAME,
  /* 005 */ KSI_SCANS,
  /* 006 */ KSI_IGN_SCANS,
  /* 007 */ KSI_PREP,
  /* 008 */ KSI_ET,
  /* 009 */ KSI_TSOURCE,
  /* 010 */ KSI_RS_IMODE,
  /* 011 */ KSI_FPWIDTH,
  /* 012 */ KSI_FPDELAY,
  /* 013 */ KSI_DELINC,
  /* 014 */ KSI_DELRANGE,
  /* 015 */ KSI_TCOUNT,
  /* 016 */ KSI_AUDIO,
  /* 017 */ KSI_TTHRESH,
  /* 018 */ KSI_TVOLTS,
  /* 019 */ KSI_DET_TEMP,
  /* 020 */ KSI_COOL_ONOFF,
  /* 021 */ KSI_PIX_TIME_OPTS,
  /* 022 */ KSI_SFT_TIME_OPTS,
  /* 023 */ KSI_64_SCOMP,
  /* 024 */ KSI_KPCLEAN_SEL,
  /* 025 */ KSI_64_SYNC_OPTS,
  /* 026 */ KSI_OPEN_SYNC,
  /* 027 */ KSI_CLOSE_OPTS,
  /* 028 */ KSI_SHUTTER_OPTS,
  /* 029 */ KSI_PLOT_TITLE,         // PlotSetupFormFields
  /* 030 */ KSI_ACTIVE_WINDOW,
  /* 031 */ KSI_X_LABEL,
  /* 032 */ KSI_X_MIN,
  /* 033 */ KSI_X_MAX,
  /* 034 */ KSI_X_UNITS,
  /* 035 */ KSI_Y_LABEL,
  /* 036 */ KSI_Y_MIN,
  /* 037 */ KSI_Y_MAX,
  /* 038 */ KSI_Y_UNITS,
  /* 039 */ KSI_Z_LABEL,
  /* 040 */ KSI_Z_MIN,
  /* 041 */ KSI_Z_MAX,
  /* 042 */ KSI_Z_UNITS,
  /* 043 */ KSI_Z_SIDE_OPTS,
  /* 044 */ KSI_Z_PER_X,
  /* 045 */ KSI_Z_PER_Y,
  /* 046 */ KSI_PLOT_METHOD,
  /* 047 */ KSI_AUTOSCALE_X,
  /* 048 */ KSI_AUTOSCALE_Y,
  /* 049 */ KSI_AUTOSCALE_Z,
  /* 050 */ KSI_WINDOW_STYLE,
  /* 051 */ KSI_PK_LABEL,
  /* 052 */ KSI_LPLOT_MODE,
  /* 053 */ KSI_PCOLOR_MODE,
  /* 054 */ KSI_ACTIVE_PLOT,
  /* 055 */ KSI_XLATE_IN_NAME,       // chformat_formFields
  /* 056 */ KSI_XLATE_START,
  /* 057 */ KSI_XLATE_COUNT,
  /* 058 */ KSI_XLATE_OUT_NAME,
  /* 059 */ KSI_XLATE_FORMAT_OPTS,
  /* 050 */ KSI_XLATE_GO,
  /* 061 */ KSI_GM_OUT_NAME,         // getmethodhdrFields
  /* 062 */ KSI_GM_DESC,
  /* 063 */ KSI_GM_OPTS,
  /* 064 */ KSI_GM_GO,
  /* 065 */ KSI_LD_OUT_NAME,         // LIVE_TO_DISK_FORMFields
  /* 066 */ KSI_LD_REPS,
  /* 067 */ KSI_LD_OPTS,
  /* 068 */ KSI_LD_GO,
      // MacroFormFields not allowed
  /* 069 */ KSI_LVFILT,             // LiveFormFields
  /* 070 */ KSI_LVREFNAME,
  /* 071 */ KSI_LIVE_OUT_NAME,      
  /* 072 */ KSI_LIVE_OPTS,
  /* 073 */ KSI_ACCUM_OUT_NAME,     // AccumFormFields
  /* 074 */ KSI_KS_NAME,            // KeyStrokeFormFields
  /* 075 */ KSI_KS_OPTS,
  /* 076 */ KSI_KS_DELAY,
  /* 077 */ KSI_KS_GO,
  /* 078 */ KSI_BGRND_NAME,         // BackGroundFields
  /* 079 */ KSI_BGRND_START,
  /* 080 */ KSI_BGRND_COUNT,
  /* 081 */ KSI_BGRND_GO,
  /* 082 */ KSI_MATH_OP1_NAME,      // MathFormFields
  /* 083 */ KSI_MATH_OP1_START,
  /* 084 */ KSI_MATH_OP1_COUNT,
  /* 085 */ KSI_MATH_OPERATOR,
  /* 086 */ KSI_MATH_OP2_NAME,
  /* 087 */ KSI_MATH_OP2_START,
  /* 088 */ KSI_MATH_OP2_COUNT,
  /* 089 */ KSI_MATH_RES_NAME,
  /* 090 */ KSI_MATH_RES_START,
  /* 091 */ KSI_MATH_RES_COUNT,
  /* 092 */ KSI_MATH_GO,
  /* 093 */ KSI_STAT_SOURCE,        // StatFormFields
  /* 094 */ KSI_STAT_CRVSTART, 
  /* 095 */ KSI_STAT_CRVCOUNT, 
  /* 096 */ KSI_STAT_PNTSTART, 
  /* 097 */ KSI_STAT_PNTCOUNT, 
  /* 098 */ KSI_STAT_GO,
  /* 099 */ KSI_FD_FILESPEC,        // fileform ControlFields
  /* 100 */ KSI_FD_CURVE_CHOICE,    // fileform CurveScrollFields
  /* 101 */ KSI_FD_FILE_CHOICE,     // fileform FileScrollFields
  /* 102 */ KSI_FD_FILE_NAME,       // fileform UserFormFields
  /* 103 */ KSI_FD_DESC,
  /* 104 */ KSI_FD_ACTION,
  /* 105 */ KSI_FD_GO,
  /* 106 */ KSI_FD_SRC_START,
  /* 107 */ KSI_FD_SRC_COUNT,
  /* 108 */ KSI_FD_DST_INDEX,
  /* 109 */ KSI_FD_DST_START,
  /* 110 */ KSI_FD_WINDOW_1,
  /* 111 */ KSI_FD_WINDOW_2,
  /* 112 */ KSI_FD_WINDOW_3,
  /* 113 */ KSI_FD_WINDOW_4,
  /* 114 */ KSI_FD_WINDOW_5,
  /* 115 */ KSI_FD_WINDOW_6,
  /* 116 */ KSI_FD_WINDOW_7,
  /* 117 */ KSI_FD_WINDOW_8,
  /* 118 */ KSI_XCAL_UNITS,         // X_CalibrationFormFields
  /* 119 */ KSI_XCAL_EXCIT,
  /* 120 */ KSI_XCAL_DEGREE,
  /* 121 */ KSI_XCAL_SCOPE,
  /* 122 */ KSI_XCAL_DEL_POINT,
  /* 123 */ KSI_XCAL_ACTION,
  /* 124 */ KSI_XCAL_GO,
  /* 125 */ KSI_XCAL_PARAM_1,
  /* 126 */ KSI_XCAL_PARAM_2,
  /* 127 */ KSI_XCAL_PARAM_3,
  /* 128 */ KSI_XCAL_PARAM_4,
  /* 129 */ KSI_XCAL_X_VAL,         // X_CalScrollFormFields
  /* 130 */ KSI_XCAL_GP_VAL,        // GetCalibPointFormFields
  /* 131 */ KSI_YCAL_LAMP_DATA,     // YCalibFormFields
  /* 132 */ KSI_YCAL_LAMP_UNITS,
  /* 133 */ KSI_YCAL_LAMP_ACT,
  /* 134 */ KSI_YCAL_LAMP_CURVE,
  /* 135 */ KSI_YCAL_CORR_CURVE,
  /* 136 */ KSI_YCAL_CORR_CREATE,
  /* 137 */ KSI_YCAL_REAL_CURVE,
  /* 138 */ KSI_YCAL_REAL_APPLY,
  /* 139 */ KSI_YCAL_ADDKNOT,
  /* 140 */ KSI_YCAL_DELKNOT,
  /* 141 */ KSI_YCAL_XKNOT_SCR,     // YKnotsFormFields
  /* 142 */ KSI_YCAL_YKNOT_SCR,
  /* 143 */ KSI_SLICES,             // ScanSetupFormFields
  /* 147 */ KSI_SLICE_MODE,
  /* 148 */ KSI_TRACKS,
  /* 149 */ KSI_TRACK_MODE,
  /* 150 */ KSI_ANTIBLOOM,          // SpecialsFormFields (scan setup)
  /* 151 */ KSI_OUT_REG,
  /* 144 */ KSI_STREAKMODE,
  /* 145 */ KSI_STREAKROWS,
  /* 146 */ KSI_PRESCAN,
  /* 152 */ KSI_SHIFT_MODE,         
  /* 153 */ KSI_ACTIVEX,
  /* 154 */ KSI_ACTIVEY,
  /* 155 */ KSI_X0,                 // SliceForm
  /* 156 */ KSI_DELTA_X,
  /* 157 */ KSI_SCROLL_X0,          // SliceScrollFormFields
  /* 158 */ KSI_SCROLL_DELTA_X,
  /* 159 */ KSI_Y0,                 // TrackForm
  /* 160 */ KSI_DELTA_Y,
  /* 161 */ KSI_SCROLL_Y0,          // TrackScrollFormFields
  /* 162 */ KSI_SCROLL_DELTA_Y,
  /* 163 */ KSI_C_ADDR_1,           // ConfigForm
  /* 164 */ KSI_C_ADDR_2,
  /* 165 */ KSI_C_ADDR_3,
  /* 166 */ KSI_C_ADDR_4,
  /* 167 */ KSI_C_ADDR_5,
  /* 168 */ KSI_C_ADDR_6,
  /* 169 */ KSI_C_ADDR_7,
  /* 170 */ KSI_C_ADDR_8,
  /* 171 */ KSI_C_READSYS_OPTS,
  /* 172 */ KSI_C_GO,
  /* 173 */ KSI_C_RUN,
  /* 174 */ KSI_C_LINE,
  /* 175 */ KSI_PIA_READ_0,
  /* 176 */ KSI_PIA_READ_1,
  /* 177 */ KSI_PIA_READ_2,
  /* 178 */ KSI_PIA_READ_3,
  /* 179 */ KSI_PIA_READ_4,
  /* 180 */ KSI_PIA_READ_5,
  /* 181 */ KSI_PIA_READ_6,
  /* 182 */ KSI_PIA_READ_7,
  /* 183 */ KSI_PIA_WRITE_0,
  /* 184 */ KSI_PIA_WRITE_1,
  /* 185 */ KSI_PIA_WRITE_2,
  /* 186 */ KSI_PIA_WRITE_3,
  /* 187 */ KSI_PIA_WRITE_4,
  /* 188 */ KSI_PIA_WRITE_5,
  /* 189 */ KSI_PIA_WRITE_6,
  /* 190 */ KSI_PIA_WRITE_7,
  /* 191 */ KSI_BL_ACTION,          // baseline_sub_fields
  /* 192 */ KSI_BL_Z,
  /* 193 */ KSI_BL_THRESHOLD,
  /* 194 */ KSI_BL_NAME,
  /* 195 */ KSI_BL_SHOW_CURVE,
  /* 196 */ KSI_BL_SHOW_BL,
  /* 197 */ KSI_BL_SHOW_SUM,
  /* 198 */ KSI_BL_SHOW_PEAKS,
  /* 199 */ KSI_BL_SHOW_ONE,
  /* 200 */ KSI_BL_DEL_KNOT,
  /* 201 */ KSI_BL_KNOT_Y_VAL,      // knots_form_fields
  /* 202 */ KSI_CGT_X,              // CursorGoToFormFields
  /* 203 */ KSI_CGT_Z,
  /* 204 */ KSI_CGT_GO,
  /* 205 */ KSI_TC_ZSTART,          // tag curves form fields     
  /* 206 */ KSI_TC_DELTA,
  /* 207 */ KSI_TC_GO,
  /* 208 */ KSI_ST_FILE_NAME,       // save tagged curves form fields     
  /* 209 */ KSI_ST_GO,
  /* 211 */ KSI_SPEC_SELECT,        // spectrograph form fields
  /* 212 */ KSI_SPEC_RESET,
  /* 213 */ KSI_SPEC_GRATING,
  /* 214 */ KSI_SPEC_GETLAM,
  /* 215 */ KSI_SPEC_OFFPXL,
  /* 216 */ KSI_SPEC_OFFWVL,
  /* 217 */ KSI_SPEC_SETOFFSET,
  /* 218 */ KSI_SPEC_GO,
  /* 219 */ KSI_SPEC_DISP,
  /* 220 */ KSI_SPGRAPH_GP_VAL,     // GetOffsetPointFormFields
  /* 221 */ KSI_REGIONS,            // rapda form fields
  /* 222 */ KSI_BASET,
  /* 223 */ KSI_ARRSIZE,
  /* 224 */ KSI_REG_X0,             // region scroll form fields
  /* 225 */ KSI_REGSIZE,
  /* 226 */ KSI_REGET,

  /* 227 */ NUM_FIELDS =  KSI_REGET
};

#endif  //  KSINDEX_INCLUDED
