/* -----------------------------------------------------------------------
/
/   himem.h
/
/ Copyright (c) 1989,  EG&G Princeton Applied Research, Inc.
/
/ Re-created by: TLB    Version 1.00    13  March   1989
/ Worked on:    TLB   Version 1.01
/
/
/ ----------------------------------------------------------------------- */

#ifndef HIMEM_INCLUDED
#define HIMEM_INCLUDED

#include "eggtype.h"

#define SEGMENT_TYPE_RO_DATA   0
#define SEGMENT_TYPE_RW_DATA   1
#define SEGMENT_TYPE_RO_STACK  2
#define SEGMENT_TYPE_RW_STACK  3
#define SEGMENT_TYPE_PURE_CODE 4  /* no constants allowed! */
#define SEGMENT_TYPE_CODE      5
#define SEGMENT_TYPE_CONF_PURE_CODE 6
#define SEGMENT_TYPE_CONF_CODE      7

#define SEGMENT_PRIORITY_HIGHEST  0
#define SEGMENT_PRIORITY_HIGH     1
#define SEGMENT_PRIORITY_LOW      2
#define SEGMENT_PRIORITY_LOWEST   3


#define SEGMENT_LENGTH_64K 0xFFFFL

#define WRITE_TO_HIGH_MEM  0
#define READ_FROM_HIGH_MEM 1

/* 
  This is the segment descriptor format for the Intel 80286/80386
  processor.  An array of these is used by the processor to determine
  segment locations in physical memory.  They must be initialized
  correctly by the calling program before attempting to enter
  protected mode, or the computer will lose it.  To see an example
  of initializing this structure, see himem.c, where it is used with
  the BIOS protected mode support routine.
*/

typedef struct {

  unsigned int    LimitPart1;
  unsigned int    BaseAddressPart1;
  unsigned char   BaseAddressPart2;
  unsigned char   Accessed          : 1 ;
  unsigned char   SegmentType       : 3 ;
  unsigned char   System            : 1 ;
  unsigned char   PrivilegeLevel    : 2 ;
  unsigned char   Present           : 1 ;
  unsigned char   LimitPart2        : 4 ;
  unsigned char   UserBit           : 1 ;
  unsigned char   reserved          : 1 ;
  unsigned char   DefaultSize       : 1 ;
  unsigned char   Granularity       : 1 ;
  unsigned char   BaseAddressPart3;

} SEGMENT_DESCRIPTOR;

BOOLEAN read_high_memory(void far * low_mem_addr, ULONG high_mem_addr,
                         USHORT byte_count);
BOOLEAN write_high_memory(void far * low_mem_addr, ULONG high_mem_addr,
                          USHORT byte_count);
BOOLEAN init_himem(ULONG base_address);

#endif /*  HIMEM_INCLUDED */
