/***************************************************************************/
/*                                                                         */
/* File name  : UIFF                                                       */
/* Author     : David DiPrato                                              */
/* Version    : 1.00 - Initial version.                                    */
/* Description: This module contains all the support functions to read     */
/*    and write EG&G Instrument's Unified Image File Format files.         */
/*                                                                         */
/* version 2.0 - Changed all integer operations from 32 to 16 bit.         */
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

/* Local  definitions ******************************************************/
#define DIR_LEN      20                /* Maximum characters per path name */
#define FILE_LEN     13                /* Maximum characters per file name */
#define LIST_LEN     40                /* Maximum matched files per list.  */
#define ERROR_LEN    80                /* Maximum error message length.    */
#define PATH_LEN     45
#define TEXT_LEN     75                /* Maximum length of text buffer.   */
#define VERBOSE      1
#define QUIET        0

/* Global defintions *******************************************************/
char ErrorMsg[40];                     /* Used for global error reporting. */


/* Functions follow ********************************************************/
/*  This function will copy the input header's information to the output
  header.  The data pointer will be copied as well.  
****************************************************************************/
void CopyUIFHeader(UIF_HEADER *In,UIF_HEADER *Out)
{
   int i;

   Out->FileFormat = In->FileFormat;    
   Out->XTotal = In->XTotal;        
   Out->YTotal = In->YTotal;        
   Out->AlwaysZero = In->AlwaysZero;    
   Out->PixelFormat = In->PixelFormat;   
   Out->UIFIdentifier = In->UIFIdentifier; 
   Out->DataType = In->DataType;      
   Out->NumDataFrames = In->NumDataFrames; 

   /* The following is needed for file import to the HiDRIS program -------*/
   Out->UserSpace1 = In->UserSpace1;
   Out->UserSpace2 = In->UserSpace2;
   for (i = 0;i < 15;i++) Out->UserSpace0[i] = In->UserSpace0[i];
}

/*  This function will read a UIFF file from disk to memory given the file's
    name.  It will assume the directory has been set.  Memory will be
    allocated and pointer toward a UIF_HEADER shall return.  A null
    pointer will return on error with the error text in global
    string 'ErrorMsg'.

    The allocated memory must be released by a 'free' call.
****************************************************************************/
UIF_HEADER *ReadUIFFFile(char *FileName,int Verbose)
{
   UIF_HEADER *Header;
   FILE *FPtr;
   int i, NumBytes;

   /* Open file  ----------------------------------------------------------*/
   if ((FPtr = fopen(FileName,"rb")) == NULL) {
      strcpy(ErrorMsg,"Could not open the source file.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      return(NULL);
      }

   /* Read Header data into Control structure and test for validity -------*/
   if ((Header = (UIF_HEADER *)calloc(sizeof(UIF_HEADER),1)) == NULL) {
      strcpy(ErrorMsg,"Could not allocate memory for file's header.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      fclose(FPtr); return(NULL);
      }

   if (fread(Header, sizeof(UIF_HEADER) - sizeof(void *), 1, FPtr) != 1) {
      strcpy(ErrorMsg,"Could not read header data from file.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      fclose(FPtr); free(Header); return(NULL);
      } 

   if (Header->UIFIdentifier != UFIFORMAT) {
      strcpy(ErrorMsg,"File is not in UIF format.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      fclose(FPtr); free(Header); return(NULL);
      }

   /* Read remaining data into Data Memory --------------------------------*/
   switch (Header->PixelFormat) {
    case BITS8GREY:
      NumBytes = 1 * Header->XTotal * Header->YTotal * Header->NumDataFrames;
      break;
    case BITS16GRAY:
      NumBytes = 2 * Header->XTotal * Header->YTotal * Header->NumDataFrames;
      break;
    case BITS32GRAY:
      NumBytes = 4 * Header->XTotal * Header->YTotal * Header->NumDataFrames;
      break;
    }

  if ((Header->DataPtr.b = calloc(NumBytes,1)) == NULL) {
      strcpy(ErrorMsg,"Could not allocate memory for data.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      fclose(FPtr); free(Header);return(NULL);
      }

  if (fread(Header->DataPtr.b,1,NumBytes,FPtr) != NumBytes) {
      strcpy(ErrorMsg,"Could not read data from file.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      fclose(FPtr); free(Header->DataPtr.b); free(Header); return(NULL);
      }

   /* Exit function gracefully  ********************************************/
   fclose(FPtr);
   return(Header);
}

/* This function will write a UIFF file from the given UIF_HEADER to
  disk.  It will assume the directory has been set.  A non-zero will
  return on error as well as the error text in global string 'ErrorMsg'.
****************************************************************************/
int WriteUIFFFile(char *FileName,UIF_HEADER *Header,int Verbose)
{
   FILE *FPtr;
   int i, NumBytes;

   /* Open file  ----------------------------------------------------------*/
   if ((FPtr = fopen(FileName,"wb")) == NULL) {
      strcpy(ErrorMsg,"Could not open the destination file.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      return(-1);
      }

   /* Write data to file --------------------------------------------------*/
   switch (Header->PixelFormat) {
    case BITS8GREY:
      NumBytes = 1 * Header->XTotal * Header->YTotal * Header->NumDataFrames;
      break;
    case BITS16GRAY:
      NumBytes = 2 * Header->XTotal * Header->YTotal * Header->NumDataFrames;
      break;
    case BITS32GRAY:
      NumBytes = 4 * Header->XTotal * Header->YTotal * Header->NumDataFrames;
      break;
    }
//  if (fwrite(Header,1,sizeof(UIF_HEADER),FPtr) != sizeof(UIF_HEADER)) {
  if (fwrite(Header,1,512,FPtr) != 512) {
      strcpy(ErrorMsg,"Could not write data to file.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      fclose(FPtr); return(-3);
    }
  if (fwrite(Header->DataPtr.b,1,NumBytes,FPtr) != NumBytes) {
      strcpy(ErrorMsg,"Could not write data to file.");
      if (Verbose) printf("ERROR: %s\n",ErrorMsg);
      fclose(FPtr); return(-3);
      }

  /* Exit function gracefully  ********************************************/
  fclose(FPtr);
  return(0);
}

/* This function is used to release all allocated memory from a
  'ReadUIFFFile' call.
****************************************************************************/
void ReleaseUIFFFile(UIF_HEADER *Header)
{
  free(Header->DataPtr.b);
  free(Header);
}

/* This function tests the given data pointed to by the UIFFFile structure
  for 32 bit integer type.  If true it returns a non-zero.
****************************************************************************/
int Integer32Bit(UIF_HEADER *Header)
{
  if (Header->PixelFormat != BITS32GRAY ||
    Header->FileFormat != BITS32 ||
    Header->DataType != SIGNED) return(0);
  return(1);
}

/* This function tests the given data pointed to by the UIFFFile structure
  for 16 bit integer type.  If true it returns a non-zero.
****************************************************************************/
int Integer16Bit(UIF_HEADER *Header)
{
  if (Header->PixelFormat != BITS16GRAY ||
    Header->FileFormat != BITS16 ||
    Header->DataType != SIGNED) return(0);
  return(1);
}

/* This function tests the given data pointed to by the UIFFFile structure
  for 32 bit floating point type.  If true it returns a non-zero.
****************************************************************************/
int Floating32Bit(UIF_HEADER *Header)
{
  if (Header->PixelFormat != BITS32GRAY ||
    Header->FileFormat != BITS32 ||
    Header->DataType != FLOATING) return(0);
  return(1);
}




