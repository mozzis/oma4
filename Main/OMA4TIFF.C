/***************************************************************************/
/*                                                                         */
/* File name  : OMA4TIFF.C (Save Curvesets in Tagged Image File Format)    */
/* Author     : Morris Maynard - based on version by Dave DiPrato          */
/* Version    : 1.00 - Initial version.                                    */
/* Description: Saves OMA4000 Curvesets in Tagged Image File Format        */
/*                                                                         */
/***************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <malloc.h>

#define ESCAPE 27

#include "change.h"
#include "tempdata.h"
#include "points.h"
#include "curvedir.h"
#include "syserror.h"  // ERROR_OPEN
#include "omaerror.h"  // error()
#include "crventry.h"
#include "crvheadr.h"
#include "plotbox.h"   // ActivePlot
#include "curvbufr.h"  // ActivePlot
#include "handy.h"

#include "oma4tiff.h"
#include "hidris.h"

/* Local  definitions ******************************************************/

/*   This TIFF file will scale the data values according to the first      */
/* plot window in which it is displayed.  All of the points in the         */
/* selected curves will be written into the output file.                   */
/*                                                                         */
/*   Data will be ordered: 1st - Header                                    */
/*                         2nd - File Directory                            */
/*                         3rd - Strip Count table                         */
/*                         4th - X Resolution value                        */
/*                         5th - Y Resolution value                        */
/*                         6th - Strip Offset table                        */
/*                         7th - Strip data arrays (actual data)           */

#define NUM_TAGS  12       /* Total number of Tags in directory.  */

typedef struct {           /* Tagged Directory Entry.             */
                           /* ## TAGS MUST BE IN ASCENDING ORDER! */
   SHORT Tag;              /* Descriptor of the entry.            */
   SHORT Type;             /* Data type of the data.              */
   LONG Count;             /* Number of items of dat 'Type'.      */
   LONG ValueOffset;       /* Actual data (value <= 4 bytes) or   */
                           /* a pointer (on word boundary).       */
   } DE;

typedef struct {           /* Image File Directory structure      */
                           /* ## MUST BE ON A WORD BOUNDARY!!!  ##*/
   SHORT Count;            /* Number of entries in the directory. */
   DE ImageWidth;          /* Image File TAGs.                    */
   DE ImageLength;
   DE BitsPerSample;
   DE Compression;
   DE Interpretation;
   DE StripOffset;
   DE SamplesPerPixel;
   DE RowsPerStrip;
   DE StripByteCount;
   DE XResolution;
   DE YResolution;
   DE ResolutionUnit;
   LONG Offset;            /* Always 0 if no more directories.   */
   } IFD;

typedef struct {           /* File Header structure              */
   SHORT Format;           /* Byte order (always 4949 hex, 'II') */
   SHORT Version;          /* Identifier (always 42, 2A hex)     */
   LONG IFDPtr;            /* Image File Directory pointer(bytes)*/
   } IFH;


typedef struct {                /* Internal structure for TIFF writes  */
   LONG FileDirectoryPosition;  /* File Directory file position.       */
   LONG StripByteCntPosition;   /* Byte Count table file position      */
   LONG StripOffsetPosition;    /* Strip Offset table file position    */
   LONG XResolutionPosition;    /* Pixels/Resolution file position     */
   LONG YResolutionPosition;    /* Pixels/Resolution file position     */
   } TIFF;

/* Definition of DE data types *********************************************/
#define BYTE_TYPE       1       /* An 8-bit unsigned integer.          */
#define ASCII_TYPE      2       /* 8-bit ASCII codes w/NULL terminator */
#define SHORT_TYPE      3       /* A 16-bit (2-byte) unsigned integer. */
#define LONG_TYPE       4       /* A 32-bit (4-byte) unsigned integer. */
#define RATIONAL_TYPE   5       /* Two LONGs:1st represents numerator, */
                                /*  2nd the denominator.               */

/* Definition of required Tags *********************************************/

