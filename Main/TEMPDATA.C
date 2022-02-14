/***************************************************************************/
/*                                                                         */
/*  tempdata.c                                                             */
/*                                                                         */
/*  copyright (c) 1988, EG&G Instruments Inc.                              */
/*                *** OMA 35 Version ***                                   */
/*                                                                         */
/*       Routines to use VDISK or normal disk files as temporary storage.  */
/*    Three buffers will be kept in normal memory to try to speed disk     */
/*    access.                                                              */
/*                                                                         */
/*    WARNING! Only one CURVEDIR structure per program should try to use   */
/*    these functions!                                                     */
/*                                                                         */
/*
*  $Header:   J:/logfiles/oma4000/main/tempdata.c_v   1.23   06 Jul 1992 10:37:32   maynard  $
*  $Log:   J:/logfiles/oma4000/main/tempdata.c_v  $
 * 
/***************************************************************************/
  
#ifdef PROT
   #define INCL_DOSMEMMGR
   #define INCL_NOPM
   #include <os2.h>
#endif
  
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>         // chsize()

#include "tempdata.h"
#include "curvbufr.h"
#include "crventry.h"
#include "filestuf.h"
#include "syserror.h"  // ERROR_OPEN
#include "omaerror.h"
#include "crvhdr35.h"
#include "curvedir.h"
#include "oma4driv.h"
#include "detsetup.h"
  
#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

FILE *hTempFile;
CHAR *TempFileBuf;
ULONG TempFileSz = 0;
  
/***************************************************************************/
/*                                                                         */
/*       ERR_OMA ChangeTempFileSize(ULONG NewSize)                */
/*                                                                         */
/*  Function:  Change the size of the temporary data file.                 */
/*                                                                         */
/*  Variables:                                                             */
/*             NewSize - Size in bytes for the Temp file.                  */
/*  Returns:                                                               */
/*             ERROR_OPEN - temp file open error                           */
/*             ERROR_CLOSE - temp file close error                         */
/*             ERROR_WRITE - stream write error                            */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Changes the file handle of the temporary data file.      */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA ChangeTempFileSize(LONG NewSize)
{
   ERR_OMA err;
  
   if(err = clearAllCurveBufs())    // flush all temporary buffers
      return err;
  
   if(chsize(fileno(hTempFile), NewSize) == -1)
      error(err = ERROR_WRITE, TempFileBuf);

   return err;
}

void tempFileIncrementSize(LONG sizeIncrement)
{
  TempFileSz += sizeIncrement;
}


LONG tempFileSize(void)
{
  return(TempFileSz);
}
  
/***************************************************************************/
/*                                                                         */
/* ERR_OMA GetTempCurveOffset(USHORT CurveNum, PULONG pCurveOffset)*/
/*                                                                         */
/*  Function:  Get the total offset to the specified curve in the temporary*/
/*             file.                                                       */
/*                                                                         */
/*  Variables:                                                             */
/*             CurveNum - Number of the curve in the Block. 0 is the index */
/*                        of the first curve in a block. Not the file curve*/
/*                        index!!!                                         */
/*             pCurveOffset - Input - Block offset from start of temp file.*/
/*                            Output - Curve's offset from start of temp   */
/*                                     file                                */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_READ - temp file seek error                           */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Changes file pointer to the start of the curve           */
/*                for following read operations.                           */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA GetTempCurveOffset(USHORT CurveNum, PULONG pCurveOffset)
{
  USHORT   i;
  CURVEHDR TmpCurvehdr;
  USHORT   DataSz;
  USHORT   StartIndex = 0;

  static ULONG  LastBlkOffset   = 0L;
  static USHORT LastCurveIndex  = 0 ;
  static ULONG  LastCurveOffset = 0L;

  if ((CurveNum >= LastCurveIndex) && (LastBlkOffset == *pCurveOffset))
    {
    *pCurveOffset = LastCurveOffset;     /* Pete P. 2-10-92 */
    StartIndex = LastCurveIndex;
    }
  else
    LastBlkOffset = *pCurveOffset;

  if (fseek(hTempFile, *pCurveOffset, SEEK_SET) != 0)
    return error(ERROR_SEEK, TempFileBuf);

  for (i = StartIndex; i < CurveNum; i++)
    {
    if (fread(&TmpCurvehdr, sizeof (CURVEHDR), 1, hTempFile) != 1)
      return error(ERROR_READ, TempFileBuf);

    if (TmpCurvehdr.MemData)    /* if mem data, only 1 curve header! */
      {                           /* and you just read it! */
      fseek(hTempFile, *pCurveOffset, SEEK_SET);
      break;                    /*quit the for loop when memdata seen*/
      }

    DataSz = TmpCurvehdr.DataType & 0x0F;

    /* add the X and Y points to the curve offset */

    *pCurveOffset += sizeof(CURVEHDR) + (ULONG) TmpCurvehdr.pointnum *
      (ULONG) (DataSz << 1);

    if (fseek(hTempFile, *pCurveOffset, SEEK_SET) != 0)
      return error(ERROR_SEEK, TempFileBuf);
    }

  LastCurveOffset = *pCurveOffset;
  LastCurveIndex = CurveNum; 
 
  return ERROR_NONE;
}
  
/***************************************************************************/
/*                                                                         */
/* ERR_OMA GetTempFileLen(CURVEDIR *pCurveDir, ULONG *pFileLen)     */
/*                                                                         */
/*  Function:  Get the temporary file length.                              */
/*             Used only by scrlltst!                                      */
/*  Variables:                                                             */
/*             CurveDir - Input. Curve directory for curve entries         */
/*             pFileLen - Output - Length of temp file                     */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_READ - temp file seek error                           */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Changes file pointer.                                    */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA GetTempFileLen(CURVEDIR *pCurveDir,ULONG *pFileLen)
{
   USHORT i;
   CURVE_ENTRY *pEntry;
   CURVEHDR TmpCurvehdr;
   USHORT DataSz;
  
   *pFileLen = 0;
   /* find the last curve block in the temp curve file */
   for (i=0; i<pCurveDir->BlkCount; i++)
   {
      pEntry = &(pCurveDir->Entries[i]);
      if (pEntry->TmpOffset > *pFileLen)
         *pFileLen = pEntry->TmpOffset;
   }
  
   /* find the offset of the last curve in the last block */
   for (i=0; i<pEntry->count; i++)
   {
      if (fseek(hTempFile, *pFileLen, SEEK_SET) != 0)
         return error(ERROR_SEEK, TempFileBuf);
      if (fread(&TmpCurvehdr, sizeof (CURVEHDR), 1, hTempFile) != 1)
         return error(ERROR_READ, TempFileBuf);
      DataSz = TmpCurvehdr.DataType & 0x0F;

      /* add the X and Y points to the curve offset */

      if (TmpCurvehdr.MemData) /* live Cvhdrs not followed by data */
         {
         *pFileLen += sizeof(CURVEHDR);
         i += (TmpCurvehdr.CurveCount - 1); /* header may contain more curves */
         }
      else 
         *pFileLen += sizeof(CURVEHDR) +
         ((ULONG) TmpCurvehdr.pointnum * (ULONG) DataSz * 2L);
   }
  
   return ERROR_NONE;
}

/***************************************************************************/
/*                                                                         */
/* ERR_OMA DelTempCurve(CURVEDIR *CurveDir, SHORT EntryIndex,     */
/*                                                    USHORT CurveIndex)  */
/*                                                                         */
/*  Function:  Remove a curve from the temporary curve file                */
/*                                                                         */
/*  Variables:                                                             */
/*             CurveDir - Input. Curve directory for curve entry           */
/*             EntryIndex - Input. Entry for curve block                   */
/*             CurveIndex - Input. File curve number to Delete.            */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - temp file open error                           */
/*             ERROR_CLOSE - temp file close error                         */
/*             ERROR_READ - temp file seek error                           */
/*             ERROR_WRITE - temp file write error                         */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: May overwite temporary curves in file. Flushes and       */
/*                invalidates all curve buffers.  Resets curve offsets in  */
/*                the curve directory. Doesn't adjust StartIndex if        */
/*                Curveindex = StartIndex                                  */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA DelTempCurve(CURVEDIR *CurveDir, SHORT EntryIndex,
                             USHORT CurveIndex)
{
  return DelMultiTempCurve(CurveDir, EntryIndex, CurveIndex, 1);
}
  
