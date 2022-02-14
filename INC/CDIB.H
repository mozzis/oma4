// CDib.h : interface of the CDib class
//
/////////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision: 1 $
//   Last check-in   $Date: 6/18/99 1:52a $
//   Last changes by $Author: Maynard $
//   Changes:
//   $Log: /OMA4000/Include/CDIB.H $
 * 
 * 1     6/18/99 1:52a Maynard
// 
//    Rev 1.1   29 Sep 1995 20:56:58   MLM
// Add PVCS info tags
//  
//////////////////////////////////////////////////////////////////////////

#ifndef CDIB_INCLUDED
#define CDIB_INCLUDED

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

/* Handle to a DIB */
DECLARE_HANDLE(HDIB);

/* DIB constants */
#define PALVERSION   0x300             // Palette version designator
#define DONT_CHANGE_FLAGS -1           // Signal for CopyPaletteChangingFlags().
#define WW_PAL_HPALINFO 0              // Handle to PALINFO structure.

enum XDrawDir { XDrawRight = 0, XDrawLeft };
enum YDrawDir { YDrawDown = 0, YDrawUp };

class CDib
{
public:
  CDib(CDC * pDC,            // device context
       ULONG __huge *pData,  // pointer to image data 
       CSize *pSize,         // dimensions of image
       long Offset,          // brightness factor
       float Scale);         // contrast factor

  virtual ~CDib();

// CDib actions
  BOOL Paint(HDC hDC, LPRECT const pDst, LPRECT const pSrc, XDrawDir XD, YDrawDir YD);
  void EditCopy(CWnd *pWnd, const CRect& SourceRegion, const CRect& ClipRegion);
  HANDLE RenderFormat(UINT nFormat);
  void EditPaste(CWnd *pWnd, const CRect& SourceRegion);
  HBITMAP CropBitmap(HBITMAP hbm, HPALETTE hPal, CRect& BRect, CSize& lpptSize);
  LRESULT DoPaletteChanged(WPARAM wParam, CFrameWnd* pAppFrame, HWND hWnd, CDocument *pDoc);

protected:
// Set/Get CDib attributes
  DWORD DIBWidth(LPSTR lpDIB);
  DWORD DIBHeight(LPSTR lpDIB);

private:
// attributes
  HPALETTE m_hPalette;        // Handle to palette object
  ULONG __huge * m_DataPtr;   // Data pointer passed in from document
  HDIB m_hDIB;                // HANDLE to the DIB's memory
  HGLOBAL m_hDDBmp;           // handle to DDB copy made for Edit function
  HWND m_hWndClip;
  CRect m_ClipRect;
  CSize m_ClipSize;

//implementation
  BOOL WINAPI CreateDIBPalette(CDC * pDC);
  BOOL WINAPI CreateDIBBitMap(CDC *pDC, CSize *pSize, long Offset, float Scale);
  WORD WINAPI ColorsInPalette(HPALETTE hPal);
  LPSTR WINAPI FindDibBits(LPSTR lpbi);
  WORD DibNumColors(LPSTR lpbi);
  WORD WINAPI ColorTableSize(LPSTR lpbi);
  HANDLE WINAPI CopyHandle(HANDLE h);
  HPALETTE CopyPaletteChangingFlags(HPALETTE hPal, BYTE bNewFlag);
  HANDLE BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal);
  HBITMAP DIBToBitmap (HANDLE hDIB, HPALETTE hPal);
  HANDLE LocalRenderFormat(HWND hWndClip, int cf, CSize ptDIBSize);
  
  inline BOOL IsWin30DIB(LPSTR lpbi)
    {
    return ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER));
    }
};

#endif
/////////////////////////////////////////////////////////////////////////////

