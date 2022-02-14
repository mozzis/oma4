/***************************************************************************/
/*  curvedir.c                                                             */
/*                *** OMA 35 Version ***                                   */
/*                                                                         */
/*  copyright (c) 1988, EG&G Instruments Inc.                              */
/*                                                                         */
/*  Functions to set up and search directories of curves held in memory.   */
/*                                                                         */
/***************************************************************************/

/*
/
/  $Header:   J:/logfiles/oma4000/main/curvedir.c_v   1.12   30 Mar 1992 11:24:46   maynard  $
/  $Log:   J:/logfiles/oma4000/main/curvedir.c_v  $
/
*/
  
#ifdef PROT
   #define INCL_DOSMEMMGR
   #define INCL_NOPM
   #include <os2.h>
#endif
  
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
  
#include "curvedir.h"
#include "crventry.h"
#include "filestuf.h"
#include "tempdata.h"
#include "di_util.h"
#include "syserror.h" // ERROR_OPEN
#include "omaerror.h"
#include "curvbufr.h"
#include "multi.h"
#include "fileform.h" // is_special_name
          
#define MAXCURVEBLKS SEGSIZE / CURVEDIRSZ

CURVEDIR MainCurveDir;
  
/***************************************************************************/
/*  function: Create a curve block directory entry and put at the end of   */
/*            the entry list.                                              */
/*                                                                         */
/*  Variables:                                                             */
/*             BlockName - Input. Null ended string. Looks like a filename.*/
/*             Path - Full pathname if block refers to a file.             */
/*             desc - Input. Null ended description of the curve block. A  */
/*                    duplicate of the string will be made in this function*/
/*             StartIndex - Input. Index into the CURVE array for the      */
/*                          starting curve of this block.                  */
/*             StartOffset - Input.  Byte offset into start of file for the*/
/*                           start of the curve block.  May point to a     */
/*                           memory curve address array which conatins the */
/*                           addresses of all the individually allocated   */
/*                           curves in a block.                            */
/*             BlkSz - Input. Number of curves in this block.              */
/*             time - Input. Approximate time of the last change to this   */
/*                    curve block.                                         */
/*             CurveDir - Input. Pointer to the curve directory in which to*/
/*                        put the entry.                                   */
/*             EntryIndex - Output. Index of new Entry in the Entries Array*/
/*             EntryType - Input. Data File entry, or Method file entry    */
/*                         or other.                                       */
/*             DisplayWindow - What display window will the block be shown */
/*                             in. Bitflags - 0 for none. 1 for 1, 2 for 2 */
/*                             4 for 3, etc                                */
/*  Returns:   ERROR_NONE = OK                                             */
/*             ERROR_ALLOC_MEM = error on reallocation attempt             */
/*                                                                         */
/*  Side Effects: none                                                     */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA AddCurveBlkToDir(PCHAR BlockName, PCHAR Path, PCHAR desc,
                         USHORT StartIndex, ULONG startOffset,
                         USHORT BlkSz, TM *time, CURVEDIR *CurveDir,
                         PSHORT EntryIndex,
                         CHAR EntryType, USHORT DisplayWindow)
{
  CURVE_ENTRY *curve_entry;  /* shorthand pointer to curve block entry */
  PVOID Tmpptr;
  USHORT i= CurveDir->BlkCount; /* get the index to the next entry */

  /* resize the directory entry array to add one more entry */
  
  if ((Tmpptr = realloc(CurveDir->Entries, (i+1) * CURVEENTRYSZ)) == 0)
    return error(ERROR_ALLOC_MEM);
  else  /* realloc successful */
    CurveDir->Entries = Tmpptr;

  curve_entry = &CurveDir->Entries[i];

  /* Convert to upper case and copy the name */
  // But don't change BlockName or Path.
  strcpy(curve_entry->name, BlockName);
  strupr(curve_entry->name);

  strcpy(curve_entry->path, Path);
  strupr(curve_entry->path);

  strncpy(curve_entry->descrip, desc, DESCRIPTION_LENGTH);
  curve_entry->descrip[DESCRIPTION_LENGTH - 1] = '\0';

  curve_entry->StartIndex    = StartIndex;
  curve_entry->StartOffset   = startOffset;
  curve_entry->TmpOffset     = 0xFFFFFFFF;     /* no temporary data yet */
  curve_entry->count         = BlkSz;
  curve_entry->DisplayWindow = DisplayWindow;
  curve_entry->time          = *time;
  curve_entry->EntryType     = EntryType;     /* Data file, method file, */
                                              /* or other file */
  *EntryIndex = CurveDir->BlkCount;  /* entry location index to return */

  CurveDir->BlkCount++;  /* increase the count of curve block entries */
  CurveDir->CurveCount += BlkSz;

  return ERROR_NONE;
}
  
