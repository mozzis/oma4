/***************************************************************************/
/*                                                                         */
/* File name  : SVGA                                                       */
/* Version    : 1.00 - Initial version.                                    */
/* Author     : David DiPrato                                              */
/* Description: This module contains functions that support the IRM        */
/*    Scrolling VGA board.                                                 */
/*                                                                         */
/***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <io.h>

#include "svga.h"

/* Global varibles *********************************************************/
int SVGAShadowReg = 0;              /* Board's control reigister varible.  */
                                    /* H/W reg. is write only.             */
int SVGADataPortAddr = 0;           /* Data port's address.                */
int SVGACntrlPortAddr = 0;          /* Control port's address.             */

/* Local function proto-types **********************************************/

/* Local defintions ********************************************************/
#define ENIMAGE_BIT     0x02
#define ENIMAGE_MASK    0xfd
#define SCROLL_BIT      0x04
#define SCROLL_MASK     0xfb
#define INVVSYNC_BIT    0x20
#define INVVSYNC_MASK   0xdf
#define INVHSYNC_BIT    0x40
#define INVHSYNC_MASK   0xbf 

/* Fucntions follow: *******************************************************/

/* This function will set the SVGA board's I/O address.  The address must
   be even.  If not it will round down.
****************************************************************************/
void SetSVGAPortAddr(int Address)
{
   SVGADataPortAddr = Address & 0xffe;
   SVGACntrlPortAddr = (Address & 0xffe) + 1;

   /* Initialize the I/O ports. -------------------------------------------*/
   SVGAShadowReg = 0;
   outp(SVGACntrlPortAddr,SVGAShadowReg);
}

/* This function will enable the image on the VGA display.  It will return
   a non-zero if the port address has not been set.
****************************************************************************/
int EnableVGAImage()
{
   /* Test for valid port address -----------------------------------------*/
   if (SVGACntrlPortAddr == 0) return(-1);

   /* Enable image --------------------------------------------------------*/
   SVGAShadowReg |= ENIMAGE_BIT;
   outp(SVGACntrlPortAddr,SVGAShadowReg);

   return(0);
}


/* This function will disable the image on the VGA display. It will return
   a non-zero if the port address has not been set.
****************************************************************************/
int DisableVGAImage()
{
   /* Test for valid port address -----------------------------------------*/
   if (SVGACntrlPortAddr == 0) return(-1);

   /* Disable image -------------------------------------------------------*/
   SVGAShadowReg &= ENIMAGE_MASK;
   outp(SVGACntrlPortAddr,SVGAShadowReg);

   return(0);
}

/* This function will check the status of the Image Enable.  It will return
   a non-zero if the image is enabled.
****************************************************************************/
int VGAImageEnabled()
{
   if (SVGAShadowReg & ENIMAGE_BIT) return(1);

   return(0);
}

/* This function will scroll the VGA image by one line.  This function is
   also used to reset the pixel pointer to position zero.  It will return
   a non-zero if the port address has not been set.
****************************************************************************/
int ScrollImage()
{
   /* Test for valid port address -----------------------------------------*/
   if (SVGACntrlPortAddr == 0) return(-1);

   /* Scroll image --------------------------------------------------------*/
   SVGAShadowReg |= SCROLL_BIT;
   outp(SVGACntrlPortAddr,SVGAShadowReg);
   SVGAShadowReg &= SCROLL_MASK;

   return(0);
}


/* This function will make the Vertical Sync pulse active low.  It will 
   return a non-zero if the port address has not been set.
****************************************************************************/
int SetVSyncActiveLow()
{
   /* Test for valid port address -----------------------------------------*/
   if (SVGACntrlPortAddr == 0) return(-1);

   /* Set Bit -------------------------------------------------------------*/
   SVGAShadowReg |= INVVSYNC_BIT;
   outp(SVGACntrlPortAddr,SVGAShadowReg);

   return(0);
}


/* This function will make the Vertical Sync pulse active high.  It will 
   return a non-zero if the port address has not been set.
****************************************************************************/
int SetVSyncActiveHigh()
{
   /* Test for valid port address -----------------------------------------*/
   if (SVGACntrlPortAddr == 0) return(-1);

   /* Set Bit -------------------------------------------------------------*/
   SVGAShadowReg &= INVVSYNC_MASK;
   outp(SVGACntrlPortAddr,SVGAShadowReg);

   return(0);
}


/* This function will make the Hortizontal Sync pulse active low.  It will 
   return a non-zero if the port address has not been set.
****************************************************************************/
int SetHSyncActiveLow()
{
   /* Test for valid port address -----------------------------------------*/
   if (SVGACntrlPortAddr == 0) return(-1);

   /* Set Bit -------------------------------------------------------------*/
   SVGAShadowReg |= INVHSYNC_BIT;
   outp(SVGACntrlPortAddr,SVGAShadowReg);

   return(0);
}


/* This function will make the Hortizontal Sync pulse active high.  It will
   return a non-zero if the port address has not been set.
****************************************************************************/
int SetHSyncActiveHigh()
{
   /* Test for valid port address -----------------------------------------*/
   if (SVGACntrlPortAddr == 0) return(-1);

   /* Set Bit -------------------------------------------------------------*/
   SVGAShadowReg &= INVHSYNC_BIT;
   outp(SVGACntrlPortAddr,SVGAShadowReg);

   return(0);
}

/* This function will write a byte to the SVGA board and increament the pixel 
   pointer.  Function 'ScrollImage' should be used to reset the pixel pointer
   to the begining of a line.  It will return a non-zero if the port address 
   has not been set.
****************************************************************************/
int WriteSVGAData(char Data)
{
   /* Test for valid port address -----------------------------------------*/
   if (SVGADataPortAddr == 0) return(-1);

   /* Write data ----------------------------------------------------------*/
   outp(SVGADataPortAddr,Data);

   return(0);
}
