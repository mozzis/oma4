/* -----------------------------------------------------------------------
/
/  curvdraw.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/curvdraw.c_v   0.38   06 Jul 1992 10:27:46   maynard  $
/  $Log:   J:/logfiles/oma4000/main/curvdraw.c_v  $
*/
  
#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif
  
#include <conio.h>     // kbhit()
  
#include "curvdraw.h"
#include "device.h"    // screen_handle
#include "hline.h"     // hplot_init()
#include "tempdata.h"  // ReadTempCurvehdr
#include "multi.h"     // WindowPlotAssignment
#include "cursor.h"    // SetCursorPos()
#include "curvbufr.h"  // clearAllCurveBufs()
#include "baslnsub.h"  // baslnsub_active(), baslnsub_plot()
#include "ycalib.h"    // ycalib_active(), ycalib_plot()
#include "fcolor.h"    // false_color_init(), etc
#include "autopeak.h"  // is_auto_peak, etc.
#include "wintags.h"   // is_tagged()
#include "forms.h"     // replace_mouse_cursor()
#include "tagcurve.h"  // TaggedColor()
#include "pltsetup.h"  // window_style
#include "splitfrm.h"  // InSplitForm
#include "crventry.h"
#include "doplot.h"    // ResizePlotForWindow()
#include "plotbox.h"
#include "curvedir.h"

// the following is only needed for GRAPHICAL scan setup
//#include "scanset.h"   // inScanSetup
//#include "scangraf.h"  // scanGrafPlotCurve
  
PRIVATE BOOLEAN oneCurveOnly = FALSE;
PRIVATE float oneCurveZval;  // Z for the one curve to draw.
  
