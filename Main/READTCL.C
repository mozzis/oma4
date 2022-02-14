/* -----------------------------------------------------------------------
/
/  change.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/change.c_v   0.21   06 Jul 1992 10:25:12   maynard  $
/  $Log:   J:/logfiles/oma4000/main/change.c_v  $
 
/
*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <conio.h>        

#include "readtcl.h"

/* definitions for Hidris conversion                                       */
#define BITS16       0                 /* FileFormat valid data values.    */
#define BIT8PACKED   1
#define BITS32       2
#define BITS8GREY    8                 /* PixelFormat valid data values.   */
#define BITS16GRAY   16
#define BITS32GRAY   32
#define TCLIDENTIFIER 1234             /* Used to identify a TCL32bit file */
#define TCL_INTEGER         0x55555555 /* Data types. */
#define TCL_SIGNED          0xaaaaaaaa
#define TCL_FLOAT           0x5a5a5a5a
#define TEXT_LEN      75           /* a number not evenly divisible by 3! */


typedef struct 
 {                                    /* TCL data header structure.       */
  USHORT FileFormat;      /* File format descriptor.          */
  USHORT NumSerialPixels; /* Number of pixels per frame.      */
  USHORT NumImagePixels;  /* Number of pixels per frame.      */
  USHORT AlwaysZero;      /* Zero for disk files.             */
  USHORT PixelFormat;     /* Pixel format descriptor.         */
  SHORT  Reserved[27];    /* This area is reserved by TCL.    */
/* User specific data -----------------------------------------------------*/
  USHORT NumFrames;       /*  Number of frames per memory.    */
  SHORT  Temperature;     /*  Desired Cooler temerature.      */
  DOUBLE ExposureTime;    /*  Exposure time in seconds.       */
  ULONG  XLeft;           /* Phyical locations values.        */
  ULONG  XRight;
  ULONG  YTop;
  ULONG  YBottom;
  LONG   Identifier;      /* Used to identify a TCL file.     */
  USHORT PixelSize;       /*  Physical size of each pixel.    */
  LONG   DataType;        /* Data type see IMAGE386.INC       */
  USHORT PixelSpeed;      /* Convesion time in uS.            */
  SHORT  NumMemories;     /* Number of Mems per file.         */
  SHORT  UnusedNumericalData[96 - 21]; /* Unused user numerical data.  */
/*------------------------------------------------------------------------*/
  CHAR UserText1[TEXT_LEN / 3];       /* HiDRIS text.         */
  CHAR UserText2[TEXT_LEN / 3];       /* HiDRIS text.         */
  CHAR UserText3[TEXT_LEN / 3];       /* HiDRIS text.         */
  CHAR UserText[152 - TEXT_LEN];      /* User text.           */
  USHORT Date[15];        /* Creatation date of file.         */
  CHAR DescriptionText[74];           /* Description text.    */
  } TCL_HEADER ;

TCL_HEADER TCL_Header;

char ReadTCLFName[CH_FILESIZE] = {""};

ERR_OMA error(ERR_OMA err, ... )
{
  va_list insert_args;
  char *msg;

  va_start(insert_args, err);
  
  msg = va_arg(insert_args, CHAR *);

  msg = (CHAR *) insert_args;

  fprintf(stderr,"error %d: %s\n", err, msg);

  va_end(va_list);
  return err;
}

/****************************************************************************/
int ReadTCLFile(char * fName, SHORT Modify)
{
  FILE * fptr;
  ULONG number, offset;

  fptr = fopen(fName, "r+b");

  if (fptr == NULL)
    {
    error(1, fName);
    return -1;
    }

  if (fread(&TCL_Header, sizeof(TCL_HEADER), 1, fptr) != 1)
    return error(-2, fName);

  printf("File Format: %x\n", TCL_Header.FileFormat);
  printf("NumSerialPixels: %x\n", TCL_Header.NumSerialPixels);

  printf("NumImagePixels: %x\n",  TCL_Header.NumImagePixels); /* USHORT */
  printf("AlwaysZero:     %x\n",  TCL_Header.AlwaysZero);     /* USHORT */
  printf("PixelFormat:    %x\n",  TCL_Header.PixelFormat);    /* USHORT */
  printf("Reserved:       %x\n",  TCL_Header.Reserved);       /* SHORT  */


  printf("NumFrames:      %x\n",  TCL_Header.NumFrames);      /* USHORT */
  printf("Temperature:    %x\n",  TCL_Header.Temperature);    /* SHORT  */
  printf("ExposureTime:   %lx\n", TCL_Header.ExposureTime);   /* DOUBLE */
  printf("XLeft:          %lx\n", TCL_Header.XLeft);          /* ULONG  */
  printf("XRight:         %lx\n", TCL_Header.XRight);         /* ULONG  */
  printf("YTop:           %lx\n", TCL_Header.YTop);           /* ULONG  */
  printf("YBottom:        %lx\n", TCL_Header.YBottom);        /* ULONG  */
  printf("Identifier:     %lx\n", TCL_Header.Identifier);     /* LONG   */
  printf("PixelSize:      %x\n",  TCL_Header.PixelSize);      /* USHORT */
  printf("DataType:       %lx\n", TCL_Header.DataType);       /* LONG   */
  printf("PixelSpeed:     %x\n",  TCL_Header.PixelSpeed);     /* USHORT */
  printf("NumMemories:    %x\n",  TCL_Header.NumMemories);    /* SHORT  */

  printf("UText1: %s\n", TCL_Header.UserText1);
  printf("UText2: %s\n", TCL_Header.UserText2);
  printf("UText2: %s\n", TCL_Header.UserText3);

  printf("Descrip: %s\n", TCL_Header.DescriptionText);

  printf("Date: %2.2d/%2.2d/%4.4d\n", TCL_Header.Date[0],
                                      TCL_Header.Date[1],
                                      TCL_Header.Date[2]);
  if (Modify)
    {
    number = 512L;
    offset = offsetof(TCL_HEADER, XRight);
    fseek(fptr, offset, SEEK_SET);
    fwrite(&number, sizeof(LONG), 1, fptr);

    offset = offsetof(TCL_HEADER, YBottom);
    fseek(fptr, offset, SEEK_SET);
    fwrite(&number, sizeof(LONG), 1, fptr);

    number = 0;
    offset = offsetof(TCL_HEADER, YTop);
    fseek(fptr, offset, SEEK_SET);
    fwrite(&number, sizeof(LONG), 1, fptr);
    }

  fclose(fptr);
  return 0;
}

int _stklen = 0x2000;

int main(int argc, char ** argv)
{
  char fName[80];
  SHORT Modify = 0;

  if (argc > 1)
    {
    strncpy(fName, argv[1], 79);
    if (argc > 2)
      Modify = atoi(argv[2]);

    ReadTCLFile(fName, Modify);
    return 0;
    }
  return 1;
}
