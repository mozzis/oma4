/***************************************************************************/
/*                                                                         */
/* File name  : IMAGE                                                      */
/* Author     : David DiPrato                                              */
/* Version    : 1.00 - Initial version.                                    */
/* Description: This module will contain all the functions required to     */
/*    control graphics using the VESA driver.                              */
/*                                                                         */
/***************************************************************************/

#ifndef VESA_IMAGE_INCLUDED
#define VESA_IMAGE_INCLUDED

#include "uiff.h"

/* Global structure definitions ********************************************/

/* Global Varible Definitions **********************************************/

/* Global Definitions ******************************************************/

/* Function prototypes *****************************************************/

/* Open the display system; save video mode, set 640x480 mode.  Will return
   a non-zero on error.
****************************************************************************/
int OpenImageDisplay(void);

/* Close the display system; restore video mode.
****************************************************************************/
int CloseImageDisplay(void);

/* Setup an image window at the given locations.  Assumimg video mode
   640x480 and values are physical pixels.  A non-zero will return on error.
****************************************************************************/
int OpenImageWindow(int XLeft,int XRight,int YTop,int YBottom);

/* Shut-down the image window.  A non-zero will return on error.
****************************************************************************/
int CloseImageWindow(void);

/* Update the image window.  A non-zero will return on error.
****************************************************************************/
int UpdateImageWindow(UIF_HEADER *In);

/* These functions adjust the size of the image window.  They return a 
   non zero when the limits have been reached.
****************************************************************************/
int IncWindowX(void);
int DecWindowX(void);
int IncWindowY(void);
int DecWindowY(void);

/* This function will auto-scale the display.
****************************************************************************/
int AutoScaleImage(UIF_HEADER *In);

/* set cursor to bottom line of display */
/***************************************************************************/
void SetCursorToPrompt(void);

/*  save the current Palette.
****************************************************************************/
void SavePalette(void);

/*  restore the Palete saved with function 'SavePalette'.
****************************************************************************/
void RestorePalette(void);

/* sets the video palette to 64 level of gray scale.  Values 64 to 128 
   are used.  It will return a non-zero on error.
****************************************************************************/
int SetPalette(void);

#endif


