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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sys\types.h>
#include <sys\timeb.h>

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

/* set up to use the InitialMethod pointer */

#define F0 0.0F
#define MAX_GRATING 8

#define M1235_ADDR  11
#define ERR  0x8000

int rummy_select = FALSE;

static float OffsetPixel;
static float OffsetWL;
static char M1235Flag = 0;       /* 1 if 1235, 2 if 1236; 0 if not */
static int GrNo = 0;             /* currently selected grating number*/
static float RelPixelWidth;      /* width of detector pixel in 25 um units.*/
static float WV;                 /* center of current window */
static float LeftLambda;         /* left edge of current window */
static float RightLambda;        /* right edge of current window */
static float W0;                 /* current 1235 wavelength reading */
float M1235_W0;                  /* current 1235 wavelength reading */
float P0 [3];                    /* Channel number corresponding to
                                  * above wavelength. In perfect world,
                                  * would be center of diode array. */
                                 /* Grating numbers for the 1235. */
static float GroovesPmm [3] = { 0, 0, 0, };
static float *GrPtr;              /* if nonNULL, points to grating info */
static int Grating = 0;
static int Spectrograph = 0;
static int SpecTog = 0;           /* 0=none, 1=1235, 2=1236 .. */
static float Wavelength = F0;

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

float DataPtToChnl (int DataPt)
{
  float X_0, DeltaX, Mode, detector;
  GetParam(DC_DMODEL, &detector);
  if (detector == RAPDA)
    return (RapdaDataPtToPixel (DataPt));
  else
    {
    GetParam(DC_PNTMODE, &Mode);
    X_0  = (float)(Mode ? (DataPt + 1) : 0); /* Data points 1 based in drivers.
                                               * in contiguous, use slot 0. */ 
    if (Mode)
      SetParam(DC_POINT, X_0);            /* Find the correct slot */
                                      /* Find the groupsize. */
    GetParam(DC_DELTAX, &DeltaX);
    if (!DeltaX)                        /* DeltaX must be 1 or greater */
      DeltaX++;
    GetParam(DC_X0, &X_0);               /* Now get X0 */
    if (Mode)                          /* If random, slot has x0 */
      DataPt = 0;
                                        /* return DataPt's "chnl". */
  return (X_0 - 1.5F + DeltaX * ((float)DataPt + 0.5F));
  }
}

  /* given x in data pt#, returns wavelength.*/
  /* Converts a floating point x value into the associated wavelength
   * by evaluating the calibration polynomial.  The x value can be
   * fractional as for grouping by even numbers, or from finding the
   * center of the spectrograph's window: P0 [#].
   */
float EvalPoly (float x) 
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
float DataPtToLambda (int DataPt)
{
  return (EvalPoly (DataPtToChnl (DataPt)));
}


    /* Finds pixel with wavelength value closest to Wavelength by bracket
     * and halve.*/
int _pascal SolvePoly (float Wavelength)
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



/* used for marking as a special case in macro playback*/
BOOLEAN InM1235Form = FALSE;     

static SHORT OldActiveWindow;    
static CRECT OldGraphArea;       
static SHORT OldLocus;
static struct save_area_info * SavedArea;           
static BOOLEAN NoAction;

/* ------------------------------------------------------------------ */

enum { DGROUP_DO_STRINGS  = 1, DGROUP_GENERAL, DGROUP_CODE };

/*
    Grating   Grooves per   Coverage
      No.      millimeter    (nm)
      1        147.5        444.5
*/
static char GratingCoverage [] =
              /*   0123456789012345678901234567890    */
                { "Coverage:                                   " };

static char GratingLabels[3][32] = {
                 /*   0123456789012345678901234567890    */
                     " 1                            ",
                     " 2                            ",
                     " 3                            ", };

