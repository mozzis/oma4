
     /********************************************************/
     /*                                                      */
     /*  monitor.h contains data structures used by the      */
     /*  oma4 monitor program                                */
     /*                                                      */
     /*                                                      */
     /********************************************************/

#ifndef MONITOR_INCLUDED
#define MONITOR_INCLUDED

#include "oma4scan.h"  /* MAX_POINT_COUNT */

#define PROG_MEM_SIZE             32767
#define SIZEOF_ASIC_PROGRAM_SPACE 65536
#define SIZEOF_68K_PROGRAM_SPACE  65536
#define RESERVED_OF_SHARED_AREA   64

#define INVALID_ADDRESS           0xFFFF

#define DMA_INITIAL_ADDRESS       0
#define FC_INITIAL_ADDRESS        0

#define OFFSET_TO_CODE_PTR_TABLE  0x700

typedef struct monitor_pointers
{
  char   lomem[OFFSET_TO_CODE_PTR_TABLE]; /* Skips memory up to table */
  USHORT fc_table_ptr;                /* Points to beginning of FCTBL */
  USHORT fc_table_index;              /* Index to next free FCTBL entry */
  USHORT fc_code_ptr;                 /* Points to next free FCCOD space */

  USHORT dma_table_ptr;               /* Points to beginning of DCTBL */
  USHORT dma_table_index;             /* Index to next free DCTBL entry */
  USHORT dma_code_ptr;                /* Points to next free DCCOD space */
                                               
  USHORT dac_table_ptr;               /* Points to beginning of DACTBL */
  USHORT dac_table_index;             /* Index to next free DACTBL entry */
  USHORT dac_code_ptr;                /* Points to next free DACCOD space */

  USHORT loopcntr_table_ptr;          /* Points to beginning of LCTBL */
  USHORT loopcntr_table_index;        /* Index into LC table */

  ULONG data_addr_ptr;              /* Points to start of data mem. */
  ULONG bgnd_addr_ptr;              /* Points to start of bgnd mem. */

  char version_string[26];

} MONITOR_PTRS;


/* Codes received from driver when signalling monitor */

#define SM_CLEAR       0
#define SM_START_DA    1
#define SM_SWITCH_DA   2
#define SM_ABORT_DA    3

/* Codes sent by monitor when signalling driver */

#define MSPC_CLEAR     0                      /* No status */
#define MSPC_IN_DN     1                      /* Init Complete */
#define MSPC_DA_DN     2                      /* Da Complete */
#define MSPC_RESVD     3 

/* Codes sent by monitor when replying to driver signal */

#define SMR_CLEAR      0
#define SMR_CMD_DN     1                      /* Command done */
#define SMR_CMD_BG     2                      /* Command started */
#define SMR_CMD_UN     3                      /* Command unknown */
#define SMR_NRDY       4                      /* Not ready for command */
#define SMR_EXCEP      5                      /* Exception error */
#define SMR_NO_DA      6                      /* No DA in progress */

/* Codes received from driver when replying to monitor signal */

#define MSPCR_CLEAR    0
#define MSPCR_ACK      1                       /*   Message received */

/* The shared memory construct for DAC - DRIVER communications */

/*           OFFSET  SHARED_MEM_START */

