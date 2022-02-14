/* -----------------------------------------------------------------------
/
/  spgraph.h
/
/  Copyright (c) 1991,  EG&G Instruments Inc.
/
*  $Header: /OMA4000/Include/SPGRAPH.H 1     6/18/99 1:53a Maynard $
*  $Log: /OMA4000/Include/SPGRAPH.H $
 * 
 * 1     6/18/99 1:53a Maynard
 * 
 *    Rev 1.8   09 Jan 1992 14:28:30   cole
 * Add include eggtype.h; delete include omatyp.h, syserror.h, forms.h
 * 
 *    Rev 1.7   09 Sep 1991 16:06:46   cole
 * Change comments to reflect file name of spgraph.h instead of m1235.h.
 * Add Header and Log comment lines. Remove externs for FORM, DATA, and
 * EXEC_DATA types. Add prototype for registerSPGraphForm().
*/
/* ----------------------------------------------------------------------- */
  
#ifndef SPGRAPH_INCLUDED
#define SPGRAPH_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern BOOLEAN InM1235Form;

SHORT ActonSpectrograph(void);
float DataPtToChnl(SHORT  DataPt);
float EvalPoly(float  x);
float DataPtToLambda(SHORT  DataPt);
SHORT _pascal SolvePoly(float  Wavelength);
SHORT Output1235CMD(char  *cmd);
SHORT Get1235Response(char  *st,SHORT  maxlen);
SHORT Store1235Settings(void);
void  RedoCalCoeffs(void);
SHORT OnePtResetOffset(void);
SHORT ResetOffset(void);
SHORT ChangeGrating(SHORT  grating);
SHORT Reset1235(void);
void Release1235(void);
SHORT SetCenterLambda(void);
SHORT Set1235(void);
SHORT Redo1235(void);
void registerSPGraphForm(void);
SHORT MacSetWV(float Wavelen);
SHORT MacChooseSpectrograph(SHORT model);
SHORT MacOnePtResetOffset(float Chnl, float Wvlngth);
SHORT MacRedo1235(void);
SHORT MacSet1235(void);
void GetOffsetPoint(void);
SHORT MacChangeDispersion(BOOLEAN IsBlueToRed);

#endif  // SPGRAPH_INCLUDED
