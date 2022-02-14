/* -----------------------------------------------------------------------
/
/  statform.c
/
/  Copyright (c) 1993,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/statform.c_v
/  $Log:   J:/logfiles/oma4000/main/statform.c_v  $
*/

#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <string.h>
#include <malloc.h>

#include "statform.h"
#include "mathops.h"
#include "tempdata.h"
#include "curvedir.h"
#include "helpindx.h"
#include "ksindex.h"
#include "formwind.h"
#include "omaerror.h"
#include "formtabs.h"
#include "crvheadr.h"
#include "forms.h"
#include "splitfrm.h"
#include "di_util.h"  // ParsePathAndName()
#include "macres2.h"  // DoStats
#include "fileform.h" // isSpecialName()

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

/* data for plotmode/menumode switching */

BOOLEAN InStatForm = FALSE;     

PRIVATE char Buffer[81];

OP_BLOCK Src = { "", 0.0, -1, 0, 0, TRUE };

PRIVATE USHORT PointStart, EndPoint;

PRIVATE float Average, StdDev, Area;
PRIVATE int sel_dummy;

static DATA DO_STRING_Reg[] = {
   { "Source Curve", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 0
   { "Start Curve",  0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 1
   { "Count",        0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 2
   { "Start Point",  0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 3
   { "Calculate",    0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 4
   { "Average:",     0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 5
   { "Std. Dev.:",   0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 6
   { "Area:",        0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 7
   { "End Point",    0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 8
};

PRIVATE int RunStat(void* field_data, char * field_string);
PRIVATE int VerifySource(void);
PRIVATE int VerifyCount(void);
PRIVATE BOOLEAN InitStatForm(void);
PRIVATE BOOLEAN ExitStatForm(void);

static EXEC_DATA CODE_Reg[] = {
   { CAST_CHR2INT RunStat, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   { VerifySource, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   { VerifyCount,  0, DATATYP_CODE, DATAATTR_PTR, 0 },
   { InitStatForm, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   { ExitStatForm, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
};

static DATA DATA_Reg[] = {
  {Src.Spec,    0, DATATYP_STRING, DATAATTR_PTR, 0}, // 0 source file name
  {&Src.Start,  0, DATATYP_INT,   DATAATTR_PTR, 0}, // 1 source start curve #
  {&Src.Count,  0, DATATYP_INT,   DATAATTR_PTR, 0}, // 2 source curve count
  {&PointStart, 0, DATATYP_INT,   DATAATTR_PTR, 0}, // 3 source start point
  {&EndPoint,   0, DATATYP_INT,   DATAATTR_PTR, 0}, // 4 source end point
  {&Average,    0, DATATYP_FLOAT, DATAATTR_PTR, 0}, // 5 calculated average 
  {&StdDev,     0, DATATYP_FLOAT, DATAATTR_PTR, 0}, // 6 calculated std. dev.
  {&Area,       0, DATATYP_FLOAT, DATAATTR_PTR, 0}, // 7 calculated area   
  {&sel_dummy,  0, DATATYP_INT, DATAATTR_PTR, 0 },  // 8 for select field
};

// names for indexing into the DataRegistry.
enum DATAREGISTRY_ACCESS { DGROUP_DOSTRING = 1, DGROUP_CODE,
                           DGROUP_DATA };

// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// Start of field and form declarations from statform.cfm

enum {LBL_SOURCE,
      LBL_CRVSTRT,
      LBL_CRVCNT,
      LBL_PNTSTRT,
      LBL_PNTCNT,
      LBL_AVG,
      LBL_STD,
      LBL_AREA,
      FLD_SOURCE,
      FLD_CRVSTRT,
      FLD_CRVCNT,
      FLD_PNTSTRT,
      FLD_PNTCNT,
      FLD_AVG,
      FLD_DEV,
      FLD_AREA,
      FLD_CALC,
      };

FIELD StatFormFields[] = {

  label_field(LBL_SOURCE,
              DGROUP_DOSTRING, 0,
              2, 2, 12),
  label_field(LBL_CRVSTRT,
              DGROUP_DOSTRING, 1,
              3, 43, 11),
  label_field(LBL_CRVCNT,
              DGROUP_DOSTRING, 2,
              3, 62, 5),
  label_field(LBL_PNTSTRT,
              DGROUP_DOSTRING, 3,
              5, 43, 11),
  label_field(LBL_PNTCNT,
              DGROUP_DOSTRING, 8,
              5, 62, 9),
  label_field(LBL_AVG,
              DGROUP_DOSTRING, 5,
              7, 2, 8),
  label_field(LBL_STD,
              DGROUP_DOSTRING, 6,
              7, 27, 10),
  label_field(LBL_AREA,
              DGROUP_DOSTRING, 7,
              7, 54, 5),

  field_set(FLD_SOURCE,
            FLDTYP_STRING,
            FLDATTR_REV_VID,
            KSI_STAT_SOURCE,
            STATFORM_FIELD_HBASE + 0,
            DGROUP_DATA, 0,
            0, 0,
            DGROUP_CODE, 1,
            80, 0,
            3, 2, 36,
            EXIT, FLD_SOURCE, FLD_CALC, FLD_CALC,
            FLD_CRVCNT, FLD_CRVSTRT, FLD_CRVCNT, FLD_CRVSTRT),

  field_set(FLD_CRVSTRT,
            FLDTYP_UNS_INT,
            FLDATTR_REV_VID,
            KSI_STAT_CRVSTART,
            STATFORM_FIELD_HBASE + 1,
            DGROUP_DATA, 1,
            0, 0,
            0, 0,
            0, 0,
            3, 55, 5,
            EXIT, FLD_CRVSTRT, FLD_CALC, FLD_PNTSTRT,
            FLD_SOURCE, FLD_CRVCNT, FLD_SOURCE, FLD_CRVCNT),

  field_set(FLD_CRVCNT,
            FLDTYP_UNS_INT,
            FLDATTR_REV_VID,
            KSI_STAT_CRVCOUNT,
            STATFORM_FIELD_HBASE + 2,
            DGROUP_DATA, 2,
            0, 0,
            DGROUP_CODE, 2,
            0, 0,
            3, 72, 5,
            EXIT, FLD_CRVCNT, FLD_CALC, FLD_PNTCNT,
            FLD_CRVSTRT, FLD_CRVCNT, FLD_CRVSTRT, FLD_PNTSTRT),

  field_set(FLD_PNTSTRT,
            FLDTYP_UNS_INT,
            FLDATTR_REV_VID,
            KSI_STAT_PNTSTART,
            STATFORM_FIELD_HBASE + 3,
            DGROUP_DATA, 3,
            0, 0,
            DGROUP_CODE, 2,
            0, 0,
            5, 55, 5,
            EXIT, FLD_PNTSTRT, FLD_CRVSTRT, FLD_CALC,
            FLD_CRVCNT, FLD_PNTCNT, FLD_CRVCNT, FLD_PNTCNT),

  field_set(FLD_PNTCNT,
            FLDTYP_UNS_INT,
            FLDATTR_REV_VID,
            KSI_STAT_PNTCOUNT,
            STATFORM_FIELD_HBASE + 4,
            DGROUP_DATA, 4,
            0, 0,
            DGROUP_CODE, 2,
            0, 0,
            5, 72, 5,
            EXIT, FLD_PNTCNT, FLD_CRVCNT, FLD_CALC,
            FLD_PNTSTRT, FLD_CALC, FLD_PNTSTRT, FLD_CALC),

  field_set(FLD_AVG,
            FLDTYP_STD_FLOAT,
            FLDATTR_DISPLAY_ONLY | FLDATTR_RJ | FLDATTR_HIGHLIGHT,
            KSI_NO_INDEX,
            0,
            DGROUP_DATA, 5,
            0, 0,
            0, 0,
            2, 0,
            7, 11, 12,
            EXIT, 0, 0, 0, 0, 0, 0, 0),

  field_set(FLD_DEV,
            FLDTYP_STD_FLOAT,
            FLDATTR_DISPLAY_ONLY | FLDATTR_RJ | FLDATTR_HIGHLIGHT,
            KSI_NO_INDEX,
            0,
            DGROUP_DATA, 6,
            0, 0,
            0, 0,
            4, 0,
            7, 38, 12,
            EXIT, 0, 0, 0, 0, 0, 0, 0),

  field_set(FLD_AREA,
            FLDTYP_STD_FLOAT,
            FLDATTR_DISPLAY_ONLY | FLDATTR_RJ | FLDATTR_HIGHLIGHT,
            KSI_NO_INDEX,
            0,
            DGROUP_DATA, 7,
            0, 0,
            0, 0,
            2, 0,
            7, 60, 12,
            EXIT, 0, 0, 0, 0, 0, 0, 0),

  field_set(FLD_CALC,                       /* select field to start calc */
            FLDTYP_SELECT,
            FLDATTR_REV_VID,
            KSI_STAT_GO, 
            STATFORM_FIELD_HBASE + 5,
            DGROUP_DATA, 8,
            DGROUP_DOSTRING, 4,
            DGROUP_CODE, 0,
            0, 0,
            9, 35, 9,
            EXIT,FLD_CALC, FLD_SOURCE, FLD_SOURCE,
            FLD_PNTCNT, FLD_SOURCE, FLD_PNTCNT, FLD_SOURCE),
};

static FORM StatForm =
{
   /* 1st 2 fields are for run-time storage                        */
   0,                          /* runtime current field_index;     */
   0,                          /* runtime previous_field_index;    */
   FORMATTR_BORDER |           /* attributes of form               */
   FORMATTR_FIRST_CHAR_ERASE |
   FORMATTR_EXIT_RESTORE |
   FORMATTR_FULLWIDTH |
   FORMATTR_VISIBLE,
   0,                          /* runtime next file offset         */
   0,                          /* runtime code from last exit      */
   0,                          /* runtime form status              */
   2,0,                        /* form position, row and column    */
   11, 80,                     /* form size, rows and columns      */
   /* Next 2 fields are for scrolling forms                        */
   0,                          /* row of active field              */
   0,                          /* "index #" of active field        */
   {DGROUP_CODE, 3},           /* function to run on entry         */
   {DGROUP_CODE, 4},           /* function to run on exit          */
   COLORS_DEFAULT,             /* color set to use                 */
   0,                          /* runtime pos of cursor in data    */
   0,                          /* runtime pos of cursor in display */
   0,                          /* runtime current chars in field   */
   0,                          /* runtime field currently overfull */
   sizeof(StatFormFields) / sizeof(StatFormFields[0]) ,
   StatFormFields,             /* pointer to field array           */
   /* Next field is for keystroke record and playback,             */
   /* an index into a file of macro commands                       */
   /* one of which will activate this form                         */
   KSI_MATH_FORM,              /* Form index in keystroke table    */
   0, DO_STRING_Reg, (DATA *)CODE_Reg, DATA_Reg, 0, 0
};


PRIVATE BOOLEAN InitStatForm(void)
{
  return InitSplitForm(&StatForm, &InStatForm, NULL);
}

PRIVATE BOOLEAN ExitStatForm(void)
{
  return ExitSplitForm(&InStatForm);
}


/*************************************************************************/
/* Checks to see if the curve block exists.  For convenience, also fills */
/* in SrcStart, SrcCount, PointStart, and EndPoint fields.  For curve    */
/* counts, values should be OK.  For points, values are taken from first */
/* curve in the set so they will be OK 99.9% of the time.  If joker made */
/* up curve set from differing length curves, DoStat routine should get  */
/* an error (from LoadCurveBuffer) when it tries to do GetDataPoint. User*/
/* may still enter correct values directly into EndPoint and PointStart  */
/*************************************************************************/
PRIVATE int VerifySource(void)
{
  CHAR Name[DOSFILESIZE+1];
  CHAR Path[DOSPATHSIZE+1];
  SHORT OverlapCheck = BADNAME;
  SHORT ReturnVal = FIELD_VALIDATE_SUCCESS;
  USHORT Index = Src.BlkIndex;

  if(is_special_name(Src.Spec))
    {
    Path[0] = '\0';
    strcpy(Name, Src.Spec);
    }
  else if (ParsePathAndName(Path, Name, Src.Spec) != 2)
    ReturnVal = FIELD_VALIDATE_WARNING;

  if (ReturnVal == FIELD_VALIDATE_SUCCESS)
    {
    Src.BlkIndex = SearchCurveBlkDir(Name, Path, 0, &MainCurveDir);
    if (Src.BlkIndex != -1 && Src.BlkIndex != Index)
      {
      CURVE_ENTRY * pEntry = &MainCurveDir.Entries[Src.BlkIndex];
      Src.Start = pEntry->StartIndex;
      Src.Count = pEntry->count;

      OverlapCheck = CheckCurveBlkOverLap(Path, Name, Src.Start,
                                          Src.Start+Src.Count, &Src.BlkIndex);
      }

    if (OverlapCheck == BADNAME)
      ReturnVal = FIELD_VALIDATE_WARNING;
    else if (Index != Src.BlkIndex)
      {
      CURVEHDR Curvehdr;

      ReadTempCurvehdr(&MainCurveDir, Src.BlkIndex, Src.Start, &Curvehdr);
      PointStart = 0;
      EndPoint = Curvehdr.pointnum - 1;
      Average = StdDev = Area = 0.0F;
      }
    }

  Index = Current.Form->field_index;
  draw_form();
  Current.Form->field_index = Index;
  format_and_display_field(FALSE);

  return ReturnVal;
}

/*************************************************************************/
/* Check the point start and point count values to see if they are OK.   */
/* This reads each curve from the source set and so check is more        */
/* positive than that in VerifySource above.                             */
/*************************************************************************/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int VerifyCount(void)
{
  USHORT i,
         Index = Src.BlkIndex,
         RealCount;
  CURVEHDR Curvehdr;

  if (Index == -1)
    return(FIELD_VALIDATE_WARNING);
  
  RealCount = MainCurveDir.Entries[Index].count;

  switch(Current.Form->field_index)
    {
    case FLD_CRVSTRT:
    case FLD_CRVCNT:
      if (Src.Count + Src.Start > RealCount)
        return FIELD_VALIDATE_WARNING;
      else
        return(FIELD_VALIDATE_SUCCESS);
    case FLD_PNTSTRT:
      if (Src.Count == 0)
        return(FIELD_VALIDATE_SUCCESS);
    case FLD_PNTCNT:
      for (i = Src.Start; i < Src.Start + Src.Count; i++)
        {
        if(ReadTempCurvehdr(&MainCurveDir, Index, i, &Curvehdr))
          return FIELD_VALIDATE_WARNING;
        if (PointStart > (Curvehdr.pointnum - 1) ||
            EndPoint > (Curvehdr.pointnum - 1) ||
            (EndPoint == PointStart && Src.Count < 2) || /* if 1 point, */
            EndPoint < PointStart)                      /* must be 2 curves */

          return FIELD_VALIDATE_WARNING;
        }
    /* fall through */
    default:
      return(FIELD_VALIDATE_SUCCESS);
    }
}

// Do simple stat functions on curve blocks
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN RunStatProc(void* field_data, char * field_string)
{
  BOOLEAN status = FALSE;
  STAT_STRUCT Stats;
  WINDOW * MessageWindow;

  if (!EndPoint || (Src.BlkIndex == -1))
    return(TRUE);

  Stats.DoCArea = FALSE;

  put_up_message_window(BusyWorkingEsc, COLORS_MESSAGE, &MessageWindow);

  DoStats(&Src, PointStart, EndPoint - PointStart + 1, &Stats);

  if (MessageWindow != NULL)
    release_message_window(MessageWindow);

  Average = (float)Stats.Avg;
  StdDev  = (float)Stats.Dev;
  Area    = (float)Stats.Area;
  display_random_field(&StatForm, FLD_AVG);
  display_random_field(&StatForm, FLD_DEV);
  display_random_field(&StatForm, FLD_AREA);

  return status;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int RunStat(void* field_data, char * field_string)
{
   if (RunStatProc(field_data, field_string))
      return FIELD_VALIDATE_WARNING;
   else
      return FIELD_VALIDATE_SUCCESS;
}

// put & StatForm into FormTable[]
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void registerStatForm(void)
{
   FormTable[KSI_STAT_FORM] = &StatForm;
}
