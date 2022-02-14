/*
* FILE : RUNFORMS.C   RAC 6/05/91
*
*                    copyright (c) 1988, EG&G Instruments Inc.
*
*    some of the forms that are used under the "RUN" menu bar option :
*       live to disk, live, accum.
*    run setup form and background form are in their own modules.
*
*  $Header:   J:/logfiles/oma4000/main/runforms.c_v   1.8   06 Jul 1992 10:36:00   maynard  $
*  $Log:   J:/logfiles/oma4000/main/runforms.c_v  $
*
*/

#include <string.h>

#include "runforms.h"
#include "handy.h"     // DOSFILESIZE
#include "ksindex.h"
#include "livedisk.h"
#include "helpindx.h"
#include "curvedir.h"
#include "live.h"
#include "omaform.h"   // VerifyFileName()
#include "fkeyfunc.h"
#include "formtabs.h"
#include "forms.h"
#include "omaerror.h"
#include "syserror.h"
#include "di_util.h"
#include "fileform.h"  // is_special_name()
#include "formwind.h"
#include "macnres3.h"  // MakeNewCurveSet()
#include "tempdata.h"  // InsertMultiTempCurve()
#include "crvheadr.h"
#include "curvbufr.h"
#include "oma4driv.h"
#include "omameth.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

// PRIVATE functions
BOOLEAN LiveFormInit();
PRIVATE BOOLEAN LiveBackFormInit();
BOOLEAN AccumBackFormInit();
SHORT   VerifyFilterSettings(void);
SHORT   CheckFilterMode(void);
BOOLEAN LiveFormExit(void);

static int rfDummySelect = 0;

CHAR * GoLiveBackStr =  "F2 Go Live-B";
CHAR * GoAccumBackStr = "F2 Go Accum-B";
CHAR * GoLiveStr =  "F2 Go Live ";
CHAR * GoAccumStr = "F2 Go Accum";
CHAR * StopLiveStr = "F3 Stop Live";

static CHAR * StopAccumStr = "F3 Stop Accum";

// names for indexing into the DataRegistry.
enum { DGROUP_DO_STRINGS = 1, DGROUP_TOGGLE, DGROUP_CODE, DGROUP_DATA };