/***************************************************************************/
/*                                                                         */
/* ERR_OMA DelMultiTempCurve(CURVEDIR *CurveDir, SHORT EntryIndex,*/
/*                                   USHORT CurveIndex, USHORT Count)     */
/*                                                                         */
/*  Function:  Remove a group of curves from the temporary curve file.     */
/*                                                                         */
/*  Variables:                                                             */
/*             CurveDir - Input. Curve directory for curve entry           */
/*             EntryIndex - Input. Entry for curve block                   */
/*             CurveIndex - Input. File curve number to Delete.            */
/*             Count - Input. Number of curves to delete.                  */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - temp file open error                           */
/*             ERROR_CLOSE - temp file close error                         */
/*             ERROR_READ - temp file seek error                           */
/*             ERROR_WRITE - temp file write error                         */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: May overwite temporary curves in file. Flushes and       */
/*                invalidates all curve buffers.  Resets curve offsets in  */
/*                the curve directory. Doesn't adjust StartIndex if        */
/*                Curveindex = StartIndex                                  */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA DelMultiTempCurve(CURVEDIR *CurveDir, USHORT EntryIndex,
                                 USHORT CurveIndex, USHORT Count)
{
  ERR_OMA err;
  USHORT i;
  ULONG SrcBlkSz, MoveLen;
  ULONG DelCurveOffset;
  CURVE_ENTRY *pEntry;

  if (Count == 0)
    return ERROR_NONE;

  if (EntryIndex >= CurveDir->BlkCount)
    {
    return error(ERROR_BAD_CURVE_BLOCK, EntryIndex);
    }

  pEntry = &(CurveDir->Entries[EntryIndex]);

  if(CurveIndex < pEntry->StartIndex) {
    return error(ERROR_CURVE_NUM, CurveIndex);
    }

  if ((CurveIndex + Count) > (pEntry->StartIndex + pEntry->count)) {
    return error(ERROR_CURVE_NUM, CurveIndex + Count);
    }

  /* flush all temporary buffers */
  if(err = clearAllCurveBufs())
    return err;

  /* get the block's starting offset */
  DelCurveOffset = pEntry->TmpOffset;
  if (err = GetTempCurveOffset(CurveIndex - pEntry->StartIndex,
    &DelCurveOffset))
    return err;

  /* Size to delete does not include mem curves referred to (==TRUE) */
  if (err = GetCurveBlkSz(hTempFile, TempFileBuf, DelCurveOffset, Count,
    & SrcBlkSz, TRUE))
    return err;

  MoveLen = TempFileSz - (DelCurveOffset + SrcBlkSz);
  /* compact file down */
  if (MoveLen)
    if (err = MoveFileBlock(hTempFile, TempFileBuf, DelCurveOffset,
                            hTempFile, TempFileBuf,
                            DelCurveOffset + SrcBlkSz, MoveLen))
      return err;

  /* bookkeeping in directory structures */
  /* Correct the temp file pointers */
  for (i=0; i<CurveDir->BlkCount; i++)
    {
    if (CurveDir->Entries[i].TmpOffset > DelCurveOffset)
      CurveDir->Entries[i].TmpOffset -= SrcBlkSz;
    }

  TempFileSz -= SrcBlkSz;
  CurveDir->CurveCount -= Count;
  CurveDir->Entries[EntryIndex].count -= Count;

  /* resize the file */
  if (err = ChangeTempFileSize(TempFileSz))
    return err;
  
  return ERROR_NONE;
}
  
/***************************************************************************/
/*  Function:  Insert a curve into a curve block before the curve at the   */
/*             DstEntryIndex and DstCurveIndex already in the temporary    */
/*             curve file.  The inserted curve must already be in the      */
/*             temporary data file.                                        */
/*                                                                         */
/*  Variables:                                                             */
/*             CurveDir - Input. Curve directory for curve entry           */
/*             SrcEntryIndex - Input. Entry for Source curve block.        */
/*             SrcCurveIndex - Input. Source file curve number.            */
/*             DstEntryIndex - Input. Entry for destination curve block.   */
/*             DstCurveIndex - Input. Destination file curve number.       */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - temp file open error                           */
/*             ERROR_CLOSE - temp file close error                         */
/*             ERROR_READ - temp file seek error                           */
/*             ERROR_WRITE - temp file write error                         */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_CURVE_NUM - source or destination curve index is bad  */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Uses CvBuf[2].  Resets curve offsets in the curve        */
/*                directory. Flushes and invalidates all curve buffers.    */
/*                Increases the curve counts for the entry and directory.  */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA InsertTempCurve(CURVEDIR *CurveDir, SHORT SrcEntryIndex,
                               USHORT SrcCurveIndex, SHORT DstEntryIndex,
                               USHORT DstCurveIndex )
{
   return InsertMultiTempCurve(CurveDir, SrcEntryIndex, SrcCurveIndex,
                               DstEntryIndex, DstCurveIndex, 1);
}
  
/***************************************************************************/
/*  Function:  Insert a group of curves into a curve block before the      */
/*             curve at the DstEntryIndex and DstCurveIndex already in     */
/*             the temporary curve file.  The inserted curves must already */
/*             be in the temporary data file.                              */
/*                                                                         */
/*  Variables:                                                             */
/*             CurveDir - Input. Curve directory for curve entry           */
/*             SrcEntryIndex - Input. Entry for Source curve block.        */
/*             SrcCurveIndex - Input. Source file curve number.            */
/*             DstEntryIndex - Input. Entry for destination curve block.   */
/*             DstCurveIndex - Input. Destination file curve number.       */
/*             Count - Input.  Number of curves to move.                   */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - temp file open error                           */
/*             ERROR_CLOSE - temp file close error                         */
/*             ERROR_READ - temp file seek error                           */
/*             ERROR_WRITE - temp file write error                         */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_CURVE_NUM - source or destination curve index is bad  */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Uses CvBuf[2].  Resets curve offsets in the curve        */
/*                directory. Flushes and invalidates all curve buffers.    */
/*                Increases the curve counts for the entry and directory.  */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA InsertMultiTempCurve(CURVEDIR *CurveDir,
                                    SHORT SrcEntryIndex,
                                    USHORT SrcCurveIndex,
                                    SHORT DstEntryIndex,
                                    USHORT DstCurveIndex,
                                    USHORT Count)
{
   ERR_OMA err = ERROR_NONE;
   ULONG SrcCurveOffset, SrcBlkSz, DstCurveOffset, MoveLen;
   CURVE_ENTRY *pSrcEntry, *pDstEntry;
   USHORT i;
  
   if (Count == 0)
      return ERROR_NONE;
  
   if (SrcEntryIndex == NOT_FOUND)
    return error(ERROR_BAD_CURVE_BLOCK, SrcEntryIndex);

   pSrcEntry = &(CurveDir->Entries[SrcEntryIndex]);
   pDstEntry = &(CurveDir->Entries[DstEntryIndex]);
  
   /* Check to see if trying to insert past block, append to block is OK */
   if((SrcCurveIndex < pSrcEntry->StartIndex) ||
      ((SrcCurveIndex + Count) > (pSrcEntry->StartIndex + pSrcEntry->count)))
      {
      return error(ERROR_CURVE_NUM, SrcCurveIndex);
      }

   if ((DstCurveIndex > (pDstEntry->StartIndex + pDstEntry->count)) ||
       (DstCurveIndex < pDstEntry->StartIndex))
     {
      return error(ERROR_CURVE_NUM, DstCurveIndex);
     }
  
   /* Make space for the insertion */
   DstCurveOffset = pDstEntry->TmpOffset;
   SrcCurveOffset = pSrcEntry->TmpOffset;
  
   /* CurveOffset goes in as the curve block's starting byte offset in the */
   /* temp file and comes out as the (CurveIndex) curve's byte offset into */
   /* the file. This function doesn't set file position for writing.       */

   if (pDstEntry->count) /* doesn't work if Block has no curves */
     {
     if (err = GetTempCurveOffset(DstCurveIndex - pDstEntry->StartIndex,
       &DstCurveOffset))
       return err;
     }
  
   if (err = GetTempCurveOffset(SrcCurveIndex - pSrcEntry->StartIndex,
      &SrcCurveOffset))
      return err;
  
   /* get the block size */
   if (err = GetCurveBlkSz(hTempFile, TempFileBuf, SrcCurveOffset, Count,
                            & SrcBlkSz, FALSE))
      return err;
  
   /* resize the file */
   if (err = ChangeTempFileSize(TempFileSz + SrcBlkSz))
      return err;
  
   MoveLen = TempFileSz - DstCurveOffset;

   /* if dest already contains curves, move following curves up */

   if (pDstEntry->count) /* doesn't work if Block has no curves */
     {
     if (err = MoveFileBlock(hTempFile, TempFileBuf,
                              DstCurveOffset + SrcBlkSz,
                              hTempFile, TempFileBuf, DstCurveOffset,
                              MoveLen))
       return err;
     }
  
   if (SrcCurveOffset >= DstCurveOffset)
   {
      SrcCurveOffset += SrcBlkSz;
      pSrcEntry->TmpOffset = SrcCurveOffset;
   }
  
   /* copy the curve to be inserted from tempfile to file */

   if (pSrcEntry->EntryType == OMA4MEMDATA || pDstEntry->EntryType == OMA4MEMDATA)
   {
      if (err = TempFileWrite(hTempFile,
                              TempFileBuf,
                              DstCurveOffset,
                              CurveDir,
                              SrcEntryIndex,
                              SrcCurveIndex,
                              DstCurveIndex,
                              Count,
                              pDstEntry->EntryType))
         return err;
   }
   else
      err = MoveFileBlock(hTempFile, TempFileBuf, DstCurveOffset,
                           hTempFile, TempFileBuf,
                           SrcCurveOffset, SrcBlkSz);
  
   /* bookkeeping in directory structures */
   /* Correct the temp file pointers */

   for (i=0; i<CurveDir->BlkCount; i++)
   {
      if (((SHORT)i != SrcEntryIndex) && ((SHORT)i != DstEntryIndex) &&
         (CurveDir->Entries[i].TmpOffset >= DstCurveOffset))
         CurveDir->Entries[i].TmpOffset += SrcBlkSz;
   }

   TempFileSz += SrcBlkSz;
   if (! err)
   {
      CurveDir->CurveCount += Count;
      CurveDir->Entries[DstEntryIndex].count += Count;
   }
  
   return err;
}

