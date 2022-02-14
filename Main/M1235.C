/* Copyright 1991 EG & G  PARC ***********************************************
 *
 *  M1235.C       Jan 17, 1991
 *
 *  Control routines for the 1235.
 *
 ****************************************************************************/

#include "M1235.h"
#include "CTRL1471.h"
#include "System.h"
#include "User.h"
#include "menu.h"
#include "plasmenu.h"
#include "loadsave.h"
#include "aufnahme.h"
#include "scrio.h"
#include "graph.h"
#include "GrCtrl.h"
#include "GrRtns.h"
#include "input.h"
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dos.h>

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

#define MAX_GRATING 7
char M1235Flag = 0;               /* 1 if 1235 found on bus, 0 if not */
static int GrNo = 0;              /* currently selected grating number*/
static float RelPixelWidth        /* width of detector pixel in 25æ units. */
        = (float)1.0;     
static float WV;                  /* center of current window */
static float W0;                  /* current 1235 wavelength reading */
float M1235_W0;                   /* current 1235 wavelength reading */
float P0 [3];                     /* Channel number corresponding to
                                   * above wavelength. In perfect world,
                                   * would be center of diode array. */

static int GroovesPmm [3] = { 0, 0, 0, };
static float *GrPtr;              /* if nonNULL, points to grating info */

           /* labels for menu; replaced by grating info after init */
static char GratingLabels[4][22] =  {{"1235 does not respond"},
                                     {"\x01A Turn ON 1235, then "},
                                     {"Press R to Reset 1235"},
                                     {"                     "}};

struct GratingType          /* for setting calibration after moving 1235 */
  {
  int Grooves;              /* grooves per mm */
  float wc[2];              /* wc0, wc1, to generate WC = wc0 + wc1*W0 */
  float  a[2];              /* a0, a1, to generate A1 = a0 + a1*W0 */
  };

struct GratingType GratingInfoM1235 [MAX_GRATING] = {
  /*  g/mm: */
  {  147, /* wc: */  { (float)(-.29986),     (float)(4.3355E-4),   },
          /* a:  */  { (float)(0.610703),    (float)(-8.3501E-6),  }},

  {  150, /* wc: */  { (float)(-.19064),     (float)(-1.39547E-4), },
          /* a:  */  { (float)(0.601754),    (float)(-7.45414E-6), }},

  {  300, /* wc: */  {(float)(.157464),      (float)(-6.05988E-4), },
          /* a:  */  { (float)(0.300498),    (float)(-8.10197E-6), }},

  {  600, /* wc: */  { (float)(-.03314),     (float)(-4.58987E-5), },
          /* a:  */  { (float)(0.152551),    (float)(-1.62572E-5), }},

  { 1200, /* wc: */  { (float)(-.18356),     (float)(1.215013E-4), },
          /* a:  */  { (float)(0.078786),    (float)(-2.24731E-5), }},

  { 1800, /* wc: */  {F0, F0, },
          /* a:  */  { (float) 0.03837, F0, }},

  { 2400, /* wc: */  {F0, F0, },
          /* a:  */  { (float) 0.03, F0, }},};

struct GratingType GratingInfoM1236 [MAX_GRATING] = {

  /*  g/mm: */
  {  147, /* wc: */  { (float)(0),     (float)(0),   },
          /* a:  */  { (float)(0.326),    (float)(0),  }},
  {  150, /* wc: */  { (float)(0),     (float)(0), },
          /* a:  */  { (float)(0.3263),    (float)(-4.8667E-6), }},

  {  300, /* wc: */  {(float)(0),      (float)(0), },
          /* a:  */  { (float)(0.150),    (float)(0), }},

  {  600, /* wc: */  { (float)(0),     (float)(0), },
          /* a:  */  { (float)(0.075),    (float)(0), }},

  { 1200, /* wc: */  { (float)(0),     (float)(0), },
          /* a:  */  { (float)(0.034),    (float)(0), }},

