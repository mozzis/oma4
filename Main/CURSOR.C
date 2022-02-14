/* -----------------------------------------------------------------------
/
/  cursor.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/cursor.c_v   0.26   06 Jul 1992 10:27:14   maynard  $
/  $Log:   J:/logfiles/oma4000/main/cursor.c_v  $
*/
  
#ifdef PROT
#define INCL_KBD
#include <os2.h>
#endif
  
#include <string.h>
#include <sys\timeb.h>
  
#include "cursor.h"
#include "forms.h"
#include "barmenu.h"
#include "tempdata.h"
#include "points.h"
#include "device.h"
#include "multi.h"
#include "di_util.h"
#include "macrecor.h"
#include "oma4000.h"
#include "live.h"     // InLiveLoop
#include "calib.h"
//#include "cursvbar.h"  // only needed for GRAPHICAL scan setup form
#include "ksindex.h"
//#include "scangraf.h"  // only needed for GRAPHICAL scan setup form
#include "omameth.h"   // InitialMethod
#include "handy.h"     // RIGHTSHIFT
#include "doplot.h"    // ResizePlotForWindow()
#include "curvdraw.h"  // DupDisplayWindow()
#include "crventry.h"
#include "crvheadr.h"
#include "plotbox.h"
#include "curvedir.h"
#include "tagcurve.h"
  
#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif
  
SHORT ActiveWindow = 0;
// SHORT ActivePlotIndex = 0;
  
SHORT CursorMode = CURSORMODE_NORMAL;   /* Normal, Live, Accum, Paused, */
/* Zoom, window select          */
  
static BOOLEAN SaveOk = FALSE;
static SHORT CursorMarkerHeight;
static BOOLEAN GraphCursorOn = FALSE;
static CXY LastCursorLoc = {0,0};
static char far StatusStringBuffer[20];
  
CURSORSTAT CursorStatus[MAXPLOTS] =
  {
  {0.0F, 0.0F, 0.0F, -1, 0, 0 },
  {0.0F, 0.0F, 0.0F, -1, 0, 0 },
  {0.0F, 0.0F, 0.0F, -1, 0, 0 },
  {0.0F, 0.0F, 0.0F, -1, 0, 0 },
  {0.0F, 0.0F, 0.0F, -1, 0, 0 },
  {0.0F, 0.0F, 0.0F, -1, 0, 0 },
  {0.0F, 0.0F, 0.0F, -1, 0, 0 },
  {0.0F, 0.0F, 0.0F, -1, 0, 0 },
  {0.0F, 0.0F, 0.0F, -1, 0, 0 }
  };
  
CHAR *   CursorModeOptions[] =
  {
  "      ",
  "LIVE  ",
  "ACCUM ",
  "PAUSED",
  "ZOOM  ",
  "SELECT",
  "TAGGED",
  "ACTIVE",
  };
  
CMARKERTYPE CursorType = CMK_Plus;
  
USHORT CursorInc = 1;

/* defines the positions of the cursor status line elements */
enum { WinCol = 1,PixCol =10, XCol = 18, YCol = 30, ZCol = 43,
       NameCol = 52, BlkCol = 67, ModeCol = 73 };
  
// max time in milliseconds to be a repeat hit.
const USHORT KeyRepeatTm = 500;
  
