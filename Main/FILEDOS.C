/****************************************************************************/
/*                                                                          */
/*  Filedos.c                                                               */
/*  copyright (c) 1989, EG&G Instruments Inc.                               */
/*  Transfer data to and from permanent disk file storage in                */
/*  OMA4000 program. Uses DOS and DOS16M memory conventions                 */
/*                                                                          */
/*  $Log:   J:/logfiles/oma4000/main/filedos.c_v  $                         */ 
/*                                                                          */
/****************************************************************************/

#ifdef PROT
   #define INCL_DOSMEMMGR
   #define INCL_NOPM
   #include <os2.h>
#endif
  
#include <io.h>
#include <malloc.h>
#include <string.h>
#include <dos.h>
#include <time.h>
#include <sys\types.h>
#include <sys\stat.h>
  
#include "filestuf.h"
#include "crvhdr35.h"  /* curveHdr1Size() */ 
#include "di_util.h"   /* ParseFileName() */ 
#include "tempdata.h"  /* InsertTempCurvesInFile() */ 
#include "oma4driv.h"   /* get_OMA4_data */ 
#include "detsetup.h"   /* get_OMA4_data */ 
#include "syserror.h"  /* ERROR_OPEN */ 
#include "omaerror.h"  /* error */ 
#include "curvbufr.h"  /* CURVEBUFFER */ 
#include "crventry.h"
#include "omameth.h"   /* MethdrWrite() */ 
#include "curvedir.h"
#include "live.h"      /* sub_background() */ 
  
#ifdef USE_D16M
   #include <dos16.h>
#endif
  
#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

/* file ID strings for FileTypeId record in METHDR structure */

const CHAR *fidDataOMA4 =      "DATA EG&G PARC OMA4";
const CHAR *fidMethodOMA4 =    "METHOD EG&G PARC OMA4";
const CHAR *fidGenericData =   "DATA EG&G";
const CHAR *fidGenericMethod = "METHOD EG&G";
const CHAR *fidMacroOMA4 =     "MACRO EG&G PARC OMA4";
  
static const PCHAR OtherDesc = "********";

/* function prototypes */
PRIVATE ERR_OMA read_data_seg(FILE *, const char *, PVOID,
                                      USHORT *, USHORT);
PRIVATE ERR_OMA write_data_seg(FILE *, const char *, PVOID,
                                       USHORT *, USHORT);
PRIVATE ERR_OMA CurveDataRead(FILE *, const char *, CURVEBUFFER far *);
PRIVATE ERR_OMA CurveDataWrite(FILE *, const char *,
                                       CURVEBUFFER far *);

/* increment for address to get next segment of a huge array */ 
static ULONG HugeSegIncr;

/***************************************************************************/
/* function:  Initialize HugeSegIncr                                       */
/*                                                                         */
/***************************************************************************/

void filedos_init(void)
{
#ifndef PROT
#ifdef USE_D16M
      HugeSegIncr = 0x00010000L;  
#else
      HugeSegIncr = 0x10000000L;
#endif
#else
   USHORT HugeSelShift;

   /* increment for the next huge selector in an array */
   DosGetHugeShift(& HugeSelShift);
   HugeSegIncr = (ULONG) (1 << HugeSelShift) * SEGSIZE;
#endif
}
  
/***************************************************************************/
/*  Function:  Read in one memory segment's worth of data. Complete values */
/*             only.  File Pointer must be at next data segment.           */
/*                                                                         */
/*  Variables: fhnd - pointer to the stream                                */
/*             fName - file name of the stream                             */
/*             dataptr - pointer to the start of this segments previously  */
/*              allocated data space                                       */
/*             numpts - Input and output. Number of data points left to be */
/*              read in for this curve.                                    */
/*             DataType - Input.  Uses definitions from OMA35.H            */
/*                                                                         */
/*  Returns:   ERROR_ALLOC_MEM - memory allocation error                   */
/*             ERROR_READ - stream read error                              */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/***************************************************************************/
  
PRIVATE ERR_OMA read_data_seg(FILE *fhnd, const char * fName,
                                      PVOID dataptr, USHORT *numpts,
                                      USHORT DataType)
{
  ULONG buffer_size_points = (SEGSIZE - FP_OFF(dataptr)) /
    (ULONG) (DataType & 0x0F);

  if ((ULONG) *numpts < buffer_size_points)
    buffer_size_points = (ULONG) *numpts;

  if (fread(dataptr, (size_t) DataType & 0x0F, (SHORT) buffer_size_points,
    fhnd) != (USHORT) buffer_size_points)
    {
    return error(ERROR_READ, fName);
    }

  *numpts -= (USHORT) buffer_size_points;

  return ERROR_NONE;
}
  
  
/***************************************************************************/
/*  Function:  Write one memory segment's worth of data. Complete values   */
/*             only.                                                       */
/*                                                                         */
/*  Variables: fhnd      - pointer to the stream                           */
/*             fName     - file name of the stream                         */
/*             dataptr   - pointer to the start of this segments           */
/*                         previously allocated data space                 */
/*             numpts    - Input and output. Number of data points left    */
/*                         to be written for this curve.                   */
/*             DataType  - Input.  Uses definitions from OMA35.H           */
/*                                                                         */
/*  Returns:   ERROR_ALLOC_MEM - memory allocation error                   */
/*             ERROR_WRITE - stream write error                            */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/***************************************************************************/
  
