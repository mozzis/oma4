/***************************************************************************/
/*                                                                         */
/*  tempdata.h                                                             */
/*                                                                         */
/*  copyright (c) 1989, EG&G Instruments Inc.                              */
/*                *** OMA 35 Version ***                                   */
/*                                                                         */
/*       Routines to use VDISK or normal disk files as temporary storage.  */
/*                                                                         */
/*    WARNING! Only one CURVEDIR structure per program should try to use   */
/*    these functions!                                                     */
/*                                                                         */
/* last change:                                                            */
/*
*  $Header: /OMA4000/Include/TEMPDATA.H 1     6/18/99 1:53a Maynard $
/***************************************************************************/
  
#ifndef TEMPDATA_INCLUDED
#define TEMPDATA_INCLUDED
  
#include <stdio.h>   // type FILE

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif
  
extern CHAR *TempFileBuf;
extern FILE *hTempFile;
extern ULONG TempFileSz;
  
ERR_OMA ChangeTempFileSize(LONG);
LONG tempFileSize(void);
void tempFileIncrementSize(LONG sizeIncrement);
ERR_OMA GetTempCurveOffset(USHORT, PULONG);
ERR_OMA GetTempFileLen(CURVEDIR *, PULONG);
ERR_OMA DelTempCurve(CURVEDIR *, SHORT, USHORT);
ERR_OMA DelMultiTempCurve(CURVEDIR *, USHORT, USHORT, USHORT);
ERR_OMA InsertTempCurve(CURVEDIR *, SHORT, USHORT, SHORT, USHORT);
ERR_OMA InsertMultiTempCurve(CURVEDIR *, SHORT, USHORT, SHORT,
                                     USHORT, USHORT);
ERR_OMA ChangeCurveSize(CURVEDIR *, SHORT, USHORT, USHORT, CHAR);
ERR_OMA ReadTempCurvehdr(CURVEDIR *, USHORT, USHORT, CURVEHDR *);
ERR_OMA WriteTempCurvehdr(CURVEDIR *, USHORT, USHORT, CURVEHDR *);
ERR_OMA AppendFileCurveToTemp(FILE *, const char *,CURVEDIR *,
                                      SHORT, USHORT);
ERR_OMA InsertFileCurveInTemp(FILE *, const char *, CURVEDIR *,
                                      SHORT, USHORT, USHORT, PULONG);
ERR_OMA InsertMultiFileCurveInTemp(FILE *, const char *, CURVEDIR *,
                                           SHORT, USHORT, USHORT, PULONG,
                                           USHORT);
ERR_OMA AppendFileCurveBlkToTemp(CURVEDIR *, USHORT, PUSHORT);
ERR_OMA InsertFileBlkInTemp(CURVEDIR *, CURVEDIR *, SHORT, USHORT,
                                    SHORT, USHORT, USHORT);
ERR_OMA AddCurveSpaceToTempBlk(CURVEDIR *, SHORT, USHORT, USHORT,
                                       CURVEHDR *);
ERR_OMA CreateTempFileBlk(CURVEDIR *, PSHORT, PCHAR, PCHAR, PCHAR,
                                  USHORT, ULONG, USHORT, CURVEHDR *, CHAR,
                                  USHORT);
ERR_OMA DelTempFileBlk(CURVEDIR *, SHORT);
  
ERR_OMA ReadDirToTemp(CURVEDIR *, PUSHORT, PUSHORT);
ERR_OMA ReadFileToTemp(PCHAR Name, PCHAR Path,
                               USHORT StartIndex, USHORT Count,
                               CURVEDIR * CurveDir, PUSHORT pEntryIndex,
                               PUSHORT pCurveIndex);
  
ERR_OMA InsertTempCurvesInFile(FILE *, const char *, CURVEDIR *,
                                       USHORT, USHORT, USHORT, USHORT,
                                       USHORT);
  
// Move all of one curve to another if they are the same size.
// Return TRUE iff a replacement was actually done.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN ReplaceCurve(CURVEDIR * CurveDir, SHORT SrcEntryIndex,
                      USHORT SrcCurveIndex, SHORT DstEntryIndex,
                      USHORT DstCurveIndex);

ERR_OMA GetWholeCurveSetSz(USHORT BlkIndex, USHORT Start, USHORT Count,
                                  PULONG pBlkSz, BOOLEAN XcludMemData);
#endif
