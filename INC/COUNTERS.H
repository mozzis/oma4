/**********************************************************************/
/* FILE : counters.h                                                  */
/* define structure in which "counters" or "tags" are stored in the   */
/* monitor address space.  Tags are used to find values in the ASIC   */
/* code which can be modified according to the detector setup; or to  */
/* refer to counters and flags in the shared memory area of data mem- */
/* ory which are used to control DAC programs.                        */
/**********************************************************************/
#ifndef COUNTERS_INCLUDED
#define COUNTERS_INCLUDED

typedef struct {
  USHORT offset;      /* address in program memory of counter */
  USHORT processor;   /* 0 = FC space, 1 = DC space, 2 = DAC space */
  USHORT class;       /* user defined hook value */
  } counter_link_record;

/**********************************************************************/
/*                                                                    */
/* Define the values of the tags.  These values must be the same as   */
/* those in the file "hooks.inc", which is used by the OMA4 compiler. */
/* (The OMA4 compiler compiles ASIC and DAC programs)                 */
/*                                                                    */
/**********************************************************************/

#define I_TOTAL        1    /* Image area total. */
#define S_TOTAL        2    /* Serial area total. */
#define OH_OUT         3    /* Over Head outer loop. */
#define OH_IN          4    /* Over Head inter loop. */
#define I_LEAD_INNER   5    /* Image index, starting. */
#define I_LEAD_OUTER   6    /* Image index, starting.  */
#define I_TRAIL_INNER  7    /* Serial index, starting. */
#define I_TRAIL_OUTER  8    /* Serial index, ending. */
#define I_GROUP        9    /* Image length in groups. */
#define S_GROUP_MIN1   10   /* Serial length in groups, minus 1. */
#define Y_DELTA        11   /* Image point size. */
#define X_DELTA        12   /* Serial point size. */
#define Y_SKIP         13   /* Image skip by value. */
#define X_SKIP         14   /* Serial skip by value. */
#define S_IGNORE_START 15   /* Serial ignore, staring. */
#define S_IGNORE_END   16   /* Serial ignore, stopping. */
#define TRACKLEN       17   /* Number of words per track */

#define I0_COUNTER     18   /* Number of frames/mem, by convention */
#define I1_COUNTER     19
#define J0_COUNTER     20   /* Number of memories, by convention */
#define J1_COUNTER     21
#define K0_COUNTER     22   /* Number of ignored frames, by convention */
#define K1_COUNTER     23
#define H0_COUNTER     24   /* Number of prep frames, by convention */
#define H1_COUNTER     25
#define L0_COUNTER     26   /* Auxiliary counter; may be array of counters */
#define L1_COUNTER     27
#define T0_COUNTER     28   /* Number of tracks (spectra) for DAC */
#define T1_COUNTER     29
#define S0_COUNTER     30   /* Auxiliary slices (points) for DAC */
#define S1_COUNTER     31

#define TEMP_TAG       32   /* Temperature setting */
#define COOL_STATUS    33   /* Code to locate cooler status byte in MSM */
#define DET_ID         34
#define SHUTTER_BEF    35   /* Shutter state before expose */
#define SHUTTER_AFT    36   /* Shutter state after expose */

#define FRAME_PTR      37   /* Address for data frame */
#define BKGND_PTR      38   /* Address for background frame */
#define SIZE_PTR       39   /* Frame size */
#define SRCCMP_PTR     40   /* Source Comp location */

#define SCITC_SET      41   /* source comp integration time constant */
#define SYNC_OPEN      42   /* wait for sync to open shutter */
#define SYNC_CLOSE     43   /* wait for sync to close shutter */

#define UT_SIMPLE      44   /* FC Scan mode 1 - Uniform tracks*/
#define UT_CLASSIC     45   /* Uniform tracks, classic */
#define RT_ALL         46   /* FC Scan mode 2 - Random tracks*/
#define SER_ACQ        47   /* Where to patch in serial read routine address*/
#define SER_CLEAN      48   /* FC mode 5 - clean shift register */
#define FC_MODE_SC     49   /* FC code to read source comp */
                         
#define RSLICE_OUT     50   /* Random slice code, all cases */
#define USSLICE_OUT_MX 51   /* Simple uniform slice code, dX=1 */
#define USSLICE_OUT_DX 52   /* Simple uniform slice code, dX>1 */
#define UCSLICE_OUT_MX 53   /* Classic uniform slice code, dX=1*/
#define UCSLICE_OUT_DX 54   /* Classic uniform slice code, dX>1*/

#define FC_OH          55   /* Address of FC code for normal shutter/expose    */
#define FC_OHNS        56   /* Address of FC code for shutter forced/expose    */
#define FC_OHNOET      57   /* Address of FC code for no expose */
#define FC_OHLCI       58
#define FC_OHLCINP     59

#define DC_OH          60   /* Address of DC code for overhead with expose     */
#define DC_OHNE        61   /* Address of DC code for overhead without expose */
#define DC_OHLCI       62

#define DC_MODE_ADD    63   /* DC Mode for adds    */
#define DC_MODE_REP    64   /* DC Mode for replace */
#define DC_MODE_SUB    65   /* DC Mode for subtract */
#define DC_MODE_IGN    66   /* DC Mode for ignores */
#define DC_MODE_SC     67   /* DC Mode for ignores */

