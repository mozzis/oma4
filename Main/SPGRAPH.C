/* -----------------------------------------------------------------------
/
/  spgraph.c
/
/  Copyright (c) 1991,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/spgraph.c_v   1.22   06 Jul 1992 10:37:04   maynard  $
/  $Log:   J:/logfiles/oma4000/main/spgraph.c_v  $
/
/ -------------------------------------------------------------------------
*/

#include <bios.h>
#include <conio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
//#include <time.h>

#include "spgraph.h"
#include "calib.h"
#include "di_util.h"
#include "curvdraw.h"
#include "device.h"
#include "cursor.h"
#include "oma4driv.h"
#include "detsetup.h"
#include "ksindex.h"
#include "helpindx.h"
#include "multi.h"
#include "formwind.h"
#include "gpibcom.h"
#include "formtabs.h"
#include "omaform.h"   // COLORS_DEFAULT
#include "omameth.h"   // InitialMethod
#include "omaerror.h"
#include "crvheadr.h"  // ADJ_NM
#include "curvbufr.h"  // RegenXData();
#include "plotbox.h"
#include "forms.h"
#include "splitfrm.h"
#include "graphops.h"

/* set up to use the InitialMethod pointer */

#define F0 0.0F
#define MAX_GRATING 8

#define M1235_ADDR  11
#define SPG_PORT 1
#define SERIAL_ERR 0x8000
#define GPIB_ERR  0x8000
#define GP_OUTPUT_READY 0x80
#define GP_COMMAND_DONE 0x01
#define GP_OK GP_OUTPUT_READY | GP_COMMAND_DONE

static enum IFC_TYPES {  IFC_NONE, IFC_GPIB, IFC_SERIAL1, IFC_SERIAL2 };
static int Interface = IFC_NONE;
static char GPIBBuf[120];
static float OffsetPixel;
static float OffsetWL;
static char M1235Flag = 0;       /* 1 if 1235, 2 if 1236; 0 if not */
static int GrNo = 0;             /* currently selected grating number*/
static float RelPixelWidth;      /* width of detector pixel in 25 um units.*/
static float WV;                 /* center of current window */
static float W0;                 /* current 1235 wavelength reading */
float M1235_W0;                  /* current 1235 wavelength reading */
float P0[3];                     /* Channel number corresponding to
                                  * above wavelength. In perfect world,
                                  * would be center of diode array. */
static float GroovesPmm [3] = { 0, 0, 0, };
static float *GrPtr;              /* if nonNULL, points to grating info */
static int Grating = 0;          /* selected grating number */

  /* Converts data point in track into physical channel in array.
   * Data points are zero based, and the X0's in GetParam are 1 based.
   * DeltaX is number of channels in
   * a group, and ranges from 1 (never 0?) to size of array.
   * The channel number returned will be zero based and will
   * be fractional for even sized groups: e.g., for group size 2,
   * DataPt 0 = Chnl 0.5, DataPt 1 = Chnl 3.5, etc...
   * The channel number for odd sized groups will be integral:
   * for group size 1, DataPt 0 = Chnl 0, 1 = 1; for group size 3
   * DataPt 0 = Chnl 1, DataPt 1 = Chnl 4, etc...
   */

float DataPtToChnl(int DataPt)
{
  float X_0, DeltaX, Mode, detector;

  GetParam(DC_DMODEL, &detector);
  if (detector == RAPDA)
    return (RapdaDataPtToPixel (DataPt));
  else
    {
    /* Data points 1 based in drivers. In contiguous, use slot 0. */ 
    GetParam(DC_PNTMODE, &Mode);
                                            
    if (Mode)
      {
      X_0 = (float)(DataPt + 1); 
      SetParam(DC_POINT, X_0);        /* Find the correct slot */
      DataPt = 0;
      }
                                      /* Find the groupsize. */
    GetParam(DC_DELTAX, &DeltaX);
    if (!DeltaX)                      /* DeltaX must be 1 or greater */
      DeltaX++;
    GetParam(DC_X0, &X_0);            /* Now get X0 */
  /* return DataPt's "chnl". */
  return (X_0 - 1.5F + DeltaX * ((float)DataPt + 0.5F));
  }
}

  /* given x in data pt#, returns wavelength.*/
  /* Converts a floating point x value into the associated wavelength
   * by evaluating the calibration polynomial.  The x value can be
   * fractional as for grouping by even numbers, or from finding the
   * center of the spectrograph's window: P0[#].
   */
float EvalPoly(float x) 
{
  int i;
  float Tmp = F0,
        *CalCoeffPtr = InitialMethod->CalibCoeff[0];
  if (CalCoeffPtr)                    /* watch out for jokers with null */
    {                                 /*  y = (a0 + x(a1 + x(a2 + x(a3)))),
                                       *  y = a0 + a1ùx + a2ùx^2 + a3ùx^3 */
    for (i = 3; i >=0; i--)
      Tmp = (float)CalCoeffPtr[i] + (Tmp * x);
    }
  if (!Tmp)                           /* if uncal'd, return data point# */
    Tmp = x;
  return (Tmp);
}

  /* Converts data point into a wavelength by first finding it's
   * corresponding "channel" and then converting the channel to
   * wavelength.
   */
float DataPtToLambda(int DataPt)
{
  return (EvalPoly(DataPtToChnl(DataPt)));
}

/* Finds pixel with wavelength value closest to Wavelength by bracket&halve */

int _pascal SolvePoly(float Wavelength)
{
  int IncVal,
      DeltaPxls;
  int LeftPxl, NewLeft;
  float tmp1, tmp2,
        *CalCoeffPtr = InitialMethod->CalibCoeff[0];
                                      /* If wavelength increases with pixel
                                       * value, IncVal = 1; 0 otherwise */
  IncVal = CalCoeffPtr[1] > (CalCoeffPtr[2] + CalCoeffPtr[3]);
  LeftPxl = 0;                        /* Initialize variables.*/
                                      /* twice as many as are really there.*/
  GetParam(DC_POINTS, &tmp1);
  DeltaPxls = 2 * (int)tmp1;
  while ((DeltaPxls > 1)   /* if 0 or 1, we're there */
                            /* or if outside the wavelength region, done. */
             && ((IncVal) ? (DataPtToLambda (LeftPxl) < Wavelength)
                          : (DataPtToLambda (LeftPxl) > Wavelength)))
    {
                                      /* divide region in half. */
    DeltaPxls = (DeltaPxls + 1) / 2;
    NewLeft = LeftPxl + DeltaPxls;    /* NewLeft = center pixel */
    tmp2 = DataPtToLambda (NewLeft);  /* tmp2 = center wavelength */
                                      /* see which half is valid; if it's
                                       * the right half, reset left edge.*/
      if ((IncVal) ? (tmp2 <= Wavelength) : (tmp2 >= Wavelength))
        LeftPxl = NewLeft;
    }                       /* difference between next and given value */
   tmp1 = DataPtToLambda (LeftPxl + 1) - Wavelength;
                            /* difference between this and given value */ 
   tmp2 = Wavelength - DataPtToLambda (LeftPxl);

   if ((IncVal) ? (tmp1 < tmp2) : (tmp1 > tmp2))
      LeftPxl++;           /* if closer, increment left pixel */
   return (LeftPxl);
}

/******************************* Form Data section **************************/
BOOLEAN InM1235Form = FALSE; /* flag for running splitform, macro playback  */
static SHORT OldLocus;
static BOOLEAN NoAction;     /* flag to not redraw plot unless necessary    */
enum { DGROUP_DO_STRINGS  = 1, DGROUP_GENERAL, DGROUP_CODE };

                                   /*           111111111122222222223    */
                                   /* 0123456789012345678901234567890    */
static char GratingCoverage[] =    { "Coverage:                                   " };
static char UnitsString[] =        { "(Nanometers)  " };
static char GratingLabels[3][32] = { " 1                            ",
                                     " 2                            ",
                                     " 3                            ", };
static FORM OffsetBox_Form;