/***************************************************************************/
static ERR_OMA MoveTempFileBlock(ULONG Dst, ULONG Src, ULONG MoveLen)
{
  return MoveFileBlock(hTempFile, TempFileBuf, Dst,
                       hTempFile, TempFileBuf, Src, MoveLen);

}
  
/***************************************************************************/
/*                                                                         */
/* ERR_OMA ChangeCurveSize(CURVEDIR *CurveDir, SHORT EntryIndex,   */
/*                                 USHORT CurveIndex, USHORT TotalPoints.  */
/*                                 CHAR DataType)                          */
/*                                                                         */
/*  Function:  Change the size of a curve in the temporary curve file.     */
/*             Current X and Y point values will stay valid but new points */
/*             will be set randomly.                                       */
/*                                                                         */
/*  Variables:                                                             */
/*             CurveDir - Input. Curve directory for curve entry           */
/*             EntryIndex - Input. Entry for curve block                   */
/*             CurveIndex - Input. File curve number to change.            */
/*             TotalPoints - Input. Number of points for new size.         */
/*             DataType - see OMA35.h, FLOATTYPE, SHORTTYPE, etc.          */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - temp file open error                           */
/*             ERROR_CLOSE - temp file close error                         */
/*             ERROR_READ - temp file seek error                           */
/*             ERROR_WRITE - temp file write error                         */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Flushes and invalidates all curve buffers.  Resets curve */
/*                offsets in the curve directory.                          */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA ChangeCurveSize(CURVEDIR *CurveDir, SHORT EntryIndex,
                                 USHORT CurveIndex, USHORT TotalPoints,
                                 CHAR DataType)
{
  USHORT i;
  ERR_OMA err;
  ULONG CurveOffset, MoveLen,
        NewCurveLen, CurCurveLen;
  USHORT CurDataSz, NewDataSz,
         CurPoints;
  CURVEHDR TmpCurvehdr;

  if (CurveDir->Entries[EntryIndex].EntryType == OMA4MEMDATA)
    return error(ERROR_IMPROPER_FILETYPE, CurveDir->Entries[EntryIndex].name);

  /* flush all temporary buffers */
  if(err = clearAllCurveBufs())
    return err;

  /* Check for valid curve */
  if (CurveIndex >= CurveDir->Entries[EntryIndex].count)
    return error(ERROR_CURVE_NUM, CurveIndex);

  /* Make space for the insertion */
  CurveOffset = CurveDir->Entries[EntryIndex].TmpOffset;

  /* CurveOffset goes in as the curve block's starting byte offset in the */
  /* temp file and comes out as the (CurveIndex) curve's byte offset into */
  /* the file. This function doesn't set file position for writing.       */
  if (err = GetTempCurveOffset(
    CurveIndex - CurveDir->Entries[EntryIndex].StartIndex, &CurveOffset))
    return err;

  if (fseek(hTempFile, CurveOffset, SEEK_SET) != 0)
    return error(ERROR_SEEK, TempFileBuf);

  if (fread(&TmpCurvehdr, sizeof (CURVEHDR), 1, hTempFile) != 1)
    return error(ERROR_READ, TempFileBuf);

  CurPoints = TmpCurvehdr.pointnum;
  CurDataSz = TmpCurvehdr.DataType & 0x0F;
  CurCurveLen = CRVHDRLEN + ((ULONG)CurPoints * (ULONG)CurDataSz * 2);

  NewDataSz = DataType & 0x0F;
  NewCurveLen = CRVHDRLEN + ((ULONG)TotalPoints * (ULONG)NewDataSz * 2);

  if (NewCurveLen > CurCurveLen)
    {
    ULONG Dst, Src;

    /* resize the file */
    if (err = ChangeTempFileSize(TempFileSz + NewCurveLen))
      return err;

    Src = CurveOffset + CurCurveLen;
    Dst = CurveOffset + NewCurveLen;
    MoveLen = TempFileSz - (CurveOffset + CurCurveLen);

    /* move following curves up */
    if (err = MoveTempFileBlock(Dst, Src, MoveLen))
      return err;

    /* Move X points up */
    Src = CurveOffset + CRVHDRLEN + ((ULONG)CurPoints * (ULONG)CurDataSz);
    Dst = CurveOffset + CRVHDRLEN + ((ULONG)TotalPoints * (ULONG)NewDataSz);
    MoveLen = (ULONG)CurPoints * (ULONG)CurDataSz;

    /* move following curves up */
    if (err = MoveTempFileBlock(Dst, Src, MoveLen))
      return err;

    /* bookkeeping in directory structures: Correct the temp file pointers */
    for (i=0; i<CurveDir->BlkCount; i++)
      {
      if (CurveDir->Entries[i].TmpOffset > CurveOffset)
        CurveDir->Entries[i].TmpOffset += NewCurveLen - CurCurveLen;
      }
    TempFileSz += NewCurveLen - CurCurveLen;
    }
  else if (NewCurveLen < CurCurveLen)  /* shrinking curve */
    {
    ULONG Src, Dst;

    Src = CurveOffset + CRVHDRLEN + ((ULONG)CurPoints * (ULONG)CurDataSz);
    Dst = CurveOffset + CRVHDRLEN + ((ULONG)TotalPoints * (ULONG)NewDataSz);
    MoveLen = NewCurveLen -
                CRVHDRLEN -   /* X has no curve header */
                ((ULONG)TotalPoints *  /* just move the X data */
                  sizeof(ULONG)(NewDataSz));

    /* Move X points down */
    if (err = MoveTempFileBlock(Dst, Src, MoveLen))
      return err;

    /* move following curves down */
    Src = CurveOffset + CurCurveLen;
    Dst = CurveOffset + NewCurveLen;
    MoveLen = TempFileSz - (CurveOffset + CurCurveLen);

    if (err = MoveTempFileBlock(Dst, Src, MoveLen))
      return err;

    /* resize the file */
    if (err = ChangeTempFileSize(TempFileSz + NewCurveLen))
      return err;

    /* bookkeeping in directory structures */
    /* Correct the temp file pointers */
    for (i=0; i<CurveDir->BlkCount; i++)
      {
      if (CurveDir->Entries[i].TmpOffset > CurveOffset)
        CurveDir->Entries[i].TmpOffset -= CurCurveLen - NewCurveLen;
      }
    TempFileSz -= CurCurveLen - NewCurveLen;
    }

  /* change size and data type */
  TmpCurvehdr.DataType = DataType;
  TmpCurvehdr.pointnum = TotalPoints;

  if (fseek(hTempFile, CurveOffset, SEEK_SET))
    return error(ERROR_SEEK, TempFileBuf);

  if (fwrite(&TmpCurvehdr, sizeof (CURVEHDR), 1, hTempFile) != 1)
    return error(ERROR_WRITE, TempFileBuf);

  return ERROR_NONE;
}
  
static void GetCurveSourceComp(CURVEHDR *pCurvehdr, USHORT CurveIndex)
{
  int j = CurveIndex;

  get_Frame(&j);
  get_source_comp_point(&(pCurvehdr->scomp), j);
}

/* Return curve buffer number of a curve, or BUFNUM if not in a buffer */
USHORT IsHeaderInBuffer(CURVEDIR * CurveDir, USHORT EntryIndex, USHORT Curve)
{
  USHORT i;
  CURVEBUFFER * pBuf;
  CHAR EntryType = CurveDir->Entries[EntryIndex].EntryType;

  /* see if header is already in a buffer */
  for (i=0; i<BUFNUM; i++)
    {
    pBuf = &CvBuf[i];

    if ((pBuf->ActiveDir == CurveDir) &&       /* same dir */
        ((USHORT)pBuf->Entry == EntryIndex))   /* same entry */
      {
      if ((pBuf->CurveIndex == Curve) ||       /* same curve, or */
          ((EntryType == OMA4MEMDATA) &&       /* cvhdr points to mem curve */
         (pBuf->Curvehdr.CurveCount +          /* with multi-curve header */
          pBuf->CurveIndex >= Curve)))         /* which includes this curve */
        {
        return i;
        }
      }
    }
  return BUFNUM;
}

