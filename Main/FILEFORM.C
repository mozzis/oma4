/* -----------------------------------------------------------------------
/
/  fileform.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/fileform.c_v   0.35   06 Jul 1992 10:36:46   maynard  $
/  $Log:   J:/logfiles/oma4000/main/fileform.c_v  $
/
*/

#include <stdio.h>
#include <dos.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <direct.h>
#include <malloc.h>
#include <stddef.h>    // offsetof

#include "fileform.h"
#include "forms.h"
#include "runforms.h"  // LiveFormInit, AccumFormInit
#include "omamenu.h"   // RunMenuItems[]
#include "curvedir.h"
#include "filestuf.h"
#include "tempdata.h"
#include "helpindx.h"
#include "di_util.h"
#include "ksindex.h"
#include "backgrnd.h"
#include "live.h"
#include "curvdraw.h"
#include "omameth.h"   // InitialMethod
#include "omaform.h"
#include "formwind.h"
#include "macnres.h"   // TempEntryName[]
#include "syserror.h"  // ERROR_OPEN
#include "omaerror.h"
#include "formtabs.h"
#include "crventry.h"
#include "plotbox.h"   // CopyPlotToMethod()

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

#define strclr(x)  (x[0] = '\0')

#define erase(ptr, type, count) memset(ptr, 0, (sizeof(type) * count))

#define USERFORM_ROW 11
#define FILES_DIRECTORY 0
#define CURVE_DIRECTORY 1
#define FILE_SPEC       2

#define CURVE_ACTION_LOAD   0
#define CURVE_ACTION_SAVE   1
#define CURVE_ACTION_DELETE 2
#define CURVE_ACTION_INSERT 3
#define CURVE_ACTION_RENAME 4

PRIVATE unsigned char RefreshFileScrollLogic(void);
PRIVATE unsigned char RefreshCurveScrollLogic(void);
PRIVATE void load_new_entry(void);

PRIVATE BOOLEAN curve_entry_init(int index);
PRIVATE BOOLEAN file_entry_init(int index);
PRIVATE BOOLEAN file_scroll_exit(unsigned int index);
PRIVATE BOOLEAN curve_scroll_exit(unsigned int index);
PRIVATE unsigned char do_filebox(void);
PRIVATE unsigned char do_curvescroll(void);
PRIVATE unsigned char do_filescroll(void);
PRIVATE unsigned char do_curvebox(void);
PRIVATE unsigned char display_only_setup(void);
PRIVATE unsigned char retain_focus_setup(void);
PRIVATE int reload_filesdir(void);
PRIVATE int GO_perform_user_request(void);
PRIVATE int ChangeDescription(void * field_data, char * field_string);
PRIVATE BOOLEAN FileDirFormInit(void);
PRIVATE BOOLEAN FileDirFormExit(void);
PRIVATE BOOLEAN UserFormInit(void);
PRIVATE BOOLEAN UserFormExit(void);

PRIVATE CURVEDIR FileCurveDir;

// information needed for display of a curve entry
typedef struct {
   char time_date[20];              // special format, null terminated
   char filename[_MAX_FNAME];       // null terminated
   char fileextension[_MAX_EXT];    // null terminated, no leading "."
   USHORT entry_start;              // StartIndex
   USHORT entry_count;              // count -- number of curves
   int entry_index;                 // index of the entry within the directory
   char  description[DESCRIPTION_LENGTH];  // entry description
} ENTRY_DISPLAY;

PRIVATE ENTRY_DISPLAY EntryImage;   // entry attributes for the current entry
PRIVATE ENTRY_DISPLAY TempImage;    // entry attributes for the current entry

static CURVEDIR *Curve_directory; // the user's curve directory
static CURVEDIR *File_directory;  // the user's file directory

// target directory, entry index, pointer to the target, and filespec of
// the target selected by the user from one of the scrolling curve
// directories.

// for file specifier display in dialog box, user edits this to do a rename
static char SourceFileSpec[DOSPATHSIZE + 1]; // all caps

// File specifier for the DOS Files scroll box.  Wild cards allowed.
// Initialize for all files in the current directory.
static char DOS_FileSpec[DOSPATHSIZE + 1] = ".\\*.*";

static int action = CURVE_ACTION_LOAD;

static int directory_of_origin;

static CURVEDIR *SourceDirectory;

PRIVATE char SourceDesc[DESCRIPTION_LENGTH];

static CURVE_ENTRY *SourceDirEntry;
PRIVATE USHORT SourceDirIndex = 0;
PRIVATE USHORT SourceStartCurve = 0;
PRIVATE USHORT SourceCurveCount = 0;

PRIVATE USHORT DestinationDirIndex = 0;
PRIVATE USHORT DestinationStartCurve = 0;

static USHORT WindowSelectBits;

static USHORT WindowBitFlags[8];

static COLOR_PAIR saved_shaded;

static struct save_area_info * SavedArea;
static BOOLEAN NoAction;
static BOOLEAN InUserForm = FALSE;
static BOOLEAN InFileDirForm = FALSE;

// dummy for select field return value
static int dummy_select_flag = FALSE;

static BOOLEAN DidDelete = FALSE;
static BOOLEAN DeleteAll = FALSE;
static PCHAR DeleteFileSpec = NULL;

char * DirActionOptions[] = {
   "Load Curve Set",
   "Save Curve Set",
   "Delete All or Part of Curve Set",
   "Insert Curve Set into Another Set",
   "Rename Curve Set"
};

char * XOnOffOptions[] = { " ", "X" };

#define COLORS_DEFAULT       0
#define COLORS_SUBFORM       1

// formed system uses global DataRegistry array

enum DATAREGISTRY_ACCESS { DGROUP_DO_STRINGS = 1, DGROUP_CODE,
                           DGROUP_SCRTEST };

static DATA DO_STRING_Reg[] = {
  {"File", 0, DATATYP_STRING, DATAATTR_PTR, 0 },                // 0
  {"Description", 0, DATATYP_STRING, DATAATTR_PTR, 0 },         // 1
  {"Start", 0, DATATYP_STRING, DATAATTR_PTR, 0 },               // 2
  {"Count", 0, DATATYP_STRING, DATAATTR_PTR, 0 },               // 3
  {"Date     Time", 0, DATATYP_STRING, DATAATTR_PTR, 0 },       // 4
  {" Curve Directory", 0, DATATYP_STRING, DATAATTR_PTR, 0 },    // 5
  {" DOS Files", 0, DATATYP_STRING, DATAATTR_PTR, 0 },          // 6
  {"FileSpec   ", 0, DATATYP_STRING, DATAATTR_PTR, 0 },         // 7
  {"Index", 0, DATATYP_STRING, DATAATTR_PTR, 0 },               // 8
  {"Action", 0, DATATYP_STRING, DATAATTR_PTR, 0 },              // 9
  {"Assign To Plot Window", 0, DATATYP_STRING, DATAATTR_PTR, 0}, // 10
  {"Source", 0, DATATYP_STRING, DATAATTR_PTR, 0 },              // 11
  {"1 2 3 4 5 6 7 8", 0, DATATYP_STRING, DATAATTR_PTR, 0 },     // 12
  {"Destination", 0, DATATYP_STRING, DATAATTR_PTR, 0 },         // 13
  {" Go ", 0, DATATYP_STRING, DATAATTR_PTR, 0 }                 // 14
};

// scrolling form init functions
// LOGIC functions
// VERIFY functions

static EXEC_DATA CODE_Reg[] = {
 {CAST_CHR2INT scroll_entry_field, 0, DATATYP_CODE, DATAATTR_PTR, 0},     //0
 {CAST_CHR2INT scroll_up_field,    0, DATATYP_CODE, DATAATTR_PTR, 0},     //1
 {CAST_CHR2INT scroll_down_field,  0, DATATYP_CODE, DATAATTR_PTR, 0},     //2
 {CAST_CHR2INT curve_entry_init,   0, DATATYP_CODE, DATAATTR_PTR, 0},     //3
 {CAST_CHR2INT file_entry_init,    0, DATATYP_CODE, DATAATTR_PTR, 0},     //4
 {NULL, 0, 0, 0, 0},                                                      //5
 {CAST_CHR2INT file_scroll_exit,   0, DATATYP_CODE, DATAATTR_PTR, 0},     //6
 {CAST_CHR2INT curve_scroll_exit,  0, DATATYP_CODE, DATAATTR_PTR, 0},     //7
 {NULL, 0, 0, 0, 0},                                                      //8
 {CAST_CHR2INT RefreshFileScrollLogic,  0, DATATYP_CODE, DATAATTR_PTR, 0},//9
 {CAST_CHR2INT RefreshCurveScrollLogic, 0, DATATYP_CODE, DATAATTR_PTR, 0},//10
 {             reload_filesdir,         0, DATATYP_CODE, DATAATTR_PTR, 0},//11
 {             GO_perform_user_request, 0, DATATYP_CODE, DATAATTR_PTR, 0},//12
 {             VerifyFileName,          0, DATATYP_CODE, DATAATTR_PTR, 0},//13
 {CAST_CHR2INT ChangeDescription,       0, DATATYP_CODE, DATAATTR_PTR, 0},//14
 {CAST_CHR2INT refresh_scroll_only,     0, DATATYP_CODE, DATAATTR_PTR, 0},//15
 {             FileDirFormInit, 0, DATATYP_CODE, DATAATTR_PTR, 0},        //16
 {             FileDirFormExit, 0, DATATYP_CODE, DATAATTR_PTR, 0},        //17
 {             UserFormInit,    0, DATATYP_CODE, DATAATTR_PTR, 0},        //18
 {             UserFormExit,    0, DATATYP_CODE, DATAATTR_PTR, 0},        //19
};

// use to eliminate warning message about & ignored, no & in this definition
#define ARRAY_offsetof(type,field) ((int)(((type *)0)->field))

// declarations here, definitions later
PRIVATE FORM FileBoxForm;
PRIVATE FORM CurveBoxForm;
PRIVATE FORM FileScrollForm;
PRIVATE FORM CurveScrollForm;
PRIVATE FORM UserForm;

