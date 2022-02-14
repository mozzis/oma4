/* -----------------------------------------------------------------------
/
/  macnres3.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/MACNRES3.H 1     6/18/99 1:53a Maynard $
 * 
 *    Rev 1.0   24 Jul 1991 08:47:34   cole
 * Initial revision.
/
*/

#ifndef MACNRES3_INCLUDED
#define MACNRES3_INCLUDED

#include "macruntm.h"  // CURVE_REF

void AssignToCurveSetHdr(CURVE_REF lCurve);
void AssignToCurveHdr(CURVE_REF lCurve);
void AssignToPoint(CURVE_REF lCurve);
void AssignToCurveClass(void);
USHORT MakeNewCurveSet(void);

#endif  // MACNRES3_INCLUDED
