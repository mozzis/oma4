/* -----------------------------------------------------------------------
/
/  Wintcl.c
/  Morris Maynard Setp 1994
/  Copyright (c) 1994,  EG&G Instruments Inc.
/
/  $Header:   $
/  $Log:   $
*/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "wintcl.h"

BOOL DebugDump = 0;

// definitions for Hidris conversion
#define BITS16       0            // FileFormat valid data values.
#define BIT8PACKED   1
#define BITS32       2

#define BITS8GREY    8            // PixelFormat valid data values.
#define BITS16GRAY   16
#define BITS32GRAY   32
#define TCLIDENTIFIER 1234        // Used to identify a TCL32bit file  
#define TCL_INTEGER  0x55555555   // Data types.  
#define TCL_SIGNED   0xaaaaaaaa
#define TCL_FLOAT    0x5a5a5a5a
#define TEXT_LEN     75           // a number not evenly divisible by 3!  

typedef struct            // TCL data header structure.       
{                                  
  USHORT FileFormat;      // File format descriptor.           
  USHORT NumSerialPixels; // Number of pixels per frame.       
  USHORT NumImagePixels;  // Number of pixels per frame.       
  USHORT AlwaysZero;      // Zero for disk files.              
  USHORT PixelFormat;     // Pixel format descriptor.          
  SHORT  Reserved[27];    // This area is reserved by TCL.     
// User specific data ----------------------------------------------------- 
  USHORT NumFrames;       // Number of frames per memory.     
  SHORT  Temperature;     // Desired Cooler temerature.       
  DOUBLE ExposureTime;    // Exposure time in seconds.        
  ULONG  XLeft;           // Phyical locations values.         
  ULONG  XRight;
  ULONG  YTop;
  ULONG  YBottom;
  LONG   Identifier;      // Used to identify a TCL file.      
  USHORT PixelSize;       // Physical size of each pixel.     
  LONG   DataType;        // Data type see IMAGE386.INC        
  USHORT PixelSpeed;      // Convesion time in uS.             
  SHORT  NumMemories;     // Number of Mems per file.          
  SHORT  UnusedNumericalData[96 - 21]; // Unused user numerical data.   
// ------------------------------------------------------------------------
  CHAR UserText1[TEXT_LEN / 3];       // HiDRIS text.          
  CHAR UserText2[TEXT_LEN / 3];       // HiDRIS text.          
  CHAR UserText3[TEXT_LEN / 3];       // HiDRIS text.          
  CHAR UserText[152 - TEXT_LEN];      // User text.            
  USHORT Date[15];                    // Creation date of file.          
  CHAR DescriptionText[74];           // Description text.     
}  TCL_HEADER;

TCL_HEADER TCL_Header;

static inline DWORD DMult(short A, short B)
{
return (DWORD)((DWORD)A * (DWORD)B);
}

ERR_OMA error(ERR_OMA err, char * msg)
{
//  va_list insert_args;
  char /* *msg, */ msgstr[128];

//  va_start(insert_args, err);
//  msg = va_arg(insert_args, CHAR *);
//  msg = (CHAR *) insert_args;
  wsprintf(msgstr, "error %d: %s\n", err, msg);
//  va_end(insert_args);
  MessageBox(NULL, msgstr, "DATMGR error!", MB_ICONHAND | MB_TASKMODAL);
  return err;
}

#define Copy(dest, src) _fmemmove(dest, src, sizeof(src))