/***************************************************************************/
/*  function: Create a curve block directory entry and insert it in the    */
/*            proper place.                                                */
/*                                                                         */
/*  Variables:                                                             */
/*             BlockName - Input. Null ended string. Looks like a filename.*/
/*             Path - Full pathname if block refers to a file.             */
/*             desc - Input. Null ended description of the curve block.    */
/*             StartIndex - Input. Index into the CURVE array for the      */
/*                          starting curve of this block.                  */
/*             BlkSz - Input. Number of curves in this block.              */
/*             time - Input. Approximate time of the last change to this   */
/*                    curve block.                                         */
/*             CurveDir - Input. Pointer to the curve directory in which to*/
/*                        put the entry.                                   */
/*             EntryIndex - Input. Index of new Entry in the Entries Array */
/*             EntryType - Input. Data File entry, or Method file entry    */
/*                         or other.                                       */
/*  Returns:   FALSE = OK                                                  */
/*             ERROR_ALLOC_MEM = error on reallocation attempt             */
/*                                                                         */
/*  Side Effects: none                                                     */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA InsertCurveBlkInDir(PCHAR BlockName, PCHAR Path, PCHAR desc,
                            USHORT StartIndex, ULONG startOffset,
                            USHORT BlkSz, TM *time,
                            CURVEDIR *CurveDir,
                            USHORT EntryIndex, CHAR EntryType)
{
  CURVE_ENTRY *pEntry;  /* shorthand pointer to curve block entry */
  PVOID Tmpptr;
  ERR_OMA err;

  /* EntryIndex should be signed (is most elsewhere) since */
  /* Index == -1 means does not exist */

  if (EntryIndex > CurveDir->BlkCount)
    {
    return error(ERROR_BAD_CURVE_BLOCK, EntryIndex);
    }

  /* flush all temporary buffers */         
  if(err = clearAllCurveBufs())
    return err;

  // resize the directory entry array to add one more entry
  // The Microsoft C version 6.0 implementation of realloc() leaves
  // the original block unchanged if the reallocation fails.

  if ((Tmpptr = realloc(CurveDir->Entries,
      (CurveDir->BlkCount+1) * CURVEENTRYSZ)) == 0)
    {
    return error(ERROR_ALLOC_MEM);
    }
  else                             /* realloc successful */
    CurveDir->Entries = Tmpptr;
 
  pEntry = &(CurveDir->Entries[EntryIndex]);
  /* move current entries to make room for new one */
  memmove(&(CurveDir->Entries[EntryIndex + 1]), pEntry,
          (CurveDir->BlkCount-EntryIndex) * CURVEENTRYSZ);
 
  strcpy(pEntry->name, BlockName);
  strupr(pEntry->name);

  strcpy(pEntry->path, Path);
  strupr(pEntry->path);
 
  strncpy(pEntry -> descrip, desc, DESCRIPTION_LENGTH);
  pEntry -> descrip[DESCRIPTION_LENGTH - 1] = '\0';
 
  pEntry->StartIndex = StartIndex;
  pEntry->StartOffset = startOffset;
  pEntry->TmpOffset = 0xFFFFFFFF;     /* no temporary data yet */
  pEntry->count = BlkSz;
  pEntry->time = *time;
  pEntry->EntryType = EntryType; /* Data entry, method file, */
                                 /* or other file */
  pEntry -> DisplayWindow = 0;
 
  EntryIndex = CurveDir->BlkCount; /* entry location index to return */
  CurveDir->BlkCount++;  /* increase the count of curve block entries */
  CurveDir->CurveCount += BlkSz;

  return ERROR_NONE;
}

/***************************************************************************/
/*  function: Remove a curve block directory entry. Compact the directory. */
/*                                                                         */
/*  Variables: All are input.                                              */
/*             EntryIndex - Array of indices of entries to remove          */
/*             EntryNum - Number of entries to remove                      */
/*             CurveDir - Pointer to the entry's curve directory.          */
/*                                                                         */
/*  Returns:   FALSE = OK                                                  */
/*             ERROR_ALLOC_MEM = error on reallocation attempt             */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA DelCurveBlksFromDir(PSHORT EntryIndex, USHORT EntryNum,
                                   CURVEDIR *CurveDir)
  
