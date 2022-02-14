/***************************************************************************/
/***************************************************************************/
/*                                                                         */
/* File name  : IMAGE                                                      */
/* Author     : David DiPrato                                              */
/* Version    : 1.00 - Initial version.                                    */
/* Description: This module will contain all the functions required to     */
/*    control graphics using the Borland library.                          */
/*                                                                         */
/***************************************************************************/

#ifndef IMAGE_INCLUDED
#define IMAGE_INCLUDED


/* Global structure definitions ********************************************/

/* Global Variable Definitions **********************************************/

/* Global Definitions ******************************************************/

/* Function prototypes *****************************************************/

/* Open the display system; save video mode, set 640x480 mode.  Will return
   a non-zero on error.
****************************************************************************/
SHORT OpenImageDisplay(void);

/* Close the display system; restore video mode.
****************************************************************************/
SHORT CloseImageDisplay(void);

/* Setup an image window at the given locations.  Assumimg video mode
   640x480 and values are physical pixels.  A non-zero will return on error.
****************************************************************************/
SHORT OpenImageWindow(SHORT XLeft,SHORT XRight,SHORT YTop,SHORT YBottom);

/* Shut-down the image window.  A non-zero will return on error.
****************************************************************************/
SHORT CloseImageWindow(void);

/* Update the image window.  A non-zero will return on error.
****************************************************************************/
SHORT UpdateImageWindow(void);

/* Get detector specific information.  a non-zero will return if the
   image display system can not display this detector or it can not be
   identified.
****************************************************************************/
SHORT GetDetectorInfo(void);

/* These functions adjust the size of the image window.  They return a 
   non zero when the limits have been reached.
****************************************************************************/
SHORT IncWindowX(void);
SHORT DecWindowX(void);
SHORT IncWindowY(void);
SHORT DecWindowY(void);

/* This function will auto-scale the display.
****************************************************************************/
SHORT AutoScaleImage(void);

/* set cursor to bottom line of display */
/***************************************************************************/
void SetCursorToPrompt(void);

/* set cursor to given line of display */
/***************************************************************************/
void SetCursorToLine(USHORT line);
#endif



