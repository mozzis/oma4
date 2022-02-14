/* -----------------------------------------------------------------------
/
/  fkeyfunc.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: DAI      Version 1.00         21 Nov.        1989
/
/  fkeyfunc.c is a collection of functions to support function key action
/
/
/  $Header:   J:/logfiles/oma4000/main/fkeyfunc.c_v   0.23   13 Jan 1992 12:57:08   cole  $
/  $Log:   J:/logfiles/oma4000/main/fkeyfunc.c_v  $
/
/ -----------------------------------------------------------------------
*/

#ifdef PROT
   #define INCL_KBD
   #include <os2.h>
#endif

#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "fkeyfunc.h"
#include "device.h"
#include "live.h"
#include "helpindx.h"
#include "cursor.h"
#include "omaform.h"
#include "macrecor.h"
#include "macruntm.h"
#include "formwind.h"
#include "omamenu.h"
#include "syserror.h"
#include "omaerror.h"
#include "ksindex.h"
#include "graphops.h"
#include "tagcurve.h"
#include "omazoom.h"
//#include "cursvbar.h"  // only needed for GRAPHICAL scan setup form
#include "autopeak.h"
#include "handy.h"
#include "crventry.h"
#include "autoscal.h"  // ScalePlotGraph
#include "curvedir.h"
#include "forms.h"
#include "macrofrm.h"  // RunMacroForm()
#include "coolstat.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

#define FKEY_ROW              (screen_rows - 2)
#define FKEY_ROW_SIZE         2
#define FKEY_ITEMS_PER_LINE   5

USHORT CurrentShiftMode = 0;
PCHAR NoCurveSetStr = "     No Curve Sets Present     ";

CHAR * LiveCaptureStr = "F5 Capture";
CHAR * GraphModeStr   = "F10 Graph";

static MENU *  CurrentFKeyMenu;

static USHORT  CurveSetHelpLines = 0;
static USHORT  CurveSetHelpItems = 0;
static BOOLEAN NewString = FALSE;

