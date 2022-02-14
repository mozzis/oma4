#include <stdio.h>
#include <stdlib.h> /* strtoul */
#include <string.h> /* strtoul */
#include <conio.h>
#include <dos16.h>

#include "primtypes.h"
#include "cache386.h"
#include "omaregs.h"
#include "mapphys.h"


ULONG OMA_phys_address     = 0x200000L;      /* OMA4 board physical address */
static ULONG OMA_memory_address   = 0x200000L;  /* VCPI translated address */
UCHAR OMA_memory_size_code = TWO_MEGABYTES;  /* part of code determined by size */
USHORT OMA_port_address    = 0x300;          /* addr of CR0 */
ULONG OMA_memory_size      = 0x200000L;      /* bytes on OMA4 board */

BOOLEAN fake_detector = FALSE;
BOOLEAN verbose = FALSE;

/***************************************************************************/
/*                                                                         */
/* turn on the memory for the OMA4 controller board                        */
/* also enables access to the ASIC program memory                          */
/*                                                                         */
/***************************************************************************/

void enable_controller_memory(int port_addr, unsigned long * base_address,
                              int memory_size)
{
  OMAControlReg0 CReg0;
  OMAControlReg1 CReg1;
  unsigned long address;

  CReg0.byte = 0;
  CReg1.byte = (unsigned char)inp( port_addr + OMA_CONTROL_1 );

  CReg1.wbits.int_level = 0;            /* this disables interrupt from board */
  CReg1.wbits.prog_mem_lo = LO_TRUE;    /* until explicitly enabled later */
  CReg1.wbits.OMA_reset_lo = LO_TRUE;   /* put board into reset */
  CReg1.wbits.da_attn = FALSE;          /* don't interrupt right now */
  CReg1.wbits.ALU_reset_lo = LO_FALSE;  /* don't reset the ALU! - hangs system */
  CReg1.wbits.host_attn_ack = FALSE;    /* clear the ACK messages */
  CReg1.wbits.cooler_attn_ack = FALSE;

  outp( (port_addr + OMA_CONTROL_1), CReg1.byte );
                                                        
  address = ((*base_address >> 21) & 7); /* get highest 3 bits of 24-bit address */
  if (address == 0L) address = 1;        /* zero means no memory! */

  *base_address = (address << 21);

  CReg0.bits.memory_map = ((UCHAR)((memory_size) << (address - 1)) );

  outp( (port_addr + OMA_CONTROL_0), CReg0.byte );
}

/***************************************************************************/
/*                                                                         */
/* set the OMA4 memory map to allow access to the ASIC program memory      */
/*                                                                         */
/***************************************************************************/

void map_program_memory(void)
{
  OMAControlReg1 CReg1;

  CReg1.byte = (unsigned char)inp( OMA_port_address + OMA_CONTROL_1 );

  CReg1.wbits.int_level = 0;             /* this disables interrupt from board */
  CReg1.wbits.prog_mem_lo = LO_TRUE;     /* until explicitly enabled later */
  CReg1.wbits.OMA_reset_lo = LO_TRUE;    /* put board into reset */
  CReg1.wbits.da_attn = FALSE;           /* don't interrupt right now */
  CReg1.wbits.ALU_reset_lo = LO_FALSE;   /* don't reset the ALU! - hangs system */
  outp( (OMA_port_address + OMA_CONTROL_1), CReg1.byte );
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

  CReg1.byte = (unsigned char)inp( OMA_port_address + OMA_CONTROL_1 );

  CReg1.wbits.da_attn = FALSE;            /* don't interrupt right now */
  CReg1.wbits.OMA_reset_lo = LO_FALSE;
  CReg1.wbits.prog_mem_lo = LO_FALSE;     /* bits should track together */
  CReg1.wbits.ALU_reset_lo = LO_FALSE;    /* don't reset the ALU! - hangs system */
  outp( (OMA_port_address + OMA_CONTROL_1), CReg1.byte );
}

/**************************************************************************/
/*                                                                        */
/*  Put the OMA board into reset: All 68000 and ASIC activity stops.      */
/*                                                                        */
/**************************************************************************/

