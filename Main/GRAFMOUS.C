/* -----------------------------------------------------------------------
/
/  grafmous.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/grafmous.c_v   0.17   06 Jul 1992 10:29:36   maynard  $
/  $Log:   J:/logfiles/oma4000/main/grafmous.c_v  $
/
*/

#include <stdio.h>
#include <string.h>

#include "grafmous.h"
#include "fkeyfunc.h"
#include "multi.h"
#include "cursor.h"
#include "omazoom.h"
#include "macrecor.h"
#include "graphops.h"
#include "calib.h"
#include "autopeak.h"     // is_auto_peak
#include "tagcurve.h"
#include "wintags.h"
#include "plotbox.h"     // PLOTBOX
#include "baslnsub.h"
#include "live.h"      // DoLive
#include "pltsetup.h"
#include "ksindex.h"
#include "formtabs.h"
#include "omaform.h"   // isPlotShowingForm()
#include "spgraph.h"
#include "omaerror.h"
#include "forms.h"
#include "splitfrm.h"
// use the following only for GRAPHICAL scan setup
//#include "scanset.h"   // inScanSetup

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

#define X_MOVE    1
#define Z_MOVE    2

static CXY StartCursorDrag;
static FLOAT StartDragZVal;
static SHORT StartDragZOffset;
static FLOAT ZDragFactor;
static FLOAT StartDragXVal;
static SHORT StartDragXOffset;
static FLOAT XDragFactor;

static CXY LastZoomMousePos;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN MouseSetActiveWindow(SHORT XPos, SHORT YPos)
{
  SHORT i;

  if (active_locus == LOCUS_FORMS)
    {
    if(isPlotShowingForm(Current.Form->MacFormIndex))
      if((XPos >= Plots[CAL_PLOTBOX].fullarea.ll.x) &&
        (XPos < Plots[CAL_PLOTBOX].fullarea.ur.x) &&
        (YPos >= Plots[CAL_PLOTBOX].fullarea.ll.y)&&
        (YPos < Plots[CAL_PLOTBOX].fullarea.ur.y))
        {
        ActiveWindow = CAL_PLOTBOX;
        ActivePlot = &Plots[CAL_PLOTBOX];
        return TRUE;
        }
      return FALSE;
    }

  if(InSplitForm())
/* use the following only for GRAPHICAL scan setup          */
/* if(baslnsub_active() || InCalibForm || InM1235Form || * inScanSetup) */
    return FALSE;

  // find which graph the mouse was in
  for (i=0; i<fullscreen_count[window_style]; i++)
    {
    if ((XPos >= PlotWindows[i].ll.x) &&
      (XPos < PlotWindows[i].ur.x) &&
      (YPos >= PlotWindows[i].ll.y) &&
      (YPos < PlotWindows[i].ur.y))
      {
      ActiveWindow = i;
      ActivePlot = &Plots[ActiveWindow];
      return TRUE;
      }
    }
  return FALSE;
}

// if Direction == X_MOVE or Direction == Z_MOVE the cursor movement will
// be tacked to the curve.  If Direction == (X_MOVE | Z_MOVE) the cursor 
// will be free floating
PRIVATE void MoveGraphCursorToMouse(SHORT XPos, SHORT YPos, SHORT Direction)
{
  dCXY MouseXZVal;

  if (Direction == X_MOVE)
    {
    MouseXZVal.x = ((XPos - StartDragXOffset) * XDragFactor) +
      StartDragXVal;
    MouseXZVal.y = CursorStatus[ActiveWindow].Z;
    }

  if (Direction == Z_MOVE) // move in Z direction
    {
    MouseXZVal.y = ((YPos - StartDragZOffset) * ZDragFactor) +
      StartDragZVal;
    MouseXZVal.x = CursorStatus[ActiveWindow].X;
    }

  // check to see if free floating cursor
  if (Direction != (Z_MOVE | X_MOVE))
    {
    JumpCursor(MouseXZVal.x, MouseXZVal.y);
    if (pKSRecord != NULL)
      {
      if (*pKSRecord)
        {
        CHAR Buf[80];

        sprintf(Buf, "MOVE_CURSOR(%.7lg, %.7lg);\n",
          MouseXZVal.y, MouseXZVal.x);
        MacRecordString(Buf);
        }
      }
    }
  else
    {
    cursor_loc.x = XPos;
    cursor_loc.y = YPos;

    MouseXZVal = current_zoom_point();
    CursorStatus[ ActiveWindow ].X = MouseXZVal.x;

    if(Plots[ ActiveWindow ].style == FALSE_COLOR)
      CursorStatus[ ActiveWindow ].Z = MouseXZVal.y;
    else
      CursorStatus[ ActiveWindow ].Y = MouseXZVal.y;

    if (* is_auto_peak)
      auto_peak_draw_threshold(ActiveWindow, &CursorStatus[ActiveWindow].Y);
    SetCursorPos(ActiveWindow,
                 CursorStatus[ ActiveWindow ].X,
                 CursorStatus[ ActiveWindow ].Y,
                 CursorStatus[ ActiveWindow ].Z);

    UpdateCursorXStat(ActiveWindow);

    if(Plots[ ActiveWindow ].style == FALSE_COLOR)
      UpdateCursorZStat(ActiveWindow);
    else
      UpdateCursorYStat(ActiveWindow);
    }
}

