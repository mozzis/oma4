/* -----------------------------------------------------------------------
/
/  omazoom.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/omazoom.c_v   0.20   06 Jul 1992 10:35:26   maynard  $
/  $Log:   J:/logfiles/oma4000/main/omazoom.c_v  $
*/

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

#ifdef PROT
   #define INCL_KBD
   #include <os2.h>
#endif

#define SPEEDFACTOR        0.13  

#define GSS_ENTER 13

#include <conio.h>
#include <sys\types.h>
#include <sys\timeb.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "omazoom.h"
#include "plotbox.h"
#include "fkeyfunc.h"
#include "graphops.h"
#include "doplot.h"
#include "curvdraw.h"
#include "device.h"
#include "cursor.h"
#include "multi.h"
#include "macrecor.h"   
#include "live.h"
#include "baslnsub.h"   // baslnsub_active()
#include "omamenu.h"
#include "syserror.h"   // ERROR_ALLOC_MEM
#include "handy.h"
#include "omaform.h"
#include "forms.h"

static SHORT OldCursorMode; 
static CLINEREPR PolyLineRepr;            

CXY cursor_loc;
static CDVHANDLE device_handle;
static CRECT plotarea;
//static USHORT CursorInc;
/* global version of CursorInc defined in cursor.c (and fshell.c ??) */

SHORT ZoomState = FALSE;      
static CRECT ZoomRect;               

BOOLEAN InitializingZoom = FALSE;
static BOOLEAN OldLivePaused;
static BOOLEAN OldDoLive;

// initialize module variables so that MoveCursor...() functions will work
// sets cursor_loc, device_handle, and plotarea[]
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void omazoom_init(void)
{
   plotarea.ll = ActivePlot->fullarea.ll;
   plotarea.ur = ActivePlot->fullarea.ur;

   /* get cursor location */
   cursor_loc = gss_position(ActivePlot, CursorStatus[ActiveWindow].X,
                                         CursorStatus[ActiveWindow].Y,
                                         CursorStatus[ActiveWindow].Z);

   if (cursor_loc.x < ActivePlot->plotarea.ll.x)
      cursor_loc.x = ActivePlot->plotarea.ll.x;
   if (cursor_loc.y < ActivePlot->plotarea.ll.y)
      cursor_loc.y = ActivePlot->plotarea.ll.y;
   if (cursor_loc.x > ActivePlot->plotarea.ur.x)
      cursor_loc.x = ActivePlot->plotarea.ur.x;
   if (cursor_loc.y > ActivePlot->plotarea.ur.y)
      cursor_loc.y = ActivePlot->plotarea.ur.y;

   /* get device handle */
   device_handle = deviceHandle();
   InitializingZoom = TRUE;

   OldLivePaused = Paused;
   OldDoLive = DoLive;
   Paused = TRUE;

   if(MenuFocus.ActiveMENU == & MainMenu) {
      int i;
      char tmpChar = MainMenu.ItemList[MenuFocus.ItemIndex].SelectCharOffset;
      
      UnselectifyMenu();
      for(i = 0; i < MainMenu.ItemCount; i ++)
         MainMenu.ItemList[i].Control |= MENUITEM_INACTIVE;

      MainMenu.ItemList[MenuFocus.ItemIndex].SelectCharOffset = -1;
      unhighlight_menuitem(MenuFocus.ItemIndex);
      MainMenu.ItemList[MenuFocus.ItemIndex].SelectCharOffset = tmpChar;
   }
}

/****************************************************************************/
PRIVATE dCXY gss_to_xydata(PLOTBOX * plot, int gssx, int gssy, float zvalue)
{
   int gssxrange = plot->x.axis_end_offset.x;
   int gssyrange = plot->y.axis_end_offset.y;
   float xmin    = plot->x.min_value;
   float ymin    = plot->y.min_value;
   float xmax    = plot->x.max_value;
   float ymax    = plot->y.max_value;
   float xrange  = xmax - xmin;
   float yrange  = ymax - ymin;
   dCXY xydatapts;

   if((plot->style != FALSE_COLOR) && (plot->z_position != NOSIDE))
   {
      float zrange   = plot->z.max_value - plot->z.min_value;
      float percentz = zvalue - plot->z.min_value;

      // don't divide by zero
      if(zrange)
         percentz = percentz / zrange;

      gssx -= (int) (percentz * plot->z.axis_end_offset.x);
      gssy -= (int) (percentz * plot->z.axis_end_offset.y);

      if(plot->z_position == LEFTSIDE)
         gssx += plot->z.axis_end_offset.x;
   }
   gssx -= plot->plotarea.ll.x;
   gssy -= plot->plotarea.ll.y;

   xydatapts.x = gssx * xrange / gssxrange + xmin;
   xydatapts.y = gssy * yrange / gssyrange + ymin;

   return xydatapts;
}