/****************************************************************************/
BOOL ReadTCLHeader(FILE * fhnd, LPENTRY pEntry)
{
  if (fseek(fhnd, 0L, SEEK_SET))
    return error(-3, "Datafile");

  if (fread(&TCL_Header, sizeof(TCL_HEADER), 1, fhnd) != 1)
    return error(-2, "Datafile");
  pEntry->Method.HidrisSpecials.TCLFileFormat = TCL_Header.FileFormat;
  pEntry->Method.ScanSetup.Points = TCL_Header.NumSerialPixels;
  pEntry->Method.ScanSetup.Tracks = TCL_Header.NumImagePixels;
  // Skip AlwaysZero
  pEntry->Method.HidrisSpecials.TCLPixelFormat = TCL_Header.PixelFormat;
  Copy(pEntry->Method.HidrisSpecials.TCLReserved1, TCL_Header.Reserved);
  pEntry->Method.DACSetup.Scans = TCL_Header.NumFrames;               // USHORT
  pEntry->Method.AuxSetup.Temperature =  TCL_Header.Temperature;      // SHORT
  pEntry->Method.ShutterSetup.ExposeTime = (float)TCL_Header.ExposureTime;   // DOUBLE
  pEntry->Method.ScanSetup.XMap.Zero[0] = 1;
  pEntry->Method.DisplaySetup.Xaxis.Min = (float)TCL_Header.XLeft;    // ULONG   
  pEntry->Method.DisplaySetup.Xaxis.Max = (float)TCL_Header.XRight;   // ULONG   
  pEntry->Method.DisplaySetup.Yaxis.Min = (float)TCL_Header.YTop;     // ULONG   
  pEntry->Method.DisplaySetup.Yaxis.Max = (float)TCL_Header.YBottom;  // ULONG   
  // Skip Identifier LONG 
  pEntry->Method.ScanSetup.XMap.Delta[0] = TCL_Header.PixelSize;      // USHORT  

  short Channels = TCL_Header.PixelSize * TCL_Header.NumSerialPixels;

  if (Channels > 512)
  pEntry->Method.DetStructSetup.ActiveX = 1024;
  else if (Channels > 256)
  pEntry->Method.DetStructSetup.ActiveX = 512;
  else
  pEntry->Method.DetStructSetup.ActiveX = 256;
  Channels = TCL_Header.PixelSize * TCL_Header.NumImagePixels;

  if (Channels > 512)
  pEntry->Method.DetStructSetup.ActiveY = 1024;
  else if (Channels > 256)
  pEntry->Method.DetStructSetup.ActiveY = 512;
  else if (Channels > 256)
  pEntry->Method.DetStructSetup.ActiveY = 256;
    
  switch (TCL_Header.DataType)       // LONG    
    {
    case TCL_INTEGER:
      pEntry->Method.DACSetup.DataType = DTYPE_ULONG;
      break;
    case TCL_SIGNED:
      pEntry->Method.DACSetup.DataType = DTYPE_LONG;
      break;
    case TCL_FLOAT:
      pEntry->Method.DACSetup.DataType = DTYPE_FLOAT;
      break;
    }
 pEntry->Method.AuxSetup.PixTimeIndex = TCL_Header.PixelSpeed;    // USHORT  
 pEntry->Method.DACSetup.Mems = TCL_Header.NumMemories;           // SHORT   
 pEntry->MCount = TCL_Header.NumMemories;                         // SHORT   
 Copy(pEntry->Method.HidrisSpecials.TCLUserText1, TCL_Header.UserText1);
 Copy(pEntry->Method.HidrisSpecials.TCLUserText2, TCL_Header.UserText2);
 Copy(pEntry->Method.HidrisSpecials.TCLUserText3, TCL_Header.UserText3);
 Copy(pEntry->Method.HidrisSpecials.TCLDescText, TCL_Header.DescriptionText);
 // Skip Date mmddyyyy
  return 0;
}

/* Return truth that given file is a TCL file */
BOOL IsFileTCL(FILE * fhnd)
{
  rewind(fhnd);
  if (fread(&TCL_Header, sizeof(TCL_HEADER), 1, fhnd) != 1)
    return FALSE;
  rewind(fhnd);
  if (TCL_Header.FileFormat == BITS32 && 
      TCL_Header.Identifier == TCLIDENTIFIER &&
      (TCL_Header.DataType == TCL_INTEGER) || 
      (TCL_Header.DataType == TCL_FLOAT) ||
      (TCL_Header.DataType == TCL_SIGNED))
    return(TRUE);
  else
    return FALSE;
}