  { 1800, /* wc: */  {F0, F0, },
          /* a:  */  { (float) 0.01, F0, }},

  { 2400, /* wc: */  {F0, F0, },
          /* a:  */  { (float) 0.015, F0, }},};

    /* Initialize pointer to 1235 as default. */
struct GratingType * GratingInfo = GratingInfoM1235;


MENUITEM M1235Entries [] =
{
{ "Center Wavelength",  0, 1,  0, NORM,NULL},
{ "          ",         0, 2,  0, NOTCHOICE | INPUT_FLT, & WV },
{ "nm",                 0, 2, 11, NOTCHOICE,NULL},
{ GratingLabels[0],     0, 3,  0, NORM,NULL},
{ GratingLabels[1],     0, 4,  0, NORM,NULL},
{ GratingLabels[2],     7, 5,  0, NORM,NULL},
{ GratingLabels[3],     0, 6,  0, NORM,NULL},
};

MENU M1235Menu = {
  "    M1235 Control     ",
  1,1,
  7,3,
  NORMAL,
  NULL,
  M1235Entries
};
            /* make sure it's ok to reset.*/
int ActonSpectrograph (void)
{
  return ((LiveCurveSet.Spectrograph >= 1235) &&
          (LiveCurveSet.Spectrograph <= 1239) &&
            LiveCurveSet.Grating);
}

  /* This makes sure the coefficients are ok after grating/wavelength
   * changes. It also keeps the areas wavelength dominated, i.e. if
   * area 1 is 400-408 nm before, it adjusts the beginning and end
   * channels so area 1 is as close to the same region as possible.
   * However, as the wavelengths are seldom chosen for exact channel
   * values, the areas will shift, and can, over several wavelength
   * changes, become inaccurate! Manual should warn !!
   */
void RedoCalCoeffs ( void)
{
  struct CurveSetType *Temp_CS;
  double *aptr = & LiveCurveSet.CalData.a[0];
  Temp_CS = AllocHeader();
  if (Temp_CS)                        /* room for the temporary */
    {
    *Temp_CS = LiveCurveSet;          /* copy detector parameters */
                                      /* Un-cal Wavelength */
    memset((void *)&LiveCurveSet.CalData, 0, sizeof(struct PolyType));
    LiveCurveSet.CalData.n = 1;       /* force linear. */
    aptr[1] = 1.0;                    /* assume uncalibrated */
    if (GrPtr)
      {                               /* A1 = a0 + a1 * W0 for 25 micron
                                       * so multiply A1 by RelPxlWidth.*/
      aptr[1] = (double)( RelPixelWidth * (GrPtr[2] + GrPtr[3] * W0) );
                                      /* A0 = W0 - A1*P0 + WC */                                                    
      aptr[0] = (double)(W0 - aptr[1]*P0[GrNo] + GrPtr[0] + W0*GrPtr[1]);
      LiveCurveSet.CalData.XAxis = 2; /* flag that coef's are calculated. */
                                      /* reset any areas for new cal */
      AdjustAreas (&LiveCurveSet, Temp_CS, LiveCurveSet.MaxChan);
      FreeStrips (&LiveCurveSet);     /* yt data no longer valid */
                                      /* now see if equations still good */
      CalcEquQueues(LiveCurveSet.Areas, LiveCurveSet.Equations);
      }
    free(Temp_CS);
    }
  else
    MemoryError();
}

              /* Reset the P0, given one wavelength and one channel.  */
              /* This also forces the use of calculated coefficients. */
