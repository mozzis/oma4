// UniMeth.h
// Universal method header structure for DATMGR DLL
// The method header includes settings for the detector which acquired the
// data in the file, and settings for the display program which last displayed
// the data.
// *** Where possible, a zero value shall indicate an uninitialized field ***

#ifndef UNIMETH_INCLUDED
#define UNIMETH_INCLUDED

#include "primtype.h"

#define TCL_TEXT_LEN 75
#define TCL_DESC_LEN 74
#define OMA_DET_MAX_DIM 1024
#define DEFAULT_NUM_GRATINGS 3
#define OMA4_FNAME_LEN 77
#define OMA4_DESC_LEN 81

// designators for axis units
// all units are SI units unless noted
                     
enum axis_units { AXUN_NONE, 
                  AXUN_WAVELEN,
                  AXUN_RAMAN,   // cm-1 Raman shift
                  AXUN_FREQ,
                  AXUN_TIME,
                  AXUN_RADIOMET, // Rodiometric intensity
                  AXUN_FLUX      // Photometric units
                 };
                                      
typedef struct axis_setup {
  char Legend[30];
  float Min;
  float Max;
  enum axis_units Units;
  BOOL AutoScale;
} AXIS_SETUP;

typedef struct displayflags {
  unsigned int wdf_live_fast : 1;
  unsigned int wdf_loop_colors : 1;
  unsigned int wdf_unused      : 6;
}  WINDISPFLAGS;

enum disp_style { wds_overlapped, 
                  wds_hidline,
                  wds_fcolor,
                  wds_contour };
                  
enum disp_zpos { wdz_nozaxis,
                 wdz_zaxis_right,
                 wdz_zaxis_left };
                  
typedef struct display_setup {
   char              Title[28];
   AXIS_SETUP        Xaxis;
   AXIS_SETUP        Yaxis;
   AXIS_SETUP        Zaxis;
   int               XZPercent;
   int               YZPercent;
   enum disp_zpos    ZPosition;
   enum disp_style   Style;
   WINDISPFLAGS      Flags;
} DISP_SETUP;
                                                 
// position 0 in pixel map arrays is for uniform setup value
// remaining positions are for random setup values
                                                 
typedef struct pixel_map {
   SHORT number;                 // number of units in pixel map
   SHORT Zero[OMA_DET_MAX_DIM];  // starting pixel of group
   SHORT Delta[OMA_DET_MAX_DIM]; // size of group
} PIXEL_MAP;

typedef struct rapda_map {
  SHORT MinETCount;
  PIXEL_MAP Regions;
  SHORT Offset[OMA_DET_MAX_DIM / 8];
} RAPDA_MAP;

typedef struct scan_setup {
// These values may be different from the 
// "curve header" values for tracks and points
  short Tracks;
  short Points;
  short Trkmode;
  short Pntmode;
  short Trknum;
  short Pntnum;
  short Antibloom;
  short Shfmode;
  short Shfreg;
  short Regions;
  PIXEL_MAP XMap;
  PIXEL_MAP YMap;
} SCAN_SETUP;

typedef struct grating_setup {
  enum axis_units CalUnits;
  float RelPixWidth;
  float grooves_mm;
  float blaze_wlen;
  float CenterChannel;
  float ExcitationWavelength;
  float CalCoeffs[4];
} GRATING_SETUP;

typedef struct spec_setup {
  short SpectroModel;
  short CurrGrating;
  short NumGratings;
  GRATING_SETUP Gratings[DEFAULT_NUM_GRATINGS];
} SPEC_SETUP;

typedef struct pulser_setup {
  short TypeIndex;
  float Width;
  short TCount;
  float Delay;
  float Delinc;
  short TSrcIndex;
  short TThrshindex;
  short IntMode;
  short BeepOnOff;
} PULSER_SETUP;

typedef struct shutter_setup {
  float ExposeTime;
  short WaitBefore;
  short WaitAfter;
  short ForceMode;
  short ExtIntControl;
} SHUTTER_SETUP;

// placed dtypes here while some logical layout issues are
// resolved. The real source is detdriv.h

enum oma_dtypes { DETYP_NONE = 0, 
                  DETYP_TSC_CCD,     /* 512 square single channel */
                  DETYP_TSM_CCD,     /* 512 square dual channel (split mode) */
                  DETYP_EEV_CCD,     /* 298 X 1152 or 256 X 1024 EEV CCD */
                  DETYP_RAPDA,       /* 1024 Reticon PDA */
                  DETYP_TSP_CCD,     /* 256 X 1024 Thomson single channel */
                  DETYP_TS_1KSQ};    /* 1024 X 1024 Thomson single channel */


