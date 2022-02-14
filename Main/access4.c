/***************************************************************************/
/*                                                                         */
/*   access4.c  -  written September 1990 by Morris Maynard                */
/*           (Copyright(C) 1990 EG&G Princeton Applied Research)           */
/*                                                                         */
/*             (some routines originally by Tom Biggs of EG&G)             */
/*                                                                         */
/*   All functions for accessing the OMA4 hardware - register writes,      */
/*   memory access, etc.                                                   */
/*                                                                         */
/*  Copyright (c) 1990,  EG&G Instruments Inc.                             */
/*                                                                         */
/***************************************************************************/

#ifdef _WINOMA_
#include <windows.h>
#endif
#include <dos.h>
#include <stdio.h>
#if defined(USE_D16M) || defined(_WINOMA_)
  #include <memory.h>
#endif
#include <conio.h>
#include <process.h>

#ifndef __TURBOC__
#include <malloc.h>
#else
#include <mem.h>
#define TURBOC
#endif

#include "eggtype.h"
#include "omaregs.h"
#include "monitor.h"
#include "asicodes.h"
#include "cache386.h"
#include "counters.h"
#include "access4.h"

#ifdef USE_D16M
#include "d16mphys.h"
#elif defined( __WATCOMC__ )
#include "wc32phys.h"
#elif defined(_WINOMA_)
#include "winphys.h"
#else
#include "himem.h"
#endif

#define ONEPAGE 0x0000FFFF

ULONG OMA_phys_address     = DEFAULT_ADDR;  /* OMA4 board physical address */
ULONG OMA_memory_address   = DEFAULT_ADDR;  /* VCPI translated address */
ULONG OMA_memory_size      = DEFAULT_SIZE;  /* bytes on OMA4 board */
ULONG OMA_DAC_address      = DEFAULT_ADDR + /* address of DAC on OMA board */
                             DAC_MEM_OFFSET;
UCHAR memsize_code         = TWO_MEGABYTES; /* part of port val to set size */
USHORT OMA_port_address    = 0x300;         /* addr of OMA4 CR0 */

BOOLEAN fake_detector = FALSE; // Truth that OMA4 board is present
BOOLEAN hi_shift = FALSE; // truth that board can address top two meg of ISA memory (14Mb-16Mb)
static BOOLEAN IsReset = FALSE;

#ifdef _PROTECTED_MODE

SHARED_COMM_AREA __far * volatile CommonArea = 0;  /* info at start of data mem */
MONITOR_PTRS __far * monitor_addr   = 0;  /* somewhere in DAC prog mem */

#ifdef _WINOMA_
void __huge * data_selector = 0;
#endif

#define get_shared_area();
#define put_shared_area();

#else

static SHARED_COMM_AREA SharedCommArea; 
SHARED_COMM_AREA far * volatile CommonArea = &SharedCommArea;
static MONITOR_PTRS monitor_ptrs;               
MONITOR_PTRS far * monitor_addr = &monitor_ptrs;

#define get_shared_area(); read_high_memory(CommonArea, 0L, sizeof(SHARED_COMM_AREA));
#define put_shared_area(); write_high_memory(CommonArea, 0L, sizeof(SHARED_COMM_AREA));

counter_link_record far counter_recs[LC_TABLE_LIMIT];
counter_link_record * loop_counters = counter_recs;

#endif

/***************************************************************************/
/*                                                                         */
/* turn on the memory for the OMA4 controller board                        */
/* also enables access to the ASIC program memory                          */
/*                                                                         */
/***************************************************************************/

