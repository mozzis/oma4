/***************************************************************************/
/*  access4.H                                                              */
/*                                                                         */
/*  copyright (c) 1990, EG&G Instruments Inc.                              */
/*                                                                         */
/*  $Header: /OMA4000/Include/cntrloma.h 1     6/18/99 1:52a Maynard $                                                             */
/*  $Log: /OMA4000/Include/cntrloma.h $                                                             */
 * 
 * 1     6/18/99 1:52a Maynard
/***************************************************************************/

#ifndef ACCESS4_INCLUDED
#define ACCESS4_INCLUDED

#include "eggtype.h"

#define FC_PROC 0  /* processor codes - who's who on the OMA board */
#define DC_PROC 1
#define DAC_PROC 2

#define DA_INIT 0
#define DA_CLEAN 1
#define DA_INIT2 2
#define DA_LIVE  2


#define DCADD  0
#define DCREP  1
#define DCSUB  2
#define DCIGN  3
#define DCOH   4
#define DCOHNE 5
#define DCOHTP 6

#define OHLCINP 4
#define OHLCI 3
#define OHNOET 2
#define OHNOSHUT 1
#define OHNORM 0

#define IMG_SIMPLE   0
#define IMG_CLASSIC  1
#define IMG_RANDOM   2
#define SER_UNI_MXS  3
#define SER_UNI_DXS  4
#define SER_UNI_MXC  5
#define SER_UNI_DXC  6
#define SER_RAN_ALL  7
#define FC_NORM_OH   8    /* These 5 must be in order! */
#define FC_NOSHUT_OH 9    /*            .              */
#define FC_NOET_OH   10   /*            .              */
#define FC_LCI_OH    11   /*            .              */
#define FC_LCINP_OH  12   /*            .              */
#define FC_CLR_SER   13

#define DAC_STRK_NORM         14
#define DAC_STRK_OT           15
#define DAC_STRK_NORM_ET      16
#define DAC_STRK_OT_ET        17
#define DAC_STRK_NORM_SYNC    18
#define DAC_STRK_OT_SYNC      19
#define DAC_STRK_NORM_ET_SYNC 20
#define DAC_STRK_OT_ET_SYNC   21
#define DAC_STRK_PRETRIG_NORM 22
#define DAC_STRK_PRETRIG_ET   23
#define DAC_STRK_LCI          24 /* FC mode for streak LCI pulser */
#define DAC_STRK_LCI_CW       25 /* FC mode for streak LCI pulser in CW */

#define LC_TABLE_LIMIT 512            /* maximum # of counters & tags */

#ifndef __WATCOMC__

extern USHORT far FC_mode_list[]; /* should be made private */

#else

extern USHORT FC_mode_list[]; /* should be made private */

#endif

/* RAPDA structures for setting up the DAC and ASIC. */
/* Structure of the DAC table 1 for the delay-read sequences. */

struct Table1Entry
{
  USHORT FCAddr;
  USHORT DCAddr;
  ULONG FBAddr;
  USHORT Table2EntryAddr;
};

/* Structure of the DAC table 2 for the read-only sequences. */

struct Table2Entry
{
  USHORT FCAddr;
  USHORT DCAddr;
  ULONG FBAddr;
};

SHORT switch_da(SHORT, SHORT, SHORT, SHORT);
SHORT stop_OMA_DA_in_progress(void);
SHORT release_OMA_and_init(void);
void  get_OMA4_data(PCHAR pData, SHORT len, USHORT curvenum);
void  put_OMA4_data(PCHAR pData, SHORT len, USHORT curvenum);

USHORT get_asic_counter(USHORT);
void  set_asic_opbits(USHORT, USHORT);
void clr_asic_opbits(USHORT, USHORT);
BOOLEAN rep_asic_opbits(USHORT, USHORT);
void set_asic_pointer(USHORT, ULONG);
void scan_counter_table(void);
void update_from_shared_memory(void);
SHORT convert_code_to_index(USHORT);
SHORT  id_detector(void);
BOOLEAN InqHasFastShift(void);

USHORT get_max_damodes(void);
USHORT set_inex_shutter_status(SHORT);
USHORT set_shutter_forced_status(SHORT);
USHORT set_sync_open_status(SHORT);
USHORT set_sync_close_status(SHORT);
USHORT set_streakmode_status(SHORT, SHORT);
USHORT set_source_comp_status(SHORT);
USHORT set_pixtime_status(SHORT);
USHORT set_shifttime_status(SHORT);
USHORT set_shiftreg_status(SHORT);

USHORT calc_antibloom_counts(USHORT, USHORT, USHORT *, USHORT *);
void   set_DAC_pointer(SHORT, ULONG);
ULONG  get_DAC_pointer(SHORT);
ULONG  access_source_comp(SHORT);
void   wait_off(void);
void   wait_on(void);
void   update_random_point_table(SHORT *, SHORT *, USHORT, SHORT);
void   update_random_track_table(SHORT *, SHORT *, USHORT, SHORT);
SHORT  SetUpRapdaTables(void);
SHORT  SetShortDelay(float *);

ULONG  get_sharedcomm_size(void);
SHORT rserial_setup(SHORT);
SHORT rimage_setup(SHORT);
void   get_monitor_string(char *, SHORT);
void   erase_comm_area(void);

void set_holdoff_release(void);
void clr_holdoff_release(void);

#endif  /*  ACCESS4_INCLUDED */
