/***************************************************************************/
/*  access4.H                                                              */
/*                                                                         */
/*  copyright (c) 1990, EG&G Instruments Inc.                              */
/*
/  $Header:   J:/logfiles/oma4000/inc/access4.h_v   1.11   06 Jul 1992 11:48:28   maynard  $
/  $Log:   J:/logfiles/oma4000/inc/access4.h_v  $
 * 
 *    Rev 1.11   06 Jul 1992 11:48:28   maynard
 * Add constants to refer to LCI/Pulser overhead routines
 * delete refs to ERROR_CATEGORY
 * 
 *    Rev 1.10   13 Jan 1992 16:07:40   cole
 * Delete extern for OMA_memory_address, now PRIVATE in access4.c
 * 
 *    Rev 1.9   12 Jan 1992 09:47:50   cole
 * Remove // at start of get_sharedcomm_size() function prototype line.
 * 
 *    Rev 1.8   07 Jan 1992 12:30:04   maynard
 * Nomenclature change to help with classic mode - rpoint and rtrack become
 * rserial and rimage
 * 
/***************************************************************************/

#ifndef ACCESS4_INCLUDED
#define ACCESS4_INCLUDED

#ifndef PRIMTYPE_INCLUDED
   #include "primtype.h"
#endif

#define WRITE_ODD_BYTES 0
#define WRITE_EVEN_BYTES 1
#define READ_ODD_BYTES 2
#define READ_EVEN_BYTES 3

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
#define FC_NORM_OH   8    /* These 4 must be in order! */
#define FC_NOSHUT_OH 9
#define FC_NOET_OH   10
#define FC_LCI_OH    11
#define FC_LCINP_OH  12
#define FC_CLR_SER   13

extern unsigned FC_mode_list[]; /* should be made private */

extern ULONG OMA_phys_address;
extern ULONG OMA_data_address;
extern USHORT OMA_port_address;
extern ULONG OMA_memory_size;
extern ULONG OMA_DAC_address;
extern BOOLEAN fake_detector;

void     release_OMA_reset(void);
void     set_OMA_reset(void);
int      start_detector(int, int, int);
int      switch_da(int, int, int, int);
int      stop_OMA_DA_in_progress(void);
void     map_program_memory(void);
void     disable_OMA_memory(void);
int      release_OMA_and_init(void);
void     get_OMA4_data(PCHAR pData, int len, USHORT curvenum);
void     put_OMA4_data(PCHAR pData, int len, USHORT curvenum);
void     shutdown_detector_interface(void);
void     set_DAC_counter(unsigned, unsigned);
unsigned get_DAC_counter(unsigned type);
void     set_asic_opbits(unsigned, unsigned);
void     clr_asic_opbits(unsigned, unsigned);
void     rep_asic_opbits(unsigned, unsigned);
void     set_asic_pointer(unsigned, unsigned long);
unsigned get_asic_counter(unsigned);
void     scan_counter_table(void);
void     update_from_shared_memory(void);
int convert_code_to_index(unsigned int code);
int      id_detector(void);

unsigned int get_max_damodes(void);
unsigned int set_inex_shutter_status(int status);
unsigned int set_shutter_forced_status(int status);
unsigned int set_sync_open_status(int status);
unsigned int set_sync_close_status(int status);
unsigned int set_source_comp_status(int status);
unsigned int set_pixtime_status(int status);
unsigned int set_shiftreg_status(int status);

unsigned       calc_antibloom_counts(unsigned, unsigned, 
                                     unsigned *, unsigned *);
void           set_DAC_pointer(int type, unsigned long value);
unsigned long  get_DAC_pointer(int type);
unsigned long access_source_comp(int frame);
void wait_off(void);
void wait_on(void);
void update_random_point_table( int arrayX0[], int arrayDeltaX[],
                                unsigned count, int mode ) ;

void update_random_track_table( int arrayY0[], int arrayDeltaY[],
                                unsigned count, int mode ) ;

unsigned long get_sharedcomm_size(void);
BOOLEAN        find_or_install_monitor(void);
ERROR_CATEGORY rserial_setup(int);
ERROR_CATEGORY rimage_setup(int);
void           get_monitor_string(char * compare_buffer, int len);
void           erase_comm_area(void);
BOOLEAN init_local_data(void);
BOOLEAN access_init_detector(USHORT port, ULONG memsize );
void access_startup_detector(USHORT port, ULONG memaddr, ULONG memsize);

#endif  /*  ACCESS4_INCLUDED */