void set_OMA_reset(void)
{
  OMAControlReg1 CReg1;

  CReg1.byte = (unsigned char)inp( OMA_port_address + OMA_CONTROL_1 );

  CReg1.wbits.da_attn = FALSE;            /* don't interrupt right now */
  CReg1.wbits.OMA_reset_lo = LO_TRUE;
  CReg1.wbits.ALU_reset_lo = LO_FALSE;    /* don't reset the ALU! - hangs system */
  outp( (OMA_port_address + OMA_CONTROL_1), CReg1.byte );
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

  CReg1.byte = (unsigned char)inp( OMA_port_address + OMA_CONTROL_1 );

  CReg1.wbits.da_attn = FALSE;            /* don't interrupt right now */
  CReg1.wbits.OMA_reset_lo = LO_TRUE; 
  CReg1.wbits.prog_mem_lo = LO_FALSE;
  CReg1.wbits.ALU_reset_lo = LO_FALSE;    /* don't reset the ALU! - hangs system */
  outp( (OMA_port_address + OMA_CONTROL_1), CReg1.byte );
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

static BOOLEAN find_detector(void) /* returns truth that detector is fake */
{
  unsigned int far * board_addr; /* selector for board memory */
  unsigned int prog_save, data_save, test_pat = 0xA5B5;
  BOOLEAN there = TRUE;

  board_addr = D16SegAbsolute(OMA_memory_address+ 12, 0);

  cacheoff();

  map_program_memory();                    /* start with program mem */

  prog_save = *board_addr;                 /* save original contents */
  if (prog_save == test_pat) test_pat++;   /* avoid nasty coincidences */
  *board_addr = test_pat;
  if (*board_addr != test_pat) there = FALSE;
  if (there)                               /* can read/write this mem */
	 {
	 *board_addr = ~test_pat;                /* try complement too */
	 if (*board_addr != ~test_pat) there = FALSE;

	 if (there)
		{
		release_OMA_reset();                   /* now try data memory */

		data_save = *board_addr;               /* save data memory contents */
		*board_addr = test_pat;                /* put test pattern there */
                                           /* (inverse still in prog. mem) */
		if (*board_addr != test_pat) there = FALSE; /* did write succeed? */
		if (there)
		  {
		  map_program_memory();
		  if (*board_addr != ~test_pat) there = FALSE; /* did swap succeed? */
		  }
		*board_addr = prog_save;               /* restore original contents */
		release_OMA_reset();
		*board_addr = data_save;
		}
	 }
  D16SegCancel(board_addr);
  cacheon();
  return(!there);
}

/***************************************************************************/
/*                                                                         */
/* try to connect to detector memory  - return non0 on error               */
/*                                                                         */
/***************************************************************************/
int setup_detector_interface(USHORT port_addr, ULONG memaddr, ULONG memsize)
{
  ULONG tlong;

  if (memaddr) OMA_phys_address = memaddr;

  if (tlong = D16MapPhysical( OMA_phys_address, memsize, 1))
    OMA_memory_address = tlong;
  else
    OMA_memory_address = OMA_phys_address;

  if (port_addr) OMA_port_address = port_addr;
  if (memsize)   OMA_memory_size = memsize;

  switch (memsize)
	  {
		default:
		case 0x200000L:
		  OMA_memory_size_code = TWO_MEGABYTES;
		break;
		case 0x400000L:
		  OMA_memory_size_code = FOUR_MEGABYTES;
		  break;
		case 0x600000L:
		  OMA_memory_size_code = SIX_MEGABYTES;
		break;
		case 0x800000L:
		  OMA_memory_size_code = EIGHT_MEGABYTES;
		break;
		}

  enable_controller_memory(OMA_port_address, &OMA_phys_address,
      OMA_memory_size_code);

  /* release_OMA_reset */

  fake_detector = find_detector();
  return(fake_detector);
}

void get_command_line(int argc, char * argv[],
                      USHORT *port_addr, ULONG *phys_addr, 
                      ULONG *size_addr)
{
  SHORT CurrentArg;
  char * dummy;
  ULONG utemp;
  USHORT shortTemp ;

  for (CurrentArg = 1; CurrentArg < argc; CurrentArg++)
    {
    if ( (argv[CurrentArg][0] == '-') || (argv[CurrentArg][0] == '/'))
      {
      switch (toupper(argv[CurrentArg][1]))
        {
        case 'A':                                           
          if (utemp = strtoul(&(argv[CurrentArg][2]), &dummy, 16))
            *phys_addr = utemp;
          else
            printf("Bad address: %s\n", argv[CurrentArg]);
        break;
        case 'C':
          if (!find_cache_type(argv[++CurrentArg]))
            CurrentArg--;
        break;
        case 'P':
          if( sscanf(&(argv[CurrentArg][2]), "%x", &shortTemp) == 1)
            *port_addr = shortTemp;
          else
            printf("Bad port address: %s\n", argv[CurrentArg]);
        break;
        case 'S':
          if (utemp = strtoul(&(argv[CurrentArg][2]), &dummy, 16))
            *size_addr =  utemp ;
          else
            printf("Bad memsize: %s\n", argv[CurrentArg]);
        break;
        case 'V':
          verbose = TRUE;
        break;
        default:
          printf("Bad argument: %s\n", argv[CurrentArg]);
        break;
        }
      }
    }
}

int main( int argc, char *argv[])
{
  ULONG OMA_addr = 0, OMA_memsize = 0;
  USHORT OMA_port = 0;
 
  get_command_line(argc,argv, &OMA_port, &OMA_addr, &OMA_memsize);
  setup_detector_interface(OMA_port, OMA_addr, OMA_memsize);
  if (verbose)
    {
    printf("Address  = %lx\n"
           "Physical = %lx\n"
           "Size     = %lx\n"
           "Port     = %x\n",
            OMA_memory_address,
            OMA_phys_address,
            OMA_memory_size,
            OMA_port_address);
    printf("Detector %s found\n", fake_detector ? "was not" : "was");
    }
  return(!fake_detector);
}
