/* -----------------------------------------------------------------------
/
/  detdriv.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/inc/detdriv.h_v   1.0   13 Aug 1991
15:43:08   maynard  $
/  $Log:   J:/logfiles/oma4000/inc/detdriv.h_v  $
 * 
 *    Rev 1.12   06 Jul 1992 11:48:56   maynard
 * add det_setup items and commands for LCI/Pulser, SAMEET, and PIA support
 * declare most data as far
 * move cooler status definitions here - they're officially part of the driver!
 * 
 *    Rev 1.11   12 Mar 1992 15:09:40   maynard
 * Declarations for external filenames added and changed
 * Constants for detector types changed to real OMA4 values and names
 * Added fields to det_info to define the guard pixels on the edges
 * of different detectors.
 * Add a bit to outputReg field to allow for 3 outputReg options.
 * Make room in pointstuff, trackstuff, and triggerstuff to hold info
 * for all the pixels in any array (defined by MAX_POINT_COUNT and
 * MAX_TRACK_COUNT)
 * Move bits_open and bits_close to access4
 * Add special function to get source comp value.
 * 
 * 
 *    Rev 1.10   14 Jan 1992 11:06:20   cole
 * Change reserved_3 in DET_SETUP structure from long to int.
 * 
 *    Rev 1.9   09 Jan 1992 08:56:24   cole
 * add StartMemory field to DET_SETUP structure.
 * 
 *    Rev 1.8   07 Jan 1992 14:14:12   cole
 * Change #include's. Change #ifndef from READSYS_INCLUDED to DETDRIV_INCLUDED.
 * Add typedef's for DET_SETUP, pointstuff, trackstuff, triggerstuff.
 * Add #define's and enum.
 * 
 *    Rev 1.7   07 Jan 1992 12:30:52   maynard
 * Added set_Mem routine
 * 
 *    Rev 1.6   11 Nov 1991 15:02:04   maynard
 * Change comments to 'C' type so TURBOC can compile the driver
 * Add ifdefs where necessary for TURBOC
 * 
 *    Rev 1.0   13 Aug 1991 15:43:08   maynard
 * Initial revision.
*/
#ifndef DETDRIV_INCLUDED     /* idempotent header */
#define DETDRIV_INCLUDED

#ifndef PRIMTYPE_INCLUDED
   #include "primtype.h"
#endif

#ifndef OMA4SCAN_INCLUDED
   #include "oma4scan.h"
#endif

extern char monitor_fname[] ;
extern char dacfile_fname[] ;
extern char ascfile_fname[] ;

#define NORMAL_SHUTTER 0 /* indexes for shutter_forced_mode */
#define FORCED_CLOSE 1
#define FORCED_OPEN 2

#define NO_CARD 0
/* 512 square single channel */
#define TSC_CCD 1
/* 512 square dual channel */
#define TSM_CCD 2
/* 298 X 1152 EEV CCD */
#define EEV_CCD 3
/* 1024 Reticon PDA */
#define RAPDA   4
/* 256 X 1024 Thomson single channel */
#define TSP_CCD 5

#define NO_PULSER  0
#define M1211_PULSER 1
#define M1302_PULSER 2
#define M1303_PULSER 3

#define M1462_FAST 1
#define M1462_SLOW 0

#define CCD_FAST 1
#define CCD_SLOW 0

enum { DISABLE = 0, INTERN, OPTIC, POS_SLOPE, NEG_SLOPE };

/* Indexes are convenient for toggle fields in the forms package, */
/* but they are a stupid way of actually storing the instrument   */
/* parameters since no one wants to remember what actual value    */
/* each index refers to in all the different fields               */
  
