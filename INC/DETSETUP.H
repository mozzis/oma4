/************************************************************************/
/* detsetup.h                                                           */
/* extra driver info and functions for the advanced user (e.g. OMA4000) */
/************************************************************************/

#ifndef DETSETUP_INCLUDED     /* idempotent header */
#define DETSETUP_INCLUDED

#include "eggtype.h"
#include "oma4scan.h"

extern const FLOAT RequiredMonitorVersion;
extern SHORT detector_index;

/* SDK only supports 1 detector */
#ifdef USE_D16M
#define MAX_DETECTS 8
#else
#define MAX_DETECTS 1
#endif

#define CCD_FAST 1
#define CCD_SLOW 0

/* Indexes are convenient for toggle fields in the forms package, */
/* but they are a stupid way of actually storing the instrument   */
/* parameters since no one wants to remember what actual value    */
/* each index refers to in all the different fields               */
  
typedef struct {
   float Version;                   /* Version of detector software */
   SHORT DA_mode;                   /* 1-19 or 99 legal */
   SHORT det_addr;                  /* 100 - F00 */
   SHORT data_word_size;            /* bytes/point, 0 is 2, 1 is 4  */
   SHORT scans;                     /* assume = value of I */
   SHORT memories;                  /* assume = value of J */
   SHORT ignored_scans;             /* assume = value of K */
   SHORT prep_frames;               /* assume = value of H */
   SHORT detector_temp;             /*  */
   SHORT cooler_locked;             /* boolean, 1 if it is. */
   SHORT detector_type_index;       /* see OMATYP.H ????? for definitions */
   SHORT cooler_type_index;         /* meaning depends on detector */
   SHORT line_freq_index;           /* 50 or 60 (Hz) */
   SHORT source_comp_index;         /* External analog mode or SC mode */
   SHORT control_index;             /* Internal, External */
   SHORT external_start_index;      /* 0 if internal, 1 ext. */

   ULONG memory_base;               /* base address for the board */
   ULONG memory_size;               /* size of memory on the board */

   SHORT  shutter_open_sync_index;  /* CCD only flags, */
   SHORT  shutter_close_sync_index; /* TRUE = wait for sync */
   SHORT  shutter_forced_mode;      /* CCD only, 0 norm 1 closed 2 open */
   SHORT  exposed_rows;             /* number of rows exposed if streaker */
   float  exposure_time;            /* Current expose time in secs */
   SHORT  min_exposure_time;        /* */
   SHORT  anti_bloom_percent;       /* 0 to 100 percent */
   BSHORT pix_time_index : 2;       /* 0 = 18us, 1 = 10us, what else??? */
   BSHORT shift_time_index : 2;     /* 0 = 18us, 1 = 10us, what else??? */
   SHORT  pulser_audio_index;       /* 0 = off, 1 = on */
   SHORT  pulser_index;             /* which pulser is present, 0 = none */
   float  pulser_delay;             /* in seconds */
   float  pulser_width;             /*    ""      */
   SHORT  pulser_trigsrc_index;     /* Diable = 0, Internal, Optical, +sl. -sl */
   USHORT pulser_trigger_count;     /* */
   SHORT  pulser_intensifier_mode;  /* CW or Gated */
   USHORT pulser_trig_threshold;    /* */
   FLOAT  pulser_delay_inc;         /* Auto inc to delay if supported */
   FLOAT  pulser_scan_timeout;      /* Not Used */
   SHORT  tracks;                   /* tracks per memory */
   SHORT  points;                   /* points per track */
   SHORT  DumXLead;                 /* Dead pixels in serial register before data*/
   SHORT  DumXTrail;                /* Dead pixels in serial register after data */
   SHORT  ActiveX;                  /* Array size, #channels X axis */
   SHORT  ActiveY;                  /* Array size, #channels Y axis */
   SHORT  DumYLead;                 /* Dead pixels in image area before data     */
   SHORT  DumYTrail;                /* Dead pixels in image area after data      */
   SHORT  regions;                  /* Regions per track for RAPDA. */
   SHORT  StartMemory;              /* Result of MEM command; */
   SHORT  Current_Point;            /* Slice # being set up */
   SHORT  Current_Track;            /* Track number being set up */
   SHORT  max_memory;               /* highest legal memory number */
   SHORT  pointmode;                /* 0 = uniform, 1 = non-uniform */
   SHORT  trackmode;                /* 0 = contiguous, 1 = random */
   BSHORT shiftmode  : 2;           /* 2 bits : 0 = CCD, 1 = diode array, */
   BSHORT outputReg  : 2;           /* 2 bits : 0 = A, 1 = B, 2 = Dual*/
   BSHORT same_et    : 1;           /* 1 bit  : 0 = fast, 1 = normal */
   BSHORT trigson    : 1;           /* 1 bit  : 0 = off, 1 = on */
   BSHORT streakmode : 2;           /* 2 bits : 0 = off, 1 = on 2 = Dual */
   SHORT  da_active;                /* 1 if it is */
   SHORT  background_active;        /* 1 if it is; app worries about */
                                    /* where it is located */
} DET_SETUP;