void OnePtResetOffset (float Chnl, float Wvlngth)
{
  if (GrPtr)
    {                                 /* A1 = a0 + a1 * W0 for 25 micron
                                       * so multiply A1 by RelPxlWidth.*/
    float A1 = RelPixelWidth * (GrPtr[2] + GrPtr[3] * W0);
                                      /* P0 = (W0 + WC - W + A1*P) / A1 */
    P0  [GrNo] = (W0 + GrPtr[0]+W0 * GrPtr[1] - Wvlngth + A1*Chnl) / A1;
    }
  RedoCalCoeffs ();                   /* reset live coeff's */
}


    /* reset after a real wavelength calibration, so don't overwrite
     * the a[0] and a[1]; just adjust P0 so the 1235 would calculate
     * the same W0, given the new a[0]...a[3].*/
 /* This resets "P0" and "RelPixelWidth" after a wavelength calibration,
  * and should not overwrite the a[0] and a[1].  It gets RelPixelWidth by
  * calculating real coverage and dividing that by the calculated coverage.
  * P0 is calculated using W0, WV, and RelPixelWidth from equation 1.
  */
void ResetOffset ( void)
{
  if (GrPtr)
    {                                 /* Solve eqn 1 for P0.. */
    float temp =  (W0 + GrPtr[0] + W0 * GrPtr[1] -
            EvalPoly((float)LiveCurveSet.MaxChan/(float)2.0,&LiveCurveSet))/
            (GrPtr[2] + W0 * GrPtr[3]);
    RelPixelWidth =
            (EvalPoly((float)LiveCurveSet.MaxChan,&LiveCurveSet) -
            EvalPoly(F0,&LiveCurveSet))/
            ((float)LiveCurveSet.MaxChan * (GrPtr[2] + GrPtr[3] * W0));
    temp /= RelPixelWidth;            /* Account for less than 25 micorn*/
//    if (M1235Flag && (LiveCurveSet.CalData.a[1] < 0))
//      temp = -temp;
    P0  [GrNo] = (float)LiveCurveSet.MinChan +
                 (float)LiveCurveSet.MaxChan/(float)2.0  + temp;
    }
}
 
     /* If "grating" is 1, 2, or 3, searches to see if data held about
      * the grating.  If grating not same as GrNo  - current grating - will
      * change to new grating and recalibrate system - if known grating;
      * if unknown grating, will uncalibrate system. */

void ChangeGrating(int grating, int MoveSpec)
{
  char TmpBuf[32];
  GrPtr = NULL;                       /* Flag unknown grating for safety */
  if ((grating > 0) && (grating < 4))
    {
    int type = 0;           /* search for grooves per mm to do A0 and A1.*/
    grating--;              /* put back into units the computer likes */
    while((type < MAX_GRATING) &&
          (GroovesPmm[grating]!=GratingInfo[type].Grooves))
      type++;

    if (type < MAX_GRATING)           /* found a known grating; un-NULL GrPtr.*/
      GrPtr = &GratingInfo[type].wc[0];
    if (grating != GrNo)              /* if not current grating, move 1235.*/
      {
      GratingLabels [GrNo][0] = ' ';  /* erase arrow pointing at current. */
      GrNo = grating++;               /* reset current. */
      sprintf(TmpBuf, "%i GRATING \r", grating);
      if (MoveSpec)
        Output1235CMD (TmpBuf);       /* send command string to interface */
      LiveCurveSet.Grating = grating; /* reset curve parameter */
      GratingLabels [GrNo][0] = 0x1A; /* put arrow pointing to new grating.*/
      }
    }
}

    /* Routine done on powerup, or if powerup is complete, to initiate
     * conversation with the 1235.  The 1235 only reports it's setting
     * to 0.1 nm, and this is ignored unless system can't autocalibrate.
     */