/* TIFF-X (all TIFF files) Tag values **************************************/
#define IMAGE_WIDTH_VALUE       256 /* Image's X direction data pt count.  */
#define IMAGE_LENGTH_VALUE      257 /* Image's Y direction data pt count.  */
#define ROWS_PER_STRIP_VALUE    278 /* Number of X rows/strip (always 1).  */
#define STRIP_OFFSETS_VALUE     273 /* Actual data offsets.                */
#define STRIP_BYTE_COUNTS_VALUE 279 /* Number of bytes/strip (X data pts). */
#define XRESOLUTION_VALUE       282 /* Number of display pixels per point. */
#define YRESOLUTION_VALUE       283 
#define RESOLUTION_UNIT_VALUE   296 /* Unit per pixel (1 = No unit).       */

/* TIFF-G (grayscale files) Tag values *************************************/
#define SAMPLES_PER_PIXEL_VALUE 277 /* Always 1 for gray scale.            */
#define BITS_PER_SAMPLE_VALUE   258 /* Number of bits (always 8).          */
#define COMPRESSION_VALUE       259 /* None for now (always 1).            */
#define INTERPRETATION_VALUE    262 /* 1 = 0 is black (always 1).          */

/* TIFF-X (all TIFF files) Tag index number ********************************/
#define IMAGE_WIDTH_INDEX       0  
#define IMAGE_LENGTH_INDEX      1  
#define ROWS_PER_STRIP_INDEX    7  
#define STRIP_OFFSETS_INDEX     5   
#define STRIP_BYTE_COUNTS_INDEX 8  
#define XRESOLUTION_INDEX       9  
#define YRESOLUTION_INDEX       10 
#define RESOLUTION_UNIT_INDEX   11

/* TIFF-G (grayscale files) Tag index number *******************************/
#define SAMPLES_PER_PIXEL_INDEX 6  
#define BITS_PER_SAMPLE_INDEX   2  
#define COMPRESSION_INDEX       3  
#define INTERPRETATION_INDEX    4  