static DATA SCRTEST_Reg[] = {
   {&EntryImage,
      ARRAY_offsetof(ENTRY_DISPLAY, description),              // 0
      DATATYP_STRING, DATAATTR_PTR_OFFS, 0 },
   {&EntryImage,
      ARRAY_offsetof(ENTRY_DISPLAY, filename),                 // 1
      DATATYP_STRING, DATAATTR_PTR_OFFS, 0 },
   {&EntryImage,
      ARRAY_offsetof(ENTRY_DISPLAY, time_date),                // 2
      DATATYP_STRING, DATAATTR_PTR_OFFS, 0 },
   {&EntryImage,
      offsetof(      ENTRY_DISPLAY, entry_count),              // 3
      DATATYP_INT,    DATAATTR_PTR_OFFS, 0 },
   {&EntryImage,
      ARRAY_offsetof(ENTRY_DISPLAY, fileextension),            // 4
      DATATYP_STRING, DATAATTR_PTR_OFFS, 0 },
   {&EntryImage,
      offsetof(      ENTRY_DISPLAY, entry_start),              // 5
      DATATYP_INT,    DATAATTR_PTR_OFFS, 0 },
   {&dummy_select_flag, 0, DATATYP_INT, DATAATTR_PTR, 0 },     // 6
   {DOS_FileSpec,    0, DATATYP_STRING, DATAATTR_PTR, 0 },     // 7
   {&EntryImage,
      offsetof(      ENTRY_DISPLAY, entry_index),              // 8
      DATATYP_INT,    DATAATTR_PTR_OFFS, 0 },
   {&SourceDirIndex,        0, DATATYP_INT, DATAATTR_PTR, 0 }, // 9
   {&SourceStartCurve,      0, DATATYP_INT, DATAATTR_PTR, 0 }, // 10
   {&SourceCurveCount,      0, DATATYP_INT, DATAATTR_PTR, 0 }, // 11
   {&DestinationDirIndex,   0, DATATYP_INT, DATAATTR_PTR, 0 }, // 12
   {&DestinationStartCurve, 0, DATATYP_INT, DATAATTR_PTR, 0 }, // 13
   {SourceDesc,          0, DATATYP_STRING, DATAATTR_PTR, 0 }, // 14
   {SourceFileSpec,      0, DATATYP_STRING, DATAATTR_PTR, 0 }, // 15
   {&action,                0, DATATYP_INT, DATAATTR_PTR, 0 }, // 16
   {&WindowBitFlags[0], 0, DATATYP_INT, DATAATTR_PTR, 0 },     // 17
   {&WindowBitFlags[1], 0, DATATYP_INT, DATAATTR_PTR, 0 },     // 18
   {&WindowBitFlags[2], 0, DATATYP_INT, DATAATTR_PTR, 0 },     // 19
   {&WindowBitFlags[3], 0, DATATYP_INT, DATAATTR_PTR, 0 },     // 20
   {&WindowBitFlags[4], 0, DATATYP_INT, DATAATTR_PTR, 0 },     // 21
   {&WindowBitFlags[5], 0, DATATYP_INT, DATAATTR_PTR, 0 },     // 22
   {&WindowBitFlags[6], 0, DATATYP_INT, DATAATTR_PTR, 0 },     // 23
   {&WindowBitFlags[7], 0, DATATYP_INT, DATAATTR_PTR, 0 },     // 24

   // these should really be in DGROUP_TOGGLE, but there is none here...
   {DirActionOptions, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 }, // 25
   {XOnOffOptions,    0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 }, // 26

   // these should really be in DGROUP_FORM, but there is none here...
   {&FileBoxForm,     0, DATATYP_VOID, DATAATTR_PTR, 0 },       // 27
   {&CurveBoxForm,    0, DATATYP_VOID, DATAATTR_PTR, 0 },       // 28
   {&FileScrollForm,  0, DATATYP_VOID, DATAATTR_PTR, 0 },       // 29
   {&CurveScrollForm, 0, DATATYP_VOID, DATAATTR_PTR, 0 },       // 30
   {&UserForm,        0, DATATYP_FORM, DATAATTR_PTR, 0 },       // 31
};


static enum {
   FLD_CURVE_BOX,
   FLD_REF_CS_LOGIC,
   FLD_FILE_BOX,
   FLD_REF_FS_LOGIC,
   LBL_FILESPEC,
   FLD_FILESPEC,
   FLD_FS_FORM,
   FLD_CS_FORM,
};

static FIELD ControlFields[] = {
   field_set(FLD_CURVE_BOX,
    FLDTYP_FORM,
    FLDATTR_NONE,
    KSI_NO_INDEX,
    CONTROL_HBASE + 0,        // 0
    DGROUP_SCRTEST, 28,       // CurveBoxForm
    0,0,
    0,0,
    0,0,
    0, 0, 0,
    FLD_REF_CS_LOGIC, FLD_REF_CS_LOGIC, FLD_REF_CS_LOGIC, FLD_REF_CS_LOGIC,
    FLD_REF_CS_LOGIC, FLD_REF_CS_LOGIC, FLD_REF_CS_LOGIC, FLD_REF_CS_LOGIC),

   field_set(FLD_REF_CS_LOGIC,
    FLDTYP_LOGIC,
    FLDATTR_NONE,
    KSI_NO_INDEX,
    CONTROL_HBASE + 1,        // 1
    DGROUP_CODE, 10,          // RefreshCurveScrollLogic
    0,0,
    0,0,
    0,0,
    1, 63, 1,
    FLD_FILE_BOX, FLD_FILE_BOX, FLD_FILE_BOX, FLD_FILE_BOX,
    FLD_FILE_BOX, FLD_FILE_BOX, FLD_FILE_BOX, FLD_FILE_BOX),

   field_set(FLD_FILE_BOX,
    FLDTYP_FORM,
    FLDATTR_NONE,
    KSI_NO_INDEX,
    CONTROL_HBASE + 2,
    DGROUP_SCRTEST, 27,
    0,0,
    0,0,                      //2
    0,0,                      // FileBoxForm
    0, 0, 0,
    FLD_REF_FS_LOGIC, FLD_REF_FS_LOGIC, FLD_REF_FS_LOGIC, FLD_REF_FS_LOGIC,
    FLD_REF_FS_LOGIC, FLD_REF_FS_LOGIC, FLD_REF_FS_LOGIC, FLD_REF_FS_LOGIC),

   field_set(FLD_REF_FS_LOGIC,
    FLDTYP_LOGIC,
    FLDATTR_NONE,
    KSI_NO_INDEX,
    CONTROL_HBASE + 3,        // 3
    DGROUP_CODE, 9,           // RefreshFileScrollLogic
    0,0,
    0,0,
    0,0,
    1, 63, 1,
    LBL_FILESPEC, LBL_FILESPEC, LBL_FILESPEC, LBL_FILESPEC,
    LBL_FILESPEC, LBL_FILESPEC, LBL_FILESPEC, LBL_FILESPEC),

   label_field(LBL_FILESPEC,
    DGROUP_DO_STRINGS, 7,     // "FileSpec : "
    10, 1, 11),

   field_set(FLD_FILESPEC,
    FLDTYP_STRING,
    FLDATTR_NONE,
    KSI_FD_FILESPEC,
    CONTROL_HBASE + 5,        // 5
    DGROUP_SCRTEST, 7,        // DOS_FileSpec
    0,0,
    DGROUP_CODE, 11,          // reload_filesdir()
    DOSPATHSIZE, 0,
    10, 12, 66,
    EXIT, FLD_FILESPEC, FLD_CS_FORM, FLD_FS_FORM,
    FLD_CS_FORM, FLD_FS_FORM, FLD_CS_FORM, FLD_FS_FORM),

   field_set(FLD_FS_FORM,
    FLDTYP_FORM,
    FLDATTR_NONE,
    KSI_NO_INDEX,
    CONTROL_HBASE + 6,        // 6
    DGROUP_SCRTEST, 29,       // FileScrollForm
    0,0,
    0,0,
    0,0,
    0, 0, 0,
    EXIT, FLD_FS_FORM, FLD_FILESPEC, FLD_CS_FORM,
    FLD_FILESPEC, FLD_CS_FORM, FLD_FILESPEC, FLD_CS_FORM),

   field_set(FLD_CS_FORM,
    FLDTYP_FORM,
    FLDATTR_NONE,
    KSI_NO_INDEX,
    CONTROL_HBASE + 7,        // 7
    DGROUP_SCRTEST, 30,       // CurveScrollForm
    0,0,
    0,0,
    0,0,
    0, 0, 0,
    EXIT, FLD_CS_FORM, FLD_FS_FORM, FLD_FILESPEC,
    FLD_FS_FORM, FLD_FILESPEC, FLD_FS_FORM, FLD_FILESPEC),
};

PRIVATE FORM ControlForm = {
   0, 0, FORMATTR_EXIT_RESTORE | FORMATTR_VISIBLE | FORMATTR_FULLSCREEN,
   0, 0, 0,
   2, 0, 21, 80,
   0, 0,
   { DGROUP_CODE, 16 },
   { DGROUP_CODE, 17 },
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(ControlFields) / sizeof(ControlFields[0]),
   ControlFields, KSI_FILE_DIR_FORM,
   0, DO_STRING_Reg, (DATA *)CODE_Reg, SCRTEST_Reg, 0, 0
};

static enum {
  LBL_DIRNAME,
  LBL_INDEX,
  LBL_FILE,
  LBL_DESC,
  LBL_START,
  LBL_COUNT,
  LBL_DATE };

static FIELD CurveTitles[] = {
   label_field(LBL_DIRNAME,
   DGROUP_DO_STRINGS, 5,
   0, 33, 17),

   label_field(LBL_INDEX,
   DGROUP_DO_STRINGS, 8,
   1, 1, 5),

   d_field_(LBL_FILE,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY | FLDATTR_RJ,
   0,
   DGROUP_DO_STRINGS, 0,
   1, 9, 6),

   label_field(LBL_DESC,
   DGROUP_DO_STRINGS, 1,
   1, 26, 14),

   label_field(LBL_START,
   DGROUP_DO_STRINGS, 2,
   1, 49, 5),

   label_field(LBL_COUNT,
   DGROUP_DO_STRINGS, 3,
   1, 55, 14),

   label_field(LBL_DATE,
   DGROUP_DO_STRINGS, 4,
   1, 63, 14),

};

PRIVATE FORM CurveBoxForm = {
   0, 0, FORMATTR_BORDER | FORMATTR_VISIBLE,
   0, 0, 0,
   0, 0, 10, 80,            // screen row 2
   0, 0, { 0, 0 }, { 0, 0 }, COLORS_SUBFORM, 0, 0, 0, 0,
   sizeof(CurveTitles) / sizeof(CurveTitles[0]),
   CurveTitles, KSI_NO_INDEX,
   0, DO_STRING_Reg, (DATA *)CODE_Reg, SCRTEST_Reg, 0, 0
};

static FIELD FileTitles[] = {
   label_field(LBL_DIRNAME,
   DGROUP_DO_STRINGS, 6,
   0, 36, 11),

   label_field(LBL_INDEX,
   DGROUP_DO_STRINGS, 8,
   1, 1, 5),

   d_field_(LBL_FILE,
   FLDTYP_STRING,
   FLDATTR_DISPLAY_ONLY | FLDATTR_RJ,
   0,
   DGROUP_DO_STRINGS, 0,
   1, 9, 6),

   label_field(LBL_DESC,
   DGROUP_DO_STRINGS, 1,
   1, 26, 14),

   label_field(LBL_START,
   DGROUP_DO_STRINGS, 2,
   1, 49, 5),

   label_field(LBL_COUNT,
   DGROUP_DO_STRINGS, 3,
   1, 55, 14),

   label_field(LBL_DATE,
   DGROUP_DO_STRINGS, 4,
   1, 63, 14),
};

PRIVATE FORM FileBoxForm = {
   0, 0, FORMATTR_BORDER | FORMATTR_VISIBLE,
   0, 0, 0,
   11, 0, 10, 80,           // screen row 13
   0, 0, { 0, 0 }, { 0, 0 }, COLORS_SUBFORM, 0, 0, 0, 0,
   sizeof(FileTitles) / sizeof(FileTitles[0]),
   FileTitles, KSI_NO_INDEX,
   0, DO_STRING_Reg, (DATA *)CODE_Reg, SCRTEST_Reg, 0, 0
};

static enum {
   FLD_CS_SCENTRY,
   FLD_CS_SCREF,
   FLD_CS_SCUP,
   FLD_CS_SCDOWN,
   FLD_CS_ENTRY,
   FLD_CS_NAME,
   FLD_CS_EXT,
   FLD_CS_START,
   FLD_CS_COUNT,
   FLD_CS_DATE,
   FLD_CS_DESC };

