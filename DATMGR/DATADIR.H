/***************************************************************************/
/*  datadir.h                                                              */
/*  copyright (c) 1994, EG&G Instruments Inc.                              */
/*  define structure used by DLL to track data files and live data         */
/*                                                                         */
/**************************************************************************/
  
#ifndef DATADIR_INCLUDED
#define DATADIR_INCLUDED

#include <dos.h>   // System length constants
#include <stdlib.h> // _MAX_PATH
#include <stdio.h> // FILE

#include "primtype.h"
#include "unimeth.h"

enum { DMGR_DESC_LEN = 128 };

typedef struct fflags {
  unsigned int dirty   : 1;  // Data has been modified but not re-written to source
  unsigned int inmem   : 1;  // Data comes from memory, not disk
  unsigned int foreign : 1;  // Data comes from non-DOS file
  unsigned int remote  : 1;  // Data comes from network file
} FFLAGS;

enum src_types {        
            STYPE_INVALID = -1,
  /*  0 */  STYPE_OTHER = 0, 
  /*  1 */  STYPE_OMA4DATA,         // OMA4 File or live data
  /*  2 */  STYPE_OMA4METHOD,       // OMA4 method file
  /*  3 */  STYPE_OMA4DAD,          // OMA4 DAD file
  /*  4 */  STYPE_OMA3DATA,         // 1460 ASCII
  /*  5 */  STYPE_OMA3METHOD,       // 1460 ASCII without data?
  /*  6 */  STYPE_OMA35METHOD,      // OMA2000 method
  /*  7 */  STYPE_DIF,        
  /*  8 */  STYPE_JCAMP,      
  /*  9 */  STYPE_OMA4V11DATA,      // Older OMA4 data format
  /* 10 */  STYPE_OMA4V11METHOD,    // Same without data
  /* 11 */  STYPE_OMA4TCL32,        // Hidris format
  /* 12 */  STYPE_OMATIFF8,         // 8 Bit TIFF format
  /* 13 */  STYPE_OMAM1470A,        // 1st 'Vison format
  /* 14 */  STYPE_OMAM1470B,        // 2nd 'Visionformat
  /* 15 */  STYPE_OMAM1470C,        // 3rd 'Visionformat
}; 
  
// for now, one data block struct for each frame/memory
// all contiguous, Y data follows header in memory
// if any X data, it is in a separate memory block pointed to by pXData;
// more than one block can share same pXData
typedef struct data_block {
  short PCount;           // the number of points in a curve
  short CCount;           // the number of curves in a memory
  float Time;             // Time in seconds from start of scan
  float SComp;
  short Frame;
  short Track;
  BOOL MemData;            // TRUE if data is in (OMA4) memory
  enum axis_units YUnits;  // Y axis units for this curve
  enum dat_type DataType;  // data type (int, float, etc.)
  DWORD szData;            // size of Y Data in bytes
  LPVOID pXData;           // Pointer to X data (always float) or NULL
} DATA_BLOCK;

#define SZDBLOCK sizeof(DATA_BLOCK)

// Y data follows as an array of the designated data type
// If X data is needed, it follows the Y data as an array of float
// Otherwise, displays will create X axis based on method calib struct

// structure used by the central data area to describe and locate curve
// blocks.  Also used by the graph sessions to show displayed curves
typedef struct dir_entry {
  CHAR Name[_MAX_PATH+1];      // filespec incl path info, null terminated.
  CHAR Desc[DMGR_DESC_LEN+1];  // file descrip - pre Win 4.0
  FILE * hFile;                // File handle from DOS
  SHORT RefCount;              // Number of "opens"
  short MCount;                // the number of frames or memories in a file
  HGLOBAL hData;
  void __huge * pData;         // pointer to data in memory
  enum src_types SourceType;   // Data type of file
  FFLAGS Flags;                // Runtime file info
  UNI_METHOD Method;
} DIR_ENTRY;

typedef struct dir_entry FAR * LPENTRY;
#define SZENTRY sizeof(DIR_ENTRY)
/* -1 is used as the index for a curve set which does not exist */
#define NOT_FOUND (-1)

typedef struct data_directory
{
  LPENTRY pEntries;  /* pointer to the start of the directory entries */
  short ECount;      /* number of directory entries, 16K MAX */
} DATADIR;

typedef struct data_directory FAR * LPDATADIR;

#endif // DATADIR_INCLUDED