{
   USHORT i;
   ERR_OMA err;
   USHORT byte_count;
   SHORT TempIndex;
  
   /* flush all temporary buffers */        
   if(err = clearAllCurveBufs())
      return err;
  
   /* if deleting all entries */
   if (EntryNum == CurveDir->BlkCount)
   {
      CurveDir->BlkCount = 0;
      /* resize the directory entry array */
      /* Something really wrong if a downsize doesn't work and there would */
      /* be no way to recover */
      if ((CurveDir->Entries = realloc(CurveDir->Entries, 1)) == 0)
        {
        return error(ERROR_ALLOC_MEM);
        }
      else
         return ERROR_NONE;
   }
  
   /* Multithread Sort */
   /* sort the array of Indices so that the highest are first. By deleting */
   /* the highest first, all of the lower indices are still valid */

   for (i=0; i<(USHORT) EntryNum-1; i++)
   {
      if (EntryIndex[i] < EntryIndex[i+1])
      { /* swap the values */
         EntryIndex[i] ^= EntryIndex[i+1];
         EntryIndex[i+1] ^= EntryIndex[i];
         EntryIndex[i] ^= EntryIndex[i+1];
         if (i!=0)
            i=-2;/* Need to recheck previous value to see which is lower.*/
         /* -2 because i will be incremented once. OK if i=-1 here*/
      }
   }
  
   for (i=0; i<(USHORT) EntryNum; i++)
   {
      CurveDir->BlkCount -= 1;
  
      /* don't need to move if at top of the entry list */
      if ((USHORT) EntryIndex[i] != CurveDir->BlkCount)
      {
         /* calculate number of bytes to move */
         TempIndex = EntryIndex[i];
  
         byte_count = (USHORT) (CurveDir->BlkCount - TempIndex) *
         sizeof (CURVE_ENTRY);
         memmove(&(CurveDir->Entries[TempIndex]),
                  &(CurveDir->Entries[TempIndex + 1]), byte_count);
      }
   }
  
   /* resize the directory entry array */
   /* Something really wrong if a downsize doesn't work and there would */
   /* be no way to recover */
   if ((CurveDir->Entries = realloc(CurveDir->Entries,
                  (USHORT) (CurveDir->BlkCount * CURVEENTRYSZ) + 1)) == 0)
     {
     return error(ERROR_ALLOC_MEM);
     }
  
   return ERROR_NONE;
}
  
/***************************************************************************/
/*  function: Remove curve block directory entries and deallocate the      */
/*            associated memory and temp curves (if necessary).  Resizes   */
/*            the curve directory entries array.                           */
/*                                                                         */
/*  Variables: All are input.                                              */
/*             EntryIndex - Array of indices of entries to remove          */
/*             EntryNum - Number of entries to remove                      */
/*             CurveDir - Pointer to the entry's curve directory.          */
/*                                                                         */
/*  Returns:   FALSE = OK                                                  */
/*             ERROR_ALLOC_MEM = error on reallocation attempt             */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA DelCurveBlocks(PSHORT EntryIndex, SHORT EntryNum,
                               CURVEDIR *CurveDir)
{
  SHORT i, j;
  ERR_OMA err;
  CURVE_ENTRY *pEntry;

  /* flush all temporary buffers */
  if(err = clearAllCurveBufs())
    return err;

  for (j=0; j<EntryNum; j++)
    {
    pEntry = &(CurveDir->Entries[EntryIndex[j]]);

    /* erase the temporary data */
    if (pEntry->TmpOffset != 0xFFFFFFFF)
      {  /* temp data is present */
      for (i=(SHORT) pEntry->StartIndex;
      i<(SHORT) (pEntry->StartIndex + pEntry->count); i++)
        {
        if (err = DelTempCurve(CurveDir, EntryIndex[j], i))
          return err;
        }
      }
    CurveDir->CurveCount -= pEntry->count;
    }

  if (err = DelCurveBlksFromDir(EntryIndex, EntryNum, CurveDir))
    return err;

  return ERROR_NONE;
}
  