PRIVATE ERR_OMA write_data_seg(FILE *fhnd, const char * fName,
                                       PVOID dataptr, USHORT * numpts,
                                       USHORT DataType)
{
  ULONG buffer_size_points = (SEGSIZE - FP_OFF(dataptr)) /
                             (ULONG) (DataType & 0x0F);
  
  if ((ULONG) *numpts < buffer_size_points)
    buffer_size_points = (ULONG) *numpts;

  if (fwrite(dataptr, (size_t) (DataType & 0x0F),
    (USHORT) buffer_size_points, fhnd) != (USHORT) buffer_size_points)
    {
    return error(ERROR_WRITE, fName);
    }

  *numpts -= (USHORT) buffer_size_points;
  
  return ERROR_NONE;
}
  
/***************************************************************************/
/*  Function:  Write all of the data points for this curve to stream       */
/*                                                                         */
/*  Variables: fhnd      - pointer to the stream                           */
/*             fName     - file name of the stream                         */
/*             pCurveBuf - pointer to the curve structure                  */
/*                                                                         */
/*  Returns:   ERROR_ALLOC_MEM - memory allocation error                   */
/*             ERROR_WRITE - stream write error                            */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/***************************************************************************/
  
PRIVATE ERR_OMA CurveDataWrite(FILE *fhnd, const char * fName,
                               CURVEBUFFER far * pCurveBuf)
{
  USHORT numpts;
  PVOID data_ptr;
  ERR_OMA err;

  /* write out the Y data */
  numpts = (USHORT) pCurveBuf->Curvehdr.pointnum;
  data_ptr = pCurveBuf->BufPtr;

  while (numpts > 0)
    {
    if (err = write_data_seg(fhnd, fName, data_ptr, &numpts,
      pCurveBuf->Curvehdr.DataType))
      return err;

    data_ptr = (PVOID) ((0xFFFF0000 & (ULONG)data_ptr) + HugeSegIncr);
    }

  /* write out the X data */
  numpts = (USHORT) pCurveBuf->Curvehdr.pointnum;
  data_ptr = pCurveBuf->Curvehdr.XData.XArray;

  while (numpts > 0)
    {
    if (err = write_data_seg(fhnd, fName, data_ptr, &numpts,
      pCurveBuf->Curvehdr.DataType))
      return err;

    data_ptr = (PVOID) ((0xFFFF0000 & (ULONG)data_ptr) + HugeSegIncr);
    }
  return ERROR_NONE;
}
  
/***************************************************************************/
/*  Function:  Get the number of curves in the file.                       */
/*                                                                         */
/*  Variables: fhnd - pointer to the file                                  */
/*             fName - file name of the file                               */
/*             pCurveNum - address of the variable to recieve the curve    */
/*                         number as specified in the METHDR.MemNumber     */
/*                         field at the start of the file.                 */
/*                                                                         */
/*  Returns:   ERROR_READ - stream read error                              */
/*             ERROR_SEEK - stream seek error                              */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Changes file pointer                                     */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA GetFileCurveNum(FILE * fhnd, const char * fName,
                               PUSHORT pCurveNum)
{

  if (fseek(fhnd, (LONG) offsetof(METHDR, FileCurveNum), SEEK_SET) != 0)
    return error(ERROR_SEEK, fName);

  if (fread(pCurveNum, sizeof (SHORT), 1, fhnd) != 1)
    return error(ERROR_READ, fName);

  return ERROR_NONE;
}
  
/***************************************************************************/
/*  Function:  Set the number of curves in the file header.                */
/*                                                                         */
/*  Variables: fhnd - pointer to the file                                  */
/*             fName - file name of the file                               */
/*             pCurveNum - curve number to be put into in the              */
/*                         METHDR.file_curvenum field at the start of the  */
/*                         file.                                           */
/*                                                                         */
/*  Returns:   ERROR_WRITE - stream write error                            */
/*             ERROR_SEEK - stream seek error                              */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Changes file pointer                                     */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA SetFileCurveNum(FILE *fhnd, const char * fName,
                               USHORT CurveNum)
{

  if (fseek(fhnd, (LONG) offsetof(METHDR, FileCurveNum), SEEK_SET) != 0)
    return error(ERROR_SEEK, fName);

  if (fwrite(&CurveNum, sizeof (USHORT), 1, fhnd) != 1)
    return error(ERROR_WRITE, fName);

  return ERROR_NONE;
}
  
