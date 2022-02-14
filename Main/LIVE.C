/***************************************************************************/
/*  live.c                                                                 */
/*                *** OMA4000 Version ***                                   */
/*                                                                         */
/*  copyright (c) 1988, EG&G Instruments Inc.                              */
/*                                                                         */
/*  Live curve handling for the  OMA4000 program.                           */
/*
*  $Header:   J:/logfiles/oma4000/main/live.c_v   1.36   30 Mar 1992 11:39:26   maynard  $
*  $Log:   J:/logfiles/oma4000/main/live.c_v  $
 *    Rev 1.35   12 Mar 1992 13:28:34   maynard
 * Removed include of access4.h, using driver calls for everything now.
 * References to ConvertedLive changed to ScaledLive, and
 * ConvertAndScaleLive changed to ScaleLive. SourceComp changed from
 * float to long (as it is delivered by the OMA4 controller.)  Use
 * driver calls to tell if detector is present or not.  Report the
 * timeout error which occasionally occurs on startup of acquisition.
 * Don't switch from false_color mode anymore when going to live, and
 * don't switch back after finishing live.  In fact, if the plot is set
 * up to do false color, then live plots the data in false color mode.
 * The sense of the value returned by the ACTIVE command to the driver
 * has inverted, so account for that.  Eliminate some extra stuff in the
 * live draw curves loop, stuff which was either vestigial or
 * misguided.  Try not to draw data on the screen until the DAC reports
 * that data has been updated.  Don't set YMAX and YMIN in
 * LiveFillLineCoord.  No conversions to float in ScaleLive (formerly
 * ConvertAndScaleLive) and no setting of YMax and YMin.
 *
 *    Rev 1.34   13 Jan 1992 12:27:52   cole
 * Change include's. Delete diags - no longer used. Use plotboxOutline(),
 * drawPlotboxOutline() for drawing outline and setting clip rectangle.
 * Delete SetFillAndClipPoly() - no longer used. Remove z_size from InitGoLive()
 * - no longer used. Use setClipRectToFullScreen(). Use screen_handle instead of
 * plotbox->dev_handle for drawing. Add CSetLineColor() call to LiveDrawCurve().
 * One less arg in FindSpecialEntry() and create_plotbox() calls. Comment out
 * correction by LINE_WIDTH in SetupLiveXPlot() - no longer needed.
 * Use isCurrentFormMacroForm(), isCurrentFormGraphWindow().
 * Add MacGoLive(), MacFreeRun(), MacStopLive() for macro support.
 *
 *    Rev 1.33   07 Jan 1992 11:56:56   maynard
 * delete pBackBuf (unused)
 * change LiveLoopDrawCurves to only plot curves selected in plot setup.
 * change sub_background to locate background curve in dir each time
 * it is called.
 * 
*/

#include <malloc.h>
#include <conio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <math.h>

#include "live.h"
#include "forms.h"
#include "runforms.h"
#include "doplot.h"    // create_plotbox()
#include "device.h"  // screen_handle, SetClipRectToFullScreen
#include "points.h"
#include "calib.h"
#include "tempdata.h"
#include "curvedir.h"
#include "di_util.h"
#include "cursor.h"
#include "omaform.h"
#include "fkeyfunc.h"
#include "backgrnd.h"
#include "macrecor.h"
#include "multi.h"    //fullscreen_count
#include "fcolor.h"   //false_color_array_plot
#include "oma4driv.h"
#include "detsetup.h"
#include "coolstat.h"
#include "formwind.h"
#include "omazoom.h"
#include "pltsetup.h"
#include "omamenu.h"
#include "syserror.h"
#include "omameth.h"   // InitialMethod
#include "crventry.h"
#include "omaerror.h"
#include "plotbox.h"
#include "curvbufr.h"
#include "curvdraw.h"  // Replot()
#include "graphops.h"  // InitGraphOps()
#include "lineplot.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

static CXY polygon[7];     // vertices of area to fill for erasing
static SHORT number_vertices;
static SHORT SavedPlotStyle;
//static char LiveXferBuf[BUFLEN];

CHAR *AcqFKeyText;

BOOLEAN InLiveLoop = FALSE;
BOOLEAN SaveLiveFrame = TRUE;
BOOLEAN EmptyFrame;
BOOLEAN DoAccum = FALSE;
USHORT SaveCurveNum;
BOOLEAN DoLive = FALSE;
USHORT LiveFilterMode = No_Filter;

CHAR LiveFileName[FNAME_LENGTH];       // initialized in oma4000.c
CHAR AccumFileName[FNAME_LENGTH];      // initialized in oma4000.c
CHAR LiveRefName[FNAME_LENGTH] = {'\0'};
CHAR *SaveFileName;

// These variables should be set and used carefully since they
// will be subject to side effects of any virtual data function 1/13/90

SHORT LiveBlkIndex = NOT_FOUND;
SHORT RefEntryIndex = NOT_FOUND;
SHORT LiveCapIndex;
BOOLEAN BackGroundActive;

static USHORT LivePointNum;
static USHORT curve_count = 1; /* which curve is being plotted */

long FrameSourceComp;

CRECT ClipRect, OldClipRect;

BOOLEAN Paused, NextStep;

static SHORT LiveCurvesToDump;
static USHORT AcqIndex = 0;

CHAR * LastLiveEntryName = "lastlive";
CHAR * PauseStr = "F4 Pause";

static CHAR * NextStr = "F4 Next";
static CHAR * FreeRunStr = "F2 Free Run";
static CHAR * LastLiveDescription = "Last Live Curve Set";