void enable_controller_memory(SHORT port_addr, ULONG base_address,
                              SHORT memory_size)
{
  OMAControlReg0 CReg0;
  OMAControlReg1 CReg1;
  ULONG address;

  CReg0.byte = 0;
  CReg1.byte = (UCHAR)inp(port_addr + OMA_CONTROL_1);

  CReg1.wbits.int_level = 0;            /* disables interrupt from board */
  CReg1.wbits.prog_mem_lo = LO_TRUE;    /* may be enabled later */
  CReg1.wbits.OMA_reset_lo = LO_TRUE;   /* put board into reset */
  CReg1.wbits.da_attn = FALSE;          /* don't interrupt right now */
  CReg1.wbits.ALU_reset_lo = LO_FALSE;  /* don't reset the ALU! - hangs system */
  CReg1.wbits.host_attn_ack = FALSE;    /* clear the ACK messages */
  CReg1.wbits.cooler_attn_ack = FALSE;

  outp((port_addr + OMA_CONTROL_1), CReg1.byte);
                                                        
  address = ((base_address >> 21) & 7); /* get highest 3 bits of 24-bit address */
  if (address == 0L) address = 1;       /* zero means no memory! */

  base_address = (address << 21);

  CReg0.bits.memory_map = ((UCHAR)((memory_size) << (address - (hi_shift ? 2 : 1))));

  outp((port_addr + OMA_CONTROL_0), CReg0.byte);
  IsReset = TRUE;
}

/***************************************************************************/
/*                                                                         */
/* set the OMA4 memory map to allow access to the ASIC program memory      */
/*                                                                         */
/***************************************************************************/

void map_program_memory(void)
{
  OMAControlReg1 CReg1;

  CReg1.byte = (UCHAR)inp(OMA_port_address + OMA_CONTROL_1);

  CReg1.wbits.int_level = 0;             /* this disables interrupt from board */
  CReg1.wbits.prog_mem_lo = LO_TRUE;     /* until explicitly enabled later */
  CReg1.wbits.OMA_reset_lo = LO_TRUE;    /* put board into reset */
  CReg1.wbits.da_attn = FALSE;           /* don't interrupt right now */
  CReg1.wbits.ALU_reset_lo = LO_FALSE;   /* don't reset the ALU! - hangs system */
  outp((OMA_port_address + OMA_CONTROL_1), CReg1.byte);
  IsReset = TRUE;
}

/**************************************************************************/
/*                                                                        */
/*  Release the reset on the OMA board: the monitor jumps through the     */
/*  reset vector to its init routine, then goes into the HALTED state.    */
/*                                                                        */
/**************************************************************************/
void release_OMA_reset(void)
{
  OMAControlReg1 CReg1;

  if (!IsReset)
    return;

  CReg1.byte = (UCHAR)inp(OMA_port_address + OMA_CONTROL_1);

  CReg1.wbits.da_attn = FALSE;            /* don't interrupt right now */
  CReg1.wbits.OMA_reset_lo = LO_FALSE;
  CReg1.wbits.prog_mem_lo = LO_FALSE;     /* bits should track together */
  CReg1.wbits.ALU_reset_lo = LO_FALSE;    /* don't reset the ALU! - hangs system */
  outp((OMA_port_address + OMA_CONTROL_1), CReg1.byte);
  IsReset = FALSE;
}

/**************************************************************************/
/*                                                                        */
/*  Put the OMA board into reset: All 68000 and ASIC activity stops.      */
/*                                                                        */
/**************************************************************************/
void set_OMA_reset(void)
{
  OMAControlReg1 CReg1;

  CReg1.byte = (UCHAR)inp(OMA_port_address + OMA_CONTROL_1);

  CReg1.wbits.da_attn = FALSE;            /* don't interrupt right now */
  CReg1.wbits.OMA_reset_lo = LO_TRUE;
  CReg1.wbits.ALU_reset_lo = LO_FALSE;    /* don't reset the ALU! - hangs system */
  outp((OMA_port_address + OMA_CONTROL_1), CReg1.byte);
  IsReset = TRUE;
}

