/***************************************************************************/
/*                                                                         */
/* File name  : VESADRVR                                                   */
/* Author     : David DiPrato                                              */
/* Version    : 1.00 - Initial version.                                    */
/* Description: This module will contain all the functions required to     */
/*    control graphics using the VESA driver.                              */
/*                                                                         */
/***************************************************************************/

#include <dos.h>
#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#ifdef _MSC_VER
#define farmalloc(a) _fmalloc(a)
#define farfree(a) _ffree(a)
#define MK_FP(seg, offset) (void _far *)(((ULONG)seg << 16) \
    + (ULONG)(unsigned)offset)
#endif

#ifdef __WATCOMC__
const unsigned short DOS_SEG = 0x34;
#endif

#include "primtype.h"
#include "vesadrvr.h"
#include "rmint86.h"

#define VGAHANDLER   0x10  /* interrupt service for video calls.            */

#ifdef __WATCOMC__
typedef unsigned realptr;
#else
typedef far * realptr;
#endif

/* Structure used in function 00h, Return Super VGA Information -----------*/
typedef struct _VgaInfoBlockType
  {  
  char VESASignature[4];     /* Should contain string 'VESA'.       */
  USHORT VESAVersion;        /* Must be version 1.1 or higher.      */
  realptr OEMStringPtr;      /* Pointer toward OEM string.          */
  char Capabilities[4];      /* Reserved at this revision.          */
  realptr VideoModePtr;
  USHORT TotalMemory;        /* Number of 64kb blocks on VGA bd.    */
  char reserved[242];        /* Pad to 256 bytes.                   */
  } VGAINFO;

/* Structure used in function 01h, Return Super VGA mode information ------*/
typedef struct _ModeInfoBlockType {

/* mandatory information */
  USHORT ModeAttributes;
  char WinAAttributes;
  char WinBAttributes;
  USHORT WinGranularity;
  USHORT WinSize;
  USHORT WinASegment;
  USHORT WinBSegment;
  realptr WinFuncPtr;
  USHORT BytesPerScanLine;

/* optional information */
  USHORT  XResolution;
  USHORT  YResolution;
  char  XCharSize;
  char  YCharSize;
  char  NumberOfPlanes;
  char  BitsPerPixel;
  char  NumberOfBanks;
  char  MemoryModel;
  char  BankSize;
  char  NumImgPgs;
  char  reserved1;
  char  reserved[225];              /* Pad to 256  */
  } MODEINFO;

VGAINFO far *pVinfo;
MODEINFO far *pMinfo;
USHORT OldVideoMode,
       far * ModeNumberPtr,
       RealBufSeg;
void far *WinFuncAddr;

/* Values for AL when AH is 0x4F ------------------------------------------*/
#define GETSVGAINFO  0     /* Get Super VGA Info                           */
#define GETMODEINFO  1     /* Get Super VGA Mode Info                      */
#define SETMODE      2     /* Get Super VGA mode #                         */
#define GETMODE      3     /* Set Super VGA mode #                         */
#define SAVRESVGASTA 4     /* Save/restore super video state               */
#define SETGETWINDOW 5     /* Set/Get Video memory window                  */
#define SETGETLOGLIN 6     /* Set/Get Logical scan line                    */
#define SETGETDSTART 7     /* Set/Get Display start                        */


// For all calls, set AH = 0x4F before call
// 
// GETSUPERINFO   AL = 00; ES:DI = ptr to info buffer
// 
// GETMODEINFO    AL = 01; CX = SVGA mode number; ES:DI = ptr to info buffer
// 
// SETMODE        AL = 02; BX:D14-D0 = mode number; BX:D15 = clear vidmem flag
// 
// GETMODE        AL = 03; BX = Current mode number (output)
// 
// SAVRESVIDSTA   AL = 04; DL = 0 (get size) 1 (save state) 2 (restore state)
//                ES:BX=ptr to buffer BX=# of 64byte blks in buffer (return)
//                CX:D0=Vid H/W CX:D1=Vid BIOS CX:D2=Vid DAC CX:D3=SVGA
// 
// SETGETVWINDO   AL = 05; BH = 0 (select window) 1 (get window)
//                BL = 00 (window A) 01 (window B)
//                DX = window position in memory in granularity units
//                Can also use winfunc * returned by subfunction 01
// 
// SETGETLOGLIN   AL = 06; BL = 00 (set length) 01 (get length)
//                BX = Bytes per scan line (output)
//                CX = Desired/Actual width in pixels (input/output)
//                DX = Maximum number of scan lines (output)
// 
// SETGETDSPST    AL = 07; BH = 0; BL = 0 (Set disp start) 1 (get disp start)
//                CX = 1st disp pixel in scan line DX = 1st disp scan line


