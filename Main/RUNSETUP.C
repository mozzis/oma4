/*
/ FILE : runsetup.c  MLM
/   run setup form and associated functions
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/runsetup.c_v   1.21   16 Jul 1992 17:49:52   maynard  $
/  $Log:   J:/logfiles/oma4000/main/runsetup.c_v  $
/
/ --------------------------------------------------------------------------
*/

#include <stdlib.h>   // NULL
#include <time.h>
#include <sys/timeb.h>
#include "runsetup.h"
#include "ksindex.h"
#include "helpindx.h"
#include "oma4driv.h"
#include "detsetup.h"
#include "config.h"
#include "formtabs.h"
#include "omaform.h"   // COLORS_DEFAULT
#include "omaerror.h"
#include "forms.h"
#include "curvbufr.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

#define SBOX_ROW 9
#define PBOX_ROW 7
#define PBOX_COL 2

// private functions
PRIVATE BOOLEAN init_da_form(int dummy);
PRIVATE BOOLEAN exit_da_form(int dummy);
PRIVATE unsigned char update_pulser_status(void);
PRIVATE void display_pulser_status(void);
PRIVATE unsigned char pulse_form_vect_select(FORM_CONTEXT *);
PRIVATE int update_src_comp(void);
PRIVATE int update_DA_mode(void);
PRIVATE int update_DA_name(void);
PRIVATE int update_dtype(void);
PRIVATE int update_src_comp(void);
PRIVATE int update_da_prog(void);
PRIVATE int update_scans(void);
PRIVATE int update_mems(void);
PRIVATE int update_ignores(void);
PRIVATE int update_et(void);
PRIVATE int update_preps(void);
PRIVATE int update_pixtime(void);
PRIVATE int update_shifttime(void);
PRIVATE int update_coolOnOff(void);
PRIVATE int update_temp(void);
PRIVATE int update_Intensifier();
PRIVATE int update_PulserAudio();
PRIVATE int update_PulserDelay();
PRIVATE int update_PulserDelayInc();
PRIVATE int update_PulserDelayRange();
PRIVATE int update_PulserTrigCount();
PRIVATE int update_PulserTrigSrc();
PRIVATE int update_PulserTrigThresh();
PRIVATE int update_PulserTrigVolts();
PRIVATE int update_PulserType();
PRIVATE int update_PulserWidth();
PRIVATE int update_control_mode(void);
PRIVATE int update_shutter_mode(void);
PRIVATE int update_waitopen(void);
PRIVATE int update_waitclose(void);
PRIVATE int update_keepclean(void);
PRIVATE int update_startmem(void);

enum { DGROUP_DO_STRINGS = 1, DGROUP_FORMS, DGROUP_TOGGLES, DGROUP_CODE,
       DGROUP_1461
};

static KEY_IDLE_CALLS keyIdle_pulser = { NULL, NULL };
static int update_enabled = FALSE;  // start out disabled