static BOOLEAN LiveOffsetUpdate;
static BOOLEAN LiveOverLimit;
static FLOAT   LiveZValue;

static PLOTBOX * LivePlot;
WINDOW * MessageWindow;

SHORT BackGroundIndex;

// static function declarations
static void HandleLiveKey(UCHAR key);
static void LiveHotKeys(BOOLEAN Startup);
static SHORT NextLive(unsigned int);


PRIVATE ERR_OMA VerifyBackground(void)
{
  /* if background is on, make sure background curve still exists */

  if (BackGroundActive)
    {
    BackGroundIndex = FindSpecialEntry(BackGroundEntryName);
    if (BackGroundIndex == NOT_FOUND)
      {
      BackGroundActive = FALSE;
      return error(ERROR_CURVESET_NOT_FOUND, BackGroundEntryName);
      }
    }
  return ERROR_NONE;
}


/***********************************************************************/
ERR_OMA SetupAcqCurveBlk(USHORT DataPoints, USHORT Curves)
{
  CURVEHDR * pCvHdr;
  ERR_OMA err;
  CURVE_ENTRY *pNewEntry;
  double XMin, XMax;
  USHORT BufNum = 0;

  pCvHdr = &CvBuf[BufNum].Curvehdr;

  /* flush all temporary buffers */
  if(err = clearAllCurveBufs())
    return err;

  /* delete the old last live curve */

  LiveBlkIndex = FindSpecialEntry(LastLiveEntryName);
  if (LiveBlkIndex != NOT_FOUND)
    {
    if (err = DelTempFileBlk(&MainCurveDir, LiveBlkIndex))
      return err;
    else
      if(LiveBlkIndex < LiveCapIndex) LiveCapIndex--; /* why? MLM */
    }

  LiveBlkIndex = MainCurveDir.BlkCount;

  /* create LastLive curve block to hold curve header for live */

  if(err = CreateTempFileBlk(&MainCurveDir,
                             &LiveBlkIndex,         /* return entry index  */
                             LastLiveEntryName,     /* name of block       */
                             "",                    /* path                */
                             LastLiveDescription,   /* description         */
                             0,                     /* start curve number  */
                             0L,                    /* start block offset  */
                             0,                     /* number of curves    */
                             NULL,                  /* pointer to curvehdr */
                             OMA4MEMDATA,           /* entry type          */
                             1 << ActiveWindow))    /* displaywindow       */
    return err;
  else
    {
    pNewEntry = &MainCurveDir.Entries[LiveBlkIndex];
    strcpy(pNewEntry->name, LastLiveEntryName);
    pNewEntry-> path[ 0 ] = '\0';
    }

  if (LiveFilterMode == AbsorbDual)
    RefEntryIndex = LiveBlkIndex;

  CheckFilterStatus();
  
  pCvHdr->pointnum = DataPoints;
  pCvHdr->XData.XUnits = InitialMethod->CalibUnits[0];
  pCvHdr->XData.XArray = (PVOID)((ULONG)CvBuf[BufNum].BufPtr + (ULONG)(BUFLEN / 2));
  pCvHdr->YUnits = COUNTS;
  pCvHdr->Ymin = (FLOAT) 0;
  pCvHdr->Ymax = (FLOAT) 0;
  pCvHdr->DataType = LONGTYPE;
  pCvHdr->experiment_num = 0;
  pCvHdr->time = (FLOAT) 0;
  pCvHdr->scomp = 0;
  pCvHdr->CurveCount = Curves;
  pCvHdr->MemData = TRUE;
  pCvHdr->pia[0] = 0;
  pCvHdr->pia[1] = 0;

  XMin = (double) 0;
  XMax = (double) pCvHdr->pointnum - 1;

  if (InitialMethod->CalibUnits[0] != COUNTS)
    {
    XMin = ApplyCalibrationToX(InitialMethod->CalibCoeff[0], (FLOAT) XMin);

    XMax = ApplyCalibrationToX(InitialMethod->CalibCoeff[0], (FLOAT) XMax);
    }

  pCvHdr->Xmin = (FLOAT) XMin;
  pCvHdr->Xmax = (FLOAT) XMax;

  err = ChangeTempFileSize(tempFileSize() + sizeof(CURVEHDR));

  tempFileIncrementSize(sizeof(CURVEHDR));

  /* write out the curve header to the temp file */

  if (fseek(hTempFile, pNewEntry->TmpOffset, SEEK_SET) != 0)
    return error(ERROR_SEEK, TempFileBuf);

  if (fwrite(pCvHdr, sizeof(CURVEHDR), 1, hTempFile) != 1)
    return error(ERROR_WRITE, TempFileBuf);

  GenXData(DataPoints, InitialMethod->CalibCoeff[0]);

  pNewEntry->count = Curves;
  MainCurveDir.CurveCount += (Curves);

  err = VerifyBackground();

  if (err)
    {
    DelMultiTempCurve(&MainCurveDir, LiveBlkIndex, 0, Curves);
    return err;
    }
  return ERROR_NONE;
  }


/***********************************************************************/
static void near get_curves_to_dump(void)
{
  LiveCurvesToDump = det_setup->tracks * det_setup->memories;
}

/***********************************************************************/
static void no_go_live(void)              /* aborts GoLive and cleans up */
{
  if (MessageWindow != NULL)
    {
    release_message_window(MessageWindow);
    MessageWindow = NULL;
    }
  DoLive = FALSE;
  InLiveLoop = FALSE;
  InitCursor(ActiveWindow, CursorType);
  StopLive(0);
  if (MessageWindow != NULL)
    release_message_window(MessageWindow);

  MessageWindow = NULL;

  return;
}

