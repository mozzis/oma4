/********************************************************************/
/* OMA4DPMI.C                                                       */
/* Morris Maynard, July 1993                                        */
/* Based on code supplied by Qualitas Inc.                          */
/*                                                                  */
/* This module features the function get_board_data, which will read*/
/* data from memory on the OMA4 board.  Calls to a DPMI host are    */
/* used to switch into protected mode so the extended memory can    */
/* be accessed.  This module is totally self contained, and is      */
/* written so that once the switch to protected mode is made, no    */
/* segment loads occur. It may be compiled in any memory model and  */
/* linked with real-mode programs to provide them with extended     */
/* memory access.                                                   */
/*                                                                  */
/********************************************************************/

#include <stdio.h>
#include <dos.h>       /* disable() */
#include <conio.h>
#include <toolhelp.h>
#include "oma4dpmi.h"

/* basic typedefs */

union reg     /* single long register */
{ ULONG l;
  USHORT s;
};

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

static ULONG near board_linear;
static ULONG near board_limit = 0x200000;

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

#ifndef __TURBOC__
#pragma optimize("", off)
#else
#pragma option -Od
#endif

/**********************************************************************/
static ULONG near _cdecl shift_right_12(ULONG number)
{
  _asm {
    mov     ax,WORD PTR number;
    mov     dx,WORD PTR number+2
	 mov     al,ah
    mov     ah,dl
    mov     dl,dh
    sub     dh,dh
    shr     dx,1
    rcr     ax,1
    shr     dx,1
    rcr     ax,1
    shr     dx,1
    rcr     ax,1
    shr     dx,1
    rcr     ax,1
    mov     WORD PTR number, ax
    mov     WORD PTR number+2, dx
    }
  return number;
}

/**********************************************************************/
static ULONG near _cdecl shift_left_4(USHORT number)
{
  ULONG retval;

  _asm {
	 mov     ax, WORD PTR number
    sub     dx,dx
    shl     ax,1
    rcl     dx,1
    shl     ax,1
    rcl     dx,1
    shl     ax,1
    rcl     dx,1
    shl     ax,1
    rcl     dx,1
    mov     WORD PTR retval, ax
    mov     WORD PTR retval+2, dx
    }
  return retval;
}

/**********************************************************************/
static ULONG near _cdecl MapPhysical(ULONG physical, ULONG limit )
{
  ULONG linear;

  _asm {
    mov ax, 0800h;                /* DPMI service 0800 = map physical */
  
    mov cx, word ptr physical;    /* physical addr in bx:cx */
    mov bx, word ptr physical+2;

	 mov di, word ptr limit;       /* limit in si:di */
    mov si, word ptr limit+2;

    int 31h;                      /* Do DPMI call */
    jc  hexit;                    /* Carry set indicates error */

    mov word ptr linear, cx;      /* return linear */
    mov word ptr linear+2, bx;

    xor ax, ax;
    }
  hexit:
  return linear;
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
/*======================================================================*/
static selector_t near _cdecl AllocateDescriptor(void)
{
  USHORT tempSel;

  _asm {
    mov ax, 0000h;
    mov cx, 1;                       /* allocate only 1 descriptor */
    int 31h;                         /* Do DPMI call */
    jc  fail;                        /* Carry set indicates error */

    mov word ptr tempSel, ax;
    xor ax, ax;
    jmp short hexit;
    }
fail:
    _asm mov ax, 1;
hexit:
  return (selector_t)(tempSel);
}

/*======================================================================*/
/* Function: Free Descriptor                                            */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  This function returns the specified descriptor to the DPMI host.    */
/*                                                                      */
/*======================================================================*/
static int near _cdecl FreeDescriptor(selector_t sel)
{
  int retval;

  _asm  {
    mov ax, 0001h;
    mov bx, sel;
    int 31h;
    jc  hexit;
    xor ax, ax;
    }
hexit:
  _asm mov retval, ax;
  return retval;
}

/*======================================================================*/
/* Function: Set Segment Base                                           */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  This function sets the base of the segment specified by the sel     */
/*  argument to the value of the base argument.                         */
/*                                                                      */
/*======================================================================*/
static int near _cdecl SetSegmentBase( selector_t sel, ULONG base)
{
  int retval;

  _asm {
    mov ax, 0007h;
    mov bx, sel;
    mov dx, word ptr base;
    mov cx, word ptr base+2;
 
    int 31h;
    jc  hexit;
 
    xor ax, ax;
    }
hexit:
    _asm mov retval, ax;
  return retval;
}

/*======================================================================*/
/* Function: Set Segment Limit                                          */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  This function sets the limit of the segment specified by the        */
/*  sel argument to the value given by the limit argument.              */
/*                                                                      */
/*======================================================================*/
static int near _cdecl SetSegmentLimit(selector_t sel, ULONG limit)
{
  int retval;

  _asm {
    mov ax, 0008h;
    mov bx, sel;
    mov dx, word ptr limit;
    mov cx, word ptr limit+2;
    int 31h;
    jc  hexit;
    xor ax, ax;
    }
hexit:
    _asm mov retval, ax;
  return retval;
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


/**************************************************************************/
/*                                                                        */
/* Retrieve data from the controller board memory                         */
/* Mininum count is 2 so be careful!                                      */
/* physical is a 32 bit offset to add to the 32bit address of the OMA4    */
/* data is a pointer to a low memory buffer                               */
/*                                                                        */
/**************************************************************************/
static void near _cdecl move_board_data(ULONG physical, void far * data,
													 USHORT count, int direction)
{
  ULONG err, limit = 0x200000L;  /* size of area addressed */
  selector_t high_sel;          /* selector for extended memory address */
  UCHAR arb = '0',
		  arb386 = '0';
  char far * phimem;

  if (count < 2) count = 2;        /* movsw used, so byte count min = 2 */

  high_sel = AllocateDescriptor();
  SetSegmentBase(high_sel, board_linear);
  SetSegmentLimit(high_sel, shift_right_12(limit));
  setup_access(limit, &arb, &arb386);
  SetSegmentAttributes(high_sel, arb, arb386);

  phimem= MK_FP(high_sel, physical);
  if (direction==WRITE_HIGH)
	 {
	 err = memcpy(phimem, data, count);
	 }
  else
	 {
	 err = memcpy(data, phimem, count);
	 }

  FreeDescriptor(high_sel);

//  printf("counts %d\n",err);
}

void far read_board_data(ULONG physical, void far * data, USHORT count)
{
  move_board_data(physical, data, count, READ_HIGH);
}

void far write_board_data(ULONG physical, void far * data, USHORT count)
{
  move_board_data(physical, data, count, WRITE_HIGH);
}

static DPMI_INFO far global_info;

DPMI_INFO far get_DPMI_info(void)
{
  return global_info;
}

int far init_DPMI(ULONG board_physical)
{
  board_linear = MapPhysical(board_physical, board_limit);
  return 0;
}
