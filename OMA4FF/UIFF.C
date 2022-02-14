/***************************************************************************/
/*                                                                         */
/* File name  : UIFF                                                       */
/* Author     : David DiPrato                                              */
/* Version    : 1.00 - Initial version.                                    */
/* Description: This module contains all the support functions to read     */
/*    and write EG&G Instrument's Unified Image File Format files.         */
/*                                                                         */
/***************************************************************************/

#include <ctype.h>
#include <conio.h>
#include <math.h>
#include <direct.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <malloc.h>
#include "uiff.h"

/* Local Global varibles ***************************************************/
char ErrorMsg[40];               /* Used for global error reporting.       */

/* This function will read a UIFF file from disk to memory given the file's
   name, memory pointer and data point Index and size (points).  It will 
   assume the directory has been set and for real-mode versions; 
   64K bytes (32K points) is the maximum.

   Memory pointer 'Data' must have been allocated large enough for the
   data.
****************************************************************************/
int ReadUIFF16BitFile(char *FileName,int DataIndex,int Verbose,short *Data);
{
   UIF_HEADER *Header;
   FILE *FPtr;
   int i;

   /* Open file  ----------------------------------------------------------*/
   if ((FPtr = fopen(FName,"rb")) == NULL) {
      strcpy(ErrorMsg,"Could not open the source file.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      return(-2);
      }

   /* Read Header data into Control structure and test for validity -------*/
   if ((Header = (UIF_HEADER *)calloc(sizeof(UIF_HEADER),1)) == NULL) {
      strcpy(ErrorMsg,"Could not allocate memory for file's header.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      fclose(FPtr);
      return(-3);
      }

   if (fread((char*)Header,sizeof(UIF_HEADER),1,FPtr) != sizeof(TCL_HEADER)) {
      strcpy(ErrorMsg,"Could not read header data from file.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      fclose(FPtr);
      free(Header);
      return(-4);
      } 

//   if (Header->Identifier != UFIFORMAT) {
//      strcpy(ErrorMsg,"File is not in UIF format.");
//      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
//      fclose(FPtr);
//      free(Header);
//      return(-5);
//      }

   if (Header->DataType != SIGNED || Header->PixelFormat != BITS16GRAY) {
      strcpy(ErrorMsg,"File is not 16 bit format.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      fclose(FPtr);
      free(Header);
      return(-5);
      }

   /* Read remaining data into Data Memory --------------------------------*/
   fseek(FPtr,Index * 2,SEEK_SET);
   if (fread(Data,Size,2,FPtr) != DataSize * 2) {
      strcpy(ErrorMsg,"Could not read data from file.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      fclose(FPtr);
      free(Header);
      return(-6);
      }

   /* Exit function gracefully  ********************************************/
   fclose(FPtr);
   free(Header);
   return(0);
}
