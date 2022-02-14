/********************************************************************/
/* WINPHYS.C                                                        */
/* Morris Maynard, Sept 1994                                        */
/* Based on code supplied by Qualitas Inc.                          */
/*                                                                  */
/* This module supplies access to the OMA4 controller board for     */
/* programs running under Windows 3.1x enhanced mode. Since such    */
/* programs are already running in protector mode, the only ser-    */
/* vices supplied here are (1) Map the board into the 386 PMMU page */
/* table for this task and (2) provide an LDT selector to access    */
/* the mapped memory.  The Windows KERNEL API's are used, except in */
/* the case of SetSelectorLimit; the example code from Microsoft    */
/* Development Library did not work when I used it, so I use DPMI   */
/* services to set the selector limit.                              */
/********************************************************************/

#include <windows.h>
#include <dos.h>

#include "winphys.h"

#define DPMICALL 0x31
#define DPMI_ALLOCSEL  0x0000
#define DPMI_FREESEL   0x0001
#define DPMI_SETBASE   0x0007
#define DPMI_SETLIMIT  0x0008
#define DPMI_SETATTRIB 0x0009
#define DPMI_MAPPHYS   0x0800

#define INT_DOS 0x21 /* DOS system call */

static selector_t BoardSelector;
static __huge * BasePointer;

typedef struct access_byte__
  {
  UCHAR Accessed       : 1;
  UCHAR SegmentType    : 3;
  UCHAR System         : 1;
  UCHAR PrivilegeLevel : 2;
  UCHAR Present        : 1;
  } access_byte_;

typedef struct access_byte_386__
  {
  UCHAR LimitPart2     : 4;
  UCHAR UserBit        : 1;
  UCHAR reserved       : 1;
  UCHAR DefaultSize    : 1;
  UCHAR Granularity    : 1;
  } access_byte_386_;

/**********************************************************************/
static void near _cdecl setup_access(ULONG limit, UCHAR * arb, UCHAR * arb386)
{
  access_byte_ access_byte;
  access_byte_386_ access_byte_386;

  access_byte.Accessed      = 1; /* unnecessary? */
  access_byte.SegmentType   = 1; /* type RW DATA */
  access_byte.System        = 1; /* system selector */
  access_byte.PrivilegeLevel= 3; /* lowest priv level */
  access_byte.Present       = 1; /* assume physical mem is present */

  access_byte_386.LimitPart2  = (UCHAR)(((limit - 1) >> 16)) & (UCHAR)0x0F;
  access_byte_386.reserved    = 0; /* why clear this? */
  access_byte_386.DefaultSize = 1; /* 16 bit segment type */
  access_byte_386.Granularity = 1; /* depends on size > 1 Mb, we assume it is! */

  *(access_byte_*)arb = access_byte;
  *(access_byte_386_*)arb386 = access_byte_386;
}

static int near _cdecl SetSegmentAttributes(selector_t sel, UCHAR arb, UCHAR arb386)
{
  int retval;

  _asm {
    mov ax, 0009h;    /* DPMI service code */
    mov bx, sel;      /* load selector to modify in bx */
    mov cl, arb;      /* load attrib bytes in cx */
    mov ch, arb386;
    int 31h;
    jc  hexit;
    xor ax, ax;
    }
hexit:
    _asm mov retval, ax;
  return retval;
}

BOOL DPMISetSelectorLimit (UINT selector, ULONG dwLimit)
{
  UCHAR arb = '0', arb386 = '0';

  setup_access(dwLimit, &arb, &arb386);
  return SetSegmentAttributes(selector, arb, arb386);
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
  selector_t temp_sel, sel = NULL;

  /* put physical address in bx:cx */
  regs.x.ax = DPMI_MAPPHYS;                        /* DPMI MapPhys call */
  regs.x.bx = (USHORT)((physical >> 16) & 0xffff); /* hi order in bx */
  regs.x.cx = (USHORT)(physical & 0xffff);         /* lo order in cx */
  regs.x.si = (USHORT)((limit >> 16) & 0xffff);    /* hi order limit */
  regs.x.di = (USHORT)(limit & 0xffff);            /* lo order limit */

  int86(DPMICALL, &regs, &regs);                   /* Do DPMI call */

  linear = (ULONG)((ULONG)regs.x.bx << 16) & 0xffff0000L;        /* return hi order */
  linear |= regs.x.cx & 0xffff;                    /* return lo order */

  if (regs.x.cflag & 1) return 0L;                 /* test for error */
  
  temp_sel = AllocSelector(0);
  if (temp_sel)
    {
    SetSelectorBase(temp_sel, linear);
    DPMISetSelectorLimit(temp_sel, limit);
    sel = AllocSelector(temp_sel);
//    DPMISetSelectorLimit(temp_sel, 100L);
//    FreeSelector(temp_sel);
    BoardSelector = sel;
    }

  BasePointer = MK_FP(sel, 0);

  if (sel)
    return(linear);
  else
    return 0;
}

void __huge * GetProtectedPointer(ULONG offset)
{
  return &((char _huge *)BasePointer)[offset];
}

void ReleaseDescriptors(void)
{
  if (BoardSelector)
    FreeSelector(BoardSelector);
}