/***********************************************************************/
BOOLEAN InitGoLive(void)
{
  USHORT WindowMask, k;
  ERR_OMA err = ERROR_NONE;
  float real_detector;

  GetParam(DC_THERE, &real_detector);

  if(!real_detector)
    return TRUE;

  /* Make sure that the active window is still displayed */
  /* MAXPLOTS - 1 is for calibration plot */

  LivePlot = &Plots[ActiveWindow];
  Paused = NextStep = EmptyFrame = FALSE;

  AcqFKeyText = FKeyItems[1].Text;

  ShowFKeys(&FKey);

  /* switch MODE part of cursor status line to "ACCUM" or "LIVE" */
  if (! DoAccum)
     UpdateCursorMode(CURSORMODE_LIVE);
  else
     UpdateCursorMode(CURSORMODE_ACCUM);

  put_up_message_window(BusyWorking, COLORS_MESSAGE, &MessageWindow);

  /* define polygon to erase curves before update */
  // use offset of 1 to move polygon one device pixel inside the axes.
  plotboxOutline(LivePlot, polygon, & number_vertices, 1);
  CInqClipRectangle(screen_handle, &OldClipRect);

  /*  start DA here so less delay from button press to data in */

  set_Lastscan(0);
  GetParam(DC_DMODEL, &real_detector);

  if(SetParam(DC_RUN, (!(DoAccum || (real_detector == RAPDA))+2)))
   {
   float run_error;

   GetParam(DC_DERROR, &run_error);
   if (run_error)
     {
     error(ERROR_DETECTOR_TIMEOUT, "RUN");
     no_go_live();
     return TRUE;
     }
   }

  DoLive = InLiveLoop = TRUE;

  LivePointNum = det_setup->points;
  get_curves_to_dump();

  if (DoAccum || SaveLiveFrame)
    SaveCurveNum = LiveCurvesToDump;
  else
    SaveCurveNum = 1;                // save one curve in live

  /* setup the live block */
  if (DoAccum)
    SaveFileName = AccumFileName;
  else
    SaveFileName = LiveFileName;

  if (SetupAcqCurveBlk(LivePointNum, LiveCurvesToDump))
    {
    no_go_live();
    return TRUE;
    }

  /* set cursor to track live curve set only */
  CursorStatus[ActiveWindow].EntryIndex = LiveBlkIndex;
  CursorStatus->UTTotalCurves = LiveCurvesToDump;
  CursorStatus->FileCurveNumber = 0;
  CursorStatus->Z = CursorStatus->UTDisplayCurve = 0;

  if (err)
    {
    no_go_live();
    return TRUE;
    }

  if (MessageWindow != NULL)
    {
    release_message_window(MessageWindow);
    MessageWindow = NULL;
    }

  // set up the actions for F keys and all hot keys for live mode
  LiveHotKeys(TRUE);

  if (MenuFocus.ActiveMENU == &MainMenu)
    UnselectifyMenu();

  // remove all other curves from display in the current active window
  WindowMask = ~(1 << ActiveWindow);
  for (k=0; k<MainCurveDir.BlkCount; k++)
    {
    if (k != (USHORT)LiveBlkIndex)
      MainCurveDir.Entries[k].DisplayWindow &= WindowMask;
    }
  return FALSE;
}

/***********************************************************************/
void ExitGoLive()
{
  float real_detector;

  GetParam(DC_THERE, &real_detector);

  if(!real_detector)
    return;

  LiveHotKeys(FALSE); // restore the actions for F keys and all hot keys

  /* if live data was in a curve buffer before startup, make sure */
  /* it gets reloaded from the OMA4 board before plotting, etc. */

  clearAllCurveBufs();  /* force reload of live before plotting */

  CSetClipRectangle(screen_handle, OldClipRect);

  // erase the live plots and replot with curves assigned to this window

  InitCursor(ActiveWindow, CursorType);
  
  Replot(ActiveWindow);

  /* call StopLive if exiting because of mode switch */
  /* or at the end of accum */
  
  if (Current.Form->status == FORMSTAT_SWITCH_MODE)
    StopLive(0);
  else if (DoAccum)
    StopLive(1);

  InLiveLoop = FALSE;

  SetCursorPos(ActiveWindow,
               CursorStatus[ActiveWindow].X,
               CursorStatus[ActiveWindow].Y,
               CursorStatus[ActiveWindow].Z);

  DoLive = FALSE;

  if (MenuFocus.ActiveMENU == &MainMenu)
    {
    redraw_menu();
    unhighlight_menuitem(MenuFocus.ItemIndex);
    }
  
  displayCoolerStatus(TRUE); /* maintain cooler status display. */
}

/***********************************************************************/
void GoLive(unsigned int dummy)
{
  if (pKSRecord != NULL)
    {
    if (*pKSRecord)
      MacRecordString("GO_LIVE();\n");
    }

  if((!InLiveLoop) && InitGoLive()) // check to see if already initialized
    return;                         // by a macro

  LiveLoop(); // may want to make this an idle loop handler later
  ExitGoLive();
}

/***********************************************************************/
static void LookForLiveInput(void)
{
  USHORT Key = '\0';
  BOOLEAN KeyHit = kbhit();

  if ((poll_mouse_event != NULL) && !KeyHit)
    {
    Key = (* poll_mouse_event)();
    if(Key != '\0')
      KeyHit = TRUE;
    }

  if (pKSPlayBack != NULL)
    if (*pKSPlayBack == TRUE)
      KeyHit = TRUE;             // fake a key hit

  if (KeyHit)
    HandleLiveKey((UCHAR) (Key & 0xFF));

  ShiftCheck();
  displayCoolerStatus(FALSE);
}

