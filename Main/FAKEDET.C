#include <windows.h>
#include <math.h>
#include "oma4driv.h"

typedef struct {
  float Value;   // Current value of parameter
  float Max;     // Maximum legal value
  float Min;     // Minimum legal value
  int   Command; // Val of command to set or get param
  } Param;

Param Antibloom;
Param ActiveX;
Param ActiveY;
Param DAProg;
Param DeltaX;
Param DeltaY;
Param DetTemp;
Param DModel;
Param DCType;
Param ExpTime;
Param Ignores;
Param Mems;
Param Points;
Param Preps;
Param PTime;
Param Scans;
Param Shutter;
Param Sreg;
Param STime;
Param Tracks;
Param TrackMode;
Param WFTO;
Param X0;
Param Y0;

static float ETScaler;       // Ditto
static int   DetErr;
static int   Incrementing;   // Flag to eliminate recursion
static char  SaveString[80]; // For Cut & Paste ops
static char  FileName[80];   // Name returned by Frm_FIle
static HINSTANCE Instance;   // Used to release DLL


static void LimitTracks(void)
{
  Tracks.Max = ActiveY.Value;
  if(Tracks.Value > Tracks.Max)
    Tracks.Value = Tracks.Max;
}

int GetOMA(SHORT Cmd, float *Setting)
{
  static int lcount;

  if (DModel.Value > -1)
    return GetParam(Cmd, Setting);
  else
    {
    switch(Cmd)
      {
      case DC_THERE:
      case DC_DMODEL:
      case DC_X0:
      case DC_Y0:
      case DC_DELTAX:
      case DC_TRACK:
      case DC_TRACKS:
        *Setting = 1.0F;
      break;
      case DC_TRKMODE:
      case DC_DAPROG:
      case DC_I:
      case DC_J:
        *Setting = 1.0F;
      break;
      case DC_L:
        *Setting = lcount++;
      break;
      case DC_DELTAY:
        *Setting = 1.0F;
      break;
      case DC_DERROR:
      case DC_SHUTMODE:
      case DC_COOLONOFF:
      case DC_RUN:
      case DC_STOP:
      case DC_WFTC:
        *Setting = 0.0F;
      break;
      case DC_POINTS:
      case DC_ACTIVEX:
      case DC_ACTIVEY:
        *Setting = 512.0F;
      break;
      case DC_ANTIBLOOM:
        *Setting = 100.0F;
      break;
      case DC_MINET:
        *Setting = 0.01F;
      break;
      case DC_MAXET:
        *Setting = 120.0F;
      break;
      case DC_WFTO:
        *Setting = 1.0F;
      break;
      case DC_ET:
      case DC_SAMEET:
      case DC_FTIME:
        *Setting = 0.04F;
      break;
      case DC_PTIME:
        *Setting = 0.00001F;
      break;
      case DC_BYTES:
        *Setting = 2048.0F;
      break;
      case DC_MAXMEM:
        *Setting = 128.0F;
      break;
      default:
        *Setting = 0.0F;
      }
    }
  return 0;
}