/***************************************************************************/
/*  Function:  Get the offset to the specified curve in the file           */
/*                                                                         */
/*  Variables: fhnd - pointer to the file                                  */
/*             fName - file name of the file                               */
/*             CurveNum - Number of the curve in the file. 0 is the index  */
/*                        of the first curve.                              */
/*             pCurveOffset - address of the variable to recieve the       */
/*                            byte offset of the specified curve block     */
/*                            from the start of the file.                  */
/*                                                                         */
/*  Returns:   ERROR_READ - stream read error                              */
/*             ERROR_SEEK - stream seek error                              */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Changes file pointer to the start of the CurveNum curve  */
/*                for following read operations.                           */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA GetFileCurveOffset(FILE *fhnd, const char * fName,
                                  USHORT CurveNum, PULONG pCurveOffset)
{
  USHORT MethdrLen;
  USHORT PointSize;
  USHORT i;
  CURVEHDR TempCurvehdr;
  UCHAR Version;

  /* read the method header length */
  if (fseek(fhnd, (LONG) offsetof(METHDR, StructureVersion), SEEK_SET) != 0)
    return error(ERROR_SEEK, fName);

  /* read the method header version */
  if (fread(&Version, sizeof (UCHAR), 1, fhnd) != 1)
    return error(ERROR_READ, fName);

  /* read the method header length */
  if (fread(&MethdrLen, sizeof (USHORT), 1, fhnd) != 1)
    return error(ERROR_READ, fName);

  *pCurveOffset = MethdrLen;

  for (i=0; i<CurveNum; i++)
    {
    /* DataType must be obtained for each curve */ 
    if(fseek(fhnd, *pCurveOffset, SEEK_SET) != 0)
      return error(ERROR_SEEK, fName);

    /* read out enough to get number of points and data type */ 
    if(fread(&TempCurvehdr, offsetof(CURVEHDR, experiment_num), 1, fhnd) != 1)
      return error(ERROR_READ, fName);

    PointSize = TempCurvehdr.DataType & 0x0F;

    /* add the X and Y points to the curve offset */
    if (Version <11)     /* check for version curvehdr length */
      *pCurveOffset += curveHdr1Size();
    else
      *pCurveOffset += sizeof(CURVEHDR);

    *pCurveOffset += (ULONG) TempCurvehdr.pointnum * (ULONG) PointSize * 2;
    }

  if (fseek(fhnd, *pCurveOffset, SEEK_SET) != 0)
    return error(ERROR_SEEK, fName);

  return ERROR_NONE;
}
  
/***************************************************************************/
/*  Function:  Write some curves to a file.                                */
/*                                                                         */
/*  Variables: fhnd         - pointer to the stream                        */
/*             fName        - file name of the stream                      */
/*             FileCurveNum - starting index into the file of the first    */
/*                            curve 0..?                                   */
/*             BlkSize      - number of curves to write.                   */
/*             pCurveBuf    - Pointer to a curvebuffer structure which has */
/*                            a curveheader and data space for the curves  */
/*                                                                         */
/*  Returns:   ERROR_ALLOC_MEM - memory allocation error                   */
/*             WRITEERR - stream read error                                */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: All curves written with same curve header, so number of  */
/*                points for each will be same.  Since used only for live  */
/*                data this should not cause a problem.                    */
/*                                                                         */
/***************************************************************************/
  
PRIVATE ERR_OMA MultiCurveWrite(FILE *fhnd, const char * fName,
                                       USHORT FileCurveNum,
                                       USHORT SrcStCurve,
                                       USHORT BlkSize,
                                       CURVEBUFFER far * pCurveBuf)
{
  USHORT i, curvelen, Frame, tmp;
  ERR_OMA err;
  USHORT FileCurveCount;
  ULONG CurveOffset;

  if (err = GetFileCurveNum(fhnd, fName, &FileCurveCount))
    return err;

  curvelen =
    pCurveBuf->Curvehdr.pointnum * (pCurveBuf->Curvehdr.DataType & 0x0F);

  if (err = GetFileCurveOffset(fhnd, fName, FileCurveNum, &CurveOffset))
    return err;

  tmp=pCurveBuf->Curvehdr.MemData;
  pCurveBuf->Curvehdr.MemData = FALSE;

  /* write curve block */
  for (i=SrcStCurve; i<BlkSize + SrcStCurve; i++)
    {
    ReadCurveFromMem(pCurveBuf->BufPtr, curvelen, i);
    Frame = i;
    get_Frame(&Frame);
    get_source_comp_point(&(pCurveBuf->Curvehdr.scomp), Frame);
    sub_background(pCurveBuf->BufPtr, i, &pCurveBuf->Curvehdr);

    if(fwrite(&(pCurveBuf->Curvehdr), sizeof(CURVEHDR), 1, fhnd) != 1)
      return error(ERROR_WRITE, fName);

    if (err = CurveDataWrite(fhnd, fName, pCurveBuf))
      return err;

    }  /* for i write all curves */

  pCurveBuf->Curvehdr.MemData = tmp;

  return ERROR_NONE;
}
  
/***************************************************************************/
/*  Function:  delete curves in a file.                                    */
/*                                                                         */
/*  Variables: fhnd - pointer to the stream                                */
/*             fName - file name of the stream                             */
/*             FileCurveNum - starting index into the file of the first    */
/*                            curve 0..?                                   */
/*             Count - number of curves to delete.                         */
/*                                                                         */
/*  Returns:   WRITEERR - stream read error                                */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Changes number of curves in the file.                    */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA MultiFileCurveDelete(FILE *fhnd, const char * fName,
                                    USHORT FileCurveNum, USHORT Count)
{
  ERR_OMA err;
  USHORT FileCurveCount;
  ULONG DelCurveOffset, BlkSz;
  ULONG FileSz, MoveLen;

  if (err = GetFileCurveNum(fhnd, fName, &FileCurveCount))
    return err;

  if (err = GetFileCurveOffset(fhnd, fName, FileCurveCount, &FileSz))
    return err;

  /* check for bad curve Numbers */
  if ((FileCurveNum + Count) > (FileCurveCount))
    return error(ERROR_CURVE_NUM, FileCurveNum + Count);

  if(err = GetFileCurveOffset(fhnd, fName, FileCurveNum, &DelCurveOffset))
    return err;

  if(err = GetCurveBlkSz(fhnd, fName, DelCurveOffset, Count, &BlkSz, TRUE))
    return err;

  MoveLen = FileSz - (DelCurveOffset + BlkSz);
  /* compact file down */
  if (err = MoveFileBlock(fhnd, fName, DelCurveOffset,
                          fhnd, fName, DelCurveOffset + BlkSz, MoveLen))
    return err;

  if (err = SetFileCurveNum(fhnd, fName, FileCurveCount - Count))
    return err;

  if (err = GetFileCurveOffset(fhnd, fName, FileCurveCount - Count, &FileSz))
    return err;

  if (chsize(fileno(fhnd), FileSz) == -1)
    return error(ERROR_WRITE, fName);

  return ERROR_NONE;
}
  
  
/***************************************************************************/
/*  Function:  Write out a block of curves to a file.                      */
/*                                                                         */
/*  Variables: File - handle of opened output file                         */
/*             fName - file name of output file                            */
/*             DstOffset - where to start writing                          */
/*             CurveDir - Curve directory for entry of block               */
/*             EntryIndex - Input. Index into curve entry directory for    */
/*                          the source curve block entry.                  */
/*             Count - Input. Number of curves in block                    */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side Effects:                                                          */
/*                                                                         */
/***************************************************************************/