DATA M1235_Registry [] = {
 /* 0 */ { "Spectrograph ",     0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 1 */ { " Reset Spectrograph",   0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 2 */ { "Pixel   Xp Unit Value", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 3 */ { "Grating   Grooves per   Coverage",  0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 4 */ {   "No.      millimeter    (nm)",    0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 5 */ { GratingLabels[0],    0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 6 */ { GratingLabels[1],    0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 7 */ { GratingLabels[2],    0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 8 */ { "Grating Selection", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 9 */ { "Center Wavelength", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 10*/ { GratingCoverage,     0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 11*/ { " Go ",              0, DATATYP_STRING, DATAATTR_PTR, 0 },
 /* 12*/ { " Reset Offset ",    0, DATATYP_STRING, DATAATTR_PTR, 0 },
};

static char * SpectrographOptions[] =
              {"  ??? ", " M1235 ", " M1236 ", " M1237 ", " M1239 " };

static char * GratingOptions[] = { " 1 ",  " 2 ",  " 3 ",};

static DATA M1235_GeneralRegistry[] = {
 /* 0 */ { GratingOptions,      0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 1 */ { &Grating,            0, DATATYP_INT,          DATAATTR_PTR, 0 },
 /* 2 */ { &WV,                 0, DATATYP_FLOAT,        DATAATTR_PTR, 0 },
 /* 3 */ { SpectrographOptions, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
 /* 4 */ { &SpecTog,            0, DATATYP_INT,          DATAATTR_PTR, 0 },
 /* 5 */ { &rummy_select,       0, DATATYP_INT,          DATAATTR_PTR, 0 }, 
 /* 6 */ { &FormTable[KSI_XCAL_SCROLL_FORM],
                                0, DATATYP_VOID,     DATAATTR_PTR_PTR, 0 },
};

static EXEC_DATA M1235_CodeRegistry[] = {
   /* 0  */  { Load1235Registry, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 1  */  { Redo1235, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 2  */  { Set1235, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 3  */  { M1235FormInit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 4  */  { M1235FormExit, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
   /* 5  */  { OnePtResetOffset, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
};

static enum {SPEC_LABEL, SP_SEL, COV_LABEL, PIX_LABEL, SCRL_FRM,
             WVL_LABEL,  GSEL_LABEL, DISP4_LABEL, DISP3_LABEL,
             DISP2_LABEL, DISP1_LABEL, GRATE_LABEL,
             SP_RESET, GR_SEL, GET_LAM, SET_OFFSET, DO_GO,};
 
static FIELD M1235FormFields[] = {
    /* "Spectrograph "*/
d_field_(SPEC_LABEL,                  /* name       */
         FLDTYP_STRING,               /* type       */
         FLDATTR_DISPLAY_ONLY,        /* attr       */
         0,                           /* help       */
         DGROUP_DO_STRINGS, 0,        /* prim data  */
         1, 4, 13),                  /* position   */

   /* Spectrograph selection */
field_set(SP_SEL,
          FLDTYP_TOGGLE, FLDATTR_REV_VID,
          KSI_SPEC_SELECT,
          X_CALSCROLLFIELD_HBASE + 0,
          DGROUP_GENERAL, 4,    /* Spectrograph  */        /* (0:1) */
          DGROUP_GENERAL, 3,    /* Spec options. */
          DGROUP_CODE, 0,       /* reset fields  */
          0, 2,
          1, 17, 7,
          EXIT,  SP_SEL,  SP_RESET,  GR_SEL,
          0,     0,       GR_SEL,   SP_RESET),

    /*  "Coverage  435.8nm - 546.1nm "*/
label_field(COV_LABEL,
            DGROUP_DO_STRINGS, 10 ,                          /* (0:2) */
            3, 2, 42),

 /* "Pixel     Xp Unit Value" */
label_field(PIX_LABEL,
         DGROUP_DO_STRINGS, 2, 
         1, 56, 22),

   /* Point scroll form */
field_set(SCRL_FRM,
          FLDTYP_FORM, FLDATTR_NONE,
          KSI_SPEC_SCRLFRM,
          X_CALSCROLLFIELD_HBASE + 6,
          DGROUP_GENERAL, 6,        /* Scroll form  */        /* (0:4) */
          0, 0,                     /* Scroll options.*/
          0, 0,                     /* reset fields*/
          0, 0,
          0, 57, 1,
          EXIT,  SET_OFFSET,  0,  0,
          SP_RESET, SET_OFFSET, SET_OFFSET, SET_OFFSET),

   /* "Grating   Dispersion   Coverage"*/
label_field(GRATE_LABEL,
         DGROUP_DO_STRINGS, 3, 
         4, 36, 32),

   /* "No.     (nm/pixel)     (nm)"*/
label_field(DISP1_LABEL,
         DGROUP_DO_STRINGS, 4, 
         5, 38, 32),

   /*  "1        0.01512       444.5 "*/
label_field(DISP2_LABEL,
         DGROUP_DO_STRINGS, 5, 
         7, 38, 31),

   /*  "2        0.231522      444.5 ",*/
label_field(DISP3_LABEL,
         DGROUP_DO_STRINGS, 6, 
         8, 38, 31),

   /*  "3        0.341655      444.5 "*/
label_field(DISP4_LABEL,
         DGROUP_DO_STRINGS, 7, 
         9, 38, 31),

   /*  "Grating Selection"*/
label_field(GSEL_LABEL,
         DGROUP_DO_STRINGS, 8, 
         4, 2, 18),

   /*  "Center Wavelength"*/
label_field(WVL_LABEL,
         DGROUP_DO_STRINGS, 9, 
         5, 2, 18),
      
    /* "Reset Spectrograph"*/
field_set(SP_RESET,
          FLDTYP_SELECT,
          FLDATTR_REV_VID,
          KSI_SPEC_RESET,
          X_CALSCROLLFIELD_HBASE + 5,
          0, 0,
          DGROUP_DO_STRINGS, 1, 
          DGROUP_CODE, 1,                     /* reset fields*/
          0, 0,                               /* (0:11) */
          9, 2, 20,
          EXIT,  SP_RESET,   SET_OFFSET,  SP_SEL,
          0,     0,         SET_OFFSET,   SP_SEL),

   /* grating selection */
field_set(GR_SEL,
          FLDTYP_TOGGLE,
          FLDATTR_REV_VID,
          KSI_SPEC_GRATING,
          X_CALSCROLLFIELD_HBASE + 1,
          DGROUP_GENERAL, 1,    /* Grating.*/               /* (0:12) */
          DGROUP_GENERAL, 0,    /*  Grating labels.*/
          0, 0,
          0, 3,
          4, 22, 3,
          EXIT,  DO_GO,  SP_SEL,   GET_LAM, 
          0,         0,  SP_SEL,   DO_GO),

   /* center wavelength */
field_set(GET_LAM,
          FLDTYP_STD_FLOAT,
          FLDATTR_REV_VID,
          KSI_SPEC_GETLAM,
          X_CALSCROLLFIELD_HBASE + 2,
          DGROUP_GENERAL, 2,    /* CenterWaveLength */    /* (0:13) */
          0, 0,
          0, 0,
          2, 0,
          5, 22, 6,
          EXIT,  DO_GO,  GR_SEL,  DO_GO,
          0,         0,  GR_SEL,  DO_GO),

    /*  " Reset Offset "*/
field_set(SET_OFFSET,
          FLDTYP_SELECT,
          FLDATTR_REV_VID,
          KSI_SPEC_SETOFFSET,
          X_CALSCROLLFIELD_HBASE + 4,
          0, 0,
          DGROUP_DO_STRINGS, 12,                          /* (0:14) */
          DGROUP_CODE, 5,       /* reset offset*/
          0, 0,
          7, 2, 14,
          EXIT,  SET_OFFSET,  DO_GO,   SP_RESET,
          0,     SCRL_FRM,    SP_RESET, SCRL_FRM),

    /*  " Go "*/
field_set(DO_GO,
          FLDTYP_SELECT,
          FLDATTR_REV_VID,
          KSI_SPEC_GO,
          X_CALSCROLLFIELD_HBASE + 3,
          DGROUP_GENERAL, 5,    /* dummy */
          DGROUP_DO_STRINGS, 11,                           /* (0:15) */
          DGROUP_CODE, 2,       /* change grating, wavelength*/
          0, 0,
          6, 22, 4,
          EXIT,  DO_GO,  GET_LAM, SET_OFFSET,
          0,         0,  GET_LAM, GR_SEL),
};

FORM  M1235Form = {
   0, 0, FORMATTR_BORDER | FORMATTR_FIRST_CHAR_ERASE | FORMATTR_VISIBLE |
         FORMATTR_FULLWIDTH,
   0, 0, 0,
   2, 0, 11, 80,
   0, 0,
   { DGROUP_CODE, 3 },  /* M1235FormInit*/
   { DGROUP_CODE, 4 },  /* M1235FormExit*/
   COLORS_DEFAULT,
   0, 0, 2, 0,
   sizeof(M1235FormFields) / sizeof (M1235FormFields [0]),
   M1235FormFields,
   KSI_SPGRAPH_FORM,
   0, M1235_Registry, M1235_GeneralRegistry, (DATA *)M1235_CodeRegistry, 0, 0
};

  /* Releases 1235 from remote control at end of program. */
void Release1235(void)
{
  if(M1235Flag)
    goto_local (M1235_ADDR);
}

int SpgraphGetNewPoints(void)
{
  int temp = get_new_points();
  ((char *)M1235_Registry [2].pointer)[0] =
     (char)((index_of_scroll_form(FormTable[KSI_XCAL_SCROLL_FORM]) == -1) ? 0: 'P');
  return temp;
}

/* ------------------------------------------------------------------  */
/* Main module, called from menu. */

BOOLEAN M1235FormInit(void)
{
  NoAction = TRUE;                   /* will be reset if GO is pressed */
  if (ActonSpectrograph())
    SpecTog = 1;
  InitialMethod->Grating = Grating+1;
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
  { 147.5F, /* wc: */  { -0.29986F,   4.3355E-4F,  },
            /* a:  */  {  0.305F,    -8.3501E-6F,  }},

  {  30.0F, /* wc: */  {  0.0F,       0.0F,        },
            /* a:  */  {  2.992F,     0.0F,        }},

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
             /* a:  */  {  0.015F,     0.0F, }},};

/* MLM notes 9/17/1993:                                                 */
/* I have only seen 1 SP-150 spectrograph; it had two gratings on one   */
/* turret (docs say possibly three turrets with 2 gratings each,        */
/* and those gratings were 1200 g/mm @500nm and 500 g/mm @500nm.        */
/* So the remaining entries are pure imagination.  All entries were     */
/* in this table were initially copied from the 1236 table, and so      */
/* must be incorrect until the necessary research is done.  Since we    */
/* do not have a tech to do this, and since the wide variation in focal */
/* focal plane distances of our CCD detectors may invalidate the        */
/* "1-pixel" calibration anyway, I will let it stand this way for now.  */

struct GratingType GratingInfoM1239 [MAX_GRATING] = {
  /*  g/mm: */
  {1200.0F, /* wc: */  { -0.18356F,   1.215013E-4F,},
            /* a:  */  {  0.039F,    -2.24731E-5F, }},

  { 500.0F, /* wc: */  { -0.03314F,  -4.58987E-5F, },
            /* a:  */  {  0.076F,    -1.62572E-5F, }},

  { 147.0F, /* wc: */  { -0.29986F,   4.3355E-4F,  },
            /* a:  */  {  0.305F,    -8.3501E-6F,  }},

  {  30.0F, /* wc: */  {  0.0F,       0.0F,        },
            /* a:  */  {  2.992F,     0.0F,        }},

  { 150.0F, /* wc: */  { -0.19064F,  -1.39547E-4F, },
            /* a:  */  {  0.3008F,   -7.45414E-6F, }},

  { 300.0F, /* wc: */  {  0.157464F, -6.05988E-4F, },
            /* a:  */  {  0.150F,    -8.10197E-6F, }},

  {1800.0F, /* wc: */  {  0.0F,       0.0F, },
            /* a:  */  {  0.02F,      0.0F, }},

  {2400.0F, /* wc: */  {  0.0F,       0.0F, },
             /* a:  */  {  0.015F,     0.0F, }},};


/* Initialize pointer to 1235 as default. */
struct GratingType * GratingInfo = GratingInfoM1235;

/* ---------------------  1235 communications  -------------------------
/  The 1235 must be at address 11 on the gpib bus !!
/ -------------------------------------------------------------------- */
#pragma optimize("", off)

int Output1235CMD (char *cmd)        /* send command string to interface */
{
  WINDOW * MessageWindow;                
  int  i, j, old_time, status;
  unsigned char spoll_byte;
  BOOLEAN success;
  unsigned long sttime;
  struct timeb tstruct;

  i = j = strlen(cmd);
  if (M1235Flag && i)                 /* if no command, don't bother*/
    {
    old_time = set_gpib_timeout(10);  /* set timeout to 300 msec. */
    success = serial_poll_gpib(M1235_ADDR, &spoll_byte);
    if (success)
      {
      status = puts_gpib(cmd, &i, M1235_ADDR);   /* send string */
      ftime(&tstruct);
      sttime = tstruct.time + (tstruct.millitm * 1000L);
      do
        {
        ftime(&tstruct);
        }
      while (((tstruct.time + (tstruct.millitm * 1000L)) - sttime) < 2000);

      if ((status & ERR) || (i != j))
        {
        goto_local(M1235_ADDR);         /* UNT, 1235 listen, GTL, UNL */
        M1235Flag = 0;
        cmd[j-1] = '\0';
        error (ERROR_1235_FAIL, cmd); /* "M1235 Communication Failure" */
        }
      else
        {
        put_up_message_window(BusyWorking, COLORS_MESSAGE, &MessageWindow);
                                        /* do serial polls until CMD-Ready */

        do
          {
          success = serial_poll_gpib(M1235_ADDR, &spoll_byte);
          }
        while (((spoll_byte & 1) == 0) && success);

        if (!success)
          {
          goto_local(M1235_ADDR);         /* UNT, 1235 listen, GTL, UNL */
          M1235Flag = 0;
          }

        if (MessageWindow != NULL)                
          release_message_window(MessageWindow);
        }
      set_gpib_timeout(old_time);       /* set timeout to original */
      }
    else M1235Flag = 0;
    }
  return(!(status & ERR) && success && M1235Flag);
}

#pragma optimize("", on)

/* Read answer from 1235. */
/* return 1 if success */

int Get1235Response (char *st, int maxlen)
{
  unsigned status;
  if (M1235Flag)                      /* don't try if not there */
    {
    BOOLEAN success = FALSE;
    UCHAR spoll_byte;

    do
      success = serial_poll_gpib(M1235_ADDR, &spoll_byte);
    while (((spoll_byte & 0x80) == 0) && success);

    if (success)
      status = gets_gpib(st, &maxlen, M1235_ADDR);
    st[maxlen] = 0;                /* end string for C */
    while (maxlen)
      if (st[--maxlen] < 26)       /* clear out control char ?? */
        st[maxlen] = 32;
    }
  return(status);
}

int ActonSpectrograph (void)       /* make sure it's ok to reset.*/
{
  return ((InitialMethod->Spectrograph >= 1235) &&
          (InitialMethod->Spectrograph <= 1239) &&
           InitialMethod->Grating);
}

int Store1235Settings(void)      /* Store present setup in 1235 EEPROM.*/
{
  char TmpBuf[80];
  int success;

  if (ActonSpectrograph()  && M1235Flag)       /* make sure it's ok to reset.*/
    {
    sprintf(TmpBuf, "%i INIT-GRATING \r", InitialMethod->Grating);
    success = Output1235CMD (TmpBuf);           /* send command to 1235 */
    if (success)
      {
      sprintf (TmpBuf, "%.2f INIT-WAVELENGTH \r", M1235_W0);
      success = Output1235CMD (TmpBuf);         /* send command to 1235 */
      goto_local (M1235_ADDR);                  /* tell m1235 GTL */
      }
    }
  return success;
}

  /* This makes sure the coefficients are ok after grating/wavelength
   * changes.
   */
void RedoCalCoeffs (void)
{
  float *aptr = InitialMethod->CalibCoeff[0];

  if (aptr[1] < F0)
    M1235Flag |= (char)0x80;
  
  memset((void *)aptr, 0, 4 * sizeof(float));
  aptr[1] = (float)1.0;               /* assume uncalibrated */
  if (GrPtr)
    {                            /* A1 = a0 + a1 * W0 for 25 micron
                                  * so multiply A1 by RelPxlWidth.*/
    aptr[1] = RelPixelWidth * (GrPtr[2] + GrPtr[3] * W0);
                                      /* A0 = W0 - A1*P0 + WC */                                                    
    aptr[0] = W0 - aptr[1]*P0[GrNo] + GrPtr[0] + W0*GrPtr[1];
    InitialMethod->CalibUnits[0] = ADJ_NM; /* flag that coef's are calculated. */
    }
  RegenXData(aptr); /* regenerate X data for live curves */
}

/*  Want to load up the correct values for the spectrograph and gratings 
                   0123456789012345678901234567
 GratingCoverage: "Coverage  435.8nm - 546.1nm "

                   0123456789012345678901234567890 
 GratingLabels:   " 1       0.01512       444.5 ",  etc for 2, 3
 
*/


int Load1235Registry (void)
{
  switch (SpecTog)
    {
    case 1:
      M1235Form.number_of_fields =
           sizeof(M1235FormFields) / sizeof (M1235FormFields [0]);
      Spectrograph = 1235;
      break;
    case 2:
    default:
      Spectrograph = 0;
      M1235Form.number_of_fields = 3;
      break;
    }
  InitialMethod->Spectrograph = Spectrograph;
  draw_form();
  return (FIELD_VALIDATE_SUCCESS);
}

int MacSetWV(float Wavelen)
{
  WV = Wavelen;
  SetCenterLambda();
  return(1);
}

int MacChooseSpectrograph(int model)
{
  if ((model == 1235) || (model == 1236) || (model == 1237))
    {
    Spectrograph = model;
    SpecTog = 1;
    InitialMethod->Spectrograph = model;
    M1235Form.number_of_fields = sizeof(M1235FormFields)/sizeof(FIELD);
    }
  else
    {
    Spectrograph = 0;
    M1235Form.number_of_fields = 3;
    SpecTog = 0;
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
  float temp;
  Spectrograph = InitialMethod->Spectrograph;
  Grating = InitialMethod->Grating - 1;
  for (i = 0; i < 3; i++)
    {
    int type = WhichGratingInTable(i);
    float Cover = F0;
    if (type < MAX_GRATING)           /* found a known grating.*/
      {
      GetParam(DC_ACTIVEX, &temp);
      Cover = temp *  RelPixelWidth * GratingInfo[type].a[0];
      }
    sprintf(GratingLabels[i]+9, "%4.4g         %7.2f", GroovesPmm[i], Cover);

    GratingLabels[i][0] = (char)((GrNo == i) ? '>' : ' ');
    }
  LeftLambda = DataPtToLambda (0);
  GetParam(DC_POINTS, &temp);

  RightLambda = DataPtToLambda ((int)temp - 1);
  sprintf(GratingCoverage + 10, "%.2f - %.2f %s", LeftLambda, RightLambda,
            WaveLengthOptions[InitialMethod->CalibUnits[0]]);
  WV = (float)0.5 * (LeftLambda + RightLambda);
}


  /*********************************************************/
  /*                                                       */
  /* Reset the P0, given one wavelength and one channel.   */
  /* This also forces the use of calculated coefficients.  */
  /*                                                       */
  /*********************************************************/
int MacOnePtResetOffset (float Chnl, float Wvlngth)
{
  if (GrPtr)
    {                          /* A1 = a0 + a1 * W0 for 25 micron
                                * so multiply A1 by RelPxlWidth.*/
    float A1 = RelPixelWidth * (GrPtr[2] + GrPtr[3] * W0);
    P0  [GrNo] = (W0 + GrPtr[0]+W0 * GrPtr[1] - Wvlngth + A1*Chnl) / A1;
    }
  RedoCalCoeffs();                  /* reset live coeff's */
  InitialMethod->GratingCenterChnl[GrNo] = P0[GrNo];
  AdjustFieldLabels();
  return (M1235Flag ? 1 : 0);
}

int OnePtResetOffset (void)
{
  int index, success;
  float Chnl, Wvlngth;
  double Temp;

  index = index_of_scroll_form(FormTable[KSI_XCAL_SCROLL_FORM]);
  if (index == -1)
    {
    get_new_points();
    index = index_of_scroll_form(FormTable[KSI_XCAL_SCROLL_FORM]);
    }
  if (index >= 0)
    {                                 /* Get the pixel value.*/
    Chnl = (float)CurrentCalibrationPoint->x_value;
    ConvertUnits (COUNTS, &Temp, ADJ_NM, (double)Chnl, 0.0);
    Chnl = (float)Temp;               /* Have to convert it to counts */
    Wvlngth = (float)CurrentCalibrationPoint->calibration_value;
    success = MacOnePtResetOffset(Chnl, Wvlngth);
    draw_form();
    }
  else
    success = 0;

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
    float Points,  temp;
    GetParam(DC_POINTS, &Points);
    Points--;                         /* Number of data points - 1. */
    /***********************************************************************************/
    /* New center wavelength, WV */  
    WV = 0.5F * (DataPtToLambda(0) + DataPtToLambda((int)Points));
    temp = (W0 + GrPtr[0] + W0 * GrPtr[1] - WV) / (GrPtr[2] + W0 * GrPtr[3]);
    InitialMethod->RelPixelWidth =    /* RelPxlWdth = real_nm / calc_nm.*/
    RelPixelWidth =
           (DataPtToLambda ((int)Points) - DataPtToLambda (0))/
                     (Points * (GrPtr[2] + GrPtr[3] * W0));
    temp /= RelPixelWidth;            /* Account for less than 25 micorn*/

    if ((M1235Flag) && (InitialMethod->CalibCoeff[0][1] < 0.0F))
      M1235Flag |= (char)(0x80);

    InitialMethod->GratingCenterChnl[GrNo] =
    P0[GrNo] = 0.5F * (DataPtToChnl(0) + DataPtToChnl((int)Points)) + temp;
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
  char TmpBuf[32];
  GrPtr = NULL;                       /* Flag unknown grating for safety */
  if ((grating > 0) && (grating < 4))
    {
    int type = 0;           /* search for grooves per mm to do A0 and A1.*/
    grating--;              /* put back into units the computer likes */
    Grating = grating;

    type = WhichGratingInTable(grating);

    if (type < MAX_GRATING)           /* found a known grating; un-NULL GrPtr.*/
      GrPtr = &GratingInfo[type].wc[0];
    if (grating != GrNo)              /* if not current grating, move 1235.*/
      {
      GratingLabels[GrNo][0] = ' ';   /* erase arrow pointing at current. */
      GrNo = grating++;               /* reset current. */
      sprintf(TmpBuf, "%i GRATING \r", grating);
      Output1235CMD (TmpBuf);         /* send command string to interface */
      InitialMethod->Grating = grating; /* reset curve parameter */
      RedoCalCoeffs ();               /* reset live coeff's */
      }
    }
  return (M1235Flag ? FIELD_VALIDATE_SUCCESS : FIELD_VALIDATE_WARNING);
}

/*************************************************************************/
/* Helper function for Reset1235 and SetCenterLambda                     */
/*************************************************************************/
void CalcNewW0(void)
{
  float ftemp, DeltaP;
  int Points;

  GetParam(DC_POINTS, &ftemp);
  Points = (int)ftemp - 1;

  ftemp  = P0[GrNo] - DataPtToChnl(0);             /* from here */
  ftemp -= 0.5F * DataPtToChnl(Points);
  DeltaP = RelPixelWidth * ftemp;

  /* Solve Eq(1) for W0 */

  W0    = WV + GrPtr[2] * DeltaP;                  /* from here */
  W0    = W0 - GrPtr[0];
  ftemp = 1.0F + GrPtr[1];
  ftemp -= GrPtr[3] * DeltaP;
  W0    = W0  / ftemp;
}

    /* Routine done on powerup, or if powerup is complete, to initiate
     * conversation with the 1235.  The 1235 only reports it's setting
     * to 0.1 nm, and this is ignored unless system can't autocalibrate.
     */
#pragma optimize("",off)

void DoReset1235 (void)
{
  char TmpBuf [80], *cptr;
  int i, success, Points;
  float ftemp;
  unsigned char spoll_byte;
  BOOLEAN Calibrated;

  success = serial_poll_gpib(M1235_ADDR, &spoll_byte);

  if (ActonSpectrograph() && success)    /* make sure it's ok to reset.*/
    {
    GetParam(DC_DMODEL, &ftemp);
    switch ((int)ftemp)
      {
      case TSC_CCD:
      case TSM_CCD:
      case TSP_CCD:
        RelPixelWidth = (float) (19.0 / 25.0); /* pixel width in 25 um units */
      break;
      default:
        RelPixelWidth = (float) 1.0;       /* width of pixel in 25 um units*/
      break;
      }
    Spectrograph = InitialMethod->Spectrograph;
    SpecTog = 1;
    Grating = InitialMethod->Grating - 1;
    M1235Flag = 1;
 
    Output1235CMD ("?NM \r");       /* send simple string; spoll bit 0=?.*/
    if (!M1235Flag)
      return;
    Get1235Response (TmpBuf, 80);   /* read response to ?NM */
    W0 = (float) (atof (TmpBuf));   /* convert to wavelength */
    WV = W0;                        /* Find Acton/EG&G model #... */

    if (!Output1235CMD("WHO \r"))
      return;                       /* if error */

    Get1235Response (TmpBuf, 80);   /* read response string */
    if (strstr(TmpBuf,"SP-150"))    /* SP-150 translates to 1239 in EG&G */
      M1235Flag = (1239 - 1234);    /* So you can see the logic */
    else
      {
      if (!Output1235CMD("EESERIAL EE@ U.\r"))  /* read serial number */
        return;                     /* if error */

      Get1235Response (TmpBuf, 80); /* read response serail number */
      if (atof (TmpBuf) > 50000.0)  /* 1236 has serial number > 50,000 */
        {
        M1235Flag++;
        Output1235CMD ("MFRONT\r"); /* put mirror on front pos.*/

        serial_poll_gpib(M1235_ADDR, &spoll_byte);
        if (spoll_byte & 0X82)
          Get1235Response (TmpBuf, 80); /* read possible error message. */
        else
          M1235Flag++;              /* If no error, it's a 1237. */
        }
      }
    
    SpectrographOptions[1][5] = (char)'4' + (char)M1235Flag;

    if (M1235Flag == (1239 - 1234))
      GratingInfo = GratingInfoM1239;
    else if (M1235Flag == (1235 - 1234))
      GratingInfo = GratingInfoM1235;
    else
      GratingInfo = GratingInfoM1236;

    Output1235CMD ("?GRATINGS \r"); /* ask about the gratings for today */
    Get1235Response (TmpBuf, 80);
    cptr = TmpBuf + 2;              /* reply starts off with 2 spaces. */
    for (i = 0; i < 3; i++)         /* stuff the array for labels.*/
      {
      GroovesPmm[i] = (float)atof(cptr + 2);
      if (cptr[0] == 26)            /* find the active grating*/  
        GrNo = i;
      cptr += 23;

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
      M1235Flag |= (char)(0x80);
      if (RelPixelWidth < F0)
        RelPixelWidth = -RelPixelWidth;
      }
                                          /* if units = nm, and grating */
    if (InitialMethod->CalibUnits[0] == NM && GrPtr)
      {                                   
      float DeltaP;  /* DeltaP = P0(center of Spgraph) - PV(center of view)*/

      /* DeltaP = P0 - 1/2(LeftPixel + RightPixel) */

      DeltaP  = P0[GrNo] - (0.5F * (DataPtToChnl(0) + DataPtToChnl(Points)));
      M1235_W0 = W0;  /* save old value just read. */
      
      /* solve eqn 1 for W0. */

      W0 = WV + RelPixelWidth * GrPtr[2] * DeltaP;
      W0 = W0 - GrPtr[0];
      ftemp = 1.0F + GrPtr[1];
      ftemp -= RelPixelWidth * GrPtr[3] * DeltaP;
      W0 = W0 / ftemp;

      if (M1235_W0 != W0)
        {
        sprintf (TmpBuf, "%.2f <GOTO> \r", W0);
        Output1235CMD (TmpBuf);             /* send command to 1235 */
//        RedoCalCoeffs();
        }
      }
    else
      SetCenterLambda();
    M1235_W0 = W0;
    }
}
#pragma optimize("",on)


int Reset1235 (void)
{
  DoReset1235();
  return (M1235Flag ? FIELD_VALIDATE_SUCCESS : FIELD_VALIDATE_WARNING);
}

int MacRedo1235(void)
{
  Reset1235();
  AdjustFieldLabels ();
  return (M1235Flag != 0);
}

int Redo1235(void)
{
  int success = MacRedo1235();
  draw_form();
  return (success ? FIELD_VALIDATE_SUCCESS : FIELD_VALIDATE_WARNING);
}

    /* The global parameter "WV" contains the wavelength WV at the center
     * of the scanned window - in nm.  If a known grating, calculates
     * the 1235 wavelength "W0" to put "WV" in the middle of the scanned
     * window.  If unknown grating, just sets W0 = WV.
     * It sends the 1235 to W0.
     */

int SetCenterLambda (void)  /* WV is lambda at center of window - in nm. */
{
  char TmpBuf[32];            /* for sprintf's */
  W0 = WV;                    /* assume one to one correspondence */
  if (GrPtr)                  /* found a known grating */
    {                         /* calculate (P0 - PV) */
    float ftemp, DeltaP;
    int Points;

    GetParam(DC_POINTS, &ftemp);
    Points = (int)ftemp - 1;

    ftemp  = P0[GrNo] - DataPtToChnl(0);             /* from here */
    ftemp -= 0.5F * DataPtToChnl(Points);
    DeltaP = RelPixelWidth * ftemp;

    /* Solve Eq(1) for W0 */

    W0    = WV + GrPtr[2] * DeltaP;                  /* from here */
    W0    = W0 - GrPtr[0];
    ftemp = 1.0F + GrPtr[1];
    ftemp -= GrPtr[3] * DeltaP;
    W0    = W0  / ftemp;
    }
  RedoCalCoeffs ();                   /* reset live coeff's */
  sprintf (TmpBuf, "%.2f <GOTO> \r", W0);
  Output1235CMD (TmpBuf);             /* send command to 1235 */
  M1235_W0 = W0;
  return (M1235Flag ? FIELD_VALIDATE_SUCCESS : FIELD_VALIDATE_WARNING);
}

int MacSet1235 (void)
{
  ChangeGrating(Grating + 1);
  SetCenterLambda ();         /* WV is lambda at center of window - in nm. */
  AdjustFieldLabels ();
  return (M1235Flag != 0);
}

int Set1235 (void)
{
  int success = MacSet1235();
  draw_form();
  return (success ? FIELD_VALIDATE_SUCCESS : FIELD_VALIDATE_WARNING);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void registerSPGraphForm(void)
{
   FormTable[KSI_SPGRAPH_FORM] = &M1235Form;
}
