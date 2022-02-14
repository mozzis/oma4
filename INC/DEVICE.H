/* -----------------------------------------------------------------------
/
/  device.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/DEVICE.H 1     6/18/99 1:52a Maynard $
/  $Log: /OMA4000/Include/DEVICE.H $
 * 
 * 1     6/18/99 1:52a Maynard
 * 
 *    Rev 1.0   07 Jan 1992 11:50:40   cole
 * Initial revision.
/
*/

#ifndef DEVICE_INCLUDED
#define DEVICE_INCLUDED

#ifndef CGIBIND_INCLUDED
   #define CGIBIND_INCLUDED
   #include <cgibind.h>
#endif

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern CDVHANDLE screen_handle;

extern CDVCAPABILITY screen;

// return the handle of the currently active device
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CDVHANDLE deviceHandle(void);

// return the number of simultaneous colors of the currently active device
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT deviceColors(void);

// set the clipping rectangle for the screen to the whole screen
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setClipRectToFullScreen(void);

void PlotPrinter(USHORT Dummy);
void PlotScreen(USHORT Dummy);
void PlotPlotter(USHORT Dummy);

// Return the scale factor for converting virtual to physical x coordinates
// for the current device.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float XVirToPhysScaleFactor(void);

// Return the scale factor for converting virtual to physical y coordinates
// for the current device.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float YVirToPhysScaleFactor(void);

// given a y value in VDC space and a dcOffset in DC space, return the VDC
// value of y moved by dcOffset pixels on the device.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CY adjustYbyDCOffset(CY yVal, CDCY dcOffset);

// given an x value in VDC space and a dcOffset in DC space, return the VDC
// value of x moved by dcOffset pixels on the device.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CX adjustXbyDCOffset(CX xVal, CDCX dcOffset);

// move a point in VDC space by x and y offsets in DC space.  Return the
// point in VDC space.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CXY movePointByDCOffset(CXY point, CDCX xDCOffset, CDCY yDCOffset);

// Set the limits of one rectangle (a plot) to be below the screen 
// rows of another rectangle (a form)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void InitSubformPlot(CRECT * Area, unsigned char row);

// plot the window to the device named in deviceName.
// Device names are printer, plotter, and screen (NOT case sensitive)
// Does nothing if deviceName doesn't match one of the above
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA plotWindowToDevice(SHORT windowNum, char * deviceName);

// GSS Open Workstation for the screen and then clear it.  Returns the
// result of the GSSOpenWorkstation call.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int openAndClearScreen(CCOLOR markerColor);

#endif   // DEVICE_INCLUDED
