/* -----------------------------------------------------------------------
/
/  formtabs.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/FORMTABS.H 1     6/18/99 1:52a Maynard $

 *    Rev 1.0   28 Aug 1991 17:39:02   cole
 * Initial revision.
*/

#ifndef FORMTABS_INCLUDED
#define FORMTABS_INCLUDED
  
#include "formtype.h"   // FORM
  
extern FORM * FormTable[];

// index constants for NorecFormTable[]
enum NOREC_INDEX { NOREC_LIVE_BACK_FORM, NOREC_ACCUM_BACK_FORM,
                   NOREC_MACRO_FORM
                 };

extern FORM * NorecFormTable[];

// initialize FormTable[] of addresses of forms
void initFormTable(void);

#endif    // FORMTABS_INCLUDED