/***********************************************************************/
static void pauseDoneCheck(void)
{
  float active;

  if(Paused)
    {
    if(!EmptyFrame)
      {
      while(DoLive && Paused && (!NextStep))
        LookForLiveInput();
    
      NextStep = FALSE;        // show next curve only
      }
    }
  else
    LookForLiveInput();

  GetParam(DC_ACTIVE, &active);
  if(DoAccum && (!active))
    DoLive = FALSE;
}

/**************************************************************************/
/*                                                                        */
/* During Live, the status line at the top of the screen is updated       */
/* by this routine after each curve is drawn.  Then the old graphic       */
/* cursor is erased, and a new one is drawn on the latest curve.  The GSS */
/* ClipRectangle is used to prevent the cursor from being drawn outside   */
/* the plotbox borders.  A record is kept of the ClipRect used to draw the*/
/* previous cursor, since the XOR mode of drawing is used to erase the    */
/* old cursor .                                                           */
/*                                                                        */
/**************************************************************************/

static void LiveDrawCursor(PLOTBOX * Plot, int Curve, BOOLEAN OldCursor)
{
  CURSORSTAT * CursorStat = & CursorStatus[ ActiveWindow ];
  FLOAT XVal = CursorStat->X;
  static CXY oldMarkerLoc;
  CHAR buffer [16];
  SHORT prefBuf = 0;

  CursorStat->Z = Curve;

  GetDataPoint(&MainCurveDir,                /* Get value to report for Y */
    CursorStat->EntryIndex,
    Curve,                                   /* Assumes that */
    CursorStat->PointIndex,                  /* CursorStat->PointIndex */
    &XVal, &(CursorStat->Y), FLOATTYPE,      /* is correctly set */
    &prefBuf);

  UpdateCursorYStat(ActiveWindow);      /* X axis done by cursor move */
  UpdateCursorZStat(ActiveWindow);      /* routine */

  GetParam(DC_I, &XVal);
  sprintf(buffer,"I=%-4.0f  ",XVal);    /* print up to 4 chars, no dec pt. */
  UpdatePlotscreenString(2, 51, buffer);

  GetParam(DC_SCMP, &XVal);
  sprintf(buffer,"SC=%-5.5g   ",XVal);  /* up to 5 chars, may have dec. */
  UpdatePlotscreenString(2, 59, buffer);

  /* if data display will fit, draw cursor */
  if(! LiveOverLimit)
    {
    static CRECT previousClipRect;
    CPIXOPS SelMode;
    CXY cursorLoc = gss_position(Plot, CursorStat->X,
                                       CursorStat->Y,
                                       CursorStat->Z);
    if(OldCursor) /* don't do this right after screen erase */
      {
      CSetClipRectangle(screen_handle, previousClipRect);
      CSetWritingMode(screen_handle, CdXORs, &SelMode);
      CPolymarker(screen_handle, 1, &oldMarkerLoc);
      }

   /* remember the clipping rectangle for use next time to erase the */
   /* polymarker which is about to be drawn                          */

    previousClipRect = ClipRect;
    CSetClipRectangle(screen_handle, ClipRect);

    /* now put the graphics cursor on the curve */

    CSetWritingMode(screen_handle, CdXORs, & SelMode);
    CPolymarker(screen_handle, 1, & cursorLoc);
    CSetWritingMode(screen_handle, CReplace, & SelMode);

    oldMarkerLoc = cursorLoc;

    /* allow area fill and cursor update to use entire screen */
    setClipRectToFullScreen();
    }
}

// if this is 1st (or only) curve in frame, erase old curves 1st
void WipeLivePlot(void)
{
  CCOLOR SelColor;
  CRECT currClipRect;

  CInqClipRectangle(screen_handle, &currClipRect);
  setClipRectToFullScreen();
  CSetFillColor(screen_handle, LivePlot->background_color, &SelColor);
  CFillArea(screen_handle, number_vertices, polygon);
  drawPlotboxOutline(LivePlot); /* because curve may draw ON axes */
  CSetClipRectangle(screen_handle, currClipRect);
}

/***********************************************************************/
PRIVATE ERR_OMA LiveLoopDrawCurves(PLOTBOX * Plot, USHORT * curveCountPtr)
{
  SHORT maxval, minval, k, prefBuf = 0;
  USHORT dummy;
  ERR_OMA err = ERROR_NONE;

  if (Plot->style == FALSE_COLOR)
    {
    maxval = (SHORT)Plot->y.max_value;
    minval = (SHORT)Plot->y.min_value;
    }
  else
    {
    maxval = (SHORT)Plot->z.max_value;
    minval = (SHORT)Plot->z.min_value;
    }
  
  if (maxval < minval)
    {
    SHORT temp = maxval;
    maxval = minval;
    minval = temp;
    }

  if (minval < 0)
    minval = 0;

  for (k = minval; k <= maxval && DoLive; k++)
    {
    LookForLiveInput();
    if (DoLive && k < LiveCurvesToDump)
      {
      dummy = 0;
      err = LoadCurveBuffer(&MainCurveDir, LiveBlkIndex, k, &dummy, &prefBuf);
      if (err)
        {
        StopLive(0);
        return err;
        }

      if (Plot->style == FALSE_COLOR)
        {
        if (k == minval)
          WipeLivePlot();
        false_color_array_plot((float)k,  /* Z value */
          &MainCurveDir,                  /* curve directory */
          LiveBlkIndex,                   /* curve blk */
          k,                              /* curve */
          LivePointNum,
          (unsigned char)Plot->x.units,
          FALSE);                         /* expand-on-tagged */
        }
      else
        {
        if (k == minval)
          setPreDrawTask(WipeLivePlot);   /* will erase right before draw */
        array_plot(Plot, &MainCurveDir,
                   LiveBlkIndex, k,
                   0, LivePointNum,
                   LiveZValue);
        }

      LiveDrawCursor(Plot, k, (*curveCountPtr > 1));

      CvBuf[prefBuf].ActiveDir = 0L;

      if(LiveOffsetUpdate)
        {
        LiveZValue ++;
        LiveOverLimit = CalcClipRect(Plot, LiveZValue, &ClipRect);
        if(* curveCountPtr < (USHORT)LiveCurvesToDump)
           * curveCountPtr += 1;
        }
      FlushCurveBuffer(prefBuf);
      }
    pauseDoneCheck();
    }
  return ERROR_NONE;
}