/**************************************************************************/
/*                                                                        */
/*  Put the OMA board into reset, but leave data memory mapped, so that   */
/*  DAC parameters may be changed without timing conflicts with the DAC   */
/*                                                                        */
/**************************************************************************/
void reset_and_map_data(void)
{
  OMAControlReg1 CReg1;

  CReg1.byte = (UCHAR)inp(OMA_port_address + OMA_CONTROL_1);

  CReg1.wbits.da_attn = FALSE;            /* don't interrupt right now */
  CReg1.wbits.OMA_reset_lo = LO_TRUE; 
  CReg1.wbits.prog_mem_lo = LO_FALSE;
  CReg1.wbits.ALU_reset_lo = LO_FALSE;    /* don't reset the ALU! - hangs system */
  outp((OMA_port_address + OMA_CONTROL_1), CReg1.byte);
  IsReset = TRUE;
}

/****************************************************************/
/*                                                              */
/* access_alternating_bytes - The FC processor's code occupies  */
/* every other even byte in the ASIC program space, while the   */
/* DC uses every other odd byte.  An odd way to do things...    */
/* The memory MUST be accessed as 16-bit words! Thus to access  */
/* one byte, that byte and its neighbor must be copied to prog. */
/* memory, the byte altered, and the two bytes written back.    */
/* Here we write or read a value to or from a location          */
/* in the ASIC program memory.  Note the address passed is      */
/* an offset within the 64K space of the ASIC program.          */
/*                                                              */
/****************************************************************/

/* WATCOM fmemcpy doesn't optimize into word loops if count is div by 2! */

void access_alternating_bytes(SHORT action, void * value,
                              USHORT offset, USHORT count)
{
  static CHAR * mix_buffer = 0;
  static USHORT bufsize = 0;
  USHORT i, j = 0;
#ifdef _PROTECTED_MODE
  void __far *far_ptr = & ((CHAR __far *)(CommonArea))[offset * 2];
#else
  ULONG address = offset * 2;
#endif
  if (!fake_detector)
  {
  count *= 2;

  if (count > bufsize)
    {
    if (mix_buffer)
      free(mix_buffer);
    mix_buffer = malloc(count);

    if (!mix_buffer)
      return;
    else
      bufsize = count;
    }

  cacheoff();

/* if using Microsoft or Borland compilers, _fmemcpy works OK because it  */
/* moves as many bytes as possible using the movsw instruction, then the  */
/* remainder using movs - and there is never a remainder, since the OMA   */
/* memory has to be moved in 16-bit accesses, never in bytes.  WATCOM did */
/* not do this in their version of _fmemcpy (at least as of ver 9.0) so   */
/* the loop must be done "by hand". And for real mode, must call himem.   */

#if defined(USE_D16M) || defined(_WINOMA_)
  _fmemcpy(mix_buffer, far_ptr, count);
#elif defined(__WATCOMC__)
  for (i = 0; i < count / 2; i++)
    ((USHORT *)mix_buffer)[i] = ((USHORT __far *)far_ptr)[i];
#else
  read_high_memory(mix_buffer, address, count);
#endif

  switch (action)
    {
    case WRITE_ODD:
      j++;

    case WRITE_EVEN:
      for (i = 0; i < count / 2; i++)
        {
        mix_buffer[j] = ((CHAR *)value)[i];
        j += 2;
        }
#if defined(USE_D16M) || defined(_WINOMA_)
      _fmemcpy(far_ptr, mix_buffer, count);
#elif defined(__WATCOMC__)
  for (i = 0; i < count / 2; i++)
    ((USHORT __far *)far_ptr)[i] = ((USHORT *)mix_buffer)[i];
#else
      write_high_memory(mix_buffer, address, count);
#endif
    break;

    case READ_ODD:
      j++;

    case READ_EVEN:
      for (i = 0;i < count / 2; i++)
        {
        ((CHAR*)value)[i] = mix_buffer[j]; /* place OMA data in PC var. */
        j += 2;
        }
    break;
    }
  cacheon();
  } /* if not fake detector */
}