void Reset1235 (void )
{
  char TmpBuf [80], *cptr;
  int i;

  if (ActonSpectrograph())            /* make sure it's ok to reset.*/
    {
    M1235Flag = 1;
    Output1235CMD ("?NM \r");         /* send simple string; spoll bit 0=?.*/
    if (M1235Flag)                    /* if no error */
      {
      Get1235Response (TmpBuf, 80);   /* read response to ?NM */
      W0 = (float) (atof (TmpBuf));   /* convert to wavelength */
      Output1235CMD ("EESERIAL EE@ U.\r");  /* read serial number */
      if (M1235Flag)                    /* if no error */
        {
        Get1235Response (TmpBuf, 80);   /* read response serial number */
        WV = (float)atof (TmpBuf);      /* convert to number */
                                        /* 1236 has serial number > 50,000;
                                         * If unit has no serial number,
                                         * it returns 0xffff == 65535.
                                         */
        if ((WV > (float)15000.0) && (WV < (float)20000.0))
         {
          M1235Flag = 5;
          XCalEntries[2].text[4] = 
          M1235Menu.header_text[8] = '9';

         }
        else if ((WV > (float)50000.0) && (WV < (float)60000.0))
          {
          M1235Flag++;
          Output1235CMD ("MFRONT\r");  /* put mirror on front pos.*/
          if (Spoll1235() & 0X82)
            Get1235Response (TmpBuf, 80);   /* read possible error message.*/
          else
            M1235Flag++;
          XCalEntries[2].text[4] = 
          M1235Menu.header_text[8] = (char)( '4' + M1235Flag);
          }
        }
      GratingInfo = (void *) (((M1235Flag >= 2) && (M1235Flag != 5)) ?
                                                  GratingInfoM1236 :
                                                  GratingInfoM1235);

      if (M1235Flag == 5)
         {
         int i;
         for (i = 0; i < MAX_GRATING; i++)
           GratingInfo[i].a[0] *= 2;
         }


      LiveCurveSet.Spectrograph = 1234 + (int)M1235Flag;
     
      Output1235CMD ("?GRATINGS \r"); /* ask about the gratings for today */
      Get1235Response (TmpBuf, 80);
      cptr = TmpBuf + 2;
      for (i = 0; i < 3; i++)         /* stuff the array for labels.*/
        {
        strncpy (GratingLabels[i], cptr, 21);
        GratingLabels[i][21] = 0;
        GroovesPmm [i] = atoi (cptr + 2);
        cptr += 23;
        M1235Menu.item[i+3].key_char = 2; /* change action to grating num.*/
        if (GratingLabels[i][0] == 26)    /* find the active grating*/  
          GrNo = i;             
        if (!P0 [i])                      /* set up P0 to default if zero */
          P0 [i] = (float)(LiveCurveSet.ArrayLength - 1)/(float)2.0;
        }
      strcpy (GratingLabels[3], "Reset 1235 Offset");
      M1235Menu.item[0].key_char = 8;       /* activate on "W" */
      M1235Menu.item[6].key_char = 12;      /* activate on "O" */
      M1235Menu.entry_item = 0;

    /* Finished the menu, now do the spectrograph. */
      {                             /* search for grating */
      int CoefFlag = LiveCurveSet.Grating != (GrNo + 1);

      ChangeGrating (LiveCurveSet.Grating, TRUE);
      if (CoefFlag &&
         (LiveCurveSet.CalData.XAxis != 1)) /* If not really calibrated, */
        RedoCalCoeffs ();                   /* reset live coeff's */
      }
      WV = LiveCurveSet.CalData.XAxis ?     /* calibrated? if so, set WV */
           EvalPoly((float)(LiveCurveSet.MaxChan)/(float)2.0,&LiveCurveSet) :
           W0;                              /* if not WV = W0. */

// save rpw on disk ???
                                      /* Flag if calibrated backwards. */
      if ((LiveCurveSet.CalData.a[1] < 0) && (RelPixelWidth > 0))
        RelPixelWidth = -RelPixelWidth;
                                            /* if really cal'd: */
      if ((LiveCurveSet.CalData.XAxis == 1) && GrPtr)
        {                                   /* solve eqn 1 for W0. */
        float DP = P0[GrNo] - ((float)LiveCurveSet.MinChan +
                        (float)LiveCurveSet.MaxChan/(float)2.0);
        M1235_W0 = W0;                      /* save old value just read. */
        W0 = (WV + GrPtr[2]*DP - GrPtr[0]) / (1 + GrPtr[1] - GrPtr[3]*DP);

        if (M1235_W0 != W0)
          {
          sprintf (TmpBuf, "%.2f <NM> \r", W0);
          Output1235CMD (TmpBuf);             /* send command to 1235 */
          }
        }
      else
        SetCenterLambda(TRUE);
      M1235_W0 = W0;
      }
    }
}

    /* The global parameter "WV" contains the wavelength at the
     * center of window - in nm.     If a known grating, calculates
     * the 1235 wavelength "W0" to put "WV" in the middle of
     * the scanned window.  If unknown grating, just sets W0 = WV.
     * It sends the 1235 to W0.
     */