typedef struct {
   SHORT number;
   SHORT X0[MAX_POINT_COUNT];
   SHORT DeltaX[MAX_POINT_COUNT];
} pointstuff;
  
typedef struct {
   SHORT number;
   SHORT Y0[MAX_TRACK_COUNT];
   SHORT DeltaY[MAX_TRACK_COUNT];
} trackstuff;
  
typedef struct {                 /* for future use */
   SHORT number;
   SHORT StartPixel[MAX_POINT_COUNT];
   SHORT PixelLength[MAX_POINT_COUNT];
} trigstuff;

   /* Holds the info for the RAPDA scan setup in an array. */ 

#define MIN_PXLS      8       /* Minimum number of pixels in an region. */
#define MIN_PXL       1       /* Minimum pixel to be scanned. Pixel 0
                               * is the "rest" pixel; the pixel the address
                               * counters are left on at the end of scan. */
#define MAX_PXL       1023
#define MAX_N         16
#define A_D_TIME      3.2E-6F
#define PROG_OVERHD     2     /* Extra instructions need for each program. */
#define SC_OVERHD       16    /* Instructions needed for source comp. */
#define SHORT_ET_OVERHD 32    /* NOP's added at end of scan for looping... */

  /* Functions that return an error code return one of the following:
   *  0 - no error
   *  2 - region overlap
   *  3 - fewer than MIN_PXLS in region
   *  4 - region falls outside array.
   *  6 - Pixel not part of any region.
   *  7 - Short.ET too short
   *  8 - ET not Short.ET * 2ü
   *  9 - (Scan.ET / Short.ET)  > 65536  (2^16)
   * 10 - Array had more than (1024 / MIN_PXLS) regions.
   */

struct RegionType
{
  char   n;                           /* Exp time = SHORT.ET * 2ü */
  SHORT StartPixel;                     /* First pixel in region. */
  SHORT Number;                         /* Number of pixels in region. */
  SHORT DataOffset;                     /* Offset to region in data points. */
};

typedef struct {                 /* for rapda scans */
   SHORT NumRegs;                       /* The number of regions. */
                                      /* Information about each region. */
   struct RegionType RapdaReg [1024/MIN_PXLS];
} rapdastuff;

/* End of RAPDA section. */

enum { NumDetSetups = 8 };  /* the number of detector setups in det_setups[]*/

#ifndef __WATCOMC__

extern DET_SETUP  far   det_setups[];
extern DET_SETUP  far * det_setup;
extern pointstuff far POINT_SETUP;
extern trackstuff far TRACK_SETUP;
extern trigstuff  far TRIGGER_SETUP;
extern rapdastuff far RAPDA_SETUP;

extern char far monitor_fname[];
extern char far dacfile_fname[];
extern char far ascfile_fname[];