/***************************************************************************/
/*                                                                         */
/*  Function:  Read the curve header of a temporary curve. May be read     */
/*             from any buffer                                             */
/*                                                                         */
/*  Variables:                                                             */
/*             CurveDir - Input. Curve directory for curve entry           */
/*             EntryIndex - Input. Entry for curve block                   */
/*             CurveIndex - Input. File curve number to change.            */
/*             pCurvehdr - output.  Address of place to put curve header   */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_READ - temp file seek error                           */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: None                                                     */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA ReadTempCurvehdr(CURVEDIR *CurveDir, USHORT EntryIndex,
                         USHORT CurveIndex, CURVEHDR *pCurvehdr)
{
  USHORT i;
  ERR_OMA err;
  ULONG CurveOffset;

  if(EntryIndex >= CurveDir->BlkCount)
    return error(ERROR_BAD_CURVE_BLOCK, EntryIndex);

  // Test for CurveIndex too big to include the StartIndex
  //    of the curve entry.
  if(CurveIndex >=  CurveDir -> Entries[EntryIndex].count
      + CurveDir -> Entries[EntryIndex].StartIndex)
    {
    return error(ERROR_CURVE_NUM, CurveIndex);
    }
  
  /* see if header is already in a buffer */
  i = IsHeaderInBuffer(CurveDir, EntryIndex, CurveIndex);

  if (i < BUFNUM)
    {
    * pCurvehdr = CvBuf[i].Curvehdr;
    if (pCurvehdr->MemData)
       GetCurveSourceComp(pCurvehdr, CurveIndex);
    return ERROR_NONE;
    }          
  
  /* header not found in buffers */
  /* get the block's starting offset */

  CurveOffset = CurveDir->Entries[EntryIndex].TmpOffset;

  /* get the curve's offset. GetTempCvOffs also knows about memory curves */
  /* and if we are dealing with one, will return offset to 1st and only */
  /* curve header for the mem curve set */

  if (err = GetTempCurveOffset(CurveIndex -
    CurveDir->Entries[EntryIndex].StartIndex, &CurveOffset))
    return err;

  if (fread(pCurvehdr, sizeof (CURVEHDR), 1, hTempFile) != 1)
    return error(ERROR_READ, TempFileBuf);

  if (pCurvehdr->MemData)
    GetCurveSourceComp(pCurvehdr, CurveIndex);

  return ERROR_NONE;
}
  
/***************************************************************************/
/*                                                                         */
/*  Function:  Write the curve header of a temporary curve. May be written */
/*             from any buffer                                             */
/*                                                                         */
/*  Variables:                                                             */
/*             CurveDir - Input. Curve directory for curve entry           */
/*             EntryIndex - Input. Entry for curve block                   */
/*             CurveIndex - Input. File curve number to change.            */
/*             pCurvehdr - Input.  Pointer to curve header                 */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_READ - temp file seek error                           */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: None                                                     */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA WriteTempCurvehdr(CURVEDIR *CurveDir, USHORT EntryIndex,
                          USHORT CurveIndex, CURVEHDR *pCurvehdr)
{
  USHORT i;
  ERR_OMA err;
  ULONG CurveOffset;
 
  /* see if header is already in a buffer */


  /* see if header is already in a buffer */
  i = IsHeaderInBuffer(CurveDir, EntryIndex, CurveIndex);

  if (i < BUFNUM)
    {
    CvBuf[i].Curvehdr = * pCurvehdr;
    return ERROR_NONE;
    }
 
 /* header not found in buffers */
 
  /* get the block's starting offset */
  CurveOffset = CurveDir->Entries[EntryIndex].TmpOffset;
  /* get the curve's offset */
  if (err = GetTempCurveOffset(
     CurveIndex - CurveDir->Entries[EntryIndex].StartIndex, &CurveOffset))
     return err;
 
  if (fwrite(pCurvehdr, sizeof (CURVEHDR), 1, hTempFile) != 1)
     return error(ERROR_READ, TempFileBuf);
 
  return ERROR_NONE;
}

/***************************************************************************/
/*                                                                         */
/*  Function:  Add a curve from a file to the temporary curve file.        */
/*             Good for putting in curves to go with a newly created       */
/*             entry, only good for entries at the end of the block        */
/*                                                                         */
/*  Variables:                                                             */
/*             fhnd - Data file handle                                     */
/*             fName - file name corresponding to fhnd                     */
/*             CurveDir - Input. Curve directory for curve entry           */
/*             EntryIndex - Input. Entry for Source curve block.           */
/*             CurveIndex - Input. Source file curve number.               */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - file open error                                */
/*             ERROR_CLOSE - file close error                              */
/*             ERROR_READ - file seek error                                */
/*             ERROR_WRITE - file write error                              */
/*             ERROR_SEEK - file seek error                                */
/*             ERROR_CURVE_NUM - source or destination curve index is bad  */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Uses CvBuf[2]. Flushes and invalidates curve buffer 2.   */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA AppendFileCurveToTemp(FILE *fhnd, const char * fName,
                                     CURVEDIR *CurveDir,
                                     SHORT EntryIndex, USHORT CurveIndex)
{
  ERR_OMA err;
  ULONG SrcCurveOffset, DstCurveOffset, CurveLen;
  USHORT MoveLen, Points, PointsLeft;
  CURVE_ENTRY *pEntry;
  USHORT DataSz;
  UCHAR Version;
  
  /* check to see if need to flush buffer */
  if (CvBuf[2].ActiveDir)
     if (err = FlushCurveBuffer(2))
        return err;
  
  if (EntryIndex < 0 || EntryIndex >= (SHORT)CurveDir->BlkCount)
    return(error(ERROR_BAD_CURVE_BLOCK, EntryIndex));

  pEntry = &(CurveDir->Entries[EntryIndex]);
  
  /* Check for valid source curve */
  if ((CurveIndex > (pEntry->StartIndex + pEntry->count)) ||
      (CurveIndex < pEntry->StartIndex))
    {
    return error(ERROR_CURVE_NUM, CurveIndex);
    }

  DstCurveOffset = TempFileSz;
  
  if (CurveIndex == pEntry->StartIndex)
    pEntry->TmpOffset = DstCurveOffset;
  
  /* get the position curve's position in the data file */
  if (err = GetFileCurveOffset(fhnd, fName, CurveIndex, &SrcCurveOffset))
    return err;
  
  Version = IDFileType(fhnd);

  /* read the curve header */
  if (fseek(fhnd, SrcCurveOffset, SEEK_SET))
    return error(ERROR_SEEK, fName);

  if (Version >= OMA4V11DATA)             // older structure compatibility
    {
    if (err = readConvertCurveHeader1(& CvBuf[2].Curvehdr, fhnd))
      return err;
    }
  else
    {
    if (fread(&(CvBuf[2].Curvehdr), sizeof (CURVEHDR), 1, fhnd) != 1)
      return error(ERROR_READ, fName);
    }

  Points = CvBuf[2].Curvehdr.pointnum;
  DataSz = CvBuf[2].Curvehdr.DataType & 0x0F;
  CurveLen = sizeof(CURVEHDR) + ((ULONG) Points * (ULONG) DataSz * 2L);
  
  /* resize the temp data file */
  if (err = ChangeTempFileSize(TempFileSz + CurveLen))
     return err;
  
  /* write out the curve header to the temp file */
  if (fseek(hTempFile, DstCurveOffset, SEEK_SET) != 0)
    return error(ERROR_SEEK, TempFileBuf);

  if(fwrite(&(CvBuf[2].Curvehdr), sizeof(CURVEHDR), 1, hTempFile) != 1)
    return error(ERROR_WRITE, TempFileBuf);

  if (Version < 12)             // older structure compatibility
     SrcCurveOffset += curveHdr1Size();
  else
     SrcCurveOffset += sizeof(CURVEHDR);

  DstCurveOffset += sizeof(CURVEHDR);
  
  PointsLeft = Points;
  while (PointsLeft)
    {
    if (((ULONG) PointsLeft * (ULONG) DataSz) > (BUFLEN / 2))
      MoveLen = BUFLEN / 2;
    else
      MoveLen = PointsLeft * DataSz;
  
    /* fill up a buffer with the curve */
    /* get the Y data */
    if (fseek(fhnd, SrcCurveOffset, SEEK_SET) != 0)
      return error(ERROR_SEEK, fName);

    if (fread(CvBuf[2].BufPtr, MoveLen, 1, fhnd) != 1)
      return error(ERROR_READ, fName);

    /* get the X data */
    if (fseek(fhnd, SrcCurveOffset+((ULONG)DataSz * (ULONG)Points), SEEK_SET))
      return error(ERROR_SEEK, fName);

    if (fread((PVOID) ((ULONG)CvBuf[2].BufPtr + (BUFLEN / 2)),
               MoveLen, 1, fhnd) != 1)
      return error(ERROR_READ, fName);

    /* write out to the temporary data file */
    /* write out the Y data */

    if (fseek(hTempFile, DstCurveOffset, SEEK_SET) != 0)
      return error(ERROR_SEEK, TempFileBuf);

    if (fwrite(CvBuf[2].BufPtr, MoveLen, 1, hTempFile) != 1)
      return error(ERROR_WRITE, TempFileBuf);

    /* write the X data */
    if (fseek(hTempFile, DstCurveOffset + (DataSz * (ULONG)Points), SEEK_SET))
      return error(ERROR_SEEK, TempFileBuf);

    if (fwrite((PVOID) ((ULONG)CvBuf[2].BufPtr + (BUFLEN / 2)),
                MoveLen, 1, hTempFile) != 1)
      return error(ERROR_READ, TempFileBuf);
  
    PointsLeft -= MoveLen / DataSz;
    SrcCurveOffset += (ULONG) MoveLen;
    DstCurveOffset += (ULONG) MoveLen;
    }
  
  CvBuf[2].status = CVBUF_CLEAN;
  TempFileSz += CurveLen;
  
  return ERROR_NONE;
}
  
