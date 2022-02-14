/********************************************************************/
/* WCDPMI.C                                                         */
/* Morris Maynard, April 1994                                       */
/* Based on code supplied by Qualitas Inc.                          */
/*                                                                  */
/* This module supplies access to the OMA4 controller board from    */
/* programs compiled with the WATCOM 32-bit 386 compiler.  Since    */
/* such programs are already running in protector mode, the only    */
/* services needed from here are (1) Map the board into the 386     */
/* PMMU for this task and (2) provide an LDT selector to access the */
/* mapped memory.  If DPMI services are available, they are used to */
/* accomplish this. Phar Lap does not supply DPMI services itself.  */
/* If Windows is running and the driver PHARLAP.386 is loaded, then */
/* these DPMI services can be used by PharLap. The other DOS ext-   */
/* ender used by this program in 32-bit mode, DOS4GW from Rational, */
/* does supply DPMI services even if no DPMI host was loaded before */
/* DOS4GW.  Thus DPMI works when DOS4GW is loaded or when PharLap   */
/* is loaded under Windows with the proper support driver.          */
/* If DPMI is not detected, then the program tries calling the Phar */
/* Lap extender directly.  If this fails, 0L is returned as the lin-*/
/* ear address of the OMA4 board.                                   */
/*                                                                  */
/********************************************************************/

#include <stdio.h>
#include <dos.h>       /* disable() */
#include <conio.h>

#include "wc32phys.h"

#define DPMICALL 0x31
#define DPMI_ALLOCSEL  0x0000
#define DPMI_FREESEL   0x0001
#define DPMI_SETBASE   0x0007
#define DPMI_SETLIMIT  0x0008
#define DPMI_SETATTRIB 0x0009
#define DPMI_MAPPHYS   0x0800

#define INT_DOS 0x21         /* DOS system call */

/* PharLap Library call designators */

#define _DX_MAP_PHYS 0x250A  /* Map physical memory to end of a selector */
#define _DOS_SEG_ALLOC 0x48  /* Allocate a new LDT selector              */

static ULONG board_linear = 0xc00000;
static ULONG board_limit = 0x200000;
static selector_t board_selector;

struct access_byte__
  {
  UCHAR Accessed       : 1;
  UCHAR SegmentType    : 3;
  UCHAR System         : 1;
  UCHAR PrivilegeLevel : 2;
  UCHAR Present        : 1;
  };

typedef struct access_byte__ access_byte_;
static access_byte_ near access_byte;

struct access_byte_386__
  {
  UCHAR LimitPart2     : 4;
  UCHAR UserBit        : 1;
  UCHAR reserved       : 1;
  UCHAR DefaultSize    : 1;
  UCHAR Granularity    : 1;
  };

typedef struct access_byte_386__ access_byte_386_;
static struct access_byte_386__ near access_byte_386;

/**********************************************************************/
static void near _cdecl setup_access(ULONG limit, UCHAR * arb, UCHAR * arb386)
{
  access_byte.Accessed      = 1; /* unnecessary? */
  access_byte.SegmentType   = 1; /* type RW DATA */
  access_byte.System        = 1; /* system selector */
  access_byte.PrivilegeLevel= 3; /* lowest priv level */
  access_byte.Present       = 1; /* assume physical mem is present */

  access_byte_386.LimitPart2  = (UCHAR)(((limit - 1) >> 16)) & (UCHAR)0x0F;
  access_byte_386.reserved    = 0; /* why clear this? */
  access_byte_386.DefaultSize = 1; /* 16 bit segment type */
  access_byte_386.Granularity = 1; /* depends on size > 1 Mb */

  *(access_byte_*)arb = access_byte;
  *(access_byte_386_*)arb386 = access_byte_386;
}

