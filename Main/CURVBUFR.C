/***************************************************************************/
/*                                                                         */
/*  curvbufr.c                                                             */
/*                                                                         */
/*  copyright (c) 1988, EG&G Instruments Inc.                              */
/*                                                                         */
/*    Curve buffers will be kept in normal memory to try to speed disk     */
/*    access.                                                              */
/*
/  $Header:   J:/logfiles/oma4000/main/curvbufr.c_v   1.3   06 Jul 1992 10:27:30   maynard  $
/  $Log:   J:/logfiles/oma4000/main/curvbufr.c_v  $
/
/***************************************************************************/
  
#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#include "curvbufr.h"
#include "tempdata.h"
#include "points.h"
#include "oma4driv.h"
#include "detsetup.h"
#include "syserror.h"  // ERROR_READ
#include "omaerror.h"
#include "live.h"      // sub_background()
#include "calib.h"
#include "omameth.h"   // InitialMethod
#include "crventry.h"
#include "curvedir.h"
#include "spgraph.h"   // DataPtToChannel  

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

/* 3 curve buffers, 2 for possible operands and one for results */
CURVEBUFFER CvBuf[BUFNUM+1];
static FLOAT CalXBuf[BUFFER_MAX_POINTS];
/* save number of points in X Cal data buffer for live */
static USHORT LastPointnum = 0;