/***************************************************************************/
/*  Function:  Add a curve from a file to the temporary curve file.        */
/*                                                                         */
/*  Variables:                                                             */
/*             fhnd - Data file handle                                     */
/*             fName - file name of the Data file                          */
/*             CurveDir - Input. Curve directory for curve entry           */
/*             DstEntry - Input. Entry number for insertion.               */
/*             DstCurve - Input. Curve number for insertion.               */
/*             SrcCurve - Input. Source file curve number.                 */
/*             DstCurveOffset - Output. Curve's starting offset into temp  */
/*                                      file.                              */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - file open error                                */
/*             ERROR_CLOSE - file close error                              */
/*             ERROR_READ - file seek error                                */
/*             ERROR_WRITE - file write error                              */
/*             ERROR_SEEK - file seek error                                */
/*             ERROR_CURVE_NUM - source or destination curve index is bad  */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Uses CvBuf[2]. Flushes and invalidates curve buffer 2.   */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA InsertFileCurveInTemp(FILE *fhnd, const char * fName,
                                     CURVEDIR *CurveDir,
                                     SHORT DstEntry, USHORT DstCurve,
                                     USHORT SrcCurve,
                                     PULONG DstCurveOffset)
{
  return InsertMultiFileCurveInTemp(fhnd, fName, CurveDir, DstEntry,
                                    DstCurve, SrcCurve, DstCurveOffset, 1);
  
}
  
/***************************************************************************/
/*  Function:  Add a group of curves from a file to the temporary curve    */
/*             file.                                                       */
/*                                                                         */
/*  Variables:                                                             */
/*             fhnd - Data file handle                                     */
/*             fName - file name of the Data file                          */
/*             CurveDir - Input. Curve directory for curve entry           */
/*             DstEntry - Input. Entry number for insertion.               */
/*             DstCurve - Input. Curve number for insertion.               */
/*             SrcCurve - Input. Source file curve number.                 */
/*             DstCurveOffset - Output. Curve's starting offset into temp  */
/*                                      file.                              */
/*             CurveNum - Input. Number of curves to copy.                 */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - file open error                                */
/*             ERROR_CLOSE - file close error                              */
/*             ERROR_READ - file seek error                                */
/*             ERROR_WRITE - file write error                              */
/*             ERROR_SEEK - file seek error                                */
/*             ERROR_CURVE_NUM - source or destination curve index is bad  */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Uses CvBuf[2]. Flushes and invalidates curve buffer 2.   */
/*                                                                         */
/***************************************************************************/
ERR_OMA InsertMultiFileCurveInTemp(FILE *fhnd, const char * fName,
                                          CURVEDIR *CurveDir,
                                          SHORT DstEntry, USHORT DstCurve,
                                          USHORT SrcCurve,
                                          PULONG DstCurveOffset,
                                          USHORT CurveNum)
{
  ERR_OMA err;
  ULONG SrcCurveOffset;
  ULONG MoveSz;
  CURVE_ENTRY *pDstEntry;
  USHORT FileCurveCount;
  ULONG SrcBlkSz;
  USHORT i, j;
  UCHAR Version;
  
  if (CurveNum == 0)
    return ERROR_NONE;
  
  /* check to see if need to flush buffer */
  if (CvBuf[2].ActiveDir)
    if (err = FlushCurveBuffer(2))
      return err;
  
  pDstEntry = &(CurveDir->Entries[DstEntry]);
  
  if(err = GetFileCurveNum(fhnd, fName, & FileCurveCount))
     return err;
  
  if (CurveNum == (USHORT)-1)
   CurveNum = FileCurveCount;  /* MLM allow to load file if size unknown */

  Version = IDFileType(fhnd);

  if (SrcCurve + CurveNum > FileCurveCount)
    return error(ERROR_CURVE_NUM, SrcCurve + CurveNum);

  /* Check for valid Destination curve - Allows for insertion after */
  /*last curve of block, instead of naming next entry */
  /* Check to see if trying to insert past block; append to block is OK */

  if((DstCurve > (pDstEntry->StartIndex + pDstEntry->count)) ||
     (DstCurve < pDstEntry->StartIndex))
    {
    return error(ERROR_CURVE_NUM, DstCurve);
    }

  /* Make space for the insertion */
  *DstCurveOffset = pDstEntry->TmpOffset;
  
  /* CurveOffset goes in as the curve block's starting byte offset in the */
  /* temp file and comes out as the (CurveIndex) curve's byte offset into */
  /* the file. This function doesn't set file position for writing.       */
  if (err = GetTempCurveOffset(DstCurve - pDstEntry->StartIndex,
                                DstCurveOffset))
    return err;
  
  if (err = GetFileCurveOffset(fhnd, fName, SrcCurve, &SrcCurveOffset))
    return err;

  if (Version < OMA4V11DATA)
    {
    /* get the block size */
    if (err = GetCurveBlkSz(fhnd, fName, SrcCurveOffset, CurveNum,
                            & SrcBlkSz, TRUE))
      return err;
  
    /* resize the file */
    if (err = ChangeTempFileSize(TempFileSz + SrcBlkSz))
      return err;
  
    MoveSz = TempFileSz - *DstCurveOffset;
    /* move following curves up */
    if (err = MoveFileBlock(hTempFile, TempFileBuf,
                            * DstCurveOffset + SrcBlkSz,
                            hTempFile, TempFileBuf, * DstCurveOffset,
                            MoveSz))
      return err;

    TempFileSz += SrcBlkSz;
    /* bookkeeping in directory structures */
    /* Correct the temp file pointers */
    for (i=0; i<CurveDir->BlkCount; i++)
      {
      if ((CurveDir->Entries[i].TmpOffset >= *DstCurveOffset) &&
         ((SHORT)i != DstEntry))
         CurveDir->Entries[i].TmpOffset += SrcBlkSz;
      }
    /* move curves from file to tempfile */
    if (err = MoveFileBlock(hTempFile, TempFileBuf, * DstCurveOffset,
                            fhnd, fName, SrcCurveOffset, SrcBlkSz))
      return err;
    }
  else     // read in old format curves
    {
    CURVEHDR tCurvehdr;

    for (i=0; i<CurveNum; i++)
      {
      /* get the block size for each curve */
      if (err = GetCurveBlkSz(fhnd, fName, SrcCurveOffset, 1, & SrcBlkSz,
                              TRUE))
         return err;

      /* resize the file */
      if (err = ChangeTempFileSize(TempFileSz + SrcBlkSz))
         return err;

      MoveSz = TempFileSz - *DstCurveOffset;
      /* move following curves up */
      if (err = MoveFileBlock(hTempFile, TempFileBuf,
                              * DstCurveOffset + SrcBlkSz,
                              hTempFile, TempFileBuf, * DstCurveOffset,
                              MoveSz))
         return err;

      TempFileSz += SrcBlkSz;
      /* bookkeeping in directory structures */  
      /* Correct the temp file pointers */
      for (j=0; j<CurveDir->BlkCount; j++)
        {
        if ((CurveDir->Entries[j].TmpOffset >= *DstCurveOffset) &&
           ((SHORT)j != DstEntry))
           CurveDir->Entries[j].TmpOffset += SrcBlkSz;
        }

      if (fseek(fhnd, SrcCurveOffset, SEEK_SET) != 0)
        return error(ERROR_SEEK, fName);

      /* read the curve header */
      if (err = readConvertCurveHeader1(& tCurvehdr, fhnd))
        return err;

      if (fseek(hTempFile, *DstCurveOffset, SEEK_SET) != 0)
        return error(ERROR_SEEK, TempFileBuf);

      /* write the new curve header */
      if (fwrite(&tCurvehdr, sizeof (CURVEHDR), 1, hTempFile) != 1)
        return error(ERROR_READ, TempFileBuf);

      SrcBlkSz -= sizeof(CURVEHDR);    // already moved
      *DstCurveOffset += sizeof(CURVEHDR);
      SrcCurveOffset += curveHdr1Size();

      /* move curves from file to tempfile */
      if (err = MoveFileBlock(hTempFile, TempFileBuf, * DstCurveOffset,
                              fhnd, fName, SrcCurveOffset, SrcBlkSz))
        return err;

      *DstCurveOffset += SrcBlkSz;
      SrcCurveOffset += SrcBlkSz;
      }
    }
  CurveDir->CurveCount += CurveNum;
  pDstEntry->count += CurveNum;
  
  return ERROR_NONE;
}
  
