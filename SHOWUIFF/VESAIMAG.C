/***************************************************************************/
/*                                                                         */
/* File name  : IMAGE                                                      */
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
#include <math.h>
#include <malloc.h>

#include "vesadrvr.h"
#include "rmint86.h"
#include "uiff.h"

#ifdef _MSC_VER
#define farmalloc(a) _fmalloc(a)
#define farfree(a) _ffree(a)
#define MK_FP(seg, offset) (void _far *)(((ULONG)seg << 16) \
    + (ULONG)(USHORT)offset)
#endif

#define DOS_SEG 0x34   /* Magic cookie for low memory access */

/* Palette definitions ----------------------------------------------------*/
#define DISPLAY_BIOS    0x10        /* BIOS service for the display driver */
#define SET_PALETTE     0x10        /* - Palette video BIOS service.       */
#define SUM_COLORS      0x1b        /*   .Sum RGB colors to gray scale.    */
#define SET_COLORS      0x10        /*   .Sets a color to the 3 registers  */
#define READ_COLORS     0x15        /*   .Reads a color from the 3 regs.   */
#define SET_BLK_COLORS  0x12        /*   .Writes a color block.            */
#define READ_BLK_COLORS 0x17        /*   .Reads a color block.             */

#define VIDEO_MODE 0x101
#define NUM_COLORS      64
#define MIN_COLOR_VALUE 64
#define MAX_COLOR_VALUE (NUM_COLORS+MIN_COLOR_VALUE-1)
#define MAX_DISPLAY_X   630
#define MAX_DISPLAY_Y   460

#define MIN_DISPLAY_X   10
#define MIN_DISPLAY_Y   10

#define EGABLACK        0
#define EGABLUE         1
#define EGAGREEN        2
#define EGACYAN         3
#define EGARED          4
#define EGAMAGENTA      5
#define EGABROWN        6
#define EGALGRAY        7
#define EGAGRAY         8
#define EGALBLUE        9
#define EGALGREEN       10
#define EGALCYAN        11
#define EGALRED         12
#define EGALMAGENTA     13
#define EGAYELLOW       14
#define EGAWHITE        15

/* VESA information block(s) ----------------------------------------------*/
typedef struct                    /* Detector specific information.      */
  {
  SHORT NumTracks;
  SHORT NumPoints;
  } IMAGE_DET_INFO;

typedef struct                    /* Display drawing information.        */
  {
  SHORT XLeft;
  SHORT XRight;
  SHORT YTop;
  SHORT YBottom;
  double XScaling;
  double YScaling;
  double IntensityScaling;
  double IntensityOffset;
  SHORT  ImageDisplayedFlag;
  } IMAGE_DRAW_INFO;

typedef struct
  {
  char r;
  char g;
  char b;
  } PALETTE_REC;


IMAGE_DET_INFO ImageDetInfo;
IMAGE_DRAW_INFO ImageDrawInfo;

char far *DisplayPtr;               /* Pointer toward video memory.        */

/* Holds old pallete for restore. */
/* add one slot for overscan value */
PALETTE_REC   OldPal[NUM_COLORS+1];
PALETTE_REC far * OldPalette;

BOOLEAN EraseNeeded = FALSE;

/*  save the current Palette.
****************************************************************************/
void SavePalette(void)
{
  struct SREGS insregs;
  union REGS inregs;

  segread(&insregs);
  inregs.h.ah = SET_PALETTE;
  inregs.h.al = READ_BLK_COLORS;
  insregs.es = FP_SEG(OldPalette);
  inregs.x.dx = FP_OFF(OldPalette);
  inregs.x.cx = NUM_COLORS;           /* Number of colors to set.         */
  inregs.x.bx = 64;                   /* Starting address.                 */
#ifdef __WATCOMC__
  int386x(DISPLAY_BIOS, &inregs, &inregs, &insregs);
#else
  int86x(DISPLAY_BIOS, &inregs, &inregs, &insregs);
#endif
}

