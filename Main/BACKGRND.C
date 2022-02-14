/* -----------------------------------------------------------------------
/
/  backgrnd.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/backgrnd.c_v   0.24   06 Jul 1992 10:24:10   maynard  $
/  $Log:   J:/logfiles/oma4000/main/backgrnd.c_v  $
 * 
 *    Rev 0.24   06 Jul 1992 10:24:10   maynard
 *    Rev 1.0    19 Aug 1993 17:54:00   maynard
 *
 * rewrote Copy To Background.  Make Entry of new background file name
 * update the start curve and count fields with values from curve dir.
 * Background file name is validated on entry as well as when
 * CopyToBackground is called for better usability.
 *
*/

#include <string.h>

#include "backgrnd.h"
#include "tempdata.h"
#include "di_util.h"
#include "live.h"
#include "oma4000.h"
#include "formwind.h"
#include "omamenu.h"   // RunMenuItems[]
#include "fileform.h"  // is_special_name()
#include "ksindex.h"
#include "helpindx.h"
#include "formtabs.h"
#include "macres2.h"
#include "mathops.h"
#include "syserror.h"  // ERROR_BAD_FILENAME
#include "omaerror.h"
#include "omameth.h"   // InitialMethod
#include "crventry.h"
#include "curvedir.h"
#include "forms.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

char *BackGroundEntryName = "backgrnd";
char *BackGroundDesc = "Background Curve Set";

PRIVATE int RunBackGround(void);
PRIVATE int VerifyBackgroundEntry(void);
PRIVATE int VerifyBackgroundCurves(void);

static char * BlkNotLoadedString[] = {
   "The requested curve block is not in",
   "      the Curve Directory!\n",
   "     Try to load from disk?",
   NULL
};

static int dummySelect = 0;
static char NameBuffer[DOSFILESIZE+DOSPATHSIZE] = {'\0'};
static USHORT StartIndex = 0;
static USHORT Count = 1;

// names for indexing into the DataRegistry.
enum { DGROUP_DO_STRINGS = 1, DGROUP_CODE, DGROUP_DATA };