ERR_OMA DataFileWrite(FILE *fhnd, const char * fName,
                             ULONG DstOffset, CURVEDIR *CurveDir,
                             SHORT EntryIndex,
                             USHORT StCurveNum, USHORT Count)
{
  ERR_OMA err;
  CURVEBUFFER far * pCurveBuf;
  USHORT PointNum;
  SHORT prefBuf = -1;

  /* get curve header for live into curve buffer.  Also loads */
  /* X data into last half of buffer */

  PointNum = 0;
  LoadCurveBuffer(CurveDir, EntryIndex, 0, &PointNum, &prefBuf);
  /* Curve  Index */

  pCurveBuf = &(CvBuf[prefBuf]);

  fseek(fhnd, DstOffset, SEEK_SET);

  if (err = MultiCurveWrite(fhnd, fName, 0, StCurveNum, Count, pCurveBuf))
    {
    fclose(fhnd);
    return err;
    }

  return ERROR_NONE;
}
  
/***************************************************************************/
/*  Function:  Write out a block of (possibly) live curves to a temp file. */
/*                                                                         */
/*  Variables: File - handle of opened output file                         */
/*             fName - file name of the opened output file                 */
/*             DstOffset  - where to start writing                         */
/*             CurveDir   - Curve directory for entry of block             */
/*             EntryIndex - Input. Index into curve directory for          */
/*                          the source curve block entry.                  */
/*             Count      - Input. Number of curves in block               */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_NONE = OK                                             */
/*                                                                         */
/*  Side Effects:                                                          */
/*                                                                         */
/***************************************************************************/

ERR_OMA TempFileWrite(FILE *fhnd,
                             const char * fName,
                             ULONG DstOffset,
                             CURVEDIR *CurveDir,
                             SHORT EntryIndex,
                             USHORT SrcStart,
                             USHORT DstStart,
                             USHORT Count,
                             CHAR DestType)
{
  ERR_OMA err = ERROR_NONE;
  CURVEBUFFER far * pCurveBuf;
  CURVEHDR * pCurvehdr;
  USHORT PointNum, i, curvelen;
  BOOLEAN tmp1, tmp2;
  SHORT prefBuf = BUFNUM - 1;

  /* get curve header for live into curve buffer.  Also loads */
  /* X data into last half of buffer */

  PointNum = 0;
  LoadCurveBuffer(CurveDir, EntryIndex, 0, &PointNum, &prefBuf);

  pCurveBuf = &(CvBuf[prefBuf]);
  pCurvehdr = &(CvBuf->Curvehdr);

  curvelen = pCurveBuf->Curvehdr.pointnum
    * ((pCurvehdr->DataType & 0x0F) << 1); /* allow for X data */

  /* write curve block */

  for (i= SrcStart; i < Count + SrcStart; i++)
    {
    PointNum = 0;
    LoadCurveBuffer(CurveDir, EntryIndex, i, &PointNum, &prefBuf);

    if (DestType != OMA4MEMDATA || i == SrcStart)
      {
      tmp1 = pCurvehdr->MemData;
      tmp2 = pCurvehdr->CurveCount;

      if (DestType != OMA4MEMDATA)
        {
        pCurvehdr->MemData = FALSE;
        pCurvehdr->CurveCount = 1;
        }

      if (fseek(fhnd, DstOffset, SEEK_SET))
        {
        return error(ERROR_SEEK, fName);
        }

      if(fwrite(pCurvehdr, sizeof(CURVEHDR), 1, fhnd) != 1)
        {
        return error(ERROR_WRITE, fName);
        }

      pCurvehdr->MemData = tmp1;
      pCurvehdr->CurveCount = tmp2;

      if (DestType != OMA4MEMDATA)
        {
        if(fwrite(pCurveBuf->BufPtr, (pCurvehdr->DataType & 0x0F),
                  (USHORT)(pCurvehdr->pointnum), fhnd) !=
           (USHORT) (pCurvehdr->pointnum))
          {
          return error(ERROR_WRITE, fName);
          }
        if(fwrite(((char *)pCurveBuf->BufPtr + (BUFLEN/2)), /*write X data*/
           sizeof(float),                                   /*always float*/
           (USHORT)(pCurvehdr->pointnum),
            fhnd) != (USHORT) (pCurvehdr->pointnum))
          {
          return error(ERROR_WRITE, fName);
          }
        DstOffset += (curvelen + sizeof(CURVEHDR));
        }
      }

    if (DestType == OMA4MEMDATA)
      {
      if (pCurveBuf->Curvehdr.DataType != LONGTYPE)
        {
        unsigned int i;
        for (i = 0;i < pCurveBuf->Curvehdr.pointnum;i++)
          {
          ((LONG *)(pCurveBuf->BufPtr))[i] =
            (LONG) ((FLOAT *)pCurveBuf->BufPtr)[i];
          }
        }
      WriteCurveToMem(pCurveBuf->BufPtr, curvelen/2, DstStart + (i - SrcStart));
      }
    }    /* for i (write all curves) */
  pCurveBuf->ActiveDir = NULL;
  return err;
}