BOOLEAN in_form(int form_index, SHORT Row, SHORT Column)
{
  FORM * pForm = FormTable[form_index];
  UCHAR Frow = (UCHAR)Row;
  UCHAR Fcol = (UCHAR)Column;

  return ((pForm == Current.Form) &&
          (Frow >= pForm->row) &&
          (Frow < pForm->row + pForm->size_in_rows) &&
          (Fcol >= pForm->column) &&
          (Fcol < pForm->column + pForm->size_in_columns));
}

/*******************************************************************/

UCHAR GraphMouseService(USHORT Buttons, SHORT Row, SHORT Column,
   SHORT XPos, SHORT YPos, BOOLEAN *LeftUp, BOOLEAN *RightUp,
   BOOLEAN *BothUp, BOOLEAN *XDragMode, BOOLEAN *ZDragMode,
   BOOLEAN *BothDrag)
{
  UCHAR ReturnKey = '\0';
  FORM * pForm = pForm;

  if (Buttons && (*RightUp || *LeftUp))
    {
    PLOTBOX * pPlot = &Plots[ActiveWindow];

    if ((active_locus == LOCUS_APPLICATION) &&
      !(*XDragMode || *ZDragMode))
      {
      if (!DebounceMouseKey())
        return NIL;
      }

    if (BothUp) // not in two key mode
      {
      if ((Buttons & MOUSE_LEFT_BUTTON) && (Buttons & MOUSE_RIGHT_BUTTON))
        {
        *BothUp = FALSE;
        if (pointOnGraphCursor(XPos, YPos))
          {
          if (is_tagged(ActiveWindow,
            CursorStatus[ActiveWindow].UTDisplayCurve))
            HotUntagCurve(0);
          else
            HotTagCurve(0);
          }
        else      // check to see if the mouse is in a graph window
          {        // if so addvance the plot setup
          if (MouseSetActiveWindow(XPos, YPos))
            SelectPlotForWindow(0);
          }
        }
      else if (Buttons & MOUSE_LEFT_BUTTON)
        {
        if (*LeftUp)
          {
          *XDragMode = FALSE;
          *LeftUp = FALSE;

          // no graph action if a popup is the active area
          // and have gotten to this point in search after
          // searching popup area
          if (active_locus == LOCUS_POPUP)
            {
            ErrorBeep();
            return NIL;
            }

          if ((ZoomState != FALSE) ||
            (baslnsub_active() &&
            (active_locus == LOCUS_APPLICATION) &&
            (XPos >= Plots[CAL_PLOTBOX].fullarea.ll.x) &&
            (XPos < Plots[CAL_PLOTBOX].fullarea.ur.x) &&
            (YPos >= Plots[CAL_PLOTBOX].fullarea.ll.y) &&
            (YPos < Plots[CAL_PLOTBOX].fullarea.ur.y)))
            {
            // treat like an enter, will need another for Zoom action
            cursor_loc.x = XPos;
            cursor_loc.y = YPos;
            return KEY_ENTER;
            }

          if ((ReturnKey = IsCursorInFKeys(Row, Column)) != KEY_EXCEPTION)
            return ReturnKey;

          if (DoLive)
            return NIL;

          if (pointOnGraphCursor(XPos, YPos))
            {
            FLOAT XRange = pPlot->x.max_value - pPlot->x.min_value;

            *XDragMode = TRUE;
            StartDragXVal = CursorStatus[ActiveWindow].X;
            StartDragXOffset = XPos;

            // factor to calculate X movement into X values
            if (pPlot->x.axis_end_offset.x)
              XDragFactor = XRange / pPlot->x.axis_end_offset.x;
            else
              XDragFactor = (float) 0.1;
            return NIL;
            }

          // check to see if the mouse is in a graph window
          if (MouseSetActiveWindow(XPos, YPos))
            {
            if (pKSRecord != NULL)
              {
              if (*pKSRecord)
                {
                CHAR Buf[ 30 ];
                sprintf(Buf, "SELECT_WINDOW(%d);\n", ActiveWindow);
                MacRecordString(Buf);
                }
              }

            // activate graph if not already in it
            if (active_locus != LOCUS_APPLICATION)
              return KEY_F10;
            else
              {
              if (! *is_auto_peak)
                {
                // make a new cursor active in the new window
                RemoveGraphCursor();
                InitCursor(ActiveWindow, CursorType);
                SetCursorPos(ActiveWindow,
                  CursorStatus[ActiveWindow].X,
                  CursorStatus[ActiveWindow].Y,
                  CursorStatus[ActiveWindow].Z);
                return NIL;
                }
              else
                return KEY_ENTER;
              }
            }

          // check to see if going after a menu item
          if ((ReturnKey = menus_left_button_handler((UCHAR) Row,
                           (UCHAR) Column)) != KEY_EXCEPTION)
            {
            GraphExitKey = ReturnKey;
            return ReturnKey;
            }

          // check to see if was in graph mode and need to go to a form
          if (gathering_calib_points ||
            (InSplitForm() && (active_locus != LOCUS_FORMS)))
            {
            ReturnKey = forms_left_button_handler((UCHAR)Row, (UCHAR)Column);

            if (ReturnKey != KEY_EXCEPTION)
              {
              if (active_locus != LOCUS_POPUP)
                {
                GraphExitKey = ReturnKey;
                ReturnKey = NIL;
                Current.Form->status = FORMSTAT_SWITCH_MODE;
                }
              }
            else
              {
              if(InSplitForm &&
                 (in_form(KSI_XCAL_FORM, Row, Column)   ||
               /* use the following only for GRAPHICAL scan setup              */
               /* in_form(KSI_SCAN_SETUP, Row, Column   || */
                  in_form(KSI_BASLN_FORM, Row, Column)  ||  
                  in_form(KSI_SPGRAPH_FORM, Row, Column)||
                  in_form(KSI_MATH_FORM, Row, Column)   ||
                  in_form(KSI_STAT_FORM, Row, Column)))
                {
                ReturnKey = KEY_EXCEPTION;
                }
              else
                {
                // need to set up for a cursor jump to the mouse 
                // position
                FLOAT XRange;
                CXY LocalLoc = GetCurrentCursorPosition(LOCUS_APPLICATION);

                StartDragXOffset = LocalLoc.x;
                StartDragXVal = CursorStatus[ActiveWindow].X;
                // factor to calculate X movement into X values
                XRange = pPlot->x.max_value - pPlot->x.min_value;
                if (pPlot->x.axis_end_offset.x)
                  XDragFactor = XRange / pPlot->x.axis_end_offset.x;
                else
                  XDragFactor = (float) 0.10;
                MoveGraphCursorToMouse(XPos, YPos, X_MOVE);
                ReturnKey = KEY_ENTER;
                }
              }
            }
          }
        else if (*XDragMode)
          {
          if (! *is_auto_peak)
            MoveGraphCursorToMouse(XPos, YPos, X_MOVE);
          else
            MoveGraphCursorToMouse(XPos, YPos, X_MOVE | Z_MOVE);
          return NIL;
          }

        }
      else if (Buttons & MOUSE_RIGHT_BUTTON)
        {
        if (*RightUp)
          {
          *RightUp = FALSE;
          *ZDragMode = FALSE;

          if ((ZoomState != FALSE) || (*is_auto_peak))
            {
            // treat like an escape
            cursor_loc.x = XPos;
            cursor_loc.y = YPos;
            return KEY_ESCAPE;
            }

          if(pointOnGraphCursor(XPos, YPos))
            {
            float ZRange = pPlot->z.max_value - pPlot->z.min_value;

            *ZDragMode = TRUE;
            StartDragZVal = CursorStatus[ActiveWindow].Z;
            StartDragZOffset = YPos;

            // factor to calculate Y movement into Z values
            if (pPlot->z.axis_end_offset.y)
              ZDragFactor = ZRange / pPlot->z.axis_end_offset.y;
            else if (pPlot->y.axis_end_offset.y)
              {
              ZRange = pPlot->y.max_value - pPlot->y.min_value;
              ZDragFactor = ZRange / pPlot->y.axis_end_offset.y;
              }
            else
              ZDragFactor = (float)0.1;
            }
          }
        else if (*ZDragMode)
          {
          MoveGraphCursorToMouse(XPos, YPos, Z_MOVE);
          return NIL;
          }
        }
      }
    else  // still possibly in two key mode, previously let up only one key
      {     // act as if just did both keys
      if ((Buttons & MOUSE_RIGHT_BUTTON) &&
        (Buttons & MOUSE_LEFT_BUTTON))
        {
        *RightUp = FALSE;
        *LeftUp = FALSE;
        if (pointOnGraphCursor(XPos, YPos))
          {
          if (is_tagged(ActiveWindow,
            CursorStatus[ActiveWindow].UTDisplayCurve))
            HotUntagCurve(0);
          else
            HotTagCurve(0);
          }
        else      // check to see if the mouse is in a graph window
          {        // if so addvance the plot setup
          if (MouseSetActiveWindow(XPos, YPos))
            SelectPlotForWindow(0);
          }
        *BothUp = FALSE;
        }
      }

    if (ReturnKey == KEY_EXCEPTION)
      {
      ReturnKey = NIL;
      ErrorBeep();
      }
    }
  else if ((ZoomState != FALSE) ||
    (baslnsub_active() && (active_locus == LOCUS_APPLICATION)))
    {
    if (InitializingZoom)      // first mouse check after starting a zoom
      {                          // or baseline subtract
      LastZoomMousePos.x = XPos;
      LastZoomMousePos.y = YPos;
      InitializingZoom = FALSE;
      }

    if ((XPos != LastZoomMousePos.x) || (YPos != LastZoomMousePos.y))
      {
      erase_mouse_cursor();
      LastZoomMousePos.x = XPos;
      LastZoomMousePos.y = YPos;
      zoomMouseMove(XPos, YPos);
      }
    }
  return ReturnKey;
}
