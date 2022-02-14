/* -----------------------------------------------------------------------
/
/   symbol.h
/
/  Copyright (c) 1988,  EG&G Princeton Applied Research
/
/  Written by: JL       Version 1.00
/  Worked on:  DAI                           16 October     1989
/
/  symbol.h defines symbol plotting functions.
/
/ ----------------------------------------------------------------------- */
  
#ifndef SYMBOL_INCLUDED
#define SYMBOL_INCLUDED

#ifndef CGIBIND_INCLUDED
   #define CGIBIND_INCLUDED
   #include <cgibind.h>
#endif

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

void symbol(CXY *, char *, float, float, float);

SHORT AlignText(FLOAT, FLOAT, SHORT, SHORT, CHAR *, CXY *, FLOAT);

#endif // SYMBOL_INCLUDED