// Replace plot_peak_labels and drawTaggedOnly with drawSpecifier argument.
/* -----------------------------------------------------------------------*/
//
//  void plot_curves_labels(CURVEDIR *pCurveDir, PLOTBOX *pPlotBox,
//                          SHORT DisplayWindow, SHORT drawSpecifier)
//
//  requires: pCurveDir - pointer to curvedirectory to be displayed
//            pPlotBox - pointer to active plotbox structure
//            DisplayWindow - indicator (0, 1, 2, 3, 4, 5, 6, etc.)
//                            showing active display window
//  returns:    TRUE if terminated before completion
//
/* ----------------------------------------------------------------------- */
BOOLEAN plot_curves_labels(CURVEDIR *pCurveDir, PLOTBOX * pPlotBox,
                           SHORT DisplayWindow, SHORT drawSpecifier)
{
  CXY p_min, p_max;
  CXY v_min, v_max;
  ERR_OMA err;
  CURVEHDR TCurvehdr;
  USHORT EntryIndex = 0;
  FLOAT zvalue, zmax, zmin;
  FLOAT XVirToPhys;
  FLOAT YVirToPhys;
  USHORT FileCurveNumber = 0;
  BOOLEAN Found, Inverted = FALSE;
  int Key;
  BOOLEAN Abort = FALSE;
  USHORT UTDisplayCurve;
  
  erase_mouse_cursor();

  clearLiveCurveBufs();

  if(!(drawSpecifier & DRAW_PEAK_LABELS))
    {
    if (deviceHandle() != screen_handle)
      create_plotbox(pPlotBox);
    }
  v_min.x = pPlotBox->x.axis_zero.x;
  v_min.y = pPlotBox->x.axis_zero.y;
  v_max.x = v_min.x + pPlotBox->x.axis_end_offset.x;
  v_max.y = v_min.y + pPlotBox->y.axis_end_offset.y;
  
  /* virtual limits of the graph window */
  
  switch (pPlotBox->z_position)
    {
    case RIGHTSIDE:
      v_max.x += pPlotBox->z.axis_end_offset.x;
      v_max.y += pPlotBox->z.axis_end_offset.y;
      break;
  
    case LEFTSIDE:
      v_min.x += pPlotBox->z.axis_end_offset.x;
      v_min.y += pPlotBox->z.axis_end_offset.y;
      break;
    }
  
  XVirToPhys = (FLOAT)((double) screen.LastXY.x /
  (double) screen.LastVDCXY.x);
  YVirToPhys = (FLOAT)((double) screen.LastXY.y /
  (double) screen.LastVDCXY.y);
  
  /* physical limits of the graph window */
  p_min.x= (int)((FLOAT) v_min.x * XVirToPhys);
  p_min.y= (int)((FLOAT) v_min.y * YVirToPhys);
  p_max.x= (int)((FLOAT) v_max.x * XVirToPhys);
  p_max.y= (int)((FLOAT) v_max.y * YVirToPhys);
  
  if (pPlotBox->style == HIDDENLINE_CURVES ||
      pPlotBox->style == HIDDENSURF_CURVES)
    {
    /* get a temp curve block entry for the horizon values */
    if(hplot_init(p_max.x - p_min.x + 1))
      {
      replace_mouse_cursor();
      return TRUE;
      }
    }
  // False color plot
  if(pPlotBox -> style == FALSE_COLOR)
    false_color_init(DisplayWindow, pPlotBox);
  
  // the following is only needed for GRAPHICAL scan setup
  //   if(* inScanSetup)
  //      scanGrafStartPlot();
  
  if(pPlotBox->style != FALSE_COLOR)
    {
    zmin = pPlotBox->z.min_value;
    zmax = pPlotBox->z.max_value;
    }
  else
    {
    zmin = pPlotBox->y.min_value;
    zmax = pPlotBox->y.max_value;
    }
  
  
  if (zmin <= zmax)
    Found = FindFirstPlotBlock(pCurveDir, &EntryIndex, &FileCurveNumber,
                               &UTDisplayCurve, DisplayWindow);
  else
    {
    float Temp = zmin;
    zmin = zmax;
    zmax = Temp;
    Found = FindLastPlotBlock(pCurveDir, &EntryIndex, &FileCurveNumber,
                              &UTDisplayCurve, DisplayWindow);
    Inverted = TRUE;
    }
  
  if (!ExpandedOnTagged)
    zvalue = (float)UTDisplayCurve;
  else
    zvalue = zmin;
  
  while (Found)
    {
    if(ReadTempCurvehdr(pCurveDir, EntryIndex, FileCurveNumber, &TCurvehdr))
      {
      replace_mouse_cursor();
      return TRUE;
      }
  
    // if curve is past the z axis limits, all done, break out of loop
    if((zvalue > zmin) && (zvalue > zmax) && ! Inverted)
      break;
    else if((zvalue < zmin) && (zvalue < zmax) && Inverted)
      break;
    // if curve is before the z axis limits, go to next curve
    else if((zvalue < zmin) && (zvalue < zmax) && ! Inverted)
     ;
    else if((zvalue > zmin) && (zvalue > zmax) && Inverted)
     ;
    // If plotting one curve only, don't do anything
    else if((oneCurveOnly) && (oneCurveZval != zvalue))
     ;
    else if((drawSpecifier & DRAW_TAGGED_ONLY) &&
            !is_tagged(WindowPlotAssignment[DisplayWindow], UTDisplayCurve))
  
      Key = '\0';          // Don't do anything real
  
    else if(drawSpecifier & DRAW_PEAK_LABELS)
      draw_peaks(pCurveDir, EntryIndex, FileCurveNumber, DisplayWindow,
                 &TCurvehdr, zvalue);
  
    else if(*is_auto_peak)
      auto_peak_plot(zvalue, EntryIndex, FileCurveNumber, & TCurvehdr);
  
    // the following is only needed for GRAPHICAL scan setup
    //      else if((* inScanSetup) && (pPlotBox->style != FALSE_COLOR))
    //         scanGrafPlotCurve(pCurveDir, EntryIndex, FileCurveNumber,
    //                            TCurvehdr.pointnum);
  
    else if(baslnsub_active())
      {
      if(pPlotBox->style == FALSE_COLOR)
        false_color_array_plot(zvalue, pCurveDir, EntryIndex,
                               FileCurveNumber, TCurvehdr.pointnum,
                               TCurvehdr.XData.XUnits,
                               drawSpecifier & DRAW_TAGGED_ONLY);
      baslnsub_plot(zvalue, EntryIndex, FileCurveNumber, & TCurvehdr);
      }
    else if(ycalib_active())
      {
      if(pPlotBox->style == FALSE_COLOR)
        false_color_array_plot(zvalue, pCurveDir, EntryIndex,
                               FileCurveNumber, TCurvehdr.pointnum,
                               TCurvehdr.XData.XUnits,
                               drawSpecifier &DRAW_TAGGED_ONLY);
      ycalib_plot(zvalue, EntryIndex, FileCurveNumber, & TCurvehdr);
      }
    else if (pPlotBox->style == HIDDENLINE_CURVES ||
             pPlotBox->style == HIDDENSURF_CURVES)
      {
      CCOLOR savedColor = pPlotBox->plot_color;
  
      // test for tagged color and switch colors.
      if(drawSpecifier & DRAW_TAGGED_ONLY)
        pPlotBox->plot_color = taggedColor(savedColor);
  
      if(pPlotBox->plot_color != WHITE)
        err = array_hplot(pPlotBox, zvalue, pCurveDir, EntryIndex,
        FileCurveNumber, &p_min, &p_max, &v_max, WHITE);
      else
        err = array_hplot(pPlotBox, zvalue, pCurveDir, EntryIndex,
        FileCurveNumber, &p_min, &p_max, &v_max, WHITE);
  
      // restore plotbox color if necessary
      if(drawSpecifier & DRAW_TAGGED_ONLY)
        pPlotBox->plot_color = savedColor;
  
      // now check for error.
      if(err)
        {
        hplot_end();
        replace_mouse_cursor();
        return TRUE;
        }
      }
    else if (pPlotBox->style == FALSE_COLOR)
      false_color_array_plot(zvalue, pCurveDir, EntryIndex,
      FileCurveNumber, TCurvehdr.pointnum,
      TCurvehdr.XData.XUnits,
      drawSpecifier & DRAW_TAGGED_ONLY);
    else   // default to standard curve drawing
      {
      CCOLOR savedColor = pPlotBox->plot_color;
  
      // test for tagged color and switch colors.
      if(drawSpecifier & DRAW_TAGGED_ONLY)
        pPlotBox->plot_color = taggedColor(savedColor);
  
      // call array_plot and then reset color and then check for error.
      err = array_plot(pPlotBox, pCurveDir, EntryIndex,
        FileCurveNumber, 0, TCurvehdr.pointnum, zvalue);
  
      // restore plotbox color if necessary
      if(drawSpecifier & DRAW_TAGGED_ONLY)
        pPlotBox->plot_color = savedColor;
  
      // now check for error.
      if(err)
        {
        replace_mouse_cursor();
        return TRUE;
        }
      }
  
    if (! Inverted)
      {
      Found = FindNextPlotCurve(pCurveDir, &EntryIndex, &FileCurveNumber,
                                &UTDisplayCurve, DisplayWindow);
      zvalue++;
      }
    else
      {
      Found = FindPrevPlotCurve(pCurveDir, &EntryIndex, &FileCurveNumber,
                                &UTDisplayCurve, DisplayWindow);
      zvalue--;
      }
  
    /* check for escape from curve draw */
    if(kbhit())
      {
      Key = getch();
      if(Key == ESCAPE)
        {
        Abort = TRUE;
        break;
        }
      }
    }

  if (pPlotBox->style == HIDDENLINE_CURVES ||
      pPlotBox->style == HIDDENSURF_CURVES)
    hplot_end();
  
  // the following is only needed for GRAPHICAL scan setup
  //   if(* inScanSetup)
  //      scanGrafEndPlot();
  
  replace_mouse_cursor();
  return Abort;
}
  