/***************************************************************/
void update_DC_counter(USHORT value, USHORT address)
{
  access_alternating_bytes(WRITE_ODD, &value, address, sizeof(SHORT));
}

/***************************************************************/
void update_FC_counter(USHORT value, USHORT address)
{
  access_alternating_bytes(WRITE_EVEN, &value, address, sizeof(SHORT));
}

/***************************************************************/
USHORT read_DC_counter(USHORT address)
{
  USHORT value;
  access_alternating_bytes(READ_ODD, &value, address, sizeof(SHORT));
  return(value);
}

/***************************************************************/
USHORT read_FC_counter(USHORT address)
{
  USHORT value;
  access_alternating_bytes(READ_EVEN, &value, address, sizeof(SHORT));
  return(value);
}

/***************************************************************/
ULONG read_FC_pointer(USHORT address)
{
  ULONG value;
  access_alternating_bytes(READ_EVEN, &value, address, sizeof(LONG));
  return(value);
}

/***************************************************************/
void update_FC_pointer(ULONG value, USHORT address)
{
  access_alternating_bytes(WRITE_EVEN, &value, address, sizeof(LONG));
}

/* ----------------------------------------------------------------------*/

/***************************************************************/
/*                                                             */
/* get_DAC_counter_address returns the address of an element   */
/* in the monitor shared memory construct                      */
/*                                                             */
/***************************************************************/

USHORT __far *get_DAC_counter_address(SHORT type)
{
  void __far * volatile counter_address;

    get_shared_area();

    switch(type)
      {
      case DA_SEQ:
        counter_address = &(CommonArea->MSM_Pgm_Inx);
      break;
      case H0_COUNTER:
        counter_address = &(CommonArea->Counter_H0);
      break;
      case H1_COUNTER:
        counter_address = &(CommonArea->Counter_H1);
      break;
      case I0_COUNTER:
        counter_address = &(CommonArea->Counter_I0);
      break;
      case I1_COUNTER:
        counter_address = &(CommonArea->Counter_I1);
      break;
      case J0_COUNTER:
        counter_address = &(CommonArea->Counter_J0);
      break;
      case J1_COUNTER:
        counter_address = &(CommonArea->Counter_J1);
      break;
      case K0_COUNTER:
        counter_address = &(CommonArea->Counter_K0);
      break;
      case K1_COUNTER:
        counter_address = &(CommonArea->Counter_K1);
      break;
      case L0_COUNTER:
        counter_address = &(CommonArea->Counter_L0);
      break;
      case L1_COUNTER:
        counter_address = &(CommonArea->Counter_L1);
      break;
      case T0_COUNTER:
        counter_address = &(CommonArea->Counter_T0);
      break;
      case T1_COUNTER:
        counter_address = &(CommonArea->Counter_T1);
      break;
      case S0_COUNTER:
        counter_address = &(CommonArea->Counter_S0);
      break;
      case S1_COUNTER:
        counter_address = &(CommonArea->Counter_S1);
      break;
      case SER_MODE:
        counter_address = &(CommonArea->MSM_Serial_Mode);
      break;
      case IMG_MODE:
        counter_address = &(CommonArea->MSM_Image_Mode);
      break;
      case DA_DONE:
        counter_address = &(CommonArea->MSM_DA_Complete);
      break;
      case SM_SGROUPS:
        counter_address = &(CommonArea->MSM_Pointnum);
      break;
      case SAME_ET:
        counter_address = &(CommonArea->MSM_Keepclean);
      break;
      case DET_ID:
        counter_address = &(CommonArea->MSM_Detector_ID);
      break;
      case SHUTID:
        counter_address = &(CommonArea->MSM_Shutter_ID);
      break;
      case COOL_STATUS:
        counter_address = &(CommonArea->MSM_Cooler_Status);
      break;
      case STREAK_MODE:
        counter_address = &(CommonArea->MSM_StreakMode);
      break;
      case PIA_OUTBITS:
        counter_address = &(CommonArea->MSM_PIA_Out);
      break;
      case PIA_INBITS:
        counter_address = &(CommonArea->MSM_PIA_In);
      break;
      default:
        counter_address = 0L;
      }
    return(counter_address);
}