/* Write the File Directory and TAGs to disk.  LEaves the Strip Byte Count
   and Strip Offsets pointers but writes the File Directory pointer
****************************************************************************/
ERR_OMA WriteFileDirectory(FILE *fptr, TIFF *Tiff,
                                  SHORT serial, SHORT image)
{
  IFD *Dir;
  LONG FilePosition;           /* Used to hold the current file pos.  */
  ERR_OMA err = ERROR_NONE;

  /* Setup file's Directory ***********************************************/    
  if ((Dir = (IFD *)malloc(sizeof(IFD))) == NULL)
    return ERROR_ALLOC_MEM;

  /* Get the current position in the output file **************************/
  FilePosition = ftell(fptr);   

  /* Setup file's directory ***********************************************/
  Dir->Count = NUM_TAGS;
  Dir->Offset = 0;              /* No more directories.                */

  Dir->ImageWidth.Tag           = IMAGE_WIDTH_VALUE;
  Dir->ImageWidth.Type          = SHORT_TYPE;
  Dir->ImageWidth.Count         = 1;
  Dir->ImageWidth.ValueOffset   = serial;

  Dir->ImageLength.Tag          = IMAGE_LENGTH_VALUE;
  Dir->ImageLength.Type         = SHORT_TYPE;     
  Dir->ImageLength.Count        = 1;
  Dir->ImageLength.ValueOffset  = image;

  Dir->RowsPerStrip.Tag         = ROWS_PER_STRIP_VALUE;
  Dir->RowsPerStrip.Type        = SHORT_TYPE;
  Dir->RowsPerStrip.Count       = 1;
  Dir->RowsPerStrip.ValueOffset = 1; 

  Dir->StripOffset.Tag          = STRIP_OFFSETS_VALUE;
  Dir->StripOffset.Type         = LONG_TYPE;
  Dir->StripOffset.Count        = (LONG)image;
  Dir->StripOffset.ValueOffset  = 0;
  Tiff->StripOffsetPosition     =
    FilePosition + offsetof(IFD, StripOffset.ValueOffset);

  Dir->StripByteCount.Tag       = STRIP_BYTE_COUNTS_VALUE;
  Dir->StripByteCount.Type      = SHORT_TYPE;
  Dir->StripByteCount.Count     = image;
  Dir->StripByteCount.ValueOffset = 0;
  Tiff->StripByteCntPosition = 
     FilePosition + offsetof(IFD, StripByteCount.ValueOffset);

  Dir->XResolution.Tag          = XRESOLUTION_VALUE;
  Dir->XResolution.Type         = RATIONAL_TYPE;
  Dir->XResolution.Count        = 1;
  Dir->XResolution.ValueOffset  = 0;
  Tiff->XResolutionPosition     = 
     FilePosition + offsetof(IFD, XResolution.ValueOffset);

  Dir->YResolution.Tag          = YRESOLUTION_VALUE;
  Dir->YResolution.Type         = RATIONAL_TYPE;
  Dir->YResolution.Count        = 1;
  Dir->YResolution.ValueOffset  = 0;
  Tiff->YResolutionPosition     = 
     FilePosition + offsetof(IFD, YResolution.ValueOffset);

  Dir->ResolutionUnit.Tag       = RESOLUTION_UNIT_VALUE;
  Dir->ResolutionUnit.Type      = SHORT_TYPE;
  Dir->ResolutionUnit.Count     = 1;
  Dir->ResolutionUnit.ValueOffset = 2;   /* Use inches */

  Dir->SamplesPerPixel.Tag      = SAMPLES_PER_PIXEL_VALUE;
  Dir->SamplesPerPixel.Type     = SHORT_TYPE;
  Dir->SamplesPerPixel.Count    = 1;
  Dir->SamplesPerPixel.ValueOffset = 1;

  Dir->BitsPerSample.Tag        = BITS_PER_SAMPLE_VALUE;
  Dir->BitsPerSample.Type       = SHORT_TYPE;
  Dir->BitsPerSample.Count      = 1;
  Dir->BitsPerSample.ValueOffset = 8;

  Dir->Compression.Tag          = COMPRESSION_VALUE;
  Dir->Compression.Type         = SHORT_TYPE;
  Dir->Compression.Count        = 1;
  Dir->Compression.ValueOffset = 1;

  Dir->Interpretation.Tag       = INTERPRETATION_VALUE;
  Dir->Interpretation.Type      = SHORT_TYPE;
  Dir->Interpretation.Count     = 1;
  Dir->Interpretation.ValueOffset = 1;

  /* Write file's Directory to disk ***************************************/
  if (fwrite(Dir, sizeof(IFD), 1, fptr) != 1)
    err = ERROR_WRITE;

  /* Write File Directory position into the File's Header *****************/
  else if (fseek(fptr, Tiff->FileDirectoryPosition, SEEK_SET))
    err = ERROR_WRITE;

  else if (fwrite(&FilePosition, sizeof(LONG), 1, fptr) != 1)
    err = ERROR_WRITE;

  /* Reposition File pointer at the end of file ***************************/
  else if (fseek(fptr,0,SEEK_END))
    err = ERROR_WRITE;

  free(Dir);
  return err;
}

/* Write the Strip Byte Count table and the Strip Byte Count pointer.
****************************************************************************/
ERR_OMA WriteStripCnt(FILE *fptr, TIFF *Tiff,
                             SHORT serial, SHORT image)
{
  LONG i, FilePosition;           /* Used to hold the current file pos.  */
  SHORT *Table;
  ERR_OMA err = ERROR_NONE;

  /* Setup file's Directory ***********************************************/    
  if ((Table = (SHORT *)malloc(image * sizeof(SHORT))) == NULL)
    return ERROR_ALLOC_MEM;

  /* Fill Count Table *****************************************************/
  for (i = 0; i < image; i++)
    Table[i] = serial;

  /* Get the current position in the output file **************************/
  FilePosition = ftell(fptr);   

  /* Write Byte Count table to disk ***************************************/
  if (fwrite(Table, sizeof(SHORT), image, fptr) != image)
    err = ERROR_WRITE;

  /* Write Strip Byte Count position into the File's Directory ************/
  else if (fseek(fptr, Tiff->StripByteCntPosition, SEEK_SET))
    err = ERROR_SEEK;

  else if (fwrite(&FilePosition, sizeof(LONG), 1, fptr) != 1)
    err = ERROR_WRITE;

  /* Reposition File pointer at the end of file ***************************/
  else if (fseek(fptr, 0, SEEK_END))

  free(Table);
  return err;
}