typedef struct {
   float       Version;                   /* Version of detector software */
   int         DA_mode;                   /* 1-19 or 99 legal */
   int         det_addr;                  /* 100 - F00 */
   int         data_word_size;            /* bytes/point, 0 is 2, 1 is 4  */
   int         scans;                     /* assume = value of I */
   int         memories;                  /* assume = value of J */
   int         ignored_scans;             /* assume = value of K */
   int         prep_frames;               /* assume = value of H */
   int         detector_temp;             /*  */
   int         cooler_locked;             /* boolean, 1 if it is. */
   int         detector_type_index;       /* see OMATYP.H for definitions */
   int         cooler_type_index;         /* meaning depends on detector */
   int         line_freq_index;           /* 50 or 60 (Hz) */
   int         source_comp_index;         /* External analog mode or SC mode */
   int         control_index;             /* Internal, External */
   int         external_start_index;      /* 0 if internal, 1 ext. */

   unsigned long memory_base;             /* base address for the board */
   unsigned long memory_size;             /* size of memory on the board */

   int         shutter_open_sync_index;   /* CCD only flags, */
   int         shutter_close_sync_index;  /* TRUE = wait for sync */
   int         shutter_forced_mode;       /* CCD only, 0 norm 1 closed 2 open */
   int         need_expose_index;         /* TRUE (default) if need expose */
   float       exposure_time;             /* Current expose time in secs */
   int         min_exposure_time;         /* Units of NOPS */
   int         anti_bloom_percent ;       /* 0 to 100 percent */
   int         pix_time_index;            /* 0 = 18us, 1 = 10us, what else??? */
   int         pulser_audio_index;        /* 0 = off, 1 = on */
   int         pulser_index;              /* */
   float       pulser_delay;              /* */
   float       pulser_width;              /* */
   int         pulser_trigsrc_index;      /* */
   int         pulser_trigger_count;      /* */
   int         pulser_intensifier_mode;   /* */
   USHORT      pulser_trig_threshold;
   FLOAT       pulser_delay_inc;          /* Auto inc to delay if supported */
   FLOAT       pulser_scan_timeout;       /* Auto inc to width if supported */
   int         tracks;                    /* tracks per memory */
   int         points;                    /* points per track */
   int         DumXLead;                  /* Dead pixels in serial register before data*/
   int         DumXTrail;                 /* Dead pixels in serial register after data */
   int         ActiveX;                   /* Array size, #channels X axis */
   int         ActiveY;                   /* Array size, #channels Y axis */
   int         DumYLead;                  /* Dead pixels in image area before data     */
   int         DumYTrail;                 /* Dead pixels in image area after data      */
   int         scan_timeout_index;
   int         StartMemory;               /* Result of MEM command; */
   int         Current_Point;             /* Slice # being set up */
   int         Current_Track;             /* Track number being set up */
   int         max_memory;                /* highest legal memory number */
   int         pointmode;                 /* 0 = uniform, 1 = non-uniform */
   int         trackmode;                 /* 0 = contiguous, 1 = random */
   unsigned int shiftmode : 2 ;           /* 2 bits : 0 = CCD, 1 = diode array, */
                                          /* 2 = streak camera */
   unsigned int outputReg : 2 ;           /* 1 bit : 0 = A, 1 = B */
   unsigned int same_et   : 1 ;           /* 1 bit : 0 = fast, 1 = normal */
   int         da_active;                 /* 1 if it is */
   int         background_active;         /* 1 if it is; app worries about */
                                          /* where it is located */
} DET_SETUP;

typedef struct {
   int number;
   int X0[MAX_POINT_COUNT];
   int DeltaX[MAX_POINT_COUNT];
} pointstuff;
  
typedef struct {
   int number;
   int Y0[MAX_TRACK_COUNT];
   int DeltaY[MAX_TRACK_COUNT];
} trackstuff;
  
typedef struct {                 /* for future use */
   int number;
   int StartPixel[MAX_POINT_COUNT];
   int PixelLength[MAX_POINT_COUNT];
} triggerstuff;
  
typedef enum { UNLOCKED, LOCKED, FAKE } CoolerLockedStatus ;

typedef enum { COOL_OK, HEAT_EXCHANGE, TOO_HIGH, DEWAR_EMPTY, DIFF_EXCEEDED,
               UNKNOWN_COOLER } CoolerErrorStatus ;

enum { NumDetSetups = 8 } ;  // the number of detector setups in det_setups[]

extern DET_SETUP far   det_setups[] ;
extern DET_SETUP far * det_setup ;
extern pointstuff   far POINT_SETUP ;
extern trackstuff   far TRACK_SETUP ;
extern triggerstuff far  TRIGGER_SETUP ;

/* command tables for detdriv, writesys, and macro language interfaces */