/***************************************************************************/
/*                                                                         */
/*  Function:  Add a curve from a file to the temporary curve file.        */
/*                                                                         */
/*  Variables:                                                             */
/*             CurveDir - Input. Curve directory for curve entry           */
/*             EntryIndex - Input. Entry for Source curve block.           */
/*             CurveIndex - Output. Last source file curve number          */
/*                          successfully read.                             */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - file open error                                */
/*             ERROR_CLOSE - file close error                              */
/*             ERROR_READ - file seek error                                */
/*             ERROR_WRITE - file write error                              */
/*             ERROR_SEEK - file seek error                                */
/*             ERROR_CURVE_NUM - source or destination curve index is bad  */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects:                                                          */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA AppendFileCurveBlkToTemp(CURVEDIR *CurveDir, USHORT Entry,
                                        PUSHORT Curve)
{
   CURVE_ENTRY *pEntry;
   FILE *fhnd;
   CHAR Buf[FNAME_LENGTH];
   ERR_OMA err;
  
  if (Entry >= CurveDir->BlkCount)
    return(error(ERROR_BAD_CURVE_BLOCK, Entry));

   pEntry = &(CurveDir->Entries[Entry]);
   strcpy(Buf, pEntry->path);
   strcat(Buf, pEntry->name);
   if ((fhnd = fopen(Buf, "rb")) == 0)
    return error(ERROR_OPEN, Buf);

   pEntry->TmpOffset = TempFileSz;
  
   /* read all the curves in the block */
   for (*Curve=pEntry->StartIndex;
      *Curve < (pEntry->count + pEntry->StartIndex); (*Curve)++)
    {
    /* add the curve to the end of the temp data file */
    if (err = AppendFileCurveToTemp(fhnd, Buf, CurveDir, Entry, *Curve))
      {
      (*Curve)--;
      return err;
      }
    }
  if (fclose(fhnd))
    return error(ERROR_CLOSE, Buf);
  return ERROR_NONE;
}
  
/***************************************************************************/
/*  Function:  Insert a curve block from a file to the temporary curve     */
/*             file and insert a new block entry into the requested curve  */
/*             directory.                                                  */
/*                                                                         */
/*  Variables:                                                             */
/*             SrcCurveDir - Input. Curve directory for curve entry        */
/*             DstCurveDir - Input. Curve directory for curve entry        */
/*             DstEntry - Input. Entry number for insertion.               */
/*             DstCurve - Input. Curve number for insertion.               */
/*             SrcEntry - Input. Entry for Source curve block.             */
/*             SrcCurve - Input. Source file curve number.                 */
/*             CurveNum - Input. Number of curves to copy.                 */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - file open error                                */
/*             ERROR_CLOSE - file close error                              */
/*             ERROR_READ - file seek error                                */
/*             ERROR_WRITE - file write error                              */
/*             ERROR_SEEK - file seek error                                */
/*             ERROR_CURVE_NUM - source or destination curve index is bad  */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Uses CvBuf[2]. Flushes and invalidates curve buffer 2.   */
/*                Changes the sourcecurve block's temporary file offset.   */
/*                Will lose previous temporary block reference if a valid  */
/*                one was present.                                         */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA InsertFileBlkInTemp(CURVEDIR *SrcCurveDir,
                                   CURVEDIR *DstCurveDir,
                                   SHORT DstEntry, USHORT DstCurve,
                                   SHORT SrcEntry, USHORT SrcCurve,
                                   USHORT CurveNum)
{
   CURVE_ENTRY *pSrcEntry, *pDstEntry;
   CHAR Buf[FNAME_LENGTH];
   FILE *fhnd;
   ERR_OMA err;
   ULONG StartOffset, NewOffset;
  
   pSrcEntry = &(SrcCurveDir->Entries[SrcEntry]);
   pDstEntry = &(DstCurveDir->Entries[DstEntry]);
  
   /* if inserting in the middle of a curve block, the DstEntry must be */
   /* split up */
   if ((DstCurve > pDstEntry->StartIndex) &&
      (DstCurve < (pDstEntry->StartIndex + pDstEntry->count)))
   {
      /* split the block entry already present */
      if (err = SplitCurveBlk(DstEntry, DstCurve, DstCurveDir))
         return err;
      /* insert before the second half of split entry */
      DstEntry++;
   }
  
   NewOffset = DstCurveDir->Entries[DstEntry].TmpOffset;
   strcpy(Buf, pSrcEntry->path);
   strcat(Buf, pSrcEntry->name);
   fhnd = fopen(Buf, "rb");
   if (fhnd == NULL) {
      return error(ERROR_OPEN, Buf);
   }

   if (err = GetFileCurveOffset(fhnd, Buf, SrcCurve, &StartOffset))
      return err;
  
   /* add a new entry for the new curve block */
   if (err = InsertCurveBlkInDir(pSrcEntry->name, pSrcEntry->path,
                                 pSrcEntry->descrip, SrcCurve, StartOffset,
                                 0, &(pSrcEntry->time), DstCurveDir,
                                 DstEntry, pSrcEntry->EntryType))
      return err;
  
   /* the entry has been added, need to put in the offset into the temp */
   /* file */
   DstCurveDir->Entries[DstEntry].TmpOffset = NewOffset;
  
   /* DstEntry is now a copy of the SrcEntry */
   /* DstEntry + 1 is now the old DstEntry */
  
   err = InsertMultiFileCurveInTemp(fhnd, Buf, DstCurveDir, DstEntry,
                                     SrcCurve, SrcCurve, &NewOffset,
                                     CurveNum);
   if (fclose(fhnd))
      error(err = ERROR_CLOSE, Buf);
  
   if (err) 
      DelCurveBlocks(&DstEntry, 1, DstCurveDir);
  
   return err;
}
  
/***************************************************************************/
/*   ERR_OMA AddCurveSpaceToTempBlk(CURVEDIR *pCurveDir,          */
/*                                          SHORT EntryIndex,              */
/*                                          USHORT CurveIndex,             */
/*                                          USHORT CurveNum,               */
/*                                          CURVEHDR *pCurvehdr)         */
/*                                                                         */
/*  Function:  Puts space in the temp file to enlarge a curve block entry  */
/*             puts in the curve headers for the new curves. Data for the  */
/*             new curves is undefined                                     */
/*                                                                         */
/*  Variables:                                                             */
/*             pCurveDir - Input. Curve directory for curve entry          */
/*             EntryIndex - Input. Block entry number.                     */
/*             CurveIndex - Input. Insertion index for the new curves.     */
/*                          an index of 1 past last existing curve file    */
/*                          number will append new curves to block.        */
/*             CurveNum - Input. Number of curves to make room for.        */
/*             pCurvehdr - Input. array of curve headers.                  */
/*  Returns:                                                               */
/*             ERROR_OPEN - file open error                                */
/*             ERROR_CLOSE - file close error                              */
/*             ERROR_READ - file read error                                */
/*             ERROR_WRITE - file write error                              */
/*             ERROR_SEEK - file seek error                                */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Uses CvBuf[2]. Flushes and invalidates curve buffer 2.   */
/*                Changes the curve block's temporary file offset.         */
/*                Will lose previous temporary block reference if a valid  */
/*                one was present.                                         */
/*                                                                         */
/***************************************************************************/

ERR_OMA AddCurveSpaceToTempBlk(CURVEDIR *pCurveDir,
                                      SHORT EntryIndex,
                                      USHORT CurveIndex,
                                      USHORT CurveNum,
                                      CURVEHDR *pCurvehdr)
{
  CURVE_ENTRY *pEntry;
  USHORT i;
  ULONG InsertSize, CurveOffset, MoveSz;
  ERR_OMA err;

  if (EntryIndex < 0 || EntryIndex > (SHORT)pCurveDir->BlkCount)
    return(error(ERROR_BAD_CURVE_BLOCK, EntryIndex));

  pEntry = &(pCurveDir->Entries[EntryIndex]);

  if (CurveIndex > pEntry->StartIndex + pEntry->count)
    return error(ERROR_CURVE_NUM, CurveIndex);

  /* Calculate space needed to add new curves */
  InsertSize = 0;
  for (i=0; i<CurveNum; i++)         /* MLM changed to use only cvhdr[0] */
    {
    InsertSize += sizeof(CURVEHDR);
    InsertSize += (ULONG) (pCurvehdr[0].DataType & 0x0F) *
      (ULONG) pCurvehdr[0].pointnum * 2L;
    }

  /* resize the temp data file */
  if (err = ChangeTempFileSize(TempFileSz + InsertSize))
    return err;

  /* Make space for the insertion */
  CurveOffset = pEntry->TmpOffset;

  /* CurveOffset goes in as the curve block's starting byte offset in the */
  /* temp file and comes out as the (CurveIndex) curve's byte offset into */
  /* the file. This function doesn't set file position for writing.       */
  if (err = GetTempCurveOffset(CurveIndex - pEntry->StartIndex,
                               &CurveOffset))
    return err;

  MoveSz = TempFileSz - CurveOffset;
  /* move following curves up */
  if (err = MoveFileBlock(hTempFile, TempFileBuf, CurveOffset + InsertSize,
                          hTempFile, TempFileBuf, CurveOffset, MoveSz))
    return err;
  
  /* bookkeeping in directory structures */
  /* Correct the temp file pointers */
  for (i=0; i<pCurveDir->BlkCount; i++)
    {
    if ((pCurveDir->Entries[i].TmpOffset >= CurveOffset) &&
        ((SHORT) i != EntryIndex))
      pCurveDir->Entries[i].TmpOffset += InsertSize;
    }

  TempFileSz += InsertSize;
  pEntry->count += CurveNum;
  pCurveDir->CurveCount += CurveNum;
  for (i=0; i<CurveNum; i++)
    {
    CurveOffset = pEntry->TmpOffset;
    if (err = GetTempCurveOffset(CurveIndex + i, &CurveOffset))
      return err;

    /* write out the curve header to the temp file */
    if (fseek(hTempFile, CurveOffset, SEEK_SET) != 0)
      {
      return error(ERROR_SEEK, TempFileBuf);
      }
    /* MLM changed to only use curvehdr[0] */
    if (fwrite(&(pCurvehdr[0]), sizeof(CURVEHDR), 1, hTempFile) != 1)
      {
      return error(ERROR_WRITE, TempFileBuf);
      }
    }
  return err;
}
  