/*  restore the Palete saved with function 'SavePalette'.
****************************************************************************/
void RestorePalette(void)
{
  struct SREGS insregs;
  union REGS inregs;

  segread(&insregs);
  inregs.h.ah = SET_PALETTE;
  inregs.h.al = SET_BLK_COLORS;
  insregs.es = FP_SEG(OldPalette);
  inregs.x.dx = FP_OFF(OldPalette);
  inregs.x.cx = NUM_COLORS;           /* Number of colors to set.         */
  inregs.x.bx = 64;                   /* Starting address.                */
#ifdef __WATCOMC__
  int386x(DISPLAY_BIOS, &inregs, &inregs, &insregs);
#else
  int86x(DISPLAY_BIOS, &inregs, &inregs, &insregs);
#endif
}

/* sets the video palette to 64 level of gray scale.  Values 64 to 128 
   are used.  It will return a non-zero on error.
****************************************************************************/
SHORT SetPalette(void)
{
  struct SREGS insregs;
  union REGS inregs;
  PALETTE_REC *Pallete; 
  SHORT i;

  /* Allocate memory for pallete. ----------------------------------------*/
  if ((Pallete = malloc(sizeof(PALETTE_REC) * (NUM_COLORS + 1))) == NULL)
     return(-1);

  /* Setup pallete to 64 shades of gray. ---------------------------------*/
  for (i = 0;i < NUM_COLORS;i++)
    {
    Pallete[i].r = (char)i;
    Pallete[i].g = (char)i;
    Pallete[i].b = (char)i;
    }

  /* Update pallete ------------------------------------------------------*/
  segread(&insregs);
  inregs.h.ah = SET_PALETTE;
  inregs.h.al = SET_BLK_COLORS;
  insregs.es = FP_SEG(Pallete);
  inregs.x.dx = FP_OFF(Pallete);
  inregs.x.cx = NUM_COLORS;           /* Number of colors to set.         */
  inregs.x.bx = 64;                   /* Staring address.          */
#ifdef __WATCOMC__
  int386x(DISPLAY_BIOS, &inregs, &inregs, &insregs);
#else
  int86x(DISPLAY_BIOS, &inregs, &inregs, &insregs);
#endif
  free(Pallete);
  return(0);
}

/* This function will draw a line from the given screen locations. 
****************************************************************************/
void DrawLine(SHORT X0,SHORT Y0,SHORT X1,SHORT Y1, UCHAR Color)
{
  USHORT DisplaySeg, OrigSeg, OldSeg = 0xFFFF, DisplayIndex;
  LONG x,y, segBytes, VideoAddress;
  double dx,dy,dx2,dy2,sin,cos;

  OrigSeg = get_VESA_window();

  /* Do outside the loop calculations ------------------------------------*/
  dx = (double)(X1 - X0);
  dy = (double)(Y1 - Y0);
  dx2 = dx * dx;
  dy2 = dy * dy;
  cos = dx / sqrt(dx2 + dy2);
  sin = dy / sqrt(dx2 + dy2);
  segBytes = GetVideoSegSize();

  /* Test for bigger side; this side will not have two points with
     the same pixel value in that dimension.                              */
  if (dx > dy)
    {
    for (x = X0;x <= X1;x++)
      {
      y = (LONG)Y0 + (LONG)((double)x * sin);

      /* set video segment address for this line -----------------------*/
      VideoAddress = x + y * GetBytesPerLine();
      DisplaySeg = (USHORT)(VideoAddress / segBytes);
      if (DisplaySeg != OldSeg)
        {
        set_VESA_window(DisplaySeg);
        OldSeg = DisplaySeg;
        }
    
      /* Set the new data point ----------------------------------------*/
      DisplayIndex = (USHORT)(VideoAddress - ((LONG)DisplaySeg * segBytes));
      DisplayPtr[DisplayIndex] = Color;
      }
    }
  else
    {
    for (y = Y0;y <= Y1;y++)
      {
      x = (LONG)X0 + (LONG)((double)y * cos);

      /* set video segment address for this line -----------------------*/
      VideoAddress = x + y * GetBytesPerLine();
      DisplaySeg = (USHORT)(VideoAddress / segBytes);
      if (DisplaySeg != OldSeg)
        {
        set_VESA_window(DisplaySeg);
        OldSeg = DisplaySeg;
        }
    
      /* Set the new data point ----------------------------------------*/
      DisplayIndex = (USHORT)(VideoAddress - ((LONG)DisplaySeg * segBytes));
      DisplayPtr[DisplayIndex] = Color;
      }
    }
  set_VESA_window(OrigSeg);
}