/***************************************************************************/
/*  Function:  Fills a CurveDir Entry structure with info from a file.     */
/*             Sets display window to 0.                                   */
/*                                                                         */
/*  Variables:                                                             */
/*             CurveDir - Input. Pointer to the curve directory in which to*/
/*                        put the entry.                                   */
/*             FileName - filename string                                  */
/*             Path - File's full path.                                    */
/*             DirIndex - Input. Requested index for the directory entry   */
/*                        Output. Actualindex for the directory entry      */
/*                                                                         */
/*  Returns:   ERROR_ALLOC_MEM - memory allocation error                   */
/*             ERROR_READ - file READ ERROR                                */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Changes current drive and directory per Path string.     */
/*                                                                         */
/***************************************************************************/

ERR_OMA FillDescFieldsFromFile(CURVEDIR *pCurveDir, PCHAR FileName,
                                      PCHAR Path, SHORT *DirIndex)
{
  ERR_OMA err;
  FILE *fhnd;
  TM *Time;
  struct stat FileStatus;
  CHAR FileType;
  ULONG StartOffset;
  CHAR FullFileName[FNAME_LENGTH];
  CHAR FullPath[DOSPATHSIZE+1];
  CHAR Desc[DESCRIPTION_LENGTH];
  USHORT CurveNum;

  /* get the full path ending in a '\' */
  if (ParsePathAndName(FullPath, FullFileName, Path) != 1)
    return error(ERROR_OPEN, Path);

  strcpy(FullFileName, FullPath);

  /* add the file name to the path */
  strcat(FullFileName, FileName);
  fhnd = fopen(FullFileName, "rb");

  if (fhnd == NULL)
    return error(ERROR_OPEN, FullFileName);

  if (stat(FullFileName, &FileStatus))
    {
    fclose(fhnd);
    return error(ERROR_READ, FullFileName);
    }

  Time = localtime(&(FileStatus.st_atime));

  err = ReadDirFileInfo(fhnd, &FileType, &StartOffset, Desc, &CurveNum);
  fclose(fhnd);
  if ((err != ERROR_CORRUPT_FILE) && (err != ERROR_NONE))
    return err;

  if (*DirIndex > (SHORT) pCurveDir->BlkCount)
    *DirIndex = (SHORT) pCurveDir->BlkCount;

  err = ERROR_NONE;

  if ((err = InsertCurveBlkInDir(FileName, FullPath, Desc, 0, StartOffset,
                                 CurveNum, Time, pCurveDir, * DirIndex,
                                 FileType)))
    return err;

  if (FileType != OTHER)
    {
    /* curves weren't really loaded */
    pCurveDir->CurveCount -= CurveNum;

    /* adjust pCurveDir[*DirIndex].count to show number of curves in file */
    pCurveDir->Entries[*DirIndex].count = CurveNum;
    }

  return ERROR_NONE;
}

/***************************************************************************/
/*  Function:  Figure whether the file is an OMA 4 file and its            */
/*             descriptive info if an OMA 4 file                           */
/*                                                                         */
/*  Variables:                                                             */
/*             fhnd - handle of opened file to check                       */
/*             FileType - Value which relates to EntryType field of        */
/*                CURVE_ENTRY to describe type of file.                    */
/*             MetLength - Length of the complete method header            */
/*             pDesc - pre-allocated buffer to store the description into. */
/*                Must be at least DESCRIPTION_LENGTH long. Copies         */
/*                OtherDesc if not an OMA4 file type                       */
/*             CurveNum - number of curves in the file.                    */ 
/*                                                                         */
/*  Returns:                                                               */
/*             Value which relates to EntryType field of CURVE_ENTRY to    */
/*             describe type of file.                                      */
/*             0 - read error or unknown file type                         */
/*             See OMA35TYP.h for allowed values and their meaning         */
/*                                                                         */
/*  Side effects: Changes file pointer.                                    */
/*                                                                         */
/***************************************************************************/