/***************************************************************************/
/*  Function:  Add a curve block entry and allocate space in the temporary */
/*             curve file for a new curve block                            */
/*             directory.                                                  */
/*                                                                         */
/*  Variables: pCurveDir - Input. Curve directory for curve entry          */
/*             pEntryIndex - Output. Block entry number.                   */
/*             Name - unique name for file ID                              */
/*             Path - File path                                            */
/*             Desc - description string for curve block                   */
/*             StartIndex - File curve number for starting curve of block  */
/*             StartOffset - Data File byte offset for start of curve block*/
/*             CurveNum - Input. Number of curves to copy.                 */
/*             pCurvehdr - Input. array of curve headers.                  */
/*             EntryType - OMA4DATA                                        */
/*             DisplayWindow - 0 for none or bitflag 1 to 1<<8             */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - file open error                                */
/*             ERROR_CLOSE - file close error                              */
/*             ERROR_READ - file read error                                */
/*             ERROR_WRITE - file write error                              */
/*             ERROR_SEEK - file seek error                                */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Uses CvBuf[2]. Flushes and invalidates curve buffer 2.   */
/*                Changes the curve block's temporary file offset.         */
/*                Will lose previous temporary block reference if a valid  */
/*                one was present.                                         */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA CreateTempFileBlk(CURVEDIR *pCurveDir,
                                 PSHORT pEntryIndex,
                                 PCHAR Name,
                                 PCHAR Path,
                                 PCHAR Desc,
                                 USHORT StartIndex,
                                 ULONG StartOffset,
                                 USHORT CurveNum,
                                 CURVEHDR *pCurvehdr,
                                 CHAR EntryType,
                                 USHORT DisplayWindow)
{
  time_t Time;
  TM *TimeStruct;
  ERR_OMA err = ERROR_NONE;
  CURVE_ENTRY *pEntry;
  USHORT i, Points;
  SHORT DataSz;
  ULONG CurveLen, OldFileSz;

  /* flush buffer 2 for use here */
  if (err = FlushCurveBuffer(2))
    return err;

  time(&Time);
  TimeStruct = localtime(&Time);

  err = AddCurveBlkToDir(Name, Path, Desc, StartIndex, StartOffset,
                         CurveNum, TimeStruct, pCurveDir, pEntryIndex,
                         EntryType, DisplayWindow);

  if (err) return err;

  pEntry = &(pCurveDir->Entries[*pEntryIndex]);

  pEntry->TmpOffset = TempFileSz;

  for (i=0; i<CurveNum; i++)
    {
    CvBuf[2].Curvehdr = pCurvehdr[0]; /* MLM change to only use cvhdr[0]*/

    Points = CvBuf[2].Curvehdr.pointnum;
    DataSz = CvBuf[2].Curvehdr.DataType & 0x0F;
    CurveLen = sizeof(CURVEHDR) + ((ULONG) Points * (ULONG) DataSz * 2L);

    OldFileSz = TempFileSz;
    /* resize the temp data file */
    if (err = ChangeTempFileSize(TempFileSz + CurveLen))
      return err;

    /* write out the curve header to the temp file */
    if (fseek(hTempFile, OldFileSz, SEEK_SET) != 0)
      return error(ERROR_SEEK, TempFileBuf);

    if (fwrite(&(CvBuf[2].Curvehdr), sizeof(CURVEHDR), 1, hTempFile) != 1)
      return error(ERROR_WRITE, TempFileBuf);

    TempFileSz += CurveLen;
    }
  return ERROR_NONE;
}
  
/***************************************************************************/
/*                                                                         */
/*   ERR_OMA DelTempFileBlk(CURVEDIR *pCurveDir,                  */
/*                                  SHORT pEntryIndex)                    */
/*                                                                         */
/*  Function:  Delete a complete curve block directory entry and its       */
/*             curves in the temporary curve file.                         */
/*                                                                         */
/*  Variables:                                                             */
/*             pCurveDir - Input. Curve directory for curve entry          */
/*             EntryIndex - Input. Block entry number.                     */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - file open error                                */
/*             ERROR_CLOSE - file close error                              */
/*             ERROR_READ - file read error                                */
/*             ERROR_WRITE - file write error                              */
/*             ERROR_SEEK - file seek error                                */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Uses CvBuf[2]. Flushes and invalidates curve buffer 2.   */
/*                Changes the curve block's temporary file offset.         */
/*                Will lose previous temporary block reference if a valid  */
/*                one was present.                                         */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA DelTempFileBlk(CURVEDIR *pCurveDir, SHORT EntryIndex)
{
  ERR_OMA err;
  CURVE_ENTRY *pEntry;

  if (EntryIndex < 0 || EntryIndex >= (SHORT)pCurveDir->BlkCount)
    return error(ERROR_BAD_CURVE_BLOCK, EntryIndex);

  pEntry = &(pCurveDir->Entries[EntryIndex]);

  if (err = DelMultiTempCurve(pCurveDir, EntryIndex, pEntry->StartIndex,
                             pEntry->count))
    return err;
  
  return DelCurveBlocks(&EntryIndex, 1, pCurveDir);
}
  
/***************************************************************************/
/*                                                                         */
/* ERR_OMA ReadDirToTemp(CURVEDIR *CurveDir, PUSHORT Entry,       */
/*                               PUSHORT Curve)                            */
/*                                                                         */
/*  Function:  Make the temporary up from the curves listed in CurveDir.   */
/*             Erases previous temp file and starts from scratch.          */
/*                                                                         */
/*  Variables:                                                             */
/*             CurveDir - Input. Curve directory.                          */
/*             Entry - Last entry completed                                */
/*             Curve - Last curve completed                                */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - file open error                                */
/*             ERROR_CLOSE - file close error                              */
/*             ERROR_READ - file seek error                                */
/*             ERROR_WRITE - file write error                              */
/*             ERROR_SEEK - file seek error                                */
/*             ERROR_CURVE_NUM - source or destination curve index is bad  */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects:                                                          */
/*                                                                         */
/***************************************************************************/
  
//ERR_OMA ReadDirToTemp(CURVEDIR *CurveDir, PUSHORT Entry,
//                              PUSHORT Curve)
//{
//   ERR_OMA err;
//  
//   /* empty the previous temp data file */
//   if (err = ChangeTempFileSize(0L))
//      return err;
//  
//   for (*Entry=0; (*Entry < CurveDir->BlkCount) && (!err); (*Entry)++)
//   {
//      if (CurveDir->Entries[*Entry].EntryType == OMA4DATA)
//         err = AppendFileCurveBlkToTemp(CurveDir, *Entry, Curve);
//   }
//  
//   *Entry -= 1;  /* last successful entry read */
//   return err;
//}
  