/****************************************************************/
/*                                                              */
/* get_DAC_counter reads a 16 bit value from the DAC counter    */
/* area of shared memory.  These are the "public" versions of   */
/* the counters, which are being incremented during DA runtime. */
/* primary purpose is to allow liveloop to monitor progress of  */
/* the DA mode.                                                 */
/*                                                              */
/****************************************************************/

USHORT get_DAC_counter(USHORT type)
{
  volatile USHORT __far * counter_address;
  unsigned value;

  if (!fake_detector)
    {
    counter_address = get_DAC_counter_address(type);
    cacheoff();
    value = *counter_address;
    cacheon();
    return(value);
    }
  else return(1); /* usually a safe fake value to return */
}

/****************************************************************/
/*                                                              */
/* set_DAC_counter sets a 16 bit value into the DAC counter     */
/* area of shared memory.  set_DAC_pointer sets a 32 bit value  */
/* into the DAC pointer area of shared memory.                  */
/*                                                              */
/****************************************************************/

void set_DAC_counter(USHORT type, USHORT value)
{
  volatile USHORT __far * counter_address;

  if (!fake_detector)
    {
    counter_address = get_DAC_counter_address(type);
    cacheoff();
    *counter_address = value;
    put_shared_area();
    cacheon();
    }
}

/*****************************************************************/
/*                                                               */
/* Get 32 bit address of value in the Monitor Shared Memory area */
/*                                                               */
/*****************************************************************/

ULONG __far * get_DAC_pointer_address(SHORT type)
{
  void __far * pointer_address;

  get_shared_area();

  switch(type)
    {
    case FRAME_PTR:
      pointer_address = &CommonArea->MSM_Data_Offset;
    break;    
    case BKGND_PTR:
      pointer_address = &(CommonArea->MSM_Bgnd_Offset);
    break;               
    case SIZE_PTR:           
      pointer_address = &(CommonArea->MSM_Data_Size);
    break;
    case SRCCMP_PTR:
      pointer_address = &(CommonArea->MSM_SC_Address);
    break;
    case DET_ID:
      pointer_address = &(CommonArea->MSM_Detector_ID);
    break;
    case SHUTID:
      pointer_address = &(CommonArea->MSM_Shutter_ID);
    break;
    case COOL_STATUS:
      pointer_address = &(CommonArea->MSM_Cooler_Status);
    break;
    case PIA_INBITS:
      pointer_address = &(CommonArea->MSM_PIA_In); /* only for input! */
    break;
    default:
      pointer_address = 0L;
    }
  return(pointer_address);
}

/****************************************************************/
/*                                                              */
/* Retrieve a 32 Bit value from the Monitor Shared Memory area  */
/*                                                              */
/****************************************************************/

ULONG get_DAC_pointer(SHORT type)
{
  USHORT Value[2];
  volatile ULONG __far * pointer_address;

  /* assumes data memory enabled! */

  if (!fake_detector)
    {
//    release_OMA_reset();
    pointer_address = get_DAC_pointer_address(type);
    cacheoff();

    Value[1] = ((USHORT __far *)(pointer_address))[0];
    Value[0] = ((USHORT __far *)(pointer_address))[1];

    cacheon();
    }
  return *((ULONG*)Value);
}

/****************************************************************/
/*                                                              */
/* Update a 32 Bit value in the Monitor Shared Memory area      */
/*                                                              */
/****************************************************************/