typedef struct 
  {
  USHORT MSM_Act_RQ;       /* ( 0) Monitor action requested by driver */
  USHORT MSM_Act_RQ_RPL;   /* ( 2) Monitor reply to driver request */
  USHORT MSM_Sig_PC;       /* ( 4) Driver action requested by monitor */
  USHORT MSM_Sig_PC_RPL;   /* ( 6) Driver reply to monitor request */
  USHORT MSM_Pgm_Inx;      /* ( 8) Which DAC program to run */
  ULONG  MSM_Data_Offset;  /* (10) Offset of frame data (assumes from 800000) */
  ULONG  MSM_Bgnd_Offset;  /* (14) Offset of bkgnd data (assumes from 800000) */
  ULONG  MSM_Data_Size;    /* (18) Size of memory allocated for experiment */
  USHORT MSM_FC_OH;        /* (22) Which overhead routine to use */
  USHORT Counter_H0;       /* (24) Number of prep frames, by convention */
  USHORT Counter_H1;       /* (36) Run time value of H */
  USHORT Counter_I0;       /* (28) Number of frames/mem, by convention */
  USHORT Counter_I1;       /* (30) Run time value of I */
  USHORT Counter_J0;       /* (32) Number of memories, by convention */
  USHORT Counter_J1;       /* (34) Run time value of J */
  USHORT Counter_K0;       /* (36) Number of ignored frames, by convention */
  USHORT Counter_K1;       /* (38) Run time value of K */
  USHORT Counter_L0;       /* (40) Aux. counter; may be array of counters */
  USHORT Counter_L1;       /* (42) Run time value of L */
  USHORT Counter_T0;       /* (44) Number of tracks (spectra) for DAC */
  USHORT Counter_T1;       /* (46) Run time value of T */
  USHORT Counter_S0;       /* (48) Number of slices (points) for DAC */
  USHORT Counter_S1;       /* (50) Run time value of S */
  USHORT MSM_FC_Scan_Mode; /* (52) ASIC FC program to run for frame read */
  USHORT MSM_FC_StreakMode;/* (54) ASIC DC program to run for streak prescan */
  USHORT MSM_FC_StrkOTMode;/* (56) ASIC DC program to run for streak prescan */
  USHORT MSM_DC_Add_Mode;  /* (58) ASIC DC program to run to add data */
  USHORT MSM_DC_Rep_Mode;  /* (60) ASIC DC program to run to replace data */
  USHORT MSM_DC_Sub_Mode;  /* (62) ASIC DC program to run to subtract data */
  USHORT MSM_DC_Ign_Mode;  /* (64) ASIC DC program to run to ignore data */
  USHORT MSM_DC_OH;        /* (66) ASIC DC program to run for expose time */
  USHORT MSM_DC_StreakMode;/* (68) ASIC DC program to run for streak prescan */
  USHORT MSM_DA_Complete;  /* (70) Flag if data acqusition complete */
  USHORT MSM_Serial_Mode;  /* (72) Flag 1 = uniform, 2 = random, others? */
  USHORT MSM_Image_Mode;   /* (74) Flag 1 = uniform, 2 = random, others? */
  USHORT MSM_Keepclean;    /* (76) Flag 0 = fast, 1 = normal */
  USHORT MSM_Pointnum;     /* (78) Number of points to sort */
  ULONG  MSM_SC_Address;   /* (80) Address of source comp array */
  USHORT MSM_StreakMode;   /* (84) Flag whether to do streaker prescan */
  USHORT MSM_PIA_Out;      /* (86) Value to write to PIA */
/* All the rest are written by ASIC so must be on 4-byte boundary  */
  ULONG  MSM_PIA_In    ;   /* (88) Value read from PIA */
  ULONG  MSM_Cooler_Status;/* (92) Status byte from cooler at upper address */
  ULONG  MSM_Shutter_ID;   /* (96) Shutter ID reply */
  ULONG  MSM_Detector_ID;  /*(100) Detector ID reply */

//  USHORT MSM_X0_Array[MAX_POINT_COUNT];
//  USHORT MSM_DeltaX_Array[MAX_POINT_COUNT];
//  USHORT MSM_Y0_Array[MAX_TRACK_COUNT];
//  USHORT MSM_DeltaY_Array[MAX_TRACK_COUNT];
//  unsigned char  reserved[RESERVED_OF_SHARED_AREA]; /* Skips memory up to detector data  */
} SHARED_COMM_AREA ;

/* these are presently defined in cntrloma.c */

extern SHARED_COMM_AREA __far *volatile CommonArea; /* info at start of data mem */
extern MONITOR_PTRS __far *monitor_addr;            /* somewhere in DAC prog mem */

/* Must end on a longword (4 bytes) boundary */

#endif  /*  MONITOR_INCLUDED */
