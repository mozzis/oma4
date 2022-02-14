/* -----------------------------------------------------------------------
/
/  macrofrm.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/MACROFRM.H 1     6/18/99 1:53a Maynard $
*/

#ifndef MACROFRM_INCLUDED
#define MACROFRM_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern char CommandInput[];
extern char CommandOutput[];
extern int * pBufLen;
extern PCHAR pDefaultName;
extern int buflen;
extern char DefaultName[];
  
void RunMacroForm(USHORT Dummy);

void registerMacroForm(void);
void displayMacroForm(void);
void LocateMacroForm(void);

#endif  //  MACROFRM_INCLUDED