/***************************************************************************/
/*  function: Remove curve block directory entries and deallocate the      */
/*            associated curves (if necessary).  Resizes the curve         */
/*            directory entries array.                                     */
/*                                                                         */
/*  Variables: All are input.                                              */
/*             EntryIndex - index of entry to split                        */
/*             CurveNum - file curve number offset to start second entry   */
/*             CurveDir - Pointer to the entry's curve directory.          */
/*                                                                         */
/*  Returns:   FALSE = OK                                                  */
/*             ERROR_OPEN - can't open file                                */
/*             ERROR_READ - stream READ ERROR                              */
/*             ERROR_ALLOC_MEM = error on reallocation attempt             */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA SplitCurveBlk(SHORT EntryIndex, USHORT CurveNum,
                             CURVEDIR *CurveDir)
{
  CURVE_ENTRY *pEntry1, *pEntry2;
  CHAR Buf[FNAME_LENGTH];
  FILE *fhnd;
  ULONG startOffset;
  ERR_OMA err;

  /* flush all temporary buffers */         
  if(err = clearAllCurveBufs())
    return err;

  pEntry1 = &(CurveDir->Entries[EntryIndex]);

  switch (pEntry1->EntryType)
    {
    case OMA4DATA:
      strcpy(Buf, pEntry1->path);
      strcat(Buf, pEntry1->name);
      fhnd = fopen(Buf, "rb");
      if (fhnd == NULL) {
        return error(ERROR_OPEN, Buf);
        }
      if (err = GetFileCurveOffset(fhnd, Buf, CurveNum, &startOffset))
        {
        if (err != ERROR_OPEN)
          {
          fclose(fhnd);
          return err;
          }
        }
      else
        fclose(fhnd);
    break;

    default:
      startOffset = 0;
    }

  if (err = InsertCurveBlkInDir(pEntry1->name, pEntry1->path,
                                pEntry1->descrip, CurveNum, startOffset,
                             pEntry1->count + pEntry1->StartIndex - CurveNum,
                                &(pEntry1->time), CurveDir, EntryIndex + 1,
                                pEntry1->EntryType))
      return err;
  
  pEntry1 = & (CurveDir -> Entries[ EntryIndex ]);
  pEntry1->count = CurveNum - pEntry1->StartIndex;
  
  pEntry2 = &(CurveDir->Entries[EntryIndex + 1]);
  
  /* get the offset of the second entry in the temp file if applicable */
  if (pEntry1->TmpOffset != 0xFFFFFFFF)
    {
    pEntry2->TmpOffset = pEntry1->TmpOffset;
    if (err = GetTempCurveOffset(CurveNum, &(pEntry2->TmpOffset)))
      return err;
    }
  return ERROR_NONE;
}
  
/***************************************************************************/
/*  function: Search a curve block directory for a matching curve block    */
/*            name and path.                                               */
/*                                                                         */
/*  Variables:                                                             */
/*             BlockName - Input. Null ended string. Looks like a filename.*/
/*             Path - Input. Null terminated string with ending '\\'.      */
/*                           Looks like a directory.                       */
/*             CurveDir - Input. Pointer to the curve search directory     */
/*             StartIndex - Input. Block index to start search             */
/*                                                                         */
/*  Returns:   Curve entry index 0..16K-1 if found                         */
/*             -1 = curve not found                                        */
/*                                                                         */
/***************************************************************************/
  
SHORT SearchNextNamePath(PCHAR BlockName, PCHAR Path,
                         CURVEDIR *CurveDir, SHORT StartIndex)
{
  SHORT i;
  CURVE_ENTRY *EntryArray = CurveDir->Entries;

  for(i = (SHORT) StartIndex; i < (SHORT) CurveDir->BlkCount; i++)
    {
    if ((strcmpi(BlockName, EntryArray[i].name) == 0) &&
      (strcmpi(Path, EntryArray[i].path) == 0))
      return i;
    }
  return NOT_FOUND; // not found
}

/***************************************************************************/
/*  function: Given a string, find the curve directory index for the cor-  */
/*            responding curve set.  The string is checked to see if it    */
/*            contains a valid path and filename. If DoSpecial is TRUE,    */
/*            then lastlive, background, temp, etc. count.                 */
/*  Variables:                                                             */
/*            Spec - Input. Null ended string. Looks like a filespec.      */
/*            Index - Output. Index in curve directory of found block.     */
/*            DoSpecial - Input.  Special files count if set to TRUE.      */
/*                                                                         */
/*  Returns:  ERROR_BAD_FILENAME if filename was invalid.                  */
/*            ERROR_NONE if filename was OK.                               */
/*                                                                         */
/***************************************************************************/