/* enum array allows command string or routine lookup */
/* last entry must be NUM_INT_CMDS! */

enum int_command { ACTIVE = 1, ACTIVEX,    ACTIVEY,    ADD,        ADPREC,    
                   ANTIBLOOM,  BYTES,      CLR,        COOLSTAT,   COOLLOCK,  
                   COOLONOFF,  COOLTYPE,   CONTROL,    DAPROG,     DELTAX,     DELTAY, 
                   DETPORT,    DMODEL,     DTEMP,      DERROR,     FRAME,
                   FREQ,       H,          HIMEM,      I,          ID,
                   IMODE, 
                   INPIA,      J,          K,          MAXMEM,     MEM,
                   MASK,       OUTPUTREG,  OUTREG,     OUTPIA,
                   PAUDIO,     
                   PLSR,       PNTMODE,    POINT,      POINTS,     PTRIG,
                   PTRIGNUM,   PTRIGSRC,   PTRIGTRSH,  RUN,        SAMEET,
                   SCITC,      SHFTMODE,   SHUTMODE,   SHUTTER,    SPEED,      
                   STOP,
                   SUBT,
                   THERE,      TRACK,      TRACKS,     TRIGS,      TRKMODE,
                   WFTC,       WFTO,       X0,         Y0,         
                   NUM_INT_CMDS };

/* last entry must be NUMBER_INT_CMDS! */

enum flt_command { ET = NUM_INT_CMDS, FTIME, MAXET, MINET, PDELAY, PDELINC, 
                        PDELRANGE, PWIDTH, SCMP, VER, NUM_FLT_CMDS };

/* last entry must be NUMBER_FLT_CMDS! */

/* #define ERROR_NONE 0 */

typedef union
    {
    int intval;
    float fltval;
    } rvalue;

/* the command table will be an array of these structs (def in detdriv) */

typedef struct
  {
  enum int_command cmd_tag;
  int (*set_routine)(int);
  int (*get_routine)(SHORT *);
  } int_cmd_entry;

typedef struct
  {
  enum flt_command cmd_tag;
  int (*set_routine)(float);
  int (*get_routine)(float *);
  } flt_cmd_entry;

typedef int (*pfsint)(SHORT);   /* ptr to func setting an int value  */
typedef int (*pfgint)(SHORT *); /* ptr to func getting an int value  */
typedef int (*pfsflt)(float);   /* ptr to func setting an flt value  */
typedef int (*pfgflt)(float *); /* ptr to func getting an flt value  */

/************************************************************************/

int ReadSys( DET_SETUP *, unsigned );
int GetParam(int, float *);
int SetParam(int, float);
int set_int_detect_param( enum int_command, SHORT );
int set_flt_detect_param( enum flt_command, float );
int get_int_detect_param( SHORT, int *);
int get_flt_detect_param( SHORT, float *);

int ReadCurveFromMem(PCHAR pData, int len, USHORT curvenum);
int WriteCurveToMem(PCHAR pData, int len, USHORT curvenum);

int get_ActiveX( SHORT *) ;
int get_ActiveY( SHORT *) ;
int get_AntiBloom(SHORT *) ;
int get_Bytes(SHORT *);
int get_ControlMode(SHORT *);
int get_CoolOnOff(SHORT *);
int get_CoolLocked(SHORT *);
int get_CoolStatus(SHORT *);
int get_CoolerType(SHORT *);
int get_CurrentPoint(SHORT *);
int get_CurrentTrack(SHORT *);
int get_DaProg(SHORT *);
int get_DaActive(SHORT *);
int get_DeltaX(SHORT *);
int get_DeltaY(SHORT *);
int get_DetPort(SHORT *);
int get_DetectorType(SHORT *);
int get_ExposeTime(float *);
int get_Error(SHORT *);
int get_Frame(SHORT * frame);
int get_FrameTime(float * retval);
int get_RealDetector(SHORT *);
int get_Freq(SHORT *);
int get_HiMem(SHORT *);
int get_Ignores(SHORT *);
int get_Ident(SHORT *);
int get_IntensifierMode(SHORT * );
int get_MainTrigger(SHORT *);
int get_MaxET(float *);
int get_MaxMem(SHORT *);
int get_Mem(SHORT *);
int get_Mems(SHORT *);
int get_MinET(float *);
int get_nothing(SHORT *);
int get_OutputReg(SHORT *) ;
int get_PIA_In(SHORT *) ;
int get_PIA_Out(SHORT *) ;
int get_PixTime(SHORT *) ;
int get_PointMode(SHORT *);
int get_Points(SHORT *);
int get_PointSize(SHORT *);
int get_Preps(SHORT *);