ERR_OMA ReadDirFileInfo(FILE *fhnd, PCHAR FileType, PULONG MetLength,
                               PCHAR pDesc, PUSHORT CurveNum)
{
  METHDR *TempMethdr;    /* pointer to a partial method header */
  USHORT ReadLen = offsetof(METHDR, InterfaceType);
  SHORT Temp;

  strcpy(pDesc, OtherDesc);   /* default */
  *CurveNum = 0;
  *MetLength = 0;

  /* size of a partial method header with a little extra */
  TempMethdr = malloc(ReadLen + 2);
  if (TempMethdr == NULL)
    return error(ERROR_ALLOC_MEM);

  rewind(fhnd);

  *FileType = OTHER;

  if (fread(TempMethdr, ReadLen, 1, fhnd) != 1)
    {
    free(TempMethdr);
    return ERROR_NONE;   /* assume too small - other type */
    }

  Temp = strlen(fidDataOMA4);
  /* Identify file type */
  if (! strncmp(TempMethdr->FileTypeID, fidDataOMA4, Temp))
    {
    if (TempMethdr->StructureVersion < 12)
      *FileType = OMA4V11DATA;
    else
      *FileType = OMA4DATA;
    }
  else
    {
    Temp = strlen(fidMethodOMA4);
    if (!strncmp(TempMethdr->FileTypeID, fidMethodOMA4, Temp))
      {
      if (TempMethdr->StructureVersion < 12)
        *FileType = OMA4V11METHOD;
      else
        *FileType = OMA4METHOD;
      }
    }

  if (*FileType == OTHER)
    {
    free(TempMethdr);
    return ERROR_NONE;
    }

  *MetLength = (ULONG) TempMethdr->Length;
  *CurveNum = TempMethdr->FileCurveNum;
  /* make sure that there is an ending NULL */ 
  TempMethdr->Description[DESCRIPTION_LENGTH-1] = '\0';
  strcpy(pDesc, TempMethdr->Description);

  free(TempMethdr);
  return ERROR_NONE;
}

/***************************************************************************/
/*  Function:  Figure whether the file is an OMA 4 or OMA 3 file and what  */
/*             type                                                        */
/*                                                                         */
/*  Variables:                                                             */
/*             fhnd - handle of opened file to check                       */
/*                                                                         */
/*  Returns:                                                               */
/*             Value which relates to EntryType field of CURVE_ENTRY to    */
/*             describe type of file.                                      */
/*             0 - read error or unknown file type                         */
/*             See OMA35TYP.h for allowed values and their meaning         */
/*                                                                         */
/*  Side effects: Changes file pointer.                                    */
/*                                                                         */
/***************************************************************************/

CHAR IDFileType(FILE *fhnd)
{
  CHAR IDBuf[FTIDLEN + 1];
  SHORT Temp;
  UCHAR Version;

  rewind(fhnd);
  Temp = strlen(fidDataOMA4);
  if (fread(IDBuf, 1, FTIDLEN, fhnd) != FTIDLEN)
    /* file not long enough or a read error */
    {
    rewind(fhnd);
    return OTHER;
    }

  if (fread(&Version, sizeof(UCHAR), 1, fhnd) != 1)
    /* file not long enough or a read error */
    {
    rewind(fhnd);
    return OTHER;
    }

  /* Identify file type */
  if (! strncmp(IDBuf, fidDataOMA4, Temp))
    {
    if (Version < 12)
      return OMA4V11DATA;
    else
      return OMA4DATA;
    }

  Temp = strlen(fidMethodOMA4);
  if (! strncmp(IDBuf, fidMethodOMA4, Temp))
    {
    if (Version < 12)
      return OMA4V11METHOD;
    else
      return OMA4METHOD;
    }
  return OTHER;
}

/***************************************************************************/
/*  Function:  Find the size of a group of curves in the given file.       */
/*                                                                         */
/*  Variables:                                                             */
/*             hFile - Input. Open file pointer                            */
/*             fName - file name of inputfile                              */
/*             StartCurveOffset - Input. Starting file byte offset of curve */
/*                                block.                                   */
/*             Count - Input. Number of curves in block.                   */
/*             BlkSz - Output. Size of the block in bytes.                 */
/*                                                                         */
/*  Returns:                                                               */
/*             ERROR_READ - temp file seek error                           */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Changes file pointer.                                    */
/*                                                                         */
/***************************************************************************/

ERR_OMA GetCurveBlkSz(FILE *hFile, const char * fName,
                             ULONG StartCurveOffset, USHORT Count,
                             PULONG BlkSz, BOOLEAN XcludMemData)
{
  CURVEHDR TmpCurvehdr;
  USHORT PointNum;
  USHORT DataSz, i;
  ULONG CurveSz, HdrSz = sizeof(CURVEHDR);

  *BlkSz = 0;
  for (i=0; i<Count; i++)
    {
    if (fseek(hFile, StartCurveOffset, SEEK_SET) != 0)
      return error(ERROR_SEEK, fName);

    if (fread(&TmpCurvehdr, sizeof (CURVEHDR), 1, hFile) != 1)
      return error(ERROR_READ, fName);

    PointNum = TmpCurvehdr.pointnum;
    DataSz = TmpCurvehdr.DataType & 0x0F;

    /* get the length of the curve */

    CurveSz = ((ULONG) PointNum * (ULONG)DataSz * 2);
    StartCurveOffset += HdrSz;

    if (TmpCurvehdr.MemData) /* live Cvhdrs may carry many curves */
      {
      int j = TmpCurvehdr.CurveCount;
      if (i + j >= Count) j = Count;
      i += j;
      if (!XcludMemData)
        *BlkSz += (CurveSz + HdrSz) * j;
      else
        *BlkSz += (HdrSz);
      }
    else  /* if not live data, CurveCount not guaranteed to be one */
      {
      StartCurveOffset += CurveSz;
      *BlkSz += (HdrSz + CurveSz);
      }
    }
  return ERROR_NONE;
}