ERR_OMA FindCurveSetIndex(CHAR *Spec, SHORT * Index, BOOLEAN DoSpecial)
{
  CHAR Name[DOSFILESIZE + 1], Path[DOSPATHSIZE + 1];
  BOOLEAN specialFile = is_special_name(Spec);

  *Index = NOT_FOUND;

  if (specialFile && !DoSpecial)
    return error(ERROR_IMPROPER_FILETYPE, Spec);
  /* expand the file block name or constant string and check to see        */
  /* that it is not a directory                                            */
  /* if FileName is a special file name, name is OK so don't uppercase it. */

  if(specialFile)
    {
    Path[0] = '\0';
    strcpy(Name, Spec);
    }
  else if (ParsePathAndName(Path, Name, Spec) != 2)
      return error(ERROR_BAD_FILENAME, Spec);

  *Index = SearchNextNamePath(Name, Path, &MainCurveDir, 0);
  return ERROR_NONE;
}
  
/***************************************************************************/
/*  function: Search a curve block directory for a matching curve.         */
/*                                                                         */
/*  Variables: All are input.                                              */
/*             BlockName - Null ended string. Looks like a filename.       */
/*             Path - Input. Null terminated string with ending '\\'.      */
/*                           Looks like a directory.                       */
/*             CurveNumber - Curve number in this block to match. Curve    */
/*                numbers in a block do not necessary start from 0.        */
/*             CurveDir - Pointer to the curve search directory            */
/*                                                                         */
/*  Returns:   Curve entry index 0..16K-1 if found                         */
/*             -1 = curve not found                                        */
/*                                                                         */
/***************************************************************************/
  
SHORT SearchCurveBlkDir(PCHAR BlockName, PCHAR Path,
                        USHORT CurveNumber, CURVEDIR *CurveDir)
{
  SHORT Found;     /* Boolean for matching operation */
  CURVE_ENTRY *EntryArray;
  SHORT LastSearch = NOT_FOUND;
 
  EntryArray = CurveDir->Entries;
  do
    {
    LastSearch++;
    LastSearch = SearchNextNamePath(BlockName, Path, CurveDir, LastSearch);
    Found = LastSearch;
    /* did name match? If so, check for curve number match.*/
    if (Found != NOT_FOUND)
      {  /* name matched but the correct curve wasn't in this block */
      if ((CurveNumber < EntryArray[Found].StartIndex) ||
        (CurveNumber >= (EntryArray[Found].StartIndex +
        EntryArray[Found].count)))
        Found = NOT_FOUND;
      }
    }
  while ((Found == NOT_FOUND) && (LastSearch != NOT_FOUND));
 
  return Found;  /* still NOT_FOUND if failure */
}

/**************************************************************************/
/*                                                                        */
/* Search for a named curve set, ignoring the path sepcification          */
/* if CurveNumber is -1, it is also ignored                               */
/*                                                                        */
/**************************************************************************/
SHORT SearchNextNameAndCurveNum(PCHAR BlockName, CURVEDIR *CurveDir,
                                SHORT StartIndex, USHORT CurveNumber)
{
  SHORT i,  Found;
  CURVE_ENTRY *EntryArray;

  Found = NOT_FOUND;
  EntryArray = CurveDir->Entries;
  for (i=StartIndex; i < (SHORT)CurveDir->BlkCount && Found == NOT_FOUND; i++)
    {
    if (strcmpi(BlockName, EntryArray[i].name) == 0)
      {
      if (CurveNumber == 0xFFFF)
        Found = i;
      else
        {
        if ((CurveNumber == EntryArray[i].StartIndex) ||
            ((CurveNumber > EntryArray[i].StartIndex) &&
            (CurveNumber < (EntryArray[i].StartIndex + EntryArray[i].count))))
          Found = i;
        }
      }
    }        /* for i */
  return Found;  /* still NOT_FOUND if failure */
}

/**************************************************************************/
/*                                                                        */
/* Get the Entry Index of a curve set.  If Path == "", it is ignored.     */
/* If CurveNumber == -1, it is also ignored                               */
/*                                                                        */
/**************************************************************************/
SHORT GetCurveSetIndex(CHAR * Name, CHAR * Path, SHORT CurveNumber)
{
  if (strlen(Path))
    {
    if (CurveNumber == -1) /* if curve # unknown (-1), don't try to match; */
      {                    /* start search at entry zero */
      return SearchNextNamePath(Name, Path, &MainCurveDir, 0);
      }
    else
      {
      return SearchCurveBlkDir(Name, Path, CurveNumber, &MainCurveDir);
      }
    }
  else     /* path not specified (curvenumber may or may not be -1) */
    {
    return SearchNextNameAndCurveNum(Name, &MainCurveDir, 0, CurveNumber);
    }
}
  
