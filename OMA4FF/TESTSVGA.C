/***************************************************************************/
/*                                                                         */
/* File name  : TESTSVGA                                                   */
/* Version    : 1.00 - Initial version.                                    */
/* Author     : David DiPrato                                              */
/* Description: This module will test the Scrolling VGA board using the    */
/*    driver 'svga'.                                                       */
/*                                                                         */
/***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <io.h>

#include "svga.h"
#include "vesadrvr.h"
#include "uiff.h"

/***************************************************************************/
int main(int argc,char *argv[])
{
   int IOAddress;
   int x,y;
   char FillValue = 0;
   unsigned short OldVideoMode;
   unsigned short VideoMode;

   /* Get current video mode for restore later and set new (800x600x256) --*/
   OldVideoMode = get_VESA_mode();
   VideoMode = (unsigned short)strtoul(argv[2],NULL,16);
   set_VESA_mode(VideoMode);

   /* Get I/O port address from the command line and set I/O port ---------*/
   IOAddress = (int)strtoul(argv[1],NULL,16);
   SetSVGAPortAddr(IOAddress);

   /* Fill up image memory and enable image -------------------------------*/
   EnableVGAImage();
   while (!kbhit()) {
      for (x = 0;x < 200;x++) {     
         WriteSVGAData(FillValue);
         }
      ScrollImage();
      FillValue++;
      delay(10);
      }
   getch();

   /* Turn off image, restore old video mode and exit ---------------------*/
   DisableVGAImage();
   set_VESA_mode(OldVideoMode);

   return(0);
}
   
