/* -----------------------------------------------------------------------
/
/  device.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/device.c_v   1.1   06 Jul 1992 10:28:54   maynard  $
/  $Log:   J:/logfiles/oma4000/main/device.c_v  $
 * 
 *    Rev 1.1   06 Jul 1992 10:28:54   maynard
 * allow drivers to send prompts to screen - change def of PRINTER info array
 * 
 *    Rev 1.0   07 Jan 1992 11:54:58   cole
 * Initial revision.
* 
*/

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

#include <string.h>
#include <stdio.h>     // NULL
#include <stddef.h>    // NULL
#include <malloc.h>    // NULL

#include "device.h"
#include "doplot.h"    // draw_plotbox()
#include "multi.h"     // multiplot_setup()
#include "cursor.h"    // ActiveWindow
#include "baslnsub.h"  // baslnsub_active()
#include "plotbox.h"   // PLOTBOX
#include "formwind.h"  // choice_window()
#include "forms.h"     // COLORS_MESSAGE 
#include "calib.h"     // InCalibForm
#include "pltsetup.h"  // window_style
#include "syserror.h"  // ERROR_DEV_OPEN
#include "spgraph.h"   // InM1235Form
#include "omaform.h"   // COLORS_MESSAGE
#include "omaerror.h"  // error()
#include "tagcurve.h"  // ExpandedOnTagged
#include "curvedir.h"  // MainCurveDir
#include "curvdraw.h"  // plot_curves()
// use the following only for GRAPHICAL scan setup
//#include "scanset.h"   // inScanSetup

CDVHANDLE screen_handle;

CDVCAPABILITY screen;

PRIVATE CDVHANDLE printer_handle;           

PRIVATE CDVCAPABILITY printer;              

PRIVATE CDVHANDLE plotter_handle;  

PRIVATE CDVCAPABILITY plotter;         

PRIVATE CDVOPEN printer_setup = {           
   // Change Prompt flag from 1 to 0 so no prompts will be sent to the screen.
   CPreserveAspect, CLN_Solid, 1,
// CMK_Dot,1,1,1,1,1,1,0, "PRINTER"
   CMK_Dot,1,1,1,1,1,1,1, "PRINTER"
};

PRIVATE CDVOPEN plotter_setup = {
   // Change Prompt flag from 1 to 0 so no prompts will be sent to the screen.
   // Plotter driver puts up an insert paper message before each plot.
   CPreserveAspect, CLN_Solid, 1,
   CMK_Dot,1,1,1,1,1,1,0, "PLOTTER"
};

PRIVATE CDVCAPABILITY const * plotDevParams = & screen;

PRIVATE CDVHANDLE activeDeviceHandle;

PRIVATE char * ReadyChoices[] = {"READY", "CANCEL", NULL };

// return the handle of the currently active device
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CDVHANDLE deviceHandle(void)
{
  return activeDeviceHandle;
}

// return the number of simultaneous colors of the currently active device
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT deviceColors(void)
{
  return plotDevParams->Colors;
}