/* Function follow: ********************************************************/

/* Get Super VGA information by filling the global structure.  It will
   return a non-zero on error.
****************************************************************************/
SHORT get_VESA_information(void)
{
  union  REGS reg;
  struct SREGS sreg;

  segread(&sreg);               /* Get current segment registers.      */
  reg.x.ax = 0x4F00;            /* VBIOS = 4f                          */
#ifdef __WATCOMC__
  sreg.es = RealBufSeg;         /* Get Real Mode addr of structure.    */
  reg.x.di = 0;
#else
  sreg.es = FP_SEG(pVinfo);     /* Get pointer to structure.           */
  reg.x.di = FP_OFF(pVinfo);
#endif

  /* Call interrupt and test for success ------------------------------*/
  int86x(VGAHANDLER,&reg,&reg,&sreg);  /* get VESA support info        */
  if (reg.h.al != 0x4F) return(-1);
  if (reg.h.ah != 0x00) return(-2);

  /* Verify VESA driver responded -------------------------------------*/
  if ((pVinfo->VESASignature[0] != 'V') ||
      (pVinfo->VESASignature[1] != 'E') ||
      (pVinfo->VESASignature[2] != 'S') ||
      (pVinfo->VESASignature[3] != 'A'))
    return(-3);

  return(0);
}


/* Get Super VGA mode information by filling the global structure.  It will
   return a non-zero on error.
****************************************************************************/
SHORT get_VESA_mode_information(USHORT Mode)
{
  union  REGS reg;
  struct SREGS sreg;

  segread(&sreg);                 /* Get current segment registers.      */
  reg.x.ax = 0x4F01;
  reg.x.cx = Mode;
#ifdef __WATCOMC__
  sreg.es = RealBufSeg;           /* Get Real Mode addr of structure.    */
  reg.x.di = sizeof(VGAINFO);
#else
  sreg.es = FP_SEG(pMinfo);       /* Set pointer toward structure.       */
  reg.x.di = FP_OFF(pMinfo);
#endif

  /* Call interrupt and test for success ---------------------------------*/
  int86x(VGAHANDLER, &reg, &reg, &sreg);  /* get VESA support info           */
  if (reg.h.al != 0x4F) return(-1);
  if (reg.h.ah != 0x00) return(-2);

  return(0);
}


/* set the address of the video memory segment mapped to a000:0000
   this function only guarantees write access to the segment
****************************************************************************/
void set_VESA_window(USHORT win_no)
{
  union  REGS reg;

  /* This function can be faster by using the function call in MODEINFO.  */   
  reg.x.ax = 0x4F05;                  /* VBIOS = 4f Get/Set Win info = 05 */
  reg.x.bx = 0x0000;                  /* BH=0=set Info, BL=0=Window A     */
  reg.x.dx = win_no;
  int86(VGAHANDLER, &reg, &reg);
}


/* get the address of the video memory segment mapped to a000:0000
****************************************************************************/
USHORT get_VESA_window(void)
{
  union  REGS reg;

  reg.x.ax = 0x4F05;                  /* VBIOS = 4f Get/Set Win info = 05 */
  reg.x.bx = 0x0100;                  /* BH=1=get Info, BL=0=Window A     */
  int86(VGAHANDLER, &reg, &reg);
  return(reg.x.dx);
}


/* set the SVGA 'mode'
****************************************************************************/
void set_VESA_mode(USHORT Mode)
{
  union  REGS reg;

  reg.x.ax = 0x4F02;                  /* VBIOS = 4f Set VESA mode = 02    */
  reg.x.bx = Mode; /* | 0x8000; */    /* BX = video mode bx:15=1 = Noclear*/
  int86(VGAHANDLER, &reg, &reg);
}