int get_PulserAudio(SHORT * );
int get_PulserDelay(float *);
int get_PulserDelayInc(float * );
int get_PulserDelayRange( float * );
int get_PulserTrigCount(SHORT * );
int get_PulserTrigSrc(SHORT * );
int get_PulserTrigThresh(SHORT * );
int get_PulserType(SHORT *);
int get_PulserWidth(float *);

int get_SameET(SHORT *);
int get_Scans(SHORT *);
int get_ShiftMode(SHORT *);
int get_ShutterCloseSync(SHORT *);
int get_ShutterMode(SHORT *);
int get_ShutterOpenSync(SHORT *);
int get_SourceComp(float *);
int get_SourceCompMode(SHORT *);
int get_Temp(SHORT *);
int get_TrigThreshVolts(float *);
int get_TrackMode(SHORT *);
int get_Tracks(SHORT *);
int get_Version(float *);
int get_X0(SHORT *);
int get_Y0(SHORT *);

int get_OMA_memory_size(unsigned long *);
int get_source_comp_point(long *, int );
float calc_rpix_time(void);

int set_ActiveX( SHORT ) ;
int set_ActiveY( SHORT ) ;
int set_AntiBloom( SHORT percent ) ;
int set_ClearMem(SHORT);
int set_ControlMode(SHORT);
int set_CoolOnOff( SHORT coolerOn ) ;
int set_CurrentPoint(SHORT);
int set_CurrentTrack(SHORT);
int set_DaProg(SHORT);
int set_DeltaX(SHORT);
int set_DeltaY(SHORT);
int set_DetPort(SHORT);
int set_DetectorType(SHORT);
int set_ExposeTime(float);
int set_RealDetector(SHORT);
int set_Freq(SHORT);
int set_Ignores(SHORT);
int set_IntensifierMode(SHORT);
int set_MainTrigger(SHORT);
int set_Mem(SHORT);
int set_Mems(SHORT);
int set_nothing(SHORT) ;
int set_flt_nothing(float) ;
int set_OutputReg( SHORT A_B ) ;
int set_PIA_Out(SHORT);
int set_PixTime(SHORT);
int set_PointMode(SHORT);
int set_Points(SHORT);
int set_PointSize(SHORT);
int set_Preps(SHORT);
int set_ProgTrigger(SHORT);
int set_PulserAudio(SHORT);
int set_PulserDelay(float);
int set_PulserDelayInc(float);
int set_PulserDelayThresh(float);
int set_PulserDelayRange(float);
int set_PulserEnable(SHORT);
int set_PulserType(SHORT);
int set_PulserTrigCount(SHORT);
int set_PulserTrigCount(SHORT);
int set_PulserTrigSrc(SHORT);
int set_PulserTrigThresh(SHORT);
int set_TrigThreshVolts(float);
int set_PulserWidth(float);
int set_SameET(SHORT);
int set_Scans(SHORT);
int set_ShiftMode(SHORT) ;
int set_ShutterCloseSync(SHORT);
int set_ShutterMode(SHORT);
int set_ShutterOpenSync(SHORT);
int set_SourceCompMode(SHORT);
int set_Temp(SHORT);
int set_TrackMode(SHORT);
int set_Tracks(SHORT);
int set_X0(SHORT);
int set_Y0(SHORT);

int setup_detector_interface(USHORT port_addr, ULONG memaddr, ULONG memsize);
int init_detector(USHORT port_addr);
void shutdown_detector_interface(void);

int start_OMA_DA(int DoLive);
int stop_OMA_DA(int);

int WriteCurveToMem(PCHAR pData, int len, USHORT curvenum);
int WriteSys( void ) ;
int X_scan_setup(void);
int Y_scan_setup(void);

#endif   /* DETDRIV_INCLUDED */