void TmpSwap(float * f1, float * f2)
{
  float Temp = *f1;
  *f1 = *f2;
  *f2 = Temp;
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
#define SPEEDFACTOR        0.13F
  
PRIVATE void CheckCursorAccel(USHORT PointNum)
{
  FLOAT TempSpd;
  USHORT TimeDiff;/* time difference in milliseconds since last key press */
  struct timeb Time;
  static struct timeb LastTime = {0, 0, 0, 0};
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
    CursorInc = 1;
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
    if (CursorInc < (PointNum >> 2))
      {
      TempSpd = (FLOAT) CursorInc * SPEEDFACTOR;
      if (TempSpd < (FLOAT) 1.0)
        TempSpd = (FLOAT) 1.0;
      CursorInc += (SHORT) TempSpd;
      }
    }
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void UpdateCursorStatusString(SHORT Column, PCHAR String)
{
  CXY TextPt;
  CCOLOR SelColor;
  
  CSetATextColor(screen_handle, BLACK, &SelColor);
  CSetBgColor(screen_handle, BRT_YELLOW, &SelColor);
  
  TextPt.x = column_to_x(Column);
  TextPt.y = row_to_y(2);
  CSetATextPosition(screen_handle, TextPt, &TextPt);
  CAText(screen_handle, String, &TextPt);
}

void UpdateWindowNumberStat(SHORT Window)
{
  /* make window 1 based instead of 0 */
  sprintf(StatusStringBuffer, "%-2d", WindowPlotAssignment[Window] + 1);
  UpdateCursorStatusString(WinCol+6, StatusStringBuffer);    /* "Plot #" */
}
  

void UpdateCursorXStat(SHORT Window)
{
  erase_mouse_cursor();
  sprintf(StatusStringBuffer, "%9.6g",
           CursorStatus[WindowPlotAssignment[Window]].X);
  StripExp(StatusStringBuffer, 9, FALSE);
  UpdateCursorStatusString(XCol+2, StatusStringBuffer);     /* "X=" */
  sprintf(StatusStringBuffer, "%-5.1d",
           CursorStatus[WindowPlotAssignment[Window]].PointIndex);
  UpdateCursorStatusString(PixCol+2, StatusStringBuffer);     /* "P=" */

  replace_mouse_cursor();
}
  
void UpdateCursorYStat(SHORT Window)
{
  if(! InLiveLoop)
    erase_mouse_cursor();
  sprintf(StatusStringBuffer, "%9.6g",
          CursorStatus[WindowPlotAssignment[Window]].Y);
  StripExp(StatusStringBuffer, 9, FALSE);
  UpdateCursorStatusString(YCol+2, StatusStringBuffer);     /* "Y=" */
  if(! InLiveLoop)
    replace_mouse_cursor();
}
  
void UpdateCursorZStat(SHORT Window)
{
  if(! InLiveLoop)
    erase_mouse_cursor();
  sprintf(StatusStringBuffer, "%5.5g",
           CursorStatus[WindowPlotAssignment[Window]].Z);
  StripExp(StatusStringBuffer, 5, FALSE);
  UpdateCursorStatusString(ZCol+2, StatusStringBuffer);     /* "Z=" */
  if(! InLiveLoop)
    replace_mouse_cursor();
}
  
void UpdateCursorBlkNameStat(SHORT Window)
{
  int i, Buflen;
  SHORT PlotIndex = WindowPlotAssignment[Window];
  
  if (CursorStatus[PlotIndex].EntryIndex != -1)
    {
    strcpy(StatusStringBuffer,
            MainCurveDir.Entries[CursorStatus[PlotIndex].EntryIndex].name);
    StatusStringBuffer[8] = '\0';  /* put in the ending NULL to use only 8 chars */
    }
  else
    StatusStringBuffer[ 0 ] = '\0';
  
  Buflen = strlen(StatusStringBuffer);
  if (Buflen != 8)
    {
    for (i=Buflen; i<8; i++)  /* pad with spaces */
      StatusStringBuffer[i] = SPACE;
    }
  erase_mouse_cursor();
  UpdateCursorStatusString(NameCol+6, StatusStringBuffer);   /* "Name: " */
  replace_mouse_cursor();
 }
  
void UpdateCursorBlkNumberStat(SHORT Window)
{
  sprintf(StatusStringBuffer, "%5u",
           CursorStatus[WindowPlotAssignment[Window]].FileCurveNumber);
  StripExp(StatusStringBuffer, 5, FALSE);
  erase_mouse_cursor();
  UpdateCursorStatusString(BlkCol+1, StatusStringBuffer);      /* "#" */
  replace_mouse_cursor();
}
  
void UpdateCursorMode(SHORT ModeNumber)
{
  erase_mouse_cursor();
  UpdateCursorStatusString(ModeCol, CursorModeOptions[ModeNumber]);
  CursorMode = ModeNumber;
  replace_mouse_cursor();
}
  
/*  cursor status line
123456789012345678901234567890123456789012345678901234567890123456789012345678
  
Window#8 P=2048 X=-3.333e-33 Y=-3.333e-33 Z=-3.333e-33 Name:testname #12345
Window#8 P=2048 X=-3.333e-33 Y=-3.333e-33 Z=-3.333e-33 I=      SC=      LIVE
  
*/
  
//************************************************************************
// Check that the cursor is placed somewhere inside the current
// plotting window
//************************************************************************
  
void ForceCursorIntoWindow(SHORT Window)
{
  USHORT EntryIndex, CurveIndex, UTDisplayIndex, i;
  USHORT PlotIndex = WindowPlotAssignment[Window];
  CURSORSTAT *CursorStat = &(CursorStatus[PlotIndex]);
  PLOTBOX * ThisPlot = &(Plots[PlotIndex]);
  BOOLEAN Found, PrevStatInvalid, ZSwap = FALSE;
  float Xmin = ThisPlot->x.min_value,
        Xmax = ThisPlot->x.max_value,
        Zmin = ThisPlot->z.min_value,
        Zmax = ThisPlot->z.max_value;
  
  if (ThisPlot->style == FALSE_COLOR)
    {
    Zmin = ThisPlot->y.min_value;
    Zmax = ThisPlot->y.max_value;
    }
  
  if (Xmin > Xmax) TmpSwap(&Xmin, &Xmax);
  if (Zmin > Zmax)
    {
    ZSwap = TRUE;
    TmpSwap(&Zmin, &Zmax);
    }
  
  /* check to see if cursor was already active in this window */
  if (CursorStat->EntryIndex == -1)
    {
    CursorStat->TotalCurves = 0;
  
    /* if it wasn't already active, give it a place to start at */
    Found = FindFirstPlotBlock(&MainCurveDir,
                                &(CursorStat->EntryIndex),
                                &(CursorStat->FileCurveNumber),
                                &(CursorStat->UTDisplayCurve),
                                Window);
    if (! Found)
      {
      CursorStat->EntryIndex = -1;
      CursorStat->UTDisplayCurve = -1;
      return;
      }
  
    CursorStat->X = (FLOAT) 0;  /* this may be a problem */
  
    // calculate the number of curves displayed
    if (ExpandedOnTagged)
      {
      EntryIndex = CursorStat->EntryIndex;
      CurveIndex = CursorStat->FileCurveNumber;
      UTDisplayIndex = CursorStat->UTDisplayCurve;
      while (Found)
        {
        CursorStat->TotalCurves++;
        Found = FindNextPlotCurve(&MainCurveDir,
                                   &EntryIndex,
                                   &CurveIndex,
                                   &UTDisplayIndex,
                                   Window);
        }
  
      CursorStat->UTTotalCurves = 0;
      for (i=0; i<MainCurveDir.BlkCount; i++)
        {
        if (MainCurveDir.Entries[i].DisplayWindow & (1 << PlotIndex))
          CursorStat->UTTotalCurves += MainCurveDir.Entries[i].count;
        }
      }
    else
      {
      for (i=0; i<MainCurveDir.BlkCount; i++)
        {
        if (MainCurveDir.Entries[i].DisplayWindow & (1 << PlotIndex))
          CursorStat->TotalCurves += MainCurveDir.Entries[i].count;
        }
      CursorStat->UTTotalCurves = CursorStat->TotalCurves;
      }
    }
  
  /* if it was already active, check out the validity of the previous */
  /* settings.  */
  
  if (MainCurveDir.BlkCount <= (USHORT) CursorStat->EntryIndex)
    {
    PrevStatInvalid = TRUE;
    }
  else
    {
    CURVE_ENTRY *pEntry = &(MainCurveDir.Entries[CursorStat->EntryIndex]);
  
    /* if Window owns Entry */
    PrevStatInvalid =
      ((!(pEntry->DisplayWindow & (1 << ActiveWindow))) ||
    /* File curve number is valid (always?) */
      (CursorStat->FileCurveNumber >= (pEntry->StartIndex + pEntry->count)) ||
      (CursorStat->FileCurveNumber < pEntry->StartIndex));               
    }
  if (PrevStatInvalid)
    {
    /* curve block is no longer set for this window */
    /* give it a place to start at */
    if (! FindFirstPlotBlock(&MainCurveDir,
                               &(CursorStat->EntryIndex),
                               &(CursorStat->FileCurveNumber),
                               &(CursorStat->UTDisplayCurve),
                               Window))
      {
      CursorStat->EntryIndex = -1;
      CursorStat->UTDisplayCurve = -1;
      }
    }
  
  if (ExpandedOnTagged)  /* MLM add test and DisplayCurve alternative */
    CursorStat->Z = Zmin;
  else
    CursorStat->Z = CursorStat->UTDisplayCurve;
  
  if (CursorStat->EntryIndex != -1)
    {
    double YVal;
    CURVEHDR Curvehdr;
  
    /* Try to come up with the cursor in the plot box */
  
    if(CursorStat->X < Xmin) CursorStat->X = Xmin;
    if(CursorStat->X > Xmax)  CursorStat->X = Xmax;
  
    // get the closest point with the X and Y values, return if error
    if(GetTempCurvePointIndex(&MainCurveDir,
                              CursorStat->EntryIndex,
                              CursorStat->FileCurveNumber,
                              &(CursorStat->X), &(CursorStat->Y),
                              (UCHAR)ThisPlot->x.units,
                              &CursorStat->PointIndex, 0))
      return;
  
    // get the curveheader and Y values, return if error
    if(ReadTempCurvehdr(&MainCurveDir, CursorStat->EntryIndex,
                        CursorStat->FileCurveNumber, &Curvehdr))
      return;
  
    /* convert cursor Y value back to plot units from curve units */
    /* take whatever number comes back */
    if (ConvertUnits((UCHAR)ThisPlot->y.units, &YVal, Curvehdr.YUnits,
                 (DOUBLE)CursorStat->Y, InitialMethod->Excitation))
      return;

    CursorStat->Y = (FLOAT)YVal;
    }
  else
    { /* no curves in this window */
    CursorStat->X = (FLOAT)0; /* maybe should be Xmin */
    CursorStat->Y = (FLOAT)0;
    }
  
  if(CursorStat->Z < Zmin)
    JumpCursor(CursorStat->X, Zmin);
  if(CursorStat->Z > Zmax)
    JumpCursor(CursorStat->X, Zmax);

  DisplayActiveGraphCursor();
}

/*************************************************************************/
void InitCursorStatus(SHORT Window)
{
  CXY TextPt, FinishPt;
  CRECT LineRect;
  CRECT OldClipRect;
  CCOLOR SelColor;
  
  LineRect.ll.x = column_to_x(1);
  LineRect.ll.y = row_to_y(2);
  LineRect.ur.x = column_to_x(screen_columns - 1);
  LineRect.ur.y = row_to_y(1);
  CInqClipRectangle(screen_handle, &OldClipRect);
  
  CSetATextColor(screen_handle, BLACK, &SelColor);
  CSetBgColor(screen_handle, BRT_YELLOW, &SelColor);
  CSetFillColor(screen_handle, BRT_YELLOW, &SelColor);
  
  CSetClipRectangle(screen_handle, LineRect);
  CBar(screen_handle, LineRect);
  
  ForceCursorIntoWindow(Window);
  GraphCursorOn = FALSE; /* since clip rect inhibited display */
  TextPt = LineRect.ll;
  CSetATextPosition(screen_handle, TextPt, &FinishPt);
  CAText(screen_handle, "Plot #", &FinishPt);
  
  UpdateWindowNumberStat(Window);
  
  TextPt.x = column_to_x(XCol);
  CSetATextPosition(screen_handle, TextPt, &FinishPt);
  CAText(screen_handle, "X=", &FinishPt);
  
  TextPt.x = column_to_x(PixCol);
  CSetATextPosition(screen_handle, TextPt, &FinishPt);
  CAText(screen_handle, "P=", &FinishPt);
  
  UpdateCursorXStat(Window);
  
  TextPt.x = column_to_x(YCol);
  CSetATextPosition(screen_handle, TextPt, &FinishPt);
  CAText(screen_handle, "Y=", &FinishPt);
  
  UpdateCursorYStat(Window);
  
  TextPt.x = column_to_x(ZCol);
  CSetATextPosition(screen_handle, TextPt, &FinishPt);
  CAText(screen_handle, "Z=", &FinishPt);
  
  UpdateCursorZStat(Window);
  
  TextPt.x = column_to_x(NameCol);
  CSetATextPosition(screen_handle, TextPt, &FinishPt);
  CAText(screen_handle, "Name:", &FinishPt);
  
  UpdateCursorBlkNameStat(Window);
  
  TextPt.x = column_to_x(BlkCol);
  CSetATextPosition(screen_handle, TextPt, &FinishPt);
  CAText(screen_handle, "#", &FinishPt);
  
  UpdateCursorBlkNumberStat(Window);
  
  UpdateCursorMode(CursorMode);
  
  CSetClipRectangle(screen_handle, OldClipRect);

  DisplayActiveGraphCursor();
}
  
void SetGraphCursorType(CMARKERTYPE CursorType)
{
  CCOLOR SelColor;
  CMARKERTYPE SelType;
  CGTEXTREPR TextRep;
  
  CSetMarkerType(screen_handle, CursorType, &SelType);
  
  // Base cursor size on text size
  CInqGTextRepr(screen_handle, &TextRep);
  CursorMarkerHeight = TextRep.CellSize.y;
  CSetMarkerHeight(screen_handle, CursorMarkerHeight, &CursorMarkerHeight);
  CSetMarkerColor(screen_handle, BRT_YELLOW, &SelColor);
}
  
void InitCursor(SHORT Window, CMARKERTYPE CursorType)
{
  USHORT i;
  USHORT EntryIndex, CurveIndex, UTDisplayIndex;
  CCOLOR SelColor;
  USHORT PlotIndex = WindowPlotAssignment[Window];
  BOOLEAN Found;
  USHORT OldCurveCount, DisplayIndex;
  
  ResizePlotForWindow(Window);
  SetGraphCursorType(CursorType);
  
  /* count the number of curves in this window */
  CursorStatus[PlotIndex].TotalCurves = 0;
  CursorStatus[PlotIndex].UTTotalCurves = 0;
  if (ExpandedOnTagged)
    {
    Found = FindFirstPlotBlock(&MainCurveDir,
                                &EntryIndex,
                                &CurveIndex,
                                &UTDisplayIndex,
                                Window);
  
    CursorStatus[PlotIndex].TotalCurves = 0;
    while (Found)
      {
      if(CursorStatus[PlotIndex].Z == CursorStatus[PlotIndex].TotalCurves)
        {
        CursorStatus[PlotIndex].EntryIndex = EntryIndex;
        CursorStatus[PlotIndex].FileCurveNumber = CurveIndex;
        CursorStatus[PlotIndex].UTDisplayCurve = UTDisplayIndex;
        }
      CursorStatus[PlotIndex].TotalCurves++;
      Found = FindNextPlotCurve(&MainCurveDir,
                                 &EntryIndex,
                                 &CurveIndex,
                                 &UTDisplayIndex,
                                 Window);
      }
    for (i=0; i<MainCurveDir.BlkCount; i++)
      {
      if (MainCurveDir.Entries[i].DisplayWindow & (1 << PlotIndex))
        CursorStatus[PlotIndex].UTTotalCurves +=
        MainCurveDir.Entries[i].count;
      }
    }
  else
    {
    DisplayIndex = (USHORT) (CursorStatus[PlotIndex].Z + 0.5);
    for (i=0; i<MainCurveDir.BlkCount; i++)
      {
      if (MainCurveDir.Entries[i].DisplayWindow & (1 << ActiveWindow))
        {
        OldCurveCount = CursorStatus[PlotIndex].TotalCurves;
        CursorStatus[PlotIndex].TotalCurves +=
        MainCurveDir.Entries[i].count;
  
        if (MainCurveDir.Entries[i].count &&
          (DisplayIndex >= OldCurveCount) &&
          (DisplayIndex < CursorStatus[PlotIndex].TotalCurves))
          {
          CursorStatus[PlotIndex].EntryIndex = i;
          CursorStatus[PlotIndex].FileCurveNumber =
          DisplayIndex- OldCurveCount +
          MainCurveDir.Entries[i].StartIndex;
          CursorStatus[PlotIndex].UTDisplayCurve = DisplayIndex;
          }
        }
      }
    CursorStatus[PlotIndex].UTTotalCurves = CursorStatus[PlotIndex].TotalCurves;
    }
  InitCursorStatus(Window);
  
  CSetBgColor(deviceHandle(), Plots[PlotIndex].background_color,& SelColor);
  
  SetCursorPos(Window, CursorStatus[PlotIndex].X,
                        CursorStatus[PlotIndex].Y,
                        CursorStatus[PlotIndex].Z);
}
  
// Given that the cursor has just been moved to a new position, clean up the
// key buffers, and take care of keystroke recording.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void processNewCursorPos(SHORT Window, unsigned keyStroke)
  {
  DumpKeyBuffer(keyStroke);
  DumpKeyBuffer(keyStroke | LOCATION_CURSOR_PAD);
  
  if(pKSRecord && * pKSRecord)
    {
    USHORT PlotIndex = WindowPlotAssignment[Window];
    char Buf[80];
  
    sprintf(Buf, "MOVE_CURSOR(%.7lg, %.7lg);\n",
    CursorStatus[PlotIndex].Z, CursorStatus[PlotIndex].X);
    MacRecordString(Buf);
    }
  }
  
void MoveCursorXPos(SHORT Window, BOOLEAN Right)
  {
  DOUBLE XVal, YVal, ZVal;
  CURVEHDR Curvehdr;
  USHORT EntryIndex, CurveIndex, OrigIndex;
  SHORT prefBuf = 0;
  FLOAT XTmp, YTmp, XMax, XMin;
  BOOLEAN Inverted = FALSE;
  CURSORSTAT * CursorStat = & (CursorStatus[WindowPlotAssignment[Window]]);
  PLOTBOX * pPlot = &Plots[WindowPlotAssignment[Window]];

  EntryIndex = CursorStat->EntryIndex;
  CurveIndex = CursorStat->FileCurveNumber;
  OrigIndex  = CursorStat->PointIndex;
  
  /* check to see if have a valid curve */
  if (EntryIndex == -1)
    return;
  
  if (ReadTempCurvehdr(&MainCurveDir, EntryIndex, CurveIndex, &Curvehdr))
    return;  // error
  
  if (Curvehdr.pointnum <= 1)
    return;
  
  ZVal = (DOUBLE) CursorStat->Z;
  XMax = pPlot->x.max_value;
  XMin = pPlot->x.min_value;

  {
  FLOAT Temp = InitialMethod->CalibCoeff[0][1];
  if ((Temp > 0 && XMin >  XMax) || (Temp < 0 && XMax > XMin))
    {
    if (Temp > 0)
      TmpSwap(&XMin, &XMax);
    Inverted = TRUE;
    }
  }

  CheckCursorAccel(Curvehdr.pointnum);
  
  XVal = (DOUBLE) CursorStat->X;
  
  XTmp = (FLOAT) XVal;
  if(GetTempCurvePointIndex(&MainCurveDir, EntryIndex, CurveIndex,
                            &XTmp, &YTmp, (UCHAR)pPlot->x.units,
                            &CursorStat->PointIndex, 0))
    return;  // error
  
  /* get next point value */
  /* don't go past the end of the curve */
  
  if ((Right && !Inverted) || (!Right && Inverted))
    {                                     /* move to the higher data point */
    if (CursorStat->PointIndex < (Curvehdr.pointnum - CursorInc - 1))
      CursorStat->PointIndex += CursorInc;
    else
      CursorStat->PointIndex = Curvehdr.pointnum - 1;
    }
  else  /* move to a lower data point */
    {
    if (CursorStat->PointIndex >=CursorInc)
      CursorStat->PointIndex -= CursorInc;
    else
      CursorStat->PointIndex = 0;
    }
  
  /* get next point value */
  
  if(GetDataPoint(&MainCurveDir, EntryIndex, CurveIndex,CursorStat->PointIndex,
                  &XTmp, &YTmp, FLOATTYPE, &prefBuf))
    return; // error
  
  YVal = YTmp;
  
  if (pPlot->x.units != COUNTS)
    {
    if (Curvehdr.XData.XUnits == COUNTS)
      {
      XTmp = (FLOAT) CursorStat->PointIndex;
  
      if (InitialMethod->CalibUnits[0] != COUNTS)
        {
        XTmp = ApplyCalibrationToX(InitialMethod->CalibCoeff[0], XTmp);
  
        /* convert cursor X value back to plot units from curve units */

        if (ConvertUnits((UCHAR) pPlot->x.units, &XVal,
                     InitialMethod->CalibUnits[0], XTmp,
                     InitialMethod->Excitation))
          return;
        }
      else
        XVal = XTmp;
      }
    else
      {
      /* convert cursor X value back to plot units from curve units */

      if (ConvertUnits((UCHAR) pPlot->x.units, &XVal,
                   Curvehdr.XData.XUnits, XTmp,
                   InitialMethod->Excitation))
        return;
      }
    }
  else
    XVal = (DOUBLE) CursorStat->PointIndex;
  
  if (XVal > (DOUBLE)XMax || XVal < (DOUBLE) XMin)  /* limit to plotbox */
    {
    CursorStat->PointIndex = OrigIndex;
    CursorInc = 1;
    return;
    }
  
  /* convert cursor Y value back to plot units from curve units */

  if (pPlot->y.units != COUNTS)
    ConvertUnits((UCHAR) pPlot->y.units, &YVal,
    Curvehdr.YUnits, (DOUBLE) YTmp, InitialMethod->Excitation);
  
  /* Update cursor position */
  CursorStat->X = (FLOAT) XVal;
  CursorStat->Y = (FLOAT) YVal;
  
  UpdateCursorXStat(Window);
  UpdateCursorYStat(Window);
  SetCursorPos(Window, (FLOAT) XVal, (FLOAT) YVal, (FLOAT) ZVal);
  
  processNewCursorPos(Window, Right ? SK_RIGHT_ARROW : SK_LEFT_ARROW);
}
  
void MoveCursorZPos(SHORT Window, BOOLEAN Up, BOOLEAN Jump)
  {
  float XVal, YVal, ZVal; /* why are these DOUBLE? */
  USHORT FileCurve = 0;
  USHORT EntryIndex;
  USHORT UTDisplayCurve = 0;
  FLOAT XTmp, YTmp, Zmin, Zmax;
  BOOLEAN Found;
  USHORT i;
  CURVEHDR Curvehdr;
  BOOLEAN Inverted = FALSE;
  USHORT PlotIndex = WindowPlotAssignment[Window];
  PLOTBOX *ThisPlot = &Plots[PlotIndex];
  CURSORSTAT * CursorStat = &(CursorStatus[PlotIndex]);
  
  if(ThisPlot->style == FALSE_COLOR)
    {
    Zmin = ThisPlot->y.min_value;
    Zmax = ThisPlot->y.max_value;
    }
  else
    {
    Zmin = ThisPlot->z.min_value;
    Zmax = ThisPlot->z.max_value;
    }
  
  /* check to see if movement is inverted from the data values */
  if(Zmin > Zmax)
    {
    TmpSwap(&Zmin, &Zmax);
    Inverted = TRUE;
    }
  
  /* see if any curves are in window */
  if (!Inverted) /* (so FileCurve & UTDisplayCurve are set correctly) */
    {
    if(! FindFirstPlotBlock(&MainCurveDir, &EntryIndex,
                              &FileCurve, &UTDisplayCurve, Window))
      return;
    }
  else
    {
    if(! FindLastPlotBlock(&MainCurveDir, &EntryIndex,
                             &FileCurve, &UTDisplayCurve, Window))
      return;
    }
  
  ZVal = (float)CursorStat->Z;
  XVal = (float)CursorStat->X;
  
  if (XVal > ThisPlot->x.max_value) XVal = ThisPlot->x.max_value;
  if (XVal < ThisPlot->x.min_value) XVal = ThisPlot->x.min_value;
  
  EntryIndex = CursorStat->EntryIndex;
  FileCurve = CursorStat->FileCurveNumber;
  UTDisplayCurve = CursorStat->UTDisplayCurve;
  
  if (! Jump)
    CheckCursorAccel(CursorStat->TotalCurves);
  
  Found = TRUE;
  
  /* check to see if need to go to higher curve blocks */
  
  for(i=0; (i<CursorInc) && Found; i++)
    {
    if (EntryIndex > MainCurveDir.BlkCount-1)
      Found = FindLastPlotBlock(&MainCurveDir, &EntryIndex,
                                 &FileCurve, &UTDisplayCurve, Window);
    if (! Found)
      return;
  
    if ((Up && !Inverted) || (!Up && Inverted))
      Found = FindNextPlotCurve(&MainCurveDir, &EntryIndex, &FileCurve,
                                 &UTDisplayCurve, Window);
    else
      Found = FindPrevPlotCurve(&MainCurveDir, &EntryIndex, &FileCurve,
                                 &UTDisplayCurve, Window);
    }
  
  /* check to see if overshot last curve */
  if (! Found)
    {
    if ((Up && !Inverted) || (!Up && Inverted))
      {
      ZVal = (FLOAT) (CursorStat->TotalCurves - 2);
  
      if (! FindLastPlotBlock(&MainCurveDir, &EntryIndex,
                                &FileCurve, &UTDisplayCurve, Window))
        return;
      }                                    /* So point to last display curve*/
    else
      {
      ZVal = Zmin + 1;
      if (! FindFirstPlotBlock(&MainCurveDir, &EntryIndex,
                                 &FileCurve, &UTDisplayCurve, Window))
        return;
      }
  
     /* reset the cursor increment value */
     CursorInc = 1;
     }
  
  if(ReadTempCurvehdr(&MainCurveDir, EntryIndex, FileCurve, &Curvehdr))
    return; // error
  
  XTmp = (FLOAT) XVal;
  if(GetTempCurvePointIndex(&MainCurveDir, EntryIndex, FileCurve,
                              &XTmp, &YTmp,
                              (UCHAR) ThisPlot->x.units,
                              &CursorStat->PointIndex, 0))
    return; // error
  
  XVal = (float) XTmp;
  YVal = (float) YTmp;

  {
  DOUBLE YTemp = (DOUBLE)YVal;
  
  /* convert cursor Y value back to plot units from curve units */
  /* take whatever number comes back */
  ConvertUnits((UCHAR) ThisPlot->y.units, &YTemp,
                Curvehdr.YUnits, YTemp,
                InitialMethod->Excitation);

  YVal = (float) YTemp;
  }

  if(ThisPlot->style == FALSE_COLOR)
    {
    if (YVal > ThisPlot->z.max_value)
      YVal = ThisPlot->z.max_value;
    }
  else
    {
    if (YVal > ThisPlot->y.max_value)
      YVal = ThisPlot->y.max_value;
    }
  
  if ((Up && !Inverted) || (!Up && Inverted))
    ZVal += (float) CursorInc;
  else
    ZVal -= (float) CursorInc;
  
  if (ZVal > (float)Zmax || ZVal < (float)Zmin)
    {
    ZVal = CursorStat->Z;
    YVal = CursorStat->Y;
    XVal = CursorStat->X;
    CursorInc = 1;
    }
  else
    {
    CursorStat->EntryIndex = EntryIndex;
    CursorStat->FileCurveNumber = FileCurve;
    CursorStat->X = (FLOAT) XVal;
    CursorStat->Y = (FLOAT) YVal;
    CursorStat->Z = (FLOAT) ZVal;
    CursorStat->UTDisplayCurve = UTDisplayCurve;
    }
  UpdateCursorXStat(Window);
  UpdateCursorYStat(Window);
  UpdateCursorZStat(Window);
  UpdateCursorBlkNameStat(Window);
  UpdateCursorBlkNumberStat(Window);

  SetCursorPos(Window, (FLOAT) XVal, (FLOAT) YVal, (FLOAT) ZVal);
  
  if(! Jump)
    processNewCursorPos(Window, Up ? SK_UP_ARROW : SK_DOWN_ARROW);
}
  
/**********************************************************************/
/* set up cursor info                                                 */
/**********************************************************************/
static void set_cursor_start(CURSORSTAT * pCursorStat)
  {
  CXY Cursor;
  
  if(! FindFirstPlotBlock(& MainCurveDir, & pCursorStat->EntryIndex,
    & pCursorStat->FileCurveNumber,
    & pCursorStat->UTDisplayCurve, ActiveWindow))
    {
    pCursorStat->EntryIndex  = -1;
    pCursorStat->TotalCurves =  0;
    pCursorStat->UTTotalCurves =  0;
    pCursorStat->X = pCursorStat->Y = pCursorStat->Z = (float)0.0;
    }
  
  // Initialize new cursor position
  Cursor = gss_position(ActivePlot,
  pCursorStat->X, pCursorStat->Y, pCursorStat->Z);
  
  if (Cursor.x > ActivePlot->plotarea.ur.x)
    Cursor.x = ActivePlot->plotarea.ur.x;
  else if (Cursor.x < ActivePlot->plotarea.ll.x)
    Cursor.x = ActivePlot->plotarea.ll.x;
  
  if (Cursor.y > ActivePlot->plotarea.ur.y)
    Cursor.y = ActivePlot->plotarea.ur.y;
  else if (Cursor.y < ActivePlot->plotarea.ll.y)
    Cursor.y = ActivePlot->plotarea.ll.y;
  }
  
/**********************************************************************/
/* set up a new active window for display, return the current value   */
/* of ActiveWindow                                                    */
/**********************************************************************/
int new_active_window(int new_window)
{
  int old_ActiveWindow = ActiveWindow;      // return value
  CURSORSTAT * pCursorStat = &CursorStatus[WindowPlotAssignment[new_window]];
  
  ActiveWindow = new_window;
  ActivePlot = &Plots[ActiveWindow];
  
  // copy plotting and displayed curve info
  DupDisplayWindow(& MainCurveDir, ActiveWindow, old_ActiveWindow);
  ResizePlotForWindow(ActiveWindow);
  
  // copy cursor information
  CursorStatus[ActiveWindow] =
    CursorStatus[WindowPlotAssignment[old_ActiveWindow]];
  
  // check to see if cursor was already active in this window
  if((pCursorStat -> EntryIndex == -1)
    || (pCursorStat -> EntryIndex >= (SHORT)MainCurveDir.BlkCount)
   )
    // if it wasn't already active, give it a place to start at
    set_cursor_start(pCursorStat);
  
  // if it was already active, check out the validity of the previous
  // settings
    if(pCursorStat -> EntryIndex != -1) {
    CURVE_ENTRY *pEntry = &MainCurveDir.Entries[pCursorStat -> EntryIndex];
    if ((!(pEntry->DisplayWindow & (1 << ActiveWindow))) ||
       ((pCursorStat->FileCurveNumber >= pEntry->StartIndex+pEntry->count)) ||
       ((pCursorStat->FileCurveNumber < pEntry->StartIndex)))
   // curve block is no longer set for this window, give it a place to start
      set_cursor_start(pCursorStat);
    }
  return old_ActiveWindow;
}
  
/**********************************************************************/
// restore a previous window as ActiveWindow
/**********************************************************************/
void restore_active_window(int old_window)
{
  USHORT i;
  CXY Cursor;
  CURSORSTAT *pCursorStat;
  
  for (i=0; i<MainCurveDir.BlkCount; i++)
    {
    /* If Live was activated in baseline or calib window, make sure that */
    /* it gets copied back to the current window */
    if (MainCurveDir.Entries[i].DisplayWindow == (1 << CAL_PLOTBOX))
      MainCurveDir.Entries[i].DisplayWindow =
      (1 << (USHORT) WindowPlotAssignment[old_window]);
    }
  ActiveWindow = old_window;
  ActivePlot = &Plots[ActiveWindow];
  
  pCursorStat = &CursorStatus[ActiveWindow];
  // Initialize new cursor position
  Cursor = gss_position(&(Plots[ActiveWindow]), pCursorStat->X, pCursorStat->Y, pCursorStat->Z);
  
  if (Cursor.x > ActivePlot->plotarea.ur.x)
    Cursor.x = ActivePlot->plotarea.ur.x;
  else if (Cursor.x < ActivePlot->plotarea.ll.x)
    Cursor.x = ActivePlot->plotarea.ll.x;
  
  if (Cursor.y > ActivePlot->plotarea.ur.y)
    Cursor.y = ActivePlot->plotarea.ur.y;
  else if (Cursor.y < ActivePlot->plotarea.ll.y)
    Cursor.y = ActivePlot->plotarea.ll.y;
  
}
  
/***************************************************************************/
/*  function:  Move the curve cursor to near the requested X and Z         */
/*             positions in the current graph window                       */
/*                                                                         */
/*  Variables: XVal - Requested X axis value                               */
/*             ZVal - Requested Z axis value                               */
/*                                                                         */
/*  returns:   error identifier                                            */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects:                                                          */
/*                Changes values in CursorStatus                           */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA JumpCursor(FLOAT XVal, FLOAT ZVal)
{
  CURSORSTAT * CursorStat = & CursorStatus[ActiveWindow];
  float ZCur = CursorStat->Z;
  float Zmin = ActivePlot->z.min_value;
  float Zmax = ActivePlot->z.max_value;
  BOOLEAN Up, ZSwap = FALSE;

  if (ActivePlot->style == FALSE_COLOR)
    {
    Zmin = ActivePlot->y.min_value;
    Zmax = ActivePlot->y.max_value;
    }
  
  if (Zmin > Zmax) { ZSwap = TRUE; TmpSwap(&Zmin, &Zmax); }
  
  if (ZVal < 0) ZVal = Zmin;      /* clip cursor to legal values */
  if (ZVal > Zmax) ZVal = Zmax;
  if (ZVal < Zmin) ZVal = Zmin;
  
  if ((ZVal != CursorStat->Z))
    {
  
    if (ZCur - ZVal > 0)
      CursorInc = (USHORT) (ZCur - ZVal);    /* CursorInc must be positive */
    else
      CursorInc = (USHORT) (ZVal - ZCur);
  
    Up = (ZVal > ZCur) - ZSwap;               /* Flag jumping up or back */
  
    MoveCursorZPos(ActiveWindow, Up, TRUE); /* Jump up or back */
  
    CursorInc = 1;
    }
  
  // the following is only needed for GRAPHICAL scan setup
  //   if(* isScanGrafMode)
  //   {
  //      float temp = CursorStatus[ActivePlotIndex].X;
  //
  //      CursorStatus[ActivePlotIndex].X = XVal;
  //
  //      if(XVal >= temp)
  //         showScangrafCursor(SK_RIGHT_ARROW);
  //      else
  //         showScangrafCursor(SK_LEFT_ARROW);
  //
  //      return;
  //   }
  
  if(XVal != CursorStat->X)
    {
    ERR_OMA err = 
      GetTempCurvePointIndex(&MainCurveDir, CursorStat->EntryIndex,
      CursorStat->FileCurveNumber,
      &XVal,
      &CursorStat->Y,
      (UCHAR) ActivePlot->x.units,
      &CursorStat->PointIndex, 0);
    if (err)
      return err; // error
  
    CursorStat->X = XVal;
    }
  
  UpdateCursorXStat(ActiveWindow);
  UpdateCursorYStat(ActiveWindow);
  UpdateCursorZStat(ActiveWindow);
  UpdateCursorBlkNameStat(ActiveWindow);
  UpdateCursorBlkNumberStat(ActiveWindow);
  SetCursorPos(ActiveWindow, CursorStat->X, CursorStat->Y, CursorStat->Z);

  return ERROR_NONE;
}
  
/**********************************************************************/
CXY GetCurrentCursorPosition(SHORT Mode)
{
  CXY CursorLoc = { 0, 0 };
  
  // in menu system return row and column
  if (Mode == LOCUS_FORMS)
    {
    CursorLoc.y = (SHORT) (Current.Form->row + Current.Field->row +
    Current.Form->display_row_offset);
    CursorLoc.x = (SHORT) (Current.Form->column + Current.Field->column +
    Current.Form->display_cursor_offset);
    }
  else if (Mode == LOCUS_MENUS)
    {
    MENUITEM * MenuItem =
    &(MenuFocus.ActiveMENU->ItemList[MenuFocus.ItemIndex]);
  
    CursorLoc.x =   MenuItem->Column + MenuItem->SelectCharOffset
    + MenuFocus.Column + 1;
    CursorLoc.y = MenuFocus.Row;
    }
  else
    {
    /* get cursor location */
    CursorLoc = LastCursorLoc;
    }
  return CursorLoc;
}
  
/***************************************************************************/
/*  function:  Draw the graph mode cursor at the requested location.       */
/*             Remove any previously drawn cursor .  Uses a marker to draw */
/*             the cursor representation so that the system cursor may     */
/*             also be active.                                             */
/*                                                                         */
/*  Variables: CursorLoc - Virtual device units for placement of marker.   */
/*                                                                         */
/*  returns:   nothing                                                     */
/*                                                                         */
/*  Side effects:                                                          */
/*                Writes to screen, changes GraphCursorOn                  */
/*                                                                         */
/***************************************************************************/
  
void DisplayGraphCursor(CXY CursorLoc)
{
  CPIXOPS SelMode;
  
  if(InLiveLoop)
    return;
  
  // the following is only needed for GRAPHICAL scan setup
  //   if(* isScanGrafMode)
  //   {
  //      scangrafSetCursorYVal();
  //      CursorLoc.y = scanGrafYPos((int) CursorStatus[ActivePlotIndex].Y);
  //   }
  
  CSetWritingMode(screen_handle, CdXORs, &SelMode);
  
  // if cursor is showing, erase it from this previous position
  if (GraphCursorOn)
    CPolymarker(screen_handle, 1, &LastCursorLoc);
  
  // output cursor to new position
  CPolymarker(screen_handle, 1, &CursorLoc);
  LastCursorLoc = CursorLoc;
  
  // Set the drawing mode to replace the current pixels
  CSetWritingMode(screen_handle, CReplace, &SelMode);
  GraphCursorOn = TRUE;
  
  // only needed for GRAPHICAL scan setup form
  //   if(Current.Form->MacFormIndex != KSI_CURSOR_GOTO_FORM)
  //      cursorDrawVBar();  // only draws if VBar is enabled
}
  
/**********************************************************************/
void RemoveGraphCursor(void)
{
  CPIXOPS SelMode;
  
  // if cursor is showing, erase it
  if (GraphCursorOn)
    {
    CSetWritingMode(screen_handle, CdXORs, &SelMode);
    CPolymarker(screen_handle, 1, &LastCursorLoc);
    // Set the drawing mode to replace the current pixels
    CSetWritingMode(screen_handle, CReplace, &SelMode);
    }
  GraphCursorOn = FALSE;
  
  // only needed for GRAPHICAL scan setup form
  //   cursorEraseVBar();  // only erases if VBar is enabled
}
  
/**********************************************************************/
void DisplayActiveGraphCursor(void)
{
  CXY cursor = gss_position(&Plots[ActiveWindow],
    CursorStatus[ActiveWindow].X,
    CursorStatus[ActiveWindow].Y,
    CursorStatus[ActiveWindow].Z);
  DisplayGraphCursor(cursor);
}
  
/**********************************************************************/
BOOLEAN pointOnGraphCursor(SHORT XPos, SHORT YPos)
{
  if(GraphCursorOn)
    {
    SHORT halfMarkerHeight = CursorMarkerHeight / 2;
  
    if (   (XPos >= LastCursorLoc.x - halfMarkerHeight)
      && (XPos <= LastCursorLoc.x + halfMarkerHeight)
      && (YPos >= LastCursorLoc.y - halfMarkerHeight)
      && (YPos <= LastCursorLoc.y + halfMarkerHeight)
     )
      return TRUE;
    }
  return FALSE;
}
  
/**********************************************************************/
// move cursor for macro language
/**********************************************************************/
ERR_OMA moveCursorMacro(float xVal, float zVal)
{
  SaveAreaInfo * SavedArea = NULL;
  ERR_OMA err;
  
  if (GraphCursorOn)
    {
    erase_cursor();
    SetGraphCursorType(CursorType);
    }
  else   // cursor status will attempt to overwrite the second menu line
    SavedArea = save_screen_area(1, 0, 1, screen_columns);
  
  err = JumpCursor(xVal, zVal);
  
  if (SavedArea)
    {
    RemoveGraphCursor();
    restore_screen_area(SavedArea);
    }
  
  set_cursor_type(TextCursor);

  return err;
}
  
/**********************************************************************/
/* XVal, YVal, and ZVal in axis units */
/**********************************************************************/
void SetCursorPos(SHORT Window, FLOAT XVal, FLOAT YVal, FLOAT ZVal)
{
  CXY Cursor;
  SHORT PlotIndex = WindowPlotAssignment[Window];
  PLOTBOX * ThisPlot = &Plots[PlotIndex];
  
  if(ThisPlot->style == FALSE_COLOR)
    {
  
    float temp = YVal;
  
    YVal = ZVal;
    ZVal = temp;
    }
  
  Cursor = gss_position(ThisPlot, XVal, YVal, ZVal);
  
  if (Cursor.x > ThisPlot->plotarea.ur.x)
    Cursor.x = ThisPlot->plotarea.ur.x;
  else if (Cursor.x < ThisPlot->plotarea.ll.x)
    Cursor.x = ThisPlot->plotarea.ll.x;
  
  if (Cursor.y > ThisPlot->plotarea.ur.y)
    Cursor.y = ThisPlot->plotarea.ur.y;
  else if (Cursor.y < ThisPlot->plotarea.ll.y)
    Cursor.y = ThisPlot->plotarea.ll.y;
  
  DisplayGraphCursor(Cursor);
  
  /* Update cursor position */
  CursorStatus[PlotIndex].X = XVal;
  
  if(ThisPlot->style == FALSE_COLOR)
    {
    CursorStatus[PlotIndex].Y = ZVal;
    CursorStatus[PlotIndex].Z = YVal;
    } else {
    CursorStatus[PlotIndex].Y = YVal;
    CursorStatus[PlotIndex].Z = ZVal;
    }
}