/***************************************************************************/
/*  Function:  Make a new directory entry and load the needed curves into  */
/*             the temporary data file.                                    */
/*                                                                         */
/*  Variables:                                                             */
/*             Name - file name to get curve block from                    */
/*             Path - file path to get curve block from                    */
/*             StartIndex - file curve number for start of block.          */
/*             Count - Number of curves to bring into block                */
/*             CurveDir - Input. Curve directory toi put entry into.       */
/*             pEntryIndex - Input. Requested index of new curve entry.    */
/*                           Output. Actual index of new curve entry       */
/*             pCurveIndex - Output. Last curve completed or -1 if error   */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_OPEN - file open error                                */
/*             ERROR_CLOSE - file close error                              */
/*             ERROR_READ - file seek error                                */
/*             ERROR_WRITE - file write error                              */
/*             ERROR_SEEK - file seek error                                */
/*             ERROR_CURVE_NUM - source or destination curve index is bad  */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects:                                                          */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA ReadFileToTemp(PCHAR Name, PCHAR Path,
                              USHORT StartIndex, USHORT Count,
                              CURVEDIR *CurveDir, USHORT *pEntryIndex,
                              USHORT *pCurveIndex)
{
  ERR_OMA err;
  CURVE_ENTRY *pEntry;
  CHAR FullFileName[FNAME_LENGTH];
  FILE *fhnd;
  ULONG TempOffset;
  SHORT TempIndex;

  *pCurveIndex = NOT_FOUND;

  if (*pEntryIndex >= CurveDir->BlkCount)
    {
    TempOffset = TempFileSz;
    *pEntryIndex = CurveDir->BlkCount;
    }
  else
    TempOffset = CurveDir->Entries[*pEntryIndex].TmpOffset;

  if (err = FillDescFieldsFromFile(CurveDir, Name, Path, pEntryIndex))
    return err;

  pEntry = &(CurveDir->Entries[*pEntryIndex]);

  pEntry->TmpOffset = TempOffset;

  strcpy(FullFileName, pEntry->path);

  /* add the file name to the path */
  strcat(FullFileName, pEntry->name);
  if ((pEntry->EntryType == OMA4DATA) || (pEntry->EntryType == OMA4V11DATA))
    {
    fhnd = fopen(FullFileName, "rb");
    if (fhnd == NULL)
      {
      DelCurveBlksFromDir(pEntryIndex, 1, CurveDir);
      return error(ERROR_OPEN, FullFileName);
      }

    pEntry->StartIndex = StartIndex;

    if (Count == 0)
      return ERROR_NONE;

    if (err = GetFileCurveOffset(fhnd, FullFileName, StartIndex,
                                 &(pEntry->StartOffset)))
      {
      DelCurveBlksFromDir(pEntryIndex, 1, CurveDir);
      return err;
      }

    fclose(fhnd);

    if (err = InsertFileBlkInTemp(CurveDir, CurveDir,
                                  (SHORT) *pEntryIndex, StartIndex,
                                  (SHORT) *pEntryIndex, StartIndex,
                                  Count))
      DelCurveBlksFromDir(pEntryIndex, 1, CurveDir);
    else
      {
      *pCurveIndex = StartIndex + Count;
      TempIndex = *pEntryIndex + 1;
      DelCurveBlksFromDir(&TempIndex, 1, CurveDir);
      }

    return err;
    }
  else
    {
    DelCurveBlksFromDir(pEntryIndex, 1, CurveDir);
    return error(ERROR_IMPROPER_FILETYPE, FullFileName);
    }
}
  
/***************************************************************************/
/*  Function:  Write a curve block from its temporary curve file to its    */
/*             named file.  The file will be created if not already        */
/*             present.                                                    */
/*                                                                         */
/*  Variables:                                                             */
/*             fhnd - Input. file handle for entry's original source file. */
/*             fName - file name of entry's original source file.          */
/*             CurveDir - Input. Curve directory for curve entry           */
/*             Entry - Input. Entry for Source curve block.                */
/*             DstCurve - Input. file curve index to write to.             */
/*             SrcCurve - Input. Block entry file number of curve to write */
/*                               from                                      */
/*             Count - Number of curves.                                   */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_READ - file seek error                                */
/*             ERROR_WRITE - file write error                              */
/*             ERROR_SEEK - file seek error                                */
/*             ERROR_CURVE_NUM - source or destination curve index is bad  */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Uses CvBuf[2]. Flushes and invalidates curve buffer 2.   */
/*                Will overwrite data in existing files.                   */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA InsertTempCurvesInFile(FILE *fhnd, const char * fName,
                                      CURVEDIR *CurveDir,
                                      USHORT SrcEntry, USHORT SrcCurve,
                                      USHORT DstCurve, USHORT Count,
                                      USHORT FileCurveCount)
{
  ERR_OMA err;
  ULONG DstOffset, SrcOffset, SrcBlkSz;
  ULONG  FileSz, MoveSz;
  struct stat Buffer;
  CURVE_ENTRY *pSrcEntry;

  if (Count == 0) return ERROR_NONE; /* nothing to do */

  if (DstCurve > FileCurveCount)
    return error(ERROR_CURVE_NUM, DstCurve);

  pSrcEntry = &(CurveDir->Entries[SrcEntry]);

  /* Check for valid curves */
  if((SrcCurve + Count) > (pSrcEntry->StartIndex + pSrcEntry->count))
    return error(ERROR_CURVE_NUM, SrcCurve + Count);
  
  if(SrcCurve < pSrcEntry->StartIndex)
    return error(ERROR_CURVE_NUM, SrcCurve);

  if (fstat(fileno(fhnd), &Buffer))
    return error(ERROR_READ, fName);

  FileSz = Buffer.st_size;

  /* flush all temporary buffers */
  if(err = clearAllCurveBufs())
    return err;

  if (err = GetFileCurveOffset(fhnd, fName, DstCurve, &DstOffset))
    return err;

  /* Make space for the insertion */
  SrcOffset = pSrcEntry->TmpOffset;

  /* CurveOffset goes in as the curve block's starting byte offset in the */
  /* temp file and comes out as the (CurveIndex) curve's byte offset into */
  /* the file. This function doesn't set file position for writing.       */

  if (err = GetTempCurveOffset(SrcCurve - pSrcEntry->StartIndex, &SrcOffset))
    return err;

  /* get the block size */
  if (err = GetCurveBlkSz(hTempFile, TempFileBuf, SrcOffset, Count,
                          & SrcBlkSz, FALSE))
     return err;

  /* resize the file */
  if (chsize(fileno(fhnd), FileSz + SrcBlkSz) == -1)
    return error(ERROR_WRITE, fName);

  MoveSz = FileSz - DstOffset;

  /* move following curves up */
  if (err = MoveFileBlock(fhnd, fName, DstOffset + SrcBlkSz, fhnd, fName,
                          DstOffset, MoveSz))
    return err;
  
  /* move curves from tempfile to file */
  if (pSrcEntry->EntryType == OMA4MEMDATA)
    {
    return DataFileWrite(fhnd, fName, DstOffset, CurveDir, SrcEntry,
                         SrcCurve, Count);
    }
  else
    {
    return MoveFileBlock(fhnd, fName, DstOffset, hTempFile,
                         TempFileBuf, SrcOffset, SrcBlkSz);
    }
}

// Move all of one curve to another if they are the same size.
// Return TRUE iff a replacement was actually done.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN ReplaceCurve(CURVEDIR *CurveDir, SHORT SrcEntryIndex,
                      USHORT SrcCurveIndex, SHORT DstEntryIndex,
                      USHORT DstCurveIndex)
{
  CURVE_ENTRY *pSrcEntry = & CurveDir->Entries[ SrcEntryIndex ];
  CURVE_ENTRY *pDstEntry = & CurveDir->Entries[ DstEntryIndex ];
  ULONG DstCurveOffset = pDstEntry->TmpOffset;
  ULONG SrcCurveOffset = pSrcEntry->TmpOffset;
  ULONG DstBlkSz;
  ULONG SrcBlkSz;
  ERR_OMA err;

  // can only replace a curve that already exists
  if(DstCurveIndex < pDstEntry->StartIndex ||
     DstCurveIndex >= pDstEntry->StartIndex + pDstEntry->count)
    {
    error(ERROR_CURVE_NUM, DstCurveIndex);
    return FALSE;
    }

  if(err = GetTempCurveOffset(DstCurveIndex - pDstEntry->StartIndex,
                              & DstCurveOffset))
    return FALSE;

  //ASSERT : DstCurveOffset is now the curve's byte offset into the file

  if(err = GetTempCurveOffset(SrcCurveIndex - pSrcEntry->StartIndex,
                              & SrcCurveOffset))
    return FALSE;

  //ASSERT : SrcCurveOffset is now the curve's byte offset into the file

  if(err = GetCurveBlkSz(hTempFile, TempFileBuf, SrcCurveOffset, 1,
                         & SrcBlkSz, FALSE))
    return FALSE;

  if(err = GetCurveBlkSz(hTempFile, TempFileBuf, DstCurveOffset, 1,
                         & DstBlkSz, FALSE))
    return FALSE;

  if(SrcBlkSz != DstBlkSz)
    return FALSE;

  if(pSrcEntry->EntryType == OMA4MEMDATA || pDstEntry->EntryType == OMA4MEMDATA)
    err = TempFileWrite(hTempFile,
                        TempFileBuf,
                        DstCurveOffset,
                        CurveDir,
                        SrcEntryIndex,
                        SrcCurveIndex,
                        DstCurveIndex,
                        1,
                        pDstEntry->EntryType);
  else
    err = MoveFileBlock(hTempFile, TempFileBuf, DstCurveOffset,
                        hTempFile, TempFileBuf,
                        SrcCurveOffset, SrcBlkSz);
  return ! err;   
}

/****************************************************************************/
ERR_OMA GetWholeCurveSetSz(USHORT BlkIndex, USHORT Start, USHORT Count,
                                  PULONG pBlkSz, BOOLEAN XcludMemData)
{
  ERR_OMA err;
  ULONG CurveByteOffset;
  CURVE_ENTRY *pEntry;

  if (BlkIndex >= MainCurveDir.BlkCount)
    return(error(ERROR_BAD_CURVE_BLOCK, BlkIndex));

  pEntry = &MainCurveDir.Entries[BlkIndex];
  CurveByteOffset = pEntry->TmpOffset;
  err = GetTempCurveOffset(pEntry->StartIndex + Start, &CurveByteOffset);

  if (!err)
    err = GetCurveBlkSz(hTempFile, TempFileBuf, CurveByteOffset, Count,
                       pBlkSz, XcludMemData);
  return err;
}