static DATA DO_STRING_Reg[] = {
/* 0 */ { "Live to Disk File Name", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 1 */ { "Number of Memories",    0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 2 */ { " Go ",                  0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 3 */ { "Live Capture Set Name", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 4 */ { "Live Capture Mode",     0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 5 */ { "Accum Set Name",        0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 6 */ { "Live Filter Type",      0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 7 */ { "Reference Curve Set",   0, DATATYP_STRING, DATAATTR_PTR, 0 },
};

static char * LiveFilter_options[] = {
     "None",
     "Absorbance (Log)",
     "Transmission",
     "Absorbance (Dual Track)",
     "Intensity Cal."
};

static char * LiveCaptureMode_options[] = {
     "Single Curve", "Multiple Curve"
};

static DATA TOGGLES_Reg[] = {
   { LiveCaptureMode_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
   { LiveFilter_options,      0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
};

static EXEC_DATA CODE_Reg[] = {
/* 0  */ { LiveFormInit,           0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 1  */ { LiveBackFormInit,       0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 2  */ { AccumFormInit,          0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 3  */ { AccumBackFormInit,      0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 4  */ { VerifyFileName,         0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 5  */ { GoLiveDisk,             0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 6  */ { VerifyWritableFileName, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 7  */ { VerifyFilterSettings,   0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 8  */ { LiveFormExit,           0, DATATYP_CODE, DATAATTR_PTR, 0 },
/* 9  */ { CheckFilterMode,        0, DATATYP_CODE, DATAATTR_PTR, 0 },
};

static DATA DATA_Reg[] = {
/* 0  */ { AccumFileName,    0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 1  */ { &rfDummySelect,   0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 2  */ { LiveFileName,     0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 3  */ { &LiveDiskCount,   0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 4  */ { LiveDiskFileName, 0, DATATYP_STRING, DATAATTR_PTR, 0 },
/* 5  */ { &SaveLiveFrame,   0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 6  */ { &LiveFilterMode,  0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 7  */ { LiveRefName,      0, DATATYP_STRING, DATAATTR_PTR, 0 },
};

/* Live to disk form */
enum LiveToDiskFieldNames { LDISK_FNAME_LABEL, NUMREPS_LABEL,
                            LDISK_FNAME, NUMREPS, LDISK_GO
};

// ====================================================================

FIELD LIVE_TO_DISK_FORMFields[] = {
   field_set(LDISK_FNAME_LABEL,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_DO_STRINGS, 0,       /* "Live to Disk File Name" */
   0, 0,
   0, 0,
   0, 0,                           /* (0:0) */
   5, 4, 25,
   NUMREPS_LABEL, LDISK_FNAME_LABEL, LDISK_FNAME_LABEL,
   LDISK_FNAME_LABEL, LDISK_FNAME_LABEL, LDISK_FNAME_LABEL,
   LDISK_FNAME_LABEL, LDISK_FNAME_LABEL),

   field_set(NUMREPS_LABEL,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_DO_STRINGS, 1,       /* "Number of Memories" */
   0, 0,
   0, 0,
   0, 0,                           /* (0:1) */
   9, 10, 18,
   LDISK_FNAME, NUMREPS_LABEL, NUMREPS_LABEL, NUMREPS_LABEL,
   NUMREPS_LABEL, NUMREPS_LABEL, NUMREPS_LABEL, NUMREPS_LABEL),

   field_set(LDISK_FNAME,
   FLDTYP_STRING,
   FLDATTR_REV_VID,
   KSI_LD_OUT_NAME,
   LIVEDISK_HBASE,
   DGROUP_DATA, 4,       /* LiveDiskFileName */
   0, 0,
   DGROUP_CODE, 4,           /* VerifyFileName */
   65, 0,                                                  /* (0:3) */
   5, 28, 48,
   EXIT, LDISK_FNAME, LDISK_GO, NUMREPS, LDISK_FNAME,
   LDISK_FNAME, LDISK_GO, NUMREPS),

   field_set(NUMREPS,
   FLDTYP_UNS_INT,
   FLDATTR_REV_VID,
   KSI_LD_REPS,
   LIVEDISK_HBASE + 1,
   DGROUP_DATA, 3,  /* LiveDiskCount */
   0, 0,
   0, 0,
   0, 0,                           /* (0:4) */
   9, 32, 5,
   EXIT, NUMREPS, LDISK_FNAME, LDISK_GO, NUMREPS, LDISK_GO,
   LDISK_FNAME, LDISK_GO),

   field_set(LDISK_GO,
   FLDTYP_SELECT,
   FLDATTR_REV_VID,
   KSI_LD_GO,
   LIVEDISK_HBASE + 3,
   DGROUP_DATA, 1,          /* rfDummy_select */
   DGROUP_DO_STRINGS, 2,       /* " Go " */
   DGROUP_CODE, 5,             /* GoLiveDisk */
   0, 0,                                                /* (0:6) */
   13, 36, 4,
   EXIT, LDISK_GO, NUMREPS, LDISK_FNAME, LDISK_GO, LDISK_GO,
   NUMREPS, LDISK_FNAME),

};

static FORM LIVE_TO_DISK_FORM = {
   0,
   0,
   FORMATTR_BORDER | FORMATTR_FULLSCREEN | FORMATTR_EXIT_RESTORE | FORMATTR_VISIBLE,
   0, 0, 0,
   2, 0, 21, 80,
   0, 0,
   { 0, 0 },
   { 0, 0 },
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(LIVE_TO_DISK_FORMFields) / sizeof(LIVE_TO_DISK_FORMFields[0]),
   LIVE_TO_DISK_FORMFields,
   KSI_LIVE_DISK_FORM,
   0,
   DO_STRING_Reg,
   TOGGLES_Reg,
   (DATA *)CODE_Reg,
   DATA_Reg, 0
};

static enum {LBL_LVFILT, LBL_LVREFNAM, LBL_LVCAPMOD, LBL_LVCAPNAM, 
             FLD_LVFILT, FLD_LVREFNAM, FLD_LVCAPMOD, FLD_LVCAPNAM };

static FIELD LiveFormFields[] = {
   label_field(LBL_LVFILT,
   DGROUP_DO_STRINGS, 6,    /*"Live Filter Mode" */
   5, 4, 17),

   label_field(LBL_LVREFNAM,
   DGROUP_DO_STRINGS, 7,    /*"Reference Curve Set" */
   7, 4, 20),

   label_field(LBL_LVCAPMOD,
   DGROUP_DO_STRINGS, 4,    /*"Live Capture Mode" */
   10, 4, 17),

   label_field(LBL_LVCAPNAM,
   DGROUP_DO_STRINGS, 3,    /* "Live Capture Set Name" */
   12, 4, 22),

   field_set(FLD_LVFILT,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_LVFILT,
   LIVE_HBASE + 2,
   DGROUP_DATA, 6,          /* LiveFilterMode */
   DGROUP_TOGGLE, 1,        /* LiveFilter_options */
   DGROUP_CODE, 9,
   0, 5,
   5, 26, 24,
   EXIT, FLD_LVFILT, FLD_LVCAPNAM, FLD_LVREFNAM,
   FLD_LVFILT, FLD_LVFILT, FLD_LVCAPNAM, FLD_LVREFNAM),

   field_set(FLD_LVREFNAM,
   FLDTYP_STRING,
   FLDATTR_REV_VID,
   KSI_LVREFNAME,
   LIVE_HBASE + 3,
   DGROUP_DATA, 7,           /* RefEntryName */
   0, 0,
   DGROUP_CODE, 7,           /* VerifyFileName */
   65, 0,                               
   7, 26, 50,
   EXIT, FLD_LVREFNAM, FLD_LVFILT, FLD_LVCAPMOD,
   FLD_LVREFNAM, FLD_LVREFNAM, FLD_LVFILT, FLD_LVCAPMOD),

   field_set(FLD_LVCAPMOD,
   FLDTYP_TOGGLE,
   FLDATTR_REV_VID,
   KSI_LIVE_OPTS,
   LIVE_HBASE + 1,
   DGROUP_DATA, 5,          /* SaveLiveFrame */
   DGROUP_TOGGLE, 0,        /* LiveCaptureMode_options */
   0, 0,
   0, 2,
   10, 26, 15,
   EXIT, FLD_LVCAPMOD, FLD_LVREFNAM, FLD_LVCAPNAM,
   FLD_LVCAPMOD, FLD_LVCAPMOD, FLD_LVREFNAM, FLD_LVCAPNAM),

  field_set(FLD_LVCAPNAM,
   FLDTYP_STRING,
   FLDATTR_REV_VID,
   KSI_LIVE_OUT_NAME,
   LIVE_HBASE,
   DGROUP_DATA, 2,          /* LiveFileName */
   0, 0,
   DGROUP_CODE, 6,          /* VerifyWritableFileName */
   65, 0,
   12, 26, 50,
   EXIT, FLD_LVCAPNAM, FLD_LVCAPMOD, FLD_LVFILT,
   FLD_LVCAPNAM, FLD_LVCAPNAM, FLD_LVCAPMOD, FLD_LVFILT),

};

static FORM LiveForm = {
   0, 0, FORMATTR_BORDER | FORMATTR_FULLSCREEN | 
    FORMATTR_EXIT_RESTORE | FORMATTR_VISIBLE,
   0, 0, 0, 2, 0, 21, 80, 0, 0,
   { DGROUP_CODE, 0 },             /* LiveFormInit */
   { DGROUP_CODE, 8 },
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(LiveFormFields) / sizeof(LiveFormFields[0]),
   LiveFormFields, KSI_LIVE_FORM,
   0, DO_STRING_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, DATA_Reg, 0
};

static FORM LiveBackForm = {
   0, 0, FORMATTR_BORDER | FORMATTR_EXIT_RESTORE |
     FORMATTR_VISIBLE | FORMATTR_FULLSCREEN,
   0, 0, 0, 2, 0, 21, 80, 0, 0,
   { DGROUP_CODE, 1 },          /* LiveBackFormInit */
   { DGROUP_CODE, 8 },
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(LiveFormFields) / sizeof(LiveFormFields[ 0 ]),
   LiveFormFields, KSI_NO_INDEX,
   0, DO_STRING_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, DATA_Reg, 0
};

static FIELD AccumFormFields[] = {
   { FLDTYP_STRING, FLDATTR_DISPLAY_ONLY, KSI_NO_INDEX,
      0, { DGROUP_DO_STRINGS, 5 },     /* "Accum Set Name" */
      {0, 0}, {0, 0}, {0, 0},
   8, 4, 15, { 1, 0, 0, 0, 0, 0, 0, 0 } },
   { FLDTYP_STRING, FLDATTR_REV_VID, KSI_ACCUM_OUT_NAME, ACCUM_HBASE,
      { DGROUP_DATA, 0 },           /* AccumFileName */
      {0, 0}, { DGROUP_CODE, 6 },      /* VerifyWritableFileName */
      {DOSFILESIZE + DOSPATHSIZE, 0},
   8, 19, 57, { -2, 0, 0, 0, 0, 0, 0, 0 } },
};

static FORM AccumForm = {
   0, 0, FORMATTR_BORDER | FORMATTR_EXIT_RESTORE |
   FORMATTR_FULLSCREEN | FORMATTR_VISIBLE,
   0, 0, 0, 2, 0, 21, 80, 0, 0,
   { DGROUP_CODE, 2 },           // AccumFormInit
   { 0, 0 }, COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(AccumFormFields) / sizeof(AccumFormFields[ 0 ]),
   AccumFormFields, KSI_ACCUM_FORM,
   0, DO_STRING_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, DATA_Reg, 0
};

static FORM AccumBackForm = {
   0, 0, FORMATTR_BORDER | FORMATTR_FULLSCREEN | 
    FORMATTR_EXIT_RESTORE | FORMATTR_VISIBLE,
   0, 0, 0, 2, 0, 21, 80, 0, 0,
   { DGROUP_CODE, 3 },       // AccumBackFormInit
   { 0, 0 }, COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(AccumFormFields) / sizeof(AccumFormFields[ 0 ]),
   AccumFormFields, KSI_NO_INDEX,
   0, DO_STRING_Reg, TOGGLES_Reg, (DATA *)CODE_Reg, DATA_Reg, 0
};

// ========================================================================


BOOLEAN LiveFormInit(void)
{
   FKeyItems[1].Text     = GoLiveStr;
   FKeyItems[1].TextLen  = (char) 0;
   FKeyItems[1].Control &= ~ MENUITEM_INACTIVE;
   FKeyItems[2].Text     = StopLiveStr;
   FKeyItems[2].Action   = StopLive;

   BackGroundActive = FALSE;
   ShowFKeys(&FKey);
   DoAccum = FALSE;

   return FALSE;
}

BOOLEAN LiveFormExit(void)
{
  if (LiveFilterMode != No_Filter && RefEntryIndex == NOT_FOUND)
    {
    error(ERROR_NO_REFERENCE_DATA);
    LiveFilterMode = No_Filter;
    display_random_field(Current.Form, FLD_LVFILT);
    return TRUE;
    }
  return FALSE;
}

/**************************************************************************
  returns truth that the Live filter setup form is visible
***************************************************************************/
PRIVATE BOOLEAN isRunForm(void)
{
  FORM * thisForm = CurrentForm();

  return (thisForm == &LiveForm || thisForm == &LiveBackForm);
}

/**************************************************************************
  Used by Live acquisition startup to check filtering setup.
  If filtering is OK, gets index of reference and returns ERROR_NONE
  If there is a problem, turns off filtering, warns user, and returns error
***************************************************************************/
ERR_OMA CheckFilterStatus(void)
{
  BOOLEAN UseLive = FALSE;

  /* if filter type is dual track, make sure scan setup agrees */
 
  if (LiveFilterMode == AbsorbDual)
    {
    float tracks;
    GetParam(DC_TRACKS, &tracks);
    /* Dual track mode needs 2 tracks */
    if (tracks == 2.0F)
      UseLive = TRUE;
    }

  /* get the index of the reference curve set into the global index */
  
  if (LiveFilterMode != No_Filter &&
      (FindCurveSetIndex(LiveRefName, &RefEntryIndex, UseLive) ||
       RefEntryIndex == NOT_FOUND))
    {
    LiveFilterMode = No_Filter;
    enable_field(&LiveForm, FLD_LVREFNAM, FALSE);
    return error(ERROR_NO_REFERENCE_DATA);
    }
  return ERROR_NONE;
}

/**************************************************************************
  Checks to see if filter mode and reference curve set are consistent, and
  if the reference set exists.  Also updates form if showing.
***************************************************************************/
SHORT CheckFilterMode(void)
{
  static SHORT HelpCount = 3;  /* only show special help 3 times */

  if (LiveFilterMode == No_Filter && RefEntryIndex != NOT_FOUND)
    {
    clearAllCurveBufs();
    RefEntryIndex = NOT_FOUND;
    enable_field(&LiveForm, FLD_LVREFNAM, TRUE);
    }
  else if (LiveFilterMode == AbsorbDual)
    {
    float tracks;
    GetParam(DC_TRACKS, &tracks);
    if (tracks != 2)                 /* Dual track mode needs 2 tracks */
      {
      error(ERROR_DUAL_TRACK_MODE);
      RefEntryIndex = NOT_FOUND;            /* turn off filtering */
      LiveFilterMode = No_Filter;
      if (isRunForm())               /* update form if showing */
        display_random_field(&LiveForm, FLD_LVFILT);
      return FIELD_VALIDATE_WARNING;
      }
    strcpy(LiveRefName, LastLiveEntryName);
    disable_field(&LiveForm, FLD_LVREFNAM, TRUE);

    FindCurveSetIndex(LastLiveEntryName, &RefEntryIndex, TRUE);

    /* Help message LIVE_HBASE + 5 tells the user what is about     */
    /* to happen. This instead of MessageBox so text can be changed */
    /* as needed without recompiling                                */

    if (HelpCount-- > 0 && isRunForm())
      form_help_from_file(LIVE_HBASE + 5);
    }
  else if (LiveFilterMode != No_Filter)
    {
    /* handle switch from none to filter with filename already chosen */
    enable_field(&LiveForm, FLD_LVREFNAM, TRUE);
    return VerifyFilterSettings();
    }
  return FIELD_VALIDATE_SUCCESS;
}

/**********************************************************************
  Verifies that selected reference can be used for selected filter mode
  If lastlive was designated as a static reference set, copies it to a
  new curveset which won't be overwritten when data acquisition starts
  Used by CheckFilterMode
***********************************************************************/
SHORT VerifyFilterSettings(void)
{
  ERR_OMA err;
  static SHORT HelpCount = 3;       /* only show help screen 3 times */
  BOOLEAN RunVisible = isRunForm(); /* flag whether to update fields */

  if (strlen(LiveRefName) != 0)
    {
    FindCurveSetIndex(LiveRefName, &RefEntryIndex, TRUE);
    
    /* if it doesn't exist, warn and return */
    
    if (RefEntryIndex == NOT_FOUND)
      {
      error(ERROR_CURVESET_NOT_FOUND, LiveRefName);
      return FIELD_VALIDATE_WARNING;
      }

    /* if the reference curve set is live, & the filter mode is not dual */
    /* track absorbance, copy reference to a more permanent curve set    */

    if (!stricmp(LastLiveEntryName, LiveRefName) &&
        LiveFilterMode != AbsorbDual)
      {
      USHORT NewIndex,
             SaveFilterMode = LiveFilterMode; /* Save current mode */
      WINDOW * MessageWindow;

      LiveFilterMode = No_Filter;   /* turn off - don't filter filter data */

      /* Help message LIVE_HBASE + 4 tells the user what is about     */
      /* to happen. This instead of MessageBox so text can be changed */
      /* as needed without recompiling                                */

      if (HelpCount-- > 0 && RunVisible)
        form_help_from_file(LIVE_HBASE + 4);

      put_up_message_window(BusyWorking, COLORS_MESSAGE, &MessageWindow);
    
      NewIndex = MakeNewCurveSet(); /* create new empty set */
      
      if (NewIndex == (USHORT)NOT_FOUND) /* if couldn't create curve set */
        {
        release_message_window(MessageWindow);
        if (RunVisible)
          display_random_field(&LiveForm, FLD_LVFILT);
        return FIELD_VALIDATE_WARNING;
        }

      /* copy data from live to new set */

      err = 
        InsertMultiTempCurve(&MainCurveDir,
                            RefEntryIndex, 0, /* source */
                            NewIndex, 0,      /* dest */
                            MainCurveDir.Entries[RefEntryIndex].count);

      release_message_window(MessageWindow);

      if (err)
        {
        if (RunVisible)
          display_random_field(&LiveForm, FLD_LVFILT);
        return FIELD_VALIDATE_WARNING;
        }

      LiveFilterMode = SaveFilterMode;  /* restore filter mode */
      
      /* copy the name and path of the new set into the field used    */
      /* to display the name on this form.  This string also used     */
      /* when live acquisition starts to find the reference curve set */

      strcpy(LiveRefName, MainCurveDir.Entries[NewIndex].path);
      strcat(LiveRefName, MainCurveDir.Entries[NewIndex].name);
      if (RunVisible)
        display_random_field(&LiveForm, FLD_LVREFNAM);
      }
    }
  return FIELD_VALIDATE_SUCCESS;
}

/*************************************************************************
  for use by macros
  turns on a filtering mode for live data
*************************************************************************/
BOOLEAN SetFilter(enum FilterModes Mode, char * RefName)
{
  if (Mode < No_Filter || Mode >= NumFiltModes)
    return FALSE;

  if (!RefName && Mode != No_Filter)
    {
    error(ERROR_NO_REFERENCE_DATA);
    return FALSE;
    }

  if (Mode == AbsorbDual && !is_special_name(RefName))
    return FALSE;

  strcpy(LiveRefName, RefName);
  LiveFilterMode = Mode;

  return (CheckFilterMode() == FIELD_VALIDATE_SUCCESS);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN LiveBackFormInit(void)
{
   FKeyItems[1].Text     = GoLiveBackStr;
   FKeyItems[1].TextLen  = (char) 0;
   FKeyItems[1].Control &= ~ MENUITEM_INACTIVE;
   FKeyItems[2].Text     = StopLiveStr;
   FKeyItems[2].Action   = StopLive;

   BackGroundActive = TRUE;
   ShowFKeys(&FKey);
   DoAccum = FALSE;

   return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN AccumFormInit(void)
{
   FKeyItems[1].Text     = GoAccumStr;
   FKeyItems[1].TextLen  = (char) 0;
   FKeyItems[1].Control &= ~ MENUITEM_INACTIVE;
   FKeyItems[2].Text     = StopAccumStr;
   FKeyItems[2].Action   = StopLive;

   BackGroundActive = FALSE;
   ShowFKeys(&FKey);
   DoAccum = TRUE;

   return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN AccumBackFormInit()
{
   FKeyItems[1].Text     = GoAccumBackStr;
   FKeyItems[1].TextLen  = (char) 0;
   FKeyItems[1].Control &= ~ MENUITEM_INACTIVE;
   FKeyItems[2].Text     = StopAccumStr;
   FKeyItems[2].Action   = StopLive;

   BackGroundActive = TRUE;
   ShowFKeys(&FKey);
   DoAccum = TRUE;

   return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void registerRUNFORMS(void)
{
   FormTable[ KSI_LIVE_DISK_FORM] = &LIVE_TO_DISK_FORM;
   FormTable[ KSI_LIVE_FORM     ] = &LiveForm;
   FormTable[ KSI_ACCUM_FORM    ] = &AccumForm;

   NorecFormTable[NOREC_LIVE_BACK_FORM]  = & LiveBackForm;
   NorecFormTable[NOREC_ACCUM_BACK_FORM] = & AccumBackForm;
}
