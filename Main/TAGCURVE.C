
/* tagcurve.c                 RAC  Oct 3, 1990.
/
/    Copyright (c) 1990,  EG&G Instruments Inc.
/
/
/  $Header:   J:/logfiles/oma4000/main/tagcurve.c_v   0.23   06 Jul 1992 10:37:16   maynard  $
/  $Log:   J:/logfiles/oma4000/main/tagcurve.c_v  $
 * 
 *    Rev 0.23   06 Jul 1992 10:37:16   maynard
 * cast some items in form defs and field data defs to elim compiler warnings
 * 
 *    Rev 0.22   30 Mar 1992 12:11:40   maynard
 * Change 2 calls to isFormGraphWindow to isPrevFormGraphWindow.  This function
 * is called with less overhead, and also makes sure there is a previous form
 * 
 *    Rev 0.21   13 Jan 1992 15:46:26   cole
 * Change include's. Add ExpandedOnTagged.
 * plot_curves_labels(), ResizePlotForWindow() have one less arg.
 * Replace setTaggedColor() with taggedColor(). Add MacExpandTagged(),
 *  MacRestoreFromExpand(), MacUntagCurve(), MacTagCurve().
 * Initial revision.
/
/ ------------------------------------------------------------------------
*/


// In graphics mode, curves can be tagged.  In isolate mode the tagged
// curves are displayed in a single window.

#include <stdio.h>

#include "tagcurve.h"
#include "wintags.h"
#include "helpindx.h"
#include "ksindex.h"
#include "cursor.h"        // ActiveWindow
#include "doplot.h"      // ResizePlotForWindow()
#include "device.h"
#include "omaform.h"
#include "graphops.h"      // SystemCursor
#include "fkeyfunc.h"
#include "multi.h"
#include "macrecor.h"
#include "macruntm.h"
#include "live.h"
#include "di_util.h"     // ParsePathAndName()
#include "tempdata.h"
#include "formwind.h"
#include "baslnsub.h"
#include "pltsetup.h"
#include "omamenu.h"
#include "formtabs.h"
#include "omaerror.h"
#include "syserror.h"
#include "crventry.h"     // OMA4DATA
#include "curvedir.h"
#include "curvdraw.h"   // PutUpPlotBoxes()
#include "plotbox.h"    // DisplayGraphArea
#include "forms.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

BOOLEAN ExpandedOnTagged = FALSE;

static struct save_area_info * SavedArea;

// start value and increment for group tag form to tag a group of curves
static float groupStartZ;
static int groupDelta = 1;

CHAR SaveFileName[DOSFILESIZE + DOSPATHSIZE + 1];
CHAR SaveFileDesc[DESCRIPTION_LENGTH] = "";

// dummy variable for "Go" select field
static int dummySelect;

// Default curve color for tagged curves and an alternate color.
// Use alternate color if standard plot color is TaggedColor.
static enum { TaggedColor = BRT_GREEN, AltTaggedColor = BRT_WHITE };

// Names for indexing into the DataRegistry array.
// Reserve locations 38 thru 40 for this module in the DataRegistry array
// in omaform1.c
static enum DATAREGISTRY_ACCESS { TC_DO_STRINGS = 1, TC_CODE, TC_DATA };

// TC_DO_STRINGS
static enum { StringTitle, StringFirst, StringDelta, StringGo,
   SAVE_TO_STRING };
