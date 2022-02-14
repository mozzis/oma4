/***************************************************************************/
/*                                                                         */
/* File name  : VESADRVR                                                   */
/* Author     : David DiPrato                                              */
/* Version    : 1.00 - Initial version.                                    */
/* Description: This module will contain all the functions required to     */
/*    control graphics using the VESA driver.                              */
/*                                                                         */
/***************************************************************************/

#ifndef VESA_INCLUDED
#define VESA_INCLUDED

#include "utypes.h"

/* Global structure definitions ********************************************/


/* Global Varible Definitions **********************************************/


/* Global Definitions ******************************************************/

#define SVGA         0x4F  /* if func is supported, returns 0x4f in AL     */
#define SVGAOK       00    /* if func successful, returns 0 in AH          */
#define SVGAFAIL     01    /* if func fails, returns 1 in AH               */

/* Function prototypes *****************************************************/


/* set the SVGA 'mode'                                                     
****************************************************************************/
void set_VESA_mode(USHORT Mode);

/* get the current SVGA mode
****************************************************************************/
USHORT get_VESA_mode(void);

/* Opens the VESA graphics system by allocating memory, verifing the
   VESA driver is present and setting the given video mode.  It will save
   the current video mode for restore when the system is closed. A non-zero
   will return on error.
****************************************************************************/
SHORT OpenVESASystem(USHORT Mode);

/* Closes the VESA graphics system by releasing any memory allocated and
   restore the initial video mode.
****************************************************************************/
void CloseVESASystem(void);

/* set the address of the video memory segment mapped to a000:0000
/* this function only guarantees write access to the segment
****************************************************************************/
void set_VESA_window(USHORT win_no);

/* get the address of the video memory segment mapped to a000:0000
****************************************************************************/
USHORT get_VESA_window(void);

/* This function will create a far pointer toward video memory.
****************************************************************************/
char far *CreateVideoPtr(void);

/* This function will return the number of bytes for each video segment
   unit. 
****************************************************************************/
LONG GetVideoSegSize(void);

/* This function will get the number of bytes per video line in the
   video memory.
****************************************************************************/
SHORT GetBytesPerLine(void);




#endif



