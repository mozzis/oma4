/***************************************************************************/
/*                                                                         */
/* File name  : SVGA                                                       */
/* Version    : 1.00 - Initial version.                                    */
/* Author     : David DiPrato                                              */
/* Description: This module contains functions that support the IRM        */
/*    Scrolling VGA board.                                                 */
/*                                                                         */
/***************************************************************************/

#ifndef svgafile
#define svgafile

/* Global definitions ******************************************************/

/* Global varibles  ********************************************************/

/* Function Prototypes. ****************************************************/

/* This function will set the SVGA board's I/O address.  The address must
   be even.  If not it will round down.
****************************************************************************/
void SetSVGAPortAddr(int Address);

/* This function will enable the image on the VGA display.  It will return
   a non-zero if the port address has not been set.
****************************************************************************/
int EnableVGAImage(void);

/* This function will disable the image on the VGA display. It will return
   a non-zero if the port address has not been set.
****************************************************************************/
int DisableVGAImage(void);

/* This function will check the status of the Image Enable.  It will return
   a non-zero if the image is enabled.
****************************************************************************/
int VGAImageEnabled(void);

/* This function will scroll the VGA image by one line.  This function is
   also used to reset the pixel pointer to position zero.  It will return
   a non-zero if the port address has not been set.
****************************************************************************/
int ScrollImage(void);

/* This function will make the Vertical Sync pulse active low.  It will 
   return a non-zero if the port address has not been set.
****************************************************************************/
int SetVSyncActiveLow(void);

/* This function will make the Vertical Sync pulse active high.  It will 
   return a non-zero if the port address has not been set.
****************************************************************************/
int SetVSyncActiveHigh(void);

/* This function will make the Hortizontal Sync pulse active low.  It will 
   return a non-zero if the port address has not been set.
****************************************************************************/
int SetHSyncActiveLow(void);

/* This function will make the Hortizontal Sync pulse active high.  It will
   return a non-zero if the port address has not been set.
****************************************************************************/
int SetHSyncActiveHigh(void);

/* This function will write a byte to the SVGA board and increament the pixel 
   pointer.  Function 'ScrollImage' should be used to reset the pixel pointer
   to the begining of a line.  It will return a non-zero if the port address 
   has not been set.
****************************************************************************/
int WriteSVGAData(char Data);

#endif