/******************************************************************/
void LiveLoop()
{
  PLOTBOX *Plot = ActivePlot;
  SHORT current_j, last_scan, this_scan;
  float real_detector;
  BOOLEAN Draw;
  
  GetParam(DC_THERE, &real_detector);
  if(!real_detector)
    return;

  GetParam(DC_DMODEL, &real_detector);   /* For rapda checks later. */

  /* need to update X axis and curve offsets if have real Z axis */
  
  LiveOffsetUpdate = Plot->z_position && (LiveCurvesToDump - 1);

  displayCoolerStatus(TRUE);  // force cooler status display

  get_Lastscan(&last_scan);
  this_scan = last_scan;
  WipeLivePlot();

  while (DoLive && (Current.Form->status != FORMSTAT_SWITCH_MODE))
    {
    get_Mem(&current_j);         /* check ASIC progress */

    if (!DoLive)                 /* if stopped during a next step seq */
      Paused = FALSE;

    if (Paused && (!NextStep))
      DoLive = FALSE;          /* don't take a new live curve */
    else
      NextStep = FALSE;        /* take just one curve */

    if (DoLive)
      {
      curve_count = 1;
      LiveZValue = Plot->z.original_min_value;
      if (Plot->z.original_max_value < LiveZValue)
        LiveZValue = Plot->z.original_max_value;

      LiveOverLimit = CalcClipRect(Plot, LiveZValue, &ClipRect);

      if (real_detector == RAPDA)
        {
        float active;
        GetParam(DC_ACTIVE, &active);
        Draw = !active;
        }
      else
        {
        if (LiveDrawFast(Plot))
          this_scan++;
        else
          get_Lastscan(&this_scan);
        Draw = (this_scan != last_scan);
        }

      if (Draw)
        {
        last_scan = this_scan;
        if(LiveLoopDrawCurves(Plot, &curve_count))
          return;  // error
        if ((!DoAccum) && (real_detector == RAPDA))
          SetParam(DC_RUN, 2);           /* If stopped, start another run. */

        }
      else
        pauseDoneCheck();

      EmptyFrame = FALSE;          /* reset to take in next full frame */
      }
    else
      LookForLiveInput();

    /* resume free run in case pause becomes invalid the next time around*/

    if (Paused)
      DoLive = TRUE;
    }
  setClipRectToFullScreen();
}

/******************************************************************/
/* Since this function is called whenever live data is retrieved, */
/* the GetDataPoint call within it could cause it to be called    */
/* recursively, which would cause the background to be subtracted */
/* from itself ad infinitum                                       */
/******************************************************************/
ERR_OMA sub_background(PLONG Data, USHORT curvenum, CURVEHDR * pCvhdr)
{
  static BOOLEAN Gate = FALSE; /* prevents recursion */
  ERR_OMA err;
  FLOAT lX, fY, RefX, RefY, ExposeTime;
  LONG lY, *pRef;
  USHORT i, BackCurve, RefCurve, points = pCvhdr->pointnum;
  SHORT prefBuf = 1, refBuf = 2;

  if (!Gate)
    {
    Gate = TRUE;

    CheckFilterStatus();

    if (LiveFilterMode == YIntensCorrex)
      {
      GetParam(DC_ET, &ExposeTime);
      if (ExposeTime < 1e-6F)
        ExposeTime = 1.0F;
      }

    if (BackGroundActive || LiveFilterMode != No_Filter)
      {
      int tempcount;

      if (BackGroundActive)
        {
        if (err = VerifyBackground())
          {
          Gate = FALSE;
          return err;
          }

        tempcount = MainCurveDir.Entries[BackGroundIndex].count;
        if (tempcount == 0)
          tempcount++;
        BackCurve = curvenum % tempcount;
        }

      if (LiveFilterMode != No_Filter)
        {
        tempcount = MainCurveDir.Entries[RefEntryIndex].count;
        if (tempcount == 0)
          tempcount++;
        if (LiveFilterMode != AbsorbDual)
          RefCurve = curvenum % tempcount;
        else            /* Dual Track Absorbance, ref curve is other track */
          {
          if (curvenum == 0)
            RefCurve = 1;
          else
            RefCurve = 0;
          pRef = (LONG *)CvBuf[LIVE_XFER_BUF].BufPtr;
          ReadCurveFromMem((void *)pRef, pCvhdr->pointnum * sizeof(LONG),
                           RefCurve);
          }
        }

      for (i = 0; i < points; i++)
        {
        if (BackGroundActive)
          {
          if (err = GetDataPoint(&MainCurveDir,
                                 BackGroundIndex,
                                 BackCurve,
                                 i,
                                 &lX,
                                 (void *)&lY, LONGTYPE, &prefBuf))
            {
            break;
            }
          Data[i] = Data[i] - lY;
          }

        if (LiveFilterMode != No_Filter)
          {
  /*************************************************************************/
  /* If "filtering" is turned on, do an absorbance or intensity correction */
  /* on the raw data.  This assumes that RefEntryIndex points to a valid   */
  /* correction curve set.  It also assumes that the live curve set was    */
  /* marked as FLOATTYPE in SetupAcquireCurveBlock                         */
  /*************************************************************************/
          fY = (float)Data[i];
          if (LiveFilterMode == AbsorbDual)
            RefY = (float)pRef[i];
          else if (err = GetDataPoint(&MainCurveDir, RefEntryIndex, RefCurve,
                                      i, &RefX, &RefY, FLOATTYPE, &refBuf))
            break;

          if (LiveFilterMode == Absorbance || LiveFilterMode == AbsorbDual)
            {
            if (RefY > (float)MAX_FLOAT_DIFF)
              {
              fY = fY / RefY;
              if (fY > 0.0F)
                fY = (float)-log10((double)fY);
              else
                fY = 0.0F;
              }
            else
              fY = 0.0F;
            }
          else if (LiveFilterMode == Transmission)
            {
            if (RefY > (float)MAX_FLOAT_DIFF)
              fY = fY / RefY;
            else
              fY = 0.0F;
            }
          else if (LiveFilterMode == YIntensCorrex)
            fY = (fY / ExposeTime) * RefY;
          
          ((float *)Data)[i] = fY;
          }
        }
      if (LiveFilterMode != No_Filter)
        pCvhdr->DataType = FLOATTYPE;
      }
    Gate = FALSE;
    }
  return err;
}