static FIELD CurveScrollFields[] = {
   field_set(FLD_CS_SCENTRY,
    FLDTYP_LOGIC,
    0,
    KSI_NO_INDEX,
    CURVESCROLL_HBASE + 0,    // 0  
    DGROUP_CODE, 0,           // scroll_entry_field()
    0,0,
    0,0,
    0,0,
    0, 0, 0,
    EXIT, FLD_CS_SCREF, FLD_CS_SCENTRY, FLD_CS_SCENTRY,
    FLD_CS_SCENTRY, FLD_CS_SCENTRY, FLD_CS_SCENTRY, FLD_CS_SCENTRY),

   field_set(FLD_CS_SCREF,
    FLDTYP_LOGIC,
    0,
    KSI_NO_INDEX,
    FILESCROLL_HBASE + 1,     // 1 
    DGROUP_CODE, 15,          // refresh_scroll_only()
    0,0,
    0,0,
    0,0,
    0, 0, 0,
    FLD_CS_DESC, EXIT, FLD_CS_SCREF, FLD_CS_SCREF,
    FLD_CS_SCREF, FLD_CS_SCREF, FLD_CS_SCREF, FLD_CS_SCREF),

   field_set(FLD_CS_SCUP,
    FLDTYP_LOGIC,
    0,
    KSI_NO_INDEX,
    CURVESCROLL_HBASE + 1,    // 2 
    DGROUP_CODE, 1,           // scroll_up_field()
    0,0,
    0,0,
    0,0,
    0, 0, 0,
    FLD_CS_SCENTRY, FLD_CS_ENTRY, FLD_CS_SCUP, FLD_CS_SCUP,
    FLD_CS_SCUP, FLD_CS_SCUP, FLD_CS_SCUP, FLD_CS_SCUP),

   field_set(FLD_CS_SCDOWN,
    FLDTYP_LOGIC,
    0,
    KSI_NO_INDEX,
    CURVESCROLL_HBASE + 2,    // 3 
    DGROUP_CODE, 2,           // scroll_down_field()
    0,0,
    0,0,
    0,0,
    0, 0, 0,
    FLD_CS_SCENTRY, FLD_CS_ENTRY, FLD_CS_SCDOWN, FLD_CS_SCDOWN,
    FLD_CS_SCDOWN, FLD_CS_SCDOWN, FLD_CS_SCDOWN, FLD_CS_SCDOWN),

   field_set(FLD_CS_ENTRY,
    FLDTYP_UNS_INT,
    FLDATTR_DISPLAY_ONLY | FLDATTR_RJ,
    KSI_NO_INDEX,
    0,                        // 4
    DGROUP_SCRTEST, 8,        // entry_index
    0,0,
    0,0,
    0,0,                      
    0, 0, 5,
    FLD_CS_NAME,FLD_CS_NAME, FLD_CS_ENTRY, FLD_CS_ENTRY,
    FLD_CS_ENTRY, FLD_CS_ENTRY, FLD_CS_ENTRY, FLD_CS_ENTRY),

   label_field(FLD_CS_NAME,
    DGROUP_SCRTEST, 1,
    0, 6, 8),

   label_field(FLD_CS_EXT,
    DGROUP_SCRTEST, 4,
    0, 15, 3),

   field_set(FLD_CS_START,
    FLDTYP_UNS_INT,
    FLDATTR_DISPLAY_ONLY | FLDATTR_RJ,
    KSI_NO_INDEX,
    0,                        // 7
    DGROUP_SCRTEST, 5,        // StartIndex
    0,0,
    0,0,
    0,0,                      
    0, 48, 5,
    FLD_CS_COUNT,FLD_CS_COUNT, FLD_CS_START, FLD_CS_START,
    FLD_CS_START, FLD_CS_START, FLD_CS_START, FLD_CS_START),

   field_set(FLD_CS_COUNT,
    FLDTYP_UNS_INT,
    FLDATTR_RJ | FLDATTR_DISPLAY_ONLY,
    KSI_NO_INDEX,
    0,                        // 8
    DGROUP_SCRTEST, 3,        // count
    0,0,
    0,0,
    0,0,                      
    0, 54, 5,
    FLD_CS_DATE,FLD_CS_DATE, FLD_CS_COUNT, FLD_CS_COUNT,
    FLD_CS_COUNT, FLD_CS_COUNT, FLD_CS_COUNT, FLD_CS_COUNT),

   field_set(FLD_CS_DATE,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY,
    KSI_NO_INDEX,
    0,                        // 9
    DGROUP_SCRTEST, 2,        // date / time
    0,0,
    0,0,
    0,0,                      
    0, 60, 17,
    FLD_CS_DESC, FLD_CS_DESC, FLD_CS_DATE, FLD_CS_DATE,
    FLD_CS_DATE, FLD_CS_DATE, FLD_CS_DATE, FLD_CS_DATE),

   field_set(FLD_CS_DESC,
    FLDTYP_SELECT,
    0,
    KSI_FD_FILE_CHOICE,
    CURVESCROLL_HBASE + 3,    // 10
    DGROUP_SCRTEST, 31,       // UserForm
    DGROUP_SCRTEST, 0,        // description
    0,0,
    0,0,                      
    0, 19, 28,
    EXIT, FLD_CS_DESC, FLD_CS_SCUP, FLD_CS_SCDOWN,
    EXIT, EXIT_DN, EXIT, EXIT_DN)
};

FORM CurveScrollForm = {
   0,                                         // SHORT  field_index;
   0,                                         // SHORT  previous_field_index;
   FORMATTR_SCROLLING | FORMATTR_VISIBLE,     // USHORT attrib;
   0,                                         // CHAR   next_field_offset;
   0,                                         // UCHAR  exit_key_code;
   0,                                         // UCHAR  status;
   2, 1, 7, 78,                               // UCHAR  row;
                                              // UCHAR  column;
                                              // UCHAR  size_in_rows;
                                              // UCHAR  size_in_columns;
   0,                                         // UCHAR  display_row_offset;
   0,                                         // USHORT virtual_row_index;
   { DGROUP_CODE, 3 }, // curve_entry_init()  //        init_function;
   { 0,0 },                                   //        exit_function;
   COLORS_DEFAULT,                            // UCHAR  color_set_index;
   0,                                         // UCHAR  string_cursor_offset;
   0,                                         // UCHAR  display_cursor_offset;
   0,                                         // UCHAR  field_char_count;
   0,                                         // UCHAR  field_overfull_flag;
   sizeof(CurveScrollFields) / sizeof(FIELD), // SHORT  number_of_fields;
   CurveScrollFields,                         // FIELD *fields;
   KSI_FD_CURVE_SCROLL,                       // SHORT  MacFormIndex;
   0, DO_STRING_Reg, (DATA *)CODE_Reg,
   SCRTEST_Reg, 0, 0
};

static enum {
   FLD_FS_SCENTRY,
   FLD_FS_SCREF,
   FLD_FS_SCUP,
   FLD_FS_SCDOWN,
   FLD_FS_ENTRY,
   FLD_FS_NAME,
   FLD_FS_EXT,
   FLD_FS_START,
   FLD_FS_COUNT,
   FLD_FS_DATE,
   FLD_FS_DESC };

static FIELD FileScrollFields[] = {

   field_set(FLD_FS_SCENTRY,
    FLDTYP_LOGIC,
    0,
    KSI_NO_INDEX,
    FILESCROLL_HBASE + 0,     // 0  
    DGROUP_CODE, 0 ,          // scroll_entry_field()
    0,0,
    0,0,
    0,0,
    0, 0, 0,
    EXIT, FLD_FS_SCREF, FLD_FS_SCENTRY, FLD_FS_SCENTRY,
    FLD_FS_SCENTRY, FLD_FS_SCENTRY, FLD_FS_SCENTRY, FLD_FS_SCENTRY),

   field_set(FLD_FS_SCREF,
    FLDTYP_LOGIC,
    0,
    KSI_NO_INDEX,
    FILESCROLL_HBASE + 1,     // 1 
    DGROUP_CODE, 15 ,         // refresh_scroll_only()
    0,0,
    0,0,
    0,0,
    0, 0, 0,
    FLD_FS_DESC, EXIT, FLD_FS_SCREF, FLD_FS_SCREF,
    FLD_FS_SCREF, FLD_FS_SCREF, FLD_FS_SCREF, FLD_FS_SCREF),

   field_set(FLD_FS_SCUP,
    FLDTYP_LOGIC,
    0,
    KSI_NO_INDEX,
    FILESCROLL_HBASE + 1,     // 2 
    DGROUP_CODE, 1 ,          // scroll_up_field()
    0,0,
    0,0,
    0,0,
    0, 0, 0,
    FLD_FS_SCENTRY, FLD_FS_ENTRY, FLD_FS_SCUP, FLD_FS_SCUP,
    FLD_FS_SCUP, FLD_FS_SCUP, FLD_FS_SCUP, FLD_FS_SCUP),

   field_set(FLD_FS_SCDOWN,
    FLDTYP_LOGIC,
    0,
    KSI_NO_INDEX,
    FILESCROLL_HBASE + 2,     // 3 
    DGROUP_CODE, 2 ,          // scroll_down_field()
    0,0,
    0,0,
    0,0,
    0, 0, 0,
    FLD_FS_SCENTRY, FLD_FS_ENTRY, FLD_FS_SCDOWN, FLD_FS_SCDOWN,
    FLD_FS_SCDOWN, FLD_FS_SCDOWN, FLD_FS_SCDOWN, FLD_FS_SCDOWN),

   field_set(FLD_FS_ENTRY,
    FLDTYP_UNS_INT,
    FLDATTR_DISPLAY_ONLY | FLDATTR_RJ,
    KSI_NO_INDEX,
    0,                        // 4
    DGROUP_SCRTEST, 8,        // entry_index
    0,0,
    0,0,
    0,0,                      
    0, 0, 5,
    FLD_FS_NAME, FLD_FS_NAME, FLD_FS_ENTRY, FLD_FS_ENTRY,
    FLD_FS_ENTRY, FLD_FS_ENTRY, FLD_FS_ENTRY, FLD_FS_ENTRY),

   label_field(FLD_FS_NAME,
   DGROUP_SCRTEST, 1,
   0, 6, 8),

   label_field(FLD_FS_EXT,
   DGROUP_SCRTEST, 4,
   0, 15, 3),

   field_set(FLD_FS_START,
    FLDTYP_UNS_INT,
    FLDATTR_DISPLAY_ONLY | FLDATTR_RJ,
    KSI_NO_INDEX,
    0,                        // 7
    DGROUP_SCRTEST, 5,        // StartIndex
    0,0,
    0,0,
    0,0,                      
    0, 48, 5,
    FLD_FS_COUNT, FLD_FS_COUNT, FLD_FS_START, FLD_FS_START,
    FLD_FS_START, FLD_FS_START, FLD_FS_START, FLD_FS_START ),

   field_set(FLD_FS_COUNT,
    FLDTYP_UNS_INT,
    FLDATTR_RJ | FLDATTR_DISPLAY_ONLY,
    KSI_NO_INDEX,
    0,                        // 8
    DGROUP_SCRTEST, 3,        // count
    0,0,
    0,0,
    0,0,                      
    0, 54, 5,
    FLD_FS_DATE, FLD_FS_DATE, FLD_FS_COUNT, FLD_FS_COUNT,
    FLD_FS_COUNT, FLD_FS_COUNT, FLD_FS_COUNT, FLD_FS_COUNT ),

   field_set(FLD_FS_DATE,
    FLDTYP_STRING,
    FLDATTR_DISPLAY_ONLY,
    KSI_NO_INDEX,
    0,                        // 9
    DGROUP_SCRTEST, 2,        // date time
    0,0,
    0,0,
    0,0,                      
    0, 60, 17,
    FLD_FS_DESC, FLD_FS_DESC, FLD_FS_DATE, FLD_FS_DATE,
    FLD_FS_DATE, FLD_FS_DATE, FLD_FS_DATE, FLD_FS_DATE ),

   field_set(FLD_FS_DESC,
    FLDTYP_SELECT,
    0,
    KSI_FD_FILE_CHOICE,
    FILESCROLL_HBASE + 3,     // 10
    DGROUP_SCRTEST, 31,       // UserForm
    DGROUP_SCRTEST, 0,        // description
    0,0,
    0,0,
    0, 19, 28,
    EXIT, FLD_FS_DESC, FLD_FS_SCUP, FLD_FS_SCDOWN,
    EXIT, EXIT_DN, EXIT, EXIT_DN)
};

