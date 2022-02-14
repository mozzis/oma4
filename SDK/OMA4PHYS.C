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
#include "oma4phys.h"

/* basic typedefs */

union reg     /* single long register */
{ ULONG l;
  USHORT s;
};

/* structure to pass state of registers for raw mode switch */

struct rawModeRegs_t
  {
  USHORT rawDS;
  USHORT rawES;
  USHORT rawSS;
  USHORT rawCS;
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

static struct rawModeRegs_t near realRegs;
static struct rawModeRegs_t near protRegs;

static DPMICALL far * near realToProt;
static DPMICALL far * near protToReal;

static DPMICALL far * near saveRealState;
static DPMICALL far * near saveProtState;
static USHORT state_size;
static ULONG saveBase, saveHandle;

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

/*======================================================================*/
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  This call determines if a DPMI host is present, and if so, returns  */
/*  necessary information on how to invoke it.  The function returns    */
/*  zero if a DPMI host was found; otherwise it returns non-zero.       */
/*                                                                      */
/*  All arguments are pointers to locations to store return values.     */
/*  The flags argument is non-zero if the DPMI host supports 32-bit     */
/*  applications.  The processor argument is written as 2 (80286),      */
/*  3 (80386), 4 (80486), or 5 (future). The version arguments indicate */
/*  the version numbers of the DPMI host (For DPMI 0.9, major version is*/
/*  zero, and minor version is 90).  The nPrivateDataParas argument     */
/*  receives the number of paragraphs of memory that the host requires  */
/*  for each client.  If this is non-zero, the client must provide      */
/*  this much memory prior to calling the mode switch entry point, and  */
/*  the memory must be paragraph aligned.                               */
/*                                                                      */
/*  The value written at the entryAddress argument is the far address   */
/*  to call to make the initial switch into protected mode. See also,   */
/*  DPMIEnterProtectedMode.                                             */
/*                                                                      */
/* DPMI reference: INT 2Fh AX=1687h                                     */
/*======================================================================*/

static int near _cdecl ObtainSwitchEntryPoint(DPMI_INFO far * info_block)
  {
  int retcode;

  _asm {
    mov ax, 1687h;    /* test for presence of DPMI host */
    int 2Fh;

    or  ax, ax;       /* ax == 0 means host responded */
    jnz noHost;

    mov ax, bx;       /* save flags in ax */
    push es;

    les bx, dword ptr info_block;   /* !!!! */
    mov word ptr es:[bx].pflags, ax;
    mov byte ptr es:[bx].processor, cl;
    pop cx;
    mov byte ptr es:[bx].majorVersion, dh;
    mov byte ptr es:[bx].minorVersion, dl;
    mov word ptr es:[bx].nPrivateDataParas, si;

//    mov word ptr es:[bx].entryAddress, di;
//    mov word ptr es:[bx].entryAddress+2, cx;
    
    mov word ptr es:[bx+7], di;
    mov word ptr es:[bx+9], cx;
    
    xor ax, ax
    }
noHost:
    _asm mov retcode, ax;
  return retcode;
}

/*======================================================================*/
/* Function: Allocate Memory Block                                      */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  This function allocates a memory block.  The nBytes argument        */
/*  specifies the desired size of the block to allocate, in bytes.      */
/*  The linear base address of the block that is allocated is           */
/*  written to the address pointed to by the base argument.  The        */
/*  block handle is written to the address pointed to by the handle     */
/*  argument.  The handle is used to free or resize the block.          */
/*                                                                      */
/* DPMI reference: INT 31h AX=0501h                                     */
/*                                                                      */
/* Version 1.0 Error returns:                                           */
/*                                                                      */
/*  8012h linear memory unavailable                                     */
/*  8013h physical memory unavailable                                   */
/*  8014h backing store unavailable                                     */
/*  8016h handle unavailable                                            */
/*  8021h invalid value                                                 */
/*======================================================================*/
static int near _cdecl DPMIAllocateMemory(ULONG nBytes, ULONG *base,
                                          ULONG *handle)
{
  int retval;

  _asm {
    mov ax, 0501h
    mov bx, word ptr nBytes;
    mov cx, word ptr nBytes+2;
    int 31h;
    jc  hexit;

    mov ax, bx;
    les bx, base;
    mov es:[bx], ax;
    mov es:[bx+2], cx;
    les bx, handle;
    mov es:[bx], si;
    mov es:[bx+2], di;
    xor ax, ax
    }
  hexit:
    _asm mov retval, ax;
  return retval;
}

/*======================================================================*/
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  This function retrieves information from the DPMI host that         */
/*  is required for raw mode switching.  The address (paragraph:offset) */
/*  of the real to protected mode service is stored at the address      */
/*  pointed to by the realToProt argument.  The address (selector:offset*/
/*  of the protected to real mode service is stored at the address      */
/*  pointed to by the protToReal argument.                              */
/*                                                                      */
/* DPMI reference: INT 31h AX=0306h                                     */
/*======================================================================*/  

static void near _cdecl GetRawSwitchProc(DPMICALL far * near * realToProtEntry,
                                         DPMICALL far * near * protToRealEntry)
{
  DPMICALL far * real_to_prot;
  DPMICALL far * prot_to_real;

  _asm  {                      /* get raw switch mode addresses */
    mov ax,0306h;
    int 31h;

    mov word ptr real_to_prot, cx;
    mov word ptr real_to_prot+2, bx;
    mov word ptr prot_to_real, di;
    mov word ptr prot_to_real+2, si;
    }

  *realToProtEntry = real_to_prot;
  *protToRealEntry = prot_to_real;
}

/*======================================================================*/
/* Function: Get State Save/Restore Procedure Addresses                 */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  This function retrieves information from the DPMI host that         */
/*  is required for saving and restoring the client state when          */
/*  using the raw mode switch function.                                 */
/*                                                                      */
/*  The DPMI specification requires that clients save their state       */
/*  prior to making a raw mode switch, and subsequently restore it.     */
/*  This function provides the size of the state information, and       */
/*  both the protected mode and the real mode addresses of functions    */
/*  to call to save and restore the client state.                       */
/*                                                                      */
/*  The size of the state information, in bytes, is stored at the       */
/*  address pointed to by the stateSize argument.  The address of the   */
/*  far procedure to call from real mode to save or restore the client  */
/*  state is stored at the address pointed to by the realProc argument. */
/*  The address of the far procedure to call from protected mode to     */
/*  save and restore the client state is stored at the address pointed  */
/*  to by the protProc argument.                                        */
/*                                                                      */
/*  The state save/restore procedures are called with AL=0 to save the  */
/*  state, and AL=1 to restore the state.  In both cases, ES:DI points  */
/*  to the state information buffer.                                    */
/*                                                                      */
/*  DPMI reference: INT 31h AX=0305h                                    */
/*======================================================================*/

static void near _cdecl GetStateSaveRestoreProcs(USHORT far *stateSize,
                                                 DPMICALL far * near * realProc,
                                                 DPMICALL far * near * protProc)
{
  USHORT save_size;
  DPMICALL far * real_proc;
  DPMICALL far * prot_proc;

  _asm {
    mov ax, 0305h;
    int 31h;

    mov save_size, ax;
    
    mov word ptr real_proc, cx;
    mov word ptr real_proc+2, bx;

    mov word ptr prot_proc, di;
    mov word ptr prot_proc+2, si;
    }

  *stateSize = save_size;

  DPMIAllocateMemory((ULONG)save_size, &saveBase, &saveHandle);

  *realProc = real_proc;
  *protProc = prot_proc;
}

/*======================================================================*/
/* Function: Do Raw Mode Switch                                         */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*  This function invokes the raw switch service.  The switchAddr       */
/*  argument is obtained from DPMIGetRawSwitchProc, and specifies       */
/*  the address of the switch service to invoke.  When the function     */
/*  returns, the processor will be in the alternate mode.               */
/*                                                                      */
/*  The rawRegs argument points to the structure containing the register*/ 
/*  contents to be put in effect after the mode switch is effected.     */
/*  The values in the structure MUST be aliases for the currently active*/
/*  segment registers.  In other words, when switching from protected   */
/*  to real mode, each segment register value in the structure must     */
/*  be the paragraph equivalent of the current protected mode descriptor*/
/*  value.  Likewise, when switching from real to protected mode, each  */
/*  segment register value in the structure must be the selector that   */
/*  maps the current paragraph in each segment register. These          */
/*  requirements imply that this routine must execute only in the low   */
/*  1 MB of the address space, and that the stack must also be in the   */
/*  low 1 MB.                                                           */
/*                                                                      */
/* DPMI reference: invoking the raw mode switch services                */
/*======================================================================*/

static void near _cdecl DoRawSwitch(DPMICALL far * switchAddr,
                                    struct rawModeRegs_t far *rawRegs)
{
  _asm {
    les di, rawRegs
    mov ax, es:[di].rawDS
    mov cx, es:[di].rawES
    mov dx, es:[di].rawSS
    mov bx, sp
    mov si, es:[di].rawCS
    mov di, offset finis
    push  word ptr switchAddr+2  /* push switch address and retf to it */
    push  word ptr switchAddr
    retf
  finis:
    }
}

/***********************************************************************/
/*  If the call returns zero, the client is then running in protected  */
/*  mode.  The segment registers CS, DS, and SS contain selectors that */
/*  address the same linear memory as they did in real mode at the     */
/*  time of the call.  The selector in ES addresses the program's PSP, */
/*  and the corresponding descriptor has a limit of 0xFF.  If running  */
/*  on a 32-bit processor, FS and GS are zero.                         */
/*                                                                     */
/* DPMI reference: calling the mode switch entry point                 */
/*                                                                     */
/* Version 1.0 Error returns:                                          */
/*                                                                     */
/*  8011h descriptor unavailable                                       */
/*  8021h 32-bit clients not supported                                 */
/***********************************************************************/
static int near _cdecl EnterProtectedMode(DPMICALL far *switchEntryPoint,
                       USHORT bitness,           /*  16 or 32 */
                       USHORT nPrivateDataParas) /* number required by host */

{
  int retval;

  _asm {
    mov ah, 48h             /* get the required memory block */
    mov bx, WORD PTR nPrivateDataParas
    int 21h                 /* returns seg addr of block in ax */
    jc  fail                /* if carry set, returns #paras free in bx */

                            /* set up and call the mode switch */
    mov es, ax              /* put segment of data area in es */
    mov ax, WORD PTR bitness
    call  DWORD PTR switchEntryPoint
    jc  fail
  
    xor ax, ax
    jmp short hexit
    }
fail:
    _asm mov ax, 1
hexit:
    _asm mov WORD PTR retval, ax;
  return retval;
}

/**************************************************************************/
/*  int DPMIGetCPUMode(void);                                             */
/*                                                                        */
/* Description:                                                           */
/*                                                                        */
/*  This function may be used by mode sensitive applications to determine */
/*  the current processor operating mode. Clients must verify the         */
/*  presence of a DPMI host prior to making this call. The function       */
/*  returns zero if executing in protected mode, and returns non-zero     */
/*  if running in real or virtual 86 mode.                                */
/*                                                                        */
/* DPMI reference: INT 2Fh AX=1686h                                       */
/**************************************************************************/

static int near _cdecl GetCPUMode(void)
{
  int retval;

  _asm {
    mov ax, 1686h;
    int 2Fh;
    mov retval, ax;
    }
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
  ULONG linear, limit = 0x200000L; /* size of area addressed */
  selector_t high_sel,             /* selector for extended memory address */
             low_sel;              /* selector for low memory address */
  UCHAR arb = '0',
        arb386 = '0';
  USHORT far *dest_ptr,            /* pointers based on LDT selectors */
         far *src_ptr;

  if (count < 2) count = 2;        /* movsw used, so byte count min = 2 */

  linear = board_linear + physical;
  
  _disable();

  DoRawSwitch(realToProt, &protRegs); /* go into protected mode */

  high_sel = AllocateDescriptor();
  SetSegmentBase(high_sel, linear);
  SetSegmentLimit(high_sel,shift_right_12(limit));
  setup_access(limit, &arb, &arb386);
  SetSegmentAttributes(high_sel, arb, arb386);

  low_sel = AllocateDescriptor();
  SetSegmentBase(low_sel, shift_left_4(FP_SEG(data)) + (ULONG)FP_OFF(data));
  SetSegmentLimit(low_sel,(ULONG)count);
  setup_access(count, &arb, &arb386);
  SetSegmentAttributes(low_sel, arb, arb386);

  if (direction)                   /* direction == 1 == write */
    {
    dest_ptr = MK_FP(high_sel, 0); /* turn descriptors into pointers */
    src_ptr  = MK_FP(low_sel, 0);
    }
  else                             /* direction == 0 == read */
    {
    src_ptr  = MK_FP(high_sel, 0); /* turn descriptors into pointers */
    dest_ptr = MK_FP(low_sel, 0);
    }

  /* move the data */
  _asm {                    /* don't do library call (memcpy) in prot mode! */
    mov cx, count;
    shr cx, 1;              /* note count divided by two here */
    
    push ds;
    push es;
    push si;
    push di;

    lds si, dword ptr src_ptr;  /* load source in ds:si */
    les di, dword ptr dest_ptr; /* load dest in es:di */
    cld;                        /* increment addresses during loop */
    rep movsw;                  /* move the data */

    pop di;
    pop si;
    pop es;
    pop ds;
    }

  FreeDescriptor(low_sel);
  FreeDescriptor(high_sel);
  DoRawSwitch(protToReal, &realRegs);
  _enable();
}

void far read_board_data(ULONG physical, void far * data, USHORT count)
{
  move_board_data(physical, data, count, READ_HIGH);
}

void far write_board_data(ULONG physical, void far * data, USHORT count)
{
  move_board_data(physical, data, count, WRITE_HIGH);
}

/**************************************************************/
/*                                                            */
/* terminate the program                                      */
/* Must use DPMI service 4c to exit, so host can clean up     */
/* DPMI must be called in protected mode so does switch first */
/*                                                            */
/**************************************************************/
void far DPMI_Terminate(void)
{
  DoRawSwitch(realToProt, &protRegs);
  _asm {
    mov ax,0x4c00;
    int 0x21;
    }
}

static DPMI_INFO far global_info;

DPMI_INFO far get_DPMI_info(void)
{
  return global_info;
}

/******************************************************/
/*                                                    */
/* Look for a DPMI host; if found, get data needed to */
/* make the initial entry into protected mode.        */
/*                                                    */
/******************************************************/
static int near DPMI_init(ULONG board_physical)
{ 
  int dpmiStat;    // status return
  DPMI_INFO info;

  dpmiStat = ObtainSwitchEntryPoint((DPMI_INFO far *)&info);
  if (dpmiStat)
    return(dpmiStat);

  global_info = info;

  _asm {   /* Save real mode registers for later Prot->Real mode switch */
    mov realRegs.rawDS, ds;
    mov realRegs.rawES, es;
    mov realRegs.rawSS, ss;
    mov realRegs.rawCS, cs;
    }

 /* Calling DPMIEnterProtectedMode switches the processor to protected mode. */
 
  if(EnterProtectedMode(info.entryAddress, 16, info.nPrivateDataParas))
    return(1);     /* couldn't jump to protected mode */
 
  if (GetCPUMode())
    return(1);     /* still in real mode */

  _asm {    /* Save prot mode registers */
    mov protRegs.rawDS, ds;
    mov protRegs.rawES, es;
    mov protRegs.rawSS, ss;
    mov protRegs.rawCS, cs;
    }
 
 /* change physical address of OMA4 to linear address we can use */
 
  board_linear = MapPhysical(board_physical, board_limit);
 
  GetRawSwitchProc(&realToProt, &protToReal);
  DoRawSwitch(protToReal, &realRegs);         /* back to real mode */

  return(0);
}

int far init_DPMI(ULONG board_physical)
{
  return DPMI_init(board_physical);
}