/******************************************************************/
static void HandleLiveKey(UCHAR key)
{
  CFILLSTYLE SelStyle;
  CCOLOR SelColor;

  if (pKSPlayBack != NULL) // ignore mouse character if in playback mode
    {
    if (*pKSPlayBack == TRUE)
    key = '\0';
    }

  if (key == '\0')
    key = get_FORM_key_input();

  OMAKey(key);

  /* 'set fill style */
  SelStyle.HatchIndex = 1;
  CSetFillStyle(screen_handle, SelStyle, &SelStyle);
  CSetFillColor(screen_handle, LivePlot->background_color,
     &SelColor);    /* 'set color */
}

/******************************************************************/
SHORT StopLive(USHORT dontforceit)
{
  float real_detector;
  GetParam(DC_THERE, &real_detector);
  if (!real_detector)
    return(TRUE);

  GetParam(DC_DMODEL, &real_detector);
  if ((!dontforceit) && (real_detector != RAPDA))
    SetParam(DC_STOP, (float)dontforceit);
  else
    {
    float active;
    WINDOW * MessageWindow;
    put_up_message_window(BusyWorking, COLORS_MESSAGE, &MessageWindow);
    do
      (GetParam(DC_ACTIVE, &active));
    while (active);
    if (MessageWindow != NULL)
      release_message_window(MessageWindow);
    clearAllCurveBufs();
    }

  /* turn on Go function keys */
  FKeyItems[1].Text = AcqFKeyText;
  FKeyItems[1].TextLen = (char) 0;
  FKeyItems[1].Action = GoLive;
  FKeyItems[1].Control &= ~ MENUITEM_INACTIVE;

  /* turn off Stop, Pause and Capture function keys */
  FKeyItems[2].Control |= MENUITEM_INACTIVE;
  FKeyItems[3].Text = PauseStr;
  FKeyItems[3].TextLen = (char) 0;
  FKeyItems[3].Action = PauseLive;
  FKeyItems[3].Control |= MENUITEM_INACTIVE;

  /* disable Next Frame */
  FKeyItems[33].Control |= MENUITEM_INACTIVE;

  FKeyItems[4].Control |= MENUITEM_INACTIVE;

  // Disable capture background
  FKeyItems[34].Control |= MENUITEM_INACTIVE;

  ShowFKeys(&FKey);
  UpdateCursorMode(CURSORMODE_NORMAL);
  Paused = FALSE;
  NextStep = FALSE;
  DoLive = FALSE;

  if (pKSRecord != NULL)
    {
    if (*pKSRecord)
      MacRecordString("STOP_LIVE();\n");
    }
  return FALSE;

}

/******************************************************************/
SHORT PauseLive(unsigned int dummy)
{
  /* turn on Go live key */
  FKeyItems[1].Text = FreeRunStr;
  FKeyItems[1].TextLen = (char) 0;
  FKeyItems[1].Action = FreeRun;
  FKeyItems[1].Control &= ~ MENUITEM_INACTIVE;  

  /* change Pause key to Next */
  FKeyItems[3].Text = NextStr;
  FKeyItems[3].TextLen = (char) 0;
  FKeyItems[3].Action = NextLive;

  /* enable Next Frame */
  FKeyItems[33].Control &= ~MENUITEM_INACTIVE;

  CSetClipRectangle(screen_handle, OldClipRect);
  ShowFKeys(&FKey);
  UpdateCursorMode(CURSORMODE_PAUSED);

  Paused = TRUE;
  EmptyFrame = FALSE;
  return FALSE;
}

/***************************************************************/
void MacPauseLive(void)
{
  PauseLive(0);
}

