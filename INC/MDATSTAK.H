/* -----------------------------------------------------------------------
/
/  mdatstak.h
/
/  $Header: /OMA4000/Include/MDATSTAK.H 1     6/18/99 1:53a Maynard $
 * 
 *    Rev 1.0   24 Jul 1991 08:50:24   cole
 * Initial revision.
/
/ ----------------------------------------------------------------------- */
  
#ifndef MDATSTAK_INCLUDED
#define MDATSTAK_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif
  
int getStackDepth(void);
void PushToDataStack(void * ValuePtr, int Type, BOOLEAN PointerRef);
int  PopFromDataStack(void * Parameter, int RequiredType);
int  IndexDataStack(int StackDepth, void * Parameter, int RequiredType);
int  DataStackPeek(int StackDepth);
void RemoveItemsFromDataStack(int count);
void InitDataStack(void);
BOOLEAN PopScalarFromDataStack(PVOID Value, SHORT Type);
BOOLEAN ScalarIndexDataStack(SHORT Index, PVOID Value, SHORT Type);
BOOLEAN ScalarDataStackPeek(SHORT StackIndex, PSHORT ReturnType);
void FillAndPushCurve(SHORT CurveType);
BOOLEAN allocateDataStack(void);
void releaseDataStack(void);
  
#endif  // MDATSTAK_INCLUDED