/* get the current SVGA mode
****************************************************************************/
USHORT get_VESA_mode(void)
{
  union  REGS reg;

  reg.x.ax = 0x4F03;                  /* VBIOS = 4f Get VESA mode = 03 */
  int86(VGAHANDLER, &reg, &reg);
  return(reg.x.bx);
}

/* Opens the VESA graphics system by allocating memory, verifing the
   VESA driver is present and setting the given video mode.  It will save
   the current video mode for restore when the system is closed. A non-zero
   will return on error.
****************************************************************************/
SHORT OpenVESASystem(USHORT Mode)
{
  SHORT i, Success = -1;

  /* Allocate information buffers ----------------------------------------*/

#ifdef __WATCOMC__
  RealBufSeg = alloc_conventional((sizeof(VGAINFO) + sizeof(MODEINFO))*2);
  if (!RealBufSeg)
    {
    printf("Could not allocate real mode buffer for VESA call\n");
    return(-1);
    }
  MK_FARPTR(pVinfo, DOS_SEG, RealBufSeg << 4);
  MK_FARPTR(pMinfo, DOS_SEG, (RealBufSeg << 4) + sizeof(VGAINFO));
#else
  if ((pVinfo = farmalloc(sizeof(VGAINFO))) == NULL)
    return(-1);
  if ((pMinfo = farmalloc(sizeof(MODEINFO))) == NULL)
    return(-1);
#endif

  /* Save video mode for restore. ----------------------------------------*/
  OldVideoMode = get_VESA_mode();

  /* Setup video mode ----------------------------------------------------*/
  if (get_VESA_information())            /* Fill SVGA info struct. */
    return -2;


  /* see if the requested VESA mode is supported by this configuration    */
  /* if WATCOM, make 48 bit far ptr from DOS segment:offset the nasty way */

#ifdef __WATCOMC__
  {
  unsigned temp = (((pVinfo->VideoModePtr & 0xFFFF0000L) >> 12) +
                   (pVinfo->VideoModePtr & 0xFFFFL));

  ModeNumberPtr = MK_FP(DOS_SEG, temp);
  }
#else
  ModeNumberPtr = (USHORT *)(pVinfo->VideoModePtr);
#endif

  for (i = 0;ModeNumberPtr[i] != 0xFFFF;i++)
    {
    if (ModeNumberPtr[i] == Mode)
      {
      Success = i;
      break;
      }
    }
  if (Success < 0) return(-3);           /* Video 'Mode' not supported.   */

  if (get_VESA_mode_information(Mode))   /* Fill Mode info structure.     */
     return(-4);

  /* Check for mode supportted in hardware and extension -----------------*/
  if (!(pMinfo->ModeAttributes & 0x02)) return(-3);
  if (!(pMinfo->WinAAttributes & 0x01)) return(-4);

  /* Set video new video mode --------------------------------------------*/
  set_VESA_mode(Mode);


  return(0);
}

/* Closes the VESA graphics system by releasing any memory allocated and
   restore the initial video mode.
****************************************************************************/
void CloseVESASystem(void)
{
  /* Restore old video mode ----------------------------------------------*/
  set_VESA_mode(OldVideoMode);

#ifndef __WATCOMC__
  /* Release information buffers -----------------------------------------*/
  farfree(pVinfo);
  farfree(pMinfo);
#else
  free_conventional(RealBufSeg);
#endif
}

   
/* This function will create a far pointer toward video memory.
****************************************************************************/
char far *CreateVideoPtr(void)
{
  #ifdef __WATCOMC__
  char far * retptr ;

  MK_FARPTR(retptr, DOS_SEG, (unsigned)pMinfo->WinASegment << 4);
  return retptr;

  #else

  return(MK_FP(pMinfo->WinASegment,0));

  #endif
}

/* This function will return the number of bytes for each video segment
   unit. 
****************************************************************************/
LONG GetVideoSegSize(void)
{
  return((long)pMinfo->WinGranularity * 1024L);
}

/* This function will get the number of bytes per video line in the
   video memory.
****************************************************************************/
SHORT GetBytesPerLine(void)
{
  return(pMinfo->BytesPerScanLine);
}