// This is a new implementation of plot_curves which first plots all the
// curves and then add peak labels "on top" of all the curves so that the
// labels will be legible.  The old plot_curves() function has been renamed
// to plot_curves_labels().  Return TRUE on abnormal termination.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN plot_curves(CURVEDIR *pCurveDir, PLOTBOX *pPlotBox, SHORT DisplayWindow)
{
  BOOLEAN Terminated;
  SHORT DrawType;
  
  if (ExpandedOnTagged)
    DrawType = DRAW_TAGGED_AS_NORMAL;
  else
    DrawType = DRAW_ALL;
  
  // first draw the curves
  Terminated = plot_curves_labels(pCurveDir, pPlotBox, DisplayWindow, DrawType);
  
  // if the plot box has peak labelling enabled and there is at least one
  // curve in the plot box enabled for peak labelling, add peak labels.
  if(pPlotBox -> plot_peak_labels.label_peaks && !Terminated)
    {
    int i;
    for(i = 0; i < MAX_LABELLED_CURVES; i ++)
      {
      if(pPlotBox -> plot_peak_labels.curve_peaks[i].enabled)
        {
        return plot_curves_labels(pCurveDir, pPlotBox, DisplayWindow,
                                  DrawType | DRAW_PEAK_LABELS);
        }
      }
    }
  return Terminated;
}
  