/*======================================================================*/
/* Function: Allocate Descriptor                                        */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  This function allocates one LDT descriptor.                         */
/*                                                                      */
/*  At the time of allocation, the descriptor is of type DATA with      */
/*  a base and limit of zero.  It is the client's responsibility to     */
/*  set up the descriptor with meaningful values.                       */
/*                                                                      */
/* DPMI reference: INT 31h AX=0000h                                     */
/*                                                                      */
/*======================================================================*/
static selector_t near _cdecl AllocateDescriptor(void)
{
  union REGS inregs;

  inregs.x.eax = DPMI_ALLOCSEL;
  inregs.x.ecx = 1;                    /* allocate only 1 descriptor */
  int386(DPMICALL, &inregs, &inregs);  /* Do DPMI call */
  if (inregs.w.cflag)                  /* Carry set indicates error */
    return 0;

  return(selector_t)inregs.w.ax;
}

/*======================================================================*/
/* Function: Free Descriptor                                            */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  This function returns the specified descriptor to the DPMI host.    */
/*                                                                      */
/* DPMI reference: INT 31h AX=0001h                                     */
/*                                                                      */
/*======================================================================*/
static SHORT near _cdecl FreeDescriptor(selector_t sel)
{
  union REGS inregs;

  inregs.x.eax = DPMI_FREESEL;
  inregs.x.ebx = sel;
  int386(DPMICALL, &inregs, &inregs);  /* Do DPMI call */

  return inregs.w.cflag;               /* Carry set indicates error */
}

/*======================================================================*/
/* Function: Set Segment Base                                           */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  This function sets the base of the segment specified by the sel     */
/*  argument to the value of the base argument.                         */
/*                                                                      */
/* DPMI reference: INT 31h AX=0007h                                     */
/*                                                                      */
/*======================================================================*/
static SHORT near _cdecl SetSegmentBase( selector_t sel, ULONG base)
{
  union REGS inregs;

  int retval;
  inregs.x.eax = DPMI_SETBASE;
  inregs.x.ebx = sel;
  inregs.x.edx = (USHORT)(base & 0xffff);
  inregs.x.ecx = (USHORT)((base >> 16)& 0xffff);
  int386(DPMICALL, &inregs, &inregs);
  if (inregs.w.cflag)
    return 0;
  else
    return inregs.w.ax;
}

/*======================================================================*/
/* Function: Set Segment Limit                                          */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  This function sets the limit of the segment specified by the        */
/*  sel argument to the value given by the limit argument.              */
/*                                                                      */
/* DPMI reference: INT 31h AX=0008h                                     */
/*                                                                      */
/*======================================================================*/
static SHORT near _cdecl SetSegmentLimit(selector_t sel, ULONG limit)
{
  union REGS inregs;

  inregs.x.eax = DPMI_SETLIMIT;
  inregs.x.ebx = sel;
  inregs.x.edx = limit & 0xffff;
  inregs.x.ecx = (limit >> 16) & 0xffff;
  int386(DPMICALL, &inregs, &inregs);
  if (inregs.w.cflag)
    return 0;
  else
    return inregs.w.ax;
}

/*======================================================================*/
/* Function: Set Segment Attributes                                     */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  This function sets the attributes of the segment specified by       */
/*  the sel argument.  The arb argument is the 6th byte of the          */
/*  descriptor, and the arb386 argument is the 8th byte of the          */
/*  descriptor.  Attempts to set privilege levels more privileged       */
/*  than the client level will fail.                                    */
/*                                                                      */
/* DPMI reference: INT 31h AX=0009h                                     */
/*                                                                      */
/*======================================================================*/
static int near _cdecl SetSegmentAttributes(selector_t sel, UCHAR arb, UCHAR arb386)
{
  int retval;

  union REGS inregs;

  inregs.x.eax = DPMI_SETATTRIB;    /* DPMI service code */
  inregs.x.ebx = sel;               /* load selector to modify in bx */
  inregs.h.cl = arb;                /* load attrib bytes in cx */
  inregs.h.ch = arb386;
  int386(DPMICALL, &inregs, &inregs);
  if (inregs.w.cflag)
    return 0;
  else
    return inregs.w.ax;
}

/*======================================================================*/
/* Function: Make Selector                                              */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  For 32 bit mode, it is not necessary to create a new selector for   */
/*  a new offset into the board memoyr.  Instead, the selector created  */
/*  by MapPhysical is combined with the offset supplied in linear to    */
/*  return a far pointer.  Note linear is the full linear address of    */
/*  the board memory for which a pointer is needed; this function       */
/*  computes the offset based on that address and the linear address    */
/*  created by MapPhysical.                                             */
/*                                                                      */
/*======================================================================*/
void far * GetProtectedPointer(ULONG linear)
{
  return MK_FP(board_selector, linear);
}