/* helper function for MoveFileBlock() */ 
/* forcibly uses Curve Buffer #2 without flushing first!!! */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ 
PRIVATE ERR_OMA transferBlock(FILE * fhDst, const char * dstName,
                                     ULONG DstOffset,
                                     FILE * fhSrc, const char * srcName,
                                     ULONG SrcOffset,
                                     USHORT BufLen)
{
  if (fseek(fhSrc, SrcOffset, SEEK_SET) != 0)
    return error(ERROR_SEEK, srcName);

  /* read in from the source */
  if (fread(CvBuf[2].BufPtr, 1, BufLen, fhSrc) != BufLen)
    return error(ERROR_READ, srcName);

  CvBuf[2].status = CVBUF_CLEAN;

  if (fseek(fhDst, DstOffset, SEEK_SET) != 0)
    return error(ERROR_SEEK, dstName);

  /* Write back to the destination */
  if (fwrite(CvBuf[2].BufPtr, 1, BufLen, fhDst) != BufLen)
    return error(ERROR_WRITE, dstName);

  return ERROR_NONE;
}

/***************************************************************************/
/*  Function:  Move the data in a file around.                             */
/*                                                                         */
/*  Variables: fhDst -  Destination file.                                  */
/*             dstName - file name of destination file                     */
/*             DstOffset - Destination address, byte offset into file      */
/*             fhSrc -  Source file.                                       */
/*             srcName - file name of source file                          */
/*             SrcOffset - Source address, byte offset into file           */
/*             Length - length of block to move                            */
/*                                                                         */
/*  Returns:   ERROR_READ - temp file seek error                           */
/*             ERROR_WRITE - temp file write error                         */
/*             ERROR_SEEK - temp file seek error                           */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Changes file pointer and CvBuffer[2] fields. May overwite */
/*                curves in file.                                          */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA MoveFileBlock(FILE * fhDst, const char * dstName,
                             ULONG DstOffset,
                             FILE * fhSrc, const char * srcName,
                             ULONG SrcOffset, ULONG Length)
{
  USHORT BufLen;
  ERR_OMA err;

  /* make sure that something needs to be done */
  if ((SrcOffset == DstOffset) || (Length == 0))
    return ERROR_NONE;

  if(err = clearAllCurveBufs())
    return err;

  /* check direction so that nothing is overwritten */
  if (SrcOffset > DstOffset)
    {
    while (Length)
      {
      if (Length > BUFLEN)
        BufLen = BUFLEN;
      else
        BufLen = (USHORT) Length;

      err = transferBlock(fhDst, dstName, DstOffset, fhSrc, srcName,
        SrcOffset, BufLen);
      if(err)
        return err;

      SrcOffset += (ULONG) BufLen;
      DstOffset += (ULONG) BufLen;
      Length -= (ULONG) BufLen;
      }
    }
  else  /* The source is below the destination */
    {
    SrcOffset += Length;
    DstOffset += Length;

    while (Length)
      {
      if (Length > BUFLEN)
        BufLen = BUFLEN;
      else
        BufLen = (USHORT) Length;

      SrcOffset -= BufLen;
      DstOffset -= BufLen;

      err = transferBlock(fhDst, dstName, DstOffset, fhSrc, srcName,
        SrcOffset, BufLen);
      if(err)
        return err;

      Length -= BufLen;
      }
    }
  return ERROR_NONE;
}
 
/***************************************************************************/
/*  Function:  Write a curve block from its temporary curve file to its    */
/*             named file.  The file will be created if not already        */
/*             present.                                                    */
/*                                                                         */
/*  Variables: DstCurve -   Input. FileCurve number to insert in front of  */
/*             pMethdr  -   Input. Method structure to be put at start of  */
/*                          file.                                          */
/*             CurveDir -   Input. Curve directory for curve entry         */
/*             Entry -      Input. Entry for Source curve block.           */
/*             SrcStCurve - Input. Starting file curve number from this    */
/*                          to be written.                                 */
/*             CurveCount - Number of curves from this block to be         */
/*                          written.                                       */
/*                                                                         */
/*  Returns:   ERROR_OPEN - file open error                                */
/*             ERROR_CLOSE - file close error                              */
/*             ERROR_READ - file seek error                                */
/*             ERROR_WRITE - file write error                              */
/*             ERROR_SEEK - file seek error                                */
/*             ERROR_CURVE_NUM - source or destination curve index is bad  */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Uses CvBuf[2]. Flushes and invalidates curve buffer 2.   */
/*                Will overwrite data in existing files.  Changes          */
/*                file_curvenum field in method header                     */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA WriteTempCurveBlkToFile(CHAR * FileName,
                                       USHORT DstCurve,
                                       METHDR *pMethdr,
                                       CURVEDIR *CurveDir,
                                       USHORT Entry,
                                       USHORT SrcStCurve,
                                       USHORT CurveCount)
{
  CURVE_ENTRY *pSrcEntry;
  FILE *fhnd;
  CHAR SaveDesc[DESCRIPTION_LENGTH];
  ERR_OMA err = ERROR_NONE;
  USHORT OldCurveCount;

  if (Entry > CurveDir->BlkCount)
    return(error(ERROR_BAD_CURVE_BLOCK, Entry));

  pSrcEntry = &(CurveDir->Entries[Entry]);

  if(SrcStCurve < pSrcEntry->StartIndex)
    return error(ERROR_CURVE_NUM, SrcStCurve);

  if((SrcStCurve + CurveCount) > (pSrcEntry->StartIndex + pSrcEntry->count))
    return error(ERROR_CURVE_NUM, SrcStCurve + CurveCount);

  if((fhnd = fopen(FileName, "w+b")) == 0)
    return error(ERROR_OPEN, FileName);

  strcpy(pMethdr->FileTypeID, fidDataOMA4);
  strncpy(SaveDesc, pMethdr->Description, DESCRIPTION_LENGTH);
  strncpy(pMethdr->Description, pSrcEntry->descrip, DESCRIPTION_LENGTH);
  pMethdr->FileCurveNum = 0;

  if (err = MethdrWrite(fhnd, FileName, pMethdr))
    return err;

  strcpy(pMethdr->FileTypeID, fidMethodOMA4);
  strncpy(pMethdr->Description, SaveDesc, DESCRIPTION_LENGTH);
  DstCurve = 0;
  OldCurveCount = 0;

  fseek(fhnd, 0L, SEEK_CUR);  /* caused errors if omitted */

  /* Write the curves in the block starting at 0 */
  err = InsertTempCurvesInFile(fhnd, FileName, CurveDir, Entry, SrcStCurve,
                               DstCurve, CurveCount, OldCurveCount);

  /* set a new file curve number */
  if (err == ERROR_NONE)
    err = SetFileCurveNum(fhnd, FileName, CurveCount);

  fclose(fhnd);

  return err;
}

