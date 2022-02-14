/* -----------------------------------------------------------------------
/
/  macrores.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/MACRORES.H 1     6/18/99 1:53a Maynard $
*/
  
#ifndef MACRORES_INCLUDED
#define MACRORES_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

// execute a procedure from the command list
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void procedureExec(int commandListIndex);

BOOLEAN isMacPlayMenu(int index);

BOOLEAN isMacPlayForm(int index);

BOOLEAN isMacPlayField(int index);

void MacBadParam(int param);

void InitComAddresses(void);

#endif  //  MACRORES_INCLUDED