/******************************************************************/
static SHORT NextLive(unsigned int dummy)
{
  NextStep = TRUE;
  return FALSE;
}

/******************************************************************/
SHORT NextFrame(USHORT Dummy)
{
  NextStep = TRUE;
  EmptyFrame = TRUE;
  return FALSE;
}

/******************************************************************/
SHORT FreeRun(unsigned int dummy)
{
  FKeyItems[1].Text = AcqFKeyText;
  if (DoAccum)
     UpdateCursorMode(CURSORMODE_ACCUM);
  else
     UpdateCursorMode(CURSORMODE_LIVE);
  FKeyItems[1].TextLen = (char) 0;
  FKeyItems[1].Action = GoLive;
  FKeyItems[1].Control |= MENUITEM_INACTIVE;

  FKeyItems[3].Text = PauseStr;
  FKeyItems[3].TextLen = (char) 0;
  FKeyItems[3].Action = PauseLive;

  /* disable Next Frame */
  FKeyItems[33].Control |= MENUITEM_INACTIVE;

  CSetClipRectangle(screen_handle, OldClipRect);
  ShowFKeys(&FKey);
  CSetClipRectangle(screen_handle, ClipRect);

  Paused = FALSE;
  DoLive = TRUE;

  if (pKSRecord != NULL)
    {
    if (*pKSRecord)
      MacRecordString("FREE_RUN();\n");
    }
  return FALSE;
}

/***************************************************************/
void MacFreeRun(void)
{
  FreeRun(0);
}

/***************************************************************/
/*                                                             */
/* Return lastlive's block index and the most recently acquired*/
/* track (for now just used by autoscale routine)              */
/*                                                             */
/***************************************************************/
void LiveScaleParams(USHORT * BlkIndex, USHORT * CvIndex)
{
  *BlkIndex = LiveBlkIndex;
  *CvIndex = curve_count - 1;
}

/******************************************************************/
SHORT CaptureLive(USHORT Dummy)
{
  CHAR Name[DOSFILESIZE + 1];
  CHAR Path[DOSPATHSIZE + 1];

  time_t Time;
  TM *TimeStruct;
  ERR_OMA err;
  WINDOW * MessageWindow;
  CURVE_ENTRY *pEntry;

  ParsePathAndName(Path, Name, SaveFileName);

  LiveCapIndex = SearchCurveBlkDir(Name, Path, 0, &MainCurveDir);
  if (LiveCapIndex == NOT_FOUND)
    {
    /* Curve set does not exist */
    /* check to make sure that the possible saved curves will not */
    /* immediately overlap with some other curve set */

    switch(CheckCurveBlkOverLap(Path, Name, 0, SaveCurveNum-1, &LiveCapIndex))
      {
      case RANGEOK:
      case SPLITRANGE:
      case OVERLAPCAT:
        error(ERROR_NOT_UNIQUE_CURVE);
      return (FALSE);
      case NOOVERLAPCAT:
      case DISJOINT:
      case BADNAME:
      break;               // OK a unique name and range was given
      }

    time(&Time);
    TimeStruct = localtime(&Time);

    /* put the curve block entry into the current directory */

    if(err = AddCurveBlkToDir(Name, Path, InitialMethod->Description,
      0, 0L, 0, TimeStruct, &MainCurveDir,
      &LiveCapIndex, OMA4DATA, 1 << ActiveWindow))
      return err;

    pEntry = &(MainCurveDir.Entries[LiveCapIndex]);
    pEntry->TmpOffset = TempFileSz;
    pEntry->count = 0;
    }
  else
    pEntry = &(MainCurveDir.Entries[LiveCapIndex]);

  put_up_message_window(BusyWorking, COLORS_MESSAGE, &MessageWindow);

  // test for unique name, path, and curve number
  switch (CheckCurveBlkOverLap(pEntry->path,
                      pEntry->name,
                      pEntry->StartIndex + pEntry->count,
                      pEntry->StartIndex + pEntry->count + SaveCurveNum - 1,
                      (SHORT *) &Dummy))
    {
    case RANGEOK:
    case SPLITRANGE:
    case OVERLAPCAT:
      if (MessageWindow != NULL)
        {
        release_message_window(MessageWindow);
        MessageWindow = NULL;
        }
      error(ERROR_NOT_UNIQUE_CURVE);
      return FALSE;
    case NOOVERLAPCAT:
    case DISJOINT:
    case BADNAME:
    break;               // OK a unique name and range was given
    }

  /* dup this curve and PUSH into the Live capture curve block */
  err = InsertMultiTempCurve(&MainCurveDir,
           LiveBlkIndex,    /*src block entry*/
           0,               /*src start curve*/
           LiveCapIndex,    /*dst block entry*/
           0,               /*dst start curve*/
           SaveCurveNum);  /*how many curves*/

  if (MessageWindow != NULL)
    {
    release_message_window(MessageWindow);
    MessageWindow = NULL;
    }

  return err;
}