static DATA DO_String_Reg[] = {
   /* 0  */ { "DA Mode    ", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 1  */ { "Data Size", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 2  */ { "bits", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 3  */ { "Data Scans", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 4  */ { " Memories", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 5  */ { "Ignored Scans", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 6  */ { "Exposure Time", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 7  */ { "Detector Temp", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 8  */ { "øC  Control", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 9  */ { "Detector type", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 10 */ { "Source Comp", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 11 */ { "Shutter Control", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 12 */ { "Detector Number", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 13 */ { "Max", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 14 */ { "Shutter mode", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 15 */ { "Pulser Width", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 16 */ { "sec.", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 17 */ { "Pixel Time", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 18 */ { "Prep Frames", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 19 */ { "for Sync Before", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 20 */ { "for Sync After",  0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 21 */ { "Pulser type", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 22 */ { "Pulser Setup", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 23 */ { "Trigger Source", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 24 */ { "Intensifier Mode", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 25 */ { "Start  Delay", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 26 */ { "Pulser type", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 27 */ { "Delay Inc/Scan", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 28 */ { "Delay Range", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 29 */ { "Trigger Count", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 30 */ { "per scan", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 31 */ { "Start", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 32 */ { "Scan Timeout", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 33 */ { "Audio", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 34 */ { "Trigger Level", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 35 */ { "Frame Time", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 36 */ { "Equipment Summary", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 37 */ { "V", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 38 */ { "Keep Clean", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 39 */ { "æsec", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 40 */ { "Shift Time", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 41 */ { "Streak Time", 0, DATATYP_STRING, DATAATTR_PTR, 0 },

};

// form defined later, just need declaration for now
static FORM PulserBox_Form;
static FORM InfoBox_Form;

static DATA FORMS_Reg[] = {
   { & PulserBox_Form,   0, DATATYP_VOID, DATAATTR_PTR, 0 },
   { & InfoBox_Form,   0, DATATYP_VOID, DATAATTR_PTR, 0 }
};

static char * dsize_opts[] =    { "16", "32" };

static char * det_type_opts[] = { "No Detector",
                                  "Thomson 512 CCD",
                                  "Dual Channel CCD",
                                  "EEV 1024 X 256 CCD",
                                  "Model 1512 PDA",
                                  "Thomson 1024 X 256 CCD",
                                  "Thomson 1024 X 1024 CCD"};

static char * syncwait_opts[] = { "Don't Wait", "Wait" };

static char * on_options[] =    { "Off", "On" };
                                 
static char * pixtime_opts[] =  { "Normal", "Fast", "Slow" };

static char * scomp_opts[] =    { "Sample",
                                  "10 æs ",
                                  "100 æs",
                                  "1 ms  ",
                                  "10 ms ",
                                  "100 ms",
                                  " 1 sec"};

static char * kpclean_opts[] =  {  "Fast",
                                   "Normal" };

static char * pulser_opts[] =   { "No Pulser or Shutter",
                                  "Shutter Option",
                                  "5ns Pulsed Intensifier",
                                  "Pulser Option 1",
                                  "Pulser Option 2",
                                  "Timer Board w/o Intensifier" };

static char * cooler_opts[] =   { "No Cooler", "Cryo Cooler", "Peltier Cooler" };

static char * shutmode_opts[] = { "Normal", "Forced Closed", "Forced Open" };

static char * sync_options[] =  { "Internal", "External" };


static char * tsource_opts[] = { "Disable", "Internal", "Optical",
                                 "/Slope", "\\Slope"  };

static char * intens_opts[]  = { "CW  ", "Gate" };

static char * scanend_opts[] = { "Timeout", "Trig Count"  };

static char * tthresh_opts[] = { "TTL", "NIM F", "NIM S", "Volts" };

static enum tthresh_vals { TTL_, FNIM_, SNIM_, VOLTS_ };

static char * numbers[] =  {
                             "1", "2", "3", "4", "5", "6", "7", "8", "9",
                            "10", "11", "12", "13", "14", "15", "16", "17",
                            "18", "19", "20", "21", "22", "23", "24"
                           };

static char *da_prog_names[40] = { /* 00 */ "NONE\0 789",
                                   /* 01 */ "NONE\0 789",
                                   /* 02 */ "NONE\0 789",
                                   /* 03 */ "NONE\0 789",
                                   /* 04 */ "NONE\0 789",
                                   /* 05 */ "NONE\0 789",
                                   /* 06 */ "NONE\0 789",
                                   /* 07 */ "NONE\0 789",
                                   /* 08 */ "NONE\0 789",
                                   /* 09 */ "NONE\0 789",
                                   /* 10 */ "NONE\0 789",
                                   /* 11 */ "NONE\0 789",
                                   /* 12 */ "NONE\0 789",
                                   /* 13 */ "NONE\0 789",
                                   /* 14 */ "NONE\0 789",
                                   /* 15 */ "NONE\0 789",
                                   /* 16 */ "NONE\0 789",
                                   /* 17 */ "NONE\0 789",
                                   /* 18 */ "NONE\0 789",
                                   /* 19 */ "NONE\0 789",
                                   /* 20 */ "NONE\0 789",
                                   /* 21 */ "NONE\0 789",
                                   /* 22 */ "NONE\0 789",
                                   /* 23 */ "NONE\0 789",
                                   /* 24 */ "NONE\0 789",
                                   /* 25 */ "NONE\0 789",
                                   /* 26 */ "NONE\0 789",
                                   /* 27 */ "NONE\0 789",
                                   /* 28 */ "NONE\0 789",
                                   /* 29 */ "NONE\0 789",
                                   /* 30 */ "NONE\0 789",
                                   /* 31 */ "NONE\0 789",
                                   /* 32 */ "NONE\0 789",
                                   /* 33 */ "NONE\0 789",
                                   /* 34 */ "NONE\0 789",
                                   /* 35 */ "NONE\0 789",
                                   /* 36 */ "NONE\0 789",
                                   /* 37 */ "NONE\0 789",
                                   /* 38 */ "NONE\0 789",
                                   /* 39 */ "NONE\0 789",
                                 };

PRIVATE SHORT da_prog;
PRIVATE SHORT max_da_prog;
PRIVATE SHORT memories;
PRIVATE SHORT startmem = 1;
PRIVATE SHORT ignores;
PRIVATE SHORT scans;
PRIVATE SHORT preps;
PRIVATE FLOAT exposure_time;
PRIVATE SHORT tempset;
PRIVATE SHORT pixtime_index;
PRIVATE SHORT shifttime_index;
PRIVATE SHORT conmode_index;
PRIVATE SHORT waitopen;
PRIVATE SHORT waitclose;
PRIVATE SHORT shutmode;
PRIVATE SHORT psize;
PRIVATE SHORT detype_index;
PRIVATE SHORT max_memory;
PRIVATE SHORT pulser_type;
PRIVATE SHORT tsource_index;
PRIVATE SHORT imode_index;
PRIVATE SHORT cooler_index = 1;
PRIVATE FLOAT pulser_width;
PRIVATE FLOAT pulser_delay;
PRIVATE FLOAT pulser_inc = (float)10e-9;
PRIVATE FLOAT pulser_range = (float)10e-6;
PRIVATE SHORT pulser_count = 0;
PRIVATE SHORT audio_index = 0;
PRIVATE SHORT tthresh_index = 0;
PRIVATE float tthresh_volts;
PRIVATE SHORT cooler_OnOff;
PRIVATE SHORT scomp_index;
PRIVATE SHORT keepclean_index;
PRIVATE float frame_time;
PRIVATE float streak_time;
PRIVATE SHORT pix_usecs;
PRIVATE SHORT shift_usecs;

static DATA TOGGLES_Reg[] = {
 /* 0  */  { dsize_opts,    0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 1  */  { det_type_opts, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 2  */  { numbers,       0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 3  */  { on_options,    0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 4  */  { sync_options,  0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 5  */  { pixtime_opts,  0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 6  */  { scomp_opts,    0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 7  */  { pulser_opts,   0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 8  */  { syncwait_opts, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 9  */  { shutmode_opts, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 10 */  { tsource_opts,  0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 11 */  { intens_opts,   0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 12 */  { scanend_opts,  0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 13 */  { tthresh_opts,  0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 14 */  { cooler_opts,   0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 15 */  { kpclean_opts,  0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 16 */  { da_prog_names, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
};

static EXEC_DATA CODE_Reg[] = {
 /* 0  */ { update_DA_mode,      0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 1  */ { update_scans,        0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 2  */ { update_mems,         0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 3  */ { update_ignores,      0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 4  */ { update_et,           0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 5  */ { update_preps,        0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 6  */ { update_pixtime,      0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 7  */ { update_dtype,        0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 8  */ { update_coolOnOff,    0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 9  */ { update_temp,         0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 10 */ { update_PulserType,   0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 11 */ { update_PulserDelay,  0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 12 */ { update_PulserWidth,  0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 13 */ { CAST_CHR2INT init_da_form,  0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 14 */ { update_shutter_mode, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 15 */ { update_control_mode, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 16 */ { update_waitopen,     0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 17 */ { update_waitclose,    0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 18 */ { update_src_comp,     0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 19 */ { update_Intensifier,  0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 20 */ { update_PulserAudio,  0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 21 */ { update_PulserDelayInc, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 22 */ { update_PulserDelayRange, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 23 */ { update_PulserTrigCount,  0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 24 */ { update_PulserTrigSrc,     0,DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 25 */ { update_PulserTrigThresh,  0,DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 26 */ { CAST_CHR2INT pulse_form_vect_select,  0,DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 27 */ { update_PulserTrigVolts,   0,DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 28 */ { update_keepclean,         0,DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 29 */ { CAST_CHR2INT exit_da_form,  0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 30 */ { update_startmem,         0,DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 31 */ { update_shifttime,    0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 32 */ { update_DA_name,      0, DATATYP_CODE, DATAATTR_PTR, 0 },
};  

static DATA DATA_Reg[] = {
   /* 0  */ { &da_prog,        0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 1  */ { &psize,          0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 2  */ { &scans,          0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 3  */ { &memories,       0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 4  */ { &ignores,        0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 5  */ { &preps,          0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 6  */ { &tempset,        0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 7  */ { &cooler_OnOff,   0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 8  */ { &detype_index,   0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 9  */ { &scomp_index,    0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 10 */ { &conmode_index,  0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 11 */ { &detector_index, 0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 12 */ { &max_memory,     0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 13 */ { &shutmode,       0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 14 */ { &waitopen,       0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 15 */ { &waitclose,      0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 16 */ { &exposure_time,  0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
   /* 17 */ { &pixtime_index,  0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 18 */ { &pulser_type,    0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 19 */ { &pulser_delay,   0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
   /* 20 */ { &pulser_width,   0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
   /* 21 */ { &pulser_inc,     0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
   /* 22 */ { &pulser_range,   0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
   /* 23 */ { &pulser_count,   0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 24 */ { &audio_index,    0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 25 */ { &tthresh_index,  0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 26 */ { &tsource_index,  0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 27 */ { &imode_index,    0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 28 */ { &cooler_index,   0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 29 */ { &tthresh_volts,  0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
   /* 30 */ { &keepclean_index, 0, DATATYP_INT, DATAATTR_PTR, 0 },
   /* 31 */ { &frame_time,     0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
   /* 32 */ { &pix_usecs,      0, DATATYP_INT, DATAATTR_PTR, 0 },
   /* 33 */ { &startmem,       0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 34 */ { &shifttime_index, 0, DATATYP_INT,   DATAATTR_PTR, 0 },
   /* 35 */ { &shift_usecs,    0, DATATYP_INT, DATAATTR_PTR, 0 },
   /* 36 */ { &streak_time,    0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
};

// define form field index values, MUST match EXACTLY with form fields

static enum {
      DMEMS_LABEL,
      MEM1_LABEL,
      DMAXJ_LABEL,
      DMODE_LABEL,
      DSCANS_LABEL,
      DIGNS_LABEL,
      DPREPS_LABEL,
      DEXTIME_LABEL,
      DETSEC_LABEL,

      DTEMP_LABEL,
      DDEG_LABEL,
      DPIXT_LABEL,
      PIXUSEC_LABEL,
      DSHFT_LABEL,
      SHFUSEC_LABEL,
      DSCMP_LABEL,
      DKPCLN_LABEL,

      PULSE_BOX,
      DINFO_BOX,
      FTIME_LABEL,
      FTIME_SECS,
      ST_TIME_LABEL,
      ST_TIME_SECS,

      DTYPE,
      DNUM_LABEL,
      D_INDEX,
      PTYPE,
      COOL_TYPE,
      FR_TIME,
      ST_TIME,
      MAX_MEMS,
      MEMS,
      START_MEM,
      DAMODE,
      DANAME,
      SCANS,
      IGNORES,
      PREPS,
      DA_ET,

      PULSE_DOWN_LOGIC,

      TSRC_LABEL,
      IMODE_LABEL,
      DPWID_LABEL,
      DWIDSEC_LABEL,
      DPDEL_LABEL,
      DDELSEC_LABEL,
      DINC_LABEL,
      DINCSEC_LABEL,
      DRANGE_LABEL,
      RANGSEC_LABEL,
      TCOUNT_LABEL,
      PERSCAN_LABEL,
      AUDIO_LABEL,
      TTHRESH_LABEL,
      
      TSOURCE,
      RS_IMODE,
      FPWIDTH,
      FPDELAY,
      DELINC,
      DELRANGE,
      TCOUNT,
      AUDIO,
      TTHRESH,
      TVOLTS,
      TVOLTS_LABEL,

      PULSE_UP_LOGIC,

      DDEGREES,
      COOLER_ONOFF,
      PIXTIME,
      PT_SEC,
      SHFTIME,
      ST_SEC,
      SCITC_SEL,
      KPCLEAN_SEL,

      DSHSYNC_LABEL,
      DWSO_LABEL,
      DWSC_LABEL,
      DSHMODE_LABEL,
   
      SCONTROL,
      WFSO,
      WFSC,
      SHUTMODE_64,
};

static FIELD DA_FormFields[] = {

   label_field(DMEMS_LABEL,         
    DGROUP_DO_STRINGS, 4,            /*  "Memories" */
    1, 7, 9),                                    

   label_field(MEM1_LABEL,
    DGROUP_DO_STRINGS, 31,           /*  "Start" */
    1, 24, 5),

   label_field(DMAXJ_LABEL,
    DGROUP_DO_STRINGS, 13,           /*  "Max" */
    1, 34, 3),

   label_field(DMODE_LABEL,
    DGROUP_DO_STRINGS, 0,            /*  "DA Mode" */
    1, 44, 11),                       
                           
   label_field(DSCANS_LABEL,
    DGROUP_DO_STRINGS, 3,            /*  "Data Scans" */
    2, 6, 10),

   label_field(DIGNS_LABEL,
    DGROUP_DO_STRINGS, 5,            /*  "Ignored Scans" */
    3, 3, 13),

   label_field(DPREPS_LABEL,
    DGROUP_DO_STRINGS, 18,           /*  "Prep Frames" */
    4, 4, 11),

   label_field(DEXTIME_LABEL,
    DGROUP_DO_STRINGS, 6,            /*  "Exposure Time" */
    5, 3, 13),

   label_field(DETSEC_LABEL,
    DGROUP_DO_STRINGS, 16,           /*  "sec." */
    5, 26, 4),

   label_field(DTEMP_LABEL,
    DGROUP_DO_STRINGS, 7,            /*  "Detector Temp" */
    2, 44, 13),

   label_field(DDEG_LABEL,
    DGROUP_DO_STRINGS, 8,            /*  "øC  Control" */
    2, 62, 12),

   label_field(DPIXT_LABEL,
    DGROUP_DO_STRINGS, 17,           /* "Pixel Time" */
    3, 44, 10),

   label_field(PIXUSEC_LABEL,
    DGROUP_DO_STRINGS, 39,           /* "æsec" */
    3, 70, 4),

   label_field(DSHFT_LABEL,
    DGROUP_DO_STRINGS, 40,           /* "Shift Time" */
    4, 44, 10),

   label_field(SHFUSEC_LABEL,
    DGROUP_DO_STRINGS, 39,           /* "æsec" */
    4, 70, 4),

    label_field(DSCMP_LABEL,
    DGROUP_DO_STRINGS, 10,           /*  "Source Comp Mode" */
    5, 44, 11),

   label_field(DKPCLN_LABEL,
    DGROUP_DO_STRINGS, 38,           /*  "Keep Clean" */
    6, 44, 10),

   d_field_(PULSE_BOX,
    FLDTYP_FORM,
    FLDATTR_DISPLAY_ONLY,
    0,                               
    DGROUP_FORMS, 0,                 /*  "Pulser Setup" */
    7, 63, 0),

   d_field_(DINFO_BOX,
    FLDTYP_FORM,
    FLDATTR_DISPLAY_ONLY,
    0,                               
    DGROUP_FORMS, 1,                 /*  "Equipment Summary" */
    8, 63, 0),                       

   label_field(FTIME_LABEL,
    DGROUP_DO_STRINGS, 35,           /*  "Frame Time" */
    14, 45, 10),

   label_field(FTIME_SECS,
    DGROUP_DO_STRINGS, 16,           /*  "sec" */
    14, 66, 4),

   d_field_(ST_TIME_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    KSI_NO_INDEX,
    DGROUP_DO_STRINGS, 41,           /*  "Streak Time" */
    15, 45, 11),

   d_field_(ST_TIME_SECS,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    KSI_NO_INDEX,
    DGROUP_DO_STRINGS, 16,           /*  "sec" */
    15, 66, 4),

   field_set(DTYPE,
   FLDTYP_TOGGLE,
   FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   DA_FORM_FIELD_HBASE + 1,
   DGROUP_1461, 8,                   /*  detector_type_index */
   DGROUP_TOGGLES, 1,                /*  det_type_options */
   DGROUP_CODE, 7,                   /*  update_dtype */
   0, 6,
   10, 45, 24,
   D_INDEX, D_INDEX, DTYPE, DTYPE,
   DTYPE, DTYPE, DTYPE, DTYPE),

   label_field(DNUM_LABEL,
    DGROUP_DO_STRINGS, 12,           /*  "Detector Number" */
    11, 45, 15),

   field_set(D_INDEX,
   FLDTYP_TOGGLE,
   FLDATTR_DISPLAY_ONLY,                                
   0,
   DA_FORM_FIELD_HBASE + 2,
   DGROUP_1461, 11,                  /*  detector_index */
   DGROUP_TOGGLES, 2,                /*  numbers */
   0, 0,
   0, 8,
   11, 63, 2,
   PTYPE, PTYPE, D_INDEX, D_INDEX,
   D_INDEX, D_INDEX, D_INDEX, D_INDEX),

  field_set(PTYPE,
   FLDTYP_TOGGLE,
   FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   DA_FORM_FIELD_HBASE + 13,
   DGROUP_1461, 18,                  /*  pulser_type */
   DGROUP_TOGGLES, 7,                /*  pulser_opts */
   DGROUP_CODE, 10,                  /*  set_pulser_type */
   0, 7,
   12, 45, 27,
   COOL_TYPE, COOL_TYPE, PTYPE, PTYPE,
   PTYPE, PTYPE, PTYPE, PTYPE),

  field_set(COOL_TYPE,
   FLDTYP_TOGGLE,
   FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_1461, 28,                  /*  cooler_index */
   DGROUP_TOGGLES, 14,               /*  cooler_opts */
   0, 0,
   0, 3,
   13, 45, 15,
   FR_TIME, FR_TIME, COOL_TYPE, COOL_TYPE,
   COOL_TYPE, COOL_TYPE, COOL_TYPE, COOL_TYPE),

   field_set(FR_TIME,
   FLDTYP_SCL_FLOAT,
   FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR | FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_1461, 31,                  /*  frame time */
   0, 0,
   0, 0,
   3, 0,
   14, 57, 9,
   ST_TIME, ST_TIME, FR_TIME, FR_TIME,
   FR_TIME, FR_TIME, FR_TIME, FR_TIME),

   field_set(ST_TIME,
   FLDTYP_SCL_FLOAT,
   FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR | FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
   KSI_NO_INDEX,
   0,
   DGROUP_1461, 36,                  /*  streak time */
   0, 0,
   0, 0,
   3, 0,
   15, 57, 9,
   MAX_MEMS, MAX_MEMS, ST_TIME, ST_TIME,
   ST_TIME, ST_TIME, ST_TIME, ST_TIME),

   field_set(MAX_MEMS,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_DISPLAY_ONLY | FLDATTR_NO_OVERFLOW_CHAR,               
   KSI_NO_INDEX,
   DA_FORM_FIELD_HBASE + 2,
   DGROUP_1461, 12,                  /*  max_memory */
   0, 0,
   0, 0,
   0, 0,
   1, 38, 5,
   MEMS, MEMS, 0, 0,
   0, 0, 0, 0),

   field_set(MEMS,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_MEMS,
   DA_FORM_FIELD_HBASE + 2,
   DGROUP_1461, 3,                   /*  memories */
   0, 0,
   DGROUP_CODE, 2,                   /*  set_mems */
   0, 0,
   1, 17, 5,
   EXIT, MEMS, PULSE_UP_LOGIC, SCANS,
   SCITC_SEL, START_MEM, PULSE_UP_LOGIC, START_MEM),

   field_set(START_MEM,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_START_MEM,
   DA_FORM_FIELD_HBASE + 24,
   DGROUP_1461, 33,                   /*  startmem */
   0, 0,
   DGROUP_CODE, 30,                   /*  update_startmem */
   0, 0,
   1, 30, 3,
   EXIT, START_MEM, PULSE_UP_LOGIC, SCANS,
   MEMS, DAMODE, MEMS, DAMODE),

   field_set(DAMODE,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID,
   KSI_DA_MODE,
   DA_FORM_FIELD_HBASE + 0,
   DGROUP_1461, 0,                  /*  DA_mode */
   0, 0,
   DGROUP_CODE, 0,                  /*  update_da_mode */
   0, 0,
   1, 58, 2,
   EXIT, DAMODE, START_MEM, DDEGREES,
   START_MEM, DANAME, START_MEM, DANAME),

   field_set(DANAME,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_DA_NAME,
   DA_FORM_FIELD_HBASE + 25,
   DGROUP_1461, 0,                   /*  DA_mode */
   DGROUP_TOGGLES, 16,               /*  da_prog_names */
   DGROUP_CODE, 32,                  /*  update_da_name */
   0, 5,
   1, 63, 9,
   EXIT, DANAME, START_MEM, DDEGREES,
   DAMODE, SCANS, DAMODE, SCANS),

   field_set(SCANS,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_SCANS,
   DA_FORM_FIELD_HBASE + 4,
   DGROUP_1461, 2,                   /*  scans */
   0, 0,
   DGROUP_CODE, 1,                   /*  set_scans */
   0, 0,
   2, 17, 5,
   EXIT, SCANS, MEMS, IGNORES,
   SCANS, SCANS, DANAME, DDEGREES),

   field_set(IGNORES,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_IGN_SCANS,
   DA_FORM_FIELD_HBASE + 5,
   DGROUP_1461, 4,                   /*  ignored_scans */
   0, 0,
   DGROUP_CODE, 3,                   /*  set_ignores */
   0, 0,
   3, 17, 5,
   EXIT, IGNORES, SCANS, PREPS,
   IGNORES, IGNORES, COOLER_ONOFF, PIXTIME),

   field_set(PREPS,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_PREP,
   DETECTOR_1464_HBASE + 0,
   DGROUP_1461, 5,                   /*  prep_frames */
   0, 0,
   DGROUP_CODE, 5,                   /*  set_preps */
   0, 0,
   4, 17, 5,
   EXIT, PREPS, IGNORES, DA_ET,
   PREPS, PREPS, PIXTIME, SCITC_SEL),

   field_set(DA_ET,
   FLDTYP_SCL_FLOAT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_ET,
   DA_FORM_FIELD_HBASE + 7,
   DGROUP_1461, 16,                  /*  exposure_time */
   0, 0,
   DGROUP_CODE, 4,                   /*  set_et */
   3, 0,
   5, 17, 9,
   EXIT, DA_ET, PREPS, PULSE_DOWN_LOGIC,
   PREPS, DDEGREES, SCITC_SEL, KPCLEAN_SEL /*PULSE_DOWN_LOGIC*/),

   field_set(PULSE_DOWN_LOGIC,
   FLDTYP_LOGIC,                     /* selects TRIGSRC or SCONTROL */
   FLDATTR_DRAW_PERMITTED,           /* based on pulser type */
   KSI_NO_INDEX,
   0,
   DGROUP_CODE, 26,                  /*  pulse_form_vect_select */
   0, 0,
   0, 0,
   0, 0,
   4, 33, 1,
   /*  expect either 0 or 1 or 2 from pulse_form_vect_select() */
   SCONTROL, TSOURCE, MEMS, PULSE_DOWN_LOGIC,
   PULSE_DOWN_LOGIC, PULSE_DOWN_LOGIC, PULSE_DOWN_LOGIC, MEMS),

   d_field_(TSRC_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 23,           /*  "Trigger Source" */
    8, 3, 14),

   d_field_(IMODE_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 24,           /*  "Intensifier Mode" */
    9, 3, 16),

   d_field_(DPWID_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 15,           /*  "Pulser Width" */
    11, 3, 12),

   d_field_(DWIDSEC_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 16,           /*  "sec." */
    11, 26, 4),

   d_field_(DPDEL_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 25,           /*  "Start Delay" */
    12, 3, 12),

   d_field_(DDELSEC_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 16,           /*  "sec." */
    12, 26, 4),

   d_field_(DINC_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 27,           /*  "Delay Inc/Scan" */
    13, 3, 14),

   d_field_(DINCSEC_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 16,           /*  "sec." */
    13, 26, 4),

   d_field_(DRANGE_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 28,            /*  "Delay Range" */
    14, 3, 11),

   d_field_(RANGSEC_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 16,
    14, 26, 4),

   d_field_(TCOUNT_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 29,           /*  "Trigger Count" */
    16, 3, 13),

   d_field_(PERSCAN_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 30,           /*  "per scan" */
    16, 27, 8),

   d_field_(AUDIO_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 33,           /*  "Audio" */
    17, 3, 5),

   d_field_(TTHRESH_LABEL,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
    0,                            
    DGROUP_DO_STRINGS, 34,           /*  "Trigger Level" */
    18, 3, 13),

  field_set(TSOURCE,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID | FLDATTR_GET_DRAW_PERMISSION,
   KSI_TSOURCE,
   DA_FORM_FIELD_HBASE + 9,
   DGROUP_1461, 26,                  /*  tsource_index, */
   DGROUP_TOGGLES, 10,               /*  tsource_opts, */
   DGROUP_CODE, 24,
   0, 5,
   8, 19, 8,
   EXIT, TSOURCE, DA_ET, RS_IMODE,
   TSOURCE, TSOURCE, KPCLEAN_SEL, RS_IMODE),

  field_set(RS_IMODE,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID | FLDATTR_GET_DRAW_PERMISSION,
   KSI_RS_IMODE,
   DA_FORM_FIELD_HBASE + 10,
   DGROUP_1461, 27,                  /*  imode_index, */
   DGROUP_TOGGLES, 11,               /*  intens_opts, */
   DGROUP_CODE, 19,
   0, 2,
   9, 20, 5,
   EXIT, RS_IMODE, TSOURCE, FPWIDTH,
   RS_IMODE, RS_IMODE, TSOURCE, FPWIDTH),

  field_set(FPWIDTH,
   FLDTYP_SCL_FLOAT,
   FLDATTR_REV_VID  | FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR | FLDATTR_GET_DRAW_PERMISSION,
   KSI_FPWIDTH,
   DA_FORM_FIELD_HBASE + 17,
   DGROUP_1461, 20,                  /*  pulser_width */
   0, 0,                             
   DGROUP_CODE, 12,                  /*  set_pulser_width */
   3, 3,
   11, 19, 7,
   EXIT, FPWIDTH, RS_IMODE, FPDELAY,
   RS_IMODE, FPDELAY, RS_IMODE, FPDELAY),

   field_set(FPDELAY,
   FLDTYP_SCL_FLOAT,
   FLDATTR_REV_VID | FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR | FLDATTR_GET_DRAW_PERMISSION,
   KSI_FPDELAY,
   DA_FORM_FIELD_HBASE + 16,
   DGROUP_1461, 19,                  /*  pulser_delay */
   0, 0,                             
   DGROUP_CODE, 11,                  /*  set_pulser_delay */
   3, 3,
   12, 19, 7,
   EXIT, FPDELAY, FPWIDTH, DELINC,
   FPWIDTH, DELINC, FPWIDTH, DELINC),

  field_set(DELINC,
   FLDTYP_SCL_FLOAT,
   FLDATTR_REV_VID  | FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR | FLDATTR_GET_DRAW_PERMISSION,
   KSI_DELINC,
   DA_FORM_FIELD_HBASE + 11,
   DGROUP_1461, 21,                  /*  pulser_inc */
   0, 0,                             
   DGROUP_CODE, 21,
   3, 2,
   13, 19, 7,
   EXIT, DELINC, FPDELAY, DELRANGE,
   FPDELAY, DELRANGE, FPDELAY, DELRANGE),

  field_set(DELRANGE,
   FLDTYP_SCL_FLOAT,
   FLDATTR_REV_VID  | FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR | FLDATTR_GET_DRAW_PERMISSION,
   KSI_DELRANGE,
   DA_FORM_FIELD_HBASE + 12,
   DGROUP_1461, 22,                  /*  pulser_range */
   0, 0,                             
   DGROUP_CODE, 22,
   3, 2,
   14, 19, 7,
   EXIT, DELRANGE, DELINC, TCOUNT,
   DELINC, TCOUNT, DELINC, TCOUNT),

   field_set(TCOUNT,
   FLDTYP_UNS_INT,
   FLDATTR_REV_VID | FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR | FLDATTR_GET_DRAW_PERMISSION,
   KSI_TCOUNT,
   DA_FORM_FIELD_HBASE + 20,
   DGROUP_1461, 23,                  /*  pulser_count */
   0, 0,                             
   DGROUP_CODE, 23,
   0, 0,
   16, 19, 7,
   EXIT, TCOUNT, DELRANGE, AUDIO,
   DELRANGE, AUDIO, DELRANGE, AUDIO),

  field_set(AUDIO,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID | FLDATTR_GET_DRAW_PERMISSION,
   KSI_AUDIO,
   DA_FORM_FIELD_HBASE + 21,
   DGROUP_1461, 24,                  /*  audio_index */
   DGROUP_TOGGLES, 3,                /*  on_options */
   DGROUP_CODE, 20,
   0, 2,
   17, 19, 3,
   EXIT, AUDIO, TCOUNT, TTHRESH,
   DELRANGE, TTHRESH, TCOUNT, TTHRESH),

  field_set(TTHRESH,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID | FLDATTR_GET_DRAW_PERMISSION,
   KSI_TTHRESH,
   DA_FORM_FIELD_HBASE + 22,
   DGROUP_1461, 25,                  /*  tthresh_index */
   DGROUP_TOGGLES, 13,               /*  tthresh_opts */
   DGROUP_CODE, 25,
   0, 3,
   18, 19, 5,
   EXIT, TTHRESH, AUDIO, TVOLTS,   /* MEMS, */
   TTHRESH, TTHRESH, AUDIO, TVOLTS /* MEMS */),

  field_set(TVOLTS,
   FLDTYP_SCL_FLOAT,
   FLDATTR_REV_VID  | FLDATTR_RJ | FLDATTR_NO_OVERFLOW_CHAR | FLDATTR_GET_DRAW_PERMISSION,
   KSI_TVOLTS,
   DA_FORM_FIELD_HBASE + 23,
   DGROUP_1461, 29,                  /*  tthresh_volts */
   0, 0,                             
   DGROUP_CODE, 27,
   1, 1,
   18, 26, 8,
   EXIT, TVOLTS, AUDIO, MEMS,
   TTHRESH, TVOLTS, TTHRESH, MEMS),

  d_field_(TVOLTS_LABEL,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
   0,                            
   DGROUP_DO_STRINGS, 37,            /*  "V" */
   18, 34, 1),

  field_set(PULSE_UP_LOGIC,
   FLDTYP_LOGIC,                     /* selects TTHRESH or SHUTMODE */
   FLDATTR_DRAW_PERMITTED,           /* based on pulser type */
   KSI_NO_INDEX,
   0,
   DGROUP_CODE, 26,                  /*  pulse_form_vect_select */
   0, 0,
   0, 0,
   0, 0,
   4, 33, 1,
   /*  expect either 0 or 1 or 2 from pulse_form_vect_select() */
   SHUTMODE_64, TVOLTS, DA_ET, PULSE_UP_LOGIC,
   PULSE_UP_LOGIC, PULSE_UP_LOGIC, PULSE_UP_LOGIC, DA_ET),

  field_set(DDEGREES,
   FLDTYP_INT,
   FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_DET_TEMP,
   DA_FORM_FIELD_HBASE + 3,
   DGROUP_1461, 6,                   /*  tempset */
   0, 0,                             
   DGROUP_CODE, 9,                   /*  set_tempset */
   0, 0,
   2, 58, 4,
   EXIT, DDEGREES, DAMODE, PIXTIME,
   SCANS, COOLER_ONOFF, SCANS, COOLER_ONOFF),

  field_set(COOLER_ONOFF,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_COOL_ONOFF,
   DA_FORM_FIELD_HBASE + 19,
   DGROUP_1461, 7,                   /*  cooler_OnOff */
   DGROUP_TOGGLES, 3,                /*  on_options */
   DGROUP_CODE, 8,                   /*  newCoolerOnOff */
   0, 2,
   2, 74, 3,
   EXIT, COOLER_ONOFF, DAMODE, PIXTIME,
   DDEGREES, PIXTIME, DDEGREES, IGNORES),
   
  field_set(PIXTIME,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_PIX_TIME_OPTS,
   DETECTOR_NONE_HBASE + 0,    
   DGROUP_1461, 17,                  /*  pix_time_index */
   DGROUP_TOGGLES, 5,                /*  pixtime_opts */
   DGROUP_CODE, 6,                   /*  update_pixtime */
   0, 3,
   3, 58, 6,
   EXIT, PIXTIME, DDEGREES, PT_SEC,
   PIXTIME, PIXTIME, IGNORES, PREPS),

  field_set(PT_SEC,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_DISPLAY_ONLY | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_NO_INDEX,
   0,
   DGROUP_1461, 32,
   0, 0,                             
   0, 0,
   0, 0,
   3, 67, 2,
   SHFTIME, PT_SEC, PT_SEC, PT_SEC,
   PT_SEC, PT_SEC, PT_SEC, PT_SEC),
   
  field_set(SHFTIME,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_SFT_TIME_OPTS,
   DETECTOR_NONE_HBASE + 2,    
   DGROUP_1461, 34,                  /*  shift_time_index */
   DGROUP_TOGGLES, 5,                /*  pixtime_opts */
   DGROUP_CODE, 31,                  /*  update_shifttime */
   0, 3,
   4, 58, 6,
   EXIT, SHFTIME, PIXTIME, ST_SEC,
   SHFTIME, SHFTIME, PIXTIME, PREPS),

  field_set(ST_SEC,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_DISPLAY_ONLY | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_NO_INDEX,
   0,
   DGROUP_1461, 35,
   0, 0,                             
   0, 0,
   0, 0,
   4, 67, 2,
   SCITC_SEL, ST_SEC, ST_SEC, ST_SEC,
   ST_SEC, ST_SEC, ST_SEC, ST_SEC),

  field_set(SCITC_SEL,
   FLDTYP_TOGGLE, FLDATTR_REV_VID,
   KSI_64_SCOMP,
   DETECTOR_1464_HBASE + 6,
   DGROUP_1461, 9,                   /*  source_comp_index */
   DGROUP_TOGGLES, 6,                /*  scomp_opts */
   DGROUP_CODE, 18,                  /*  set_source_comp_mode */
   0, 7,
   5, 58, 6,
   EXIT, SCITC_SEL, SHFTIME, KPCLEAN_SEL,
   PREPS, DA_ET, PREPS, DA_ET),

  field_set(KPCLEAN_SEL,
   FLDTYP_TOGGLE, FLDATTR_REV_VID,
   KSI_KPCLEAN_SEL,
   DA_FORM_FIELD_HBASE + 8,
   DGROUP_1461, 30,                /*  keepclean_index */
   DGROUP_TOGGLES, 15,             /*  kpclean_opts */
   DGROUP_CODE, 28,                /*  update_keepclean */
   0, 2,
   6, 58, 6,
   EXIT, KPCLEAN_SEL, SCITC_SEL, PULSE_DOWN_LOGIC,
   DA_ET, PULSE_DOWN_LOGIC, DA_ET, PULSE_DOWN_LOGIC),

  d_field_(DSHSYNC_LABEL,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
   0,                            
   DGROUP_DO_STRINGS, 11,            /*  "Shutter Sync" */
  10, 18, 15),

  d_field_(DWSO_LABEL,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
   0,                            
   DGROUP_DO_STRINGS, 19,            /*  "for Sync Before" */
   11, 18, 15),

  d_field_(DWSC_LABEL,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
   0,                            
   DGROUP_DO_STRINGS, 20,            /*  "for Sync After" */
   12, 18, 14),

  d_field_(DSHMODE_LABEL,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY | FLDATTR_GET_DRAW_PERMISSION,
   0,                            
   DGROUP_DO_STRINGS, 14,            /*  "Shutter mode" */
   13, 18, 12),                     

  field_set(SCONTROL,
   FLDTYP_TOGGLE,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_GET_DRAW_PERMISSION,
   KSI_64_SYNC_OPTS,
   DETECTOR_1464_HBASE + 1,
   DGROUP_1461, 10,                  /*  sync_index */
   DGROUP_TOGGLES, 4,                /*  sync_options */
   DGROUP_CODE, 15,                  /*  set_sync_mode */
   0, 2,
   10, 8, 8,
   EXIT, SCONTROL, DA_ET, WFSO,
   SCONTROL, SCONTROL, KPCLEAN_SEL, WFSO),

   field_set(WFSO,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID | FLDATTR_GET_DRAW_PERMISSION,
   KSI_OPEN_SYNC,
   DETECTOR_1464_HBASE + 2,
   DGROUP_1461, 14,                  /*  shutter_open_sync_index */
   DGROUP_TOGGLES, 8,                /*  syncwait_opts */
   DGROUP_CODE, 16,                  /*  set_shutter_open_sync */
   0, 2,
   11, 6, 10,
   EXIT, WFSO, SCONTROL, WFSC,
   WFSO, WFSO, SCONTROL, WFSC),

   field_set(WFSC,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID | FLDATTR_GET_DRAW_PERMISSION,
   KSI_CLOSE_OPTS,
   DETECTOR_1464_HBASE + 3,
   DGROUP_1461, 15,                  /*  shutter_close_sync_index */
   DGROUP_TOGGLES, 8,                /*  syncwait_opts */
   DGROUP_CODE, 17,                  /*  set_shutter_close_sync */
   0, 2,
   12, 6, 10,
   EXIT, WFSC, WFSO, SHUTMODE_64,
   WFSC, WFSC, WFSO, SHUTMODE_64),

   field_set(SHUTMODE_64,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID | FLDATTR_RJ | FLDATTR_GET_DRAW_PERMISSION,
   KSI_SHUTTER_OPTS,
   DETECTOR_1464_HBASE + 5,
   DGROUP_1461, 13,                  /*  shutter_forced_mode */
   DGROUP_TOGGLES, 9,                /*  shutmode_opts */
   DGROUP_CODE, 14,                  /*  set_shutter_mode */
   0, 3,
   13, 3, 13,
   EXIT, SHUTMODE_64, WFSC, MEMS,
   SHUTMODE_64, SHUTMODE_64, WFSC, MEMS),
};

static FORM  DA_Form = {
   0, 0, FORMATTR_BORDER | FORMATTR_EXIT_RESTORE | FORMATTR_VISIBLE | FORMATTR_FULLSCREEN,
   0, 0, 0,
   2, 0, 21, 80,
   0, 0,
   { DGROUP_CODE, 13 },                        /*  init_da_form */
   { DGROUP_CODE, 29 },                        /*  exit_da_form */
   COLORS_DEFAULT,
   0, 0, 0, 0, sizeof(DA_FormFields) / sizeof(FIELD),
   DA_FormFields, KSI_DA_FORM,
   0, DO_String_Reg, FORMS_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, DATA_Reg
};

static FIELD PulserBox_Fields[] = {
   label_field(0,
   DGROUP_DO_STRINGS, 22,                 /*  "Pulser Setup" */
   0, 7, 7 )
};

static FORM  PulserBox_Form =
  {
  0,                                      /*  int   field_index; */
  0,                                      /*  int   previous_field_index; */
  FORMATTR_BORDER | FORMATTR_VISIBLE |    /*  uint  attrib; */
  FORMATTR_DEPENDENT_FORM,
  0,                                      /*  char  next_field_offset; */
  0,                                      /*  uchar exit_key_code; */
  0,                                      /*  uchar status; */
  PBOX_ROW,                               /*  uchar row; */
  PBOX_COL,                               /*  uchar column; */
  13,                                     /*  uchar size_in_rows; */
  34,                                     /*  uchar size_in_columns; */
  0,                                      /*  uchar display_row_offset;  for scrolling forms */
  0,                                      /*  uint  virtual_row_index;   for scrolling forms */
  { 0, 0 },                               /*  init_function; */
  { 0, 0 },                               /*  exit_function; */
  COLORS_DEFAULT,                         /*  uchar color_set_index; */
  0,                                      /*  uchar string_cursor_offset; */
  0,                                      /*  uchar display_cursor_offset; */
  0,                                      /*  uchar field_char_count; */
  0,                                      /*  uchar field_overfull_flag; */
  0,                                      /*  number_of_fields; */
  PulserBox_Fields,                       /*  FIELD *   fields; */
  KSI_NO_INDEX,                           /*  int   MacFormIndex;for keystroke record and playback */
  0, DO_String_Reg, 0, 0, 0, 0            /*  DATA * dataRegistry[ 6 ];  0 means no entry */
  };

static FIELD    InfoBox_Fields[] = {
   label_field(0,
   DGROUP_DO_STRINGS, 36,                 /*  "Equipment Summary" */
   0, 9, 17)
};   

static FORM  InfoBox_Form =
  {
  0,                                      /* int   field_index; */
  0,                                      /* int   previous_field_index; */
  FORMATTR_BORDER | FORMATTR_VISIBLE,     /* uint  attrib; */
  0,                                      /* char  next_field_offset; */
  0,                                      /* uchar exit_key_code; */
  0,                                      /* uchar status; */
   8,                                     /* uchar row; */
  40,                                     /* uchar column; */
  10,                                     /* uchar size_in_rows; */
  34,                                     /* uchar size_in_columns; */
  0,                                      /* uchar display_row_offset; (scrollforms) */
  0,                                      /* uint  virtual_row_index;  (scrollforms) */
  { 0, 0 },                               /* init_function; */
  { 0, 0 },                               /* exit_function; */
  COLORS_DEFAULT,                         /* uchar     color_set_index; */
  0,                                      /* uchar     string_cursor_offset; */
  0,                                      /* uchar     display_cursor_offset; */
  0,                                      /* uchar     field_char_count; */
  0,                                      /* uchar     field_overfull_flag; */
  sizeof(InfoBox_Fields)/sizeof(FIELD),   /* int       number_of_fields; */
  InfoBox_Fields,                         /* FIELD *   fields; */
  KSI_NO_INDEX,                           /* int       MacFormIndex;  // for keystroke record and playback */
  0, DO_String_Reg, 0, 0, 0, 0            /* DATA *    dataRegistry[ 6 ];  0 means no entry */
  };

void set_for_pulser(void)
{
  int i;

  for (i = DSHSYNC_LABEL; i <= DSHMODE_LABEL; i++)
    DA_Form.fields[i].attrib &= (~FLDATTR_DRAW_PERMITTED);

  for (i = SCONTROL; i <= SHUTMODE_64; i++)
    DA_Form.fields[i].attrib &= (~FLDATTR_DRAW_PERMITTED);

  for (i = TSRC_LABEL; i <= TTHRESH_LABEL; i++)
    DA_Form.fields[i].attrib |= (FLDATTR_DRAW_PERMITTED);

  for (i = TSOURCE; i <= TVOLTS_LABEL; i++)
    DA_Form.fields[i].attrib |= (FLDATTR_DRAW_PERMITTED);

  /* the trigger threshold is actually stored in terms of the     */
  /* LCI threshold register setting, which is from 0 to 255,      */
  /* with 0 volts being around register value 128. Function       */
  /* get_TrigThreshVolts gets the form's value for the Voltage    */
  /* readout, and get_PulserTrigThresh gets the register value.   */
  /* Certain register values correspond to predefined settings,   */
  /* such as "TTL" (defined by John Kalinowski to be 0.631V!)     */
  /* If the register value is not the same as one of the pre-     */
  /* defined settings, then the setting type displayed is "Volts" */

  get_TrigThreshVolts(&tthresh_volts);
  get_PulserTrigThresh(&tthresh_index);
  switch (tthresh_index)
    {
    case 139:
    case 116:
      tthresh_index = 0;  /* TTL equivalent */
    break;
    case 122:
      tthresh_index = 1;  /* Fast NIM equivalent */
    break;
    case 148:
    case 107:
      tthresh_index = 2;  /* Slow NIM equivalent */
    break;
    default:
      tthresh_index = 3;  /* Specified Volts */
    }
  PulserBox_Form.row = DA_Form.row + (UCHAR)PBOX_ROW;
  PulserBox_Form.column = DA_Form.column + (UCHAR)PBOX_COL;
  PulserBox_Form.size_in_rows = 13;
  PulserBox_Form.attrib |= FORMATTR_VISIBLE;
}

void set_for_shutter(void)
{
  int i;

  for (i = DSHSYNC_LABEL; i <= DSHMODE_LABEL; i++)
    DA_Form.fields[i].attrib |= (FLDATTR_DRAW_PERMITTED);

  for (i = SCONTROL; i <= SHUTMODE_64; i++)
    DA_Form.fields[i].attrib |= (FLDATTR_DRAW_PERMITTED);

  for (i = TSRC_LABEL; i <= TTHRESH_LABEL; i++)
    DA_Form.fields[i].attrib &= (~FLDATTR_DRAW_PERMITTED);

  for (i = TSOURCE; i <= TVOLTS_LABEL; i++)
    DA_Form.fields[i].attrib &= (~FLDATTR_DRAW_PERMITTED);

  PulserBox_Form.row = DA_Form.row + (UCHAR)SBOX_ROW;
  PulserBox_Form.column = DA_Form.column + (UCHAR)PBOX_COL;
  PulserBox_Form.size_in_rows = 6;
  PulserBox_Form.attrib |= FORMATTR_VISIBLE;
}

void set_for_rapda(void)
{
  int i;

  for (i = DSHSYNC_LABEL; i <= DSHMODE_LABEL; i++)
    DA_Form.fields[i].attrib &= (~FLDATTR_DRAW_PERMITTED);

  for (i = SCONTROL; i <= SHUTMODE_64; i++)
    DA_Form.fields[i].attrib &= (~FLDATTR_DRAW_PERMITTED);

  for (i = TSRC_LABEL; i <= TTHRESH_LABEL; i++)
    DA_Form.fields[i].attrib &= (~FLDATTR_DRAW_PERMITTED);

  for (i = TSOURCE; i <= TVOLTS_LABEL; i++)
    DA_Form.fields[i].attrib &= (~FLDATTR_DRAW_PERMITTED);

  PulserBox_Form.attrib &= (~FORMATTR_VISIBLE);
}


static void setup_da_names(void)
{
  int i;
 
  /* limit toggle field in case nothing loaded */
  DA_FormFields[DANAME].specific.tglfld.total_items = 1;
  get_DaMaxProg(&max_da_prog);
  {
  if (max_da_prog > 1) /* is this check necessary? */
    {
    for (i = 1; i < max_da_prog+1; i++) /* Don't use 0'th element */
      {
      GetOMAString(DS_DANAME, i, da_prog_names[i]);
//      get_DaName(da_prog_names[i], i);  /* Since can't set DA mode 0 */
      }
    DA_FormFields[DANAME].specific.tglfld.total_items = (UCHAR)(max_da_prog+1);
    }
  }
}

PRIVATE BOOLEAN init_da_form(int dummy)
{
  int temp;

  get_CoolOnOff(      &cooler_OnOff);
  get_CoolerType(     &cooler_index);
  get_ControlMode(    &conmode_index);
  get_DaProg(         &da_prog);
  get_DetectorType(   &detype_index);
  get_PulserType(     &pulser_type);
  get_ExposeTime(     &exposure_time);
  get_Ignores(        &ignores);
  get_IntensifierMode(&imode_index);
  get_MaxMem(         &max_memory);
  get_Mems(           &memories);
  get_Mem(            &startmem);
  get_PixTime(        &pixtime_index);
  get_ShiftTime(      &shifttime_index);
  get_PointSize(      &psize);
  get_Preps(          &preps);
  if ((pulser_type > 1) && detype_index) /* not shutter board, may be pulser */
    {
    get_PulserAudio(    &audio_index);
    get_PulserDelay(    &pulser_delay);
    get_PulserDelayInc( &pulser_inc);
    get_PulserDelayRange(&pulser_range);
    get_PulserTrigCount(&pulser_count);
    get_PulserTrigSrc   (&tsource_index);
    get_PulserWidth(    &pulser_width);
    set_for_pulser();
    }
  else if ((pulser_type == 1) && detype_index && detype_index != RAPDA)
    {
    get_ShutterOpenSync(&waitopen);
    get_ShutterCloseSync(&waitclose);
    get_ShutterMode(    &shutmode);
    set_for_shutter();
    }
  else
    set_for_rapda();    /* no shutter or pulser */

  get_Scans(          &scans);
  get_SourceCompMode( &scomp_index);
  get_Temp(           &tempset);
  get_SameET(&keepclean_index);
  frame_time = calc_rpix_time();
  pix_usecs = (unsigned)(frame_time * (float)1.001e6);
  frame_time = shift_line_time();
  shift_usecs = (unsigned)(frame_time * (float)1.001e6);
  get_FrameTime(&frame_time);
  get_StreakTime(&streak_time);
  get_StreakMode(&temp);
  if (temp)
    {
    DA_Form.fields[ST_TIME_LABEL].attrib |= (FLDATTR_DRAW_PERMITTED);
    DA_Form.fields[ST_TIME_SECS].attrib  |= (FLDATTR_DRAW_PERMITTED);
    DA_Form.fields[ST_TIME].attrib       |= (FLDATTR_DRAW_PERMITTED);
    }
  else
    {
    DA_Form.fields[ST_TIME_LABEL].attrib &= (~FLDATTR_DRAW_PERMITTED);
    DA_Form.fields[ST_TIME_SECS].attrib  &= (~FLDATTR_DRAW_PERMITTED);
    DA_Form.fields[ST_TIME].attrib       &= (~FLDATTR_DRAW_PERMITTED);
    }

  if (detype_index)
    setup_da_names();

  if(! keyIdle_pulser.current_handler)
    {
    keyIdle_pulser.current_handler = update_pulser_status;
    keyIdle_pulser.prev_handler = keyboard_idle;
    keyboard_idle = & keyIdle_pulser;
    update_enabled = TRUE;
    }
  return FALSE;
}
 
PRIVATE BOOLEAN exit_da_form(int dummy)
{
  if(keyIdle_pulser.current_handler)
    {
    keyboard_idle = keyIdle_pulser.prev_handler;
    keyIdle_pulser.prev_handler = NULL;
    keyIdle_pulser.current_handler = NULL;
    }
  update_enabled = FALSE;
  return FALSE;
}
 
PRIVATE unsigned char pulse_form_vect_select(FORM_CONTEXT * dummy)
{
  if (!pulser_type | !detype_index)
    return 7;                  /* avoid pulser/shutter form if no detector */
  if (detype_index != RAPDA)
    return (unsigned char) (pulser_type > 1);
  else
    return(pulser_type > 1 ? (unsigned char)1 : (unsigned char)2);
}

PRIVATE int update_DA_mode(void)
{
  int err = set_DaProg(da_prog);
  display_random_field(&DA_Form, DANAME);
  return errorCheckDetDriver(err);
}

PRIVATE int update_DA_name(void)
{
  int err = set_DaProg(da_prog);
  display_random_field(&DA_Form, DAMODE);
  return errorCheckDetDriver(err);
}

/*  function to set the source comp integration time in response to the  */
/*  user toggling the source comp field */

PRIVATE int update_src_comp(void)
{
   return errorCheckDetDriver(set_SourceCompMode(scomp_index));
}

PRIVATE int update_keepclean(void)
{
  return errorCheckDetDriver(set_SameET(keepclean_index));
}

/*  stub, change detector type -- CCD, PDA */
PRIVATE int update_dtype(void)
{
   return FIELD_VALIDATE_SUCCESS;
}

PRIVATE int update_da_prog(void)
{
   return errorCheckDetDriver(set_DaProg(da_prog));
}

PRIVATE int update_scans(void)
{
   return errorCheckDetDriver(set_Scans(scans));
}

PRIVATE int update_mems(void)
{
  int err;
  if (! (err = set_Mems(memories)))
    pulser_range = memories * pulser_inc;
  if (pulser_type > 1 && detype_index)
    {
    set_for_pulser();
    display_random_field(&DA_Form, DELRANGE);
    }
  else
    set_for_shutter();

  return errorCheckDetDriver(err);
}

PRIVATE int update_startmem(void)
{
  int err;
  if (! (err = set_Mem(startmem)))
  clearAllCurveBufs();
  return errorCheckDetDriver(err);
}

PRIVATE int update_ignores(void)
{
  return errorCheckDetDriver(set_Ignores(ignores));
}

PRIVATE void refresh_frame_time(void)
{
  USHORT temp;
  
  get_FrameTime(&frame_time);
  display_random_field(&DA_Form, FR_TIME);
  get_StreakTime(&streak_time);
  get_StreakMode(&temp);
  if (temp)
    display_random_field(&DA_Form, ST_TIME);
}

PRIVATE int update_et(void)
{
  int err = set_ExposeTime(exposure_time);
  get_ExposeTime(&exposure_time);
  if (pulser_type > 1 && detype_index)
    {
    get_PulserWidth(&pulser_width);
    get_PulserTrigCount(&pulser_count);
    set_for_pulser();
    display_random_field(&DA_Form, FPWIDTH);
    display_random_field(&DA_Form, TCOUNT);
    }
  else
    set_for_shutter();

  refresh_frame_time();

  return(errorCheckDetDriver(err));
}

PRIVATE int update_preps(void)
{
  return errorCheckDetDriver(set_Preps(preps));
}

PRIVATE int update_pixtime(void)
{
  int err = set_PixTime(pixtime_index);
  float ptime = calc_rpix_time();

  pix_usecs = (unsigned)(ptime * (float)1.001e6);
  display_random_field(&DA_Form, PT_SEC);
  refresh_frame_time();
  return errorCheckDetDriver(err);
}

PRIVATE int update_shifttime(void)
{
  int err = set_ShiftTime(shifttime_index);
  float shifttime = shift_line_time();

  shift_usecs = (unsigned)(shifttime * (float)1.001e6);
  display_random_field(&DA_Form, ST_SEC);
  get_ExposeTime(&exposure_time);
  display_random_field(&DA_Form, DA_ET);
  refresh_frame_time();
  return errorCheckDetDriver(err);
}

PRIVATE int update_coolOnOff(void)
{
  return errorCheckDetDriver(set_CoolOnOff(cooler_OnOff));
}

PRIVATE int update_temp(void)
{
  int err = errorCheckDetDriver(set_Temp(tempset));

  get_Temp(&tempset);
  return err;
}

PRIVATE int update_control_mode(void)
{
  return errorCheckDetDriver(set_ControlMode(conmode_index));
}

PRIVATE int update_shutter_mode(void)
{
  int err = set_ShutterMode(shutmode);
  refresh_frame_time();
  return errorCheckDetDriver(err);
}

PRIVATE int update_waitopen(void)
{
  return errorCheckDetDriver(set_ShutterOpenSync(waitopen));
}

PRIVATE int update_waitclose(void)
{
  return errorCheckDetDriver(set_ShutterCloseSync(waitclose));
}

/************************** LCI pulser stuff ****************************/

/* set the LCI mode to "GATED" or "CW" */

PRIVATE int update_Intensifier()
{
  struct timeb time_str;
  unsigned long millisecs, killisecs;
  int err = set_IntensifierMode(imode_index);

    ftime(&time_str);
    millisecs = time_str.millitm + time_str.time * 1000;

    /* wait 100 ms for the LCI to reply.  if the LCI switch is */
    /* set to CW, then even if the field is changed to GATE,   */
    /* the displayed mode will be CW */
    do
      {
      ftime(&time_str);
      killisecs = (time_str.millitm + time_str.time * 1000) - millisecs;
      }
    while (killisecs < 100);

  set_for_pulser();

  get_IntensifierMode(&imode_index);
  display_random_field(&DA_Form, RS_IMODE);
  return(errorCheckDetDriver(err));
}


PRIVATE int update_PulserAudio()
{
  return(errorCheckDetDriver(set_PulserAudio(audio_index)));
}

PRIVATE int update_PulserDelay()
{
  return(errorCheckDetDriver(set_PulserDelay(pulser_delay)));
}

PRIVATE int update_PulserDelayInc()
{
  int err = set_PulserDelayInc(pulser_inc);
  pulser_range = memories * pulser_inc;
  set_for_pulser();
  display_random_field(&DA_Form, DELRANGE);
  return(errorCheckDetDriver(err));
}

PRIVATE int update_PulserDelayRange()
{
  int err = set_PulserDelayRange(pulser_range);
  get_Mems(&memories);
  set_for_pulser();
  display_random_field(&DA_Form, MEMS);
  return(errorCheckDetDriver(err));
}

PRIVATE int update_PulserTrigCount()
{
  int err = set_PulserTrigCount(pulser_count);
  get_ExposeTime(&exposure_time);
  set_for_pulser();
  display_random_field(&DA_Form, DA_ET);
  return(errorCheckDetDriver(err));
}

PRIVATE int update_PulserTrigSrc()
{
  int err = set_PulserTrigSrc(tsource_index);

  if (tsource_index == POS_SLOPE || tsource_index == NEG_SLOPE)
    {
    set_for_pulser();
    display_random_field(&DA_Form, TTHRESH);
    display_random_field(&DA_Form, TVOLTS);
    }
  return(errorCheckDetDriver(err));
}

void calc_tthresh(float volts, int * tthresh)
{
  *tthresh = (int)((volts + (float)6.944) / ((float)13.888 / (float)255.0));
}

PRIVATE int update_PulserTrigThresh()
{
  int err;

  switch (tthresh_index)
    {
    case TTL_:          /* TTL voltage wanted */
      tthresh_volts = (float)0.620;
      if (tsource_index == NEG_SLOPE)
        {
        tsource_index = POS_SLOPE;
        set_PulserTrigSrc(POS_SLOPE);
        display_random_field(&DA_Form, TSOURCE);
        }
    break;
    case FNIM_:          /* FAST NIM voltage wanted */
      {
      tthresh_volts = (float)-0.3;
      if (tsource_index == POS_SLOPE)
        {
        tsource_index = NEG_SLOPE;
        set_PulserTrigSrc(NEG_SLOPE);
        display_random_field(&DA_Form, TSOURCE);
        }
      }
    break;
    case SNIM_:          /* STD NIM voltage wanted */
      tthresh_volts = (float)1.125;
      if (tsource_index == NEG_SLOPE)
        tthresh_volts = -tthresh_volts;
    break;
    }
  err = set_TrigThreshVolts(tthresh_volts);
  set_for_pulser();
  display_random_field(&DA_Form, TVOLTS);
  return(errorCheckDetDriver(err));
}

PRIVATE int update_PulserTrigVolts()
{
  int err;

  err = set_TrigThreshVolts(tthresh_volts);

  tthresh_index = 4;
  set_for_pulser();
  display_random_field(&DA_Form, TTHRESH);

  return(errorCheckDetDriver(err));
}

PRIVATE int update_PulserType()
{
  return(errorCheckDetDriver(set_PulserType(pulser_type)));
}

PRIVATE int update_PulserWidth()
{
  int err = set_PulserWidth(pulser_width);
  get_ExposeTime(&exposure_time);
  set_for_pulser();
  display_random_field(&DA_Form, DA_ET);
  return(errorCheckDetDriver(err));
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void registerDA_Form(void)
{
   FormTable[KSI_DA_FORM] = & DA_Form;
}

static unsigned char call_prev_idle_handler(void)
{
  if(keyboard_idle -> prev_handler)
    {
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

PRIVATE void display_pulser_status(void)
{
  int temp_imode, temp_ptype;

  get_PulserType(&temp_ptype);
  if (!temp_ptype)
    return;
  get_IntensifierMode(&temp_imode);

  if (imode_index != temp_imode || pulser_type != temp_ptype)
    {
    imode_index = temp_imode;
    pulser_type = temp_ptype;
    display_random_field(&DA_Form, RS_IMODE);
    display_random_field(&DA_Form, PTYPE);
    }
}

static unsigned char update_pulser_status(void)
{
#define WAIT_INTERVAL 400

  static long next_showing = 0L;
  long now;

  if(update_enabled)
    {
    now = clock();

    if(next_showing <= now)
      {
      next_showing = now + WAIT_INTERVAL;
      display_pulser_status();
      }
    }
   return call_prev_idle_handler();
}