/* Open the display system; save video mode, set 640x480 mode.  Will return
   a non-zero on error.
****************************************************************************/
SHORT OpenImageDisplay(void)
{
  if (OpenVESASystem(VIDEO_MODE))
    return(-1);
  DisplayPtr = CreateVideoPtr();

#ifdef __WATCOMC__
  OldPalette = MK_FP(FP_SEG(OldPal), FP_OFF(OldPal));
#else
  OldPalette = &OldPal[0];
#endif

  SavePalette();
  if (SetPalette())
    return(-3);

  return(0);
}

/* Close the display system; restore video mode.
****************************************************************************/
SHORT CloseImageDisplay(void)
{
  RestorePalette();
  CloseVESASystem();

  return(0);
}

/* Setup an image window at the given locations.  Assumimg video mode
   640x480 and values are physical pixels.  A non-zero will return on error.
****************************************************************************/
SHORT OpenImageWindow(SHORT XLeft,SHORT XRight,SHORT YTop,SHORT YBottom)
{
  /* Calculate scaling parameters ----------------------------------------*/
  ImageDrawInfo.XScaling = 
     (double)ImageDetInfo.NumPoints / (double)(XRight - XLeft - 1);
  ImageDrawInfo.YScaling = 
     (double)ImageDetInfo.NumTracks / (double)(YBottom - YTop - 1); 

  /* Setup window limits -------------------------------------------------*/
  ImageDrawInfo.XLeft = XLeft;
  ImageDrawInfo.XRight = XRight;
  ImageDrawInfo.YTop = YTop;
  ImageDrawInfo.YBottom = YBottom;

  ImageDrawInfo.IntensityScaling = 244e-6;    /* Display full 18 bits.    */
  ImageDrawInfo.IntensityOffset = 0;          /* Display full 18 bits.    */

  /* Draw border ---------------------------------------------------------*/   
  DrawLine(XLeft,YTop,XRight,YTop, MAX_COLOR_VALUE);
  DrawLine(XLeft,YBottom,XRight,YBottom,MAX_COLOR_VALUE);
  DrawLine(XLeft,YTop,XLeft,YBottom,MAX_COLOR_VALUE);
  DrawLine(XRight,YTop,XRight,YBottom,MAX_COLOR_VALUE);

  return(0);
}

/* Shut-down the image window.  A non-zero will return on error.
****************************************************************************/
SHORT CloseImageWindow(void)
{
  /* Erase border --------------------------------------------------------*/   
  DrawLine(ImageDrawInfo.XLeft,ImageDrawInfo.YTop,ImageDrawInfo.XRight,ImageDrawInfo.YTop,0);
  DrawLine(ImageDrawInfo.XLeft,ImageDrawInfo.YBottom,ImageDrawInfo.XRight,ImageDrawInfo.YBottom,0);
  DrawLine(ImageDrawInfo.XLeft,ImageDrawInfo.YTop,ImageDrawInfo.XLeft,ImageDrawInfo.YBottom,0);
  DrawLine(ImageDrawInfo.XRight,ImageDrawInfo.YTop,ImageDrawInfo.XRight,ImageDrawInfo.YBottom,0);
  
  return(0);
}

/* Erase the image window.  A non-zero will return on error.
****************************************************************************/