// Set the Max and Min values and the command value for all
// of the detector parameters
// Then query the driver for the actual values
void InitParams(void)
{
  float FParam;
  
  GetOMA(DC_ACTIVEX,   &ActiveX.Value);
  GetOMA(DC_ACTIVEY,   &ActiveY.Value);
  GetOMA(DC_ANTIBLOOM, &Antibloom.Value);
  GetOMA(DC_DAPROG,    &DAProg.Value);
  GetOMA(DC_DELTAX,    &DeltaX.Value);
  GetOMA(DC_DELTAY,    &DeltaY.Value);
  GetOMA(DC_DTEMP,     &DetTemp.Value);
  GetOMA(DC_ET,        &ExpTime.Value);
  GetOMA(DC_K,         &Ignores.Value);
  GetOMA(DC_J,         &Mems.Value);
  GetOMA(DC_POINTS,    &Points.Value);
  GetOMA(DC_PTIME,     &PTime.Value);
  GetOMA(DC_H,         &Preps.Value);
  GetOMA(DC_I,         &Scans.Value);
  GetOMA(DC_SHUTMODE,  &Shutter.Value);
  GetOMA(DC_OUTREG,    &Sreg.Value);
  GetOMA(DC_STIME,     &STime.Value);
  GetOMA(DC_TRACKS,    &Tracks.Value);
  GetOMA(DC_TRKMODE,   &TrackMode.Value);
  GetOMA(DC_WFTO,      &WFTO.Value);
  GetOMA(DC_X0,        &X0.Value);
  GetOMA(DC_Y0,        &Y0.Value);

  Antibloom.Max = 100;
  Antibloom.Min = 0;
  Antibloom.Command = DC_ANTIBLOOM;
  ActiveX.Command = DC_ACTIVEX;
  ActiveY.Command = DC_ACTIVEY;
  DAProg.Command = DC_DAPROG;
  GetOMA(DC_DAMAX, &DAProg.Max);
  DAProg.Min = 1;
  DeltaX.Command = DC_DELTAX;
  DeltaX.Max = ActiveX.Value;
  DeltaX.Min = 1;
  DeltaY.Command = DC_DELTAY;
  DeltaY.Max = ActiveY.Value;
  DeltaY.Min = 1;
  DetTemp.Command = DC_DTEMP;

  if(DCType.Value == 2) // If Cryo Cooler
    {
    DetTemp.Max = 20.0F;
    DetTemp.Min = -80.0F;
    }
  else
    {
    DetTemp.Max = 0.0F;
    DetTemp.Min = -120.0F;
    }

  ExpTime.Command = DC_ET;
  GetOMA(DC_MAXET, &ExpTime.Max);
  GetOMA(DC_MINET, &ExpTime.Min);

  if(ExpTime.Min < 0.000003F)
    ExpTime.Min = 0.000003F;
  if(ExpTime.Max < .000003F)
    ExpTime.Max = 0.01F;
  FParam = (float)(log10((double)ExpTime.Max) - log10((double)ExpTime.Min));
  if(FParam > 0)
    ETScaler = FParam / 65535.0F;
  else
    ETScaler = (float)log10((double)ExpTime.Max) / 65535.0F;
  Ignores.Command = DC_K;
  Ignores.Max = 32767;
  Mems.Command = DC_J;
  GetOMA(DC_MAXMEM, &Mems.Max);
  Mems.Min = 1;
  Points.Command = DC_POINTS;
  Points.Max = ActiveX.Value;
  Points.Min = 1;
  Preps.Command = DC_H;
  Preps.Max = 32767;
  Preps.Min = 0;
  Tracks.Command = DC_TRACKS;
  Tracks.Min = 1;
  TrackMode.Command = DC_TRKMODE;
  LimitTracks();
  Scans.Command = DC_I;
  Scans.Max = 32767;
  Scans.Min = 1;
  Shutter.Command = DC_SHUTMODE;
  Shutter.Max = 2;
  Shutter.Min = 0;
  Sreg.Command = DC_OUTREG;
  WFTO.Command = DC_WFTO;
  X0.Command = DC_X0;
  X0.Max = ActiveX.Value;
  X0.Min = 1;
  Y0.Command = DC_Y0;
  Y0.Max = ActiveY.Value;
  Y0.Min = 1;
}

float SbCountToExpTime(int SbCount)
{
  long ETCount;
  float Fval;

  ETCount = (SbCount + 32767L);
  Fval = ETCount * ETScaler;
  Fval = Fval + (float)log10((double)ExpTime.Min);
  Fval = (float)pow(10.0, (double)Fval);
  return Fval;
}

int SetOMA (SHORT Cmd, float Setting)
{
  if(DModel.Value > -1)
    return SetParam(Cmd, Setting);
  else
    return 1;
}


