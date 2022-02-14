/***************************************************************************/
/*                                                                         */
/* File name  : UIFF                                                       */
/* Author     : David DiPrato                                              */
/* Version    : 1.00 - Initial version.                                    */
/* Description: This module contains all the support functions to read     */
/*    and write EG&G Instrument's Unified Image File Format files.         */
/*                                                                         */
/***************************************************************************/

#ifndef uifffile
#define uifffile

/* Local  definitions ******************************************************/
#define DIR_LEN      20                /* Maximum characters per path name */
#define FILE_LEN     13                /* Maximum characters per file name */
#define LIST_LEN     40                /* Maximum matched files per list.  */
#define ERROR_LEN    80                /* Maximum error message length.    */
#define PATH_LEN     45
#define TEXT_LEN     75                /* Maximum length of text buffer.   */
#define VERBOSE      1
#define QUIET        0

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
#define FLOAT        0x5a5a5a5a        /* IEEE 32 bit floating point.      */
#define UFIFLOAT     0x5555aaaa        /* UIF 16 bit floating point.       */

typedef struct {                       /* TCL data header structure.       */
   short    FileFormat;                /* File format descriptor.          */
   short    XTotal;                    /* Maximum number of X acis element */
   short    YTotal;                    /* Maximum number of Y axis element */
   short    AlwaysZero;                /* Zero for disk files.             */
   short    PixelFormat;               /* Type of data; gray versus color. */
   short    Reserved[27];              /* This area is reserved by TCL.    */
   short    UserSpace0[14];            /* Application specific area zero.  */
   long     UIFIdentifier;             /* Used to identify a UIF file.     */
   short    UserSpace1;                /* Application specific area one.   */
   long     DataType;                  /* Data format descriptor.          */
   short    UserSpace2;                /* Application specific area two.   */
   short    NumDataFrames;             /* Number of data images per file.  */
   short    UserSpace3[203];           /* Application specific area three. */
   }UIF_HEADER ;
   

/* Function Prototypes. ****************************************************/

/* This function will read a UIFF file from disk to memory given the file's
   name, memory pointer and data point Index and size (points).  It will 
   assume the directory has been set and for real-mode versions; 
   64K bytes (32K points) is the maximum.

   Memory pointer 'Data' must have been allocated large enough for the
   data.
****************************************************************************/
int ReadUIFF16BitFile(char *FName,int Index,int Size,int Verbose,short *Data);


#endif