// RenameCurveBlk() is never used, comment out. RAC 9/10/91
/***************************************************************************/
/*  Variables: All are input.                                              */
/*             BlockName - Null ended string. Looks like a filename.       */
/*             Path - Null ended string. Looks like a directory.           */
/*             EntryIndex - Index of curve block entry                     */
/*             CurveDir - Pointer to a curve directory                     */
/*                                                                         */
/*  Returns:   FALSE if OK                                                 */
/*             TRUE if EntryIndex is not available                         */
/*                                                                         */
/* Side Effects:                                                           */
/*                                                                         */
/***************************************************************************/
//  
//ERR_OMA RenameCurveBlk(PCHAR BlockName, PCHAR Path, SHORT EntryIndex,
//                              CURVEDIR *CurveDir)
//{
//   /* check to see if EntryIndex is valid */
//   if ((USHORT) EntryIndex > CurveDir->BlkCount)
//      return TRUE;
//   /* capitalize new string */
//  
//   //strupr(BlockName);
//   strcpy(CurveDir->Entries[EntryIndex].name, BlockName);
//   strupr(CurveDir->Entries[EntryIndex].name);
//
//   //strupr(Path);
//   strcpy(CurveDir->Entries[EntryIndex].path, Path);
//   strupr(CurveDir->Entries[EntryIndex].path);
//   return FALSE;
//}
  
/***************************************************************************/
/*  function: Combine curve block entries.  The order is curves from       */
/*            block 0 followed by the other curves.  The entry fields      */
/*            from block 0 will be used with the updated curve count and   */
/*            time-date. The individual curve blocks will be not be erased.*/
/*                                                                         */
/*  Variables:                                                             */
/*             BlockName - Input. Null ended string for naming new         */
/*                         duplicate block. Looks like a filename.         */
/*             Path - Input. Null ended string. Looks like a directory.    */
/*             pEntryIndex[] - I/O. Array of indices of curve block        */
/*                            entries to merge. On exit EntryIndex[0].Index*/
/*                            is set to the index of the merged curves.    */
/*             NumBlocks - Input. Number of blocks to merge                */
/*             CurveDir - Input. Pointer to a curve directory              */
/*                                                                         */
/*  Returns:   FALSE if OK                                                 */
/*             ERROR_ALLOC_MEM if error                                    */
/*                                                                         */
/***************************************************************************/
// only #ifdef OS2_MEM  
//ERR_OMA MergeCurveBlks(PCHAR BlockName, PCHAR Path,
//                              INDEXEDENTRY *pEntryIndex,
//                              SHORT NumBlocks, CURVEDIR *CurveDir)
//{
//   return ERROR_NONE;
//}
 
/***************************************************************************/
/*  function: Sort directory entries according to given sorting criteria.  */
/*                                                                         */
/*  Variables:                                                             */
/*             pCurveDir - Input. Pointer to curve directory.              */
/*             SortFlags - Input. Bit flags as to sorting criteria.        */
/*                         SORT_TIME or SORT_NAME may be OR'd with         */
/*                         SORT_ASCEND or SORT_DESCEND                     */
/*                                                                         */
/*  Returns:   FALSE                                                       */
/*                                                                         */
/* Side Effects:                                                           */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA SortCurveDir(CURVEDIR *pCurveDir, USHORT SortFlags)
{
   ERR_OMA err;
  
   /* flush all temporary buffers */         
   if(err = clearAllCurveBufs())
      return err;
  
   if (SortFlags & SORT_NAME)
   {
      if (SortFlags & SORT_ASCEND)
         qsort(pCurveDir->Entries, pCurveDir->BlkCount, CURVEENTRYSZ,
         DirEntryNameCompare);
      else if (SortFlags & SORT_DESCEND)
         qsort(pCurveDir->Entries, pCurveDir->BlkCount, CURVEENTRYSZ,
         InverseDirEntryNameCompare);
   }
  
   else if (SortFlags & SORT_TIME)
   {
      if (SortFlags & SORT_ASCEND)
         qsort(pCurveDir->Entries, pCurveDir->BlkCount, CURVEENTRYSZ,
         DirEntryTimeCompare);
      else if (SortFlags & SORT_DESCEND)
         qsort(pCurveDir->Entries, pCurveDir->BlkCount, CURVEENTRYSZ,
         InverseDirEntryTimeCompare);
   }
  
   return ERROR_NONE;
}

