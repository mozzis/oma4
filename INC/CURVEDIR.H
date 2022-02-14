/***************************************************************************/
/*  curvedir.h                                                             */
/*                *** OMA 35 Version ***                                   */
/*                                                                         */
/*  copyright (c) 1989, EG&G Instruments Inc.                              */
/*                                                                         */
/*  Functions to set up and search directories of curves held in memory.   */
/*                                                                         */
/***************************************************************************/
  
#ifndef CURVEDIR_INCLUDED
#define CURVEDIR_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

#ifndef CRVENTRY_INCLUDED
  #include "crventry.h"
#endif

struct curve_directory
{
   CURVE_ENTRY * Entries; /* pointer to the start of the directory entries */
   USHORT BlkCount;       /* number of directory entries, 16K MAX */
   USHORT CurveCount;     /* number of curves over all entries    */
};

/* typedef struct curve_directory CURVEDIR; */
/* typedef CURVEDIR far * CURVEDIR *;       */

extern CURVEDIR MainCurveDir;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

ERR_OMA AddCurveBlkToDir(PCHAR, PCHAR, PCHAR, USHORT, ULONG, USHORT,
                                 struct tm *, CURVEDIR *, PSHORT, CHAR,
                                 USHORT);
ERR_OMA InsertCurveBlkInDir(PCHAR, PCHAR, PCHAR, USHORT, ULONG,
                                    USHORT, struct tm *, CURVEDIR *, USHORT,
                                    CHAR);
ERR_OMA DelCurveBlksFromDir(PSHORT, USHORT, CURVEDIR *);
ERR_OMA DelCurveBlocks(PSHORT, SHORT, CURVEDIR *);
ERR_OMA SplitCurveBlk(SHORT, USHORT, CURVEDIR *);
SHORT SearchNextNameAndCurveNum(PCHAR BlockName, CURVEDIR * CurveDir,
                                SHORT StartIndex, USHORT CurveNumber);
ERR_OMA FindCurveSetIndex(CHAR *Name, SHORT * Index, BOOLEAN DoSpecial);
SHORT SearchNextNamePath(PCHAR, PCHAR, CURVEDIR *, SHORT);
SHORT SearchCurveBlkDir(PCHAR, PCHAR, USHORT, CURVEDIR *);
SHORT GetCurveSetIndex(CHAR * Name, CHAR * Path, SHORT CurveNumber);
ERR_OMA RenameCurveBlk(PCHAR, PCHAR, SHORT, CURVEDIR *);
  
#ifdef OS2_MEM
ERR_OMA MergeCurveBlks(PCHAR, PCHAR, INDEXEDENTRY *, SHORT,
                               CURVEDIR *);
#endif  // OS2_MEM
  
ERR_OMA CopyCurveEntry(PCHAR, PCHAR, SHORT *, USHORT, USHORT,
                               CURVEDIR *, CURVEDIR *);

// Values for the SortFlags argument of SortCurveDir().
enum { SORT_TIME   = 1,    SORT_NAME    = 2,
       SORT_ASCEND = 0x10, SORT_DESCEND = 0x20
     };

ERR_OMA SortCurveDir(CURVEDIR * pCurveDir, USHORT SortFlags);
  
// Search MainCurveDir for a special (background or temporary)
// curve block
SHORT FindSpecialEntry(PCHAR Name);

// return values for CheckCurveBlkOverLap
enum { RANGEOK, BADNAME, SPLITRANGE, OVERLAPCAT, NOOVERLAPCAT, DISJOINT };

SHORT CheckCurveBlkOverLap(char *Path, char *Name, SHORT LowRange,
                            SHORT HighRange, PSHORT BlkIndex);

#endif
