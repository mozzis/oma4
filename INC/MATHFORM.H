/* -----------------------------------------------------------------------
/
/  mathform.h
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/MATHFORM.H 1     6/18/99 1:53a Maynard $
/
/ ----------------------------------------------------------------------- */

#ifndef MATHFORM_INCLUDED
#define MATHFORM_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

#ifndef _MAX_DIR
#include <stdlib.h>
#endif

extern USHORT min_count, max_count;

int VerifyConst(char String[]);
int VerifyCurveBlk(OP_BLOCK * Op);
int VerifyBlkOrConstProc(OP_BLOCK * Op);

// put & MathForm into FormTable[KSI_MATH_FORM]
void registerMathForm(void);

#endif  //  MATHFORM_INCLUDED