// draw a single curve in window using curve_color.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN draw_one_curve(USHORT window, float zVal, SHORT drawSpecifier)
  {
  BOOLEAN Terminated;
  
  oneCurveOnly = TRUE;
  oneCurveZval = zVal;
  
  Terminated = plot_curves_labels(&MainCurveDir, & Plots[ window ],
                                  window, drawSpecifier);
  oneCurveOnly = FALSE;
  
  return Terminated;
  }
  
// clear the interior of a plot box and then replot the curves in it without
// redrawing all the axis labels, tick marks, etc. The plotbox outline is
// redrawn since previous curves may have drawn on top of it.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReplotCurvesOnly(SHORT Window)
  {
  CXY       polygon[7];       // vertices of area to fill for erasing
  SHORT     number_vertices;  // number of vertices in polygon[]
  CCOLOR    selColor;
  int       index = WindowPlotAssignment[ Window ];
  PLOTBOX * plotbox = & Plots[ index ];
  
  erase_mouse_cursor();
  RemoveGraphCursor();
  
  // define polygon to erase curves only.
  // use offset of 1 to move polygon one device pixel inside the axes.
  plotboxOutline(plotbox, polygon, & number_vertices, 1);
  
  // erase everything inside the plotbox outline
  CSetFillColor(deviceHandle(), plotbox->background_color, &selColor);
  CFillArea(deviceHandle(), number_vertices, polygon);
  
  drawPlotboxOutline(plotbox);
  
  plot_curves(&MainCurveDir, plotbox, Window);
  
  SetCursorPos(Window, CursorStatus[index].X, CursorStatus[index].Y,
               CursorStatus[index].Z);
  
  replace_mouse_cursor();
  }
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Replot(SHORT Window)
  {
  int index = WindowPlotAssignment[Window];
  
  erase_mouse_cursor();
  RemoveGraphCursor();
  
  create_plotbox(&Plots[index]);
  
  plot_curves(&MainCurveDir, & Plots[index], Window);
  
  SetCursorPos(Window, CursorStatus[index].X, CursorStatus[index].Y,
  CursorStatus[index].Z);
  
  replace_mouse_cursor();
  }
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int PutUpPlotBoxes(void)
  {
  erase_mouse_cursor();
  RemoveGraphCursor();
  
  /* plot all the graph boxes */
  /* number of windows is not really the number of graphs, it is */
  /* currently just the style number */
  // allow plot reassignment in windows
  if(ExpandedOnTagged || InSplitForm())
    // the following is needed only for GRAPHICAL scan setup
    //      || * inScanSetup)
    {
    ResizePlotForWindow(ActiveWindow);
    create_plotbox(&Plots[ActiveWindow]);
    }
  else
    {
    SHORT i;
  
    for (i=0; i<fullscreen_count[window_style]; i++)
      {
      ResizePlotForWindow(i);
      create_plotbox(&Plots[WindowPlotAssignment[i]]);
      }
    }
  ForceCursorIntoWindow(ActiveWindow);
  RemoveGraphCursor(); 
  replace_mouse_cursor();
  return FALSE;
  }
  
  
