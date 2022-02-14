#include <dos.h>
#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "uiff.h"
#include "vesaimag.h"

/* Local defintions ********************************************************/

/* Local function prototypes ***********************************************/

/* Functions follow: *******************************************************/

int main(int argc, char **argv) 
{
  UIF_HEADER *InputImage = NULL;

  if (argc < 2)
    {
    fprintf(stderr, "\nUseage: %s <filename>"
                    "\nwhere <filename> names a UIF format file\n",argv[0]);
    return 1;
    }

  /* Get given image -----------------------------------------------------*/
  if ((InputImage = ReadUIFFFile(argv[1],0)) == NULL)
   {
   printf("ERROR: Unable to read input file %s.\n",argv[1]);
   return(-3);
   }

  /* Open graphic system -------------------------------------------------*/
  if (OpenImageDisplay())
    {
    printf("ERROR: Install a VESA video driver.\n");
    ReleaseUIFFFile(InputImage);
    return(-1);
    }

  if (OpenImageWindow(100,500,50,400))
    {
    CloseImageWindow();
    CloseImageDisplay();
    RestorePalette();
    printf("ERROR: Unable to open graphics display.\n");
    ReleaseUIFFFile(InputImage);
    return(-2);
    }

  /* Display data --------------------------------------------------------*/
  if (AutoScaleImage(InputImage))
    {
    CloseImageWindow();
    CloseImageDisplay();
    RestorePalette();
    printf("ERROR: Unable to autoscale data.\n");
    ReleaseUIFFFile(InputImage);
    return(-4);
    }

  if (UpdateImageWindow(InputImage))
    {
    CloseImageWindow();
    CloseImageDisplay();
    RestorePalette();
    printf("ERROR: Unable to display data.\n");
    ReleaseUIFFFile(InputImage);
    return(-4);
    }

  SetCursorToLine(1);
  printf("                          Press any key to End\n");
  flushall();
  getch();
  /* Exit gracefully -----------------------------------------------------*/
  CloseImageWindow();
  CloseImageDisplay();
  RestorePalette();
  ReleaseUIFFFile(InputImage);
  return(0);
}
