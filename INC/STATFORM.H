/* -----------------------------------------------------------------------
/
/  statform.h
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/ ----------------------------------------------------------------------- */

#ifndef STATFORM_INCLUDED
#define STATFORM_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

#ifndef MATHFORM_INCLUDED
  #include "mathform.h"
#endif

// put &StatForm into FormTable[KSI_STAT_FORM]
void registerStatForm(void) ;

#endif  //  STATFORM_INCLUDED