/* DstWindow and SrcWindow are 0 based indices */
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
USHORT DupDisplayWindow(CURVEDIR *pCurveDir, USHORT DstWindow,
                        USHORT SrcWindow)
{
  USHORT i;
  USHORT SrcFlag, DstFlag;
  CURVE_ENTRY *pEntry;
  
  /* copy the plotbox info */
  Plots[WindowPlotAssignment[DstWindow]] =
    Plots[WindowPlotAssignment[SrcWindow]];
  
  /* clear the window from any currently displayed destination curves and */
  /* set the destination display window flag on all curves displayed in */
  /* the source window */
  SrcFlag = 1 << SrcWindow;
  DstFlag = 1 << DstWindow;
  CursorStatus[WindowPlotAssignment[SrcWindow]].TotalCurves = 0;
  
  for (i=0; i<pCurveDir->BlkCount; i++)
    {
    pEntry = &(pCurveDir->Entries[i]);
  
    /* clear destination window curves */
    if (pEntry->DisplayWindow & DstFlag)
      pEntry->DisplayWindow &= ~DstFlag;
  
    if (pEntry->DisplayWindow & SrcFlag)
      {
      pEntry->DisplayWindow |= DstFlag;
      CursorStatus[WindowPlotAssignment[DstWindow]].TotalCurves +=
      pEntry->count;
      }
    }
  return CursorStatus[WindowPlotAssignment[DstWindow]].TotalCurves;
}
  
/* -----------------------------------------------------------------------
/
/ BOOLEAN FindFirstPlotBlock(CURVEDIR pCurveDir, PSHORT pEntryIndex,
/                            PUSHORT pFileCurveNumber, SHORT Window)
/
/  requires: pCurveDir - pointer to curve directory to be displayed
/            pEntryIndex - pointer to returned entry index of the first
/                          curve block for this window.
/            pFileCurveNumber - pointer to returned index
/                               of the first curve for this window.
/            pCurveIndex - Display Curve Number (Z value) of the
/                          first displayed curve
/            Window -    number (0, 1, 2, 3, 4, 5, 6, etc.)
/                            showing active display window
/  returns:    TRUE if a curve is found
/              FALSE if no curve is found
/
/ ----------------------------------------------------------------------- */
  
BOOLEAN FindFirstPlotBlock(CURVEDIR *pCurveDir, SHORT *pEntryIndex,
                           USHORT *pFileCurveNumber, USHORT *pCurveIndex,
                           SHORT Window)
  {
  CURVE_ENTRY *pEntry;
  BOOLEAN Found = FALSE;
  SHORT PlotIndex = WindowPlotAssignment[Window];
  
  for (*pEntryIndex=0;
    (*pEntryIndex < (SHORT) pCurveDir->BlkCount) && !Found;
    (*pEntryIndex)++)
    {
    pEntry = &(pCurveDir->Entries[*pEntryIndex]);
    *pFileCurveNumber = pEntry->StartIndex;
  
    if ((pEntry->DisplayWindow & (1 << PlotIndex)) && pEntry->count)
      {
      *pCurveIndex = 0;
  
      /* When expanded on tagged curves, untagged curves don't count */
  
      if (ExpandedOnTagged && (! is_tagged(PlotIndex, 0)))
        Found = FindNextPlotCurve(pCurveDir, pEntryIndex, pFileCurveNumber,
                                  pCurveIndex, Window);
      else    // not expanded
        Found = TRUE;
      }
    }
  
  if (!Found)
    {
    *pEntryIndex = 0;
    *pCurveIndex = 0;
    }
  else
    (*pEntryIndex)--; // remove of last increment caused by for loop
  
  return Found;
  }
  
/* -----------------------------------------------------------------------
/
/ BOOLEAN FindLastPlotBlock(CURVEDIR pCurveDir, SHORT *pEntryIndex,
/                           USHORT *pFileCurveNumber, SHORT Window)
/
/  requires: pCurveDir - pointer to curve directory to be displayed
/            pEntryIndex - pointer to returned entry index of the first
/                          curve block for this window.
/            pFileCurveNumber - pointer to returned index
/                               of the first curve for this window.
/            pCurveIndex - Display Curve Number (Z value) of the
/                          last displayed curve
/            Window -    number (0, 1, 2, 3, 4, 5, 6, etc.)
/                            showing active display window
/  returns:    TRUE if a curve is found
/              FALSE if no curve is found
/
/ ----------------------------------------------------------------------- */
  
