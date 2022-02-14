/* -----------------------------------------------------------------------
/
/  macnres.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/MACNRES.H 1     6/18/99 1:53a Maynard $
*/

#ifndef MACNRES_INCLUDED
#define MACNRES_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern char TempEntryName[];

void get_2flt_response(char * response_string, BOOLEAN err_flag);
void FltPushDetVal(USHORT Command);
void IntPushDetVal(USHORT Command);
void IntSetSingleDetVal(USHORT Command);
void FltSetSingleDetVal(USHORT Command);
void SetDoubleDetVal(USHORT Command, int *SetVal1, int *SetVal2 );
void PopSingleIntAndSetVal(int command);
void PopSingleFltAndSetVal(int command);
void DArea(void);
void DASD(void);
void DCArea(void);
void DGMinMax(void);
void LoadFileCurves(void);
void SaveFileCurves(void);
void InsCurve(void);
void InsCurveSet(void);
void PlotWindow(void);
void SPlotStyle(void);
void GPlotStyle(void);
void MacSetCal(void);
void MacGetCal(void);
void LoadBackground(void);
  
void DReadMem(void);
void DLoadMem(void);
void CreateCurve(void);
ERR_OMA create_new_curve(BOOLEAN XCal, SHORT Curves, SHORT Points,
                                SHORT SetIndex, SHORT CrvIndex, DOUBLE Yfill);

#endif  // MACNRES_INCLUDED
