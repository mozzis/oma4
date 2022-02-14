/* -----------------------------------------------------------------------
/   himem.c
/
/ Copyright (c) 1989,  EG&G Princeton Applied Research, Inc.
/
/ Written by: TLB   Version 1.00    20-28 November    1989
/
/ ----------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>
#ifndef __TURBOC__
#include <memory.h>
#else
#include <mem.h>
#endif
#include <bios.h>
#include <dos.h>

#include "himem.h"
#include "oma4phys.h"

#define BIOS_AT_SUPPORT    0x15
#define BLOCK_MOVE         0x87
#define SOURCE_DESC        2
#define DESTINATION_DESC   3
#define GDT_SIZE           6

SEGMENT_DESCRIPTOR GlobalDescriptorTable[GDT_SIZE];
ULONG HighMemBaseAddress = 0xC00000L;
BOOLEAN DPMI_Present = FALSE;

/* -----------------------------------------------------------------------
/ function: build a segment descriptor (selector) for a linear address
/ requires: pointer to where the selector will reside, address info
/ returns:  (void)
/ side effects: 
/ ----------------------------------------------------------------------- */

void build_segment_descriptor(SEGMENT_DESCRIPTOR * desc,
  ULONG base_address, ULONG max_size,
  UCHAR segment_type, UCHAR privilege_level,
  BOOLEAN operands_32_bit, BOOLEAN system_segment)
{
  desc->BaseAddressPart1 = (USHORT) (base_address & 0xFFFFL);
  desc->BaseAddressPart2 = (UCHAR) ((base_address >> 16) & 0xFF);
  desc->BaseAddressPart3 = (UCHAR) ((base_address >> 24) & 0xFF);
  
  if (max_size > 1000000L) /* is max_size greater than 1 megabyte? */
  {
    max_size = (max_size >> 12);
    desc->Granularity = TRUE;
  }
  else 
    desc->Granularity = FALSE;

  max_size--;
  desc->LimitPart1 = (USHORT) (max_size & 0xFFFFL);
  desc->LimitPart2 = (UCHAR) ((max_size >> 16) & 0x0F);

  desc->SegmentType = segment_type;
  desc->PrivilegeLevel = privilege_level;

  if (operands_32_bit)
    desc->DefaultSize = TRUE;
  else 
    desc->DefaultSize = FALSE;

  if (system_segment)
    desc->System = TRUE;
  else 
    desc->System = FALSE;

  desc->Present = TRUE; /* I hope it is!  (if not, causes fatal crash!!!) */

  desc->reserved = FALSE;
}

#if FALSE
// #include <stdio.h>
/* -----------------------------------------------------------------------
/ function: dump global descriptor table for diagnostic purposes
/ requires: (void)
/ returns:    (void)
/ side effects: 
/ ----------------------------------------------------------------------- */

void dump_gdt(void)
{
  int             i;
  SEGMENT_DESCRIPTOR *    desc;

  printf("\nLim1 BAddr1 BAddr2 A Typ S PL P L2 U D G BA3");

  for (i=0; i<GDT_SIZE; i++)
  {
    desc = &GlobalDescriptorTable[i];
    printf("\n%5u %5u %5u  %u %3u %u  %u %u %3u %u %u %u %4u",
    desc->LimitPart1,
    desc->BaseAddressPart1,
    desc->BaseAddressPart2,
    desc->Accessed,
    desc->SegmentType,
    desc->System,
    desc->PrivilegeLevel,
    desc->Present,
    desc->LimitPart2,
    desc->UserBit,
    desc->DefaultSize,
    desc->Granularity,
    desc->BaseAddressPart3 );
    getch();
  }
}
#endif

/* -----------------------------------------------------------------------
/ function: clean out the GDT
/ requires: (void)
/ returns:  (void)
/ side effects: 
/ ----------------------------------------------------------------------- */

void init_GDT(void)
{
  int i;

  for (i=0; i<GDT_SIZE; i++)
    memset(&GlobalDescriptorTable[i], 0, sizeof(SEGMENT_DESCRIPTOR));
}

/* -----------------------------------------------------------------------
/ function: convert an intel segment:offset address to linear
/ requires: (void)
/ returns:    (void)
/ side effects: 
/ ----------------------------------------------------------------------- */

ULONG intel_to_linear_address(void far * base_address_ptr)
{
  return ( ((ULONG) FP_SEG(base_address_ptr) << 4)
    + (ULONG) FP_OFF(base_address_ptr) );
}

