/***************************************************************************/
/* splitfrm.c                                                              */
/* standard method for using split form/graph screen                       */
/* Morris Maynard July 1993 Copyright (C) 1993 EG&G Instruments            */
/* $Header:                                                                */
/* $Log:                                                                   */
/***************************************************************************/

#include <string.h>
#include <malloc.h>

#include "splitfrm.h"
#include "forms.h"
#include "plotbox.h"
#include "device.h"
#include "cursor.h"
#include "multi.h"
#include "curvdraw.h"
#include "omaform.h"
#include "graphops.h"
#include "barmenu.h"

#include "calib.h"     // InCalibForm, until clib.c modified to use this

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

/* data for plotmode/menumode switching */

static SHORT OldActiveWindow;    
static CRECT OldGraphArea;       
static SHORT OldLocus;
static struct save_area_info * SavedArea;
static BOOLEAN SplitFormActive = FALSE;
static int (*SwitchRoutine) (void) = 0;

/*************************************************************************/
/*                                                                       */
/* If program needs to run a form with plotted data easily available for */
/* reference, this module provides an easy way to create the split plot/ */
/* form screen, handle input focus switching, and return back to full-   */
/* screen operation when the form goes away.  The form's module needs to */
/* call InitSplitForm in its init routine, and ExitSplitForm in its exit */
/* routine.  If the form needs some special action to happen when the    */
/* user switches focus to the plot (using F10 or a by mouse click on the */
/* plot) it may pass the address of such a routine to InitSplitForm.     */
/* A routine is also provided for functions (such as GraphOps) which     */
/* need to know whether a split form is active, and if so, whether form  */
/* or plot has input focus.                                              */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* Default routine executed when input passes to graphics screen.        */
/*                                                                       */
/*************************************************************************/
int SwitchToGraphMode(void)
{
  int ReturnVal;

  erase_cursor();
  if(push_form_context())
    {
    setCurrentFormToGraphWindow();
    ReturnVal = GraphOps();        /* run plot windows */
    erase_screen_area(1, 0, 1, screen_columns,
      specialGrafPrepareForExit(), FALSE);
    redraw_menu();
    specialGrafReturnToForm();  /* does pop_form_context */
    return(ReturnVal);
    }
  return(FIELD_VALIDATE_WARNING);
}

/*************************************************************************/
/*                                                                       */
/* Use in init function of a form which needs graphics screen for refer- */
/* ence or input.  Form is the address of the form in question; InForm   */
/* is the address of a variable which the form module can test to find   */
/* the state of the input focus.  If DoOnSwitch is not NULL, it is the   */
/* address of a routine to execute when the input focus switches.  If    */
/* NULL is supplied, then SwitchToGraphMode will be used.                */
/*                                                                       */
/**************************************************************************/
BOOLEAN InitSplitForm(FORM * Form, BOOLEAN * InForm, int (*DoOnSwitch)(void))
{
  CRECT *pWin = &PlotWindows[CAL_PLOTBOX];

  InitSubformPlot(pWin, (UCHAR)(Form->row+Form->size_in_rows));

  if (!*InForm)
    {
    SavedArea= save_screen_area(Form->row, 0, (UCHAR)(screen_rows-Form->row),
      (UCHAR)(screen_columns-Form->column));

    OldGraphArea = DisplayGraphArea;
    DisplayGraphArea = *pWin;

    /* set up for a new active window, use OldActiveWindow for restoration  */
    OldActiveWindow = new_active_window(CAL_PLOTBOX);
    *InForm = SplitFormActive = TRUE;
    }

  if (DoOnSwitch)
    SwitchRoutine = DoOnSwitch;
  else
    SwitchRoutine = SwitchToGraphMode;

  erase_mouse_cursor();
  Replot(ActiveWindow); /* may want to autoscale instead */
  RemoveGraphCursor();
  replace_mouse_cursor();
  return FALSE;
}

/*************************************************************************/
/*                                                                       */
/* Use as exit routine of form which has plot screen component.  "Exit"  */
/* occurs when user switches from form to plot screen, as well as when   */
/* they escape out of the form back to full screen. In the latter case,  */
/* the InForm flag becomes FALSE and the previous screen is redrawn. If  */
/* user is just switching to graph mode within the split form, then      */
/* the InForm flag remains TRUE and the SwitchRoutine is run.  The de-   */
/* fault switch routine is SwitchToGraphMode, which runs GraphOps and    */
/* allows all of the normal cursor, scaling, etc. operations.  Another   */
/* switch routine used by calib and spgraph is get_new_points, which     */
/* responds to a mouse click on the curve by requesting a new wavelength */
/* value for that X position.                                            */
/*                                                                       */
/*************************************************************************/
BOOLEAN ExitSplitForm(BOOLEAN * InForm)
{
  CRECT FullArea, PlotArea;
  CXY AxisZero[3], AxisEndOffset[3];
  SHORT PlotIndex = WindowPlotAssignment[OldActiveWindow];

  if ((Current.Form->status != FORMSTAT_EXIT_ALL_FORMS) &&
    (Current.Form->status != FORMSTAT_EXIT_THIS_FORM) &&
    (Current.Form->status != FORMSTAT_EXIT_TO_MENU1) &&
    (Current.Form->status != FORMSTAT_EXIT_TO_MENU2))
    {
    (*SwitchRoutine)();
    draw_form();

    if ((Current.Form->status != FORMSTAT_EXIT_ALL_FORMS) &&
      (Current.Form->status != FORMSTAT_EXIT_TO_MENU1)  &&
      (Current.Form->status != FORMSTAT_EXIT_TO_MENU2))
      return TRUE;
    }

  /* user did something to exit from split form back to parent form */
  /* restore previous screen display */

  DisplayGraphArea = OldGraphArea;
  FullArea = Plots[PlotIndex].fullarea;
  PlotArea = Plots[PlotIndex].plotarea;
  AxisZero[0] = Plots[PlotIndex].x.axis_zero;
  AxisEndOffset[0] = Plots[PlotIndex].x.axis_end_offset;
  AxisZero[1] = Plots[PlotIndex].y.axis_zero;
  AxisEndOffset[1] = Plots[PlotIndex].y.axis_end_offset;
  AxisZero[2] = Plots[PlotIndex].z.axis_zero;
  AxisEndOffset[2] = Plots[PlotIndex].z.axis_end_offset;

  strcpy(Plots[CAL_PLOTBOX].x.legend, Plots[PlotIndex].x.legend);
  Plots[PlotIndex] = Plots[CAL_PLOTBOX];
  CursorStatus[PlotIndex] = CursorStatus[CAL_PLOTBOX];

  Plots[PlotIndex].fullarea = FullArea;
  Plots[PlotIndex].plotarea = PlotArea;
  Plots[PlotIndex].x.axis_zero = AxisZero[0];
  Plots[PlotIndex].x.axis_end_offset = AxisEndOffset[0];
  Plots[PlotIndex].y.axis_zero = AxisZero[1];
  Plots[PlotIndex].y.axis_end_offset = AxisEndOffset[1];
  Plots[PlotIndex].z.axis_zero = AxisZero[2];
  Plots[PlotIndex].z.axis_end_offset = AxisEndOffset[2];
  restore_active_window(OldActiveWindow);

  if (SavedArea)
    {
    restore_screen_area(SavedArea);
    SavedArea = 0;
    }
  else
    PutUpPlotBoxes();

  *InForm = SplitFormActive = FALSE;

  return FALSE;
}

/********************************************************************/
BOOLEAN InSplitForm(void)
{
  return SplitFormActive || InCalibForm;
}