/*======================================================================*/
/* Function: Free Selector                                              */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  For 32-bit mode, this function is unneccesary.  The 32-bit create   */
/*  selector function merely adds an offset to the existing board sel-  */
/*  ector created when MapPhysical is called.                           */
/*                                                                      */
/*======================================================================*/

void FreeSelector(void far * fPtr)
{
  return;
}

/*======================================================================*/
/* Return TRUE if DPMI services are present                             */
/*======================================================================*/

BOOLEAN IsDPMI(void)
{
  union REGS regs;

  regs.x.eax = 0x1687;           /* code to test for DPMI */
  int386(0x2f, &regs, &regs);    /* DOS Multiplex interrupt */

  if (regs.w.ax != 0)            /* clears AX if successful */
    return FALSE;

//  printf("Host information:\n");
//  printf("\tFlags:\t\t%04x\n",  info.regs.w.bx);
//  printf("\tVersion:  %d.%d\n", regs.h.dh, regs.h.dl);
//  printf("\tProcessor:  80%d86\n", regs.h.cl);
//  printf("\tPrivate data:\t%d paragraphs\n", regs.w.si);

    return FALSE;
}

/*======================================================================*/
/* Return a linear address for a given physical address and size.       */
/* Also allocates a new LDT selector for the linear address.            */
/* Returns 0L on error, else returns the linear address.                */
/*======================================================================*/
ULONG MapPhysical(ULONG physical, ULONG limit)
{
  union REGS regs;
  ULONG linear;
  UCHAR arb = '0',
        arb386 = '0';
     
  if (IsDPMI()) /* this works for DOS4GW and PharLap under Windows */
    {
    /* put physical address in bx:cx */
    regs.x.eax = DPMI_MAPPHYS;                        /* DPMI MapPhys call */
    regs.x.ebx = (USHORT)((physical >> 16) & 0xffff); /* hi order in bx */
    regs.x.ecx = (USHORT)(physical & 0xffff);         /* lo order in cx */
    regs.w.si = (USHORT)((limit >> 16) & 0xffff);     /* hi order limit */
    regs.w.di = (USHORT)(limit & 0xffff);             /* lo order limit */

    int386(DPMICALL, &regs, &regs);                   /* Do DPMI call */

    linear = (regs.x.ebx << 16) & 0xffff0000L;        /* return hi order */
    linear |= regs.x.ecx & 0xffff;                    /* return lo order */

    if (regs.w.cflag & 1) return 0L;                  /* test for error */

    board_linear = linear;                            /* remember linear */

    board_selector = AllocateDescriptor();            /* make selector for */
    SetSegmentBase(board_selector, linear);           /* linear address */
    SetSegmentLimit(board_selector, limit >> 12);
    setup_access(limit, &arb, &arb386);
    SetSegmentAttributes(board_selector, arb, arb386);
    return(linear);
    }
  else /* use PharLap specific way if DPMI not available */
    {
    struct SREGS sregs;

    regs.x.ebx = 0;                /* Zero space allocated. */
    regs.h.ah = _DOS_SEG_ALLOC;    /* PharLap library call designator */
    int386(INT_DOS, &regs, &regs); /* extended DOS call */  
    if (regs.w.cflag & 1)          /* test for error in cflag */
      return(0L);
    board_selector = regs.w.ax;    /* Selector of physical mem in ax */
    sregs.es = regs.w.ax;          /* Setup call to set address of selector */
    regs.x.ebx = physical;         /* Give physical address for selector */
    regs.x.ecx = limit >> 12;      /* Find number of 4K pages to map. */
    regs.w.ax = _DX_MAP_PHYS;      /* PharLap call designator */
    int386x(INT_DOS, &regs, &regs, &sregs);
    if (regs.w.cflag & 1)          /* check for error */
      return(0L);
    board_linear = physical;       /* linear and physical addrs same */
    return physical;
    }
}

