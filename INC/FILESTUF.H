/***************************************************************************/
/*  filestuf.h                                                             */
/*                *** OMA 35 Version ***                                   */
/*                                                                         */
/*  copyright (c) 1988, EG&G Instruments Inc.                              */
/*                                                                         */
/*  Functions to set up and search directories of curves held in memory.   */
/*
/* $Header: /OMA4000/Include/FILESTUF.H 1     6/18/99 1:52a Maynard $
/***************************************************************************/
  
#ifndef FILESTUF_INCLUDED
#define FILESTUF_INCLUDED

#include <stdio.h>     // FILE

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern const CHAR *fidDataOMA4;
extern const CHAR *fidMethodOMA4;
extern const CHAR *fidGenericData;
extern const CHAR *fidGenericMethod;
extern const CHAR *fidMacroOMA4;

// initialize filedos module, call at system startup time.
void filedos_init(void);

// ERR_OMA DeAllocCurve(LPCURVE);
// ERR_OMA DeAllocXArray(PVOID);
  
ERR_OMA GetFileCurveNum(FILE *, const char *, PUSHORT);
ERR_OMA SetFileCurveNum(FILE *, const char *, USHORT);
ERR_OMA GetFileCurveOffset(FILE *, const char *, USHORT, PULONG);
  
// ERR_OMA CurveRead(FILE *, LPCURVE FAR *, USHORT);
// ERR_OMA CurveWrite(FILE *, CURVEBUFFER far *, USHORT);
// ERR_OMA MultiCurveRead(FILE *, USHORT, USHORT,
//                                LPCURVE * *);
  
ERR_OMA MultiFileCurveDelete(FILE *fhnd, const char * fName,
                                     USHORT FileCurveNum, USHORT Count);
  
// ERR_OMA DataFileRead(PCHAR, PCHAR, LPCURVEDIR, SHORT *,
//                              LPMETHDR FAR *);
ERR_OMA DataFileWrite(FILE *fhnd, const char * fName,
                             ULONG DstOffset, LPCURVEDIR CurveDir,
                             SHORT EntryIndex, USHORT SrcStart, USHORT Count);

ERR_OMA TempFileWrite(FILE *fhnd, const char * fName,
                             ULONG DstOffset, LPCURVEDIR CurveDir,        
                             SHORT EntryIndex, USHORT SrcSt, 
                             USHORT DstSt, USHORT Count, CHAR Dtype);

ERR_OMA FillDirListInfo(PCHAR, PCHAR, LPCURVEDIR);
ERR_OMA FillDescFieldsFromFile(LPCURVEDIR, PCHAR, PCHAR,
                                       SHORT *);
ERR_OMA ReadDirFileInfo(FILE *fhnd, PCHAR FileType,
                                PULONG MetLength, PCHAR pDesc, 
                                PUSHORT CurveNum);    
CHAR  IDFileType(FILE *);
  
ERR_OMA GetCurveBlkSz(FILE *hFile, const char * fName,
                              ULONG StartCurveOffset, USHORT Count,
                              PULONG BlkSz, BOOLEAN XcludMemData);
  
ERR_OMA MoveFileBlock(FILE *fhDst, const char * dstName,
                              ULONG DstOffset,
                              FILE *fhSrc, const char * srcName,
                              ULONG SrcOffset, ULONG Length);

ERR_OMA WriteTempCurveBlkToFile(CHAR * fName,
                                       USHORT DstCurve, LPMETHDR pMethdr,
                                       LPCURVEDIR CurveDir, USHORT Entry,
                                       USHORT Curve, USHORT CurveNumber);

// ERR_OMA WriteDirFromTemp(LPMETHDR pMethdr, LPCURVEDIR CurveDir,
//                                  PUSHORT pEntry, PUSHORT pCurve);

#endif  /* (ifdef FILESUTF) */
