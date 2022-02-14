/* -----------------------------------------------------------------------
/
/ loadrec.h
/
/ Copyright (c) 1988,  EG&G Princeton Applied Research, Inc.
/
/ Written by: TLB  Version 1.00  12 December 1988
/ Worked on:  TLB  Version 1.01
/             TLB  Version 1.02
/
/ ----------------------------------------------------------------------- */

#include "eggtype.h"

#define LOAD_RECORD_START_ID 0x5555

#define EOF_RECORD           (-1)
#define FC_BLOCK_RECORD      (-2)
#define DMA_BLOCK_RECORD     (-3)
#define FIXUP_RECORD         (-4)
#define DAC_BLOCK_RECORD     (-5)     

#define CNTR_BLOCK_RECORD    (-6)     

#define TAKE_DEFAULT_ADDRESS 65535

/* values for fix_type */

#define FIXUP_FC   0
#define FIXUP_DMA  1
#define FIXUP_DAC  2

#define COUNTER_FC  0
#define COUNTER_DMA 1
#define COUNTER_DAC 2

#define BLK_NAME_LEN 8

typedef struct loadrechdr 
  {
  SHORT start_id;
  SHORT type;
  char name[8];     
  USHORT load_address;
  USHORT length;
  } LoadRecordHeader;

typedef struct fixup_rec         /* structure of fixup records */
   {
   SHORT fix_op_count;           /* operand involved, 1st=1, 2nd=2, etc. */
   USHORT fix_pc;                /* location needing fixing */
   ULONG fix_addr;               /* value to fix at that loc (necessary?) */
   BOOLEAN fix_val;              /* whether fixup is 'validated' */
   SHORT fix_type;               /* processor which 'owns' fixup */
   SHORT fix_len;                /* proposed change to allow fix- */
                                 /* up of movelong refs in dac */
   } fixrec;

typedef struct counter_rec       /* structure of counter records */
   {
   SHORT counter_op_count;       /* operand involved, 1st=1, 2nd=2, etc. */
   USHORT counter_pc;            /* location of count */
   ULONG counter_value;          /* value of cntr at that loc (necessary?) */
   BOOLEAN counter_valid;        /* whether cntr is 'validated' */
   SHORT counter_type;           /* processor which 'owns' cntr */
   SHORT counter_category;       /* category of counter - program specific */
   } counter_record;