void set_DAC_pointer(SHORT type, ULONG value)
{
  volatile ULONG __far * pointer_address;

  /* assumes data memory enabled! */

  if (!fake_detector)
    {
    pointer_address = get_DAC_pointer_address(type);

    cacheoff();
  
    ((USHORT __far *)(pointer_address))[1] = (USHORT)(value & 0xFFFF);
    ((USHORT __far *)(pointer_address))[0] = (USHORT)( ( value >> 16) & 0xFFFF);

    put_shared_area();

    cacheon();
    }
}

/***************************************************************************/
/*                                                                         */
/* Find out if OMA4 board present - make sure the memory can be written to */
/* and read from, and that different memory spaces are selected when the   */
/* proper values are written to the control registers.  A crude test.      */
/* This version for the OMA4000 application uses DOS16M to access XMemory. */
/* Another version below uses INT15 calls instead for standalone driver.   */
/*                                                                         */
/***************************************************************************/

#ifdef _PROTECTED_MODE
static BOOLEAN find_detector(void) /* returns truth that detector is fake */
{
  USHORT __far * board_addr; /* selector for board memory */
  USHORT prog_save, data_save, test_pat = 0xA5B5;
  BOOLEAN there = TRUE;

  board_addr = GetProtectedPointer(12L);

  cacheoff();

  map_program_memory();                     /* start with program mem */

  prog_save = *board_addr;                  /* save original contents */
  if (prog_save == test_pat)
    test_pat++;                             /* avoid nasty coincidences */
  
  *board_addr = test_pat;
  if ((*board_addr & 0xFFFF) != (test_pat & 0xFFFF))
    there = FALSE;
  else                                      /* can read/write this mem */
    {
    *board_addr = ~test_pat;                /* try complement too */
    if ((*board_addr & 0xFFFF) != (~test_pat & 0xFFFF))
     there = FALSE;
    else
      {
      release_OMA_reset();                   /* now try data memory */

      data_save = *board_addr;               /* save data memory contents */
      *board_addr = test_pat;                /* put test pattern there */
                                             /* (inverse still in prog. mem) */
      if ((*board_addr & 0xFFFF) != (test_pat & 0XFFFF))
        there = FALSE;                       /* did write succeed? */
      else
        {
        map_program_memory();
        if ((*board_addr & 0xFFFF) != (~test_pat & 0xFFFF))
          there = FALSE;                     /* did swap succeed? */
        }
      *board_addr = prog_save;               /* restore original contents */
      release_OMA_reset();
      *board_addr = data_save;
      }
    }
  cacheon();
  return(!there);
}

#else /* if not DOS16M */

/***************************************************************************/
/*                                                                         */
/* Find out if OMA4 board present - make sure the memory can be written to */
/* and read from, and that different memory spaces are selected when the   */
/* proper values are written to the control registers.  A crude test.      */
/* This version for the standalone driver uses INT15 calls to access XMEM  */
/*                                                                         */
/***************************************************************************/
static BOOLEAN find_detector(void) /* returns truth that detector is fake */
{
  USHORT board_value;
  USHORT prog_save, data_save, test_store = 0xA5B5;
  ULONG test_address = 12L;

  map_program_memory();

  read_high_memory((void __far *)(&prog_save), test_address, sizeof(SHORT));

  if (prog_save == test_store) test_store++;

  write_high_memory((void __far *)(&test_store),test_address, sizeof(SHORT));

  read_high_memory((void __far *)(&board_value),test_address, sizeof(SHORT));

  if (board_value != test_store) return 1;

  test_store = ~test_store;

  write_high_memory((void __far *)(&test_store),test_address, sizeof(SHORT));

  read_high_memory((void __far *)(&board_value),test_address, sizeof(SHORT));

  if (board_value != test_store) return 2;

  test_store = ~test_store;
  release_OMA_reset();

  read_high_memory((void __far *)(&data_save),test_address, sizeof(SHORT));

  write_high_memory((void __far *)(&test_store),test_address, sizeof(SHORT));

  read_high_memory((void __far *)(&board_value),test_address, sizeof(SHORT));

  if (board_value != test_store) return 3;

  map_program_memory();
  read_high_memory((void __far *)(&board_value),test_address, sizeof(SHORT));

  if (board_value != ~test_store) return 4;

  write_high_memory((void __far *)(&prog_save),test_address, sizeof(SHORT));
  release_OMA_reset();
  write_high_memory((void __far *)(&data_save),test_address, sizeof(SHORT));
  return(0);
}
#endif