FORM FileScrollForm = {
   0, 0,
   FORMATTR_SCROLLING | FORMATTR_VISIBLE,
   0, 0, 0,
   13, 1, 7, 78,
   0, 0,
   { DGROUP_CODE, 4 },
   { 0,0 },
   COLORS_DEFAULT,  // file_entry_init()
   0, 0, 0, 0,
   sizeof(FileScrollFields) / sizeof(FIELD),
   FileScrollFields,
   KSI_FD_FILE_SCROLL,
   0, DO_STRING_Reg, (DATA *)CODE_Reg, SCRTEST_Reg, 0, 0
};

// popup user dialog box for editing curve directories
static enum {
   LBL_USER_FILE,
   LBL_USER_DESC,
   LBL_USER_ACT,
   LBL_USER_INDX,
   LBL_USER_START,
   LBL_USER_COUNT,
   LBL_USER_ASSGN,
   LBL_USER_SRC,
   LBL_USER_NUMS,
   LBL_USER_DEST,

   FLD_USER_FNAME,
   FLD_USER_DESC,
   FLD_USER_ACT,
   FLD_USER_GO,
   FLD_USER_ENTRY,
   FLD_USER_START,
   FLD_USER_COUNT,
   FLD_USER_DEST_ENT,
   FLD_USER_DEST_START,
   FLD_USER_WINFLG1,
   FLD_USER_WINFLG2,
   FLD_USER_WINFLG3,
   FLD_USER_WINFLG4,
   FLD_USER_WINFLG5,
   FLD_USER_WINFLG6,
   FLD_USER_WINFLG7,
   FLD_USER_WINFLG8
};

FIELD UserFormFields[] = {

   label_field(LBL_USER_FILE,
   DGROUP_DO_STRINGS, 0,
   1, 9, 4),

   label_field(LBL_USER_DESC,
   DGROUP_DO_STRINGS, 1,
   2, 2, 11),

   label_field(LBL_USER_ACT,
   DGROUP_DO_STRINGS, 9,
   4, 7, 6),

   label_field(LBL_USER_INDX,
   DGROUP_DO_STRINGS, 8,
   6, 14, 5),

   label_field(LBL_USER_START,
   DGROUP_DO_STRINGS, 2,
   6, 21, 5),

   label_field(LBL_USER_COUNT,
   DGROUP_DO_STRINGS, 3,
   6, 28, 5),

   label_field(LBL_USER_ASSGN,
   DGROUP_DO_STRINGS, 10,
   6, 40, 21),

   label_field(LBL_USER_SRC,
   DGROUP_DO_STRINGS, 11,
   7, 7, 6),

   label_field(LBL_USER_NUMS,
   DGROUP_DO_STRINGS, 12,
   7, 40, 15),

   label_field(LBL_USER_DEST,
   DGROUP_DO_STRINGS, 13,
   8, 2, 11),

   field_set(FLD_USER_FNAME,
    FLDTYP_STRING,
    FLDATTR_REV_VID,
    KSI_FD_FILE_NAME,
    USERFORM_HBASE + 0,
    DGROUP_SCRTEST, 15,       /* file name */
    0, 0,
    DGROUP_CODE, 13,
    65, 0, /* (5:10) */
    1, 14, 64,
    EXIT, FLD_USER_FNAME, FLD_USER_DEST_ENT, FLD_USER_DESC,
    FLD_USER_FNAME, FLD_USER_FNAME, FLD_USER_WINFLG8, FLD_USER_DESC),

   field_set(FLD_USER_DESC,
    FLDTYP_STRING,
    FLDATTR_REV_VID,
    KSI_FD_DESC,
    USERFORM_HBASE + 1,
    DGROUP_SCRTEST, 14,      /* description */
    0, 0,
    DGROUP_CODE, 14,
    80, 0,/* (5:11) */
    2, 14, 64,
    EXIT, FLD_USER_DESC,  FLD_USER_FNAME, FLD_USER_ACT,
    FLD_USER_DESC, FLD_USER_DESC, FLD_USER_FNAME, FLD_USER_ACT),

   field_set(FLD_USER_ACT,
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_FD_ACTION,
    USERFORM_HBASE + 2,
    DGROUP_SCRTEST, 16,      /* action */
    DGROUP_SCRTEST, 25,
    0, 0,
    0, 5,  /* (5:12) */
    4, 14, 33,
    EXIT, FLD_USER_ACT,  FLD_USER_DESC, FLD_USER_START,
    FLD_USER_ACT, FLD_USER_GO, FLD_USER_DESC, FLD_USER_GO),

   field_set(FLD_USER_GO,
    FLDTYP_SELECT,
    FLDATTR_REV_VID,
    KSI_FD_GO,
    USERFORM_HBASE + 3,
    DGROUP_SCRTEST, 6,       /* go */
    DGROUP_DO_STRINGS, 14,
    DGROUP_CODE, 12,
    0, 0, /* (5:13) */
    4, 52, 4,
    EXIT, FLD_USER_GO, FLD_USER_DESC, FLD_USER_START,
    FLD_USER_ACT, FLD_USER_WINFLG1, FLD_USER_ACT, FLD_USER_START),

   field_set(FLD_USER_ENTRY,
    FLDTYP_UNS_INT,
    FLDATTR_RJ | FLDATTR_DISPLAY_ONLY | FLDATTR_NO_OVERFLOW_CHAR,
    KSI_NO_INDEX,
    0,
    DGROUP_SCRTEST, 9,
    0,0,                      /* source entry index */
    0,0,
    0,0,  /* (5:14) */
    7, 14, 5,
    EXIT, FLD_USER_ENTRY, FLD_USER_ENTRY, FLD_USER_ENTRY,
    FLD_USER_ENTRY, FLD_USER_ENTRY, FLD_USER_ENTRY, FLD_USER_ENTRY),

   field_set(FLD_USER_START,
    FLDTYP_UNS_INT,
    FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
    KSI_FD_SRC_START,
    USERFORM_HBASE + 4,
    DGROUP_SCRTEST, 10,      /* source start curve */
    0, 0,
    0, 0,
    0, 0, /* (5:15) */
    7, 21, 5,
    EXIT, FLD_USER_START, FLD_USER_ACT, FLD_USER_DEST_ENT,
    FLD_USER_ACT, FLD_USER_COUNT, FLD_USER_GO, FLD_USER_COUNT),

   field_set(FLD_USER_COUNT,
    FLDTYP_UNS_INT,
    FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
    KSI_FD_SRC_COUNT,
    USERFORM_HBASE + 5,
    DGROUP_SCRTEST, 11,      /* source curve count */
    0, 0,
    0, 0,
    0, 0, /* (5:16) */
    7, 28, 5,
    EXIT, FLD_USER_COUNT, FLD_USER_ACT, FLD_USER_DEST_ENT,
    FLD_USER_START, FLD_USER_GO, FLD_USER_START, FLD_USER_DEST_ENT),

   field_set(FLD_USER_DEST_ENT,
    FLDTYP_UNS_INT,
    FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
    KSI_FD_DST_INDEX,
    USERFORM_HBASE + 6,
    DGROUP_SCRTEST, 12,
    0, 0,                     /* destination entry index */
    0, 0,
    0, 0, /* (5:17) */
    8, 14, 5,
    EXIT, FLD_USER_DEST_ENT, FLD_USER_START, FLD_USER_FNAME,
    FLD_USER_DEST_ENT, FLD_USER_DEST_ENT, FLD_USER_COUNT, FLD_USER_DEST_START),

   field_set(FLD_USER_DEST_START,
    FLDTYP_UNS_INT,
    FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
    KSI_FD_DST_START,
    USERFORM_HBASE + 7,
    DGROUP_SCRTEST, 13,      /* destination start curve */
    0, 0,
    0, 0,
    0, 0, /* (5:18) */
    8, 21, 5,
    EXIT, FLD_USER_DEST_START, FLD_USER_START, FLD_USER_FNAME,
    FLD_USER_DEST_ENT, FLD_USER_WINFLG1, FLD_USER_DEST_ENT, FLD_USER_WINFLG1),

   field_set(FLD_USER_WINFLG1,
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_FD_WINDOW_1,
    USERFORM_HBASE + 8,
    DGROUP_SCRTEST, 17,      /* window flags */
    DGROUP_SCRTEST, 26,
    0, 0,
    0, 2,  /* (5:19) */
    8, 40, 1,
    EXIT, FLD_USER_WINFLG1, FLD_USER_START, FLD_USER_FNAME,
    FLD_USER_COUNT, FLD_USER_WINFLG2, FLD_USER_DEST_START, FLD_USER_WINFLG2),

   field_set(FLD_USER_WINFLG2,
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_FD_WINDOW_2,
    USERFORM_HBASE + 9,
    DGROUP_SCRTEST, 18,
    DGROUP_SCRTEST, 26,
    0, 0,
    0, 2,  /* (5:20) */
    8, 42, 1,
    EXIT, FLD_USER_WINFLG2, FLD_USER_START, FLD_USER_FNAME,
    FLD_USER_COUNT, FLD_USER_WINFLG3, FLD_USER_WINFLG1, FLD_USER_WINFLG3),

   field_set(FLD_USER_WINFLG3,
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_FD_WINDOW_3,
    USERFORM_HBASE + 10,
    DGROUP_SCRTEST, 19,
    DGROUP_SCRTEST, 26,
    0, 0,
    0, 2,  /* (5:21) */
    8, 44, 1,
    EXIT, FLD_USER_WINFLG3, FLD_USER_START, FLD_USER_FNAME,
    FLD_USER_WINFLG2, FLD_USER_WINFLG4,  FLD_USER_WINFLG2, FLD_USER_WINFLG4),

   field_set(FLD_USER_WINFLG4,
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_FD_WINDOW_4,
    USERFORM_HBASE + 11,
    DGROUP_SCRTEST, 20,
    DGROUP_SCRTEST, 26,
    0, 0,
    0, 2,  /* (5:22) */
    8, 46, 1,
    EXIT, FLD_USER_WINFLG4, FLD_USER_START, FLD_USER_FNAME,
    FLD_USER_WINFLG3, FLD_USER_WINFLG5,  FLD_USER_WINFLG3, FLD_USER_WINFLG5),

   field_set(FLD_USER_WINFLG5,
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_FD_WINDOW_5,
    USERFORM_HBASE + 12,
    DGROUP_SCRTEST, 21,
    DGROUP_SCRTEST, 26,
    0, 0,
    0, 2,  /* (5:23) */
    8, 48, 1,
    EXIT, FLD_USER_WINFLG5, FLD_USER_START, FLD_USER_FNAME,
    FLD_USER_WINFLG4, FLD_USER_WINFLG6,  FLD_USER_WINFLG4, FLD_USER_WINFLG6),

   field_set(FLD_USER_WINFLG6,
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_FD_WINDOW_6,
    USERFORM_HBASE + 13,
    DGROUP_SCRTEST, 22,
    DGROUP_SCRTEST, 26,
    0, 0,
    0, 2,  /* (5:24) */
    8, 50, 1,
    EXIT, FLD_USER_WINFLG6, FLD_USER_START, FLD_USER_FNAME,
    FLD_USER_WINFLG5, FLD_USER_WINFLG7,  FLD_USER_WINFLG5, FLD_USER_WINFLG7),

   field_set(FLD_USER_WINFLG7,
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_FD_WINDOW_7,
    USERFORM_HBASE + 14,
    DGROUP_SCRTEST, 23,
    DGROUP_SCRTEST, 26,
    0, 0,
    0, 2,  /* (5:25) */
    8, 52, 1,
    EXIT, FLD_USER_WINFLG7, FLD_USER_START, FLD_USER_FNAME,
    FLD_USER_WINFLG6, FLD_USER_WINFLG8,  FLD_USER_WINFLG6, FLD_USER_WINFLG8),

   field_set(FLD_USER_WINFLG8,
    FLDTYP_TOGGLE,
    FLDATTR_REV_VID,
    KSI_FD_WINDOW_8,
    USERFORM_HBASE + 15,
    DGROUP_SCRTEST, 24,
    DGROUP_SCRTEST, 26,
    0, 0,
    0, 2,  /* (5:26) */
    8, 54, 1,
    EXIT, FLD_USER_WINFLG8, FLD_USER_START, FLD_USER_FNAME,
    FLD_USER_WINFLG7, FLD_USER_GO,  FLD_USER_WINFLG7, FLD_USER_FNAME),
};