/* -----------------------------------------------------------------------
/ function: Use the AT BIOS int 15 service to move words between extended
/           memory and low (DOS) memory
/ requires: pointer to the GDT containing source and dest pointers, count
/ returns:    (void)
/ side effects: 
/ ----------------------------------------------------------------------- */

UCHAR bios_himem_block_move(void * gdt, USHORT word_count)
{
  union REGS   general;
  struct SREGS segment;

  /* dump_gdt(); */

  general.h.ah = BLOCK_MOVE;
  general.h.al = 0;
  general.x.bx = 0;
  general.x.cx = word_count;
  general.x.dx = 0;
  general.x.si = FP_OFF(gdt);
  segment.es   = FP_SEG(gdt);
  segment.ds   = FP_SEG(gdt);

  _int86x(BIOS_AT_SUPPORT, &general, &general, &segment);

  return(general.h.ah);
}

/* -----------------------------------------------------------------------
/ function: read a block of high memory
/ requires: (void)
/ returns:  (void)
/ side effects: 
/ ----------------------------------------------------------------------- */
BOOLEAN read_high_memory(void far * low_addr, ULONG high_addr, USHORT bytes)
{
#ifndef __TURBOC__  
  if (DPMI_Present)
    {
    read_board_data(high_addr, low_addr, bytes);
    return(FALSE);
    }
  else
#endif
    {
    ULONG source_address;
    ULONG destination_address;
    USHORT word_count = ((bytes + 1) / 2);

    source_address = high_addr + HighMemBaseAddress;
    destination_address = intel_to_linear_address(low_addr);

    build_segment_descriptor( &GlobalDescriptorTable[SOURCE_DESC],
      source_address, SEGMENT_LENGTH_64K, SEGMENT_TYPE_RW_DATA,
      SEGMENT_PRIORITY_HIGHEST, FALSE, TRUE);

    build_segment_descriptor( &GlobalDescriptorTable[DESTINATION_DESC],
      destination_address, SEGMENT_LENGTH_64K, SEGMENT_TYPE_RW_DATA,
      SEGMENT_PRIORITY_HIGHEST, FALSE, TRUE);

    return(BOOLEAN)bios_himem_block_move(GlobalDescriptorTable, word_count);
    }
}

/* -----------------------------------------------------------------------
/ function: write a block of high memory
/ requires: (void)
/ returns:  (void)
/ side effects:
/ ----------------------------------------------------------------------- */
BOOLEAN write_high_memory(void far * low_addr, ULONG high_addr, USHORT bytes)
{
#ifndef __TURBOC__  
  if (DPMI_Present)
    {
    write_board_data(high_addr, low_addr, bytes);
    return(FALSE);
    }
  else
#endif
    {
    ULONG source_address;
    ULONG destination_address;
    USHORT word_count = ((bytes + 1) / 2);

    source_address = intel_to_linear_address(low_addr);
    destination_address = high_addr + HighMemBaseAddress;

    build_segment_descriptor( &GlobalDescriptorTable[SOURCE_DESC],
      source_address, SEGMENT_LENGTH_64K, SEGMENT_TYPE_RW_DATA,
      SEGMENT_PRIORITY_HIGHEST, FALSE, TRUE);

    build_segment_descriptor( &GlobalDescriptorTable[DESTINATION_DESC],
      destination_address, SEGMENT_LENGTH_64K, SEGMENT_TYPE_RW_DATA,
      SEGMENT_PRIORITY_HIGHEST, FALSE, TRUE);

    return(BOOLEAN)bios_himem_block_move(GlobalDescriptorTable, word_count);
    }
}

BOOLEAN init_himem(ULONG base_address)
{
#ifndef __TURBOC__  
  if (!init_DPMI(base_address))                /* 0 gives success */
    {
  #ifdef __TURBOC__                            /* for BorlandC               */
   #ifdef _M_I86LM                             /* if large model             */
    atexit(DPMI_Terminate);                    /* make sure exit is via DPMI */
   #else                                       /* for COMPACT or SMALL       */
    atexit((atexit_t near)DPMI_Terminate);     /* only pass offset of addr   */
   #endif
  #else                                        /* Microsoft C                */
   #ifdef M_I86LM
    atexit(DPMI_Terminate);                    /* make sure exit is via DPMI */
   #else
    atexit((void *)near)DPMI_Terminate);
   #endif
  #endif
    DPMI_Present = TRUE;
    }
  else
#endif
    {
    init_GDT();
    HighMemBaseAddress = base_address;
    }
  return(FALSE);
}