// return the X-Y coordinate of the current cursor location
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
dCXY current_zoom_point(void)
{
   return gss_to_xydata(& Plots[ ActiveWindow ], cursor_loc.x,
                         cursor_loc.y, CursorStatus[ ActiveWindow ].Z);
}

/****************************************************************************/
PRIVATE void CheckAcceleration(int key)
{
   FLOAT TempSpd;
   USHORT TimeDiff;/* time difference in milliseconds since last key press */
   struct timeb Time;
   static struct timeb LastTime = {0, 0, 0, 0};
   static int last_key = 0;
#ifdef PROT
   KBDINFO KbdInfo;
#else
   PUSHORT pShiftStatus = (PUSHORT) SHIFT_STATUS_ADDR;
#endif

   /* check for too long between keys */
   ftime(&Time);
   /* let it truncate as needed */
   TimeDiff = (USHORT) ((Time.time - LastTime.time) * 1000) +
   (USHORT) (Time.millitm - LastTime.millitm);

   if (TimeDiff > KeyRepeatTm)
      // set to move left one device pixel
      CursorInc = adjustYbyDCOffset(0, 1);
   memcpy(&LastTime, &Time, sizeof(Time));

   /* check for shift status, if shifted, speed up cursor */
#ifndef PROT
   if (*pShiftStatus & (LEFTSHIFT | RIGHTSHIFT))
#else
   KbdInfo.cb = sizeof(KbdInfo);     
   KbdGetStatus(&KbdInfo, 0);
   if (KbdInfo.fsState & (RIGHTSHIFT | LEFTSHIFT))
#endif
   {
      /* don't let the cursor increment get too big */
      
      if ((int)CursorInc < ((plotarea.ur.y - plotarea.ll.y) >> 2))
      {
         TempSpd = (FLOAT) CursorInc * (FLOAT) SPEEDFACTOR;
         if (TempSpd < (FLOAT) 1.0)
            TempSpd = (FLOAT) 1.0;
         CursorInc += (SHORT) TempSpd;
      }
      
      if ((int)CursorInc > ((plotarea.ur.y - plotarea.ll.y) >> 2))
         CursorInc = (plotarea.ur.y - plotarea.ll.y) >> 2;
   }

   if(key != last_key)
      // set to move left one device pixel
      CursorInc = adjustYbyDCOffset(0, 1);
   last_key = key;  // remember for the next time
}

/****************************************************************************/

// erase previous zoom box and draw new one
void DrawZoomBox(CRECT *ZoomRect, BOOLEAN Init)
{
   CPIXOPS SelMode, OldMode;
   static CXY RectanglePts[5];

   CLINETYPE SelType;
   CCOLOR SelColor;

   // set writing mode and line type and color for a dashed box
   CSetLineType(device_handle, CLN_LongDashed, &SelType);
   CSetLineColor(device_handle, BRT_YELLOW, &SelColor);
   CSetBgColor(device_handle, ActivePlot->background_color,
      &SelColor);    /* 'set color */

   CInqWritingMode(screen_handle, &OldMode);
   CSetWritingMode(screen_handle, CdXORs, &SelMode);

   if (! Init)
      CPolyline(screen_handle, 5, RectanglePts);

   RectanglePts[0] = ZoomRect->ll;
   RectanglePts[1].x = ZoomRect->ll.x;
   RectanglePts[1].y = ZoomRect->ur.y;
   RectanglePts[2] = ZoomRect->ur;
   RectanglePts[3].x = ZoomRect->ur.x;
   RectanglePts[3].y = ZoomRect->ll.y;
   RectanglePts[4] = ZoomRect->ll;

   CPolyline(screen_handle, 5, RectanglePts);

   // reset writing mode and line type and color
   CSetWritingMode(screen_handle, OldMode, &SelMode);
   CSetLineType(device_handle, PolyLineRepr.Type, &SelType);
   CSetLineColor(device_handle, PolyLineRepr.Color, &SelColor);
}

// assumes tha a Zoom box really is present!!!
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void EraseZoomBox(CRECT *ZoomRect)
{
   DrawZoomBox(ZoomRect, TRUE);
}