#else

extern DET_SETUP  det_setups[];
extern DET_SETUP  * det_setup;
extern pointstuff POINT_SETUP;
extern trackstuff TRACK_SETUP;
extern trigstuff  TRIGGER_SETUP;
extern rapdastuff RAPDA_SETUP;

extern char monitor_fname[];
extern char dacfile_fname[];
extern char ascfile_fname[];

#endif

SHORT get_ActiveX(SHORT *);
SHORT get_ActiveY(SHORT *);
SHORT get_AntiBloom(SHORT *);
SHORT get_Bytes(SHORT *);
SHORT get_ControlMode(SHORT *);
SHORT get_CoolOnOff(SHORT *);
SHORT get_CoolLocked(SHORT *);
SHORT get_CoolStatus(SHORT *);
SHORT get_CoolerType(SHORT *);
SHORT get_CurrentPoint(SHORT *);
SHORT get_CurrentTrack(SHORT *);
SHORT get_DaMaxProg(SHORT *);
SHORT get_DaProg(SHORT *);
SHORT get_DaActive(SHORT *);
SHORT get_DeltaX(SHORT *);
SHORT get_RegSize(SHORT *);
SHORT get_DeltaY(SHORT *);
SHORT get_DetPort(SHORT *);
SHORT get_DetectorType(SHORT *);
SHORT get_ExposeTime(float *);
SHORT get_ExposedRows(SHORT *);
SHORT get_Error(SHORT *);
SHORT get_Frame(SHORT * frame);
SHORT get_FrameTime(float * retval);
SHORT sort_regions (SHORT * retval);
SHORT get_Freq(SHORT *);
SHORT get_HiMem(SHORT *);
SHORT get_Ignores(SHORT *);
SHORT get_Lastscan(SHORT *);
SHORT get_Ident(SHORT *);
SHORT get_IntensifierMode(SHORT *);
SHORT get_MainTrigger(SHORT *);
SHORT get_MaxET(float *);
SHORT get_MaxMem(SHORT *);
SHORT get_Mem(SHORT *);
SHORT get_Mems(SHORT *);
SHORT get_MinET(float *);
SHORT get_nothing(SHORT *);
SHORT get_OutputReg(SHORT *);
SHORT get_PIA_In(SHORT *);
SHORT get_PIA_Out(SHORT *);
SHORT get_PixTime(SHORT *);
SHORT get_PointMode(SHORT *);
SHORT get_Points(SHORT *);
SHORT get_RealDetector(SHORT *);
SHORT get_RegionET(float *);
SHORT get_Regions(SHORT * retval);
SHORT get_PointSize(SHORT *);
SHORT get_Preps(SHORT *);
SHORT get_Prescan(SHORT *);
SHORT get_PrescanTime(float * retval);

SHORT get_PulserAudio(SHORT *);
SHORT get_PulserDelay(float *);
SHORT get_PulserDelayInc(float *);
SHORT get_PulserDelayRange(float *);
SHORT get_PulserTrigCount(SHORT *);
SHORT get_PulserTrigSrc(SHORT *);
SHORT get_PulserTrigThresh(SHORT *);
SHORT get_PulserType(SHORT *);
SHORT get_PulserWidth(float *);

SHORT get_SameET(SHORT *);
SHORT get_Scans(SHORT *);
SHORT get_ShiftMode(SHORT *);
SHORT get_ShiftTime(SHORT *);
SHORT get_ShutterCloseSync(SHORT *);
SHORT get_ShutterMode(SHORT *);
SHORT get_ShutterOpenSync(SHORT *);
SHORT get_SourceComp(float *);
SHORT get_SourceCompMode(SHORT *);
SHORT get_StreakMode(SHORT *);
SHORT get_ShiftTime(SHORT *);
SHORT get_StreakTime(float * retval);
SHORT get_Temp(SHORT *);
SHORT get_TrigThreshVolts(float *);
SHORT get_TrackMode(SHORT *);
SHORT get_Tracks(SHORT *);
SHORT get_Version(float *);
SHORT get_X0(SHORT *);
SHORT get_Y0(SHORT *);