/******************************************************************/
static void LiveHotKeys(BOOLEAN Startup)
{
  if (Startup)      // starting live mode
    {
    /* turn on Pause, Stop and Capture function keys */
    FKeyItems[2].Control &= ~ MENUITEM_INACTIVE;

    if (! DoAccum)
      {
      // enable pause
      FKeyItems[3].Control &= ~ MENUITEM_INACTIVE;
      // enable capture
      FKeyItems[4].Control &= ~ MENUITEM_INACTIVE;

      FKeyItems[34].Control &= ~ MENUITEM_INACTIVE; // enable capture bkgrnd
      }

    FKeyItems[1].Control |= MENUITEM_INACTIVE;
    FKeyItems[8].Control |= MENUITEM_INACTIVE;
    FKeyItems[38].Control |= MENUITEM_INACTIVE;
    // disable control-F6, redraw all tagged curves.
    FKeyItems[ 25 ].Control |= MENUITEM_INACTIVE;
    // disable alt-F6, Expand on tagged curves.
    FKeyItems[ 35 ].Control |= MENUITEM_INACTIVE;
    ShowFKeys(&FKey);

    // Turn off hot keys
    AltHotKeyItems['G'-'A'].Control |= MENUITEM_INACTIVE;
    AltHotKeyItems['P'-'A'].Control |= MENUITEM_INACTIVE;
    AltHotKeyItems['R'-'A'].Control |= MENUITEM_INACTIVE;
    AltHotKeyItems['S'-'A'].Control |= MENUITEM_INACTIVE;
    AltHotKeyItems['T'-'A'].Control |= MENUITEM_INACTIVE;
    AltHotKeyItems['U'-'A'].Control |= MENUITEM_INACTIVE;

    CtrlHotKeyItems['T'-'A'].Control |= MENUITEM_INACTIVE;
    CtrlHotKeyItems['U'-'A'].Control |= MENUITEM_INACTIVE;

    // turn off cursor keys
    // leave left and right keys on.  RAC 6/07/91
    SpecialHotKeyItems[KEY_UP - KEY_ENTER].Control |= MENUITEM_INACTIVE;
    SpecialHotKeyItems[KEY_DOWN - KEY_ENTER].Control |= MENUITEM_INACTIVE;
    }
  else  // exiting from Live mode
    {
    FKeyItems[1].Control &= ~ MENUITEM_INACTIVE;
    /* turn off Pause, Stop and Capture function keys */
    FKeyItems[2].Control |= MENUITEM_INACTIVE;
    // disable pause
    FKeyItems[3].Control |= MENUITEM_INACTIVE;
    // disable Next frame
    FKeyItems[33].Control |= MENUITEM_INACTIVE;
    // disable capture
    FKeyItems[4].Control |= MENUITEM_INACTIVE;
    // disable capture background
    FKeyItems[34].Control |= MENUITEM_INACTIVE;
    FKeyItems[8].Control &= ~MENUITEM_INACTIVE;
    FKeyItems[38].Control &= ~MENUITEM_INACTIVE;

    // enable control-F6, redraw all tagged curves.
    FKeyItems[ 25 ].Control &= ~ MENUITEM_INACTIVE;
    // enable alt-F6, Expand on tagged curves.
    FKeyItems[ 35 ].Control &= ~MENUITEM_INACTIVE;

    ShowFKeys(&FKey);

    // Turn on hot keys
    AltHotKeyItems['G'-'A'].Control &= ~MENUITEM_INACTIVE;
    AltHotKeyItems['P'-'A'].Control &= ~MENUITEM_INACTIVE;
    AltHotKeyItems['R'-'A'].Control &= ~MENUITEM_INACTIVE;
    AltHotKeyItems['S'-'A'].Control &= ~MENUITEM_INACTIVE;
    AltHotKeyItems['T'-'A'].Control &= ~MENUITEM_INACTIVE;
    AltHotKeyItems['U'-'A'].Control &= ~MENUITEM_INACTIVE;

    CtrlHotKeyItems['T'-'A'].Control &= ~MENUITEM_INACTIVE;
    CtrlHotKeyItems['U'-'A'].Control &= ~MENUITEM_INACTIVE;

    // turn on cursor keys
    SpecialHotKeyItems[KEY_UP - KEY_ENTER].Control &= ~MENUITEM_INACTIVE;
    SpecialHotKeyItems[KEY_DOWN - KEY_ENTER].Control &= ~MENUITEM_INACTIVE;
    }
}

/******************************************************************/
void SwitchModeLiveKey(USHORT Dummy)
{
  float real_detector;

  GetParam(DC_THERE, &real_detector);

  if (!real_detector) return; /* mlm do we need this check ? */

   /* go to graph mode */
   if (isCurrentFormMacroForm() && isPrevFormGraphWindow())
     Current.Form->status = FORMSTAT_EXIT_THIS_FORM;
   else
     Current.Form->status = FORMSTAT_SWITCH_MODE;

   DoLive = TRUE;
}

/******************************************************************/
SHORT CaptureBackGround(USHORT Dummy)
{
   return(CopyToBackGround(LastLiveEntryName, 0, SaveCurveNum));
}

// live loop adustments if autoscale
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA LiveLoopAutoScaleAdjust(PLOTBOX * pPlotBox)
{
  if(LiveOffsetUpdate)
    {
    CRECT ClipRect;

    LiveOverLimit = CalcClipRect(pPlotBox, LiveZValue, & ClipRect);
    }
  return ERROR_NONE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacGoLive(void)       
{
  SaveAreaInfo *SavedArea;
  BOOLEAN RemGCursor = FALSE;

  if(plotAreaShowing())
    {
    if(isCurrentFormMacroForm())
      {
      RemGCursor = TempChangeCursorType(&SavedArea);
      RemoveGraphCursor();
      }

    InitGoLive();

    if(isCurrentFormMacroForm())
      {
      LiveLoop();
      ExitGoLive();
      TempRestoreCursorType(RemGCursor, &SavedArea);
      UnselectifyMenu();
      }
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacStopLive(void)       
{
   StopLive(0);
   ExitGoLive();
}
