/* -----------------------------------------------------------------------
/
/   autoscal.h
/
/  Copyright (c) 1989,  EG&G Princeton Applied Research
/
/  $Header: /OMA4000/Include/AUTOSCAL.H 1     6/18/99 1:52a Maynard $
/  $Log: /OMA4000/Include/AUTOSCAL.H $
 * 
 * 1     6/18/99 1:52a Maynard
 * 
 *    Rev 1.0   07 Jan 1992 11:45:32   cole
 * Initial revision.
/
/ ----------------------------------------------------------------------- */
  
#ifndef AUTOSCAL_INCLUDED
#define AUTOSCAL_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

ERR_OMA AutoScalePlotBox(LPCURVEDIR, PLOTBOX *, SHORT);

SHORT ScalePlotGraph(USHORT Dummy);
SHORT ScaleXAxis(USHORT Dummy);
SHORT ScaleYAxis(USHORT Dummy);
SHORT ScaleZAxis(USHORT Dummy);

void MacAutoScale(void);
void MacScaleX(void);
void MacScaleY(void);
void MacScaleZ(void);

#endif  //  AUTOSCAL_INCLUDED