/* These functions adjust the size of the image window.  They return a 
   non zero when the limits have been reached.
****************************************************************************/
SHORT IncWindowX(void)
{
  SHORT X;

  X = ImageDrawInfo.XRight;
  if ((X += 10) > MAX_DISPLAY_X) return(-1);
  CloseImageWindow();
  EraseImageWindow();
  OpenImageWindow(ImageDrawInfo.XLeft,X,ImageDrawInfo.YTop,ImageDrawInfo.YBottom);
 
  return(0);
}

/***************************************************************************/
SHORT DecWindowX(void)
{
  SHORT X;

  X = ImageDrawInfo.XRight;
  if ((X -= 10) < MIN_DISPLAY_X + 40) return(-1);
  CloseImageWindow();
  EraseImageWindow();
  OpenImageWindow(ImageDrawInfo.XLeft,X,ImageDrawInfo.YTop,ImageDrawInfo.YBottom);

  return(0);
}

/***************************************************************************/
SHORT IncWindowY(void)
{
  SHORT Y;

  Y = ImageDrawInfo.YBottom;
  if ((Y += 10) > MAX_DISPLAY_Y) return(-1);
  CloseImageWindow();
  EraseImageWindow();
  OpenImageWindow(ImageDrawInfo.XLeft,ImageDrawInfo.XRight,ImageDrawInfo.YTop,Y);

  return(0);
}

/***************************************************************************/
SHORT DecWindowY(void)
{
  SHORT Y;

  Y = ImageDrawInfo.YBottom;
  if ((Y -= 10) < MIN_DISPLAY_Y + 40) return(-1);
  CloseImageWindow();
  EraseImageWindow();
  OpenImageWindow(ImageDrawInfo.XLeft,ImageDrawInfo.XRight,ImageDrawInfo.YTop,Y);
  return(0);
}


/* Update the image window.  A non-zero will return on error.
****************************************************************************/
int UpdateImageWindow(UIF_HEADER *In)
{
   char Color;
   double XScaling, YScaling;
   int IntDataXIndex, IntDataYIndex;
   double FloatDataXIndex, FloatDataYIndex;
   int x,y;
   short lbytes = GetBytesPerLine();
   long VideoAddress, segBytes;
   unsigned short OldDisplaySeg = 0xffff, DisplaySeg, DisplayIndex;

   segBytes = GetVideoSegSize();
   
   /* Test for 16 bit data -------------------------------------------------*/
   if (Integer16Bit(In) == 0) {
      strcpy(ErrorMsg,"Data is not 16 bit.");
      return(NULL);
      }

   /* Get spatial scaling -------------------------------------------------*/
   XScaling = (double)In->XTotal / 
      (double)(ImageDrawInfo.XRight - ImageDrawInfo.XLeft - 1);
   YScaling = (double)In->YTotal / 
      (double)(ImageDrawInfo.YBottom - ImageDrawInfo.YTop - 1); 


   /* Draw Image ----------------------------------------------------------*/
   IntDataYIndex = 0;FloatDataYIndex = 0;                    
   for (y = ImageDrawInfo.YTop + 1;y < ImageDrawInfo.YBottom;y++) {

      IntDataXIndex = 0;FloatDataXIndex = 0;
      for (x = ImageDrawInfo.XLeft + 1; x < ImageDrawInfo.XRight;x++) {

         /* Draw display pixel --------------------------------------------*/
        if (In->FileFormat == BITS16)
         Color = (char)(((
            (double)(In->DataPtr.s[(IntDataYIndex * In->XTotal) + IntDataXIndex])
            - ImageDrawInfo.IntensityOffset) 
            * ImageDrawInfo.IntensityScaling)
            + MIN_COLOR_VALUE);
        else
         Color = (char)(((
            (double)(In->DataPtr.l[(IntDataYIndex * In->XTotal) + IntDataXIndex])
            - ImageDrawInfo.IntensityOffset) 
            * ImageDrawInfo.IntensityScaling)
            + MIN_COLOR_VALUE);

         if ((unsigned char)Color > MAX_COLOR_VALUE) Color = MAX_COLOR_VALUE;
         if (Color < MIN_COLOR_VALUE) Color = MIN_COLOR_VALUE;

         /* set video segment address for this line -----------------------*/
         VideoAddress = x + y * lbytes;
         DisplaySeg = VideoAddress / segBytes;
         if (DisplaySeg != OldDisplaySeg) {
            set_VESA_window(DisplaySeg);
            OldDisplaySeg = DisplaySeg;
            }
   
         /* Set the new data point ----------------------------------------*/
         DisplayIndex = VideoAddress - (DisplaySeg * segBytes);
         DisplayPtr[DisplayIndex] = Color;


         FloatDataXIndex += XScaling;
         IntDataYIndex = (int)FloatDataXIndex;
         }
      FloatDataYIndex += YScaling;
      IntDataYIndex = (int)FloatDataYIndex;
      }

   ImageDrawInfo.ImageDisplayedFlag = 1;
   return(0);
}

