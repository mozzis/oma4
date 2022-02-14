/* -----------------------------------------------------------------------
/
/  maccomp.h
/
/  $Header: /OMA4000/Include/MACCOMP.H 1     6/18/99 1:53a Maynard $
 * 
 *    Rev 1.0   24 Jul 1991 08:42:10   cole
 * Initial revision.
/
/ ----------------------------------------------------------------------- */
  
#ifndef MACCOMP_INCLUDED
#define MACCOMP_INCLUDED

#include "macruntm.h"  // SemanticRecord
 
void init_sem(void);
void apply(int ProductionFlag, SemanticRecord * ReductionSemRec);
void FORGET_ALL(void);
long RelativeCodeOffset(OPERATOR * Location);
  
#endif  // MACCOMP_INCLUDED