/* Write the X and Y Resolution values and both resolution pointers
****************************************************************************/
ERR_OMA WriteResolutionValues(FILE *fptr, TIFF *Tiff,
                                     SHORT serial, SHORT image)
{
  LONG FilePosition,           /* Used to hold the current file pos.  */
       TempValue[4];           /* Used to hold the resolution values. */
  ERR_OMA err = ERROR_NONE;

  /* Get the current position in the output file **************************/
  FilePosition = ftell(fptr);   

  /* Write X Resolution Value to disk *************************************/
  TempValue[0] = (LONG)(serial / 7);
  TempValue[1] = 0x00000001;
  TempValue[2] = (LONG)(image / 7);
  TempValue[3] = 0x00000001;

  if (fwrite(TempValue, sizeof(LONG), 4, fptr) != 4)
    err = ERROR_WRITE;

  /* Write Strip Byte Count position into the File's Directory ************/
  else if (fseek(fptr, Tiff->XResolutionPosition, SEEK_SET))
    err = ERROR_SEEK;

  else if (fwrite(&FilePosition, sizeof(LONG), 1, fptr) != 1)
    err = ERROR_WRITE;

  /* Reposition File pointer at the end of file ***************************/
  else if (fseek(fptr, 0L, SEEK_END))
    err = ERROR_SEEK;

  if (err)
    return err;

  /* Get the current position in the output file **************************/
  FilePosition = ftell(fptr);   

  /* Write Y Resolution Value to disk *************************************/
  if (fwrite(TempValue, sizeof(LONG), 4, fptr) != 4)
    err = ERROR_WRITE;

  /* Write Strip Byte Count position into the File's Directory ************/
  else if (fseek(fptr, Tiff->YResolutionPosition, SEEK_SET))
    err = ERROR_SEEK;

  else if (fwrite(&FilePosition, sizeof(LONG), 1, fptr) != 1)
    err = ERROR_WRITE;

  /* Reposition File pointer at the end of file ***************************/
  else if (fseek(fptr,0,SEEK_END))
    err = ERROR_SEEK;

  return err;
}

/* Write the Strip Offset Table and Strip Offset Pointer 
****************************************************************************/
ERR_OMA WriteStripOffset(FILE *fptr, TIFF *Tiff,
                                SHORT serial, SHORT image)
{
  SHORT i;
  LONG AddressOffset, *StripOffset, FilePosition;
  ERR_OMA err = ERROR_NONE;

  /* Setup file's Header **************************************************/    
  if ((StripOffset = malloc(image * sizeof(LONG))) == NULL)
    return ERROR_ALLOC_MEM;

  /* Get the current position in the output file **************************/
  FilePosition = ftell(fptr);   
  AddressOffset = 0;

  for (i = 0; i < image; i++)
    {
    StripOffset[i] = FilePosition + (image * sizeof(LONG)) + AddressOffset;
    AddressOffset += (LONG)serial;
    }

  /* Write Strip Offsets to disk ******************************************/
  if (fwrite(StripOffset, sizeof(LONG), image, fptr) != image)
    err = ERROR_WRITE;

  /* Write Strip Ofsset position into the File's Directory ****************/
  else if (fseek(fptr, Tiff->StripOffsetPosition, SEEK_SET))
    err = ERROR_SEEK;

  else if (fwrite(&FilePosition, sizeof(LONG), 1, fptr) != 1)
    err = ERROR_WRITE;

  /* Reposition File pointer at the end of file ***************************/
  else if (fseek(fptr, 0L, SEEK_END))
    err = ERROR_SEEK;

  free(StripOffset);
  return err;
}