DATA DO_STRING_Registry[] = {
 /*  0 */  { "Grating Selection",   0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /*  1 */  { "Center Wavelength",   0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /*  2 */  { "Pixel",               0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /*  3 */  { "Wavelength",          0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /*  4 */  { "Spectrograph",        0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /*  5 */  {  GratingCoverage,      0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /*  6 */  { "Grating",             0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /*  7 */  { "Grooves/mm",          0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /*  8 */  { "Coverage",            0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /*  9 */  { GratingLabels[0],      0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 10 */  { GratingLabels[1],      0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 11 */  { GratingLabels[2],      0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 12 */  { "Go",                  0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 13 */  { "Reset",               0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 14 */  { "Identify",            0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 15 */  { "Detector Offset",     0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 16 */  { "Value",               0, DATATYP_STRING, DATAATTR_PTR, 0 },
};

static char * SpectrographOptions[] =
              {"???", "M1235", "M1236", "M1237", "M1239" };

static char * GratingOptions[] = { "1",  "2",  "3",};

static char * DispersionOptions[] = { "Blue<->Red", "Red<->Blue" };
static int DispersionToggle = 0;
static int GetOffWLToggle = 0;
static int SpecTog = 0;           /* 0=none, 1=1235, 2=1236 .. */
int SelectState = FALSE;

static DATA GENERAL_Registry[] = {
 /* 0 */ { GratingOptions,      0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 1 */ { &Grating,            0, DATATYP_INT,          DATAATTR_PTR, 0 },
 /* 2 */ { &WV,                 0, DATATYP_FLOAT,        DATAATTR_PTR, 0 },
 /* 3 */ { SpectrographOptions, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 4 */ { &SpecTog,            0, DATATYP_INT,          DATAATTR_PTR, 0 },
 /* 5 */ { &SelectState,        0, DATATYP_INT,          DATAATTR_PTR, 0 }, 
 /* 6 */ { &OffsetBox_Form,     0, DATATYP_VOID,         DATAATTR_PTR, 0 },
 /* 7 */ { &OffsetPixel,        0, DATATYP_FLOAT,        DATAATTR_PTR, 0 },
 /* 8 */ { &OffsetWL,           0, DATATYP_FLOAT,        DATAATTR_PTR, 0 },
 /* 9 */ { &DispersionToggle,   0, DATATYP_INT,          DATAATTR_PTR, 0 }, 
 /*10 */ { DispersionOptions,   0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /*11 */ { &GetOffWLToggle,     0, DATATYP_INT,          DATAATTR_PTR, 0 }, 
 /*12 */ { WaveLengthOptions,   0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
};

static int ChangeDispersion(void);
static BOOLEAN M1235FormInit(void);
static BOOLEAN M1235FormExit(void);
static BOOLEAN GetOffsetPointFormInit(void);
static BOOLEAN GetOffsetPointFormExit(void);
static SHORT Load1235Registry(void);
static void  AdjustFieldLabels(void);

static EXEC_DATA CODE_Registry[] = {
   /* 0 */ { Load1235Registry,       0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 1 */ { Redo1235,               0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 2 */ { Set1235,                0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 3 */ { M1235FormInit,          0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 4 */ { M1235FormExit,          0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 5 */ { OnePtResetOffset,       0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 6 */ { ChangeDispersion,       0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 7 */ { GetOffsetPointFormInit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 8 */ { GetOffsetPointFormExit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
};

static enum {
            FLD_OFFSET_BOX,

            LBL_GR_SEL,
            LBL_CTR_WVL,
            
            LBL_OFF_PXL,
            LBL_OFF_WVL,
            
            LBL_SPGRAPH,
            
            LBL_COVRG_SEL,

            LBL_GR_NO,
            LBL_GROOVES,
            LBL_COVRG_ALL,

            LBL_COVRG_1,
            LBL_COVRG_2,
            LBL_COVRG_3,

            FLD_GR_SEL,
            FLD_CTR_WVL,
            FLD_GO,
            
            FLD_OFF_PXL,
            FLD_OFF_WVL,
            FLD_OFF_RESET,

            FLD_SPGRAPH,
            FLD_SPGRAPH_ID,
            FLD_DISP,
            };
 
static FIELD M1235FormFields[] = {
  d_field_(FLD_OFFSET_BOX,           /* Box for ResetOffset controls */
           FLDTYP_FORM,
           FLDATTR_DISPLAY_ONLY,
           0,                               
           DGROUP_GENERAL, 6,        /* OffsetBox_Form */
           5, 1, 0),                       
  label_field(LBL_GR_SEL,       
              DGROUP_DO_STRINGS, 0,   /* "Grating Selection" */
              1, 2, 18),
  label_field(LBL_CTR_WL,
              DGROUP_DO_STRINGS, 1,   /* "Center Wavelength" */
              2, 2, 18),                       
  label_field(LBL_OFF_PXL,
              DGROUP_DO_STRINGS, 2,  /* "Pixel" */
              6, 4, 5),
  label_field(LBL_OFF_WVL,
              DGROUP_DO_STRINGS, 3,  /* "Wavelength" */
              6, 11, 10),
  label_field(LBL_SPGRAPH,
              DGROUP_DO_STRINGS, 4,  /* "Spectrograph" */
              1, 36, 12),
  label_field(LBL_COVRG_SEL,
              DGROUP_DO_STRINGS, 5,  /* "Coverage  435.8nm - 546.1nm" */
              3, 36, 42),
  label_field(LBL_GR_NO  ,
              DGROUP_DO_STRINGS, 6,  /* "Grating" */
              5, 38, 7),
  label_field(LBL_GROOVES,
              DGROUP_DO_STRINGS, 7,  /* "Grooves/mm" */
              5, 48, 11),
  label_field(LBL_COVRG_ALL,
              DGROUP_DO_STRINGS, 8,  /* "Coverage" */
              5, 60, 8),
  label_field(LBL_COVRG_1,
              DGROUP_DO_STRINGS, 9,  /* "1        0.01512       444.5" */
              6, 40, 31),
  label_field(LBL_COVRG_2,
              DGROUP_DO_STRINGS, 10, /* "2        0.231522      444.5" */
              7, 40, 31),
  label_field(LBL_COVRG_3,
              DGROUP_DO_STRINGS, 11, /* "3        0.341655      444.5" */
              8, 40, 31),
  field_set(FLD_GR_SEL,              /* grating selection */
            FLDTYP_TOGGLE,
            FLDATTR_REV_VID,
            KSI_SPEC_GRATING,
            X_CALSCROLLFIELD_HBASE + 1,
            DGROUP_GENERAL, 1,       /* Grating */
            DGROUP_GENERAL, 0,       /* GratingOptions */
            0, 0,
            0, 3,
            1, 21, 3,
            EXIT,       FLD_GR_SEL, FLD_OFF_PXL, FLD_CTR_WVL, 
            FLD_GR_SEL, FLD_GR_SEL, FLD_DISP,    FLD_SPGRAPH),
  field_set(FLD_CTR_WVL,              /* Center Wavelength */
            FLDTYP_STD_FLOAT,
            FLDATTR_REV_VID,
            KSI_SPEC_GETLAM,
            X_CALSCROLLFIELD_HBASE + 2,
            DGROUP_GENERAL, 2,       /* CenterWaveLength */
            0, 0,
            0, 0,
            2, 0,
            2, 21, 6,
            EXIT,        FLD_CTR_WVL, FLD_GR_SEL,   FLD_GO,
            FLD_CTR_WVL, FLD_CTR_WVL, FLD_SPGRAPH, FLD_SPGRAPH_ID),
  field_set(FLD_GO,                  /* Go */
            FLDTYP_SELECT,
            FLDATTR_REV_VID,
            KSI_SPEC_GO,
            X_CALSCROLLFIELD_HBASE + 3,
            DGROUP_GENERAL, 5,       /* SelectState */
            DGROUP_DO_STRINGS, 12,   /* "GO" */
            DGROUP_CODE, 2,          /* change grating, wavelength */
            0, 0,
            3, 21, 4,
            EXIT,   FLD_GO, FLD_CTR_WVL, FLD_OFF_PXL,
            FLD_GO, FLD_GO, FLD_CTR_WVL, FLD_OFF_PXL),
  field_set(FLD_OFF_PXL,              /* Offset Pixel */
            FLDTYP_STD_FLOAT,
            FLDATTR_REV_VID,
            KSI_SPEC_OFFPXL,
            X_CALSCROLLFIELD_HBASE + 2,
            DGROUP_GENERAL, 7,       /* OffsetPixel */
            0, 0,
            0, 0,
            2, 0,
            7, 3, 6,
            EXIT,        FLD_OFF_PXL,  FLD_GO, FLD_GR_SEL,
            FLD_OFF_PXL, FLD_OFF_PXL,  FLD_GO, FLD_OFF_WVL),
  field_set(FLD_OFF_WVL,             /* Offset Wavelength */
            FLDTYP_STD_FLOAT,
            FLDATTR_REV_VID,
            KSI_SPEC_OFFWVL,
            X_CALSCROLLFIELD_HBASE + 2,
            DGROUP_GENERAL, 8,       /* OffsetWL */
            0, 0,
            0, 0,
            2, 0,
            7, 12, 6,
            EXIT,        FLD_OFF_WVL, FLD_GO,      FLD_GR_SEL,
            FLD_OFF_WVL, FLD_OFF_WVL, FLD_OFF_PXL, FLD_OFF_RESET),
  field_set(FLD_OFF_RESET,           /* Reset Offset */
            FLDTYP_SELECT,
            FLDATTR_REV_VID,
            KSI_SPEC_SETOFFSET,
            X_CALSCROLLFIELD_HBASE + 4,
            DGROUP_GENERAL, 5,       /* SelectState */
            DGROUP_DO_STRINGS, 13,   /* "Reset" */
            DGROUP_CODE, 5,          /* OnePtResetOffset */
            0, 0,
            7, 21, 5,
            EXIT,          FLD_OFF_RESET, FLD_GO,      FLD_GR_SEL,
            FLD_OFF_RESET, FLD_OFF_RESET, FLD_OFF_WVL, FLD_SPGRAPH),
  field_set(FLD_SPGRAPH,
            FLDTYP_TOGGLE, FLDATTR_REV_VID,
            KSI_SPEC_SELECT,
            X_CALSCROLLFIELD_HBASE + 0,
            DGROUP_GENERAL, 4,    /* Spectrograph  */
            DGROUP_GENERAL, 3,    /* Spec options. */
            DGROUP_CODE, 0,       /* Load1235Registry  */
            0, 2,
            1, 49, 5,
            EXIT,        FLD_SPGRAPH, FLD_SPGRAPH, FLD_SPGRAPH,
            FLD_SPGRAPH, FLD_SPGRAPH, FLD_GR_SEL,  FLD_SPGRAPH_ID),
  field_set(FLD_SPGRAPH_ID,
            FLDTYP_SELECT,
            FLDATTR_REV_VID,
            KSI_SPEC_RESET,
            X_CALSCROLLFIELD_HBASE + 5,
            0, 0,
            DGROUP_DO_STRINGS, 14, /* "Identify" */
            DGROUP_CODE, 1,        /* Redo1235 */
            0, 0,
            1, 56, 8,
            EXIT,           FLD_SPGRAPH_ID, FLD_SPGRAPH_ID, FLD_SPGRAPH_ID,
            FLD_SPGRAPH_ID, FLD_SPGRAPH_ID, FLD_SPGRAPH,    FLD_DISP),
  field_set(FLD_DISP,              /* dispersion toggle */
            FLDTYP_TOGGLE,
            FLDATTR_REV_VID,
            KSI_SPEC_DISP,
            X_CALSCROLLFIELD_HBASE + 7,
            DGROUP_GENERAL, 9,     /* Dispersion */
            DGROUP_GENERAL, 10,    /* DispersionOptions */
            DGROUP_CODE, 6,        /* ChangeDispersion */
            0, 2,
            1, 66, 10,
            EXIT,           FLD_DISP,   FLD_DISP,       FLD_DISP, 
            FLD_SPGRAPH_ID, FLD_GR_SEL, FLD_SPGRAPH_ID, FLD_GR_SEL),
  };
  
FORM M1235Form = {
    0, 0,
    FORMATTR_BORDER | FORMATTR_FIRST_CHAR_ERASE | FORMATTR_VISIBLE |
      FORMATTR_FULLWIDTH,
    0, 0, 0,
    2, 0, 10, 80,
    0, 0,
    { DGROUP_CODE, 3 },  /* M1235FormInit*/
    { DGROUP_CODE, 4 },  /* M1235FormExit*/
    COLORS_DEFAULT,
    0, 0, 2, 0,
    sizeof(M1235FormFields) / sizeof (M1235FormFields [0]),
    M1235FormFields,
    KSI_SPGRAPH_FORM,
    0, DO_STRING_Registry, GENERAL_Registry, (DATA *)CODE_Registry, 0, 0
  };
  
static FIELD OffsetBox_Fields[] = {
   label_field(0,
   DGROUP_DO_STRINGS, 15,                 /* "Detector Offset" */
   0, 6, 15)
};   

static FORM OffsetBox_Form =
  {
  0, 0,
  FORMATTR_BORDER | FORMATTR_VISIBLE,     /* attrib; */
  0, 0, 0,    
  5, 1, 4, 28,                            /* row, column, sz_rows, sz_cols */
  0, 0,
  { 0, 0 },                               /* init_function; */
  { 0, 0 },                               /* exit_function; */
  COLORS_DEFAULT,                         /* color_set_index; */
  0, 0, 0, 0,
  sizeof(OffsetBox_Fields)/sizeof(FIELD), /* number_of_fields; */
  OffsetBox_Fields,                       /* fields; */
  KSI_NO_INDEX,                           /* FormIndex for record/playback */
  0, DO_STRING_Registry, 0, 0, 0, 0       /* dataRegistry[6]; 0 means no entry */
  };

static FIELD GetOffsetPointFormFields[] = {
   { FLDTYP_TOGGLE,
     FLDATTR_RJ | FLDATTR_DISPLAY_ONLY,
     KSI_NO_INDEX,
     0,
     {DGROUP_GENERAL, 11},    // CalibUnitsFlag
     {DGROUP_GENERAL, 12},    // WaveLengthOptions
     {0, 0},
     {0, 13},
     1, 1, 14,
     { 1, 0, 0, 0, 0, 0, 0, 0 } },
   { FLDTYP_STRING,
     FLDATTR_DISPLAY_ONLY,
     KSI_NO_INDEX,
     0,
     {DGROUP_DO_STRINGS, 16}, // "Value"
     {0, 0},
     {0, 0},
     {0, 0},
     1, 16, 5,
     { 1, 0, 0, 0, 0, 0, 0, 0 } },
   { FLDTYP_STD_FLOAT,
     FLDATTR_REV_VID,
     KSI_XCAL_GP_VAL,
     GET_CALIB_PT_FIELD_HBASE+0,
     {DGROUP_GENERAL, 8 },    // OffsetWL
     {0, 0},					   
     {0, 0},
     {5, 0},
     1, 22, 10,
     { -3, -3, 0, 0, 0, 0, 0, 0 } }
};

static FORM GetOffsetPointForm = {
   0, 0,
   FORMATTR_EXIT_RESTORE | FORMATTR_BORDER |
   FORMATTR_FIRST_CHAR_ERASE | FORMATTR_VISIBLE,
   0, 0, 0,
   6, 2, 3, 34,
   0, 0,
   { DGROUP_CODE, 7 },     // GetOffsetPointFormInit
   { DGROUP_CODE, 8 },     // GetOffsetPointFormExit
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(GetOffsetPointFormFields) / sizeof(FIELD),
   GetOffsetPointFormFields, 
   KSI_SPGRAPH_POINT_FORM,
   0, DO_STRING_Registry, GENERAL_Registry, (DATA *)CODE_Registry, 0, 0
};

  /* Releases 1235 from remote control at end of program. */
void Release1235(void)
{
  if(M1235Flag)
    goto_local (M1235_ADDR);
}

/****************************************************************************/
/* GRAPHOPS will call this function when the user presses the RETURN key... */
/* Puts up a form so user can enter the wavelength of the point at the      */
/* cursor position; the Pixel/Wavelength Pair will be stored in the         */
/* OffsetPixel and OffsetWL fields for use by OnePtResetOffset.             */
/****************************************************************************/
void GetOffsetPoint(void)
{
  run_form(&GetOffsetPointForm, &default_form_attributes, FALSE);
}

/* executes when the user switches to the graph below the form */
/* activates graph mode, handles return to form						*/
int SpgraphGetNewPoints(void)
{
  erase_cursor();
  if (push_form_context())
    setCurrentFormToGraphWindow();
  GraphOps();
  erase_screen_area(1, 0, 1, screen_columns, specialGrafPrepareForExit(), FALSE);
  specialGrafReturnToForm();
  return(FIELD_VALIDATE_SUCCESS);
}

static BOOLEAN GetOffsetPointFormInit(void)
{
   OldLocus = active_locus;         

   if(! popupWindowBegin()) // allocate PopupWindow
      return TRUE;          // error

   /* Initial value for form display */
   OffsetWL = CursorStatus[ActiveWindow].X;

   // fake out the forms system so that the mouse handler
   // will require this form to be taken care of first.
   active_locus = LOCUS_POPUP;
   popupWindowSetup(GetOffsetPointForm.row, GetOffsetPointForm.column,
                    GetOffsetPointForm.size_in_rows,
                    GetOffsetPointForm.size_in_columns);
   return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static BOOLEAN GetOffsetPointFormExit(void)
{
  popupWindowEnd();
  
  if (Current.Form->exit_key_code != KEY_ESCAPE)
    OffsetPixel = CursorStatus[ActiveWindow].PointIndex;
  SetGraphCursorType(CursorType);            
  active_locus = OldLocus;

  return FALSE;
}

/****************************************************************************/
/* Either gray out or "un-gray" the form fields so the user knows whether   */
/* a spectrograph has been identified. If the spectorgraph was disabled by  */
/* selecting "???" or was not detected on bootup, most of the controls will */
/* also be disabled.                                                        */
/****************************************************************************/
static void SetShadedFields(void)
{
  int i;

  for (i = 0;i < M1235Form.number_of_fields; i++)
    {
    switch (i)                   /* list of fields to skip is here */
      {                          /* these fields are active no matter what */
      case LBL_SPGRAPH:          
      case FLD_OFFSET_BOX:       
      case FLD_GR_SEL:           /* Skip because it's the 1st field on form */
      case FLD_SPGRAPH:
      case FLD_SPGRAPH_ID:
      case FLD_DISP:
        continue;
      default:
        if (SpecTog && M1235Flag) /* turn on field if device connected */
          M1235FormFields[i].attrib &= ~(FLDATTR_SHADED | FLDATTR_DISABLED);
        else                      /* else turn field off */
          M1235FormFields[i].attrib |= (FLDATTR_SHADED | FLDATTR_DISABLED);
      }
    }
  /* handle title of OffsetBox separately */
  if(SpecTog && M1235Flag)
    OffsetBox_Fields[0].attrib &= ~FLDATTR_SHADED;
  else /* no spectrograph present or selected, disable subform */
    {
    OffsetBox_Fields[0].attrib |= FLDATTR_SHADED;
    M1235Form.field_index = FLD_SPGRAPH; /* force main form field on entry */
    }
}

/* ------------------------------------------------------------------ */
/* Form initialization, called from menu on form entry                */
/* ------------------------------------------------------------------ */
BOOLEAN M1235FormInit(void)
{
  NoAction = TRUE;                   /* will be reset if GO is pressed */
  if (ActonSpectrograph())
    SpecTog = 1;
  SetShadedFields();
  InitialMethod->Grating = Grating+1;
  if (RelPixelWidth < 0.0F)
    DispersionToggle = 1;
  else
    DispersionToggle = 0;
  AdjustFieldLabels();
  return InitSplitForm(&M1235Form, &InM1235Form, SpgraphGetNewPoints);
}

BOOLEAN M1235FormExit(void)
{
  BOOLEAN temp = ExitSplitForm(&InM1235Form);
  InitialMethod->Excitation = (FLOAT)ExcitationWaveLength;
  if (!NoAction)
    PutUpPlotBoxes();
  return temp;
}

/* --------------------------------------------------------------------
  Parameters and symbols:
    Wavelength in general is "W"
    Channel is "P" (pixel); they are related by:  W = A0 + A1*P

    PA is the center channel of the array: either 255.5 or 511.5.
    WA is the wavelength at the center PA.
    W0 is the wavelength reading of the 1235.
    P0 is the "channel" corresponding to W0.

  For situations where part of the array is viewed:
    WV is the wavelength of the center of the viewing window, and
    PV is the channel corresponding to WV.

  Finally there is a fudge factor for wavelength correction, "WC",
  because, while:
                 W  = A0 + A1*P
                 WA = A0 + A1*PA
                 WV = A0 + A1*PV,

                 W0 = A0 + A1*P0 - WC.
                    = (A0 - WC) + A1*P0.
                 A0 = W0 - A1*P0 + WC
  
  Both A1 and WC are functions of wavelength and grating g/mm, and are
  measured quantities.  It is assumed they are spectrograph independent.
  P0 is spectrograph/detector dependent, since moving the detector 0.01"
  relative to the spectrograph (in focussing the system,...) changes P0
  by 10 channels.  A1 and WC are expressed in a power series of W0:

                 A1 = a0 + a1 * W0
                 WC = wc0 + wc1 * W0.

  The coefficients a0, a1, wc0, and wc1 are stored for each grating sold
  with the 1235.  The parameter P0 is measured for each grating during
  wavelength calibration and stored in the "M1470.PRM" file.  Actually
  deltaP = (P0 - PA) is stored.  The reason is that an uncalibrated
  system will have "0" stored as the default parameter or PA = P0.
  To get P0, P0 = PA + deltaP.

  In practice, the user asks for WA and the coefficients are in terms
  of W0.  To get A0, A1 and W0 do the following:

    WA = A0 + A1*PA
    A0 = WA - A1*PA
    W0 = A0 + A1*P0 - WC
       = WA - A1*PA + A1*P0 - WC
       = WA + A1(P0 - PA) - WC;
    DP = (P0 - PA)
    W0 = WA + A1*DP - WC;
       = WA + (a0 + a1*W0)DP - (wc0 + wc1*W0);
       = WA + a0*DP - wc0 - wc1*W0  + a1*W0*DP
       = WA + a0*DP - wc0 - W0(wc1  - a1*DP)
    W0 + W0*(wc1  - a1*DP) = WA + a0*DP - wc0
    W0(1 + wc1  - a1*DP) = WA + a0*DP - wc0
    W0 = (WA + a0*DP - wc0) / (1 + wc1  - a1*DP)

  In general:
    W0 = (WV + a0(P0 - PV) - wc0) / (1 + wc1  - a1(P0 - PV))   Eq. (1)

  Then A1 and WC are calculated using W0; Finally A0 is calculated.
  
  In order to determine P0, a known spectrum must be taken.  W0 is known
  from the 1235, so A1 and WC can be calculated directly.  A known peak
  in the spectrum is entered using peak find to get PV.  The value of the
  peak, WV', is reported assuming P0 = PA in Eq. (1).  The user enters the
  correct value of the peak WV.  A0 is calculated from WV = A0 + A1*PV,
  and this used to calculate P0 from A0 = W0 - A1*P0 + WC, or
    P0 = (W0 + WC - A0)/A1.
  The parameter deltaP = P0 - PA is stored in the prm file.

/---------------------------------------------------------------------- */

struct GratingType          /* for setting calibration after moving 1235 */
  {
  float Grooves;            /* grooves per mm */
  float wc[2];              /* wc0, wc1, to generate WC = wc0 + wc1*W0 */
  float  a[2];              /* a0, a1, to generate A1 = a0 + a1*W0 */
  };

struct GratingType GratingInfoM1235 [MAX_GRATING] = {
  /*  g/mm: */
  { 147.0F, /* wc: */  { -0.29986F,   4.3355E-4F,  },
            /* a:  */  {  0.610703F, -8.3501E-6F,  }},

  {  30.0F, /* wc: */  {  0.0F,       0.0F,        },
            /* a:  */  {  2.992F,     0.0F,        }},

  { 150.0F, /* wc: */  { -0.19064F,  -1.39547E-4F, },
            /* a:  */  {  0.601754F, -7.45414E-6F, }},

  { 300.0F, /* wc: */  {  0.157464F, -6.05988E-4F, },
            /* a:  */  {  0.300498F, -8.10197E-6F, }},

  { 600.0F, /* wc: */  { -0.03314F,  -4.58987E-5F, },
            /* a:  */  {  0.152551F, -1.62572E-5F, }},

  {1200.0F, /* wc: */  { -0.18356F,   1.215013E-4F,},
            /* a:  */  {  0.078786F, -2.24731E-5F, }},

  {1800.0F, /* wc: */  {  0.0F,       0.0F, },
            /* a:  */  {  0.04F,      0.0F, }},

  {2400.0F, /* wc: */  {  0.0F,       0.0F, },
            /* a:  */  {  0.03F,      0.0F, }},};

struct GratingType GratingInfoM1236 [MAX_GRATING] = {
  /*  g/mm: */
  { 147.5F, /* wc: */  { -0.0F,       0.0F,  },
            /* a:  */  {  0.336F,     0.0F,  }},

  {  30.0F, /* wc: */  {  0.0F,       0.0F,        },
            /* a:  */  {  1.5F,       0.0F,        }},

  { 150.0F, /* wc: */  { -0.19064F,  -1.39547E-4F, },
            /* a:  */  {  0.3008F,   -7.45414E-6F, }},

  { 300.0F, /* wc: */  {  0.157464F, -6.05988E-4F, },
            /* a:  */  {  0.150F,    -8.10197E-6F, }},

  { 600.0F, /* wc: */  { -0.03314F,  -4.58987E-5F, },
            /* a:  */  {  0.076F,    -1.62572E-5F, }},

  {1200.0F, /* wc: */  { -0.18356F,   1.215013E-4F,},
            /* a:  */  {  0.039F,    -2.24731E-5F, }},

  {1800.0F, /* wc: */  {  0.0F,       0.0F, },
            /* a:  */  {  0.02F,      0.0F, }},

  {2400.0F, /* wc: */  {  0.0F,       0.0F, },
             /* a:  */ {  0.015F,     0.0F, }},};

/* MLM notes 9/17/1993:                                                 */
/* MLM notes 6/28/1995:                                                 */
/* I have only seen 1 SP-150 spectrograph; it had two gratings on one   */
/* turret (docs say possibly three turrets with 2 gratings each),       */
/* and those gratings were 1200 g/mm @500nm and 500 g/mm @500nm.        */
/* So the remaining entries are pure imagination.  All entries were     */
/* in this table were initially copied from the 1235 table, and the     */
/* a0 values multiplied by 2 as the proces vision software does. This   */
/* should yield a close approximation until the SP-150 can be charac-   */
/* terized properly in the way that the other two Actons were.          */

struct GratingType GratingInfoM1239 [MAX_GRATING] = {
  /*  g/mm: */
  {1200.0F, /* wc: */  { -0.18356F,   1.215013E-4F,},
            /* a:  */  {  0.129997F, -2.24731E-5F, }},

  { 500.0F, /* wc: */  { -0.03314F,  -4.58987E-5F, },
            /* a:  */  {  0.33308F,  -1.62572E-5F, }},

  { 147.0F, /* wc: */  { -0.29986F,   4.3355E-4F,  },
            /* a:  */  {  1.007659F, -8.3501E-6F,  }},

  {  30.0F, /* wc: */  {  0.0F,       0.0F,        },
            /* a:  */  {  4.937F,     0.0F,        }},

  { 300.0F, /* wc: */  {  0.157464F, -6.05988E-4F, },
            /* a:  */  {  0.495822F, -8.10197E-6F, }},

  { 600.0F, /* wc: */  { -0.03314F,  -4.58987E-5F, },
            /* a:  */  {  0.251709F, -1.62572E-5F, }},

  {1800.0F, /* wc: */  {  0.0F,       0.0F, },
            /* a:  */  {  0.066F,     0.0F, }},

  {2400.0F, /* wc: */  {  0.0F,       0.0F, },
             /* a:  */  { 0.495F,     0.0F, }},};

// struct GratingType GratingInfoM1239 [MAX_GRATING] = {
//   /*  g/mm: */
//   {1200.0F, /* wc: */  { -0.18356F,   1.215013E-4F,},
//             /* a:  */  {  0.039F,    -2.24731E-5F, }},
// 
//   { 500.0F, /* wc: */  { -0.03314F,  -4.58987E-5F, },
//             /* a:  */  {  0.076F,    -1.62572E-5F, }},
// 
//   { 147.0F, /* wc: */  { -0.29986F,   4.3355E-4F,  },
//             /* a:  */  {  0.305F,    -8.3501E-6F,  }},
// 
//   {  30.0F, /* wc: */  {  0.0F,       0.0F,        },
//             /* a:  */  {  2.992F,     0.0F,        }},
// 
//   { 150.0F, /* wc: */  { -0.19064F,  -1.39547E-4F, },
//             /* a:  */  {  0.3008F,   -7.45414E-6F, }},
// 
//   { 300.0F, /* wc: */  {  0.157464F, -6.05988E-4F, },
//             /* a:  */  {  0.150F,    -8.10197E-6F, }},
// 
//   {1800.0F, /* wc: */  {  0.0F,       0.0F, },
//             /* a:  */  {  0.02F,      0.0F, }},
// 
//   {2400.0F, /* wc: */  {  0.0F,       0.0F, },
//              /* a:  */ {  0.015F,     0.0F, }},};
// 

/* Initialize pointer to 1235 as default. */
struct GratingType * GratingInfo = GratingInfoM1235;

void SpFactor(FLOAT Factor)
{
  int i, j;
  float grooves;

  for (i = 0; i < MAX_GRATING; i++)
    {
    grooves = GratingInfo[i].Grooves;
    for (j = 0; j < MAX_GRATING; j++)
      {
      if (GratingInfoM1235[j].Grooves == grooves)
        {
        GratingInfo[i].a[0] = GratingInfoM1235[j].a[0] * Factor;
        break;
        }
      }
    }
}

 /*******************************************************************/
 /*                        1235 communications                      */
 /*        The 1235 must be at address 11 on the gpib bus !!        */
 /*                                                                 */
 /*******************************************************************/


BOOLEAN CheckSpgraph(void)
{
  BOOLEAN success = FALSE;
  UCHAR spoll_byte;

  if (IFC_GPIB == Interface)
    {
    /* do serial polls until CMD-Ready. The loop will */
    /* terminate if the spectrograph is shut off */
    /* note that it is important that success is set to */
    /* FALSE as last "condition" of while test! */
    do
      {
      success = serial_poll_gpib(M1235_ADDR, &spoll_byte);
      }
    while ((!(spoll_byte & 1)) && success &&
           (!kbhit() || getch() != 27 && (success = FALSE)));
    }
  else /* NOT GPIB (=serial) */
    {
    int i, j;
    for (i = 0; i < 32;i++)
      {
      j = _bios_serialcom(_COM_STATUS, SPG_PORT, 0);
      if ((char)j || (j & SERIAL_ERR)) /* look for <lf> or timeout */
        break;
      }
    success = TRUE;
    }
  return success;
}

#pragma optimize("", off)
/* return 0 if error, non-zero if success */

int Output1235CMD(char *cmd)        /* send command string to interface */
{
  USHORT i, j, old_time,
  status = FALSE; /* status != 0 is OK, == 0 is error */
  UCHAR spoll_byte;
  BOOLEAN success = FALSE;

  i = j = strlen(cmd);
  if (M1235Flag && i)                 /* if no command, don't bother*/
    {
    if (IFC_GPIB == Interface)
      {
      old_time = set_gpib_timeout(10);  /* set timeout to 300 msec. */
      success = serial_poll_gpib(M1235_ADDR, &spoll_byte);
      if (success)
        {
        status = puts_gpib(cmd, &i, M1235_ADDR);  /* send string */
        status = ((status & GPIB_ERR) == 0);      /* remember whether an error here */
        SysWait(500);
        }
      else /* not success */
        M1235Flag = 0;

      }
    else /* NOT GPIB (=serial) */
      {
      status = TRUE;
      for (j = 0;j < i;j++)
        if (_bios_serialcom(_COM_SEND, SPG_PORT, (USHORT)cmd[j]) & SERIAL_ERR)
          {
          status = FALSE;
          break;
          }
      }

    if (!M1235Flag || !status || (i != j))   /* if an error or incomplete */
      {
      if (IFC_GPIB == Interface)
        goto_local(M1235_ADDR);         /* UNT, 1235 listen, GTL, UNL */
      M1235Flag = 0;
      cmd[j-1] = '\0';                /* terminate incomplete string */
      error (ERROR_1235_FAIL, cmd);   /* "M1235 Communication Failure" */
      }
    else
      {
      success= CheckSpgraph();
      if (!success)
        {
        goto_local(M1235_ADDR);         /* UNT, 1235 listen, GTL, UNL */
        M1235Flag = 0;
        }
      }
    if (IFC_GPIB == Interface)
        set_gpib_timeout(old_time);       /* set timeout to original */
    }
  else
    M1235Flag = 0;
  return(status && success && M1235Flag);
}

#pragma optimize("", on)

 /*******************************************************************/
 /*                                                                 */
 /* Read answer from 1235. Return 1 if success                      */
 /*                                                                 */
 /*******************************************************************/
int Get1235Response(char *st, SHORT maxlen)
{
  USHORT status = TRUE;

  if (M1235Flag)                      /* don't try if not there */
    {
    if (IFC_GPIB == Interface)
      {
      BOOLEAN success = FALSE;
      UCHAR spoll_byte;

      do
        success = serial_poll_gpib(M1235_ADDR, &spoll_byte);
      while ((!(spoll_byte & GP_OK)) && success &&
             (!kbhit() || getch() != 27 && (success = FALSE)));

      if (success && (spoll_byte & GP_OUTPUT_READY))
        status = gets_gpib(st, (USHORT *)&maxlen, M1235_ADDR);
      else
        status = FALSE;

      st[maxlen] = 0;                /* end string for C */
      while (maxlen)
        if (st[--maxlen] < 26)       /* clear out control char ?? */
          st[maxlen] = 32;
      }
    else  /* NOT GPIB (=serial) */
      {
      int i, j;

      for (i = 0;i < maxlen;)
        {
        j = _bios_serialcom(_COM_RECEIVE, SPG_PORT, 0);
        st[i++] = (char)j;
        if (j & SERIAL_ERR)
          status = FALSE;
        if (j & SERIAL_ERR || (char)j == 'k')       /* Look for the "ok". */
          break;
        }

      if (status)
        {
        for (j = 0; j < i; j++)     /* Remove leading <lf> */
          st[j] = st[j+1];
        i -= 3;                     /* back up over the "ok" */
        }

      st[i] = '\0';               /* end string for C */
      while (i)                   
      if ((UCHAR)st[--i] < '\x1a')/* save the arrow "->" from the ?GRATINGS command, */
        st[i] = ' ';              /* but change control chars to spaces. */
      }
    }
  return(status);
}

int SpgraphCmdResp(char *query, char *resp, int maxlen)
{
  int status = FALSE;

  if (M1235Flag)                       /* don't try if not there */
    {
    ShowBusyMsg(BAF_NONE);
    status = Output1235CMD(query) && Get1235Response(resp, maxlen);
    EndBusyMsg();
    }
  return status;
}

int ActonSpectrograph(void)       /* make sure it's ok to reset.*/
{
  return ((InitialMethod->Spectrograph >= 1235) &&
          (InitialMethod->Spectrograph <= 1239) &&
           InitialMethod->Grating);
}

 /*******************************************************************/
 /*                                                                 */
 /* Store present setup in 1235 EEPROM.                             */
 /*                                                                 */
 /*******************************************************************/
int Store1235Settings(void)
{
  int success;

  if (ActonSpectrograph() && M1235Flag)       /* make sure it's ok to reset.*/
    {
    sprintf(GPIBBuf, "%i INIT-GRATING \r", InitialMethod->Grating);
    success = SpgraphCmdResp(GPIBBuf, GPIBBuf, 80);
    if (success)
      {
      sprintf (GPIBBuf, "%.2f INIT-WAVELENGTH \r", M1235_W0);
      success = SpgraphCmdResp(GPIBBuf, GPIBBuf, 80);
      goto_local (M1235_ADDR);                  /* tell m1235 GTL */
      }
    }
  return success;
}

 /* This ensures the coefficients are ok after grating/wavelength changes. */

void RedoCalCoeffs(void)
{
  float *aptr = InitialMethod->CalibCoeff[0];

/*  if (aptr[1] < F0) (should RelPixelWidth be changed here? */
  
  memset((void *)aptr, 0, 4 * sizeof(float));
  aptr[1] = (float)1.0;               /* assume uncalibrated */
  if (GrPtr)
    {
    /* A1 = a0 + a1 * W0 for 25 micron; so multiply A1 by RelPixelWidth. */

    aptr[1] = RelPixelWidth * (GrPtr[2] + GrPtr[3] * W0);

    /* A0 = W0 - A1*P0 + WC */                                                    

    aptr[0] = W0 - aptr[1]*P0[GrNo] + GrPtr[0] + W0*GrPtr[1];

    InitialMethod->CalibUnits[0] = ADJ_NM; /* flag that coef's are calculated. */
    }
  RegenXData(aptr); /* regenerate X data for live curves */
}

/* Update the form and the Method after the Spectrograph is selected */
/* (Or de-selected) */
int Load1235Registry (void)
{
  switch (SpecTog)
    {
    case 1:         /* shouldn't this be case 0:...case 1: ? */
      InitialMethod->Spectrograph = 1235;
    break;
    case 2:
    default:
      InitialMethod->Spectrograph = 0;
    }
  SetShadedFields();
  draw_form();
  return (FIELD_VALIDATE_SUCCESS);
}

/* Hook for macro to select spectrograph */
int MacChooseSpectrograph(int model)
{
  switch (model)
    {
    case 1235:
    case 1236:
    case 1237:
    case 1239:
      SpecTog = 1;
      InitialMethod->Spectrograph = model;
    break;
    default:
      SpecTog = 0;
      InitialMethod->Spectrograph = 0;
	if (InM1235Form)
		draw_form();
    }
  return(1);
}

/***********************************************************************/
/* search the internal table of grating info for an entry where the    */
/* resolution matches the resolution reported by the spectrograph for  */
/* a given grating.  Return the index into the internal table, or      */
/* MAX_GRATING if no grating matches the reported resolution           */
/***********************************************************************/
static int WhichGratingInTable(int this_one)
{
  int type = 0;
  
  /* do integer compare because 1236 reports 147.5 grating as 147 */

  while((type < MAX_GRATING) && 
          ((int)GroovesPmm[this_one] != (int)GratingInfo[type].Grooves))
      type++;
  return type;
}

/********************************************************************/
/* Change the label fields which report the Grooves/mm and coverage */
/* for each grating.                                                */
/********************************************************************/
void AdjustFieldLabels (void)
{
  int i;
  float LeftLambda, RightLambda, temp;
  Grating = InitialMethod->Grating - 1;
  for (i = 0; i < 3; i++)
    {
    int type = WhichGratingInTable(i);
    float Cover = F0;
    if (type < MAX_GRATING)           /* found a known grating.*/
      {
      GetParam(DC_ACTIVEX, &temp);
      Cover = temp * RelPixelWidth * GratingInfo[type].a[0];
      }
    if (Cover != 0.0F)
      sprintf(GratingLabels[i]+9, "%4.4g        %7.2f", GroovesPmm[i], Cover);
    else
      sprintf(GratingLabels[i]+9, "%4.4g        (Unknown)", GroovesPmm[i]);

    GratingLabels[i][0] = (char)((GrNo == i) ? '>' : ' ');
    }
  LeftLambda = DataPtToLambda(0);
  GetParam(DC_POINTS, &temp);

  RightLambda = DataPtToLambda((int)temp - 1);

  for (i = 0;i <= ADJ_NM;i++)
    {
    if (WaveLengthUnitTable[i] == InitialMethod->CalibUnits[0])
      {
      GetOffWLToggle = i;
      break;
      }
    }

  sprintf(GratingCoverage + 10, "%.2f - %.2f %s",
          LeftLambda, RightLambda,WaveLengthOptions[i]);
  WV = 0.5F * (LeftLambda + RightLambda);
}


  /*********************************************************/
  /*                                                       */
  /* Reset the P0, given one wavelength and one channel.   */
  /* P0 is the detector channel corresponding to the center*/
  /* wavelength reported by the spectrograph.              */
  /* This also forces the use of calculated coefficients.  */
  /* This routine is called by the macro language.         */
  /*                                                       */
  /*********************************************************/
int MacOnePtResetOffset (float Chnl, float Wvlngth)
{
  if (GrPtr)
    {                          
    /* A1 = a0 + a1 * W0 for 25 micron so multiply A1 by RelPxlWidth.*/
    float A1 = RelPixelWidth * (GrPtr[2] + GrPtr[3] * W0);
    P0[GrNo] = (W0 + GrPtr[0] + W0 * GrPtr[1] - Wvlngth + A1 * Chnl) / A1;
    }
  RedoCalCoeffs();                  /* reset live coeff's */
  InitialMethod->GratingCenterChnl[GrNo] = P0[GrNo];
  AdjustFieldLabels();
  return (M1235Flag ? 1 : 0);
}

/************************************************************/
/* Use the Offset Pixel and Offset wavelength from the form */
/* to reset the detector offset                             */
/************************************************************/
int OnePtResetOffset (void)
{
  int success;

  if (OffsetPixel == 0.0F || OffsetWL == 0.0F)
    success = 0;
  else
    {
    success = MacOnePtResetOffset(OffsetPixel, OffsetWL);
    draw_form();
    }
  return (success ? FIELD_VALIDATE_SUCCESS : FIELD_VALIDATE_WARNING);
}

 /* This resets "P0" and "RelPixelWidth" after a wavelength calibration,
  * and should not overwrite a[0] and a[1].  It gets RelPixelWidth by
  * calculating real coverage and dividing that by the calculated coverage.
  * P0 is calculated using W0, WV, and RelPixelWidth from equation 1.
  */
int ResetOffset(void)
{
  if (GrPtr)
    {
    float Points,  WlMin, WlMax, temp;
    GetParam(DC_POINTS, &Points);
    Points--;                         /* Number of data points - 1. */

    WlMin = DataPtToLambda(0);
    WlMax = DataPtToLambda((SHORT)Points);
    /***********************************************************************************/
    /* New center wavelength, WV */  
    WV = 0.5F * (WlMin + WlMax);
    temp = (W0 + GrPtr[0] + W0 * GrPtr[1] - WV) / (GrPtr[2] + W0 * GrPtr[3]);
     /* RelPxlWdth = real_nm / calc_nm.*/
    InitialMethod->RelPixelWidth = RelPixelWidth =
      (WlMax - WlMin) / ((DataPtToChnl((SHORT)Points) - DataPtToChnl(0)) *
        (GrPtr[2] + GrPtr[3] * W0));
    temp /= RelPixelWidth;            /* Account for less than 25 micorn*/

    InitialMethod->GratingCenterChnl[GrNo] = P0[GrNo] =
      0.5F * (DataPtToChnl(0) + DataPtToChnl((int)Points)) + temp;
    /***********************************************************************************/
    }
  return (M1235Flag ? FIELD_VALIDATE_SUCCESS : FIELD_VALIDATE_WARNING);
}

  /***********************************************************************/
  /* If "grating" is 1, 2, or 3, searches to see if data held about      */
  /* the grating.  If grating not same as GrNo (current grating) will    */
  /* change to new grating and recalibrate system - if known grating;    */
  /* if unknown grating, will uncalibrate system.                        */
  /***********************************************************************/
int ChangeGrating(int grating)
{
  GrPtr = NULL;                       /* Flag unknown grating for safety */
  if ((grating > 0) && (grating < 4))
    {
    int type = 0;           /* search for grooves per mm to do A0 and A1.*/
    grating--;              /* put back into units the computer likes */
    Grating = grating;

    type = WhichGratingInTable(grating);

    /* found a known grating; un-NULL GrPtr. */
    if (type < MAX_GRATING)
      GrPtr = &GratingInfo[type].wc[0];
    /* if not current grating, move 1235.*/
    if (grating != GrNo)               
      {
      GratingLabels[GrNo][0] = ' ';     /* erase arrow pointing at current. */
      GrNo = grating++;                 /* reset current. */
      sprintf(GPIBBuf, "%i GRATING \r", grating);
      SpgraphCmdResp(GPIBBuf, GPIBBuf, 80);
      InitialMethod->Grating = grating; /* reset system parameter */
      RedoCalCoeffs();                  /* reset live coeff's */
      }
    OffsetPixel = P0[grating];
    OffsetWL = EvalPoly(OffsetPixel); /* Pixel is already a "channel" */
    }
  return (M1235Flag ? FIELD_VALIDATE_SUCCESS : FIELD_VALIDATE_WARNING);
}

/*************************************************************************/
/* Helper function for Reset1235 and SetCenterLambda                     */
/*************************************************************************/
float CalcNewW0(void)
{
  float DeltaP, ftemp;   /* DeltaP = P0 - 1/2(LeftPixel + RightPixel) */
  int Points;            /* Number of highest data point */

  GetParam(DC_POINTS, &ftemp);  /* get count of points */
  Points = (int)ftemp - 1;      /* change to ordinal value */

  /* If method had Relative Pixel Width, use it. */
  if (InitialMethod->RelPixelWidth)
    RelPixelWidth = InitialMethod->RelPixelWidth;

  DeltaP = P0[GrNo] - (0.5F * (DataPtToChnl(0) + DataPtToChnl(Points)));
  DeltaP *= RelPixelWidth;
      
  /* solve eqn 1 for W0. */
  return(WV + GrPtr[2] * DeltaP - GrPtr[0]) /
        (1.0F + GrPtr[1] - GrPtr[3] * DeltaP);
}

char *TempSt[] = { "Adjusting a0 values", "Factor is:", NULL, NULL };

  /*********************************************************************/
  /* Routine done on powerup, or if powerup is complete, to initiate   */
  /* conversation with the 1235.  The 1235 only reports it's setting	 */
  /* to 0.1 nm, and this is ignored unless system can't autocalibrate. */
  /*********************************************************************/
#pragma optimize("",off)   /* to help with a National timing problem ??? */
void DoReset1235 (void)
{
  char *cptr;
  SHORT success = FALSE, Points, i;
  float ftemp;
  unsigned char spoll_byte;
  BOOLEAN Calibrated;
  
  {
  FILE * fin;
  float factor;
  char st[20];

  fin = fopen("SPFACTOR.TXT", "r");
  if (fin)
    {
    fscanf(fin, "%f", &factor);
    fclose(fin);
    sprintf(st, "%f", factor);
    TempSt[1] = st;
    message_window(TempSt, COLORS_MESSAGE);
    SpFactor(factor);
    }
  }

  Interface = IFC_GPIB;

  if (WasGPIBInit())
    {
    clear_gpib();
    success = serial_poll_gpib(M1235_ADDR, &spoll_byte);
    }

  /* if no response from unit on GPIB, try serial port 1 (COM2) */
  if (!success)
    {
    int i, j;
    _bios_serialcom (_COM_INIT, SPG_PORT, _COM_CHR8 | _COM_STOP1 | _COM_NOPARITY | _COM_9600);
    _bios_serialcom (_COM_SEND, SPG_PORT, 0x0d); /* Try sending carriage return. */
    for (i = 0; i < 32 ;i++ )             /* Try for ok<crlf>, or ?<crlf>.. */
      {                               
      j = _bios_serialcom (_COM_RECEIVE, SPG_PORT, 0);
      if (((char)j == 0x0a ) || (j & SERIAL_ERR)) /* look for <lf> or timeout */
        break;
      }
    M1235Flag = (char)(!(j & SERIAL_ERR)); /* No 1235 communications if timeout. */
    success = M1235Flag;
    Interface = IFC_SERIAL1;
    SpgraphCmdResp("NO-ECHO\r", GPIBBuf, 80); /* If ok, suppress command echo. */
    }

  if (ActonSpectrograph() && success)  /* make sure it's ok to reset.*/
    {
    GetParam(DC_DMODEL, &ftemp);
    switch ((SHORT)ftemp)
      {
      case TSC_CCD:
      case TSM_CCD:
      case TSP_CCD:
        RelPixelWidth = (float)(19.0/25.0); /* pixel width in 25 um units */
      break;
      default:
        RelPixelWidth = 1.0F;               /* pixel width in 25 um units */
      break;
      }
    SpecTog = 1;
    Grating = InitialMethod->Grating - 1;
    M1235Flag = 1;
 
    SpgraphCmdResp("?NM \r", GPIBBuf, 80);   /* read response to ?NM */
    if (!M1235Flag)
      return;
    W0 = (float) (atof (GPIBBuf));   /* convert to wavelength */
    WV = W0;                        /* Find Acton/EG&G model #... */

    if (!SpgraphCmdResp("WHO \r", GPIBBuf, 80))
      return;
    
    if (strstr(GPIBBuf,"SP-150"))    /* SP-150 translates to 1239 in EG&G */
      M1235Flag = (1239 - 1234);    /* So you can see the logic */
    else
      {
      /* read serial number */
      if (!SpgraphCmdResp("EESERIAL EE@ U.\r", GPIBBuf, 80))
        return;                     /* if error */

      if (atof (GPIBBuf) > 50000.0)  /* 1236 has serial number > 50,000 */
        {
        M1235Flag++;
        Output1235CMD ("MFRONT\r"); /* put mirror on front pos.*/

        if (IFC_GPIB == Interface)
          {
          serial_poll_gpib(M1235_ADDR, &spoll_byte);
          if (spoll_byte & 0X82)
            Get1235Response (GPIBBuf, 80); /* read possible error message. */
          else
            M1235Flag++;              /* If no error, it's a 1237. */
          }
        else
          {
          GPIBBuf[0] = '\0';
          Get1235Response(GPIBBuf,80); /* Get back possible '?' */
          if (GPIBBuf[0] != '?')       /* No error? */
            M1235Flag++;              /* flag a 1237 */
          }
        }
      }
    
    /* patch label in form field - saved a few bytes over using a normal */
    /* string array for the field options, I guess,  plus prevents user  */
    /* from guessing (wrongly) at spectrograph model number              */

    SpectrographOptions[1][4] = (char)'4' + (char)M1235Flag;

    if (M1235Flag == (1239 - 1234))
      GratingInfo = GratingInfoM1239;
    else if (M1235Flag == (1235 - 1234))
      GratingInfo = GratingInfoM1235;
    else
      GratingInfo = GratingInfoM1236;

    /* ask about the gratings for today */
    SysWait(1500);
    SpgraphCmdResp("?GRATINGS \r", GPIBBuf, 100);

    /* reply starts off with 2 or 3 spaces. */
    for (cptr = GPIBBuf,i = 0;i < 10;i++, cptr++)
      if (cptr[0] != ' ') break;

    for (i = 0; i < 3; i++)         /* stuff the array for labels.*/
      {
      GroovesPmm[i] = (float)atof(cptr + 2);
      /* find the active grating */
      /* Acton response puts a right arrow next to it */
      /* Right arrow is ASCII 26 (CTRL-Z) which stops compiler */
      /* if entered in text as character literal */
      if (cptr[0] == 26)            
        GrNo = i;
      cptr += 22; /* Assumes constant line length for reply */
      if (cptr[0] == ' ')
        cptr++;

      P0[i] = InitialMethod->GratingCenterChnl[i];
      if (!P0[i])                      /* set up P0 to default if zero */
        {
        GetParam(DC_ACTIVEX, &ftemp);
        P0[i] = 0.5F * (ftemp - 1.0F);
        }
      }
    ChangeGrating(InitialMethod->Grating); /* search for grating */

    /* ChangeGrating can change CalibUnits!!! */

    Calibrated = (InitialMethod->CalibUnits[0] == ADJ_NM ||
                  InitialMethod->CalibUnits[0] == NM);

    if (Calibrated) /* calibrated? if so, set WV */
      {
      GetParam(DC_POINTS, &ftemp);
      Points = (int)ftemp - 1;
      WV = 0.5F * (DataPtToLambda(0) + DataPtToLambda(Points));
      }
    /* See if calibrated backwards. */
    if (InitialMethod->CalibCoeff[0][1] <
       (InitialMethod->CalibCoeff[0][2] + InitialMethod->CalibCoeff[0][3]))
      {
      if (RelPixelWidth < 0.0F)         /* Make Width Positive it it's not */
        RelPixelWidth = -RelPixelWidth;
      }
    /* if units = nm, and grating */
    if (InitialMethod->CalibUnits[0] == NM && GrPtr)
      {
      M1235_W0 = W0;      /* save old value just read. */
      W0 = CalcNewW0();
      if (M1235_W0 != W0) /* Move 1235 if its W0 differs from calc'd W0 */
        {
        sprintf (GPIBBuf, "%.2f <GOTO> \r", W0);
        SpgraphCmdResp(GPIBBuf, GPIBBuf, 80);
        }
      }
    else
      SetCenterLambda();
    M1235_W0 = W0;
    }
}
#pragma optimize("",on)

/* General entry point for spectrograph/module initialization */
/* called from main()                                         */
int Reset1235 (void)
{
  DoReset1235();
  return (M1235Flag ? FIELD_VALIDATE_SUCCESS : FIELD_VALIDATE_WARNING);
}

/* Macro hook for Reset1235 */
int MacRedo1235(void)
{
  Reset1235();
  AdjustFieldLabels();
  return (M1235Flag != 0);
}

/* Re-initialize spectrograph and module from form */
int Redo1235(void)
{
  int success = MacRedo1235();
  SetShadedFields();
  draw_form();
  return (success ? FIELD_VALIDATE_SUCCESS : FIELD_VALIDATE_WARNING);
}

  /* The global parameter "WV" contains the wavelength WV at the center
   * of the scanned window - in nm.  If a known grating, calculates
   * the 1235 wavelength "W0" to put "WV" in the middle of the scanned
   * window.  If unknown grating, just sets W0 = WV.
   * It sends the 1235 to W0.
   */

int SetCenterLambda(void)  /* WV is lambda at center of window - in nm. */
{
  W0 = WV;                    /* assume one to one correspondence */
  if (GrPtr)                  /* found a known grating */
    {                         /* calculate (P0 - PV) */
    W0 = CalcNewW0();
    }
  RedoCalCoeffs();            /* reset live coeff's */
  sprintf(GPIBBuf, "%.2f <GOTO> \r", W0);
  SpgraphCmdResp(GPIBBuf, GPIBBuf, 32);
  M1235_W0 = W0;
  return (M1235Flag ? FIELD_VALIDATE_SUCCESS : FIELD_VALIDATE_WARNING);
}

  /* Hook for macro to set wavelength */
int MacSetWV(float Wavelen)
{
  WV = Wavelen;
  SetCenterLambda();
  return(1);
}

  /* Macro hook for the 'GO' field function */
int MacSet1235(void)
{
  ChangeGrating(Grating + 1);
  SetCenterLambda();         /* WV is lambda at center of window - in nm. */
  AdjustFieldLabels();
  return (M1235Flag != 0);
}

  /* Form routine entry point for the 'GO' field function */
int Set1235(void)
{
  int success = MacSet1235();
  draw_form();
  return (success ? FIELD_VALIDATE_SUCCESS : FIELD_VALIDATE_WARNING);
}

  /*********************************************************************/
  /* The calibration determined at bootup assumes that longer wave-    */
  /* lengths are at lower pixel numbers. Although this is not always	  */
  /* true, there is no easy way to determine otherwise without some    */
  /* user input. The dispersion field indicates whether the current    */
  /* calibration is Red-to-Blue or Blue-to-Red. If the spectrum is     */
  /* actually Blue-to-Red, the user must switch the field to the       */
  /* position, but only the first time the spectrograph is initialized */
  /* On subsequent resets (even after power off) the system remembers  */
  /* the "backwards" calibration                                       */
  /*********************************************************************/
int ChangeDispersion(void)
{
 /* the sign of RelPixelWidth determines the calibration direction */

  RelPixelWidth = (float)fabs(RelPixelWidth);
  if (DispersionToggle)
    RelPixelWidth = -(RelPixelWidth);
  InitialMethod->RelPixelWidth = RelPixelWidth;
  return Redo1235();
}

SHORT MacChangeDispersion(BOOLEAN IsBlueToRed)
{
  DispersionToggle = (IsBlueToRed != 0);
  return ChangeDispersion();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void registerSPGraphForm(void)
{
   FormTable[KSI_SPGRAPH_FORM] = &M1235Form;
}
