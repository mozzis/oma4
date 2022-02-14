/* -------------------------------------------------------------------------
/  FILE : ycalib.h 
/  
/  form and plot for baseline subtract
/
/  $Header: /OMA4000/Include/YCALIB.H 1     6/18/99 1:53a Maynard $
/  $Log: /OMA4000/Include/YCALIB.H $
 * 
 * 1     6/18/99 1:53a Maynard
/ --------------------------------------------------------------------------
*/
  
#ifndef YCALIB_INCLUDED
#define YCALIB_INCLUDED
  
#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

// return TRUE iff y calibration menu is active
BOOLEAN ycalib_active(void);
  
// plot special y calibration curves for curve Z in the y calibration plot
// box.  EntryIndex and FileCurveNumber define the location of the curve
// data in MainCurveDir.  TCurveHdr is the curve header for the curve data.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA ycalib_plot(float Z, USHORT EntryIndex,
                           USHORT CurveNumber, CURVEHDR * TCurveHdr);

BOOLEAN LoadLampFile(char * Name);
BOOLEAN SaveLampFile(char * Name);
BOOLEAN AddYcalibPair(FLOAT Wlen, FLOAT Value);
BOOLEAN DeleteYCalibPair(SHORT index);
BOOLEAN DeleteLampData(void);
BOOLEAN GenCorrexCurve(char *LampName, char *CorrName);

// init FormTable[] with & baseline_sub_form and & knots_form
void registerYCalibForms(void);

#endif   // YCALIB_INCLUDED