static DATA DO_STRING_Reg[] = {
 /* 0 */ { "Background", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 1 */ { "Curve Start", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 2 */ { "Count", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 3 */ { "Copy To BackGround", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
};

static EXEC_DATA CODE_Reg[] = {
 /* 0  */ { VerifyBackgroundEntry, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 1  */ { VerifyBackgroundCurves, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
 /* 2  */ { RunBackGround, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
};

static DATA DATA_Reg[] = {
 /* 0  */ { &dummySelect, 0, DATATYP_INT, DATAATTR_PTR, 0 },
           // background curve block name
 /* 1  */ { NameBuffer, 0, DATATYP_STRING, DATAATTR_PTR, 0 },
           // background curve block low range limit
 /* 2  */ { &StartIndex, 0, DATATYP_INT, DATAATTR_PTR, 0 },
           // background curve block high range limit
 /* 3  */ { &Count, 0, DATATYP_INT, DATAATTR_PTR, 0 },
};

static enum { LBL_SRC, LBL_START, LBL_COUNT,
              FLD_SRC, FLD_START, FLD_COUNT,
              FLD_BKGO };

static FIELD BackGroundFields[] = {
 label_field(LBL_SRC,
  DGROUP_DO_STRINGS, 0,         /* "Background" */
  8, 1, 10),

 label_field(LBL_START,
  DGROUP_DO_STRINGS, 1,         /* "Curve Start" */
  8, 49, 11),

 label_field(LBL_COUNT,
  DGROUP_DO_STRINGS, 2,         /* "Count" */
  8, 67, 5),

 field_set(FLD_SRC,
  FLDTYP_STRING,
  FLDATTR_REV_VID,
  KSI_BGRND_NAME,               /* 3 */
  BACKGROUNDFIELDS_HBASE + 0,
  DGROUP_DATA, 1,               /* NameBuffer */
  0, 0,
  DGROUP_CODE, 0,               /* VerifyBackgrounEntry */
  80, 0,
  8, 12, 36,
  EXIT, FLD_SRC, FLD_BKGO, FLD_BKGO,
  FLD_SRC, FLD_COUNT, FLD_BKGO, FLD_START),

 field_set(FLD_START,
  FLDTYP_UNS_INT,
  FLDATTR_REV_VID,
  KSI_BGRND_START,              /* 4 */
  BACKGROUNDFIELDS_HBASE + 1,
  DGROUP_DATA, 2,               /* StartIndex */
  0, 0,
  DGROUP_CODE, 1,               /* VerifyBackgroundCurves */
  0, 0,
  8, 61, 5,
  EXIT, FLD_START, FLD_BKGO, FLD_BKGO,
  FLD_SRC, FLD_COUNT, FLD_SRC, FLD_COUNT),
   
 field_set(FLD_COUNT,
  FLDTYP_UNS_INT,
  FLDATTR_REV_VID,
  KSI_BGRND_COUNT,              /* 5 */
  BACKGROUNDFIELDS_HBASE + 2,
  DGROUP_DATA, 3,               /* Count */
  0, 0,
  DGROUP_CODE, 1,               /* VerifyBackgroundCurves */
  0, 0,
  8, 73, 5,
  EXIT, FLD_COUNT, FLD_BKGO, FLD_BKGO, FLD_SRC,
  FLD_COUNT, FLD_START, FLD_BKGO),

 field_set(FLD_BKGO,
  FLDTYP_SELECT,
  FLDATTR_REV_VID,
  KSI_BGRND_GO,                 /* 6 */
  BACKGROUNDFIELDS_HBASE + 3,
  DGROUP_DATA, 0,               /* rfDummySelect */
  DGROUP_DO_STRINGS, 3,         /* "Copy To BackGround" */
  DGROUP_CODE, 2,               /* RunBackGround */
  0, 0,
  11, 27, 18,
  EXIT, FLD_BKGO, FLD_SRC, FLD_SRC,
  FLD_BKGO, FLD_BKGO, FLD_COUNT, FLD_SRC),
};

static FORM BackGroundForm = {
   0, 0,
   FORMATTR_BORDER | FORMATTR_FULLSCREEN | FORMATTR_EXIT_RESTORE | FORMATTR_VISIBLE,
   0, 0, 0, 2, 0, 21,
   80, 0, 0,
   { 0, 0 }, { 0, 0 }, COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(BackGroundFields) / sizeof(BackGroundFields[ 0 ]),
   BackGroundFields, KSI_BACKGROUND_FORM,
   0, DO_STRING_Reg, (DATA*)CODE_Reg, DATA_Reg, 0, 0
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA CheckBackgroundEntry(CHAR *FileName, SHORT * SrcIndex)
{
  CHAR Name[DOSFILESIZE + 1], Path[DOSPATHSIZE + 1];
  ERR_OMA err;
  WINDOW * MessageWindow;
  BOOLEAN specialFile = is_special_name(FileName);

  *SrcIndex = -1;

  /* expand the file block name or constant string and check to see        */
  /* that it is not a directory                                            */
  /* if FileName is a special file name, name is OK so don't uppercase it. */

  if(specialFile)
    {
    Path[0] = '\0';
    strcpy(Name, FileName);
    }
  else if (ParsePathAndName(Path, Name, FileName) != 2)
      return error(ERROR_BAD_FILENAME, FileName);

  *SrcIndex = SearchNextNamePath(Name, Path, &MainCurveDir, 0);
  if (*SrcIndex == -1)
    {
    SHORT Curves;
    if(yes_no_choice_window(BlkNotLoadedString, 0, COLORS_ERROR) != YES)
      return ERROR_NONE;

    put_up_message_window(BusyWorking, COLORS_MESSAGE, &MessageWindow);
    
    *SrcIndex = MainCurveDir.BlkCount;
    err = ReadFileToTemp(Name, Path, 0, -1, &MainCurveDir, SrcIndex, &Curves);
    release_message_window(MessageWindow);
    return err;
    }
  return ERROR_NONE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int VerifyBackgroundEntry(void)
{
  SHORT SrcIndex;
  
  if (CheckBackgroundEntry(NameBuffer, &SrcIndex))
    return FIELD_VALIDATE_WARNING;

  if (SrcIndex != -1)
    {
    StartIndex = MainCurveDir.Entries[SrcIndex].StartIndex;
    Count = MainCurveDir.Entries[SrcIndex].count;
    display_random_field(&BackGroundForm, FLD_START);
    display_random_field(&BackGroundForm, FLD_COUNT);
    }
    
  return FIELD_VALIDATE_SUCCESS;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int VerifyBackgroundCurves(void)
{
  SHORT SrcIndex;
  CURVE_ENTRY * pEntry;

  if (CheckBackgroundEntry(NameBuffer, &SrcIndex))
    return FIELD_VALIDATE_WARNING;
 
  if (SrcIndex != -1)
    {
    pEntry = &MainCurveDir.Entries[SrcIndex];
    if ((StartIndex + Count) > (pEntry->StartIndex + pEntry->count))
      return FIELD_VALIDATE_WARNING;
    }
  return FIELD_VALIDATE_SUCCESS;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA CopyToBackGround(CHAR *Name, USHORT StartCurve, USHORT Count)
{
  SHORT BkgIndex;
  USHORT SrcIndex;
  CURVE_ENTRY *pSrcEntry;
  WINDOW * MessageWindow;
  BOOLEAN TempBkgActive;
  ERR_OMA err;

  /* See if supplied name is OK, and if so, fetch its curve dir index */
  err = CheckBackgroundEntry(Name, &SrcIndex);
  if (err || SrcIndex == (USHORT)NOT_FOUND)
    return err;
  
  BkgIndex = FindSpecialEntry(BackGroundEntryName);

  /* Catch case of copying backgrd to backgrd */
  if (BkgIndex == (SHORT)SrcIndex)
    return(error(ERROR_BKG_TO_BKG, BackGroundEntryName));

  // These indices need to be coordinated with those in omaform1.c
  // Deactivate Live-B and Accum-B in case of error
  RunMenuItems[2].Control |= MENUITEM_INACTIVE;
  RunMenuItems[5].Control |= MENUITEM_INACTIVE;

  /* check start curve and count to be sure they are legal */
  pSrcEntry = &MainCurveDir.Entries[SrcIndex];

  if((pSrcEntry->StartIndex > StartCurve) ||
    (pSrcEntry->StartIndex + pSrcEntry->count < StartCurve + Count))
      return error(ERROR_BAD_CURVE_BLOCK, SrcIndex);

  /* turn off auto subtract of live, but remember if it was on */
  TempBkgActive = BackGroundActive, BackGroundActive = FALSE;

  /* delete background entry if it exists, then get new index of */
  /* source to copy */
  if(BkgIndex != -1)
    {
    err = DelTempFileBlk(&MainCurveDir, BkgIndex);
    if (err)
      return err;
    err = CheckBackgroundEntry(Name, &SrcIndex);
    if (err || SrcIndex == (USHORT)NOT_FOUND)
      return err;
    /* reget index of lastlive in case LiveLoop is running */
    LiveBlkIndex = FindSpecialEntry(LastLiveEntryName);
    }

  put_up_message_window(BusyWorking, COLORS_MESSAGE, &MessageWindow);

  err = CreateTempFileBlk(&MainCurveDir, &BkgIndex, BackGroundEntryName, "",
                          BackGroundDesc, 0, 0L, 0, NULL, OMA4DATA, 0);
  if (!err)
    {
    BackGroundIndex = BkgIndex;

    /* insert curves at index 0 */
    err = InsertMultiTempCurve(&MainCurveDir, SrcIndex,
                               StartCurve, BkgIndex, 0, Count);
    if (err)
      DelTempFileBlk(&MainCurveDir, BkgIndex);
    else
      {
      /* re-use pSrcEntry as background entry pointer */
      pSrcEntry = &MainCurveDir.Entries[BkgIndex];
      strcpy(pSrcEntry->name, BackGroundEntryName);
      pSrcEntry->path[0] = '\0';

      // These indices need to be coordinated with those in omaform1.c
      RunMenuItems[2].Control &= ~MENUITEM_INACTIVE;
      RunMenuItems[5].Control &= ~MENUITEM_INACTIVE;
      draw_menu();

      strcpy(NameBuffer, Name);  // in case called by macro, set field data
      strcpy(InitialMethod->BackgrndName, Name);
      }
    }
  
  if (!err)
    BackGroundActive = TempBkgActive;

  if (MessageWindow != NULL)
    release_message_window(MessageWindow);

  return err;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int RunBackGround()
{
   if(CopyToBackGround(NameBuffer, StartIndex, Count))
      return FIELD_VALIDATE_WARNING;

   return FIELD_VALIDATE_SUCCESS;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void registerBackgroundForm(void)
{
   FormTable[KSI_BACKGROUND_FORM] = &BackGroundForm;
}
