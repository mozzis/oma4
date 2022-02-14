// omablt.h - definition for oma4blt.asm

const long SYS_COLORS = 20;
const long MIN_COLOR = 9;
const long MAX_COLOR = 245;
const long NUM_COLOR = 235;
const long TOTAL_COLOR = 256;
const long COLOR_RANGE = (MAX_COLOR - MIN_COLOR) + 1;
const float COLOR_STEP = (float)TOTAL_COLOR / (float)NUM_COLOR;

// external 32-bit assembler routine to move data from DATMGR source
// to the image bitmap
extern "C" __far _cdecl CopyBlock(
       char __far * pDest   , //pointer to destination bitmap
       DWORD __far *pSource , //pointer to source data
       DWORD dwWidth , //Width of data in pixels
       DWORD dwHeight, //Height of data in lines
       DWORD dwScanD , //Size of Dest scan line in bytes
       DWORD dwScanS , //Size of Src scan line in bytes
       DWORD dwOffset, //Brightness adjustment
       float dwScale   //Contrast adjustment
      );