static DATA DO_String_Reg[] = {
   { "Tag Curve Group", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 0
   { "First Z", 0, DATATYP_STRING, DATAATTR_PTR, 0 },          // 1
   { "Delta Z", 0, DATATYP_STRING, DATAATTR_PTR, 0 },          // 2
   { " Go ",    0, DATATYP_STRING, DATAATTR_PTR, 0 },           // 3
   { "Copy Tagged Curves To", 0, DATATYP_STRING, DATAATTR_PTR, 0 }// 4
};

// functions for code registry
PRIVATE BOOLEAN TagGroupInit(void);
PRIVATE BOOLEAN TagGroupExit(void);
PRIVATE int     TagGroupGo(void);
PRIVATE BOOLEAN SaveTaggedInit(void);
PRIVATE BOOLEAN SaveTaggedExit(void);
PRIVATE int     SaveTaggedGo(void);

// TC_CODE
static enum { CodeInit, CodeGo, CodeExit, SaveInit, SaveGo, SaveExit,
              VerifyWriteFileName};
static EXEC_DATA Code_Reg[] = {
   { TagGroupInit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },  // 0
   { TagGroupGo,   0, DATATYP_CODE, DATAATTR_PTR, 0 },  // 1
   { TagGroupExit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },  // 2
   { SaveTaggedInit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },  // 3
   { SaveTaggedGo,   0, DATATYP_CODE, DATAATTR_PTR, 0 },  // 4
   { SaveTaggedExit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },  // 5
   { VerifyWritableFileName, 0, DATATYP_CODE, DATAATTR_PTR, 0 }, // 6
};

// TC_DATA
static enum { DataStartZ, DataDelta, DummySelect, SAVE_FILE_NAME };
static DATA Data_Reg[] = {
   { & groupStartZ, 0, DATATYP_FLOAT,  DATAATTR_PTR, 0 },  //  0
   { & groupDelta,  0, DATATYP_INT,    DATAATTR_PTR, 0 },  //  1
   { & dummySelect, 0, DATATYP_INT,    DATAATTR_PTR, 0 },   //  2
   { & SaveFileName, 0, DATATYP_STRING, DATAATTR_PTR, 0 }   //  3
};

// fields and form for tagging a group of curves
static enum { Z_first = 3, Z_delta = 4, GoButton = 5 };
static FIELD TagCurveGroupFormFields[] =
{
   // "Tag Curve Group" display only, row 1, column 8.            // 0
   do_field_set(FLDTYP_STRING, FLDATTR_DISPLAY_ONLY, 0,
   TC_DO_STRINGS, StringTitle, 0, 0, 0, 0, 0, 0,
   1, 8, 15),

   // "First Z" display only, row 3, column 1                     // 1
   do_field_set(FLDTYP_STRING, FLDATTR_DISPLAY_ONLY, 0,
   TC_DO_STRINGS, StringFirst, 0, 0, 0, 0, 0, 0,
   3, 1, 7),

   // "Delta Z" display only, row 3, column 20                    // 2
   do_field_set(FLDTYP_STRING, FLDATTR_DISPLAY_ONLY, 0,
   TC_DO_STRINGS, StringDelta, 0, 0, 0, 0, 0, 0,
   3, 20, 7),

   // first Z, float value, editable, row 3, col 9, length 9
   field_set(                                                    // 3
   Z_first,   FLDTYP_STD_FLOAT, FLDATTR_REV_VID | FLDATTR_RJ,
   KSI_TC_ZSTART, TAGCURVE_HBASE,
   TC_DATA, DataStartZ, 0, 0, 0, 0, 5, 0,
   3, 9, 9,
   EXIT, Z_first, GoButton, GoButton,
   Z_delta, Z_delta, GoButton, Z_delta),

   // delta Z, integer value, editable, row 3, col 28, length 7
   field_set(                                                    // 4
   Z_delta, FLDTYP_INT, FLDATTR_REV_VID | FLDATTR_RJ,
   KSI_TC_DELTA, TAGCURVE_HBASE + 1,
   TC_DATA, DataDelta, 0, 0, 0, 0, 5, 0,
   3, 28, 7,
   EXIT, Z_delta, GoButton, GoButton,
   Z_first, Z_first, Z_first, GoButton),

   // "Select" select field, row 5, col 15, length 4
   field_set(                                                           // 5
   GoButton, FLDTYP_SELECT, FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_TC_GO, TAGCURVE_HBASE + 2,
   TC_DATA, DummySelect,         // dummy select value
   TC_DO_STRINGS, StringGo,      // " Go "
   TC_CODE, CodeGo,              // TagGroupGo()
   1, 0,                         // match value, dummy
   5, 15, 4,
   EXIT, FORM_EXIT_UP, Z_first, Z_first,
   0, 0, Z_delta, Z_first)
};

static FORM TagCurveGroupForm = {
   0, 0, FORMATTR_BORDER | FORMATTR_FIRST_CHAR_ERASE | FORMATTR_VISIBLE,
   0, 0, 0, 8, 21, 7, 37, 0, 0,
   { TC_CODE, CodeInit }, { TC_CODE, CodeExit }, COLORS_DEFAULT,
   0, 0, 0, 0, 6, TagCurveGroupFormFields, KSI_TAG_CURVES_FORM,
   0, DO_String_Reg, (DATA *)Code_Reg, Data_Reg, 0, 0
};

// fields and form for tagging a group of curves
static enum { FILE_NAME = 1, GO_BUTTON = 2 };
static FIELD SaveTaggedFormFields[] =
{
   // "Save To" display only, row 1, column 2.            // 0
   do_field_set(FLDTYP_STRING, FLDATTR_DISPLAY_ONLY, 0,
   TC_DO_STRINGS, SAVE_TO_STRING, 0, 0, 0, 0, 0, 0,
   1, 1, 21),

   // first Z, float value, editable, row 1, col 23, length 56
   field_set(FILE_NAME,
   FLDTYP_STRING,
   FLDATTR_REV_VID,
   KSI_ST_FILE_NAME,
   TAGCURVE_HBASE + 3,
   TC_DATA, SAVE_FILE_NAME,
   0, 0,
   TC_CODE, VerifyWriteFileName,
   sizeof(SaveFileName) - 1, 0,
   1, 23, 55,
   FORM_EXIT_UP, FILE_NAME, GO_BUTTON, GO_BUTTON,
   FILE_NAME, FILE_NAME, GO_BUTTON, GO_BUTTON),

   // "Select" select field, row 5, col 15, length 4
   field_set(                                                           // 5
   GO_BUTTON, FLDTYP_SELECT, FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_ST_GO, TAGCURVE_HBASE + 4,
   TC_DATA, DummySelect,         // dummy select value
   TC_DO_STRINGS, StringGo,      // " Go "
   TC_CODE, SaveGo,              // SaveTaggedGo()
   1, 0,                         // match value, dummy
   3, 38, 4,
   FORM_EXIT_UP, GO_BUTTON, FILE_NAME, FILE_NAME,
   GO_BUTTON, GO_BUTTON, FILE_NAME, FILE_NAME)
};

static FORM SaveTaggedForm = {
   0, 0, FORMATTR_BORDER | FORMATTR_FIRST_CHAR_ERASE |
    FORMATTR_EXIT_RESTORE | FORMATTR_VISIBLE,
   0, 0, 0, 8, 0, 5, 80, 0, 0,
   { TC_CODE, SaveInit }, { TC_CODE, SaveExit }, COLORS_DEFAULT,
   0, 0, 0, 0, 3, SaveTaggedFormFields, KSI_SAVE_TAGGED_FORM,
   0, DO_String_Reg, (DATA *)Code_Reg, Data_Reg, 0, 0
};

// Initialization function for group tag form.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN TagGroupInit(void)
{
   RemoveGraphCursor();
   SavedArea = save_screen_area(TagCurveGroupForm.row,
                                 TagCurveGroupForm.column,
                                 TagCurveGroupForm.size_in_rows,
                                 TagCurveGroupForm.size_in_columns);

   // default start at current curve
   groupStartZ = CursorStatus[ ActiveWindow ].Z;

   // always start out in the delta field
   TagCurveGroupForm.field_index = 2;

   return FALSE;
}

// Run the add tag group form and then redraw all the tagged curves after
// the form exits and then restore the cursor.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN TagGroupExit(void)
{
   CXY CursorLoc = GetCurrentCursorPosition(LOCUS_APPLICATION);

   if(SavedArea)
      restore_screen_area(SavedArea);
   else
    PutUpPlotBoxes();
   drawTagged(0);

   SetGraphCursorType(CursorType);
   DisplayGraphCursor(CursorLoc );

   return FALSE;
}
// GO field action for group tag form.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int TagGroupGo(void)
{
   addTagGroup(ActiveWindow, groupStartZ, groupDelta);

   return FIELD_VALIDATE_SUCCESS;
}

// Tag the curve that the cursor is on in the active plot and redraw the
// curve as a tagged curve.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HotTagCurve(USHORT dummy)
{
   CHAR Buf[16];

   addTag(ActiveWindow, CursorStatus[ ActiveWindow ].UTDisplayCurve);

   draw_one_curve(ActiveWindow, CursorStatus[ ActiveWindow ].Z,
                   DRAW_TAGGED_ONLY);

   if (pKSRecord != NULL)
   {
      if (*pKSRecord)
      {
         sprintf(Buf, "TAG_CURVE();\n");
         MacRecordString(Buf);
      }
   }
   dummy;
}

// add curve untagging function.
// Untag the curve that the cursor is on in the active plot and redraw the
// curve in the normal plot color.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HotUntagCurve(USHORT dummy)
{
   CURSORSTAT * pCursorStat = &CursorStatus[ ActiveWindow ];

   deleteTag(ActiveWindow, pCursorStat->UTDisplayCurve);

   // redraw if showing tagged curves
   if (ExpandedOnTagged)
   {
      PutUpPlotBoxes();
      PlotScreen(dummy);
      CursorStatus[ActiveWindow].TotalCurves--;
   }
   else
      draw_one_curve(ActiveWindow, pCursorStat->Z, DRAW_ALL);

   JumpCursor(pCursorStat->X, pCursorStat->Z);

   if (pKSRecord != NULL)
   {
      if (*pKSRecord)
      {
         char Buf[ 18 ];

         sprintf(Buf, "UNTAG_CURVE();\n");
         MacRecordString(Buf);
      }
   }
}

// Redraw all the tagged curves in window.
// asTagged true iff draw as tagged curves, else draw as normal curves.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void redrawTagged(SHORT window, BOOLEAN asTagged)
{
   SHORT PlotIndex = WindowPlotAssignment[window];

   plot_curves_labels(&MainCurveDir, & Plots[PlotIndex], window,
                      asTagged ? DRAW_TAGGED_ONLY : DRAW_TAGGED_AS_NORMAL);
}

// Redraw all the tagged curves in the active plot in normal plot color
// and then untag them.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HotUntagAll(USHORT dummy)
{
   RemoveGraphCursor();
   redrawTagged(ActiveWindow, FALSE);

   deleteAllTags(ActiveWindow);

   // redraw if showing tagged curves
   if (ExpandedOnTagged)
   {
      PutUpPlotBoxes();
      PlotScreen(dummy);
      // reset to an uninitialized condition
      CursorStatus[ActiveWindow].EntryIndex = -1;
      ForceCursorIntoWindow(ActiveWindow);
      CursorStatus[ActiveWindow].TotalCurves = 0;
      CursorStatus[ActiveWindow].UTDisplayCurve = 0;
   }

   JumpCursor(CursorStatus[ActiveWindow].X,
               CursorStatus[ActiveWindow].Z);

   dummy;
}

// Redraw all the tagged curves in special tagged curve color.
// Invoked via CNTRL-F6 in graphics mode.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void drawTagged(USHORT dummy)
{
   redrawTagged(ActiveWindow, TRUE);

   dummy;
}

// Return an appropriate color for drawing tagged curves, given the current
// curve drawing color
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CCOLOR taggedColor(CCOLOR currentColor)
{
   if (currentColor == TaggedColor)
      return AltTaggedColor;
   else
      return TaggedColor;
}

// connected to ALT-F6 hot key
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ExpandTaggedToggle(USHORT Dummy)
{
   CHAR Buf[26];
   static UCHAR OldLiveControl;
   static SHORT OldWindowStyle;

   if (!MacroRunProgram)
      ExpandedOnTagged = !ExpandedOnTagged;

   if (active_locus != LOCUS_APPLICATION)
      RemoveGraphCursor();

   if (ExpandedOnTagged)
   {
      OldWindowStyle = window_style;
      window_style = 0;
      multiplot_setup(&(PlotWindows[ActiveWindow]),
         &DisplayGraphArea, window_style);
   }
   else
   {
      window_style = OldWindowStyle;
      multiplot_setup(PlotWindows,
                       &DisplayGraphArea, window_style);
   }

   // replot if plotarea is showing
   if(plotAreaShowing())
   {
      PutUpPlotBoxes();
      PlotScreen(Dummy);
   }

   ResizePlotForWindow(ActiveWindow);

   // reset to an uninitialized condition
   CursorStatus[ActiveWindow].EntryIndex = -1;
   ForceCursorIntoWindow(ActiveWindow);

   // put up the cursor if plotarea is showing
   if(plotAreaShowing())
   {
      SetCursorPos(ActiveWindow, CursorStatus[ActiveWindow].X,
         CursorStatus[ActiveWindow].Y,
         CursorStatus[ActiveWindow].Z);
      if (active_locus == LOCUS_APPLICATION)
         InitCursorStatus(ActiveWindow);
   }


   if (ExpandedOnTagged)
   {
      // Disable live
      OldLiveControl = FKeyItems[1].Control;
      FKeyItems[1].Control |= MENUITEM_INACTIVE;

      // Enable Capture of tagged curves
      FKeyItems[4].Text = TagSaveStr;
      FKeyItems[4].Control = MENUITEM_CALLS_FORM;
      FKeyItems[4].SubMenu = &SaveTaggedForm;
      FKeyItems[4].Action = NULL;
      FKeyItems[35].Text = TagShrinkStr;

      // disable F9 Sel Window and Alt-F9 Sel Layout
      FKeyItems[8].Control |= MENUITEM_INACTIVE;
      FKeyItems[38].Control |= MENUITEM_INACTIVE;

      // can't retag tagged curves in expanded mode
      CtrlHotKeyItems['T' - 'A'].Control |= MENUITEM_INACTIVE;
      AltHotKeyItems['T' - 'A'].Control |= MENUITEM_INACTIVE;
      if (pKSRecord != NULL)
      {
         if (*pKSRecord)
         {
            sprintf(Buf, "EXPAND_TAGGED();\n");
            MacRecordString(Buf);
         }
      }
      UpdateCursorMode(CURSORMODE_TAGGED);
   }
   else
   {
      FKeyItems[1].Control = OldLiveControl;
      FKeyItems[35].Text = TagExpandStr;
      FKeyItems[4].Text = LiveCaptureStr;
      FKeyItems[4].Control = MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE;
      FKeyItems[4].Action = CaptureLive;
      FKeyItems[4].SubMenu = NULL;

      // reenable F9 Sel Window and Alt-F9 Sel Layout
      FKeyItems[8].Control &= ~MENUITEM_INACTIVE;
      FKeyItems[38].Control &= ~MENUITEM_INACTIVE;

      // reenable tag curves in expanded mode
      CtrlHotKeyItems['T' - 'A'].Control &= ~MENUITEM_INACTIVE;
      AltHotKeyItems['T' - 'A'].Control &= ~MENUITEM_INACTIVE;
      if (pKSRecord != NULL)
      {
         if (*pKSRecord)
         {
            sprintf(Buf, "RESTORE_EXPAND();\n");
            MacRecordString(Buf);
         }
      }
      UpdateCursorMode(CURSORMODE_NORMAL);
   }

   FKeyItems[4].TextLen = 0;
   FKeyItems[35].TextLen = (char) 0;

   // force a redraw on next ShiftCheck
   CurrentShiftMode = -1;
   return;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN SaveTaggedInit(void)
{
   RemoveGraphCursor();

   return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN SaveTaggedExit(void)
{
   CXY CursorLoc = GetCurrentCursorPosition(LOCUS_APPLICATION);

   SetGraphCursorType(CursorType);
   DisplayGraphCursor(CursorLoc);

   return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int SaveTaggedGo(void)
{
   CHAR Name[DOSFILESIZE + 1];
   CHAR Path[DOSPATHSIZE + 1];
   USHORT DstEntryIndex, CurveNum;
   USHORT InsertCurveNumber, SrcEntryIndex, SrcCurveNumber, UTDisplayIndex;
   ERR_OMA err;
   BOOLEAN Found;
   WINDOW *MessageWindow;

   CurveNum = CursorStatus[ActiveWindow].TotalCurves;

   if (CurveNum == 0)
   {
      error(ERROR_CURVE_NUM, 0);
      return TRUE;
   }

   if (ParsePathAndName(Path, Name, SaveFileName) != 2)
   {
      error(ERROR_BAD_FILENAME, SaveFileName);
      return TRUE;  /* flag error */
   }

   /* check to see that the curve range is present */
   // always use a start index of 0
   switch (CheckCurveBlkOverLap(Path, Name, 0, CurveNum - 1, &DstEntryIndex))
   {
      case BADNAME:     // good
         break;
  
      case SPLITRANGE:
         error( ERROR_SPLIT_RANGE, 0, CurveNum - 1);
         return TRUE;
         break;
  
      case DISJOINT:
         /* OK same file name but different ranges */
         break;
  
      case RANGEOK:
         /* OK */
         break;
      case NOOVERLAPCAT:
      case OVERLAPCAT:
         /* put up overwrite warning */
         if(yes_no_choice_window(BlkOverWritePrompt, 0, COLORS_MESSAGE)
              != YES)   // exit on escape or NO
            return FALSE;
         if (err = DelTempFileBlk(&MainCurveDir, DstEntryIndex))
            return TRUE;

         break;
   }

   if (err = CreateTempFileBlk(& MainCurveDir, & DstEntryIndex,
                                Name, Path, SaveFileDesc, 0, 0L, 0, NULL,
                                OMA4DATA, 0))
      return TRUE;

   put_up_message_window(BusyWorking, COLORS_MESSAGE, &MessageWindow);

   InsertCurveNumber = 0;
   Found = FindFirstPlotBlock(& MainCurveDir,   & SrcEntryIndex,
                               & SrcCurveNumber, & UTDisplayIndex,
                               ActiveWindow);
   while (Found)
   {
      if (err = InsertMultiTempCurve(&MainCurveDir, SrcEntryIndex,
                                      SrcCurveNumber, DstEntryIndex,
                                      InsertCurveNumber, 1))
      {
         if (MessageWindow != NULL)
            release_message_window(MessageWindow);
         return TRUE;
      }

      Found = FindNextPlotCurve(&MainCurveDir, &SrcEntryIndex,
                                 &SrcCurveNumber, &UTDisplayIndex,
                                 ActiveWindow);
      InsertCurveNumber++;
   }

   if (MessageWindow != NULL)
      release_message_window(MessageWindow);

   Current.Form->status = FORMSTAT_EXIT_THIS_FORM;

   return FIELD_VALIDATE_SUCCESS;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacExpandTagged(void)        
{
   BOOLEAN RemGCursor = FALSE;
   SaveAreaInfo *SavedArea;

   if(plotAreaShowing())
   {
      if ((active_locus == LOCUS_MENUS) ||
           ((active_locus == LOCUS_FORMS) && !
             ((isCurrentFormMacroForm()) &&
              (isFormGraphWindow(Current.PreviousStackedContext->Form))))
        )
      {
         RemGCursor = TRUE;
         erase_cursor();
         SetGraphCursorType(CursorType);
         SavedArea = save_screen_area(1, 0, 1, 80);
      }
      else if(isCurrentFormMacroForm())
      {
         erase_cursor();
         SetGraphCursorType(CursorType);
      }

      ExpandedOnTagged = TRUE;
      ExpandTaggedToggle(0);

      if (RemGCursor)
      {
         RemoveGraphCursor();
         set_cursor_type(TextCursor);
         restore_screen_area(SavedArea);
      }
      else if(isCurrentFormMacroForm())
         set_cursor_type(TextCursor);
   }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacRestoreFromExpand(void)   
{

  if(plotAreaShowing())
    {
    SaveAreaInfo *SavedArea;
    BOOLEAN RemGCursor = TempChangeCursorType(&SavedArea);

    ExpandedOnTagged = FALSE;
    ExpandTaggedToggle(0);

    TempRestoreCursorType(RemGCursor, &SavedArea);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacUntagCurve(void)
{

  if(plotAreaShowing())
    {
    SaveAreaInfo *SavedArea;
    BOOLEAN RemGCursor = TempChangeCursorType(&SavedArea);

    HotUntagCurve(0);
    TempRestoreCursorType(RemGCursor, &SavedArea);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacTagCurve(void)
{
   if(plotAreaShowing()) HotTagCurve(0);
}

// initialize FormTable[]
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void registerTagcurveForms(void)
{
   FormTable[ KSI_TAG_CURVES_FORM] = &TagCurveGroupForm;
   FormTable[ KSI_SAVE_TAGGED_FORM] = &SaveTaggedForm;
}