// Update the cursor status line with new X and Y values
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void ZoomDisplayCursorStatus(CXY CursorLoc)
{
   dCXY XYData = gss_to_xydata(ActivePlot, CursorLoc.x, CursorLoc.y,
                                        CursorStatus[ActiveWindow].Z);
   CursorStatus[ActiveWindow].X = XYData.x;
   UpdateCursorXStat(ActiveWindow);

   if(Plots[ ActiveWindow ].style != FALSE_COLOR) {
      CursorStatus[ ActiveWindow ].Y = XYData.y;
      UpdateCursorYStat(ActiveWindow);
   }
   else {
      CursorStatus[ ActiveWindow ].Z = XYData.y;
      UpdateCursorZStat(ActiveWindow);
   }
}

/****************************************************************************/
void MoveCursorLeft(USHORT Dummy) 
{
   LONG lTemp;                      

   DumpKeyBuffer(SK_LEFT_ARROW);                         
   DumpKeyBuffer(SK_LEFT_ARROW | LOCATION_CURSOR_PAD);   
   CheckAcceleration(KEY_LEFT);
   lTemp = (LONG) cursor_loc.x - (LONG) CursorInc;

   if (lTemp > (LONG) plotarea.ur.x)
      cursor_loc.x = plotarea.ur.x;
   else if (lTemp < (LONG) plotarea.ll.x)
      cursor_loc.x = plotarea.ll.x;
   else cursor_loc.x = (SHORT) lTemp;

   erase_mouse_cursor();                   
   if (ZoomState == ZOOM_CHOOSE_PT2)
   {
      ZoomRect.ur = cursor_loc;
      DrawZoomBox(&ZoomRect, FALSE);
   }

   ZoomDisplayCursorStatus(cursor_loc);

   CDisplayCursor(device_handle, cursor_loc);

   return;

   Dummy;                           
}
/****************************************************************************/
void MoveCursorRight(USHORT Dummy)   
{
   LONG lTemp;

   DumpKeyBuffer(SK_RIGHT_ARROW);                         
   DumpKeyBuffer(SK_RIGHT_ARROW | LOCATION_CURSOR_PAD);   

   CheckAcceleration(KEY_RIGHT);
   lTemp = (LONG) cursor_loc.x + (LONG) CursorInc;

   if (lTemp > (LONG) plotarea.ur.x)      
      cursor_loc.x = plotarea.ur.x;
   else if (lTemp < (LONG) plotarea.ll.x)
      cursor_loc.x = plotarea.ll.x;
   else cursor_loc.x = (SHORT) lTemp;

   erase_mouse_cursor();                   
   if (ZoomState == ZOOM_CHOOSE_PT2)
   {
      ZoomRect.ur = cursor_loc;
      DrawZoomBox(&ZoomRect, FALSE);
   }

   ZoomDisplayCursorStatus(cursor_loc);

   CDisplayCursor(device_handle, cursor_loc);  
   return;

   Dummy;            
}
/****************************************************************************/
void MoveCursorUp(USHORT Dummy)      
{
   LONG lTemp;                                 

   DumpKeyBuffer(SK_UP_ARROW);                      
   DumpKeyBuffer(SK_UP_ARROW | LOCATION_CURSOR_PAD);

   CheckAcceleration(KEY_UP);

   lTemp = (LONG) cursor_loc.y + (LONG) CursorInc;

   if (lTemp > (LONG) plotarea.ur.y)      
      cursor_loc.y = plotarea.ur.y;
   else if (lTemp < (LONG) plotarea.ll.y)
      cursor_loc.y = plotarea.ll.y;
   else cursor_loc.y = (SHORT) lTemp;

   erase_mouse_cursor();                   
   if (ZoomState == ZOOM_CHOOSE_PT2)
   {
      ZoomRect.ur = cursor_loc;
      DrawZoomBox(&ZoomRect, FALSE);
   }

   ZoomDisplayCursorStatus(cursor_loc);

   CDisplayCursor(device_handle, cursor_loc);     
   return;

   Dummy;                        
}
/****************************************************************************/
void MoveCursorDown(USHORT Dummy)    
{
   LONG lTemp;                   

   DumpKeyBuffer(SK_DOWN_ARROW);                      
   DumpKeyBuffer(SK_DOWN_ARROW | LOCATION_CURSOR_PAD);
   CheckAcceleration(KEY_DOWN);

   lTemp = (LONG) cursor_loc.y - (LONG) CursorInc;

   if (lTemp > (LONG) plotarea.ur.y)      
      cursor_loc.y = plotarea.ur.y;
   else if (lTemp < (LONG) plotarea.ll.y)
      cursor_loc.y = plotarea.ll.y;
   else cursor_loc.y = (SHORT) lTemp;

   erase_mouse_cursor();                   
   if (ZoomState == ZOOM_CHOOSE_PT2)
   {
      ZoomRect.ur = cursor_loc;
      DrawZoomBox(&ZoomRect, FALSE);
   }

   ZoomDisplayCursorStatus(cursor_loc);

   CDisplayCursor(device_handle, cursor_loc);  
   return;

   Dummy;                              
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void LeaveZoom()
{
   dCXY new_point;
   CXY CursorLoc;

   ZoomState = FALSE;         

   if(baslnsub_active())
   {
      MoveCursorUp(0);
      MoveCursorDown(0);
      new_point = current_zoom_point();
      CursorStatus[ ActiveWindow ].X = new_point.x;
      CursorStatus[ ActiveWindow ].Y = new_point.y;
   }

   // restore the cursor actions to previous values
   restoreGraphicsSpecialHotKeys();
   
   Paused = OldLivePaused;
   DoLive = OldDoLive;

   ShowFKeys(&FKey);

   CursorLoc = gss_position(&(Plots[ActiveWindow]),
               CursorStatus[ActiveWindow].X,
               CursorStatus[ActiveWindow].Y,
               CursorStatus[ActiveWindow].Z);

   MouseCursorEnable(TRUE);             
   replace_mouse_cursor();                

   ForceCursorIntoWindow(ActiveWindow); 
   if (!DoLive)
      SetCursorPos(ActiveWindow, CursorStatus[ActiveWindow].X,
                    CursorStatus[ActiveWindow].Y,
                    CursorStatus[ActiveWindow].Z);

   UpdateCursorMode(OldCursorMode);

   if(MenuFocus.ActiveMENU == & MainMenu) {

      int i;
      for(i = 0; i < MainMenu.ItemCount; i ++)
         MainMenu.ItemList[i].Control &= ~ MENUITEM_INACTIVE;
      
      redraw_menu();
      unhighlight_menuitem(MenuFocus.ItemIndex);
   }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void HandleZoomEnter(USHORT Dummy)
{
   CLINETYPE SelType;
   CCOLOR SelColor;

   if (ZoomState == ZOOM_CHOOSE_PT1)   
   {
      ZoomRect.ll = cursor_loc;
      ZoomRect.ur = cursor_loc;
      ZoomState = ZOOM_CHOOSE_PT2;
      DrawZoomBox(&ZoomRect, TRUE);     
   }
   else  // got a second point
   {
      ZoomState = FALSE;

      CSetLineType(device_handle, PolyLineRepr.Type, &SelType);
      CSetLineColor(device_handle, PolyLineRepr.Color, &SelColor);
      ZoomToRect(ActiveWindow, &ZoomRect);
      LeaveZoom();
   }

   Dummy;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void HandleZoomEscape(USHORT Dummy)     
{
   if (ZoomState == ZOOM_CHOOSE_PT1)
      LeaveZoom();
   else      // choosing point 2
   {
      EraseZoomBox(&ZoomRect);
      ZoomState = ZOOM_CHOOSE_PT1;
   }

   Dummy;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Zoom(USHORT Dummy)                 
{
   PLOTBOX * the_plotbox = & Plots[ ActiveWindow ];

   if (DoLive)                
   {
      InitCursor(ActiveWindow, CursorType);
      SetCursorPos(ActiveWindow, CursorStatus[ActiveWindow].X,
                                  CursorStatus[ActiveWindow].Y,
                                  CursorStatus[ActiveWindow].Z);
   }

   OldCursorMode = CursorMode;      
   UpdateCursorMode(CURSORMODE_ZOOM); /* place mode in ZOOM mode */
   /* get range of cursor movements */

   omazoom_init();

   CInqLineRepr(device_handle, &PolyLineRepr);

   // save current cursor actions
   saveGraphicsSpecialHotKeys();
   
   /* replace cursor actions */
   setSpecialHotKey(KEY_LEFT,   MoveCursorLeft  );
   setSpecialHotKey(KEY_RIGHT,  MoveCursorRight );
   setSpecialHotKey(KEY_UP,     MoveCursorUp    );
   setSpecialHotKey(KEY_DOWN,   MoveCursorDown  );
   setSpecialHotKey(KEY_ENTER,  HandleZoomEnter );
   setSpecialHotKey(KEY_ESCAPE, HandleZoomEscape);

   ZoomState = ZOOM_CHOOSE_PT1;
   ShowFKeys(&FKey);      

   RemoveGraphCursor();             
   MouseCursorEnable(FALSE);      

   CDisplayCursor(device_handle, cursor_loc);     
}

/****************************************************************************/
SHORT RestoreZoom(USHORT Dummy)
{
   dCXY new_point;

   initAxisToOriginal(& ActivePlot->x);
   initAxisToOriginal(& ActivePlot->y);
   initAxisToOriginal(& ActivePlot->z);

   scale_axis(&ActivePlot->x);
   scale_axis(&ActivePlot->y);
   scale_axis(&ActivePlot->z);

   draw_plotbox(ActivePlot);

   if (!DoLive)
      Replot(ActiveWindow);

   if(baslnsub_active())
   {
      new_point = current_zoom_point();
      CursorStatus[ ActiveWindow ].X = new_point.x;
      CursorStatus[ ActiveWindow ].Y = new_point.y;
   }

   ShowFKeys(&FKey);        

   if(pKSRecord && * pKSRecord)          
      MacRecordString("RESTORE_ZOOM();\n");

   return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA ZoomToRect(SHORT Window, CRECT * ZoomRect)
{
   ERR_OMA err = ERROR_NONE;
   dCXY data1,data2;
   SHORT PlotIndex = WindowPlotAssignment[Window];           
   PLOTBOX * pPlot = & (Plots[PlotIndex]);                 

   // swap corners if needed
   if (ZoomRect->ll.x > ZoomRect->ur.x)
   {
      CX temp = ZoomRect->ur.x;
      ZoomRect->ur.x = ZoomRect->ll.x;
      ZoomRect->ll.x = temp;
   }

   if (ZoomRect->ll.y > ZoomRect->ur.y)
   {
      CX temp = ZoomRect->ur.y;
      ZoomRect->ur.y = ZoomRect->ll.y;
      ZoomRect->ll.y = temp;
   }

   data1 = gss_to_xydata(pPlot, ZoomRect->ll.x, ZoomRect->ll.y,
                          CursorStatus[PlotIndex].Z); 
   data2 = gss_to_xydata(pPlot, ZoomRect->ur.x, ZoomRect->ur.y,
                          CursorStatus[PlotIndex].Z); 

   if (pKSRecord != NULL)          
   {
      if (*pKSRecord)
      {
         CHAR Buf[80];
         sprintf(Buf, "ZOOM(%.7f, %.7f, %.7f, %.7f);\n",  
            data1.x, data1.y, data2.x, data2.y);  
         MacRecordString(Buf);
      }
   }

   if (pPlot->style == FALSE_COLOR)
    {
    data1.y = (float)((int)data1.y);
    data2.y = (float)((int)data2.y);
    }

   pPlot->x.min_value = data1.x;
   pPlot->x.max_value = data2.x;
   pPlot->y.min_value = data1.y;
   pPlot->y.max_value = data2.y;
   
   scale_axis(&pPlot->x);
   scale_axis(&pPlot->y);

   if(DoLive)
      draw_plotbox(pPlot);
   else
      Replot(Window);

   return err;
}

// receive mouse motion from grafmous module. XPos, YPos is the new mouse
// position.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void zoomMouseMove(SHORT XPos, SHORT YPos)
{
   cursor_loc.x = XPos;
   cursor_loc.y = YPos;

   if(ZoomState == ZOOM_CHOOSE_PT2) {
      ZoomRect.ur = cursor_loc;
      DrawZoomBox(& ZoomRect, FALSE);
   }
   ZoomDisplayCursorStatus(cursor_loc);
   CDisplayCursor(screen_handle, cursor_loc);
}

void MacDoZoom(double zoomPts[4])
{
   if(plotAreaShowing())
   {
      CRECT   ZoomRect;
      BOOLEAN RemGCursor = TempChangeCursorType(NULL);

      // Calculate Gss positions based on current Z value.
      ZoomRect.ll = gss_position(&Plots[ActiveWindow], (FLOAT) zoomPts[0],
         (FLOAT) zoomPts[1], CursorStatus[ActiveWindow].Z);
      ZoomRect.ur = gss_position(&Plots[ActiveWindow], (FLOAT) zoomPts[2],
         (FLOAT) zoomPts[3], CursorStatus[ActiveWindow].Z);
      ZoomToRect(ActiveWindow, &ZoomRect);

      TempRestoreCursorType(RemGCursor, NULL);
   }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacRestoreZoom(void)
{

  if(plotAreaShowing())
    {
    BOOLEAN RemGCursor = TempChangeCursorType(NULL);

    RestoreZoom(0);
    TempRestoreCursorType(RemGCursor, NULL);
    }
}