typedef struct det_struct {
  enum oma_dtypes Type;
  short Coolertype;
  short ShutterType;
  short DumXLead;
  short DumXLast;
  short DumYLead;
  short DumYLast;
  short ActiveX;
  short ActiveY;
  short shift_regs;
  short output_nodes;
} DET_STRUCT;
                     
// dattype low nibble gives bytes in data word
// dattype high nibble high bit = 1 = unsigned
// dattype high nibble bit 0 = 1 = float
                     
enum dat_type {
    DTYPE_INVALID = -1,
    DTYPE_CHAR = 0x01,
    DTYPE_UCHAR = 0x81,
    DTYPE_SHORT = 0x02,
    DTYPE_USHORT = 0x82,
    DTYPE_LONG = 0x04,
    DTYPE_ULONG = 0x84,
    DTYPE_FLOAT = 0x14,
    DTYPE_DOUBLE = 0x18
};

typedef struct DA_setup {
  short DAC_pgm;
  short Mems;
  short Scans;
  short Ignores;
  short Preps;
  short StartMem;
  short MaxMem;
  enum dat_type DataType; // indicates A/D precision
  short SameET;   // BOOL, 0 = NORMAL keepclean, 1 = FAST keepclean
  short BackGrnd; // BOOL, 0 = none, 1 = DA does background sub
} DAC_SETUP;

typedef struct streaker_setup {
  short ExposeRows;
  short PreTrigRows;
  short StreakMode;
} STREAKER_SETUP;

typedef struct aux_setup {
  short Temperature;
  short CoolOnOff;
  short LineFreq;
  short SrcCompIndex;
  short PixTimeIndex;
  short ShiftTimeIndex;
  short TrigsOnOff;
} AUX_SETUP;

typedef struct interface_setup {
  short InterfaceType;
  LONG Membase;
  LONG Memsize;
  short PortAddr;
} INTERFACE_SETUP;

typedef struct hidris_specials {
  USHORT TCLFileFormat;
  USHORT TCLPixelFormat;
  CHAR TCLUserText1[TCL_TEXT_LEN / 3];
  CHAR TCLUserText2[TCL_TEXT_LEN / 3];
  CHAR TCLUserText3[TCL_TEXT_LEN / 3];
  CHAR TCLUserText4[152 - TCL_TEXT_LEN];
  CHAR TCLDescText[TCL_DESC_LEN];
  short TCLReserved1[27];
  short TCLReserved2[96 - 21];
} HIDRIS_SPECIALS;

typedef struct oma4_specials {
  float Oma4FloatGap[19];
  float Oma4YTInterval;
  float Oma4YTPreDelay;
  float Oma4DetsetupVersion;
  short Oma4ExtStartIndex;
  short Oma4ScanTimeout;
  short Oma4PIA[2];
  short Oma4SoftwareVersion;
  short Oma4PlotWindowIndex;
  short Oma4ActivePlotSetup;
  short Oma4Autoscale[3];
  short Oma4WinPlotMaps[8];
  BOOL Oma4LabelPeaks;
  char Oma4Reserved[100];
  char Oma4BackName[OMA4_FNAME_LEN];
  char Oma4DADName[OMA4_FNAME_LEN];
  char Oma4IOName[OMA4_FNAME_LEN];
  char Oma4InputName[OMA4_FNAME_LEN];
  char Oma4OutputName[OMA4_FNAME_LEN];
  char UserChar1;
} OMA4_SPECIALS;

typedef struct uni_method {
 short           VerLength;
 short           ActiveInterface;
 INTERFACE_SETUP InterfaceSetup;
 DET_STRUCT      DetStructSetup;
 DAC_SETUP       DACSetup;
 SCAN_SETUP      ScanSetup;
 SHUTTER_SETUP   ShutterSetup;
 PULSER_SETUP    PulserSetup;
 AUX_SETUP       AuxSetup;
 SPEC_SETUP      SpectroSetup;
 STREAKER_SETUP  StreakSetup;
 DISP_SETUP      DisplaySetup;
 HIDRIS_SPECIALS HidrisSpecials;
 OMA4_SPECIALS   OMA4Specials;
 char            UniMetReserved[128];
} UNI_METHOD;

#endif