/***************************************************************************/
/*  function: Search a curve block directory for a special (background or  */
/*            temporary curve block -- always use MainCurveDir             */
/*                                                                         */
/*  Variables:                                                             */
/*             Name - Input. Case sensitive block name                     */
/*                                                                         */
/*  Returns:   Curve entry index 0..16K-1 if found                         */
/*             -1 = curve not found                                        */
/*                                                                         */
/***************************************************************************/

SHORT FindSpecialEntry(PCHAR Name)
{
   SHORT i;
   CURVE_ENTRY *EntryArray = MainCurveDir.Entries;

   for(i = 0; i < (SHORT) MainCurveDir.BlkCount; i++)
   {
      if(   (strcmp(Name, EntryArray[i].name) == 0)
          && (EntryArray[i].path[0] == '\0'))
         return i;
   }
   return NOT_FOUND;
}

/****************************************************************************
* Requires: Path - full pathname, with ending '\\'                          *
*           Name - file name                                                *
*           LowRange -  the low index of the curve block                    *
*           HighRange - the high index of the curve block                   *
*           BlkIndex - output.  Index to the block, conditional to return   *
*                      restrictions as shown by return value. Returns as -1 *
*                      if no path/name is found                             *
*                                                                           *
* Returns: RANGEOK - a curve block entry exists in which the complete       *
*                    range is present                                       *
*          BADNAME - Name/Path Not found                                    *
*          SPLITRANGE - Range present in two or more path/name blocks,      *
*                       BlkIndex returns with the lowest block index        *
*                       part that contains some of the range.               *
*          OVERLAPCAT - range can be concatenated to a block that is        *
*                       present, with some overlap. BlkIndex is set to the  *
*                       block's index.                                      *
*          NOOVERLAPCAT - Range can be concatenated to a block that is      *
*                         present, with no overlap. BlkIndex is set to      *
*                         the block's index.                                *
*          DISJOINT - Range cannot be concatenated to any blocks present,   *
*                        Sets are disjoint.                                 *
****************************************************************************/
  
SHORT CheckCurveBlkOverLap(CHAR *Path, CHAR *Name, SHORT LowRange,
                            SHORT HighRange, PSHORT BlkIndex)
{
  CURVE_ENTRY *pEntry;
  SHORT i;
  SHORT TempIndex;

  *BlkIndex = 0;
  /* check for name/path */
  if (SearchNextNamePath(Name, Path, &MainCurveDir, *BlkIndex) == NOT_FOUND)
    {
    *BlkIndex = NOT_FOUND;
    return BADNAME;   /* name and path not found */
    }
  TempIndex = *BlkIndex;

  /* find the block */
  if ((*BlkIndex = SearchCurveBlkDir(Name, Path, LowRange,
    &MainCurveDir)) != NOT_FOUND)
    {
    pEntry = &(MainCurveDir.Entries[*BlkIndex]);
    if ((i=pEntry->StartIndex + pEntry->count) > HighRange)
      return RANGEOK;
    }
  else
    i=LowRange+1;

  /* check to see if any part of the range is contained in any other */
  /* blocks */
  while ((i <= HighRange) &&
    (SearchCurveBlkDir(Name, Path, i, &MainCurveDir) == NOT_FOUND))
    i++;

  /* check to see if any curves found in other blocks */
  if (i > HighRange)
    {
    if (*BlkIndex != NOT_FOUND)
      return OVERLAPCAT;
    else
      {
      *BlkIndex = TempIndex;
      return NOOVERLAPCAT;
      }
    }
  else
    return SPLITRANGE;

  /* code from this point on disabled.  program will not deal with */
  /* "disjoint" curve blocks */


//  /* No part of block was found in the curve directory. */
//  /* Check for concatenation with no overlap. */
//  *BlkIndex = SearchCurveBlkDir(Name, Path, LowRange-1, &MainCurveDir);
//  if (*BlkIndex == NOT_FOUND)
//    return DISJOINT;
//  else
//    return NOOVERLAPCAT;
  
}