#ifndef _PROTECTED_MODE
void init_local_data(void)
{
  if (!fake_detector)
    {                  
    map_program_memory();
    read_high_memory(monitor_addr, DAC_MEM_OFFSET, sizeof(MONITOR_PTRS));
    read_high_memory(loop_counters, DAC_MEM_OFFSET +
            monitor_addr->loopcntr_table_ptr,
            monitor_addr->loopcntr_table_index *
            sizeof(counter_link_record));
    release_OMA_reset();
    read_high_memory(CommonArea, 0L, sizeof(SHARED_COMM_AREA));
    }
}
#endif

/***************************************************************************/
/*                                                                         */
/* try to connect to detector memory  - return non-zero on error           */
/* to be called whenever a new detector is selected, including when        */
/* application first starts                                                */
/*                                                                         */
/***************************************************************************/
BOOLEAN access_init_detector(USHORT port, ULONG memsize)
{
  if (port)    OMA_port_address = port;
  if (memsize) OMA_memory_size = memsize;

  switch (memsize)
  {
  default:
  case 0x200000L:
    memsize_code = TWO_MEGABYTES;
  break;
  case 0x400000L:
    memsize_code = FOUR_MEGABYTES;
    break;
  case 0x600000L:
    memsize_code = SIX_MEGABYTES;
  break;
  case 0x800000L:
    memsize_code = EIGHT_MEGABYTES;
  break;
  }

  enable_controller_memory(OMA_port_address, OMA_phys_address, memsize_code);
  fake_detector = find_detector();
  return(fake_detector);
}

/***************************************************************************/
/*                                                                         */
/* init global addresses; do VCPI/DPMI remap and DOS16M selector allocation*/
/* to be called once when program first starts                             */
/*                                                                         */
/***************************************************************************/
#ifndef _MSC_VER
#pragma argsused
#endif
void access_startup_detector(ULONG memaddr, ULONG memsize)
{
  /* prevent from calling multiple times so '386 resources not used up */
  static BOOLEAN first_time = TRUE;
  if (first_time)
    {
    first_time = FALSE;
    if (memaddr) OMA_phys_address = memaddr;

#ifdef _PROTECTED_MODE /* check if VCPI or DPMI present */
    if (OMA_memory_address = MapPhysical(OMA_phys_address, memsize))
      ;
    else
#endif
      OMA_memory_address = OMA_phys_address;

    OMA_DAC_address = OMA_memory_address + DAC_MEM_OFFSET;

#ifdef _PROTECTED_MODE
    if (!CommonArea)
      CommonArea = GetProtectedPointer(0);
    if (!monitor_addr)
      monitor_addr = GetProtectedPointer(DAC_MEM_OFFSET);
#else
    init_himem(OMA_phys_address);
#endif
    }
}

/***************************************************************************/
/*                                                                         */
/* Return a far pointer to the beginning of OMA4 detector data             */
/*                                                                         */
/***************************************************************************/

#ifdef _WINOMA_
void __huge * get_data_address(void)
{
  ULONG data_offset = get_DAC_pointer(FRAME_PTR);
  return GetProtectedPointer(data_offset);
}
#elif defined(_PROTECTED_MODE)
void __far * get_data_address(void)
{
  ULONG data_offset = get_DAC_pointer(FRAME_PTR);
  return GetProtectedPointer(data_offset);
}
#endif

void access_shutdown_detector(void)
{
#ifdef _WINOMA_  
  ReleaseDescriptors();
#endif
}