BOOLEAN FindLastPlotBlock(CURVEDIR *pCurveDir, SHORT *pEntryIndex,
                          USHORT *pFileCurveNumber, USHORT *pCurveIndex,
                          SHORT Window)
  {
  CURVE_ENTRY *pEntry;
  BOOLEAN Found = FALSE;
  SHORT PlotIndex = WindowPlotAssignment[Window];
  
  for (*pEntryIndex = (pCurveDir->BlkCount - 1);
    (*pEntryIndex >= 0) && (! Found); (*pEntryIndex)--)
    {
    pEntry = &(pCurveDir->Entries[*pEntryIndex]);
    *pFileCurveNumber = pEntry->StartIndex + pEntry->count - 1;
  
    // check to see if entry is assigned to this window
    if ((pEntry->DisplayWindow & (1 << PlotIndex)) && pEntry->count)
      {
      *pCurveIndex = CursorStatus[PlotIndex].UTTotalCurves - 1;
  
      // When expanded on tagged curves, untagged curves don't count
      if (ExpandedOnTagged && (! is_tagged(PlotIndex, *pCurveIndex)))
        Found = FindPrevPlotCurve(pCurveDir, pEntryIndex,
        pFileCurveNumber, pCurveIndex, Window);
      else    // not expanded
        Found = TRUE;
      }
    }
  
  if (!Found)
    {
    *pEntryIndex = 0;
    *pCurveIndex = 0;
    }
  else
    (*pEntryIndex)++; // remove last increment caused by for loop
  
  return Found;
  }
  
/* -----------------------------------------------------------------------
/
/ BOOLEAN FindNextPlotCurve(CURVEDIR *pCurveDir, SHORT *pEntryIndex,
/               USHORT *pFileCurveNumber, USHORT *pCurveIndex, SHORT Window)
/
/  requires: pCurveDir - pointer to curve directory to be displayed
/            pEntryIndex:
/                    on entry - pointer to current index of a curve block
/                               in this window
/                    on exit  - pointer to returned entry index of the first
/                               curve block for this window.
/            pFileCurveNumber:
/                    on entry - pointer to current index of a curve in this
/                               window
/                    on exit  - pointer to returned index
/                               of the next curve for this window.
/            pCurveIndex:
/                    on entry - Display Curve Number (Z value) of the
/                               current displayed curve
/                    on exit  - Display Curve Number (Z value) of the
/                               next displayed curve
/            Window:          - number (0, 1, 2, 3, 4, 5, 6, etc.)
/                               showing active display window
/  returns:    TRUE if a curve is found
/              FALSE if no next curve is found
/
/ ----------------------------------------------------------------------- */
  
BOOLEAN FindNextPlotCurve(CURVEDIR *pCurveDir, SHORT *pEntryIndex,
                          USHORT *pFileCurveNumber, USHORT *pCurveIndex,
                          SHORT Window)
  {
  USHORT i;
  CURVE_ENTRY *pEntry;
  BOOLEAN Found = FALSE;
  SHORT PlotIndex = WindowPlotAssignment[Window];
  USHORT TempCIndex  = *pCurveIndex;
  USHORT TempFCIndex = *pFileCurveNumber;
  SHORT  TempEIndex  = *pEntryIndex;
  
  /* assume that EntryIndex is set to a valid block for this window */
  /* and that if only showing tagged curves, that pCurveIndex is also */
  /* valid */
  
  pEntry = &(pCurveDir->Entries[*pEntryIndex]);
  while ((*pFileCurveNumber < (pEntry->StartIndex + pEntry->count - 1)) &&
    !Found)
    {
    (*pFileCurveNumber)++;
    (*pCurveIndex)++;
  
    /* When expanded on tagged curves, untagged curves don't count */
  
    if ((!ExpandedOnTagged) || (is_tagged(PlotIndex, *pCurveIndex)))
      Found = TRUE;
    }
  
  if (Found)
    return TRUE;
  
  for (i=*pEntryIndex + 1; (i < pCurveDir->BlkCount) & !Found; i++)
    {
    pEntry = &(pCurveDir->Entries[i]);
    *pFileCurveNumber = pEntry->StartIndex;
  
    if ((pEntry->DisplayWindow & (1 << PlotIndex)) && pEntry->count)
      {
      // When expanded on tagged curves, untagged curves don't count
      if (ExpandedOnTagged)
        {
        while (
          (*pFileCurveNumber < (pEntry->StartIndex + pEntry->count)) &&
          ! Found)
          {
          if (is_tagged(PlotIndex, *pCurveIndex))
            {
            Found = TRUE;
            *pEntryIndex = i;
            }
          else
            {
            (*pCurveIndex)++;
            (*pFileCurveNumber)++;
            }
          }
        }
      else
        {
        *pEntryIndex = i;
        Found = TRUE;
        }
      }
    else
      {
      *pEntryIndex = i;
      }
    }
  
  if (! Found)
    {
    *pEntryIndex = TempEIndex;
    *pCurveIndex = TempCIndex;
    *pFileCurveNumber = TempFCIndex;
    }
  return Found;
  }
  
