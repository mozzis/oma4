/***************************************************************************/
/*  crventry.h    uselessly split off from curvedir module by you know who */
/*                                                                         */
/*  copyright (c) 1989, EG&G Instruments Inc.                              */
/*                                                                         */
/***************************************************************************/
  
#ifndef CRVENTRY_INCLUDED
#define CRVENTRY_INCLUDED

#include <time.h> /* type struct tm */

#include "handy.h"  // DOSFILESIZE, DESCRIPTION_LENGTH

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

  /* structure used by the central data area to describe and locate curve */
  /* blocks.  Also used by the graph sessions to show displayed curves */
  /* change from PCHAR desc ; to CHAR descrip[]. */
struct curve_entry
{
  CHAR name[DOSFILESIZE+1]; /* filename without path info, null terminated. */
                            /* All capitals */
  CHAR path[DOSPATHSIZE+1]; /* path, null terminated, all capitals, '\'ended. */
  CHAR descrip[ DESCRIPTION_LENGTH ] ;         /* description of curve block */
  USHORT StartIndex;        /* Index of 1461 memory, OMA4 detector, or  */
                            /* curve file where this block starts */
  ULONG StartOffset ;       /* used to be StartOffset.Offset RAC 29 Dec 91 */
  ULONG TmpOffset;          /* Byte offset of curve Block into temp file */
  USHORT count;             /* the number of curves in the block */
  struct tm time;           /* file time and date info */
  CHAR EntryType;           /* File or curve type, e.g. OMA4 data, method-only, */
                            /* DAD, OMA3 data, DIF, JCAMP, SUBDIR, other */
  CHAR Dirty;               /* reserved. Will be set when block is changed in */
                            /* future revision. */
  USHORT DisplayWindow;     /* flag bits 0..7 - If curve block is  */
                            /* displayed, it is being displayed in the  */
                            /* flagged graph windows. */
} ;

/* typedef struct curve_entry CURVE_ENTRY;  */
/* typedef CURVE_ENTRY far * LPCURVE_ENTRY; */

#define CURVEENTRYSZ sizeof (CURVE_ENTRY)

/* -1 is used as the index for a curve set which does not exist */
#define NOT_FOUND (-1)

/* EntryType Definitions */
#define OTHER        0
#define OMA4DATA     1
#define OMA4METHOD   2
#define OMA4DAD      3
#define OMA3DATA     4      /* 1460 ASCII */
#define OMA3METHOD   5      /* 1460 ASCII */
#define OMA35MEMDATA 6      /* data contained in normal DOS memory */
#define OMA3561DATA  7      /* data contained in the 1461 */
#define OMA35FDATA   8      /* data contained in a file */
#define OMA4MEMDATA  9      /* data contained in OMA4 detector board */
#define OMA35METHOD 10
#define DIF         11
#define JCAMP       12
#define SUBDIR      13
#define OMA4V11DATA     14
#define OMA4V11METHOD   15

int DirEntryNameCompare       (const CURVE_ENTRY *, const CURVE_ENTRY *);
int InverseDirEntryNameCompare(const CURVE_ENTRY *, const CURVE_ENTRY *);
int DirEntryTimeCompare       (const CURVE_ENTRY *, const CURVE_ENTRY *);
int InverseDirEntryTimeCompare(const CURVE_ENTRY *, const CURVE_ENTRY *);

#endif   // CRVENTRY_INCLUDED
