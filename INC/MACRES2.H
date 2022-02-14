/* -----------------------------------------------------------------------
/
/  macres2.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/MACRES2.H 1     6/18/99 1:53a Maynard $
 *
 *    Rev 1.0   24 Jul 1991 08:49:00   cole
 * Initial revision.
/
*/
  
#ifndef MACRES2_INCLUDED
#define MACRES2_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

void DeleteTempMathFiles(void);
void CSCount(void);
void DCCount(void);
void MCCount(void);
void LoadMethod(void);
void SaveMethod(void);
void DelCurveSet(void);
void DelCurve(void);
void MacCreateCurveSet(void);
void GCurveSetIndex(void);
void GActiveWindow(void);
void GActivePlotSetup(void);
void SWindowStyle(void);
void SAccum(void);
void SLive(void);
void SFrameCapture(void);

DOUBLE DoBinMath(CURVEDIR*, SHORT*, USHORT*, USHORT*, DOUBLE*, SHORT,BOOLEAN);

ERR_OMA CreateCurveSet(char *Name, char *Path, char *Desc, int NIndex);

BOOLEAN DoBinom(OP_BLOCK *Src, OP_BLOCK *Dst, int Degree);
void DoStats(OP_BLOCK *Op, int FirstPoint, int PointCount, STAT_STRUCT *Stats);
void MacDoSmoothOrDeriv(void);

#endif  //  MACRES2_INCLUDED