/* -----------------------------------------------------------------------
/
/ BOOLEAN FindPrevPlotCurve(CURVEDIR *pCurveDir, SHORT *pEntryIndex,
/         USHORT *pFileCurveNumber, USHORT *pCurveIndex, SHORT Window)
/
/  requires: pCurveDir:       - pointer to curve directory to be displayed
/            pEntryIndex:
/                    on entry - pointer to current index of a curve block
/                               in this window
/                    on exit  - pointer to returned entry index of the
/                               previous curve block for this window.
/            pFileCurveNumber:
/                    on entry - pointer to current index of a curve in this
/                               window
/                    on exit  - pointer to returned index
/                               of the previous curve for this window.
/            pCurveIndex:
/                    on entry - Display Curve Number (Z value) of the
/                               current displayed curve
/                    on exit  - Display Curve Number (Z value) of the
/                               previous displayed curve
/            Window:          - number (0, 1, 2, 3, 4, 5, 6, etc.)
/                               showing active
/                               display window
/  returns:    TRUE if a curve is found
/              FALSE if no previous curve is found
/
/ ----------------------------------------------------------------------- */
  
BOOLEAN FindPrevPlotCurve(CURVEDIR *pCurveDir, SHORT *pEntryIndex,
                          USHORT *pFileCurveNumber, USHORT *pCurveIndex,
                          SHORT Window)
{
  USHORT i;
  CURVE_ENTRY *pEntry;
  BOOLEAN Found = FALSE;
  SHORT PlotIndex = WindowPlotAssignment[Window];
  
  /* assume that EntryIndex is set to a valid block for this window */
  /* and that if only showing tagged curves, that pCurveIndex is also */
  /* valid */
  
  pEntry = &(pCurveDir->Entries[*pEntryIndex]);
  while ((*pFileCurveNumber > pEntry->StartIndex) && !Found)
    {
    (*pFileCurveNumber)--;
    (*pCurveIndex)--;
  
    if ((!ExpandedOnTagged) || (is_tagged(PlotIndex, *pCurveIndex)))
      Found = TRUE;
    }
  
  if (Found)
    return TRUE;
  
  if (*pEntryIndex == 0)
    return FALSE;
  
  i=*pEntryIndex;
  do  /* problem here is that curvenum is decremented to point to */
      /* previous (un)displayed curves when it should not be */
    {
    i--;                       // try previous curve set
    pEntry = &(pCurveDir->Entries[i]);
    *pFileCurveNumber = pEntry->StartIndex + pEntry->count - 1;
  
    if ((pEntry->DisplayWindow & (1 << PlotIndex)) && pEntry->count)
      {
      if (ExpandedOnTagged)
        {
        while (*pFileCurveNumber >= pEntry->StartIndex && !Found)
          {
          if (is_tagged(PlotIndex, *pCurveIndex))
            Found = TRUE;
          else
            {
            (*pCurveIndex) --;
            (*pFileCurveNumber)--;
            }
          }
        }
      else
        {
        *pEntryIndex = i;
        Found = TRUE;
        }
      }
    }
  while ((i > 0) && (!Found));
  
  return Found;
}
  
