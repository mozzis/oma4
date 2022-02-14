/* -----------------------------------------------------------------------
/
/  graphops.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/graphops.c_v   0.16   30 Mar 1992 11:28:02   maynard  $
/  $Log:   J:/logfiles/oma4000/main/graphops.c_v  $
 * 
 *    Rev 1.4   26 Nov 1990 15:17:36   irving
 * The kbhit calls needed to be spaced further apart and with a delay from
 * any mouse action because of interference when running under DOS16M
 *
*/

#include <conio.h>
#include <stdio.h>

#include "graphops.h"
#include "omazoom.h"       // MoveCursorRight(), etc
#include "fkeyfunc.h"
#include "device.h"
#include "cursor.h"
#include "live.h"
#include "multi.h"
#include "calib.h"
#include "baslnsub.h"      // baslnsub_active()
#include "autopeak.h"      // is_auto_peak, etc.
#include "omaform.h"
#include "macrecor.h"
#include "macruntm.h"
#include "di_util.h"
#include "pltsetup.h"
#include "omamenu.h"
#include "doplot.h"        // ResizePlotForWindow()
#include "plotbox.h"
#include "curvdraw.h"      // Replot()
#include "curvedir.h"      // MainCurveDir
#include "forms.h"       
#include "barmenu.h"
#include "splitfrm.h"
#include "spgraph.h"

// the following is needed only for GRAPHICAL scan setup
//#include "scanset.h"    // inScanSetup
//#include "scangraf.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

int GraphExitKey = 0;
PRIVATE UCHAR LastKey = 0;

CCURHANDLE SystemCursor;