/* Read TCL data file into structure pointed to by pEntry */
/* Return TRUE if error, else FALSE */
/****************************************************************************/
BOOL ReadTCLData(FILE * fhnd, LPENTRY pEntry, DWORD Offset, DWORD Size)
{  
  DATA_BLOCK __huge * pDBlock;
  size_t SizeX, SizeY, SizeData;
  DWORD SizeBlk;
  
  if (!pEntry)
    return TRUE;
  
  SizeData = (pEntry->Method.DACSetup.DataType & 0x0F);
  
  if (Size = -1)
    {
     SizeX = pEntry->Method.ScanSetup.Points * SizeData;
     SizeY = pEntry->MCount * pEntry->Method.ScanSetup.Tracks;
     }
  else
    {
    if (Size < 0x10000L)
      {
      SizeX = 1;
      SizeY = (size_t)Size;
      }
    else
      {
      SizeX = (size_t)(Size / 0x10000L);
      SizeY = (size_t)(Size / SizeX);
      }
    }   
    
  Size = SizeBlk = DMult(SizeX, SizeY); /* Size is the product of X and Y dims. */
  Size += (DWORD)SZDBLOCK; /* plus size of data block header */

  if (!pEntry->pData) /* if pointer is null, is not allocated */
    {
     pEntry->hData = GlobalAlloc(GPTR | GMEM_DDESHARE, Size);  /* allocate on heap */
     if (!pEntry->hData) /* if failure */
       return (TRUE);
     pDBlock = (DATA_BLOCK __huge *)GlobalLock(pEntry->hData); /* convert to pointer */
     if (!pDBlock) /* check if lock failed */
       {
       GlobalFree(pEntry->hData);
       return (TRUE);
       }
     /* convert pointer to huge form - not necessary in USE32! */
     /* also adds offset past block header - pData points just to data */
     pEntry->pData = (void __huge *)(&(((char __huge *)pDBlock)[SZDBLOCK]));
     }
   else /* back-convert existing pointer to data to pointer to data block */
     {
     pDBlock = (DATA_BLOCK __huge *)
       ((DWORD)(pEntry->pData) - (DWORD)SZDBLOCK);
     }

  /* klugey method of putting legal values in some fields */
  /* assumes square data for TCL! */     
  pDBlock->PCount = SizeX / SizeData;           // the number of points in a curve
  pDBlock->CCount = SizeY / SizeData;           // the number of curves in a memory
  pDBlock->MemData = FALSE;                     // TRUE if data is in (OMA4) memory
  pDBlock->DataType = pEntry->Method.DACSetup.DataType;  // data type (int, float, etc.)
  pDBlock->szData = Size - SZDBLOCK;            // size of Y Data in bytes

  /* read in the data */
  fseek(fhnd, sizeof(TCL_HEADER), SEEK_SET);
  if (fread(pEntry->pData, SizeX, SizeY, fhnd) != SizeY)
    return TRUE;
  
  Size -= SizeBlk;
  if (Size < SZDBLOCK)
    {
    char __huge * pTemp = &(((char __huge *)(pEntry->pData))[SizeBlk]);
    if (fread(pTemp, (size_t)Size - SZDBLOCK, 1, fhnd) != 1)
      return TRUE;
    }
  return FALSE;
}