MENUITEM far FKeyItems[] = {

   { "F1 Help", 0, 0, 0, NULL, GetHelp,
    MENUITEM_CALLS_FUNCTION, 0 },                     // 0
   { NULL,              0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION, 0 },
   { NULL,              0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL,              0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL,      0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { "F6 Zoom",         0, 0, 0, NULL, Zoom,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { "F7 Restore Zoom", 0, 0, 0, NULL, (MenuAction *)RestoreZoom,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { "F8 AutoScale",    0, 0, 0, NULL, (MenuAction *)ScalePlotGraph,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { "F9 Sel Window",   0, 0, 0, NULL, (MenuAction *)SelectWindow,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL,              0, 0, 0, NULL, (MenuAction *)MenuModeToggle,
   MENUITEM_CALLS_FUNCTION, 0 },

   // shift FKey Items
   { NULL, 0, 0, 0, NULL, NULL,                 // 10
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { "F8 AutoScale Z", 0, 0, 0, NULL, (MenuAction *)ScaleZAxis,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },

   // Control FKey Items                              // 20
   { "F1 Key Info", 0, 0, 0, NULL, ShowLocalInfo,
   MENUITEM_CALLS_FUNCTION, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { "F6 ID Tagged", 0, 0, 0, NULL, drawTagged,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { "F8 AutoScale X", 0, 0, 0, NULL, (MenuAction *)ScaleXAxis,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },

   // Alt FKey Items                                  // 30
   { "F1 Curve Dir", 0, 0, 0, NULL, GetPopCurveDir,
   MENUITEM_CALLS_FUNCTION, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { "F3 Stop KeyStrk", 0, 0, 0, NULL, StopKeyStroke,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { "F4 Next Frame", 0, 0, 0, NULL, (MenuAction *)NextFrame,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { "F5 To Bkgrnd", 0, 0, 0, NULL, (MenuAction *)CaptureBackGround,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, ExpandTaggedToggle,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { NULL, 0, 0, 0, NULL, NULL,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { "F8 AutoScale Y", 0, 0, 0, NULL, (MenuAction *)ScaleYAxis,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { "F9 Sel Layout",  0, 0, 0, NULL, (MenuAction *)SelectPlotForWindow,
   MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 },
   { "F10 Command Ln", 0, 0, 0, NULL, RunMacroForm,
   MENUITEM_CALLS_FUNCTION, 0 }
};

MENU  FKey = { 0, COLORS_MENU, 10, FKeyItems, KSI_FKEY_HOT };

static MENU ShiftFKey = { 0, COLORS_MENU, 10, &FKeyItems[10],
                                                      KSI_SHIFT_FKEY_HOT };

static MENU ControlFKey = { 0, COLORS_MENU, 10, &FKeyItems[20],
                                                    KSI_CONTROL_FKEY_HOT };

static MENU AltFKey = { 0, COLORS_MENU, 10, &FKeyItems[30],
                                                        KSI_ALT_FKEY_HOT };

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT ShowFKeys(MENU *FKeyMenu)
{
   MENUCONTEXT OldContext = MenuFocus;
   SHORT       i;
   UCHAR       column_offset = 0;
   UCHAR       attribute;
   MENUITEM *  ThisItem;
   SHORT       ColumnInc = screen_columns / FKEY_ITEMS_PER_LINE;
   CRECT       OldClipRect;
   CFILLREPR   FillRep;
   int         shadeItem;

   CurrentFKeyMenu = FKeyMenu;

   if (MacroRunProgram)
    return FALSE;

   CInqFillRepr(screen_handle, &FillRep);

   erase_mouse_cursor();
   MouseCursorEnable(FALSE);

   CInqClipRectangle(screen_handle, &OldClipRect);
   setClipRectToFullScreen();

   MenuFocus.Row          = (char)FKEY_ROW;
   MenuFocus.Column       = 0;
   MenuFocus.SizeInRows   = FKEY_ROW_SIZE;
   MenuFocus.ItemIndex    = 0;
   MenuFocus.ActiveMENU   = FKeyMenu;
   MenuFocus.MenuColorSet = &ColorSets[MenuFocus.ActiveMENU->ColorSetIndex];
   MenuFocus.PriorContext = NULL;

   attribute = set_attributes(MenuFocus.MenuColorSet->regular.foreground,
                              MenuFocus.MenuColorSet->regular.background);

   erase_screen_area(MenuFocus.Row, MenuFocus.Column, MenuFocus.SizeInRows,
               screen_columns, attribute, FALSE);

   for (i=0; i < MenuFocus.ActiveMENU->ItemCount; i++)
   {
      ThisItem = &MenuFocus.ActiveMENU->ItemList[i];

      if (ThisItem->Text != NULL)
      {
         if (ThisItem->TextLen == 0)
            ThisItem->TextLen = (char) strlen(ThisItem->Text);

         /* Wrap To next line */
         if ((column_offset + ThisItem->TextLen) >= (UCHAR)screen_columns)
         {
            column_offset = 0;
            MenuFocus.Column = 0;
            MenuFocus.Row++;
         }

         ThisItem->Column = (UCHAR) column_offset;

         shadeItem = ThisItem->Control & MENUITEM_INACTIVE;
         if(ZoomState != FALSE) {
            if((FKeyMenu == & AltFKey) && (i == 0))
              ;
            else if((FKeyMenu == & FKey) && (i == 0))
              ;
            else
               shadeItem = TRUE;
         }

         if (shadeItem)
            shade_menuitem((UCHAR) i);
         else
         {
            SHORT j = 0;

            unhighlight_menuitem((UCHAR) i);

            /* highlight select char (Function key), */
            /* highlights to first space */
            do
            {
               emit(ThisItem->Text[j], MenuFocus.Row, column_offset + j + 1,
                     set_attributes(
                                MenuFocus.MenuColorSet->highlight.foreground,
                                MenuFocus.MenuColorSet->highlight.background)
                  );
               j++;
            }
            while (ThisItem->Text[j] != ' ');
         }
      }

      column_offset += ColumnInc;
   }

   MenuFocus = OldContext;

   CSetClipRectangle(screen_handle, OldClipRect);

   MouseCursorEnable(TRUE);
   replace_mouse_cursor();

   CSetFillRepr(screen_handle, &FillRep);

   displayCoolerStatus(TRUE);

   return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UCHAR HandleFKey(UCHAR Key)
{
  SHORT      ChoiceIndex  = Key - KEY_F1;
  MENUITEM * Choice       = & (FKeyItems[ChoiceIndex]);

  // if item is inactive, don't change current status
  if(! (Choice->Control & MENUITEM_INACTIVE))
    {
    /* Allow function pointers and active status to be changed, but */
    /* can't make completely generic. Still need to pass some specific */
    /* parameters */
    switch ((USHORT) Key)
      {
      case KEY_F1:  // help
      case KEY_F2:  // Live "GO"
      case KEY_F3:  // Live, plot, and command replay(?) "STOP" key
      case KEY_F4:  // Live, plot, and command replay(?) "PAUSE" key
      case KEY_F5:  // Live data capture key
      case KEY_F6:  // Graph zoom key
      case KEY_F7:  // Graph restore zoom key
      case KEY_F8:  // Graph autoscale key
      case KEY_F9:  // Graph change display window key
      case KEY_F10: // switch mode
      case KEY_F18: // SHIFT F8 (?) Autoscale Z Axis
      case KEY_F21: // show key info
      case KEY_F26: // CTRL F6 redraw tagged curves
      case KEY_F28: // CTRL F8 autoscale X axis
      case KEY_F31: // popup curve directory
      case KEY_F33: // Stop Keystroke
      case KEY_F34: // Next frame
      case KEY_F35: // ALT_F5 capture to background
      case KEY_F36: // ALT_F6 Expand on tagged toggle
      case KEY_F38: // ALT_F8 Autoscale Y axis
      case KEY_F39: // ALT_F9 Switch plot in window
      case KEY_F40: // Command Line
        if(ZoomState == FALSE || Key == KEY_F1 || Key == KEY_F31)
          {
          if (Choice->Control & MENUITEM_CALLS_FORM)
            run_form(Choice->SubMenu, &default_form_attributes, FALSE);
          else if(Choice->Control & MENUITEM_CALLS_FORMTABLE)
            run_form(* ((FORM **) Choice->SubMenu), & default_form_attributes, FALSE);
           else if (Choice->Control & MENUITEM_CALLS_FUNCTION)
              (*(Choice->Action))(0);
          }
      break;

      default:
      break;
    }
  }
  return Current.Form->status;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static UCHAR HandleCtrlHotKey(UCHAR Key)
{
   MENUITEM * Choice = & (CtrlHotKeyItems[ Key - KEY_CTRL_A ]);

   /* if item is inactive, don't do anything */
   if (   (! (Choice->Control & MENUITEM_INACTIVE))
        && (ZoomState == FALSE))
   {
      /* Allow function pointers and active status to be changed, but */
      /* can't make completely generic. Still need to pass some specific */
      /* parameters */
      switch ((USHORT) Key)
      {
         // NOTE : control-C is a special MSDOS "break" key
         case KEY_CTRL_T :           // tag curve group
         case KEY_CTRL_U :           // untag all curves
            if (Choice->Control & MENUITEM_CALLS_FORM)
               run_form(Choice->SubMenu, &default_form_attributes, FALSE);
            else if(Choice->Control & MENUITEM_CALLS_FORMTABLE)
               run_form(* ((FORM **) Choice->SubMenu),
                         & default_form_attributes, FALSE);
            else if (Choice->Control & MENUITEM_CALLS_FUNCTION)
               (*(Choice->Action))(0);
            break;

         default:
            (*(Choice->Action))(0);
            break;
      }
   }
   return Current.Form->status;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UCHAR HandleAltHotKey(UCHAR Key)
{
   SHORT ChoiceIndex = Key - KEY_ALT_A;
   MENUITEM *Choice = &(AltHotKeyItems[ChoiceIndex]);

   /* if item is inactive, don't do anything */
   if(   (! (Choice->Control & MENUITEM_INACTIVE))
       && (ZoomState == FALSE))
   {
      /* Allow function pointers and active status to be changed, but */
      /* can't make completely generic. Still need to pass some specific */
      /* parameters */
      switch ((USHORT) Key)
      {
         case KEY_ALT_X:    // exit, re-enable main menu, just in case
            if(* is_auto_peak)
               HotAutoPeakEnd();
         // NO break, fall thru to next case is deliberate
         case KEY_ALT_G:           // Cursor GoTo Form
         case KEY_ALT_P:           // Start peak finder
         case KEY_ALT_R:            // replot current window
         case KEY_ALT_S:            // replot current window
         case KEY_ALT_T:            // Tag this curve
         case KEY_ALT_U:            // Untag this curve
            if (Choice->Control & MENUITEM_CALLS_FORM)
               run_form(Choice->SubMenu, &default_form_attributes, FALSE);
            else if(Choice->Control & MENUITEM_CALLS_FORMTABLE)
               run_form(* ((FORM **) Choice->SubMenu),
                         & default_form_attributes, FALSE);
            else if (Choice->Control & MENUITEM_CALLS_FUNCTION)
               (*(Choice->Action))(0);
            break;

         default:
            (*(Choice->Action))(0);
            break;
      }
   }
   return Current.Form->status;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UCHAR HandleSpecialHotKey(UCHAR Key)
{
   USHORT     ChoiceIndex  = Key - KEY_ENTER;
   MENUITEM * Choice       = & (SpecialHotKeyItems[ChoiceIndex]);

   /* if item is inactive, don't do anything */
   if(! (Choice->Control & MENUITEM_INACTIVE))
   {
      // Keystroke record and playback will be handled by the called functions
      switch (ChoiceIndex + KEY_ENTER)
      {
         case KEY_ENTER:
         case KEY_ESCAPE:
         case KEY_TAB:
         case KEY_BACK_TAB:
         case KEY_BACKSPACE:
         case KEY_DELETE:
         case KEY_DELETE_FAR:
         case KEY_INSERT:
         case KEY_UP:
         case KEY_UP_FAR:
         case KEY_DOWN:
         case KEY_DOWN_FAR:
         case KEY_LEFT:
         case KEY_LEFT_FAR:
         case KEY_RIGHT:
         case KEY_RIGHT_FAR:
         case KEY_HOME:
         case KEY_HOME_FAR:
         case KEY_END:
         case KEY_END_FAR:
         case KEY_PG_UP:
         case KEY_PG_UP_FAR:
         case KEY_PG_DN:
         case KEY_PG_DN_FAR:
         case KEY_PLUS:
         case KEY_MINUS:
            if (Choice->Control & MENUITEM_CALLS_FORM)
               run_form(Choice->SubMenu, &default_form_attributes, FALSE);
            else if(Choice->Control & MENUITEM_CALLS_FORMTABLE)
               run_form(* ((FORM **) Choice->SubMenu),
                         & default_form_attributes, FALSE);
            else if (Choice->Control & MENUITEM_CALLS_FUNCTION)
               (*(Choice->Action))(0);
            break;
         }
   }
   return Current.Form->status;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UCHAR OMAKey(UCHAR Key)
{
   UCHAR ReturnStatus = Current.Form->status;

   if ((Key >= KEY_F1) && (Key <= KEY_F40))
      ReturnStatus = HandleFKey(Key);
   else if ((Key >= KEY_ENTER) && (Key <= KEY_MINUS))
      ReturnStatus = HandleSpecialHotKey(Key);
   else if ((Key > KEY_ALT_A) && (Key <= KEY_ALT_Z))  // ALT_A maps into F40
   {

// only needed for GRAPHICAL scan setup form
//      if(Key == KEY_ALT_G)
//         cursorEraseVBar();

      ReturnStatus = HandleAltHotKey(Key);

// only needed for GRAPHICAL scan setup form
//      if(Key == KEY_ALT_G)
//         cursorDrawVBar();  // only draws if VBar is enabled
   }
   else if((Key >= KEY_CTRL_A) && (Key <= KEY_CTRL_Z))
      ReturnStatus = HandleCtrlHotKey(Key);

   return ReturnStatus;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT MenuModeToggle(USHORT Dummy)
{
   Current.Form->status = FORMSTAT_SWITCH_MODE;
   DoLive = FALSE;
   return FALSE;
}

//--------------------------------------------------------------------------
//    UCHAR ShiftCheck(void)
//
//    Keyboard idle handler.  Will change function key display if any of the
//    SHIFT/ALT/CNTRL keys are pressed.
//
//--------------------------------------------------------------------------
UCHAR ShiftCheck(void)
{

#ifndef PROT

   SHORT shiftStatus = * ((PUSHORT) SHIFT_STATUS_ADDR);

   if(shiftStatus & ALT_FLAG)
   {
      if (CurrentShiftMode != ALT_FLAG)
      {
         ShowFKeys(&AltFKey);
         ShowHKeys(&AltHotKey);
         CurrentShiftMode = ALT_FLAG;
      }
   }
   else if(shiftStatus & CNTRL_FLAG)
   {
      // don't redraw if in command line form
      if ((CurrentShiftMode != CNTRL_FLAG) && (! isCurrentFormMacroForm()))
      {
         ShowFKeys(&ControlFKey);
         ShowHKeys(&CtrlHotKey);
         CurrentShiftMode = CNTRL_FLAG;
      }
   }
   else
   {
      // don't redraw if in command line form
      if ((CurrentShiftMode != 0) && (! isCurrentFormMacroForm()))
      {
         ShowFKeys(&FKey);
         ShowHKeys(&MainMenu);
         CurrentShiftMode = 0;
      }
   }

#else

   KBDINFO KbdInfo;

   KbdInfo.cb = sizeof(KbdInfo);
   KbdGetStatus(&KbdInfo, 0);

   if (KbdInfo.fsState & ALT_FLAG)
   {
      if (CurrentShiftMode != ALT_FLAG)
      {
         ShowFKeys(&AltFKey);
         ShowHKeys(&AltHotKey);
         CurrentShiftMode = ALT_FLAG;
      }
   }
   else if (KbdInfo.fsState & CNTRL_FLAG)
   {
      // don't redraw if in command line form
      if((CurrentShiftMode != CNTRL_FLAG) && (! isCurrentFormMacroForm()))
      {
         ShowFKeys(&ControlFKey);
         ShowHKeys(&CtrlHotKey);
         CurrentShiftMode = CNTRL_FLAG;
      }
   }
   else
   {
      // don't redraw if in command line form
      if((CurrentShiftMode != 0) && (! isCurrentFormMacroForm()))
      {
         ShowFKeys(&FKey);
         ShowHKeys(&MainMenu);
         CurrentShiftMode = 0;
      }
   }

#endif

   // use lowLevelKeyboardIdle() to invoke any other keyboard
   // idle loop functions.
   return lowLevelKeyboardIdle();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HotExit(USHORT Dummy)
{
   Paused = FALSE;         // just in case
   DoLive = FALSE;

   Current.Form->status = FORMSTAT_EXIT_ALL_FORMS;
   if (active_locus != LOCUS_APPLICATION)
      DoExit(0);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void GetHelp(USHORT Dummy)
{
   SHORT HelpIndex = GRAPH_HBASE; // default to in graph mode

   if (active_locus == LOCUS_FORMS)
      HelpIndex = Current.Field->help_index;
   if (active_locus == LOCUS_MENUS)     // in menu system
      HelpIndex =
              MenuFocus.ActiveMENU->ItemList[MenuFocus.ItemIndex].help_index;

   form_help_from_file(HelpIndex);

   if (pKSRecord != NULL)
   {
      if (*pKSRecord)
      {
         char Buf[ 20 ];

         sprintf(Buf, "HELP(%d);\n", HelpIndex);
         MacRecordString(Buf);
      }
   }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT SegmentString(PCHAR InBuffer, PCHAR *OutBuffer, SHORT MaxInStrLen,
                     SHORT MaxOutStrLen, SHORT MaxLines)
{
   PCHAR pTemp = InBuffer;
   CHAR  cTemp;
   SHORT i;
   SHORT Offset = 0;
   SHORT Lines  = 0;

   for (i=strlen(InBuffer); i<MaxInStrLen; i++)  // zero unused buffer
      InBuffer[i] = '\0';

   // size string for the window
   do
   {
      // save the current character
      cTemp = InBuffer[Offset + MaxOutStrLen];
      // set the saved charater's position to a NIL
      InBuffer[Offset + MaxOutStrLen] = NIL;

      // copy the next part of the buffer
      OutBuffer[Lines] = strdup(pTemp);
      InBuffer[Offset + MaxOutStrLen] = cTemp; // restore saved character

      Lines++;
      Offset += MaxOutStrLen;
      pTemp = &(InBuffer[Offset]);    // reset string pointer
   }
   while (((SHORT) strlen(pTemp) > MaxOutStrLen) && (Lines < MaxLines));

   // finish off the last of the buffer, if there is more
   if (strlen(pTemp) && (Lines < MaxLines))
   {
      OutBuffer[Lines] = strdup(pTemp);
      Lines++;
   }

   return Lines;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA AddCurveSetLines(CURVEDIR *CurveDir, SHORT EntryIndex,
                                BOOLEAN HighLight, PCHAR **CurveStrList,
                                PUSHORT *CurveStrAttr, PUCHAR *ItemStrCount)
{
  PVOID pTemp = * ItemStrCount;
  PCHAR DisplayLines[ 3 * MAX_MESSAGE_ROWS];
  UCHAR Line;
  CHAR Buffer[DESCRIPTION_LENGTH];
  USHORT i;

  *ItemStrCount = realloc(*ItemStrCount, CurveSetHelpItems + 1);
  if (*ItemStrCount == NULL)
    {
    *ItemStrCount = pTemp; // retrieve the old pointer
    return error(ERROR_ALLOC_MEM);
    }

  // form the lines for the curve name, description, start and count
  strcpy(Buffer, "   ");   // space for => line pointer, first line only
  strcat(Buffer, CurveDir->Entries[EntryIndex].path);
  strcat(Buffer, CurveDir->Entries[EntryIndex].name);
  for (i=strlen(Buffer); i<MAX_MESSAGE_LEN; i++)// spaces to max line width
    Buffer[i] = ' ';
  Buffer[i] = '\0';    // make sure that a terminator is present

  // divide up the name if it is too long for the window
  Line = (UCHAR) SegmentString(Buffer, DisplayLines, DESCRIPTION_LENGTH,
                                MAX_MESSAGE_LEN, MAX_MESSAGE_ROWS);

  // print out the description
  strcpy(Buffer, CurveDir->Entries[EntryIndex].descrip);
  // divide up the description if it is too long for the window
  Line += (UCHAR) SegmentString(Buffer, &DisplayLines[Line],
                                DESCRIPTION_LENGTH, MAX_MESSAGE_LEN,
                                MAX_MESSAGE_ROWS);

  // print out start and count and anything else that may be added later
  sprintf(Buffer, "Start %u    Count %u",
           CurveDir->Entries[EntryIndex].StartIndex,
           CurveDir->Entries[EntryIndex].count);
  // divide further attributes if the string too long for the window
  Line += (UCHAR) SegmentString(Buffer, &DisplayLines[Line],
                                DESCRIPTION_LENGTH, MAX_MESSAGE_LEN,
                                MAX_MESSAGE_ROWS);

  // Make the string list array larger
  pTemp = *CurveStrList;
  *CurveStrList = realloc(*CurveStrList,          // + 1 is for ending null
                          sizeof(PVOID) * (CurveSetHelpLines + Line + 1));

  if (*CurveStrList == NULL)
    {
    *CurveStrList = pTemp; // retrieve the old pointer
    for (i=0; i<Line; i++)
      free(DisplayLines[i]);   // free last allocated lines

    return error(ERROR_ALLOC_MEM);
    }

  // Make the line attribute list array larger
  pTemp = *CurveStrAttr;
  *CurveStrAttr = realloc(*CurveStrAttr,
                           sizeof(USHORT) * (CurveSetHelpLines + Line));

  if (*CurveStrAttr == NULL)
    {
    *CurveStrAttr = pTemp; // retrieve the old pointer
    for (i=0; i<Line; i++)
      free(DisplayLines[i]);   // free last allocated lines

    return error(ERROR_ALLOC_MEM);
    }

  for(i=0; i<Line; i++)
    {
    (*CurveStrList)[CurveSetHelpLines + i] = DisplayLines[i];
    (*CurveStrAttr)[CurveSetHelpLines + i] = 0;    // default attribute
    }

  // return the number of lines for this item
  (*ItemStrCount)[CurveSetHelpItems] = Line;
  CurveSetHelpItems++;
  CurveSetHelpLines += Line;
  (*CurveStrList)[CurveSetHelpLines] = NULL;         // terminating line
  return ERROR_NONE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void FreeCurveSetLines(PCHAR *CurveStrList, PUSHORT CurveStrAttr,
                        PUCHAR ItemStrCount)
{
  USHORT i;

  if (ItemStrCount != NULL)
    free(ItemStrCount);
  if (CurveStrAttr != NULL)
    free(CurveStrAttr);
  if (CurveStrList != NULL)
    {
    for (i=0; i<CurveSetHelpLines; i++)
      {
      if (CurveStrList[i] != NULL)
        free(CurveStrList[i]);
      }
    free(CurveStrList);
    }
  CurveSetHelpItems = 0;
  CurveSetHelpLines = 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void HighlightPlottedCurveSets(PUCHAR  ItemRowCount,
                                       PUSHORT CurveStrAttr,
                                       SHORT   PlotSetup)
{
   USHORT i, j;
   USHORT Row = 0;

   for (i=0; i<MainCurveDir.BlkCount; i++)
   {
      if (MainCurveDir.Entries[i].DisplayWindow & (1 << PlotSetup))
      {
         for (j=0; j<ItemRowCount[i]; j++)
            CurveStrAttr[Row + j] = STRATTR_HIGHLIGHT;
      }
      Row += ItemRowCount[i];
   }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static BOOLEAN PopDirPlotChoice(WINDOW * WindowPtr, UCHAR RowOffset,
                                SHORT RowCount, PUSHORT ItemAttr,
                                PUCHAR ItemRowCount, USHORT ItemCount,
                                USHORT ItemIndex, BOOLEAN Pointer, UCHAR Key)
{
  USHORT FirstRow = 0;
  USHORT i, j;

  switch (Key)
    {
    case SPACE:
    case KEY_PLUS:
    case KEY_MINUS:
    case KEY_LEFT:
    case KEY_RIGHT:                 // invert highlight and display bit
      for (i=0; i<ItemIndex; i++)   // get the rows for this item
        FirstRow += ItemRowCount[i];

      if (ItemAttr[FirstRow] != STRATTR_HIGHLIGHT)
        {
        for (j=0; j<ItemRowCount[ItemIndex]; j++)
          ItemAttr[FirstRow + j] = STRATTR_HIGHLIGHT;

        MainCurveDir.Entries[ItemIndex].DisplayWindow |= (1<<ActiveWindow);
        }
      else  // turn off plot setup
        {
        for (j=0; j<ItemRowCount[ItemIndex]; j++)
          ItemAttr[FirstRow + j] = 0;

        MainCurveDir.Entries[ItemIndex].DisplayWindow &=~(1<<ActiveWindow);
        }
      return TRUE;

    case KEY_ENTER:
    case KEY_ESCAPE:
      return KEY_EXCEPTION;
    }
  return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static BOOLEAN PopDirStringChoice(WINDOW * WindowPtr, UCHAR RowOffset,
                                  SHORT RowCount, PUSHORT ItemAttr,
                                  PUCHAR ItemRowCount, USHORT ItemCount,
                                  USHORT ItemIndex, BOOLEAN Pointer,
                                  UCHAR Key)
{
  CHAR Buffer[ DOSFILESIZE + DOSPATHSIZE + 1 ];
  USHORT FirstRow = 0, i, j;

  NewString = FALSE;

  switch (Key)
    {
    case SPACE:
    case KEY_PLUS:
    case KEY_MINUS:
    case KEY_LEFT:
    case KEY_RIGHT:                 // invert highlight and display bit
      for (i=0; i<ItemIndex; i++)   // get the rows for this item
        FirstRow += ItemRowCount[i];

      if (ItemAttr[FirstRow] != STRATTR_HIGHLIGHT)
        {
        for (j=0; j<ItemRowCount[ItemIndex]; j++)
          ItemAttr[FirstRow + j] = STRATTR_HIGHLIGHT;

        MainCurveDir.Entries[ItemIndex].DisplayWindow |= (1<<ActiveWindow);
        }
      else  // turn off plot setup
        {
        for (j=0; j<ItemRowCount[ItemIndex]; j++)
          ItemAttr[FirstRow + j] = 0;

        MainCurveDir.Entries[ItemIndex].DisplayWindow &=~(1<<ActiveWindow);
        }
    return FALSE;

    case KEY_ENTER:
      strcpy(Buffer, MainCurveDir.Entries[ItemIndex].path);
      strcat(Buffer, MainCurveDir.Entries[ItemIndex].name);
      string_to_field_string(Buffer);
      update_display_string();
      NewString = TRUE;

    case KEY_ESCAPE:
      return KEY_EXCEPTION;
    }
  return FALSE; /* ??? mlm 7-18-94 ??? */
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void GetPopCurveDir(USHORT Dummy)
{
  PCHAR          *CurveStrList;
  PUSHORT        CurveStrAttr;
  BOOLEAN        HighLight;
  USHORT         i;
  PUCHAR         ItemStrCount;
  ERR_OMA err;
  CharActFunct   *KeyHandler;
  PVOID          pTemp;

  CurveStrList = malloc(sizeof(PCHAR));    // seed for later realloc's
  CurveStrAttr = malloc(sizeof(USHORT));   // seed for later realloc's
  ItemStrCount = malloc(sizeof(UCHAR));    // seed for later realloc's

  if (!CurveStrList || !CurveStrAttr || !ItemStrCount)
    {
    FreeCurveSetLines(CurveStrList, CurveStrAttr, ItemStrCount);
    error(ERROR_ALLOC_MEM);
    return;
    }

  CurveStrList[0] = NULL; // initialize first line to be last line

  for (i=0; i<MainCurveDir.BlkCount; i++)
    {
    HighLight = (MainCurveDir.Entries[i].DisplayWindow & (i<<ActiveWindow))
      != 0;

    err = AddCurveSetLines(&MainCurveDir, i, HighLight, &CurveStrList,
      &CurveStrAttr, &ItemStrCount);
    if (err)
      {
      FreeCurveSetLines(CurveStrList, CurveStrAttr, ItemStrCount);
      return;
      }
    }

  // put in the no curves loaded message
  if (MainCurveDir.BlkCount == 0)
    {
    // Make the string list array larger
    pTemp = *CurveStrList;
    *CurveStrList = realloc(*CurveStrList,
      sizeof(PVOID) * 2);  // 1 is for ending null

    if (*CurveStrList == NULL)
      {
      *CurveStrList = pTemp; // retrieve the old pointer
      error(ERROR_ALLOC_MEM);
      }
    else
      {
      CurveStrList[0] = strdup(NoCurveSetStr);
      CurveStrList[1] = NULL;
      CurveStrAttr[0] = STRATTR_REGULAR;
      ItemStrCount[0] = 1;
      CurveSetHelpLines = 1;
      CurveSetHelpItems = 1;
      KeyHandler = NULL;
      }
    }
  else if (active_locus == LOCUS_APPLICATION)
    {
    RemoveGraphCursor();
    HighlightPlottedCurveSets(ItemStrCount, CurveStrAttr, ActiveWindow);
    KeyHandler = PopDirPlotChoice;
    }
  else if ((active_locus == LOCUS_FORMS) &&
    (Current.Field->type == FLDTYP_STRING))
    KeyHandler = PopDirStringChoice;
  else
    KeyHandler = NULL;      // use default handler

  title_message_window("Curve Directory", CurveStrList, CurveStrAttr,
                       ItemStrCount, MainCurveDir.BlkCount,
                       MAX_MESSAGE_ROWS, COLORS_MESSAGE, TRUE, KeyHandler);

  FreeCurveSetLines(CurveStrList, CurveStrAttr, ItemStrCount);

  if (active_locus == LOCUS_APPLICATION)
    InitCursor(ActiveWindow, CursorType);
  else if ((active_locus == LOCUS_FORMS) &&
    (Current.Field->type == FLDTYP_STRING) && NewString)
    {
    display_field_to_screen(FALSE);
    StandardVerify(KEY_ENTER);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UCHAR IsCursorInFKeys(SHORT Row, SHORT Column)
{
   MENUITEM *  ThisItem;
   SHORT ColumnInc = screen_columns / 5; // divide screen width into 5 fields
   SHORT TextLen;
   SHORT ColumnOffset;
   SHORT i, StartIndex;
   UCHAR ReturnKey;

   if ((Row >= FKEY_ROW) && (Row < (FKEY_ROW + FKEY_ROW_SIZE)))
   {
      ColumnOffset = 0;
      if (Row > FKEY_ROW)
         StartIndex = FKEY_ITEMS_PER_LINE;
      else
         StartIndex=0;

      for (i=StartIndex; i < StartIndex + FKEY_ITEMS_PER_LINE; i++)
      {
         ThisItem = &(CurrentFKeyMenu->ItemList[i]);
         TextLen = strlen(ThisItem->Text);

         if ((Column > ColumnOffset) &&
              (Column <= (ColumnOffset + TextLen)))
         {
            if(ThisItem->Control & MENUITEM_INACTIVE)
               return KEY_EXCEPTION;
            if(ZoomState != FALSE) {
               if(i != 0)
                  return KEY_EXCEPTION;
               else if(CurrentFKeyMenu == & ControlFKey)
                  return KEY_EXCEPTION;
               else if(CurrentFKeyMenu == & ShiftFKey)
                  return KEY_EXCEPTION;
            }

            // found it!
            ReturnKey = (UCHAR) (KEY_F1 + i);
            if (CurrentFKeyMenu == &ShiftFKey)
               ReturnKey += (UCHAR) 10;
            else if (CurrentFKeyMenu == &ControlFKey)
               ReturnKey += (UCHAR) 20;
            else if (CurrentFKeyMenu == &AltFKey)
               ReturnKey += (UCHAR) 30;

            return ReturnKey;
         }

         ThisItem->Column = (UCHAR) ColumnOffset;
         ColumnOffset += ColumnInc;
      }
   }
   return KEY_EXCEPTION;
}

// ------------------------------------------------------------------------
void reinit_FKeys(void)
{
   FKeyItems[1].TextLen = (char) 0;
   if (! ExpandedOnTagged)
      FKeyItems[1].Control &= ~ MENUITEM_INACTIVE;
   FKeyItems[1].Action = SwitchModeLiveKey;
   FKeyItems[2].Control |= MENUITEM_INACTIVE;
   FKeyItems[3].Text = PauseStr;
   FKeyItems[3].Action = PauseLive;
   FKeyItems[3].TextLen = (char) 0;
   FKeyItems[3].Control |= MENUITEM_INACTIVE;
   FKeyItems[4].Text = LiveCaptureStr;
   FKeyItems[4].Control |= MENUITEM_INACTIVE;
   FKeyItems[4].Action = CaptureLive;
   FKeyItems[5].Control |= MENUITEM_INACTIVE;
   FKeyItems[6].Control |= MENUITEM_INACTIVE;
   FKeyItems[7].Control |= MENUITEM_INACTIVE;
   FKeyItems[8].Control |= MENUITEM_INACTIVE;
   FKeyItems[9].Text = GraphModeStr;
   FKeyItems[9].TextLen = (char) 0;

   ShowFKeys(&FKey);
}