// define directions for autopeakMove()
enum { AUTO_LEFT, AUTO_RIGHT, AUTO_UP, AUTO_DOWN, AUTO_NONE };

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void autopeakMove(int direction)
{
  dCXY new_point;

  switch(direction)
    {
    case AUTO_LEFT  : MoveCursorLeft (0); break;
    case AUTO_RIGHT : MoveCursorRight(0); break;
    case AUTO_UP    : MoveCursorUp   (0); break;
    case AUTO_DOWN  : MoveCursorDown (0); break;
    }
     
  new_point = current_zoom_point();

  CursorStatus[ActiveWindow].X = new_point.x;
  CursorStatus[ActiveWindow].Y = new_point.y;

  if((direction == AUTO_UP) || (direction == AUTO_DOWN))
    auto_peak_draw_threshold(ActiveWindow, &CursorStatus[ActiveWindow].Y);

  UpdateCursorXStat(ActiveWindow);
  UpdateCursorYStat(ActiveWindow);
  SetCursorPos(ActiveWindow, CursorStatus[ActiveWindow].X,
               CursorStatus[ActiveWindow].Y,
               CursorStatus[ActiveWindow].Z);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void GraphLeft(USHORT Dummy)
{
   if(* is_auto_peak)
     autopeakMove(AUTO_LEFT);
   else
     MoveCursorXPos(ActiveWindow, FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void GraphRight(USHORT Dummy)
{
  if(* is_auto_peak)
    autopeakMove(AUTO_RIGHT);
  else
    MoveCursorXPos(ActiveWindow, TRUE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void GraphUp(USHORT Dummy)
{
  if(* is_auto_peak)
    autopeakMove(AUTO_UP);
  else
    MoveCursorZPos(ActiveWindow, TRUE, FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void GraphDown(USHORT Dummy)
{
  if(* is_auto_peak)
    autopeakMove(AUTO_DOWN);
  else
    MoveCursorZPos(ActiveWindow, FALSE, FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void GraphEscape(USHORT Dummy)
{
  if(* is_auto_peak)
    {
  // get out of auto peak mode BUT stay in graphics mode
  // disable the current curve for peak labels
    auto_peak_curve_enable(FALSE);
    auto_peak_terminate(ActiveWindow);
    HotAutoPeakEnd();
    }
  else
    Current.Form->status = FORMSTAT_SWITCH_MODE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void GraphEnter(USHORT Dummy)
{
  if(* is_auto_peak)
    {
    // enable the current curve for peak labels
    auto_peak_curve_enable(TRUE);
    auto_peak_terminate(ActiveWindow);
    HotAutoPeakEnd();
    return;
    }

  if (gathering_calib_points)
    get_point_for_calibration();
  else if (InM1235Form)
    GetOffsetPoint();

  if(baslnsub_active())
    add_graphics_cursor_knot();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ExpandAxis(AXISDATA * axis, float cpos, float factor)
{
  float xmin = axis->min_value,
        xmax = axis->max_value,
        xdist = ((xmax - xmin) / 2) * (1 - (factor / 100));

  axis->min_value = cpos - xdist;
  axis->max_value = cpos + xdist;

  if (! InLiveLoop)
    Replot(ActiveWindow);
  else
    {
    scalePlotbox(&(Plots[ActiveWindow]));
    LiveLoopAutoScaleAdjust(&(Plots[ActiveWindow]));
    draw_plotbox(&(Plots[ActiveWindow]));
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ContractAxis(AXISDATA * axis, float cpos, float factor)
{
  float xmin = axis->min_value,
        xmax = axis->max_value,
        xdist = ((xmax - xmin) / 2) / (1 - (factor / 100));

  axis->min_value = cpos - xdist;
  axis->max_value = cpos + xdist;

  if (! InLiveLoop)
    Replot(ActiveWindow);
  else
    {
    scalePlotbox(&(Plots[ActiveWindow]));
    LiveLoopAutoScaleAdjust(&(Plots[ActiveWindow]));
    draw_plotbox(&(Plots[ActiveWindow]));
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ExpandXAxis(USHORT Dummy)
{
  SHORT shiftStatus = * ((PUSHORT) SHIFT_STATUS_ADDR);
  PLOTBOX *pPlotBox = &(Plots[ActiveWindow]);
  float cpos = CursorStatus[WindowPlotAssignment[ActiveWindow]].X;

  if (pKSRecord != NULL)
    {
    if (*pKSRecord)
      MacRecordString("X_EXPAND();\n");
    }

   if(plotAreaShowing())
    {
    if (shiftStatus & (RIGHTSHIFT | LEFTSHIFT))
      ExpandAxis(&(pPlotBox->x), cpos, (float)50);
    else
      ExpandAxis(&(pPlotBox->x), cpos, (float)20);
    if(isCurrentFormMacroForm())
      RemoveGraphCursor();
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ContractXAxis(USHORT Dummy)
{
  SHORT shiftStatus = * ((PUSHORT) SHIFT_STATUS_ADDR);
  PLOTBOX *pPlotBox = &(Plots[ActiveWindow]);
  float cpos = CursorStatus[WindowPlotAssignment[ActiveWindow]].X;

  if (pKSRecord != NULL)
    {
    if (*pKSRecord)
      MacRecordString("X_CONTRACT();\n");
    }
   if(plotAreaShowing())
    {
    if (shiftStatus & (RIGHTSHIFT | LEFTSHIFT))
      ContractAxis(&(pPlotBox->x), cpos, (float)50);
    else
      ContractAxis(&(pPlotBox->x), cpos, (float)20);
    if(isCurrentFormMacroForm())
      RemoveGraphCursor();
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ExpandYAxis(USHORT Dummy)
{
  SHORT shiftStatus = * ((PUSHORT) SHIFT_STATUS_ADDR);
  PLOTBOX *pPlotBox = &(Plots[ActiveWindow]);
  float cpos = CursorStatus[WindowPlotAssignment[ActiveWindow]].Y;

  if (pKSRecord != NULL)
    {
    if (*pKSRecord)
      MacRecordString("Y_EXPAND();\n");
    }
  
   if(plotAreaShowing())
    {
    if (shiftStatus & (RIGHTSHIFT | LEFTSHIFT))
      ExpandAxis((pPlotBox->style == FALSE_COLOR) ?
        &(pPlotBox->z) : &(pPlotBox->y), cpos, (float)50);
    else
      ExpandAxis((pPlotBox->style == FALSE_COLOR) ?
        &(pPlotBox->z) : &(pPlotBox->y), cpos, (float)20);
    if(isCurrentFormMacroForm())
      RemoveGraphCursor();
    }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ContractYAxis(USHORT Dummy)
{
  SHORT shiftStatus = * ((PUSHORT) SHIFT_STATUS_ADDR);
  PLOTBOX *pPlotBox = &(Plots[ActiveWindow]);
  float cpos = CursorStatus[WindowPlotAssignment[ActiveWindow]].Y;

  if (pKSRecord != NULL)
    {
    if (*pKSRecord)
      MacRecordString("Y_CONTRACT();\n");
    }
  
   if(plotAreaShowing())
    {
    if (shiftStatus & (RIGHTSHIFT | LEFTSHIFT))
      ContractAxis((pPlotBox->style == FALSE_COLOR) ?
        &(pPlotBox->z) : &(pPlotBox->y), cpos, (float)50);
    else
      ContractAxis((pPlotBox->style == FALSE_COLOR) ?
        &(pPlotBox->z) : &(pPlotBox->y), cpos, (float)20);

    if(isCurrentFormMacroForm())
      RemoveGraphCursor();
    }
}

void MacExpandX(void)
{
  ExpandXAxis(0);
}
void MacExpandY(void)
{
  ExpandYAxis(0);
}
void MacContractX(void)
{
  ContractXAxis(0);
}
void MacContractY(void)
{
  ContractYAxis(0);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void InitGraphOps(void)
{
  GraphExitKey = 0;

  Current.Form->status = FORMSTAT_ACTIVE_FIELD;
  active_locus = LOCUS_APPLICATION;

  if (pKSRecord != NULL)
    {
    if (*pKSRecord && (Current.Form->status != FORMSTAT_EXIT_ALL_FORMS))
      MacRecordString("ENTER_GRAPH();\n");
    }

  FKeyItems[1].Action = GoLive;
  FKeyItems[5].Control &= ~MENUITEM_INACTIVE;
  FKeyItems[6].Control &= ~MENUITEM_INACTIVE;
  FKeyItems[7].Control &= ~MENUITEM_INACTIVE;
  if(!InSplitForm())
    // the following should be used only for GRAPHICAL scan setup
    //   if(! (baslnsub_active() || InCalibForm || InM1235Form || * inScanSetup))
    {
    // enable F9, select window, and Alt-F9, select window layout
    FKeyItems[8].Control &= ~MENUITEM_INACTIVE;
    FKeyItems[38].Control &= ~MENUITEM_INACTIVE;
    }
  FKeyItems[9].Text = MenuModeStr;
  FKeyItems[9].TextLen = (char) 0;

  // enable control-F6, redraw all tagged curves.
  FKeyItems[25].Control &= ~ MENUITEM_INACTIVE;
  // enable Alt-F6, Expand on tagged curves.
  FKeyItems[35].Control &= ~ MENUITEM_INACTIVE;
  // enable Ctrl-F8, Autoscale X axis
  FKeyItems[27].Control &= ~ MENUITEM_INACTIVE;
  // enable Alt-F8, Autoscale Y Axis
  FKeyItems[37].Control &= ~ MENUITEM_INACTIVE;
  // enable Shift-F8, Autoscale Z axis
  FKeyItems[17].Control &= ~ MENUITEM_INACTIVE;

  // enable control-U for untagging all curves.
  CtrlHotKeyItems[KEY_CTRL_U - KEY_CTRL_A].Control &= ~MENUITEM_INACTIVE;
  // enable control-T for tagging curve groups.
  CtrlHotKeyItems[KEY_CTRL_T - KEY_CTRL_A].Control &= ~MENUITEM_INACTIVE;

  AltHotKeyItems[KEY_ALT_G - KEY_ALT_A].Control &= ~ MENUITEM_INACTIVE;
  AltHotKeyItems[KEY_ALT_P - KEY_ALT_A].Control &= ~ MENUITEM_INACTIVE;

  // add ALT_T, ALT_U for tagging/untagging a curve.
  AltHotKeyItems[KEY_ALT_T - KEY_ALT_A].Control &= ~ MENUITEM_INACTIVE;
  AltHotKeyItems[KEY_ALT_U - KEY_ALT_A].Control &= ~ MENUITEM_INACTIVE;


// the following is only needed for GRAPHICAL scan setup
//   if(! (* isScanGrafMode))
//     {
  if (!baslnsub_active())
    {
    setSpecialHotKey(KEY_LEFT,  GraphLeft );
    setSpecialHotKey(KEY_RIGHT, GraphRight);
    setSpecialHotKey(KEY_UP,    GraphUp   );
    setSpecialHotKey(KEY_DOWN,  GraphDown );
    }
  else
    {
    setSpecialHotKey(KEY_LEFT,  MoveCursorLeft );
    setSpecialHotKey(KEY_RIGHT, MoveCursorRight);
    setSpecialHotKey(KEY_UP,    MoveCursorUp   );
    setSpecialHotKey(KEY_DOWN,  MoveCursorDown );
    }

  setSpecialHotKey(KEY_INSERT, ExpandXAxis);
  setSpecialHotKey(KEY_DELETE, ContractXAxis);
  setSpecialHotKey(KEY_PG_UP,  ExpandYAxis);
  setSpecialHotKey(KEY_PG_DN,  ContractYAxis);
  setSpecialHotKey(KEY_ESCAPE, GraphEscape);
  setSpecialHotKey(KEY_ENTER,  GraphEnter );

  //   }   // only needed for GRAPHICAL scan setup

  ShowFKeys(&FKey);

  // redraw all the plotboxes or the full screen

  // set up so that MoveCursor...() will work
  if(baslnsub_active())
    {
    omazoom_init();
    InitCursorStatus(ActiveWindow);
    MouseCursorEnable(FALSE);
    CDisplayCursor(screen_handle, cursor_loc);
    }
  else
    InitCursor(ActiveWindow, CursorType);

  if (DoLive)
    GoLive(0);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void GraphOpsKeyLoop(void)
{
  UCHAR key;
  USHORT LoopCount = 100;
  BOOLEAN KeyHit = FALSE;

  do
    {
    key = '\0';
#ifndef PROT
    if (LoopCount++ >= 100)
      {
      LoopCount = 0;
      SysWait(50);
      KeyHit = kbhit();
      }
#else
    KeyHit = kbhit();
#endif

    if ((poll_mouse_event != NULL) && (! KeyHit))
      {
      key = (*poll_mouse_event)();
      }

    if (KeyHit || (key != '\0'))
      {
      KeyHit = FALSE;
      if (key == '\0')
        key = get_FORM_key_input();

      if (key != LastKey)
        {
        CursorInc = 1;
        LastKey = key;
        }

      if((key >= KEYS_HIGH_BIT)|| ((key >= KEY_CTRL_A) && (key <= KEY_CTRL_Z)))
        OMAKey(key);
      else if (select_item_by_character(key))
        {
        if ((Current.Form->status != FORMSTAT_EXIT_TO_MENU1) &&
          (Current.Form->status != FORMSTAT_EXIT_TO_MENU2))
          Current.Form->status = FORMSTAT_SWITCH_MODE;
        GraphExitKey = key;
        }
      }
    else
      ShiftCheck();
    }
  while ((Current.Form->status != FORMSTAT_SWITCH_MODE) &&
    (Current.Form->status != FORMSTAT_EXIT_ALL_FORMS) &&
    (Current.Form->status != FORMSTAT_EXIT_TO_MENU1) &&
    (Current.Form->status != FORMSTAT_EXIT_TO_MENU2) && ! DoLive);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned char ExitGraphOps(void)
{
   if(* is_auto_peak)
      auto_peak_terminate(ActiveWindow);

   RemoveGraphCursor();

   FKeyItems[1].Action = SwitchModeLiveKey;
   FKeyItems[5].Control |= MENUITEM_INACTIVE;
   FKeyItems[6].Control |= MENUITEM_INACTIVE;
   FKeyItems[7].Control |= MENUITEM_INACTIVE;

   FKeyItems[8].Control |= MENUITEM_INACTIVE;
   FKeyItems[38].Control|= MENUITEM_INACTIVE;
   FKeyItems[9].Text = GraphModeStr;
   FKeyItems[9].TextLen = (char) 0;

   // disable control-F6, redraw all tagged curves
   FKeyItems[25].Control |= MENUITEM_INACTIVE;
   // disable Alt-F6, Expand on tagged curves
   FKeyItems[35].Control |= MENUITEM_INACTIVE;

   // disable Ctrl-F8, Autoscale X axis
   FKeyItems[27].Control |= MENUITEM_INACTIVE;
   // disable Alt-F8, Autoscale Y Axis
   FKeyItems[37].Control |= MENUITEM_INACTIVE;
   // disable Shift-F8, Autoscale Z Axis
   FKeyItems[17].Control |= MENUITEM_INACTIVE;

   // disable control-U for untagging all curves
   CtrlHotKeyItems[KEY_CTRL_U - KEY_CTRL_A].Control |= MENUITEM_INACTIVE;
   // disable control-T for tagging curve groups
   CtrlHotKeyItems[KEY_CTRL_T - KEY_CTRL_A].Control |= MENUITEM_INACTIVE;

   AltHotKeyItems[KEY_ALT_G - KEY_ALT_A].Control |= MENUITEM_INACTIVE;
   AltHotKeyItems[KEY_ALT_P - KEY_ALT_A].Control |= MENUITEM_INACTIVE;

   // add ALT_T, ALT_U for tagging/untagging a curve
   AltHotKeyItems[KEY_ALT_T - KEY_ALT_A].Control |= MENUITEM_INACTIVE;
   AltHotKeyItems[KEY_ALT_U - KEY_ALT_A].Control |= MENUITEM_INACTIVE;

   clearSpecialHotKey(KEY_LEFT);
   clearSpecialHotKey(KEY_RIGHT);
   clearSpecialHotKey(KEY_UP);
   clearSpecialHotKey(KEY_DOWN);
   clearSpecialHotKey(KEY_ESCAPE);
   clearSpecialHotKey(KEY_INSERT);
   clearSpecialHotKey(KEY_DELETE);
   clearSpecialHotKey(KEY_PG_UP);
   clearSpecialHotKey(KEY_PG_DN);
   clearSpecialHotKey(KEY_ENTER);

   ShowFKeys(&FKey);

   // erase menu area
   erase_screen_area(1, 0, 1, screen_columns,
       (unsigned char) ((ColorSets[COLORS_DEFAULT].regular.background << 4) |
       ColorSets[COLORS_DEFAULT].regular.foreground), FALSE);

   if (pKSRecord != NULL)
   {
      if (*pKSRecord)
         MacRecordString("LEAVE_GRAPH();\n");
   }

   return Current.Form->status;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UCHAR GraphOps(void)
{
   /* may already be initialized for graph mode by a macro */
  if (active_locus != LOCUS_APPLICATION)
    InitGraphOps();
  else
    DisplayActiveGraphCursor();

  do
    {
    if (DoLive)                // change to handle F2 in macro form
      GoLive(0);
    GraphOpsKeyLoop();
    }
  while ((Current.Form->status != FORMSTAT_SWITCH_MODE) &&
         (Current.Form->status != FORMSTAT_EXIT_ALL_FORMS) &&
         (Current.Form->status != FORMSTAT_EXIT_TO_MENU1) &&
         (Current.Form->status != FORMSTAT_EXIT_TO_MENU2));

  return ExitGraphOps();
}

/************************************************************************/
/* The purpose of this routine was changed (MLM Aug1993) from selecting */
/* the "current plot setup" to selecting the current window layout      */
/* This was done as part of removing the whole idea of multiple "plot   */
/* setups".  This concept was very confusing to the user, and also, it  */
/* seems, to the programmer, since the mechanisms used to select the    */
/* plot setup never worked very well and introduced bugs in the selec-  */
/* tion of the current plot window and plot window layout.  The change  */
/* was done by adding the lines with !!!'s at the end; several lines    */
/* were deleted as well, and who knows how much of the remaining code   */
/* here is redundant!                                                   */
/************************************************************************/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT SelectPlotForWindow(USHORT Dummy)
{

  BOOLEAN ReplaceGraphCursor = FALSE;

  if (++window_style > 9)   /* !!! */
    window_style = 0;       /* !!! */

  if (ActiveWindow > fullscreen_count[window_style]-1)  /* !!! */
    ActiveWindow = 0;                                   /* !!! */

  multiplot_setup(PlotWindows, &DisplayGraphArea, window_style); /* !!! */

  // limit to the number of general purpose plots
  WindowPlotAssignment[ActiveWindow] %= MAXPLOTS - 1;

  ActivePlot = &Plots[ActiveWindow];
  ResizePlotForWindow(ActiveWindow);

  if (!(((active_locus != LOCUS_FORMS) && (active_locus != LOCUS_POPUP)) ||
    ((isCurrentFormMacroForm()) && (isPrevFormMenu1() ||
    isPrevFormGraphWindow()))))
    return FALSE;

  if(isCurrentFormMacroForm())
    {
    ReplaceGraphCursor = TRUE;
    erase_cursor();
    SetGraphCursorType(CursorType);
    }

  PutUpPlotBoxes(); /* !!! */
  //  PlotScreen(0);    /* !!! */

  if (pKSRecord != NULL)
    {
    if (*pKSRecord)
      {
      CHAR Buf[30];

      sprintf(Buf, "S_ACTIVE_PLOT(%d);\n", ActiveWindow + 1);
      MacRecordString(Buf);
      }
    }

  RemoveGraphCursor();
  InitCursor(ActiveWindow, CursorType);
  SetCursorPos(ActiveWindow,
               CursorStatus[ActiveWindow].X,
               CursorStatus[ActiveWindow].Y,
               CursorStatus[ActiveWindow].Z);

  if((isCurrentFormMacroForm()) && (!isPrevFormGraphWindow()))
    RemoveGraphCursor();

  if (ReplaceGraphCursor)
    set_cursor_type(TextCursor);

  return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT SelectWindow(USHORT Dummy)
{
  CHAR Buf[30];
  BOOLEAN ReplaceGraphCursor = FALSE;

  // if plotarea is showing and in correct mode
  if (! (((active_locus != LOCUS_FORMS) && (active_locus != LOCUS_POPUP)) ||
    ((isCurrentFormMacroForm()) &&
    (isPrevFormMenu1() || isPrevFormGraphWindow()))))
    return FALSE;

  if (! MacroRunProgram)
    ActiveWindow++;
  // correct for circular effect and incorrect macro values
  ActiveWindow %= fullscreen_count[window_style];

  ActivePlot = &Plots[ActiveWindow];
  ResizePlotForWindow(ActiveWindow);

  if (pKSRecord != NULL)
    {
    if (*pKSRecord)
      {
      sprintf(Buf, "S_ACTIVE_WINDOW(%d);\n", ActiveWindow +1);
      MacRecordString(Buf);
      }
    }

  if(isCurrentFormMacroForm())
    {
    ReplaceGraphCursor = TRUE;
    erase_cursor();
    SetGraphCursorType(CursorType);
    }
  // make a new cursor active in the new window
  RemoveGraphCursor();
  InitCursor(ActiveWindow, CursorType);
  SetCursorPos(ActiveWindow,
                CursorStatus[ActiveWindow].X,
                CursorStatus[ActiveWindow].Y,
                CursorStatus[ActiveWindow].Z);
  if((isCurrentFormMacroForm()) && (!isPrevFormGraphWindow()))
    RemoveGraphCursor();
  if (ReplaceGraphCursor)
    set_cursor_type(TextCursor);
  return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HotPlotWindow(USHORT Dummy)
{
  if(plotAreaShowing())
    {
    if (pKSRecord != NULL)
      {
      if (*pKSRecord)
        MacRecordString("REPLOT_WINDOW();\n");
      }

    erase_mouse_cursor();
    if ((active_locus != LOCUS_MENUS) && ((active_locus != LOCUS_FORMS) ||
      ((isCurrentFormMacroForm()) && (isPrevFormGraphWindow()))))
      {
      if(isCurrentFormMacroForm())
        {
        erase_cursor();
        SetGraphCursorType(CursorType);
        }
      }
    RemoveGraphCursor();
    ReplotCurvesOnly(ActiveWindow);
    DisplayActiveGraphCursor();
    if(isCurrentFormMacroForm())
      set_cursor_type(TextCursor);
    replace_mouse_cursor();
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HotPlotScreen(USHORT Dummy)
{
  BOOLEAN ReplaceGraphCursor = FALSE;

  if(plotAreaShowing())
    {
    if (pKSRecord != NULL)
      {
      if (*pKSRecord)
        MacRecordString("REPLOT_SCREEN();\n");
      }

    erase_mouse_cursor();
    if ((active_locus != LOCUS_MENUS) && ((active_locus != LOCUS_FORMS) ||
      ((isCurrentFormMacroForm()) && (isPrevFormGraphWindow()))))
      {
      ReplaceGraphCursor = TRUE;
      if (isCurrentFormMacroForm())
        {
        erase_cursor();
        SetGraphCursorType(CursorType);
        }
      RemoveGraphCursor();
      }

    PutUpPlotBoxes();
    PlotScreen(0);
    if (ReplaceGraphCursor)
      {
      SetCursorPos(ActiveWindow, CursorStatus[ActiveWindow].X,
                                 CursorStatus[ActiveWindow].Y,
                                 CursorStatus[ActiveWindow].Z);
      if (isCurrentFormMacroForm())
        set_cursor_type(TextCursor);
      }
    replace_mouse_cursor();
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HotAutoPeakStart(USHORT Dummy)
{
   if (Plots[ActiveWindow].style == FALSE_COLOR)
      return;

   auto_peak_start(ActiveWindow);
   omazoom_init();          // set up for omazoom cursor motion
   autopeakMove(AUTO_NONE);
   AltHotKeyItems['P'-'A'].Control |= MENUITEM_INACTIVE;
   AltHotKeyItems['G'-'A'].Control |= MENUITEM_INACTIVE;
   // switch out of graphics mode while doing autopeak not allowed
   FKeyItems[9].Control |= MENUITEM_INACTIVE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HotAutoPeakEnd(void)
{
   int i;

   AltHotKeyItems['P'-'A'].Control &= ~MENUITEM_INACTIVE;
   AltHotKeyItems['G'-'A'].Control &= ~MENUITEM_INACTIVE;

   FKeyItems[9].Control &= ~MENUITEM_INACTIVE;
   ShowFKeys(& FKey);

   // Main menu may be disabled by omazoom_init() which is called by
   // HotAutoPeakStart(), so enable it again.
   for(i = 0; i < MainMenu.ItemCount; i ++)
      MainMenu.ItemList[i].Control &= ~ MENUITEM_INACTIVE;
      
   redraw_menu();
   unhighlight_menuitem(MenuFocus.ItemIndex);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacEnterGraph(void)
{
   if (active_locus != LOCUS_APPLICATION)
   {
      if (InCalibForm)                    
         GetNewPointsInit();
      if(baslnsub_active())
        {
        Current.Form->status = FORMSTAT_ACTIVE_FIELD;
        erase_cursor();
        if(push_form_context())
          setCurrentFormToGraphWindow(); // no error
        }
      InitGraphOps();
   }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacLeaveGraph(void)
{
   if(active_locus != LOCUS_APPLICATION)
      return;
   
   if (InCalibForm)                    
      GetNewPointsExit();
   if(baslnsub_active())
    {
    UCHAR attribute = specialGrafPrepareForExit();

    erase_screen_area(1, 0, 1, screen_columns, attribute, FALSE);
    redraw_menu();
    UnselectifyMenu();

    specialGrafReturnToForm();
    }

   ExitGraphOps();
   if(!InSplitForm())
// the following should be used only for GRAPHICAL scan setup
//   if(! (baslnsub_active() || InCalibForm || InM1235Form || * inScanSetup))
      active_locus = LOCUS_MENUS;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacReplotWindow(void)
{
  HotPlotWindow(0);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacReplotScreen(void)
{
  HotPlotScreen(0);
}
