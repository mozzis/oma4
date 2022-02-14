
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <io.h>

#include "vesadrvr.h"

/***************************************************************************/
int main(int argc,char *argv[])
{
   unsigned short ModeNumber;

   ModeNumber = (unsigned short)strtoul(argv[1],NULL,16);
   set_VESA_mode(ModeNumber);

   return(0);
}