#define DAS_INIT       68   /* DAC Mode for ASIC sync */
#define DAS_CLEAN      69   /* DAC Mode for KEEPCLEAN */
#define DAS_0          70   /* DAC Mode for LIVE      */
#define DAS_1          71   /* DAC Mode for DA Seq #1 */
#define DAS_2          72   /* DAC Mode for DA Seq #2 */
#define DAS_USER       73   /* DAC Mode for user code */

#define SER_MODE       74
#define IMG_MODE       75

#define DA_DONE        76   /* Code to locate da done status in MSM */

#define BIAS_AB        80   /* Switch for shift to A or B register */
#define APROC_AB       81   /* Switch to use A or B output node */
#define PIXTIME_NP     82
#define INEX_SHUTTER   83   /* Internal or external drivesync for shutter */

#define CONFIG         100  /* Address of FC LDCFG command                     */
#define TCNT           101  /* Address of FC LDTCNT command                    */
#define PCNT           102  /* Address of FC LDPCNT command                    */
#define SCNT           103  /* Address of FC LDSCNT command                    */

#define SM_SGROUPS     104  /* Number of points for split mode serial read */
#define SEC_CUE        105  /* second cue of data point for spmode sc emu */

#define SORTQ          106  /* DAC Flag is true if data should be sorted */

#define DUMXLD         107  /* Dead pixels in serial register before data*/
#define DUMXTR         108  /* Dead pixels in serial register after data */
#define DUMYLD         109  /* Dead pixels in image area before data     */
#define DUMYTR         110  /* Dead pixels in image area after data      */
#define SACTIVE        111  /* Total live pixels in serial area*/
#define IACTIVE        112  /* Total live pixels in image area */
#define PIXTIME_AS     113  /* Adjust speed of EEV AS code */
#define SHUTID         114  /* shutter id code */
#define SAME_ET        115  /* 0 = do fast keepclean, 1 = normal */
#define PIA_OUTBITS    116  /* location of PIA input or output */
#define PIA_INBITS     117  /* location of PIA input or output */
#define CLOSE_TIME     118  /* counter to wait for shutter to close */
#define DA_SEQ         119  /* Address DAC DA prog num variable */

#define STREAK_MODE     120 /* DAC flag for streak mode */
#define STREAK_ET       121 /* Special ET counter for streaker mode */
#define STRK            122 /* FC mode for normal streak */
#define STRK_OT         123 /* FC mode for streak on trigger */
#define STRK_ET         124 /* FC mode for streak with ET */
#define STRK_OT_ET      125 /* FC mode for streak on trigger with ET */
#define STRK_SYNC       126 /* FC mode for streak with sync */
#define STRK_OT_SYNC    127 /* FC mode for streak on trigger with sync */
#define STRK_ET_SYNC    128 /* FC mode for streak with ET and sync */
#define STRK_OT_ET_SYNC 129 /* FC mode for streak on trigger with ET and sync */
#define STRK_PT         130 /* FC mode for streak with pre-trigger */
#define STRK_LCI        181 /* FC mode for streak LCI pulser */
#define STRK_LCI_CW     182 /* FC mode for streak LCI pulser in CW */
            
/* Hooks for LCI timer board */

#define LCI_TRIG_LEVEL 131
#define LCI_DELAY_ANA  132
#define LCI_DELAY_LSB  133
#define LCI_DELAY_MSB  134
#define LCI_DELTA_ANA  135
#define LCI_DELTA_LSB  136
#define LCI_DELTA_MSB  137
#define LCI_WIDTH_ANA  138
#define LCI_WIDTH_LSB  139
#define LCI_WIDTH_MSB  140
#define LCI_CNT_LSB    141
#define LCI_CNT_MSB    142
#define LCI_CMD        143
#define LCI_CONFIG     144

#define I_LEAD_REM     145 /* "Remainder" part of leading tracks  */
#define I_TRAIL_REM    146 /* "Remainder" part of trailing tracks */

#define LEAD_PIXELS    147
#define SHIFT_SPEED    148 /* Select Fast, Normal, or Slow shift line */
#define PT_TRACKS      149 /* Tracks for streak "pre-trigger" mode */
#define STRK_TRACKS    180 /* Tracks for streak - 1/2 normal if dual mode */

/*************************** RAPDA Tags ******************************/

#define DELAY_READ_0   150           /* The first delay read program */
#define OUTER_COUNT    151           /* the outer counter delay */
#define INNER_COUNT    152           /* the inner counter delay */
#define READ_OFFSET    153           /* the read part of delay read. */
#define DELAY_RD_COUNT 154           /* Number pixels to read, */
#define DELAY_RD_START 155           /* starting at this one. */
#define DELAY_READ_1   156           /* The second delay read program */

#define READ_0         157           /* The first read program */
#define READ_COUNT     158           /* Number pixels to read, */
#define READ_START     159           /* starting at this one. */
#define READ_1         160           /* The second read program */

#define DC_PGM_1       161           /* The first DC program */
#define DMA_COUNT      162           /* Number pixels to read. */
#define DC_PGM_2       163           /* The second DC program */
#define DC_ACTION      164           /* What the DC program does. */

#define DAS_RAPDA_TBL1 165           /* The delay_read program table */
#define DAS_RAPDA_TBL2 166           /* the read program table. */
#define MEMCNT         167           /* Number of memories. */

#endif  /*  COUNTERS_INCLUDED */

