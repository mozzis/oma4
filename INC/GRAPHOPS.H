/* -----------------------------------------------------------------------
/
/  graphops.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/GRAPHOPS.H 1     6/18/99 1:52a Maynard $
*/

#ifndef GRAPHOPS_INCLUDED
#define GRAPHOPS_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern BOOLEAN DoLiveDisk;
extern int GraphExitKey;

void GraphEscape(USHORT Dummy);

void InitGraphOps(void);
unsigned char ExitGraphOps(void);
unsigned char GraphOps(void);

SHORT SelectPlotForWindow(USHORT Dummy);
SHORT SelectWindow(USHORT Dummy);

void HotPlotWindow(USHORT Dummy);
void HotPlotScreen(USHORT Dummy);
void HotAutoPeakStart(USHORT Dummy);
void HotAutoPeakEnd(void);

void MacEnterGraph(void);
void MacLeaveGraph(void);

void MacReplotWindow(void);
void MacReplotScreen(void);

void ContractXAxis(USHORT Dummy);
void ContractYAxis(USHORT Dummy);
void ExpandXAxis(USHORT Dummy);
void ExpandYAxis(USHORT Dummy);

void MacExpandX(void);
void MacExpandY(void);
void MacContractX(void);
void MacContractY(void);

#endif  //  GRAPHOPS_INCLUDED
