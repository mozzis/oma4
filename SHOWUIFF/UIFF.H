/***************************************************************************/
/*                                                                         */
/* File name  : UIFF                                                       */
/* Author     : David DiPrato                                              */
/* Description: This module contains all the support functions to read     */
/*    and write EG&G Instrument's Unified Image File Format files.         */
/*                                                                         */
/* version 2.0 - Changed all integer operations from 32 to 16 bit.         */
/*                                                                         */
/***************************************************************************/

#ifndef uifffile
#define uifffile

/* Function Prototypes. ****************************************************/
/* File Format definitions ------------------------------------------------*/
#define BITS16       0                 /* FileFormat valid data values.    */
#define BIT8PACKED   1
#define BITS32       2

/* Pixel Format definitions -----------------------------------------------*/
#define BITS8GREY    8                 /* PixelFormat valid data values.   */
#define BITS16GRAY   16
#define BITS32GRAY   32

/* UFI Identifier definitions ---------------------------------------------*/
#define UFIFORMAT    1234              /* Used to identify a TCL32bit file */

/* Data Type definitions --------------------------------------------------*/
#define UNSIGNED     0x55555555        /* Unsigned data.                   */
#define SIGNED       0xaaaaaaaa        /* Signed data.                     */
#define FLOATING     0x5a5a5a5a        /* IEEE 32 bit floating point.      */
#define UFIFLOAT     0x5555aaaa        /* UIF 16 bit floating point.       */

typedef union {
   long  *l;
   short *s;
   float *f;
   char  *b;
   } LF_PTR;

typedef struct {                       /* TCL data header structure.       */
   short    FileFormat;                /* File format descriptor.          */
   short    XTotal;                    /* Maximum number of X acis element */
   short    YTotal;                    /* Maximum number of Y axis element */
   short    AlwaysZero;                /* Zero for disk files.             */
   short    PixelFormat;               /* Type of data; gray versus color. */
   short    Reserved[27];              /* This area is reserved by TCL.    */
   unsigned short UserSpace0[14];      /* Application specific area zero.  */
   long     UIFIdentifier;             /* Used to identify a UIF file.     */
   short    UserSpace1;                /* Application specific area one.   */
   long     DataType;                  /* Data format descriptor.          */
   short    UserSpace2;                /* Application specific area two.   */
   short    NumDataFrames;             /* Number of data images per file.  */
   short    UserSpace3[203];           /* Application specific area three. */
   LF_PTR   DataPtr;                   /* Pointer toward data.             */
   } UIF_HEADER ;
   
/* Global varibles *********************************************************/
extern char ErrorMsg[40];        /* Used for global error reporting.       */

/* function prototype(s) ***************************************************/
/*  This function will copy the input header's information to the output
  header.  The data pointer will be copied as well.  
****************************************************************************/
void CopyUIFHeader(UIF_HEADER *In,UIF_HEADER *Out);

/*  This function will read a UIFF file from disk to memory given the file's
    name.  It will assume the directory has been set.  Memory will be
    allocated and pointer toward a UIF_HEADER shall return.  A null
    pointer will return on error with the error text in global
    string 'ErrorMsg'.

    The allocated memory must be released by a 'ReleaseUIFFFile' call.
****************************************************************************/
UIF_HEADER *ReadUIFFFile(char *FileName,int Verbose);

/* This function is used to release all allocated memory from a
  'ReadUIFFFile' call.
****************************************************************************/
void ReleaseUIFFFile(UIF_HEADER *Header);

/* This function will write a UIFF file from the given UIF_HEADER to
  disk.  It will assume the directory has been set.  A non-zero will
  return on error as well as the error text in global string 'ErrorMsg'.
****************************************************************************/
int WriteUIFFFile(char *FileName,UIF_HEADER *Header,int Verbose);

/* This function tests the given data pointed to by the UIFFFile structure
  for 32 bit integer type.  If true it returns a non-zero.
****************************************************************************/
int Integer32Bit(UIF_HEADER *Header);

/* This function tests the given data pointed to by the UIFFFile structure
  for 16 bit integer type.  If true it returns a non-zero.
****************************************************************************/
int Integer16Bit(UIF_HEADER *Header);

/* This function tests the given data pointed to by the UIFFFile structure
  for 32 bit floating point type.  If true it returns a non-zero.
****************************************************************************/
int Floating32Bit(UIF_HEADER *Header);



#endif