SHORT get_OMA_memory_size(ULONG *);
SHORT get_source_comp_point(LONG *, SHORT);
float calc_rpix_time(void);
float shift_line_time(void);

SHORT set_ActiveX(SHORT);
SHORT set_ActiveY(SHORT);
SHORT set_AntiBloom(SHORT percent);
SHORT set_ClearMem(SHORT);
SHORT set_ControlMode(SHORT);
SHORT set_CoolOnOff(SHORT coolerOn);
SHORT set_CurrentPoint(SHORT);
SHORT set_CurrentTrack(SHORT);
SHORT set_DaActive(SHORT Active);
SHORT set_DaProg(SHORT);
SHORT set_DeltaX(SHORT);
SHORT set_RegSize(SHORT);
SHORT set_DeltaY(SHORT);
SHORT set_DetPort(SHORT);
SHORT set_DetectorType(SHORT);
SHORT set_ExposeTime(float);
SHORT set_ExposedRows(SHORT);
SHORT set_RealDetector(SHORT);
SHORT set_Freq(SHORT);
SHORT set_Ignores(SHORT);
SHORT set_Lastscan(SHORT);
SHORT set_IntensifierMode(SHORT);
SHORT set_MainTrigger(SHORT);
SHORT set_Mem(SHORT);
SHORT set_Mems(SHORT);
SHORT set_nothing(SHORT);
SHORT set_flt_nothing(float);
SHORT set_OutputReg(SHORT A_B);
SHORT set_PIA_Out(SHORT);
SHORT set_PixTime(SHORT);
SHORT set_PointMode(SHORT);
SHORT set_Points(SHORT);
SHORT set_Regions(SHORT regions);
SHORT set_PointSize(SHORT);
SHORT set_Preps(SHORT);
SHORT set_Prescan(SHORT);
SHORT set_ProgTrigger(SHORT);
SHORT set_PulserAudio(SHORT);
SHORT set_PulserDelay(float);
SHORT set_PulserDelayInc(float);
SHORT set_PulserDelayThresh(float);
SHORT set_PulserDelayRange(float);
SHORT set_PulserEnable(SHORT);
SHORT set_PulserType(SHORT);
SHORT set_PulserTrigCount(SHORT);
SHORT set_PulserTrigCount(SHORT);
SHORT set_PulserTrigSrc(SHORT);
SHORT set_PulserTrigThresh(SHORT);
SHORT set_PulserWidth(float);
SHORT set_RegionET(float);
SHORT set_SameET(SHORT);
SHORT set_Scans(SHORT);
SHORT set_ShiftMode(SHORT);
SHORT set_ShiftTime(SHORT);
SHORT set_ShutterCloseSync(SHORT);
SHORT set_ShutterMode(SHORT);
SHORT set_ShutterOpenSync(SHORT);
SHORT set_SourceCompMode(SHORT);
SHORT set_StreakMode(SHORT);
SHORT set_Temp(SHORT);
SHORT set_TrackMode(SHORT);
SHORT set_Tracks(SHORT);
SHORT set_TrigThreshVolts(float);
SHORT set_X0(SHORT);
SHORT set_Y0(SHORT);

/* some private functions for "cheater" apps */
SHORT X_scan_setup(void);
SHORT Y_scan_setup(void);
SHORT InitStartDetector(void);
SHORT ReadSys(DET_SETUP *);        /* not needed */
SHORT WriteSys(void);
SHORT ClearMems(SHORT Start, SHORT Count);
SHORT set_ClearMem(SHORT mem);
SHORT ClearRunMems(void);
SHORT ClearAllMems(void);

float RapdaDataPtToPixel (SHORT DataPt);
SHORT CheckRegionOverlap (void);

#endif