void SetCenterLambda (int MoveSpec)
{
  char TmpBuf[32];            /* for sprintf's */
  W0 = WV;                    /* assume one to one correspondence */
  if (GrPtr)                  /* found a known grating */
    {                         /* calculate (P0 - PV) */
    float DeltaP = RelPixelWidth * (P0 [GrNo] -
      (float)LiveCurveSet.MinChan - (float)LiveCurveSet.MaxChan*(float)0.5);
                              /* Solve Eq (1) */
    W0 = (WV + GrPtr[2]*DeltaP - GrPtr[0]) /
                  ((float)1 + GrPtr[1] - GrPtr[3]*DeltaP);
    }
  RedoCalCoeffs ();                   /* reset live coeff's */
/*  sprintf (TmpBuf, "%.2f <NM> \r", W0);  faster than below.*/
  sprintf (TmpBuf, "%.2f <GOTO> \r", W0);
  if (MoveSpec)
    Output1235CMD (TmpBuf);           /* send command to 1235 */
  M1235_W0 = W0;
}

void GetCenterLambda (int MoveSpec)
{
  if (!ReadFloat (& WV, F0, (float)2500.0,
                "Enter Center Wavelength (nm): "))
     SetCenterLambda (MoveSpec);
}


void M1235Loop (void)
{
  int Key,
      depth = menu_depth;
  UpdateMenu();
  while (menu_depth >= depth)   /* loop til this menu done. */
    {
    Key = TranslateKey();
    if (Key >= 0 && Key < 4)
      {
      EraseMenu();                    /* allows done flag */
      if (Key)
        {
        char buf[32];
        sprintf (buf, "Change to grating %i", Key);
        PrintIAText (buf, I_CLEAR);
        if (GetJN ())               /* allow saying "no" */
          {
          ChangeGrating (Key, TRUE);
          RedoCalCoeffs ();               /* reset live coeff's */
          SetCenterLambda (TRUE);
          UpdateMenu();
          }
        } 
      else
        {
        if (GratingLabels[2][0] != 'P')  /* start of third is "Press R to" */
          {
          ClrInfoArea();
          GetCenterLambda (TRUE);
          }
        else
          {
          M1235Menu.entry_item = 3;
          PrintIAText ("Reset model 1235", I_CLEAR);
          if (GetJN ())               /* Try to talk to 1235 */
            Reset1235 ();
          }
        }
      DrawMenu();
      SetInfoArea ();
      }
    else if (Key == 4)
      {
      SaveKey(31);                /* non alphanumeric, so can't be typed */
      EraseMenus();
      SetCursorPar();
      CalXAxis ();
      LiveCurveSet.CalData.XAxis = 2;
      ReDrawMenus();
      SetIArea(ACQUIS_MENU);
      }

/* DEBUG !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
    else if (Key == SHF_KEY2)
      {
      printf ("RPW %g ",RelPixelWidth);
      printf ("P0[3], %g, ",P0[2]);
      printf ("W0 %g, ",W0);
      printf(" A0 %g, ", LiveCurveSet.CalData.a[0]);
      printf(" A1 %g ", LiveCurveSet.CalData.a[1]);
      printf(" A2 %g ", LiveCurveSet.CalData.a[2]);
      printf(" A3 %g\n", LiveCurveSet.CalData.a[3]);
      }
*/

    else
      InstantAction(Key);     /* allow live... */
    }
}