/***************************************************************************/
/*  Function:  Fills a CurveDir structure with info from files matching    */
/*             the given file spec and path.                               */
/*                                                                         */
/*                                                                         */
/*  Variables: file - filename string, can include wildcards               */
/*             path - pathname string                                      */
/*             CurveDir - Input. Pointer to the curve directory in which to */
/*                        put the entry.                                   */
/*                                                                         */
/*  Returns:   ERROR_ALLOC_MEM - memory allocation error                   */
/*             ERROR_READ - file READ ERROR                                */
/*             ERROR_BAD_DIRNAME - bad path given                          */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: Changes current drive and directory per Path string.     */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA FillDirListInfo(PCHAR file, PCHAR Path, CURVEDIR *pCurveDir)
{
  SHORT DirIndex;
  ERR_OMA err = ERROR_NONE;
  CHAR FullPath[DOSPATHSIZE + 1];
  PCHAR FAR *FileNames;
  PVOID Tmpptr;
  SHORT FileCount, i;
  struct find_t Buffer;
  BOOLEAN SearchOK;

  /* change to drive and path specified in Path */
  if (ParseFileName(FullPath, Path) == 0)
    return error(ERROR_BAD_DIRNAME, Path);

  FileNames = malloc(sizeof (PCHAR));
  if(FileNames == 0)
    return error(ERROR_ALLOC_MEM);

  FileCount = 0;

  /* find the first file matching the spec.  Don't include subdirectories */
  SearchOK = _dos_findfirst(file, _A_RDONLY, &Buffer);
  while (! SearchOK)
    {
    FileNames[FileCount] = (PCHAR) strdup(Buffer.name);
    FileCount++;
    /* get space for next file or directory name */ 
    /* Previous Entries array will be freed if not enough space so need to */ 
    /* save it's old pointer and realloc it to previous size if enlargement */ 
    /* fails */ 
    if ((Tmpptr = realloc(FileNames,
      (FileCount+1) * sizeof(PCHAR))) == NULL)
      {
      FileNames = realloc(FileNames, FileCount * sizeof(PCHAR));
      return error(ERROR_ALLOC_MEM);
      }
    else  /* realloc successful */
      FileNames = Tmpptr;

    /* find the next file matching the spec */
    SearchOK = _dos_findnext(&Buffer);
    }

  /* fill in the descriptions from information in the file */
  for (i=0; i<FileCount; i++)
    {
    DirIndex = pCurveDir->BlkCount;
    if (! err)
      err = FillDescFieldsFromFile(pCurveDir, FileNames[i], FullPath, &DirIndex);
    free(FileNames[i]);
    }
  free(FileNames);
  return err;
}

/***************************************************************************/
/*  Function:  Save all the temporary curves listed in CurveDir.           */
/*                                                                         */
/*  Variables: pMethdr - Input. Mehthod structure to be put at start of    */
/*                       file if file not already exist.  If the file      */
/*                       already exists its method header will remain.     */
/*                       file_curvenum in the method header and file will  */
/*                       be automatically adjusted.                        */
/*             CurveDir - Input. Curve directory.                          */
/*             Entry - OutputLast entry completed                          */
/*             Curve - Output. Last curve completed                        */
/*                                                                         */
/*  Returns:   ERROR_OPEN - file open error                                   */
/*             ERROR_CLOSE - file close error                                 */
/*             ERROR_READ - file seek error                                   */
/*             ERROR_WRITE - file write error                                  */
/*             ERROR_SEEK - file seek error                                   */
/*             ERROR_CURVE_NUM - source or destination curve index is bad      */
/*             ERROR_NONE - OK                                                  */
/*                                                                         */
/*  Side effects: Will overwrite data in existing files.                   */
/*                                                                         */
/***************************************************************************/
  
/* ERR_OMA WriteDirFromTemp(METHDR *pMethdr, CURVEDIR *CurveDir, */ 
/*                                 PUSHORT pEntry, PUSHORT pCurve) */ 
/* { */ 
/*    ERR_OMA err = ERROR_NONE; */ 
/*   */ 
/*    for (*pEntry=0; (*pEntry < CurveDir->BlkCount) && (!err); (*pEntry)++) */ 
/*       err = WriteTempCurveBlkToFile(*pCurve, pMethdr, CurveDir, *pEntry, */ 
/*                                      *pCurve, */ 
/*                                      CurveDir->Entries[*pEntry].count); */ 
/*   */ 
/*    *pEntry -= 1;  /* last successful entry read */ 
/*    return err; */ 
/* } */ 