/* Erase the image window.  A non-zero will return on error.
****************************************************************************/
int EraseImageWindow(void)
{
   char Color;
   int x,y;
   short lbytes = GetBytesPerLine();
   long VideoAddress, segBytes;
   unsigned short OldDisplaySeg = 0xffff, DisplaySeg, DisplayIndex;

   segBytes = GetVideoSegSize();
   

   /* Draw Image ----------------------------------------------------------*/
   for (y = ImageDrawInfo.YTop + 1;y < ImageDrawInfo.YBottom;y++) {

      for (x = ImageDrawInfo.XLeft + 1; x < ImageDrawInfo.XRight;x++) {

         /* Draw display pixel --------------------------------------------*/
         Color = MIN_COLOR_VALUE;

         /* set video segment address for this line -----------------------*/
         VideoAddress = x + y * lbytes;
         DisplaySeg = VideoAddress / segBytes;
         if (DisplaySeg != OldDisplaySeg) {
            set_VESA_window(DisplaySeg);
            OldDisplaySeg = DisplaySeg;
            }
   
         /* Set the new data point ----------------------------------------*/
         DisplayIndex = VideoAddress - (DisplaySeg * segBytes);
         DisplayPtr[DisplayIndex] = Color;
         }
      }

   ImageDrawInfo.ImageDisplayedFlag = 0;
   return(0);
}


/* This function will auto-scale the display.
****************************************************************************/
int AutoScaleImage(UIF_HEADER *In)
{

  short Min, Max;

  FindMinMaxByArea(In,&Min,&Max);
  SetCursorToLine(2);
  printf("Min is %d and Max is %d", Min, Max);
  flushall();

  ImageDrawInfo.IntensityOffset = (double)Min;
  ImageDrawInfo.IntensityScaling = NUM_COLORS / (double)(Max - Min);

  return(0);
}


/***************************************************************************/
void SetCursorToLine(USHORT line)
{
  union REGS inregs;
  char rows;

#ifdef __WATCOMC__
  char far * rowptr;
  MK_FARPTR(rowptr, DOS_SEG, 0x484);    /* 0x34 is PharLap hardwired DOSSEG */
#else
  char far *rowptr = MK_FP(0x40, 0x84); /* point to BIOS data video rows */
#endif
  rows = *rowptr;

  inregs.x.ax = 0x0f00;                 /* get active page number in bx */
#ifdef __WATCOMC__
  int386(DISPLAY_BIOS, &inregs, &inregs);
#else
  int86(DISPLAY_BIOS, &inregs, &inregs);
#endif
  inregs.x.ax = 0x0200;                 /* set cursor to line */
  inregs.h.dh = rows - line;
  inregs.h.dl = 1;                      /* set column = 1 */

#ifdef __WATCOMC__
  int386(DISPLAY_BIOS, &inregs, &inregs);
#else
  int86(DISPLAY_BIOS, &inregs, &inregs);
#endif
}

void SetCursorToPrompt(void)
{
  SetCursorToLine(0);
}

