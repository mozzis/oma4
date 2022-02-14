/* -----------------------------------------------------------------------
/
/  calib.h
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00        8-     January     1990
/  Worked on:  TLB      Version 1.01
/
/  $Header: /OMA4000/Include/CALIB.H 1     6/18/99 1:52a Maynard $
/
/ ----------------------------------------------------------------------- */
  
#ifndef CALIB_INCLUDED
#define CALIB_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

    /* a structure for the calibration point array. used by calib.c and
     * spgraph.c (for 1235/1236)
     */
typedef struct 
  {
   double x_value;
   double calibration_value;

  } CalibPoint;

    /* address of array. */                
extern CalibPoint * CurrentCalibrationPoint;
 
extern double CalCoeff[];
extern double ExcitationWaveLength;

extern BOOLEAN gathering_calib_points;
extern BOOLEAN InCalibForm;

extern char * WaveLengthOptions[];

extern UCHAR WaveLengthUnitTable[];

BOOLEAN verify_list_order(void);
void least_squares_fit(void);
int get_new_points(void);
void get_point_for_calibration(void);
void GetNewPointsInit(void);
int GetNewPointsExit(void);
FLOAT _pascal ApplyCalibrationToX(PFLOAT CalibCoeff, FLOAT X);
ERR_OMA ApplyCalibrationToCurve(LPCURVEDIR pCurveDir,
  SHORT EntryIndex, USHORT CurveIndex, PFLOAT CalibCoeff, CHAR CalibUnits);
ERR_OMA ExtractCalPointsFromCurve(LPCURVEDIR pCurveDir,
                                     SHORT EntryIndex, USHORT CurveIndex);
  
// new function for use by autopeak module
// add a calibration point and redisplay the scrolling form of cal points
// is_first TRUE means delete all previous calibration points and then
// add the new one.  is_first FALSE means just add a new point.
void add_calibration_point(double x_pix, double x_cal, BOOLEAN is_first);

// init FormTable with form addressed : X_CalibrationForm, X_CalScrollForm,
//                                      GetCalibPointForm
void registerXCalibForms(void);

#endif  // CALIB_INCLUDED