FORM  UserForm = {
   0, 0,
   FORMATTR_BORDER | FORMATTR_INDEP | FORMATTR_VISIBLE | FORMATTR_FULLWIDTH,
   0, 0, 0,
   USERFORM_ROW, 0, 10, 80,
   0, 0,
   { DGROUP_CODE, 18 },
   { DGROUP_CODE, 19 },
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(UserFormFields) / sizeof(FIELD),
   UserFormFields,
   KSI_FD_USER,
   0, DO_STRING_Reg, (DATA *)CODE_Reg, SCRTEST_Reg, 0, 0
};

//=========================================================================

static char * DataFileDeletionPrompt[] =
  {
  "This option may irretrievably delete ",
  "the entire data file. ",
  "Continue?",
  NULL
  };

// return TRUE iff name is a special file name.  The special files
// are : background, temp0, temp1 ..., lastlive, tagwin1 ... tagwin8
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN is_special_name(char * name)
{
   // for now, just check for background, temp, and lastlive files
   if(!stricmp(name, BackGroundEntryName)) return TRUE;
   if(!strnicmp(name, TempEntryName, 4))  return TRUE;
   if(!stricmp(name, LastLiveEntryName))  return TRUE;
   return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// static BOOLEAN entry_init(int index, CURVEDIR *directory)
//
// Function  : sets up global data for one entry of a directory scroll box
//
// Arguments : directory is the curve directory to use
//             index indicates which entry of the directory to use
//
// Returns   : FALSE if index is less than the number of entries in the
//             directory, else TRUE.
//
// Side Effects : sets file globals : time_date, filename, filepath
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN entry_init(int index, CURVEDIR *directory)
{
  char        filedrive[_MAX_DRIVE];   // for _splitpath(), not used
  char        filedir  [_MAX_FNAME];   // for _splitpath(), not used
  CURVE_ENTRY *entry_ptr;              // entry to be displayed
  BOOLEAN     IndexFudged = FALSE;

  // this is now possible if scroll form was previously emptied
  if (index == -1)
    {
    index = 0;
    IndexFudged = TRUE;
    }

  if ((directory->BlkCount > 0) && (index < (SHORT) directory->BlkCount))
    {
    entry_ptr =  (directory->Entries) + index;
    // set up time/date string for this entry
    sprintf(EntryImage.time_date, "%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d",
            entry_ptr->time.tm_mon + 1,
            entry_ptr->time.tm_mday,
            entry_ptr->time.tm_year,
            entry_ptr->time.tm_hour,
            entry_ptr->time.tm_min,
            entry_ptr->time.tm_sec);
    // set up file name and path for this entry
    _splitpath(entry_ptr->name, filedrive, filedir, EntryImage.filename,
      EntryImage.fileextension);

    if(EntryImage.fileextension[0] == '.') // remove leading '.'
      {
      USHORT i;

      for(i = 1; i < _MAX_EXT; i ++)
        EntryImage.fileextension[i - 1] = EntryImage.fileextension[i];
      }
    EntryImage.entry_start = entry_ptr->StartIndex;
    EntryImage.entry_count = entry_ptr->count;

    // WARNING : desc points to the actual curve entry string, NOT a copy

    // ASSUME each special file already has its description set up so that
    // it is sufficient just to copy it.

    strncpy(EntryImage.description, entry_ptr->descrip, DESCRIPTION_LENGTH);
    EntryImage.description[DESCRIPTION_LENGTH - 1] = '\0';

    EntryImage.entry_index = index;
    return(IndexFudged);                 // all OK
    }
  return(TRUE);                          // not OK
}

////////////////////////////////////////////////////////////////////////////
// BOOLEAN curve_entry_init(int index)
//
// Function  : This is the "scroll_entry_field" function for the curve
//             directory scroll box.  Called by the formed system before
//             each entry is displayed.
//
// Arguments : index indicates which entry of the directory to use
//
// Returns   : same as entry_init()
//
// Side Effects : sends global Curve_directory to entry_init()
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

BOOLEAN curve_entry_init(int index)
{
  return entry_init(index, Curve_directory);
}

////////////////////////////////////////////////////////////////////////////
// BOOLEAN file_entry_init(int index)
//
// Function  : This is the "scroll_entry_field" function for the file
//             directory scroll box.  Called by the formed system before
//             each entry is displayed.
//
// Arguments : index indicates which entry of the directory to use
//
// Returns   : same as entry_init()
//
// Side Effects : sends global File_directory to entry_init()
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

BOOLEAN file_entry_init(int index)
{
   return entry_init(index, File_directory);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN file_scroll_exit(unsigned int index)
{
   if ((directory_of_origin == FILES_DIRECTORY)
      && (File_directory->BlkCount > 0))
   {
//      show_active_field(Current.Form, FLD_FS_DESC);
   }
   return(FALSE);
   index;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN curve_scroll_exit(unsigned int index)
{
   if ((directory_of_origin == CURVE_DIRECTORY)
      && (Curve_directory->BlkCount > 0))
   {
//      show_active_field(Current.Form, FLD_CS_DESC);
   }
   return(FALSE);
   index;
}

////////////////////////////////////////////////////////////////////////////
// unsigned char do_filebox(void)
//
// Function  : logic field function, run the FileBox form
//
// Arguments : used by formed system
//
// Returns   : always returns zero
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned char do_filebox(void)
{
   run_form(&FileBoxForm, &default_form_attributes, FALSE);
   return 0;
}

////////////////////////////////////////////////////////////////////////////
// unsigned char do_curvebox(void)
//
// Function  : logic field function, run the FileBox form
//
// Arguments : used by formed system
//
// Returns   : always returns zero
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned char do_curvebox(void)
{
   run_form(&CurveBoxForm, &default_form_attributes, FALSE);
   return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void RefreshCurveScrollBox(void)
{
   int   field_index;

   field_index = -1; /* don't draw highlighted active field */

   reset_bit(CurveScrollForm.fields[FLD_CS_DESC].attrib, FLDATTR_HIGHLIGHT);

   redraw_scroll_form(&CurveScrollForm, CurveScrollForm.virtual_row_index,
   field_index);

   reset_bit(CurveScrollForm.fields[FLD_CS_DESC].attrib, FLDATTR_HIGHLIGHT);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE unsigned char RefreshCurveScrollLogic(void)
{
   redraw_scroll_form(&CurveScrollForm, CurveScrollForm.virtual_row_index,
   -1); /* don't draw highlighted active field */
   return(EXIT_DEFAULT);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void RefreshFileScrollBox(void)
{
   int   field_index;

   field_index = -1; /* don't draw highlighted active field */

   reset_bit(FileScrollForm.fields[FLD_FS_DESC].attrib, FLDATTR_HIGHLIGHT);

   redraw_scroll_form(&FileScrollForm, FileScrollForm.virtual_row_index,
   field_index);

   reset_bit(FileScrollForm.fields[FLD_FS_DESC].attrib, FLDATTR_HIGHLIGHT);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE unsigned char RefreshFileScrollLogic(void)
{
   redraw_scroll_form(&FileScrollForm, FileScrollForm.virtual_row_index,
   -1); /* don't draw highlighted active field */
   return(EXIT_DEFAULT);
}

////////////////////////////////////////////////////////////////////////////
// static void clearFileDir(void)
//
// Function  : deletes all the entries from the DOS file curve directory
//
// Side Effects : DOS file curve directory is emptied
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void clearFileDir(void)
{
   USHORT entryIndex;

   // empty the current directory, need all new curve entries
   // delete one entry at a time starting with the last one
   for(entryIndex = File_directory->BlkCount - 1;
        File_directory->BlkCount > 0; entryIndex--)
   {
      if(DelCurveBlocks(&entryIndex, 1, File_directory))
         //error, failed to delete curve entry
         File_directory->BlkCount--; // try to delete the next entry
   }
}

////////////////////////////////////////////////////////////////////////////
// static void new_files(const PCHAR new_file_spec, USHORT sort_order)
//
// Function  : // Reload the files curve directory.
//
// Arguments : new_file_spec describes which DOS files to load.  It may
//   contain wild card characters.  sort_order specifies how the newly
//   loaded curve directory should be sorted.
//
// Side Effects : Reloads the DOS files curve directory and sorts it
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void new_files(const PCHAR new_file_spec, USHORT sort_order)
{
   char FileDrive[_MAX_DRIVE + 1];
   char FileDir  [_MAX_DIR   + 1];
   char FileName [_MAX_FNAME + 1];
   char FileExt  [_MAX_EXT   + 1];
   char TheName  [_MAX_FNAME + _MAX_EXT + 1]; // FileName  + FileExt
   char TheDir   [_MAX_DRIVE + _MAX_DIR + 1]; // FileDrive + FileDir

   clearFileDir(); // delete all curve entries from DOS files curve dir.

   _splitpath(new_file_spec, FileDrive, FileDir, FileName, FileExt);

   strncpy(TheName, FileName,  _MAX_FNAME);
   strncat(TheName, FileExt,   _MAX_EXT  );
   strncpy(TheDir , FileDrive, _MAX_DRIVE);
   strncat(TheDir , FileDir,   _MAX_DIR  );

   if(! FillDirListInfo(TheName, TheDir, File_directory))
      SortCurveDir(File_directory, sort_order);
}

////////////////////////////////////////////////////////////////////////////
// int reload_filesdir(void * field_data, char * field_string)
// Function  : empties and then reloads the File_directory. Displays it.
//
// Arguments : field_data, field_string not used
//
// Returns   : always returns FIELD_VALIDATE_SUCCESS
//
// Side Effects : uses DOS_FileSpec to specify which files to load into
//                File_directory
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int reload_filesdir(void)
{
  CHAR TempPath[DOSPATHSIZE + 1];
  CHAR TempName[DOSFILESIZE + 1];
  CHAR TempFileSpec[FNAME_LENGTH];
  CHAR *pChar;
  CHAR OldChar;
  BOOLEAN Slash = TRUE;
  SHORT StrLen;

//  clearFileDir();  // clear out all entries from File_directory

  StrLen = strlen(DOS_FileSpec);
  /* if only a directory, change to show all files */
  if((StrLen == 0) ||
     ((StrLen == 2) && (DOS_FileSpec[StrLen-1] == ':')) ||
     (DOS_FileSpec[StrLen-1] == '\\'))
    strcat(DOS_FileSpec, "*.*");
  else if (! chdir(DOS_FileSpec))
    strcat(DOS_FileSpec, "\\*.*");

  strcpy(TempFileSpec, DOS_FileSpec);
  pChar = strrchr(TempFileSpec, '\\');
  if (pChar == NULL)
    {
    Slash = FALSE;
    if ((pChar = strrchr(TempFileSpec, ':')) == NULL)
      pChar = TempFileSpec;
    else
      {
      pChar++;
      if (*pChar == '\0')
        pChar[1] = '\0';
      }
    }
  OldChar = *pChar;
  *pChar = '\0';

  if (ParsePathAndName(TempPath, TempName, TempFileSpec) == 0)
    return FIELD_VALIDATE_WARNING;
  if (! Slash)  // if no occurrance of '\\'
    *pChar = OldChar;
  else
    pChar++;

  strcpy(DOS_FileSpec, TempPath);
  strcat(DOS_FileSpec, pChar);

  // reload new entries into File_directory
  new_files(DOS_FileSpec, SORT_NAME | SORT_ASCEND);

  do_filebox();

  if (Current.Form == &ControlForm)   // don't put in highlight if using as
    directory_of_origin = FILE_SPEC; // verify function for the file spec

  RefreshFileScrollBox();
  return FIELD_VALIDATE_SUCCESS;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void expand_window_bits_for_toggle(void)
{
  int               i;
  unsigned int      bit = 1;

  for (i=0; i<8; i++)
    {
    if (WindowSelectBits & bit)
      WindowBitFlags[i] = 1;
    else
      WindowBitFlags[i] = 0;

    bit = (bit << 1);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void gather_window_bits_from_toggle(void)
{
  int               i;
  unsigned int      bit = 1;

  WindowSelectBits = 0;

  for (i=0; i<8; i++)
    {
    if (WindowBitFlags[i])
      WindowSelectBits |= bit;

    bit = (bit << 1);
    }
}

////////////////////////////////////////////////////////////////////////////
// Given that SourceDirectory has already been
// set, put the correct values into SourceDirEntry and SourceFileSpec.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void SelectSourceIndex(SHORT new_index)
{
   if (new_index >= (SHORT) SourceDirectory->BlkCount)
      new_index = SourceDirectory->BlkCount - 1;

   SourceDirIndex = new_index;
   if (SourceDirIndex == -1)
      return;

   SourceDirEntry = &SourceDirectory->Entries[SourceDirIndex];

   if (directory_of_origin == CURVE_DIRECTORY)
      WindowSelectBits = SourceDirEntry->DisplayWindow;
   else
      WindowSelectBits = 1;

   expand_window_bits_for_toggle();

   SourceStartCurve = SourceDirEntry->StartIndex;
   SourceCurveCount = SourceDirEntry->count;
   DestinationStartCurve = SourceStartCurve;
   DestinationDirIndex = Curve_directory->BlkCount;

   strncpy(SourceDesc, SourceDirEntry->descrip, DESCRIPTION_LENGTH);
   SourceDesc[DESCRIPTION_LENGTH - 1] = '\0';

   // concatenate the name and path entries from the target
   strclr(SourceFileSpec);
   strncat(SourceFileSpec, SourceDirEntry->path, _MAX_DRIVE + _MAX_DIR);
   strncat(SourceFileSpec, SourceDirEntry->name, _MAX_FNAME + _MAX_EXT);
}

////////////////////////////////////////////////////////////////////////////
// setup SourceDirectory and SourceDirIndex to indicate which curve
// entry the user has selected to work with.  Popup the user dialog box to
// allow the user to operate on/with the selected target.  Restore the
// screen when done by redrawing the DOS files directory form.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN UserFormInit(void)
{
  if (!InUserForm)
    {
    InUserForm = TRUE;

    if(Current.PreviousStackedContext->Form == &CurveScrollForm)
      {
      directory_of_origin = CURVE_DIRECTORY;
      SourceDirectory = Curve_directory;
      }
    else
      {
      directory_of_origin = FILES_DIRECTORY;
      SourceDirectory = File_directory;
      }

    UserForm.row = ControlForm.row + (CHAR)USERFORM_ROW;

    SavedArea = save_screen_area(UserForm.row, UserForm.column,
                                 UserForm.size_in_rows,
                                 UserForm.size_in_columns);

    FileBoxForm.attrib    &= ~FORMATTR_VISIBLE;
    FileScrollForm.attrib &= ~FORMATTR_VISIBLE;

    NoAction = TRUE;  // will be reset if GO is pressed

    if (SourceDirectory->BlkCount == 0)
      return TRUE; // always succeed

    TempImage = EntryImage;
    SelectSourceIndex(EntryImage.entry_index);

    // turn off scroll attribute so that mouse use in UserForm can't
    // accidently activate this scroll form

    FileScrollForm.attrib &= ~FORMATTR_SCROLLING;

    }

  return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN UserFormExit(void)
{
  PCHAR pDesc;
  ERR_OMA err;

  if (!DidDelete && (directory_of_origin == CURVE_DIRECTORY))
    {
    gather_window_bits_from_toggle();
    SourceDirEntry->DisplayWindow = WindowSelectBits;
    }
  else if (DeleteAll)
    {
    if (directory_of_origin == CURVE_DIRECTORY)
      {
      pDesc = SourceDirEntry->descrip;

      err = DelTempFileBlk(Curve_directory, SourceDirIndex);
      }
    else
      {
      if(remove(DeleteFileSpec))
        error(ERROR_WRITE, DeleteFileSpec);
      free(DeleteFileSpec);
      }
    }

  FileBoxForm.attrib    |= FORMATTR_VISIBLE;
  FileScrollForm.attrib |= FORMATTR_VISIBLE;

  if (NoAction && (SavedArea != NULL))
    restore_screen_area(SavedArea);

  else if ((Current.Form->status != FORMSTAT_EXIT_ALL_FORMS) &&
    (Current.Form->status != FORMSTAT_SWITCH_MODE))
    {
    reload_filesdir();
    EntryImage = TempImage;
    }

  //  Must be after reload_filesdir call
  if (directory_of_origin == FILES_DIRECTORY)
    {
    SelectSourceIndex(SourceDirIndex);
    }
  /* SourceDirIndex may be -1 if deleted all curves in memory */
  if (SourceDirIndex < Curve_directory->BlkCount)
    {
    strncpy(EntryImage.description, SourceDirEntry->descrip,
      DESCRIPTION_LENGTH);
    EntryImage.description[DESCRIPTION_LENGTH - 1] = '\0';
    }
  else
    {
    if (Curve_directory->BlkCount)
      SourceDirIndex--;
    else
      SourceDirIndex = 0;
    }

  if (Curve_directory->BlkCount == 0)
    CurveScrollForm.virtual_row_index = -1;

  if (DidDelete)
    {
    /* don't draw highlighted active field */
    reset_bit(CurveScrollForm.fields[FLD_CS_DESC].attrib, FLDATTR_HIGHLIGHT);
    redraw_scroll_form(&CurveScrollForm, CurveScrollForm.virtual_row_index, -1);
    }

  DidDelete = FALSE;
  DeleteAll = FALSE;
  FileScrollForm.attrib |= FORMATTR_SCROLLING;
  InUserForm = FALSE;

  return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// delete function from user dialog box.  If the target is a file, then
// delete the file and redisplay the DOS files directory.  If the target
// is an entry in the curve directory, then delete the curves in the
// range SourceStartCurve .. SourceCurveCount.  If all of the curves of an
// entry are deleted, then delete the entry.  Redisplay the curve directory.
// Pick a new target and show it in the dialog box.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN delete_entry(void)
{
  FILE *fhnd;
  BOOLEAN ReturnVal = TRUE;
  CHAR FileType;

  DidDelete = TRUE;

  if(directory_of_origin == CURVE_DIRECTORY)
    {
    // delete from curve directory
    NoAction = TRUE;
    // verify that from/to numbers are within the range of the Target
    // correct error test condition
    if(SourceStartCurve < SourceDirEntry->StartIndex)
      {
      error(ERROR_CURVE_NUM, SourceStartCurve);
      return FALSE;
      }

    if(SourceStartCurve+SourceCurveCount >
      SourceDirEntry->StartIndex + SourceDirEntry->count)
      {
      error(ERROR_CURVE_NUM, SourceStartCurve + SourceCurveCount);
      return FALSE;
      }

    // delete the entire entry

    if((SourceStartCurve == SourceDirEntry->StartIndex)
      && (SourceCurveCount == SourceDirEntry->count))
        {
        Current.Form->status = FORMSTAT_EXIT_THIS_FORM;
        DeleteAll = TRUE;
        if(SourceDirIndex == (USHORT)BackGroundIndex)
          {
          DoAccum ? AccumFormInit() : LiveFormInit();
          RunMenuItems[2].Control |= MENUITEM_INACTIVE;
          RunMenuItems[5].Control |= MENUITEM_INACTIVE;
          }
        }
      else
        {
        if (is_special_name(SourceDirEntry->name))
          {
          error(ERROR_NO_SPECIAL_DELETE, SourceDirEntry->name);
          return(FALSE);
          }
        else if((SourceStartCurve + SourceCurveCount ==
          SourceDirEntry->StartIndex + SourceDirEntry->count) ||
          (SourceStartCurve == SourceDirEntry->StartIndex))
          {
          // delete a block of curves from the end of the entry
          DelMultiTempCurve(Curve_directory, SourceDirIndex,
            SourceStartCurve, SourceCurveCount);

          // set start index of the entry to the new starting curve number
          // in case it changed
          if(SourceStartCurve == SourceDirEntry->StartIndex)
            SourceDirEntry->StartIndex = SourceStartCurve + SourceCurveCount;
          SelectSourceIndex(EntryImage.entry_index);
          }
        else
          {
          // delete a block of curves from the middle of the entry
          // use SplitCurveBlk() from curvedir.c to form two entries
          if(SplitCurveBlk(SourceDirIndex, SourceStartCurve, Curve_directory))
            return TRUE; // error

          // now delete curves from the beginning of the newly added entry
          // new entry created by SplitCurveBlk()
          DelMultiTempCurve(Curve_directory, SourceDirIndex + 1,
            SourceStartCurve, SourceCurveCount);

          SourceDirEntry++;
          SourceDirEntry->StartIndex = SourceStartCurve + SourceCurveCount;
          SelectSourceIndex(EntryImage.entry_index + 1);
          }
        }

      // flag the lack of entries
      if(Curve_directory->BlkCount == 0)
        CurveScrollForm.virtual_row_index = -1;

      }
    else /* FILE_DIRECTORY */
      {
      /* delete the curves in the target file from the DOS files directory */
      /* if all curves deleted then the whole file is erased */

      /* put up data deletion warning exit on escape or NO */
      if(yes_no_choice_window(DataFileDeletionPrompt, 0,
        COLORS_MESSAGE) != YES)
        return TRUE;

      NoAction = FALSE;

      if((fhnd = fopen(SourceFileSpec, "r+b")) == 0)
        {
        error(ERROR_OPEN, SourceFileSpec);
        return FALSE;
        }

      FileType = IDFileType(fhnd);

      if((FileType == OMA4DATA) && (SourceDirEntry->count > 0) &&
        SourceDirEntry->count != SourceCurveCount)
        {
        DestinationStartCurve = SourceDirEntry->count;
        MultiFileCurveDelete(fhnd, SourceFileSpec, SourceStartCurve,
          SourceCurveCount);
        ReturnVal = FALSE;
        }
      else if(FileType == OMA4V11DATA || FileType == OMA4V11DATA)
        {
        error(ERROR_OLD_VERSION, SourceFileSpec);
        }
      else /* if deleting non-OMA file or all curves in file */
        {
        DeleteFileSpec = strdup(SourceFileSpec);
        ReturnVal = FALSE;     /* FALSE means don't redraw */
        DeleteAll = TRUE;      /* since delete action will redraw anyhow */
        Current.Form->status = FORMSTAT_EXIT_THIS_FORM;
        }

      fclose(fhnd);
      // flag the lack of entries
      if(File_directory->BlkCount == 0)
        FileScrollForm.virtual_row_index = -1;

      }
    return ReturnVal;      // set redraw flag
}

////////////////////////////////////////////////////////////////////////////
// If the target directory is the file directory, then rename the target
// file to SourceFileSpec using DOS rename().  If the target directory is
// the curve directory, then change the name and path fields.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE ERR_OMA rename_just_name(void)
{
  char   NewFileName[_MAX_FNAME + _MAX_EXT + 1];
  char   CurrentFileSpec[DOSPATHSIZE + 1];
  char   FileDrive[_MAX_DRIVE + 1];
  char   FileDir[_MAX_DIR + 1];
  char   FileName[_MAX_FNAME + 1];
  char   FileExt[_MAX_EXT + 1];
  char   Buffer[DOSPATHSIZE + 1];
  SHORT  Dummy;
  USHORT TestStart, TestCount;

  strupr(SourceFileSpec);
  _splitpath(SourceFileSpec, FileDrive, FileDir, FileName, FileExt);
  strncpy(NewFileName, FileName, _MAX_FNAME);
  strncat(NewFileName, FileExt, _MAX_EXT);

  strncpy(Buffer, FileDrive, _MAX_DRIVE);
  strncat(Buffer, FileDir,   _MAX_DIR  );
  strupr(Buffer);             // upper case

  // if NOT the same, rename!
  if(stricmp(SourceDirEntry->name, NewFileName) ||
    stricmp(SourceDirEntry->path, Buffer))
    {
    if (directory_of_origin == CURVE_DIRECTORY)
      {
      // change name and path
      // Do not allow renaming an entry TO one of the special file names.
      // Renaming FROM a special file is OK.

      if(!is_special_name(NewFileName))
        {
        TestStart = SourceDirEntry->StartIndex;
        TestCount = SourceDirEntry->count;

        // test for unique name, path, and curve number
        switch (CheckCurveBlkOverLap(Buffer, NewFileName,
                TestStart, TestStart + TestCount - 1,
                (PSHORT) &Dummy))
          {
          case RANGEOK:
          case SPLITRANGE:
          case OVERLAPCAT:
            return error(ERROR_NOT_UNIQUE_CURVE);
          case NOOVERLAPCAT:
          case DISJOINT:
          case BADNAME:
          break;            // OK a unique name and range was given
          }

        strcpy(SourceDirEntry->path, Buffer);

        strcpy(SourceDirEntry->name, NewFileName);
        // redraw the curve directory because it has changed
        if (!(SourceDirEntry->EntryType == OMA4MEMDATA))
          RefreshCurveScrollBox();
        }
      else  // cannot overwrite special file.
        {
        return error(ERROR_NO_SPECIAL_OVERWRITE, SourceDirEntry->name);
        }
      }
    else // do a DOS rename, then reload the files directory and redisplay
      {
      NoAction = FALSE;

      strncpy(CurrentFileSpec, SourceDirEntry->path, _MAX_DRIVE+_MAX_DIR);
      strncat(CurrentFileSpec, SourceDirEntry->name, _MAX_FNAME+_MAX_EXT);

      if(rename(CurrentFileSpec , SourceFileSpec))
        {
        return error(ERROR_WRITE, CurrentFileSpec);
        }
      else
        {
        reload_filesdir(); // reload and redisplay the DOS files dir
        SelectSourceIndex(EntryImage.entry_index);
        }
      }
    }
  return ERROR_NONE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void rename_entry(void)
{
  char     NewFileName[_MAX_FNAME + _MAX_EXT + 1];
  char     CurrentFileSpec[DOSPATHSIZE + 1];
  char     FileDrive[_MAX_DRIVE + 1];
  char     FileDir[_MAX_DIR + 1];
  char     FileName[_MAX_FNAME + 1];
  char     FileExt[_MAX_EXT + 1];
  char     Buffer[DOSPATHSIZE + 1];
  SHORT    Dummy;
  USHORT   TestStart, TestCount;

  if (directory_of_origin == CURVE_DIRECTORY)
    {
    if(is_special_name(SourceDirEntry->name))
      {
      load_new_entry();
      return;
      }
    }

  /* the count can't change */
  SourceCurveCount = SourceDirEntry->count;

  strupr(SourceFileSpec);
  _splitpath(SourceFileSpec, FileDrive, FileDir, FileName, FileExt);
  strclr(NewFileName);
  strncat(NewFileName, FileName, _MAX_FNAME);
  strncat(NewFileName, FileExt, _MAX_EXT);
  strclr(Buffer);
  strncat(Buffer, FileDrive, _MAX_DRIVE);
  strncat(Buffer, FileDir,   _MAX_DIR  );
  strupr(Buffer);             // upper case

  // if NOT the same, rename!
  if (stricmp(SourceDirEntry->name, NewFileName) ||
    stricmp(SourceDirEntry->path, Buffer) ||
    (SourceStartCurve != SourceDirEntry->StartIndex))
    {
    if (directory_of_origin == CURVE_DIRECTORY)
      {
      // change name and path
      // Do not allow renaming an entry TO one of the special file names.
      // Renaming FROM a special file is OK.
      if(is_special_name(NewFileName))
        {
        error(ERROR_NO_SPECIAL_OVERWRITE, NewFileName);
        return;
        }

      /* if only changing curve numbering, look for overlap with */
      /* other sets */
      if ((! stricmp(SourceDirEntry->name, NewFileName)) &&
        (SourceStartCurve != SourceDirEntry->StartIndex))
        {
        if (SourceStartCurve < SourceDirEntry->StartIndex)
          {
          TestStart = SourceStartCurve;
          if ((TestStart + SourceCurveCount) >
            SourceDirEntry->StartIndex)
            TestCount = SourceStartCurve - TestStart;
          else
            TestCount = SourceCurveCount;
          }
        else
          {
          if (SourceStartCurve >
            (SourceCurveCount + SourceDirEntry->StartIndex))
            {
            TestStart = SourceStartCurve;
            TestCount = SourceCurveCount;
            }
          else
            {
            TestStart = SourceDirEntry->StartIndex + SourceCurveCount;
            TestCount = SourceCurveCount -
              (TestStart - SourceDirEntry->StartIndex);
            }
          }
        }
      else
        {
        TestStart = SourceStartCurve;
        TestCount = SourceCurveCount;
        }

      // test for unique name, path, and curve number
      switch (CheckCurveBlkOverLap(Buffer, NewFileName,
        TestStart,
        TestStart + TestCount - 1,
        (PSHORT) &Dummy))
        {
        case RANGEOK:
        case SPLITRANGE:
        case OVERLAPCAT:
          error(ERROR_NOT_UNIQUE_CURVE);
        return;
        case NOOVERLAPCAT:
        case DISJOINT:
        case BADNAME:
          break;               // OK a unique name and range was given
          }

        strcpy(SourceDirEntry->path, Buffer);

        strcpy(SourceDirEntry->name, NewFileName);
        SourceDirEntry->StartIndex = SourceStartCurve;

        // redraw the curve directory because it has changed
        RefreshCurveScrollBox();
        }
      else // do a DOS rename, then reload the files directory and redisplay
        {
        NoAction = FALSE;

        strclr(CurrentFileSpec);
        strncat(CurrentFileSpec, SourceDirEntry->path, _MAX_DRIVE+_MAX_DIR);
        strncat(CurrentFileSpec, SourceDirEntry->name,
          (_MAX_FNAME + _MAX_EXT));

         if(rename(CurrentFileSpec , SourceFileSpec))
            error(ERROR_WRITE, CurrentFileSpec);
         else
         {
            reload_filesdir(); // reload and redisplay the DOS files dir
            SelectSourceIndex(EntryImage.entry_index);
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////////
// insert the SourceDirectory[SourceDirIndex] curve entry into the
// user's curve directory at DestinationDirIndex.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/* may have bug related to InsertMultiTempCurve - mlm, 7/94 */

PRIVATE void insert_to_entry(void)
{
  ERR_OMA    err = ERROR_NONE;
  USHORT source_start = SourceStartCurve; // starting target curve number
  CURVE_ENTRY *pSrcEntry; // entry where the new entry will be inserted
  CURVE_ENTRY *pDstEntry = &Curve_directory->Entries[DestinationDirIndex];
  USHORT SrcHigh, DstHigh;
  ULONG NewOffset;
  CHAR Buf[FNAME_LENGTH];
  FILE *fhnd;
  SHORT TestEntryIndex;
  USHORT LowIndex, HighIndex;

  if (directory_of_origin == FILES_DIRECTORY)
    pSrcEntry = &File_directory->Entries[SourceDirIndex];
  else
    {
    if(DestinationDirIndex == SourceDirIndex)
      {
      error(ERROR_BAD_CURVE_BLOCK, DestinationDirIndex);
      return;
      }
    pSrcEntry = &Curve_directory->Entries[SourceDirIndex];
    }

  SrcHigh = SourceStartCurve + SourceCurveCount - 1;
  DstHigh = DestinationStartCurve + SourceCurveCount - 1;

  if(DestinationDirIndex >= Curve_directory->BlkCount)
    {
    error(ERROR_BAD_CURVE_BLOCK, DestinationDirIndex);
    return;
    }

  // Split up error messages
  if ((SourceStartCurve < pSrcEntry->StartIndex) ||
    (SourceStartCurve >= pSrcEntry->StartIndex + pSrcEntry->count))
    {
    error(ERROR_CURVE_NUM, SourceStartCurve);
    return;
    }
  if (SrcHigh >= pSrcEntry->StartIndex + pSrcEntry->count)
    {
    error(ERROR_CURVE_NUM, SrcHigh);
    return;
    }
  if ((DestinationStartCurve < pDstEntry->StartIndex) ||
    (DestinationStartCurve > pDstEntry->StartIndex + pDstEntry->count))
    {
    error(ERROR_CURVE_NUM, DestinationStartCurve);
    return;
    }

  // test for unique name, path, and curve number
  HighIndex = pDstEntry->StartIndex + pDstEntry->count +
    SourceCurveCount - 1;
  if (DestinationStartCurve < pDstEntry->StartIndex)
    {
    LowIndex = DestinationStartCurve;
    HighIndex -= pDstEntry->StartIndex - DestinationStartCurve;
    }
  else
    LowIndex = pDstEntry->StartIndex;

  switch (CheckCurveBlkOverLap(pDstEntry->path, pDstEntry->name,
                               LowIndex, HighIndex,
                               (PSHORT) &TestEntryIndex))
    {
    case RANGEOK:
    case SPLITRANGE:
    case OVERLAPCAT:
      if (TestEntryIndex != (SHORT) DestinationDirIndex)
        {
        error(ERROR_NOT_UNIQUE_CURVE);
        return;
        }
      break;
    case NOOVERLAPCAT:
    case DISJOINT:
    case BADNAME:
      break;               // OK a unique name and range was given
    }

  if (directory_of_origin == FILES_DIRECTORY)
    {
    strcpy(Buf, pSrcEntry->path);
    strcat(Buf, pSrcEntry->name);
    if(! (fhnd = fopen(Buf, "rb")))
      {
      error(ERROR_OPEN, Buf);
      return;
      }

    if(InsertMultiFileCurveInTemp(fhnd, Buf, Curve_directory,
                                  DestinationDirIndex,
                                  SourceStartCurve, SourceStartCurve,
                                  & NewOffset, SourceCurveCount))
      {
      fclose(fhnd);
      return;
      }
    fclose(fhnd);
    }
  else   // CURVE DIRECTORY
    {
    /* check for overlap */
    if ((DestinationDirIndex == SourceDirIndex) &&
        (DestinationStartCurve > SourceStartCurve) &&
        (DestinationStartCurve <= SrcHigh))
      {
      error(ERROR_SOURCE_OVERWRITE);
      return;
      }

    InsertMultiTempCurve(Curve_directory, SourceDirIndex,
                         SourceStartCurve, DestinationDirIndex,
                          DestinationStartCurve, SourceCurveCount);

    }
   // redraw the curve directory because it may have changed
   RefreshCurveScrollBox();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void load_new_entry(void)
{
  ULONG        TmpOffset;
  CURVE_ENTRY  *pDstEntry;
  char         SrcPath[DOSPATHSIZE + 1],
               SrcName[DOSFILESIZE + 1],
               new_descrip[DESCRIPTION_LENGTH],
               DstEntryType;
  USHORT Dummy;

  // test for unique name, path, and curve number
  ParsePathAndName(SrcPath, SrcName, SourceFileSpec);
  // New entry may not have one of the special file names.
  if(is_special_name(SrcName))
  {
    error(ERROR_NO_SPECIAL_OVERWRITE, SourceDirEntry->name);
    return;
  }
  switch(CheckCurveBlkOverLap(SrcPath, SrcName, DestinationStartCurve,
                               DestinationStartCurve + SourceCurveCount - 1,
                               (PSHORT) &Dummy))
  {
    case RANGEOK:
    case SPLITRANGE:
    case OVERLAPCAT:
      error(ERROR_NOT_UNIQUE_CURVE);
      return;
    case NOOVERLAPCAT:
    case DISJOINT:
    case BADNAME:
      break;               // OK a unique name and range was given
  }


  /* Destination must be end of curve directory for now */

  if(DestinationDirIndex >= Curve_directory->BlkCount)
    DestinationDirIndex = Curve_directory->BlkCount;
  else
    {
    error(ERROR_NO_INDEX_OVERWRITE, DestinationDirIndex);
    return;
    }

  strncpy(new_descrip, SourceDesc, DESCRIPTION_LENGTH);
  new_descrip[DESCRIPTION_LENGTH - 1] = '\0';

  if (directory_of_origin == FILES_DIRECTORY)
  {
    if(ReadFileToTemp(SourceDirEntry->name, SourceDirEntry->path,
                        SourceStartCurve, SourceCurveCount,
                        Curve_directory, &DestinationDirIndex, &Dummy))
      return; // error
    else
    {
      pDstEntry = &Curve_directory->Entries[DestinationDirIndex];
    }
  }
  else /* (directory_of_origin == CURVE_DIRECTORY) */
  {
    WindowSelectBits = SourceDirEntry->DisplayWindow;

    if(GetTempFileLen(Curve_directory, &TmpOffset))
      return; // error

    if (SourceDirEntry->EntryType  == OMA4MEMDATA)
      DstEntryType = OMA4DATA;
    else
      DstEntryType = SourceDirEntry->EntryType;

    if(InsertCurveBlkInDir(SourceDirEntry->name,
                           SourceDirEntry->path, "",
                           DestinationStartCurve,
                           SourceDirEntry->StartOffset, 0,
                           &SourceDirEntry->time, Curve_directory,
                           DestinationDirIndex,
                           DstEntryType))
      return;  // error

    if (DestinationDirIndex <= SourceDirIndex)
    {
      SourceDirIndex++;
      SourceDirEntry++;
    }

    // this line must be after InsertCurveBlkInDir call, which may
    // change the address of the Entries array

    pDstEntry = &Curve_directory->Entries[DestinationDirIndex];
    pDstEntry->TmpOffset = TmpOffset;

    if(InsertMultiTempCurve(Curve_directory, SourceDirIndex,
                            SourceStartCurve, DestinationDirIndex,
                            DestinationStartCurve, SourceCurveCount))
      return; // error
    else
      SelectSourceIndex(SourceDirIndex);
  }

  /* finish new entry */
  pDstEntry->StartIndex = DestinationStartCurve;
  pDstEntry->DisplayWindow = WindowSelectBits;
  strcpy(pDstEntry->path, SrcPath);
  strcpy(pDstEntry->name, SrcName);
  strcpy(pDstEntry->descrip, new_descrip);

  if((directory_of_origin == CURVE_DIRECTORY) &&
    (SourceDirIndex >= DestinationDirIndex)) /* MLM or oldDestDirIdx */
    SelectSourceIndex(SourceDirIndex + 1);

  // redraw the curve directory because it may have changed
  RefreshCurveScrollBox();
}

////////////////////////////////////////////////////////////////////////////
// Save the curves from SourceStartCurve to save_to in the Target curve entry
// to the target file spec starting at the DestinationStartCurve location
// within the file.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void save_entry(void)
{
  char Buf[FNAME_LENGTH];
  FILE * fhnd;
  ERR_OMA err;
  static char temp_src_name[DOSFILESIZE + 1];
  static char temp_src_path[DOSPATHSIZE + 1];

  if(directory_of_origin != CURVE_DIRECTORY)
    // do nothing for files, they're already saved.
    return;

  strcpy(temp_src_name, SourceDirEntry->name);
  strcpy(temp_src_path, SourceDirEntry->path);

  NoAction = FALSE;

  /* append path to name if no path given */
  /* check for illegal curve block overlap on name */
  /* check for legal dos filespec */
  /* if it doesn't work, restore curve name and leave procedure */

  err = rename_just_name();

  if (err)
    return;

  strcpy(Buf, SourceDirEntry->path);
  strcat(Buf, SourceDirEntry->name);

  /* if the data saved came from a special file, restore that file's */
  /* old special name, since the data is likely to be live */

  if(is_special_name(temp_src_name))
    {
    strcpy(SourceDirEntry->name, temp_src_name);
    strcpy(SourceDirEntry->path, temp_src_path);
    }

  if(fhnd = fopen(Buf, "rb"))
    {
    fclose(fhnd);

    // exit on escape or NO
    if(yes_no_choice_window(DataFileOverwritePrompt, 0,COLORS_MESSAGE) != YES)
      return;

    if(remove(Buf))
      {
      error(ERROR_WRITE, SourceDirEntry->name);
      return;
      }
    }

  /* copy the plotbox values to InitialMethod */
  CopyPlotToMethod();

  /* copy the scan values to the method */
  DetInfoToMethod(&InitialMethod);

  WriteTempCurveBlkToFile(Buf, DestinationStartCurve, InitialMethod,
                          SourceDirectory, SourceDirIndex,
                          SourceStartCurve, SourceCurveCount);

  // get rid of copied detector data from the method
  DeAllocMetDetInfo(InitialMethod);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int GO_perform_user_request(void)
{
   BOOLEAN Redraw = TRUE;
   WINDOW *MessageWindow;

   put_up_message_window(BusyWorking, COLORS_MESSAGE, &MessageWindow);

   gather_window_bits_from_toggle();
   SourceDirEntry->DisplayWindow = WindowSelectBits;

   dummy_select_flag = FALSE;

   switch (action)
   {
      case CURVE_ACTION_LOAD:
         load_new_entry();
         Redraw = TRUE;
         break;
      case CURVE_ACTION_SAVE:
         save_entry();
         break;
      case CURVE_ACTION_DELETE:
         Redraw = delete_entry();
         break;
      case CURVE_ACTION_INSERT:
         insert_to_entry();
         Redraw = TRUE;
         break;
      case CURVE_ACTION_RENAME:
         rename_entry();
         if (directory_of_origin == CURVE_DIRECTORY)
            Redraw = TRUE;
         break;
   }
   if (MessageWindow != NULL)
      release_message_window(MessageWindow);
   if (Redraw)
   {
      WindowSelectBits = SourceDirEntry->DisplayWindow;
      expand_window_bits_for_toggle();

      draw_form();             // redraw the user dialog box
   }
   return FIELD_VALIDATE_SUCCESS; // always succeed
}

////////////////////////////////////////////////////////////////////////////
// Function  : puts scrolling curve directory form on the screen.
//             restores the screen when done
//
// Arguments : curve_dir points to the curve directory to be shown on screen.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

BOOLEAN FileDirFormInit(void)
{
  CHAR    TempPath[DOSPATHSIZE + 1];
  CHAR    TempName[DOSFILESIZE + 1];
  CHAR    TempFileSpec[FNAME_LENGTH];
  CHAR    *pChar;
  CHAR    OldChar;
  BOOLEAN Slash = TRUE;

  Curve_directory = & MainCurveDir;
  File_directory  = & FileCurveDir;

  if(!InFileDirForm) /* in case re-init from macro, etc. */
    {
    if(! (FileCurveDir.Entries = malloc(1)))
      {
      // error, could not allocate memory for files directory
      error(ERROR_ALLOC_MEM);
      return FALSE;
      }
    else
      {
      InFileDirForm = TRUE;

      // set up and load the files curve directory

      FileCurveDir.BlkCount   = 0;  // no curves or entries loaded yet
      FileCurveDir.CurveCount = 0;

      strcpy(TempFileSpec, DOS_FileSpec);
      pChar = strrchr(TempFileSpec, '\\');
      if (pChar == NULL)
        {
        Slash = FALSE;
        if ((pChar = strrchr(TempFileSpec, ':')) == NULL)
          pChar = TempFileSpec;
        else
          {
          pChar++;
          if (*pChar == '\0')
            pChar[1] = '\0';
          }
        }
      OldChar = *pChar;
      *pChar = '\0';

      ParsePathAndName(TempPath, TempName, TempFileSpec);

      if (! Slash)  // if no occurrance of '\\'
        {
        *pChar = OldChar;
        strcpy(TempFileSpec, TempPath);
        strcat(TempFileSpec, pChar);
        strcpy(DOS_FileSpec, TempFileSpec);
        }
      else
        {
        strcpy(DOS_FileSpec, TempPath);
        strcat(DOS_FileSpec, ++pChar);
        }

      // sort ascending by name for now, need to specify options later
      new_files(DOS_FileSpec, SORT_NAME | SORT_ASCEND);
      }
    // switch the shaded color pair for the default color set to be the
    // same as the regular color pair so that select fields in the scrolling
    // form will not change color if the user types backspace or delete.

    saved_shaded.foreground = ColorSets[COLORS_DEFAULT].shaded.foreground;
    saved_shaded.background = ColorSets[COLORS_DEFAULT].shaded.background;
    ColorSets[COLORS_DEFAULT].shaded.foreground =
      ColorSets[COLORS_DEFAULT].regular.foreground;
    ColorSets[COLORS_DEFAULT].shaded.background =
      ColorSets[COLORS_DEFAULT].regular.background;

    ControlForm.exit_key_code = KEY_ESCAPE;
    }
  return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN FileDirFormExit(void)
{
   ControlForm.field_index = 0;

   // restore the shaded color pair to its original value
   ColorSets[COLORS_DEFAULT].shaded.foreground = saved_shaded.foreground;
   ColorSets[COLORS_DEFAULT].shaded.background = saved_shaded.background;

   clearFileDir(); // delete all entries from the files curve directory
   free(FileCurveDir.Entries);

   InFileDirForm = FALSE;
   return FALSE;
}

// get file directory display filter passed in from "outside"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void set_dataspec(char * fileSpec)
{
  if(fileSpec)
    strcpy(DOS_FileSpec, fileSpec);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int ChangeDescription(void * field_data, char * field_string)
{
   if (directory_of_origin == CURVE_DIRECTORY)
   {
      strncpy(SourceDirEntry->descrip, field_string, DESCRIPTION_LENGTH);
      SourceDirEntry->descrip[DESCRIPTION_LENGTH - 1] = '\0';
   }
   else
   {
      strcpy(field_string, SourceDirEntry->descrip);
      return FIELD_VALIDATE_WARNING;
   }
   return FIELD_VALIDATE_SUCCESS;
}

// init FormTable with form addresses
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void registerScrlltstForms(void)
{
   FormTable[KSI_FILE_DIR_FORM  ] = &ControlForm;
   FormTable[KSI_FD_CURVE_SCROLL] = &CurveScrollForm;
   FormTable[KSI_FD_FILE_SCROLL ] = &FileScrollForm;
   FormTable[KSI_FD_USER        ] = &UserForm;
}