// set the clipping rectangle for the screen to full screen
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setClipRectToFullScreen(void)
{
  CRECT screenArea = { { 0, 0 }, { 0, 0 } };

  screenArea.ur = screen.LastVDCXY;

  CSetClipRectangle(screen_handle, screenArea);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void PlotPrinter(USHORT Dummy)        
{
   CRECT ScreenArea;          
   SHORT i;
   BOOLEAN Terminated = FALSE;  
   WINDOW * MessageWindow = NULL;

   static char * PrinterReadyPrompt[] =
    {
    "Check that the printer and paper are ready.",
    "Choose 'READY' to begin printing.",  
    "ESCAPE will cancel printing in progress.",
    NULL
    };

   /* put up paper ready warning */    
   if (choice_window(PrinterReadyPrompt, ReadyChoices, 0, COLORS_MESSAGE))
     return;

   put_up_message_window(BusyWorkingEsc, COLORS_MESSAGE, &MessageWindow);

   if (COpenWorkstation(&printer_setup, &printer_handle, &printer))
    {
    release_message_window(MessageWindow);
    error(ERROR_DEV_OPEN, "PRINTER");
    return;
    }

  init_colors(printer_handle, printer.Colors);

  plotDevParams = &printer;
  activeDeviceHandle = printer_handle;

  ScreenArea.ll.x = 0;        /* position lower left of plot box */
  ScreenArea.ll.y = 0;
  /* set upper right of plot box */
  /* (this sizes the plot area) */

  ScreenArea.ur = printer.LastVDCXY;

  if ((! ExpandedOnTagged) && (fullscreen_count[window_style] != 1))
    {
    multiplot_setup(PlotWindows, &ScreenArea, window_style);
    for (i=0; (i<fullscreen_count[window_style]) && !Terminated; i++)
      {
      ResizePlotForWindow(i);
      Terminated = plot_curves(&MainCurveDir,
                               &Plots[WindowPlotAssignment[i]], i);
      }
    multiplot_setup(PlotWindows, &DisplayGraphArea, window_style);
    }
  else
    {
    // Window style 0 plots one graph at full screen
    multiplot_setup(&PlotWindows[ActiveWindow], &ScreenArea, 0);
    ResizePlotForWindow(ActiveWindow);
    Terminated = plot_curves(&MainCurveDir, ActivePlot, ActiveWindow);

    multiplot_setup(&PlotWindows[ActiveWindow], &DisplayGraphArea, 0);
    }

  if(MessageWindow)
    release_message_window(MessageWindow);
  put_up_message_window(BusyWorkingBreak, COLORS_MESSAGE, &MessageWindow);

  CCloseWorkstation(printer_handle);
  /* reset the area */
  plotDevParams = &screen;
  activeDeviceHandle = screen_handle;
  init_colors(screen_handle, screen.Colors);
  if(MessageWindow)
    release_message_window(MessageWindow);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void PlotScreen(USHORT Dummy)          
{
  SHORT i;
  BOOLEAN Terminated = FALSE;

  if((! ExpandedOnTagged)  && (fullscreen_count[window_style] != 1)
       && (! baslnsub_active()) && (! InCalibForm) && (! InM1235Form)
// use the following only for GRAPHICAL scan setup
//       && (! (* inScanSetup))
    )
    {
    multiplot_setup(PlotWindows, &DisplayGraphArea, window_style);
    for (i=0; (i<fullscreen_count[window_style]) && !Terminated; i++)
      {
      ResizePlotForWindow(i);

      create_plotbox(& Plots[WindowPlotAssignment[i]]);

      Terminated = plot_curves(&MainCurveDir,
        &(Plots[WindowPlotAssignment[i]]), i);

        SetCursorPos(i, CursorStatus[ WindowPlotAssignment[ i ] ].X,
                        CursorStatus[ WindowPlotAssignment[ i ] ].Y,
                        CursorStatus[ WindowPlotAssignment[ i ] ].Z);
      }
    }
  else
    {
    // Window style 0 plots one graph at full screen
    multiplot_setup(&PlotWindows[ActiveWindow], &DisplayGraphArea, 0);

    ResizePlotForWindow(ActiveWindow);

    create_plotbox(& Plots[ ActiveWindow ]);

    Terminated = plot_curves(&MainCurveDir, ActivePlot, ActiveWindow);

    SetCursorPos(ActiveWindow, CursorStatus[ ActiveWindow ].X,
      CursorStatus[ ActiveWindow ].Y,
      CursorStatus[ ActiveWindow ].Z);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void PlotPlotter(USHORT Dummy)        
{
   CRECT ScreenArea;    
   SHORT i;
   BOOLEAN Terminated = FALSE;
   WINDOW * MessageWindow = NULL;

   static char * PlotterReadyPrompt[] = {
      "Check that the plotter and paper are ready.",
      "Choose 'READY' to begin plotting.",  
      "ESCAPE will cancel plotting in progress.",
      NULL
   };
      
   /* put up paper ready warning */    
   if (choice_window(PlotterReadyPrompt, ReadyChoices, 0, COLORS_MESSAGE))
      return;

   put_up_message_window(BusyWorkingEsc, COLORS_MESSAGE, &MessageWindow);

   ScreenArea.ll.x = 0;        /* position lower left of plot box */
   ScreenArea.ll.y = 0;          

   if (COpenWorkstation(&plotter_setup, &plotter_handle, &plotter))
   {
      release_message_window(MessageWindow);
      error(ERROR_DEV_OPEN, "PLOTTER"); 
      return;
   }

   init_colors(plotter_handle, plotter.Colors);

   plotDevParams = &plotter;
   activeDeviceHandle = plotter_handle;
   ScreenArea.ur = plotter.LastVDCXY;     
   
   if ((! ExpandedOnTagged) && (fullscreen_count[window_style] != 1))
   {
      /* reset the area */
      multiplot_setup(PlotWindows, &ScreenArea, window_style); 

      for (i=0; (i<fullscreen_count[window_style]) && !Terminated; i++)
      {
         ResizePlotForWindow(i); 
         Terminated = plot_curves(&MainCurveDir,
                                   &(Plots[WindowPlotAssignment[i]]), i);                 
      }
      multiplot_setup(PlotWindows, &DisplayGraphArea, window_style);
   }
   else  
   {
      // Window style 0 plots one graph at full screen
      multiplot_setup(&PlotWindows[ActiveWindow], &ScreenArea, 0);
      Terminated = plot_curves(&MainCurveDir, ActivePlot, ActiveWindow);
      
      multiplot_setup(&PlotWindows[ActiveWindow], &DisplayGraphArea, 0);
   }
   if(MessageWindow)
     release_message_window(MessageWindow);
   put_up_message_window(BusyWorkingBreak, COLORS_MESSAGE, &MessageWindow);
   CCloseWorkstation(plotter_handle);     
   plotDevParams = &screen;
   activeDeviceHandle = screen_handle;
   init_colors(screen_handle, screen.Colors);
   if(MessageWindow)
     release_message_window(MessageWindow);
}

// Return the scale factor for converting virtual to physical x coordinates
// for the current device.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float XVirToPhysScaleFactor(void)
{
   return (float) ((double) plotDevParams->LastVDCXY.x
                           / plotDevParams->LastXY.x);
}

// Return the scale factor for converting virtual to physical y coordinates
// for the current device.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float YVirToPhysScaleFactor(void)
{
   return (float) ((double) plotDevParams->LastVDCXY.y
                           / plotDevParams->LastXY.y);
}

// given a y value in VDC space and a dcOffset in DC space, return the VDC
// value of y moved by dcOffset pixels on the device.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CY adjustYbyDCOffset(CY yVal, CDCY dcOffset)
{
   double sizeDevY = (double) plotDevParams->LastXY.y;
   double sizeVDCY = (double) plotDevParams->LastVDCXY.y;

   return yVal + (CY) (dcOffset * sizeVDCY / sizeDevY + 0.5);
}

// given an x value in VDC space and a dcOffset in DC space, return the VDC
// value of x moved by dcOffset pixels on the device.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CX adjustXbyDCOffset(CX xVal, CDCX dcOffset)
{
   double sizeDevX = (double) plotDevParams->LastXY.x;
   double sizeVDCX = (double) plotDevParams->LastVDCXY.x;

   return xVal + (CX) (dcOffset * sizeVDCX / sizeDevX + 0.5);
}

// move a point in VDC space by x and y offsets in DC space.  Return the
// point in VDC space.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CXY movePointByDCOffset(CXY point, CDCX xDCOffset, CDCY yDCOffset)
{
   if(xDCOffset) point.x = adjustXbyDCOffset(point.x, xDCOffset);

   if(yDCOffset) point.y = adjustYbyDCOffset(point.y, yDCOffset);

   return point;
}

void InitSubformPlot(CRECT * Area, unsigned char row)
{
  Area->ll.x = 0;         
  Area->ll.y = row_to_y(screen_rows - 2);
  Area->ur.x = screen.LastVDCXY.x;
  Area->ur.y = adjustYbyDCOffset(row_to_y(row), -1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA plotWindowToDevice(SHORT windowNum, char * deviceName)
{
   PLOTBOX * pPlot = & Plots[ windowNum ];
   CRECT screenArea;

   if(strcmp(strupr(deviceName), "PRINTER") == 0)
   {
      if(COpenWorkstation(&printer_setup, &printer_handle, &printer))
      {
         return error(ERROR_DEV_OPEN, deviceName);
      }
  
      activeDeviceHandle = printer_handle;
      plotDevParams = & printer;
  
      screenArea.ll.x = 0;     /* position lower left of plot box */
      screenArea.ll.y = 0;
      screenArea.ur = printer.LastVDCXY;

      multiplot_setup(PlotWindows, & screenArea, window_style);
   }
   else if(strcmp(strupr(deviceName), "PLOTTER") == 0)
   {
      if(COpenWorkstation(&plotter_setup, &plotter_handle, &plotter))
      {
         return error(ERROR_DEV_OPEN, deviceName);
      }
  
      init_colors(plotter_handle, plotter.Colors);
      activeDeviceHandle = plotter_handle;
      plotDevParams = & plotter;

      screenArea.ll.x = 0;     /* position lower left of plot box */
      screenArea.ll.y = 0;
      screenArea.ur = plotter.LastVDCXY;

      multiplot_setup(PlotWindows, & screenArea, window_style);
   }
   else if(strcmp(strupr(deviceName), "SCREEN") == 0)
   {
      activeDeviceHandle = screen_handle;
      plotDevParams = & screen;
   }
  
   // redraw plot box
   draw_plotbox(pPlot);
   plot_curves(& MainCurveDir, pPlot, windowNum);
  
   if(activeDeviceHandle != screen_handle)
      CCloseWorkstation(activeDeviceHandle);

   activeDeviceHandle = screen_handle;
   plotDevParams = & screen;
   init_colors(screen_handle, screen.Colors);
  
   /* reset the area */
   multiplot_setup(PlotWindows, & DisplayGraphArea, window_style);

   return ERROR_NONE;
}

#define CGI_NOT_PRESENT -3003
#define CGI_NOT_TRANSIENT   -2978
#define DRIVERS_ALREADY_LOADED  -2977
#define MEMORY_TOO_SMALL_FOR_GSSCGI -3034
#define INSUFFICIENT_MEMORY -1
#define ALLOCATE_FAR_MEMORY(x) (char far *) halloc(x, sizeof (char))
#define FREE_FAR_MEMORY(x) hfree (x)

char far   *where;            /* Where to load drivers */
CCONFIGURATION config;        /* GSS*CGI Configuration structure */
int transient;                /* Transient-drivers-loaded flag */

/************************************************************************
 *
 * Load Drivers
 *
 ************************************************************************/

int load_drivers (void)
{
  auto int  err;      /* Error variable */
  auto long bytes_needed;   /* Memory required for driver load */

  err = 0;        /* Assume no error */

  if (((CLoadCgi ((char far *)0, 0L, &bytes_needed))))
    {
    switch ((err = CInqCGIError()))
      {
      case CGI_NOT_PRESENT:
        printf ("\n%s%s%s\n",
          "GSS*CGI is not presently in memory.  Please load it by running the GSS*CGI\n",
          "Device Driver Management Utility, DRIVERS.EXE.  Then try running this\n",
          "program again.\n");
      break;

      case DRIVERS_ALREADY_LOADED:
      case CGI_NOT_TRANSIENT:
          err = 0;    /* These conditions are okay */
          transient = CFalse;
      break;

      default:
        printf ("\n%s%d.\n%s\n",
          "The following unrecognized error was returned from Load CGI: ", err,
          "Please refer to the GSS*CGI Programmer's Guide for the appropriate action.\n");
      break;
      }
    }
  else if ((where = malloc((USHORT)bytes_needed)) != (char far *) 0)
    {
    CLoadCgi (where, bytes_needed, &bytes_needed);
    transient = CTrue;
    }
  else
    {
    printf ("\n%s%s%s%s\n",
      "The program was unable to allocate sufficient memory to load GSS*CGI\n",
      "device drivers.  Please load them manually, using the GSS*CGI Device\n",
      "Driver Management Utility, DRIVERS.EXE.  Or remove resident programs\n",
      "in order to make additional memory available.\n");
    err = INSUFFICIENT_MEMORY;
    }
    return (err);
}


/***********************************************************************
 *
 * Find out what, if any, GSS*CGI configuration is in memory and proceed
 * to load it in, if it's not present.
 *
 ************************************************************************/

int load_configuration ()
{
  auto int    error;

  config.CGIPath = NULL;
  config.Where = NULL;
  config.Available = 0L;
  config.Required = 0L;
  error = 0;

  if (CCgiConfiguration (CStatusCGI, &config) == 0)
    {
    switch (config.Status)
      {
      case CNotLoaded:
        if (CCgiConfiguration(CLoadCGI, &config) &&
            (CInqCGIError() == MEMORY_TOO_SMALL_FOR_GSSCGI) &&
            ((config.Where = ALLOCATE_FAR_MEMORY(config.Required)) !=
             (char far *) 0))
          {
          config.Available = config.Required;
          if (CCgiConfiguration(CLoadCGI, &config))
            {
            error = CInqCGIError ();
            }
          }
        else
          error = CInqCGIError ();
      break;

      case CLoadedStatic:
      case CTransientLoaded:
      case CLoadedTSR:
      case CLoadedApp:
      break;                  /* No action necessary */

      case CTransient:
        error = load_drivers();
      break;
        }
    }
    return (error);
}


/************************************************************************
 *
 * Remove the GSS*CGI Configuration
 *
 ************************************************************************/

/**********************************************************************
 *
 * If the program loaded GSS*CGI device drivers with Load CGI, then
 * remove the drivers and release the memory used to contain them.
 *
 **********************************************************************/
void remove_drivers(void)
{
  if (transient == CTrue)
    {
    CKillCgi();
    FREE_FAR_MEMORY(where);
    }
}

/***********************************************************************
 *
 * If the GSS*CGI configuration was loaded by the application, then
 * remove it.
 *
 ************************************************************************/

int remove_configuration (void)
{
  int error;

  config.CGIPath = NULL;
  /* Save config.Where for call to FREE_FAR_MEMORY below */
  config.Available = 0L;
  config.Required = 0L;
  error = 0;

  if (CCgiConfiguration(CStatusCGI, &config) == 0)
    {
    switch (config.Status)
      {
      case CLoadedApp:
        if (CCgiConfiguration(CRemoveCGI, &config))
          {
          error = CInqCGIError();
          }
        FREE_FAR_MEMORY(config.Where); /* Release the configuration memory */
      break;

      case CTransientLoaded:
        remove_drivers();
      break;

      case CNotLoaded:
      case CLoadedStatic:
      case CTransient:
      case CLoadedTSR:
      break;                  /* No action necessary */
      }
    }
  return (error);
}

// open the screen and clear it
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int openAndClearScreen(CCOLOR markerColor)
{
  int returnVal;
  CDVOPEN screen_setup =
            {         
            CPreserveAspect,              // coordinate transform mode 1
            CLN_Solid,                    // initial line type
            1,                            // initial line color
            CMK_Plus,                     // marker type = Star
            0,                            // marker color, for graph cursor
            1,                            // initial graphics text font
            1,                            // initial graphics text color
            1,                            // initial fill interior style
            1,                            // initial fill style index
            1,                            // initial fill color index
            1,                            // prompting flag
            "DISPLAY"                     // driver link name
            };

  screen_setup.MarkerColor = markerColor;

  if (load_configuration() < 0)
    return (-1);

  returnVal = COpenWorkstation(&screen_setup, &screen_handle, &screen);

  if(returnVal != -1)
    CClearWorkstation(screen_handle);

  activeDeviceHandle = screen_handle;
  plotDevParams = & screen;
  
  return returnVal;
}
