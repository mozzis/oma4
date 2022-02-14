/***************************************************************************/
/*                                                                         */
/* File Name:   IMAGE386.inc                                               */
/* Version:     1.00 - Initial version.                                    */
/* Description: This include file defines the common memory comunications  */
/*      for the DAC program IMAGE and the OMACP program IMAGE.             */
/*                                                                         */
/*      WARNING : This file should be the same as IMAGE.INC used for       */
/*                the DAC compiler.  This file has the data types changed  */
/*                for the 32 bit 386 processor.                            */
/*                                                                         */
/***************************************************************************/

#ifndef image386file
#define image386file

/* General equates  ********************************************************/
#define SERIAL_MIN      32          /* Minimum number of pixels.           */
#define IMAGE_MIN       32          /* Minimum number of pixels.           */
#define DATA_HEADER     (4 + 200)   /* Source Comp and Data info structure */
#define TCL_INTEGER     0x55555555  /* These are the different data types. */
#define TCL_SIGNED      0xaaaaaaaa
#define TCL_FLOAT       0x5a5a5a5a
#define TEXT_LEN     75             /* Maximum length of text buffer.      */
#define ERROR_LEN       80

struct ControlType {                /* Host and DAC Control structure.     */
/* DAC to Host ------------------------------------------------------------*/
   ULONG DataMemAddress;     /*  Starting location of Data memory.  */
   ULONG OptionReply;        /*  Option board's reply value.        */
   ULONG CoolerStatus;       /*  Status reply from the Servo board. */
   ULONG DetectorID;         /*  Detector Controller's reply.       */
   ULONG AvailableMemories;  /*  Number of Data Memory bytes to use.*/
   ULONG MemSize;            /*  Size of each memory in bytes.      */
   ULONG Acquired;           /*  Number of memories acquired.       */
   USHORT ReturnMessage;/*  Message to Host.                   */
   SHORT DataStatusFlag;        /*  Used to drive the display.         */
   USHORT MaxSerialPixels; /* Maximum number for this detector.*/
   USHORT MaxImagePixels;  /* Maximum number for this detector.*/
/* Host to DAC ------------------------------------------------------------*/
   ULONG NumMemories;        /*  Number of memories per DA Mode.    */
   ULONG UsableMemory;       /*  Physical memory available.         */
   USHORT NumSerialPixels; /*  Number of pixels per frame.     */
   USHORT NumImagePixels;  /*  Number of pixels per frame.     */
   USHORT DAMode;       /*  DA Mode to run.                    */
   USHORT NumPrepFrames;/*  Number of preps per DA Mode.       */
   USHORT NumFrames;    /*  Number of frames per memory.       */
   USHORT NumIgnores;   /*  Number of Ignores per memory.      */
   USHORT PixelSize;    /*  Physical size of each pixel.       */
   USHORT HandShake;    /*  Host to DAC handshake register.    */
   SHORT AcqToDiskFlag;         /* Flag for Acquire data to disk.      */
/* General Control stuff --------------------------------------------------*/
   SHORT Temperature;           /*  Desired Cooler temerature.         */
   double ExposureTime;             /*  Exposure time in seconds.          */
   ULONG XLeft;              /* Left side of the image, zero based. */
   ULONG XRight;             /* Right side of the image, zero based.*/
   ULONG YTop;               /* Top  side of the image, zero based. */
   ULONG YBottom;            /* Bottom side of the image, zero based*/
   LONG AntiBloom;                   /* AntiBlooming percent.               */
   SHORT ReadFastFlag;          /* Flag for 10us conversions.          */
   SHORT AutoSubFlag;           /* Flag for realtime BG subtraction.   */
   SHORT AutoAcqFlag;           /* Flag for auto BG acquisition.       */
   SHORT ShutterClosedFlag;     /* Flag for BG acq. w/shutter closed.  */
   char ErrorMsg[ERROR_LEN + 1];    /* Used to cue errors.                 */
   double Version;                  /* HDRIS program(s) version.           */
   SHORT AutoAutoFlag;              /* Flag for auto scale during live.    */
};

/* Global varibles.  *******************************************************/
extern struct ControlType far *Cntrl;     

typedef enum {SAVE, RETURN, ADJUST, BW, COLOR} COMMAND_TYPE;
typedef enum {SETUP,FILES,CONFIG,MATH,TRIG,INFO,STAT,PRINT,LCIFM} NEWFORM;

/*  fill the Display structure with given values and 
   assign addresses to the pointers.                                       */
/***************************************************************************/
struct DISPLAY_CNTRL {
   LONG  MemoryNum;                  /* Data Memory to be displayed.        */
   LONG  IndexLeft;                  /* Index positioning parameters.       */
   LONG  IndexWidth;                                                          
   LONG  IndexTop;  
   LONG  IndexBottom;
   LONG  XLeft;                      /* Left side of the image.             */
   LONG  XRight;                     /* Right side of the image.            */
   LONG  YTop;                       /* Top  side of the image.             */
   LONG  YBottom;                    /* Bottom side of the image.           */
   LONG  XLineXLeft;                 /* Left side of the X Line graph.      */
   LONG  XLineXRight;                /* Right side of the X Line graph.     */
   LONG  XLineYTop;                  /* Top  side of the X Line graph.      */
   LONG  XLineYBottom;               /* Bottom side of the X Line graph.    */
   LONG  YLineXLeft;                 /* Left side of the Y Line graph.      */
   LONG  YLineXRight;                /* Right side of the Y Line graph.     */
   LONG  YlineYTop;                  /* Top  side of the Y Line graph.      */
   LONG  YLineYBottom;               /* Bottom side of the Y Line graph.    */
   LONG  CurX;                       /* Values for the image's cursor.      */
   LONG  CurY;                 
   LONG  ShowCursor;                 /* Image cursor enable flag.           */   
   LONG  ZoomXLeft;                  /* Size of the Zoom box.               */
   LONG  ZoomXRight;                 /* Size of the Zoom box.               */
   LONG  ZoomYTop;                   /* Size of the Zoom box.               */
   LONG  ZoomYBottom;                /* Size of the Zoom box.               */
   LONG  CursorOnFlag;               /* Cursor shown on image flag.         */
   LONG  ZoomBoxOnFlag;              /* Zoom box shown on image flag.       */
   LONG  ShowZoomBox;                /* Zoom box enable flag.               */
   LONG  SquareZoomBoxFlag;          /* Always draw a square zoom box.      */
   LONG  GlobalScalingFlag;          /* Global scaling factors for display. */
   float ZoomFactor;                /* Used for scaling(zoom) display.     */
   LONG  ScaleIndex;                 /* False color scale index value.      */
   NEWFORM NewForm;                 /* Discribes which Option form to open.*/
   char far *DisplayPtr;            /* Video display memory pointer.       */
   char *XYGraphLUPtr;              /* XY Graph Look-up memory pointer.    */
   char ErrorMsg[50];               /* Used to queue errors.               */
   };

/* Global varibles  ********************************************************/
extern struct DISPLAY_CNTRL far *Display;

#endif