/***************************************************************************/
/*                                                                         */
/* ERR_OMA InitCurveBuffer(VOID)                                  */
/*                                                                         */
/*  function:  Initialize the curve memory buffer for the temporary data   */
/*             file (named by TempFile). Uses the environment variable     */
/*             TMP to set the path for TempFile.  Should be                */
/*             called at program initialization                            */
/*                                                                         */
/*  Variables:                                                             */
/*                                                                         */
/*  returns:   ERROR_OPEN - File open error                                */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects:                                                          */
/*                Opens the temporary data file                            */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA InitCurveBuffer()
{
  SHORT i;
  static char panicTmpName[] = "c:\\OMA1";
  
  for (i=0; i<BUFNUM+1; i++)
    {
    CvBuf[i].BufPtr = malloc(BUFLEN);
    if (CvBuf[i].BufPtr == NULL)
      return error(ERROR_ALLOC_MEM);   
    CvBuf[i].ActiveDir = NULL; /* Nothing loaded into buffer yet */
    }
  
  TempFileBuf = tempnam("\\", "OMA");

  if (TempFileBuf == NULL)
   TempFileBuf = panicTmpName;

  /* open new file for writing and reading, erases previous file */
  hTempFile = fopen(TempFileBuf, "w+b");
  if (hTempFile == NULL)
     return ERROR_OPEN;
  
  return ERROR_NONE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA clearLiveCurveBufs(void)
{
  USHORT i;
  ERR_OMA err;
  CURVEDIR * pCvDir;

  for(i = 0; i < BUFNUM; i ++)
    {
    pCvDir = CvBuf[i].ActiveDir;
    if (pCvDir && pCvDir->Entries[CvBuf[i].Entry].EntryType == OMA4MEMDATA)
      if (err = FlushCurveBuffer(i))
      return err;
    }
  return ERROR_NONE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA clearAllCurveBufs(void)
{
   USHORT i;
   ERR_OMA err;

   for(i = 0; i < BUFNUM; i ++)
      if(err = FlushCurveBuffer(i))
         return err;

   return ERROR_NONE;
}

/***************************************************************************/
/*                                                                         */
/* ERR_OMA ReleaseCurveBuffer(VOID)                               */
/*                                                                         */
/*  function:  Dealloc the curve memory buffers, erase the temporary       */
/*             data file (named by TempFileBuf), deallocate TempFileBuf.   */
/*             Should be on program exit                                   */
/*                                                                         */
/*  Variables:                                                             */
/*                                                                         */
/*  returns:   ERROR_CLOSE - file close error                              */
/*             WRITEERR - File delete error                                */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects:                                                          */
/*                Deletes temporary data file                              */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA ReleaseCurveBuffer()
{
   SHORT i;
   ERR_OMA err;
  
   for (i=0; i<BUFNUM; i++)
   {
      /* flush the buffer first just in case file recovery is attempted */
      /* after deletion */
      if (err = FlushCurveBuffer(i))
         return err;
      free(CvBuf[i].BufPtr);
   }
  
   /* open new file for writing and reading, erases previous file */
   if (fclose(hTempFile)) {
      return error(ERROR_CLOSE, TempFileBuf);
   }
  
   if (remove(TempFileBuf)) {
      return error(ERROR_WRITE, TempFileBuf);
   }
   free(TempFileBuf);
   return ERROR_NONE;
  
}
 
/***************************************************************************/
/*                                                                         */
/* ERR_OMA FlushCurveBuffer(SHORT BufferNum)                      */
/*                                                                         */
/*  Function:  Flush the curve memory buffer out to the temp data file     */
/*                                                                         */
/*  Variables:                                                             */
/*             BufferNum - Input. Buffer number to write. 0..2             */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_READ - temp file seek error                           */
/*             ERROR_WRITE - temp file write error                         */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Changes file pointer. Marks buffer as emptied.           */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA FlushCurveBuffer(SHORT BufferNum)
{
   USHORT DataSz;
   ULONG PointOffset, FirstPoint, WriteSz;
   FLOAT * XPtr;
   ERR_OMA err;
   CURVEBUFFER *pBuf;
   LPCURVE_ENTRY pEntry;

   pBuf = &(CvBuf[BufferNum]);
  
   // see if buffer is active or clean
   if (pBuf->ActiveDir == 0) 
    {
    pBuf->status = CVBUF_CLEAN;
    return ERROR_NONE;       // buffer is not active
    }
  
   if (pBuf->status == CVBUF_CLEAN)
    {
    pBuf->ActiveDir = 0; 
    return ERROR_NONE;       // buffer is clean
    }

   pEntry = &((pBuf->ActiveDir)->Entries[pBuf->Entry]);
   pBuf->ActiveDir = 0;

   PointOffset = pEntry->TmpOffset;
  
   /* PointOffset goes in as the curve block's starting byte offset in the */
   /* temp file and comes out as the (CurveIndex) curve's byte offset into */
   /* the file. This function doesn't set file position for writing.       */

   if (err = GetTempCurveOffset(pBuf->CurveIndex - pEntry->StartIndex,
                                 &PointOffset))
      return err;
  
   /* Set the file pointer to the start of the curve */
   if (fseek(hTempFile, PointOffset, SEEK_SET) != 0)
    {
      return error(ERROR_SEEK, TempFileBuf);
   }

   /* write out the curve header */
   if (fwrite(&(pBuf->Curvehdr), sizeof (CURVEHDR), 1, hTempFile) != 1)
    {
      return error(ERROR_WRITE, TempFileBuf);
   }

   DataSz = pBuf->Curvehdr.DataType & 0x0F;
   FirstPoint = pBuf->BufferOffset;
   /* calculate the number of bytes from the start of the file to the start */
   /* of the buffered Y data pointblock */
   PointOffset += sizeof(CURVEHDR) + (FirstPoint * (ULONG) DataSz);
  
   WriteSz = (ULONG) (pBuf->Curvehdr.pointnum - (USHORT) FirstPoint)
             * (ULONG) DataSz;
   if (WriteSz > (BUFLEN / 2))
      WriteSz = (BUFLEN / 2);

   if(pEntry->EntryType == OMA4MEMDATA)
    {
    if (pBuf->Curvehdr.DataType != LONGTYPE)
     {
     return error(ERROR_READ_CURVE_HEADER);
     }

      WriteCurveToMem(pBuf->BufPtr, (USHORT)WriteSz, pBuf->CurveIndex);

      XPtr = (PFLOAT)((ULONG) pBuf->BufPtr + (ULONG) (BUFLEN / 2));
      memcpy(CalXBuf + FirstPoint, XPtr,
             sizeof(FLOAT) * pBuf->Curvehdr.pointnum);
      pBuf->status = CVBUF_CLEAN;
      return ERROR_NONE;
   }
   else
   {
      /* Set the file pointer to the start of the Y data */
      if (fseek(hTempFile, PointOffset, SEEK_SET) != 0)
        {
         return error(ERROR_SEEK, TempFileBuf);
      }

      /* write the Y data points */
      if (fwrite(pBuf->BufPtr, (USHORT) WriteSz, 1, hTempFile) != 1) {
         return error(ERROR_WRITE, TempFileBuf);
      }

      /* set file pointer to the start of the X data */
      PointOffset += (ULONG) pBuf->Curvehdr.pointnum * (ULONG) DataSz;
      if (fseek(hTempFile, PointOffset, SEEK_SET) != 0) {
         return error(ERROR_SEEK, TempFileBuf);
      }

      /* write out the X data points */
      if (fwrite((PVOID) ((ULONG)pBuf->BufPtr + (BUFLEN / 2)),
                  (USHORT) WriteSz, 1, hTempFile) != 1) {
         return error(ERROR_WRITE, TempFileBuf);
      }
   }
   pBuf->status = CVBUF_CLEAN;
   return ERROR_NONE;
}
 
/***************************************************************************/
/*  Function:  Load a memory buffer with contents of a curve from the      */
/*             temporary curve file                                        */
/*                                                                         */
/*  Variables:                                                             */
/*             CurveDir - Input. Curve directory for curve entry           */
/*             EntryIndex - Input. Entry for curve block                   */
/*             CurveIndex - Input. File curve number to read.              */
/*             FirstPoint - Input. Requested first point of buffer curve   */
/*                                 portion                                 */
/*                          Output. Actual first point of read-in curve    */
/*                                  buffer portion.                        */
/*             BufferNum - Input. Buffer number to read curve into. 0..2   */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_READ - temp file seek error                           */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_POINT_NUM - point index not in curve                  */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Flushes out previous buffer contents.  Changes file      */
/*                pointer and CvBuf fields.                                */
/*                                                                         */
/*  When a point is requested via GetDataPoint or SetDataPoint, those      */
/*  functions check to see if the point is in a curve which is already     */
/*  loaded into a buffer.  This test is repeated here, though, for those   */
/*  functions (math functions) which call LoadCurveBuffer directly.  The   */
/*  performance hit is accepted since it only applies to curves which      */
/*  were not found by Get/SetDataPoint and so must be read from disk any-  */
/*  way.                                                                   */
/*                                                                         */
/*  The caller supplied SHORT *Buffer points to a buffer which the caller  */
/*  would prefer to use.  If the curve and point are not already in a buf- */
/*  fer, this buffer is used. If *Buffer is -1, then the first unused buf- */
/*  fer (starting at 0) is used instead.                                   */
/*                                                                         */
/***************************************************************************/

ERR_OMA LoadCurveBuffer(LPCURVEDIR CurveDir, SHORT EntryIndex,
                               USHORT CurveIndex, PUSHORT FirstPoint,
                               SHORT *BufferNum)
{
  SHORT Frame, i;
  USHORT DataSz;
  LONG TempPt, MinFirstPoint;
  ULONG PointOffset, ReadSz;
  BOOLEAN HeaderOk = FALSE;
  CURVEBUFFER *pBuf;
  LPCURVE_ENTRY pEntry;
  static SHORT LastEntry = -1;
  static USHORT LastCurve = 0;
  static USHORT LastPoint = 0;
  ERR_OMA err;


  /* Some assembler functions use incorrect type comparisons. This was   */
  /* discovered (by mlm) during final debug of last release (3.1). I     */
  /* didn't write the assembler and I decided that rather than change it,*/
  /* I will perform a test here. I believe that in addition to using the */
  /* correct types, I would have to add another comparison to the assem- */
  /* bly code for complete safety.  Adding it here means the program     */
  /* spends less time doing the comparisons, and there is less chance of */
  /* me introducing a bug due to my limited familiarity with 80x86 ASM.  */
  
  if (*FirstPoint > 0x8000)
    return error(ERROR_POINT_INDEX, *FirstPoint);

  if(((*BufferNum > BUFNUM) && (*BufferNum != LIVE_XFER_BUF)))
    return error(ERROR_READ_CURVE_HEADER);

  /* see if curve is already in a buffer */
  /* refer to comments above */

  for (i = 0; i < BUFNUM; i++)
    {
    pBuf = &CvBuf[i];
    if((pBuf->ActiveDir == CurveDir) && (pBuf->Entry == EntryIndex) &&
       (pBuf->CurveIndex == CurveIndex))
      {
      *BufferNum = i;
      HeaderOk = TRUE;
      }
    }
  
  if (*BufferNum == -1) /* no buffer specified, look for free buffer */
    {
    int i;

    for (i = 0; i < BUFNUM; i++)
      {
      if (CvBuf[i].ActiveDir == NULL)
        {
        *BufferNum = i;
        break;
        }
      }
    if (*BufferNum == -1)
      *BufferNum = 0;
    }

  pBuf = &(CvBuf[*BufferNum]);

  if (EntryIndex < 0 || EntryIndex >= (SHORT)CurveDir->BlkCount)
    return(error(ERROR_BAD_CURVE_BLOCK, EntryIndex));

  pEntry = &(CurveDir->Entries[EntryIndex]);

  if (CurveIndex >= pEntry->count + pEntry->StartIndex)
    return error(ERROR_CURVE_NUM, CurveIndex);

  /* write out previous buffer's contents */
  if (err = FlushCurveBuffer(*BufferNum))
    return err;
  
  PointOffset = pEntry->TmpOffset;
  
  /* PointOffset goes in as the curve block's starting byte offset in the */
  /* temp file and comes out as the (CurveIndex) curve's byte offset into */
  /* the file. This function also sets file position for read at the      */
  /* curve's start.                                                       */
  
  if (err = GetTempCurveOffset(CurveIndex - pEntry->StartIndex,
                               &PointOffset))
    return err;
  
  if (!HeaderOk)
    {
    /* read in the curve header */
    if(fread(&(pBuf->Curvehdr), sizeof (CURVEHDR), 1, hTempFile) != 1)
      {
      return error(ERROR_READ, TempFileBuf);
      }
    }

  DataSz = pBuf->Curvehdr.DataType & 0x0F;
  if (!DataSz)
    return error(ERROR_READ_CURVE_HEADER);
  
  /* check to see if going backwards */
  if ((EntryIndex == LastEntry) && (CurveIndex == LastCurve))
    {
    if (*FirstPoint < LastPoint)
      {
      /* overshoot by 1/2 buffer size */

      TempPt = (LONG)*FirstPoint - ((LONG)(BUFLEN / 4) / (LONG) DataSz);
      if (TempPt < 0)
        *FirstPoint = 0;
      else
        *FirstPoint = (USHORT)TempPt;
      }
    }

  LastEntry = EntryIndex;
  LastCurve = CurveIndex;
  LastPoint = *FirstPoint;

  /* If the desired 1st point is >= MinFirstPoint, then the */
  /* desired part of the curve fits in the buffer */

  MinFirstPoint = (LONG) pBuf->Curvehdr.pointnum - ((BUFLEN / 2) / DataSz);
  if (MinFirstPoint < 0L) MinFirstPoint = 0L;
  if (*FirstPoint > (USHORT) MinFirstPoint)
    *FirstPoint = (USHORT) MinFirstPoint;

  /* calculate the number of bytes from the start of the file to the start */
  /* of the requested Y data pointblock */
  PointOffset += sizeof(CURVEHDR) + ((ULONG) *FirstPoint * (ULONG) DataSz);

  ReadSz =
    (ULONG)(pBuf->Curvehdr.pointnum - (USHORT)*FirstPoint) * (ULONG)DataSz;
  if (ReadSz > (BUFLEN / 2)) ReadSz = (BUFLEN / 2);

  if (pEntry->EntryType == OMA4MEMDATA)
    {
    ReadCurveFromMem(pBuf->BufPtr, (USHORT)ReadSz, CurveIndex);

    LoadCalibratedX(pBuf, pBuf->Curvehdr.pointnum);

    sub_background(pBuf->BufPtr, CurveIndex, &pBuf->Curvehdr);
    Frame = CurveIndex;
    get_Frame(&Frame);
    get_source_comp_point(&(pBuf->Curvehdr.scomp), Frame);
    }
  else
    {
    /* Set the file pointer to the start of the Y data */
    if (fseek(hTempFile, PointOffset, SEEK_SET) != 0)
      return error(ERROR_SEEK, TempFileBuf);

    /* read in the Y data points */
    if (fread(pBuf->BufPtr, (USHORT) ReadSz, 1, hTempFile) != 1)
      return error(ERROR_READ, TempFileBuf);

    /* set file pointer to the start of the X data */
    PointOffset += (ULONG) pBuf->Curvehdr.pointnum * (ULONG) DataSz;
    if (fseek(hTempFile, PointOffset, SEEK_SET) != 0)
      return error(ERROR_SEEK, TempFileBuf);

    /* read in the X data points */
    if (fread((PVOID) ((ULONG)pBuf->BufPtr + (BUFLEN / 2)), (USHORT) ReadSz,
              1, hTempFile) != 1)
      return error(ERROR_READ, TempFileBuf);

    pBuf->status = CVBUF_CLEAN;
    }

  /* set the buffer info to correct values */

  pBuf->Curvehdr.XData.XArray = (PVOID)((ULONG) pBuf->BufPtr + (ULONG)(BUFLEN / 2));
  pBuf->BufferOffset = *FirstPoint;
  pBuf->ActiveDir = CurveDir;
  pBuf->Entry = EntryIndex;
  pBuf->CurveIndex = CurveIndex;
  
  return ERROR_NONE;
}

/**********************************************************************/
/*                                                                    */
/* Fills XData part of a curvebuffer using calibration coefficients   */
/* pointed to by XCal.                                                */
/*                                                                    */
/**********************************************************************/

void LoadCalibratedX(CURVEBUFFER *pBuf, USHORT PointNum)
{
  PFLOAT XPtr;

  if ((PointNum * sizeof(FLOAT)) > (BUFLEN/2))
    {
    error(ERROR_POINT_NUM, PointNum);
    PointNum = ((BUFLEN/2) / sizeof(FLOAT));
    }

  XPtr = (PFLOAT)((ULONG) pBuf->BufPtr + (ULONG) (BUFLEN / 2));

  memcpy(XPtr, CalXBuf + pBuf->BufferOffset, sizeof(FLOAT) * PointNum);

  if (XPtr[0] <= XPtr[PointNum-1])
    {
    pBuf->Curvehdr.Xmin = XPtr[0];
    pBuf->Curvehdr.Xmax = XPtr[PointNum-1];
    }
  else
    {
    pBuf->Curvehdr.Xmax = XPtr[0];
    pBuf->Curvehdr.Xmin = XPtr[PointNum-1];
    }
}

/**********************************************************************/
/*                                                                    */
/* Fills XData part of a curvebuffer using calibration coefficients   */
/* pointed to by XCal. Used by Live when creating LastLive.           */
/*                                                                    */
/**********************************************************************/

void GenXData(USHORT PointNum, PFLOAT XCal)
{
  USHORT i;

  if ((PointNum * sizeof(FLOAT)) > (BUFLEN/2))
    {
    error(ERROR_POINT_NUM, PointNum);
    PointNum = ((BUFLEN/2) / sizeof(FLOAT));
    }

  LastPointnum = PointNum;

  for(i=0; i < PointNum; i ++)
    CalXBuf[i] = ApplyCalibrationToX(XCal, (FLOAT)i);
}

/**********************************************************************/
/* Fill XData part of existing lastlive with new calibrated X Data    */
/* Used by Calib routines                                             */
/**********************************************************************/
void RegenXData(PFLOAT XCal)
{
  GenXData(LastPointnum, XCal);
}

/**********************************************************************/
/* Get the Min and Max X values for live data.                        */
/*                                                                    */
/**********************************************************************/
void GetXMinMax(FLOAT * XMin, FLOAT * XMax)
{
  *XMin = CalXBuf[0];
  *XMax = CalXBuf[LastPointnum-1];
}

#ifdef DEBUG   // print curve buffer function for debugging use only

/***************************************************************************/
/*  VOID print_tempbuf(CURVEBUFFER FAR *pBuf)                                      */
/*                                                                         */
/*  function:  Prints information from a curve buffer to the stderr stream.*/
/*                                                                         */
/*   Variables:   pBuf - pointer to a curve buffer structure               */
/*                                                                         */
/*   Returns: none                                                         */
/*                                                                         */
/* last changed:                                                           */
/*               8/1/89   DI                                               */
/*                                                                         */
/***************************************************************************/
  
VOID print_tempbuf(CURVEBUFFER FAR *pBuf)
{
   USHORT i, LastPrinted;
   PVOID Xptr, Yptr;
   USHORT pointnum;
   SHORT continued_segment = -1;
   USHORT print_interval;
   USHORT DataType;
   FLOAT XTmp, YTmp;
  
   fprintf(stderr, "File name = -%s-\n",
   pBuf->ActiveDir->Entries[pBuf->Entry].name);
  
   fprintf(stderr, "File curve number = %u\n", pBuf->CurveIndex);
  
   pointnum = pBuf->Curvehdr.pointnum;
   fprintf(stderr, "curve pointnum =          %u\n", pointnum);
  
   fprintf(stderr, "curve starting point =    %u\n", pBuf->BufferOffset);
  
   fprintf(stderr, "curve X Units =     %d\n",
   (int) pBuf->Curvehdr.XData.XUnits);
   DataType = (USHORT) pBuf->Curvehdr.DataType;
   fprintf(stderr, "curve DataType =     %Xx\n", DataType);
   fprintf(stderr, "experiment number = %u\n",
   pBuf->Curvehdr.experiment_num);
   fprintf(stderr, "time =            %lu\n", pBuf->Curvehdr.time);
   fprintf(stderr, "SC val =          %lu\n", pBuf->Curvehdr.scomp);
   fprintf(stderr, "curve PIA =    %xX %xX\n",
   pBuf->Curvehdr.pia[0], pBuf->Curvehdr.pia[1]);
   fprintf(stderr, "curve Y min = %f Y max = %f\n",
   pBuf->Curvehdr.Ymin, pBuf->Curvehdr.Ymax);
   fprintf(stderr, "curve X min = %f X max = %f\n",
   pBuf->Curvehdr.Xmin, pBuf->Curvehdr.Xmax);
  
   Yptr = (PVOID) pBuf->BufPtr;
   Xptr = (PVOID) ((ULONG) pBuf->BufPtr + (BUFLEN / 2));
  
   pointnum -= pBuf->BufferOffset;
   if (pointnum > ((BUFLEN / 2) / (DataType & 0x0F)))
      pointnum = (BUFLEN / 2) / (DataType & 0x0F);
  
   print_interval = (pointnum / 100) + 1; /* only print out 100 values */
  
   LastPrinted = -1;
   for (i=0; i < pointnum; i++)
   {
      if ((i % print_interval) == 0)
      {
         ConvertTypes(Xptr, DataType, &XTmp, FLOATTYPE);
         ConvertTypes(Yptr, DataType, &YTmp, FLOATTYPE);
         fprintf(stderr, "data[ %u ]    %f %f\n",
         pBuf->BufferOffset + i, XTmp, YTmp);
         LastPrinted = i;
      }
      Yptr = (PVOID) ((ULONG) Yptr + (ULONG)(DataType & 0x0F));
      Xptr = (PVOID) ((ULONG) Xptr + (ULONG)(DataType & 0x0F));
   }
   i--;
   if (LastPrinted < i)
   {
      Yptr = (PVOID) ((ULONG) Yptr - (ULONG)(DataType & 0x0F));
      Xptr = (PVOID) ((ULONG) Xptr - (ULONG)(DataType & 0x0F));
      fprintf(stderr, "data[ %u ]    %f %f\n",
      pBuf->BufferOffset + i, XTmp, YTmp);
   }
}
 
#endif // DEBUG