/****************************************************************************/
BOOL WriteTCLHeader(FILE * fhnd, LPENTRY pEntry)
{
  TCL_Header.FileFormat = BITS32;
  TCL_Header.NumSerialPixels = pEntry->Method.ScanSetup.Points;
  TCL_Header.NumImagePixels = pEntry->Method.ScanSetup.Tracks;
  TCL_Header.AlwaysZero = 0;
  TCL_Header.PixelFormat = BITS32GRAY;
  Copy(TCL_Header.Reserved, pEntry->Method.HidrisSpecials.TCLReserved1);
  TCL_Header.NumFrames = pEntry->Method.DACSetup.Scans;               // USHORT  
  TCL_Header.Temperature = pEntry->Method.AuxSetup.Temperature;       // SHORT   
  TCL_Header.ExposureTime = pEntry->Method.ShutterSetup.ExposeTime;   // DOUBLE  

  pEntry->Method.DisplaySetup.Xaxis.Min = 110L;
  TCL_Header.XLeft = 110L;
  pEntry->Method.DisplaySetup.Xaxis.Max = 511L;
  TCL_Header.XRight = 511L;
  pEntry->Method.DisplaySetup.Yaxis.Min = 120L;
  TCL_Header.YTop = 120L;
  pEntry->Method.DisplaySetup.Yaxis.Max = 511L;
  TCL_Header.YBottom = 511L;

//  TCL_Header.XLeft = (ULONG)pEntry->Method.DisplaySetup.Xaxis.Min;    // ULONG   
//  TCL_Header.XRight = (ULONG)pEntry->Method.DisplaySetup.Xaxis.Max;   // ULONG   
//  TCL_Header.YTop = (ULONG)pEntry->Method.DisplaySetup.Yaxis.Min;     // ULONG   
//  TCL_Header.YBottom = (ULONG)pEntry->Method.DisplaySetup.Yaxis.Max;  // ULONG
   
  TCL_Header.Identifier = TCLIDENTIFIER;
  TCL_Header.PixelSize = pEntry->Method.ScanSetup.XMap.Delta[0];      // USHORT  
  switch (pEntry->Method.DACSetup.DataType)
    {
    case DTYPE_ULONG:
      TCL_Header.DataType = TCL_INTEGER;
      break;
    case DTYPE_LONG:
      TCL_Header.DataType = TCL_SIGNED;
      break;
    case DTYPE_FLOAT:
      TCL_Header.DataType = TCL_FLOAT;
      break;
    }
 TCL_Header.PixelSpeed = pEntry->Method.AuxSetup.PixTimeIndex;    // USHORT  
 TCL_Header.NumMemories = pEntry->Method.DACSetup.Mems;           // SHORT   
 Copy(TCL_Header.UserText1, pEntry->Method.HidrisSpecials.TCLUserText1);
 Copy(TCL_Header.UserText2, pEntry->Method.HidrisSpecials.TCLUserText2);
 Copy(TCL_Header.UserText3, pEntry->Method.HidrisSpecials.TCLUserText3);
 Copy(TCL_Header.DescriptionText, pEntry->Method.HidrisSpecials.TCLDescText);
 TCL_Header.Date[0] = 1;
 TCL_Header.Date[1] = 1;
 TCL_Header.Date[2] = 1;
 TCL_Header.Date[3] = 9;
 TCL_Header.Date[4] = 9;
 TCL_Header.Date[5] = 5;
 
  if (fseek(fhnd, 0L, SEEK_SET))
    return error(-3, "Datafile");

  if (fwrite(&TCL_Header, sizeof(TCL_HEADER), 1, fhnd) != 1)
    return error(-2, "Datafile");

  return 0;
}

/****************************************************************************/
BOOL WriteTCLData(FILE * fhnd, LPENTRY pEntry, DWORD Offset, DWORD Size)
{  
  DATA_BLOCK __huge * pDBlock;
  size_t SizeX, SizeY, SizeData;
  DWORD SizeBlk;
  
  if (!pEntry || !pEntry->pData)
    return TRUE;
                               
  SizeData = pEntry->Method.DACSetup.DataType & 0x0F;
                               
  if (Size = -1)
    {
     SizeX = pEntry->Method.ScanSetup.Points * SizeData;
     SizeY = pEntry->Method.ScanSetup.Tracks;
     }
  else
    {
    if (Size < 0x10000L)
      {
      SizeX = 1;
      SizeY = (size_t)Size;
      }
    else
      {
      SizeX = (size_t)(Size / 0x10000L);
      SizeY = (size_t)(Size / SizeX);
      }
    }   

   SizeBlk = Size = DMult(SizeX, SizeY);
   Size += (DWORD)SZDBLOCK;

  pDBlock = ((DATA_BLOCK __huge *)pEntry->pData) - SZDBLOCK;
  
  fseek(fhnd, sizeof(TCL_HEADER), SEEK_SET);
  if (fwrite(pEntry->pData, SizeX, SizeY, fhnd) != SizeY)
     return TRUE;
     
   Size -= SizeBlk;
   if (Size < SZDBLOCK)
     {
     char __huge * pTemp = &(((char __huge *)(pEntry->pData))[SizeBlk]);
     if (fwrite(pTemp, (size_t)Size - SZDBLOCK, 1, fhnd) != 1)
       return TRUE;
     }
   return FALSE;
}