/* Calculate the data scaling factors                                    */
/* Looks through the plot windows, and uses the scaling of the first     */
/* window in which the data is selected for plotting                     */
/* If the data appears in no window, window 0 is used.                   */
/* Only the intensity scaling from the window is used.                   */
/*************************************************************************/
static void CalcScales(USHORT Blk, FLOAT *YMin, FLOAT * YMax, FLOAT *StepSize)
{
  PLOTBOX * Plot;
  SHORT i, pIndex = 0;

  for (i = 0; i < MAXPLOTS; i++)
    {
    if (MainCurveDir.Entries[Blk].DisplayWindow & (1<<i))
      {
      pIndex = i;
      break;
      }
    }
  Plot = &Plots[pIndex];

  if(Plot->style != FALSE_COLOR)
    {
    *YMin = Plot->y.original_min_value;
    *YMax = Plot->y.original_max_value;
    }
  else
    {
    *YMin = Plot->z.original_min_value;
    *YMax = Plot->z.original_max_value;
    }

  if (*YMax < *YMin)
    {
    FLOAT Temp = *YMin;
    *YMin = *YMax;
    *YMax = Temp;
    }
  *StepSize = (*YMax - *YMin) / 255.0;
}

/* Write a TIFF-G (256 Grayscale) file to disk
****************************************************************************/
ERR_OMA trans_TIFF256(FILE* fptr, const char * fName,
                             CURVEDIR *CvDir,
                             SHORT Blk, SHORT StCurve, SHORT Curves,
                             BOOLEAN KbChk)
{
  TIFF *Tiff;
  IFH *Header;
  SHORT i, j, Points, BufNum = 0;
  FLOAT YMin, YMax, X, Y, StepSize;
  BOOLEAN Done = FALSE;
  CHAR *CBuff;
  CURVEHDR CvHdr;
  ERR_OMA err = ERROR_NONE;
  
  /* Allocate memory for the internal structure ***************************/
  if ((Tiff = (TIFF *)malloc(sizeof(TIFF))) == NULL)
    return error(ERROR_ALLOC_MEM);

  /* Setup file's Header **************************************************/    
  if ((Header = (IFH *)malloc(sizeof(IFH))) == NULL)
    {
    free(Tiff);
    return(error(ERROR_ALLOC_MEM));
    }

  err = ReadTempCurvehdr(CvDir, Blk, StCurve, &CvHdr);
  if (err)
    return err;

  Points = CvHdr.pointnum;

  CalcScales(Blk, &YMin, &YMax, &StepSize);

  /* Setup file's Header **************************************************/
  Header->Format = 0x4949;         /* Intel byte order.                   */
  Header->Version = 0x2a;          /* All TIFF files use this value.      */
  Header->IFDPtr = 0;              /* Leave pointer blank for now.        */

  /* Write file's Header to disk ******************************************/
  if (fseek(fptr,0,SEEK_SET))
    {
    free(Header);
    free(Tiff);
    return(error(ERROR_SEEK, fName));
    }

  if (fwrite(Header, sizeof(IFH), 1, fptr) != 1)
    {
    free(Header);
    free(Tiff);
    return(error(ERROR_SEEK, fName));
    }

  /* Update the internal structure ****************************************/
  Tiff->FileDirectoryPosition = offsetof(IFH, IFDPtr);
  
  free(Header);

  err = WriteFileDirectory(fptr, Tiff, Points, Curves);
  if (err)
    return error(err, fName);

  err = WriteStripCnt(fptr, Tiff, Points, Curves);
  if (err)
    return error(err, fName);

  err = WriteResolutionValues(fptr, Tiff, Points, Curves);
  if (err)
    return error(err, fName);

  err = WriteStripOffset(fptr, Tiff, Points, Curves);
  if (err)
    return error(err, fName);

  CBuff = malloc(Points);
  if (!CBuff)
    return error(ERROR_ALLOC_MEM);

  for (i = StCurve + Curves-1; i >= StCurve && !Done; i--)
    {
    if (KbChk && kbhit() && (getch() == ESCAPE))
      break;

    for (j = 0; j < Points; j++)
      {
      err = GetDataPoint(CvDir, Blk, i, j, &X, &Y, FLOATTYPE, &BufNum);
      if (err)
        {
        Done = TRUE;
        break;
        }
      if (Y > YMax)
        Y = YMax;
      if (Y < YMin)
        Y = YMin;
      CBuff[j] = (UCHAR)((Y - YMin) / StepSize); /* use X as temp storage */
      }

    if (!err)
      {
      /* Write Strip data to disk */
      if (fwrite(CBuff, Points, 1, fptr) != 1)
        {
        err = ERROR_WRITE;
        break;
        }
      }
    }
  free(CBuff);
  return err;
}
