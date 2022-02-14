/* -----------------------------------------------------------------------
/
/  macnres2.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/MACNRES2.H 1     6/18/99 1:53a Maynard $
 * 
 *    Rev 1.0   24 Jul 1991 08:44:30   cole
 * Initial revision.
/
*/

#ifndef MACNRES2_INCLUDED
#define MACNRES2_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

SHORT MakeNewTempCurveSet(SHORT SrcEntryIndex, USHORT SrcStartIndex,
                          USHORT Count);
void OperateOnCurves(SHORT lType, SHORT rType, SHORT Operator,
                     SHORT OperandCount);
void OperateOnScalars(SHORT Operator, SHORT OperandCount);
void OperateOnStrings(SHORT Operator, SHORT OperandCount);
void RunOperator(SHORT Operator);
void AssignTo(void);
void Assign(void);

void MacChangeCurveSize(void);
void MacReadChar(void);
void ReadProc(BOOLEAN ToEOLN);
void WriteProc(BOOLEAN ToEOLN);
void GetCommandPrompt(void);
void SetCommandPrompt(void);
void minmax_curve(unsigned SetIndex, unsigned CurveIndex,
                  unsigned Count, float *min, float *max);
  
#endif  // MACNRES2_INCLUDED
