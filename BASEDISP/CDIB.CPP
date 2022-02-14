//////////////////////////////////////////////////////////////////
// CDib - DIB handler class
// 
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 21:00:32  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/CDIB.CPV  $
// 
//    Rev 1.1   29 Sep 1995 21:00:32   MLM
// Add PVCS info tags
// 
//////////////////////////////////////////////////////////////////////////
// Author: MLM September 1995

 /* This is a C++ class to encapsulate some common DIB operations. At
 present we support creating, destroying, displaying, and the clip-
 board edit and paste functions.

 I created this class because I found that I needed to deal with DIB's
 somewhat differently than is usual. Specifically, my DIB's usually
 come from some piece of hardware, such as a CCD detector, rather than
 a file or the clipboard. As such, they normally do not come with an
 attached palette. In fact, the palette of the display device being
 used will is usually inadequate to fully display the 14 to 18 bit
 data on which these bitmaps are based, and there are not even
 adequate DIB format definitions as yet to handle them.
 
 Instead, I need to be able to just create a DIB structure of given
 dimensions and change around the palette on it at will. In this frame
 of reference, it seems ridiculous that the CreatePalette functions
 offered by the examples all expect to receive a prepared CPalette
 object or handle to a palette, etc. For my purposes, I will always
 want a grayscale palette. I also want to optimize the palette layout
 so that Windows will not slow down its display by trying to match
 entries in my palette table with system colors.

 I also need to be able to copy the DIB bitmap and palette to the
 clipboard in order to exchange data with other applications. I know
 MFC has some high-level stuff in place to do this via OLE, but for
 now (since I'm just stealing most of this stuff from various places)
 I will just do it the normal clipboard way. 
 
 I have borrowed heavily here from the MSVC and MFC Samples directo-
 ries, viz. DIBLOOK and DIBVIEW, as well as from my previous efforts
 using less structured approaches. */

#include "stdafx.h"
#include "cdib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//  The CDib structure contains all three parts necesary for storage and
//  display of DIBs. They are:
//
//  - A BITMAPINFO structure containing Bitmap info. The first long word of
//  this structure contains a 32 bit size field which is set to the size of
//  the structure.
// 
//   - A color table. The size of this table is stored in the above
//   structure. Determining the exact size in bytes is complicated by
//   possiblities involving video display schemes, but a member function
//   called ColorTableSize is provided to calculate it.
//
//  - An array of data which is the actual DIB bitmap. The offset of this
//  array from the beginning of the array may be found using the FindDIBBits
//  member function.
//
//  A LOGPALETTE structure is also involved in setting up a DIB for display
//  or storage. This structure contains a version number, the #entries, and
//  an array of palette entries which can be used to initialize a Windows
//  palette object. This structure is created in the course of creating the
//  DIB object, used to initialize a Windows palette object, and then is
//  destroyed. A handle to the Windows object is retained in the member 
//  variable m_hPalette.

// Structure of object at start of DIB block
// typedef struct tagBMPINFO {
//    BITMAPINFOHEADER bmiHeader; // win 3.0 bitmap info
//    RGBQUAD bmiColors[1];       // variable size array of colors
// } BMPINFO;

/////////////////////////////////////////////////////////////////////////////
// CDib

/////////////////////////////////////////////////////////////////////////////
// CDib construction/destruction

CDib::CDib(CDC * pDC, 
           ULONG __huge *pData, 
           CSize *pSize,
           long Offset, 
           float Scale)
{
  long DIBsize = (long)((long)pSize->cx * (long)pSize->cy);
  m_hDIB = (HDIB) ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 
                                sizeof(BITMAPINFO) + 
                                TOTAL_COLOR * sizeof(RGBQUAD) +
                                DIBsize);
  m_DataPtr = pData;
  m_hPalette = 0;
  m_hWndClip = 0;
  CreateDIBPalette(pDC);
  CreateDIBBitMap(pDC, pSize, Offset, Scale);
}

// CDib - destructor
CDib::~CDib()
{
  if (m_hDIB)
    {
    GlobalUnlock(m_hDIB);
    GlobalFree(m_hDIB);
    m_hDIB = 0;
    }

  if (m_hDDBmp)
    {
    GlobalUnlock(m_hDDBmp);
    GlobalFree(m_hDDBmp);
    m_hDDBmp = 0;
    }

  if (m_hPalette)
    {
    DeleteObject(m_hPalette);
    m_hPalette = 0;
    }
  m_hWndClip = 0;
  m_ClipRect.SetRectEmpty();
  m_ClipSize.cx = m_ClipSize.cy = 0;
}

// Calculate nearest multiple of 4 of input
/////////////////////////////////////////////////////////////////////////////
static inline short DWAlign(short Bytes)
{
  return ((Bytes + 3) / 4) * 4;
}

// Our version of Create Palette - make gray scale.
// This will try to create an identity palette; where Windows Palette
// manager will not waste time on translation during display
// Assume only dealing with Win 3.0 bitmaps for now.
//////////////////////////////////////////////////////////////////////////////
BOOL CDib::CreateDIBPalette(CDC * pDC)
{
  HANDLE hLogPalette;             // handle for our Logical Palette
  LPLOGPALETTE  lpLogPalette;     // pointer to the Logical Palette
  LPSTR lpDIB;                    // pointer to packed DIB
  LPBITMAPINFO lpBmpInfo;         // pointer to Win 3.0 BITMAPINFO structure

  if (m_hDIB == NULL)
    return(FALSE);
    
  if (m_hPalette)
    ::DeleteObject(m_hPalette);
    
  lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);
  lpBmpInfo = (LPBITMAPINFO)lpDIB;

  hLogPalette = GlobalAlloc(GHND, sizeof(LOGPALETTE) + 
                            TOTAL_COLOR * sizeof(PALETTEENTRY));

  if (!hLogPalette)
    {
    ::GlobalUnlock((HGLOBAL) m_hDIB);
    return(FALSE);
    }

  lpLogPalette = (LPLOGPALETTE) ::GlobalLock((HGLOBAL)hLogPalette);

  lpLogPalette->palVersion = PALVERSION;
  lpLogPalette->palNumEntries = TOTAL_COLOR;
  // Set first and last 10 entries in pallete to system colors
  GetSystemPaletteEntries(pDC->GetSafeHdc(), 0, (WORD)TOTAL_COLOR, lpLogPalette->palPalEntry);
  // Set remaining entries to our colors. Note both the bitmapinfo in the
  // associated DIB and the palette are set up here. 
  for (long i = MIN_COLOR; i < MAX_COLOR; i++) 
    { 
    unsigned char palval = (unsigned char)((float)(i-MIN_COLOR) * COLOR_STEP);
    lpBmpInfo->bmiColors[i].rgbBlue  = 
      lpBmpInfo->bmiColors[i].rgbGreen = 
      lpBmpInfo->bmiColors[i].rgbRed   = 
      lpLogPalette->palPalEntry[i].peRed =
    lpLogPalette->palPalEntry[i].peGreen =
    lpLogPalette->palPalEntry[i].peBlue  = palval;
    lpBmpInfo->bmiColors[i].rgbReserved = 0;
    lpLogPalette->palPalEntry[i].peFlags = PC_RESERVED;
    }
  m_hPalette = ::CreatePalette(lpLogPalette);
  GlobalUnlock(hLogPalette);
  GlobalFree(hLogPalette);
  return(TRUE);
}

//////////////////////////////////////////////////////////////////////////////
// Create a bitmap from data at m_DataPtr
// 
BOOL WINAPI CDib::CreateDIBBitMap(CDC *pDC, CSize *pSize, long Offset, float Scale)
{ 
  BITMAPINFO FAR *lpBmpInfo;
  char __huge *hpDibBits;
  long szBmpBlk;

  szBmpBlk = (ULONG)((ULONG)DWAlign(pSize->cx) * (ULONG)pSize->cy);
  if (szBmpBlk == 0L) /* protect against uninitialized parameters */
    return(FALSE);
  
  if (!m_hDIB)
    return FALSE;

  lpBmpInfo = (BITMAPINFO FAR *)GlobalLock(m_hDIB);    // points to DIB start

  // Set up the BMPINFO structure
  lpBmpInfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
  lpBmpInfo->bmiHeader.biWidth         = (long)DWAlign(pSize->cx);
  lpBmpInfo->bmiHeader.biHeight        = (long)pSize->cy;
  lpBmpInfo->bmiHeader.biPlanes        = 1;
  lpBmpInfo->bmiHeader.biBitCount      = 8;
  lpBmpInfo->bmiHeader.biCompression   = BI_RGB;
  lpBmpInfo->bmiHeader.biSizeImage     = szBmpBlk;
  lpBmpInfo->bmiHeader.biXPelsPerMeter = 0L;
  lpBmpInfo->bmiHeader.biYPelsPerMeter = 0L;
  lpBmpInfo->bmiHeader.biClrUsed       = TOTAL_COLOR;
  lpBmpInfo->bmiHeader.biClrImportant  = TOTAL_COLOR;

  hpDibBits = FindDibBits((LPSTR)lpBmpInfo);   // points to DIB bitmap bits

  if (!IsBadHugeReadPtr(hpDibBits, szBmpBlk))
    { 
    // call 32-bit assembly routine to move 32-bit data from detector
    // into 8-bit bitmap
    CopyBlock(hpDibBits,                // *Dest
              m_DataPtr,                // *Src
              pSize->cx,                // Src Width
              pSize->cy,                // Src Height
              DWAlign(pSize->cx),       // Dest line size 
              pSize->cx * sizeof(long), // Src line size
              Offset, Scale);   // map to 8 bits with brite and contrast adj
    }
  GlobalUnlock(m_hDIB);
  return(TRUE);
}

static inline short RectWidth(LPRECT pRect)
{
  return(pRect->right - pRect->left);
}

static inline short RectHeight(LPRECT pRect)
{
  return(pRect->bottom - pRect->top);
}

/*************************************************************************
 * Parameters:   HDC hDC          - DC to do output to
 *               LPRECT lpDCRect  - rectangle on DC to do output to
 *               HDIB hDIB        - handle to global memory with a DIB spec
 *                                  in it followed by the DIB bits
 *               LPRECT lpDIBRect - rectangle of DIB to output into lpDCRect
 *               CPalette* pPal   - pointer to CPalette containing DIB's palette
 * Return Value: BOOL             - TRUE if DIB was drawn, FALSE otherwise
 *
 * Description:
 *   Painting routine for a DIB.  Calls StretchDIBits() or
 *   SetDIBitsToDevice() to paint the DIB.  The DIB is
 *   output to the specified DC, at the coordinates given
 *   in lpDCRect.  The area of the DIB to be output is
 *   given by lpDIBRect.
 ************************************************************************/
BOOL CDib::Paint(HDC hDC, LPRECT const pDst, LPRECT const pSrc, XDrawDir XD, YDrawDir YD)
{
  LPBITMAPINFO lpDIBHdr;        // Pointer to BITMAPINFOHEADER
  LPSTR    lpDIBBits;           // Pointer to DIB bits
  BOOL     bSuccess=FALSE;      // Success/fail flag
  HPALETTE hOldPal=NULL;        // Previous palette
  short XLeft, XSize, YTop, YSize;

  /* Check for valid DIB handle */
  if (m_hDIB == NULL)
    return FALSE;

  /* Lock down the DIB, and get a pointer to the beginning of the bit
   *  buffer
   */
  lpDIBHdr  = (LPBITMAPINFO) ::GlobalLock((HGLOBAL) m_hDIB);
  lpDIBBits = FindDibBits((LPSTR)lpDIBHdr);

  // calculate stretching size and origin based on orientation
  YTop = pDst->top;
  YSize = RectHeight(pDst);
  if (YD == YDrawDown)
    {
    YTop += RectHeight(pDst);
    YSize = -YSize;
    }
    
  XLeft = pDst->left;
  XSize = RectWidth(pDst);
  if (XD == XDrawLeft)
    {
    XLeft += RectWidth(pDst);
    XSize = -XSize;
    }

  hOldPal = ::SelectPalette(hDC, m_hPalette, FALSE);
  ::RealizePalette(hDC);

  /* Make sure to use the stretching mode best for color pictures */
  ::SetStretchBltMode(hDC, STRETCH_DELETESCANS);

   bSuccess = ::StretchDIBits(hDC,             // hDC
                              XLeft, YTop,     // Dest X & Y
                              XSize, YSize,    // Dest Width & Height
                              pSrc->left,      // SrcX
                              pSrc->top,       // SrcY
                              RectWidth(pSrc), // wSrcWidth
                              RectHeight(pSrc), // wSrcHeight
                              lpDIBBits,       // The Bits
                              lpDIBHdr,        // lpBitsInfo
                              DIB_RGB_COLORS,  // wUsage
                              SRCCOPY);        // dwROP

   ::GlobalUnlock((HGLOBAL) m_hDIB);

  /* Reselect old palette */
  if (hOldPal != NULL)
    ::SelectPalette(hDC, hOldPal, TRUE);

  return bSuccess;
}

/*************************************************************************
 * Parameter:     LPSTR lpbi       - pointer to packed-DIB memory block
 * Return Value:  LPSTR            - pointer to the DIB bits
 *
 * Description:
 * This function calculates the address of the DIB's bits and returns a
 * pointer to the DIB bits.
 ************************************************************************/
LPSTR WINAPI CDib::FindDibBits(LPSTR lpbi)
{
  return (lpbi + *(LPDWORD)lpbi + ColorTableSize(lpbi));
}

/*************************************************************************
 * Parameter:      LPSTR lpbi       - pointer to packed-DIB memory block
 * Return Value:    WORD             - size of the color palette of the DIB
 * Description:
 * This function gets the size required to store the DIB's palette by
 * multiplying the number of colors by the size of an RGBQUAD (for a
 * Windows 3.0-style DIB) or by the size of an RGBTRIPLE (for an other-
 * style DIB).
 ************************************************************************/
WORD WINAPI CDib::ColorTableSize(LPSTR lpbi)
{
  /* calculate the size required by the palette */
  return (WORD)(DibNumColors(lpbi) * sizeof(RGBQUAD));
}

/*************************************************************************
 * Parameter:     LPSTR lpbi      - pointer to packed-DIB memory block
 * Return Value:  WORD            - number of colors in the color table
 * Description:
 * This function calculates the number of colors in the DIB's color table
 * by finding the bits per pixel for the DIB (whether Win3.0 or other-style
 * DIB). If bits per pixel is 1: colors=2, if 4: colors=16, if 8: colors=256,
 * if 24, no colors in color table.
 ************************************************************************/
WORD WINAPI CDib::DibNumColors(LPSTR lpbi)
{
  WORD wBitCount;  // DIB bit count

  /*  If this is a Windows-style DIB, the number of colors in the
   *  color table can be less than the number of bits per pixel
   *  allows for (i.e. lpbi->biClrUsed can be set to some value).
   *  If this is the case, return the appropriate value. */

  DWORD dwClrUsed;

  dwClrUsed = ((LPBITMAPINFOHEADER)lpbi)->biClrUsed;
  if (dwClrUsed != 0)
    return (WORD)dwClrUsed;

  /*  Calculate the number of colors in the color table based on
   *  the number of bits per pixel for the DIB. */
  wBitCount = ((LPBITMAPINFOHEADER)lpbi)->biBitCount;

  /* return number of colors based on bits per pixel */
  switch (wBitCount)
    {
    case 1:
      return 2;
    case 4:
      return 16;
    case 8:
      return 256;
    default:
      return 0;
  }
}

//////////////////////////////////////////////////////////////////////////
//// Clipboard support

static long inline WidthBytes(long bits)
{
  return(((bits) + 31) / 32 * 4);
}

//---------------------------------------------------------------------
// Purpose:    Does a "standard" initialization of a BITMAPINFOHEADER,
//             given the Width, Height, and Bits per Pixel for the
//             DIB.
//
//             By standard, I mean that all the relevant fields are set
//             to the specified values.  biSizeImage is computed, the
//             biCompression field is set to "no compression," and all
//             other fields are 0.
//
//             Note that DIBs only allow BitsPixel values of 1, 4, 8, or
//             24.  This routine makes sure that one of these values is
//             used (whichever is most appropriate for the specified
//             nBPP).
//
// Parms:      lpBmInfoHdr == Far pointer to a BITMAPINFOHEADER structure
//                            to be filled in.
//             dwWidth     == Width of DIB (not in Win 3.0 & 3.1, high
//                            word MUST be 0).
//             dwHeight    == Height of DIB (not in Win 3.0 & 3.1, high
//                            word MUST be 0).
//             nBPP        == Bits per Pixel for the DIB.
//
// History:   Date      Reason
//            11/07/91  Created
//            9/13/95   Ported into MFC for CDib class
//---------------------------------------------------------------------
void InitBitmapInfoHeader (LPBITMAPINFOHEADER lpBmInfoHdr,
                           DWORD dwWidth, DWORD dwHeight, int nBPP)
{
  _fmemset (lpBmInfoHdr, 0, sizeof (BITMAPINFOHEADER));

  lpBmInfoHdr->biSize   = sizeof (BITMAPINFOHEADER);
  lpBmInfoHdr->biWidth  = dwWidth;
  lpBmInfoHdr->biHeight = dwHeight;
  lpBmInfoHdr->biPlanes = 1;

  if (nBPP <= 1)
    nBPP = 1;
  else if (nBPP <= 4)
    nBPP = 4;
  else if (nBPP <= 8)
    nBPP = 8;
  else
    nBPP = 24;

  lpBmInfoHdr->biBitCount  = nBPP;
  lpBmInfoHdr->biSizeImage = WidthBytes(dwWidth * nBPP) * dwHeight;
}

//---------------------------------------------------------------------
// Purpose:    Given a device dependent bitmap and a palette, returns
//             a handle to global memory with a DIB spec in it.  The
//             DIB is rendered using the colors of the palette passed in.
//
//             Stolen almost verbatim from ShowDIB.
//
// Parms:      hBitmap == Handle to device dependent bitmap compatible
//                        with default screen display device.
//             hPal    == Palette to render the DDB with.  If it's NULL,
//                        use the default palette.
//
// History:   Date      Reason
//             6/01/91  Created
//             9/13/95  Ported into MFC for CDib class
//---------------------------------------------------------------------
HANDLE CDib::BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal)
{
  BITMAP             Bitmap;
  BITMAPINFOHEADER   bmInfoHdr;
  LPBITMAPINFOHEADER lpbmInfoHdr;
  LPSTR              lpBits;
  HDC                hMemDC;
  HANDLE             hDIB;
  HPALETTE           hOldPal = NULL;

  // Do some setup -- make sure the Bitmap passed in is valid,
  //  get info on the bitmap (like its height, width, etc.),
  //  then setup a BITMAPINFOHEADER.

  if (!hBitmap)
    return NULL;

  if (!GetObject (hBitmap, sizeof (Bitmap), (LPSTR) &Bitmap))
    return NULL;

  InitBitmapInfoHeader(&bmInfoHdr,
                       Bitmap.bmWidth,
                       Bitmap.bmHeight,
                       Bitmap.bmPlanes * Bitmap.bmBitsPixel);

  // Now allocate memory for the DIB.  Then, set the BITMAPINFOHEADER
  //  into this memory, and find out where the bitmap bits go.

  hDIB = GlobalAlloc(GHND, sizeof (BITMAPINFOHEADER) +
         ColorTableSize((LPSTR)&bmInfoHdr) + bmInfoHdr.biSizeImage);

  if (!hDIB)
     return NULL;

  lpbmInfoHdr  = (LPBITMAPINFOHEADER)GlobalLock(hDIB);
  *lpbmInfoHdr = bmInfoHdr;
  lpBits       = FindDibBits((LPSTR)lpbmInfoHdr);

  // Now, we need a DC to hold our bitmap.  If the app passed us
  //  a palette, it should be selected into the DC.

  hMemDC = GetDC (NULL);

  if (hPal)
    {
    hOldPal = ::SelectPalette (hMemDC, hPal, FALSE);
    ::RealizePalette(hMemDC);
    }

  // We're finally ready to get the DIB.  Call the driver and let
  // it party on our bitmap.  It will fill in the color table,
  // and bitmap bits of our global memory block.

  if (!::GetDIBits(hMemDC,
                   hBitmap,
                   0,
                   Bitmap.bmHeight,
                   lpBits,
                   (LPBITMAPINFO) lpbmInfoHdr,
                   DIB_RGB_COLORS))
    {
    ::GlobalUnlock (hDIB);
    ::GlobalFree (hDIB);
    hDIB = NULL;
    }
  else
    ::GlobalUnlock (hDIB);

    // Finally, clean up and return.
  if (hOldPal)
    ::SelectPalette (hMemDC, hOldPal, FALSE);
  ReleaseDC (NULL, hMemDC);
  return hDIB;
}

//---------------------------------------------------------------------
// Function:   DIBToBitmap
//
// Purpose:    Given a handle to global memory with a DIB spec in it,
//             and a palette, returns a device dependent bitmap.  The
//             The DDB will be rendered with the specified palette.
//
// Parms:      hDIB == HANDLE to global memory containing a DIB spec
//                     (either BITMAPINFOHEADER or BITMAPCOREHEADER)
//             hPal == Palette to render the DDB with.  If it's NULL,
//                     use the default palette.
//
// History:   Date      Reason
//            6/01/91  Created
//            9/13/95  Ported into MFC for CDib class
//---------------------------------------------------------------------
HBITMAP CDib::DIBToBitmap (HANDLE hDIB, HPALETTE hPal)
{
  LPSTR    lpDIBHdr, lpDIBBits;
  HBITMAP  hBitmap;
  HDC      hDC;
  HPALETTE hOldPal = NULL;

  if (!hDIB)
     return NULL;

  lpDIBHdr  = (char FAR *)GlobalLock(hDIB);
  lpDIBBits = FindDibBits(lpDIBHdr);
  hDC       = ::GetDC(NULL);

  if (!hDC)
     {
     GlobalUnlock (hDIB);
     return NULL;
     }

  if (hPal)
     hOldPal = SelectPalette(hDC, hPal, FALSE);
  RealizePalette(hDC);
  hBitmap = ::CreateDIBitmap(hDC,
                             (LPBITMAPINFOHEADER) lpDIBHdr,
                             CBM_INIT,
                             lpDIBBits,
                             (LPBITMAPINFO) lpDIBHdr,
                             DIB_RGB_COLORS);

  if (hOldPal)
    SelectPalette (hDC, hOldPal, FALSE);

  ReleaseDC (NULL, hDC);
  GlobalUnlock (hDIB);

  return hBitmap;
}

//---------------------------------------------------------------------
// Purpose:    Crops a bitmap to a new size specified by the lpRect
//             parameter.  The lpptSize parameter is used to determine
//             how much to stretch/compress the bitmap.  Returns a
//             handle to a new bitmap.  If lpRect is empty, copies the
//             bitmap to a new one.
//
//             Stolen almost verbatim out of ShowDIB.
//
// Parms:      hbm      == Handle to device dependent bitmap to crop.
//             hPal     == Palette to use in cropping (NULL for default pal.)
//             lpRect   == New bitmap's size (size we're cropping to).
//             lpptSize == A scaling factor scale by the proportion:
//                           Bitmap Width / lpptSize->x horizontally,
//                           Bitmap Height / lpptSize->y horizontally.
//                           Note that if lpptSize is set to the bitmap's
//                           dimensions, no scaling occurs.
//
// History:   Date      Reason
//            6/15/91   Stolen from ShowDIB
//            9/13/95   Ported into MFC for CDib class
//---------------------------------------------------------------------
HBITMAP CDib::CropBitmap(HBITMAP hbm, HPALETTE hPal, CRect& BRect, CSize& lpptSize)
{
  HDC      hMemDCsrc;
  HDC      hMemDCdst;
  HBITMAP  hNewBm = NULL;
  BITMAP   bm;
  int      dxDst,dyDst, dxSrc, dySrc;
  double   cxScale, cyScale;
  HPALETTE hOldPal1 = NULL;
  HPALETTE hOldPal2 = NULL;

  if (!hbm)
    return NULL;

  ::GetObject(hbm, sizeof(BITMAP), (LPSTR)&bm);

  hMemDCsrc = ::CreateCompatibleDC(NULL);
  hMemDCdst = ::CreateCompatibleDC(NULL);

  if (hPal)
    {
    hOldPal1 = SelectPalette (hMemDCsrc, hPal, FALSE);
    hOldPal2 = SelectPalette (hMemDCdst, hPal, FALSE);
    RealizePalette (hMemDCdst);
    }
  dxDst     = BRect.right  - BRect.left;
  dyDst     = BRect.bottom - BRect.top;
  cxScale   = (double) bm.bmWidth  / lpptSize.cx;
  cyScale   = (double) bm.bmHeight / lpptSize.cy;
  dxSrc     = (int) ((BRect.right - BRect.left) * cxScale);
  dySrc     = (int) ((BRect.bottom - BRect.top) * cyScale);

  if (dxDst == 0 || dyDst == 0)
    {
    dxDst = bm.bmWidth;
    dyDst = bm.bmHeight;
    }

  if (dxSrc == 0) dxSrc = 1;
  if (dySrc == 0) dySrc = 1;

  hNewBm = ::CreateBitmap (dxDst, dyDst, bm.bmPlanes, bm.bmBitsPixel, NULL);

  if (hNewBm)
    {
    HBITMAP hOldBitmap1, hOldBitmap2;

    hOldBitmap1 = (HBITMAP)SelectObject(hMemDCsrc, hbm);
    hOldBitmap2 = (HBITMAP)SelectObject(hMemDCdst, hNewBm);

    StretchBlt(hMemDCdst,
               0,
               0,
               dxDst,
               dyDst,
               hMemDCsrc,
               (int) (BRect.left * cxScale),
               (int) (BRect.top  * cyScale),
               dxSrc,
               dySrc,
               SRCCOPY);

    SelectObject(hMemDCsrc, hOldBitmap1);
    SelectObject(hMemDCdst, hOldBitmap2);
    }

  if (hOldPal1)
    SelectPalette (hMemDCsrc, hOldPal1, FALSE);

  if (hOldPal2)
    SelectPalette (hMemDCdst, hOldPal1, FALSE);

  DeleteDC (hMemDCsrc);
  DeleteDC (hMemDCdst);

  return hNewBm;
}

//---------------------------------------------------------------------
// Purpose:    Given a handle to a palette, returns the # of colors
//             in that palette.
// Parms:      hPal == Handle to palette we want info on.
// Returns:    # of colors in the palette.
// History:   Date      Reason
//             6/01/91  Created
//             9/13/95  Ported into MFC for CDib class
//---------------------------------------------------------------------
WORD CDib::ColorsInPalette (HPALETTE hPal)
{
  int nColors;
  if (!hPal)
    return 0;
  GetObject(hPal, sizeof (nColors), (LPSTR) &nColors);
    return nColors;
}

//---------------------------------------------------------------------
// Purpose:   Duplicate a given palette, changing all the flags in
//            it to a certain flag value (i.e. peFlags member of
//            the PALETTEENTRY structure).
// Parms:     hPal     == Handle to palette to duplicate.
//            bNewFlag == New peFlags PALETTEENTRY flag.  Set
//                        to DONT_CHANGE_FLAGS if don't want
//                        to touch the flags.
// Returns:   Handle to the new palette.  NULL on error.
// History:   Date      Reason
//            6/01/91  Created
//            9/13/95  Ported into MFC for CDib class
//---------------------------------------------------------------------
HPALETTE CDib::CopyPaletteChangingFlags(HPALETTE hPal, BYTE bNewFlag)
{
  WORD         wEntries, i;
  HANDLE       hLogPal;
  LPLOGPALETTE lpLogPal;

  if (!hPal)
    return NULL;

  wEntries = ColorsInPalette(hPal);

  if (!wEntries)
    return NULL;

  hLogPal = GlobalAlloc(GHND, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * wEntries);

  if (!hLogPal)
     return NULL;

  lpLogPal = (LPLOGPALETTE)GlobalLock(hLogPal);

  lpLogPal->palVersion    = PALVERSION;
  lpLogPal->palNumEntries = wEntries;

  GetPaletteEntries(hPal, 0, wEntries, lpLogPal->palPalEntry);

  if (bNewFlag != DONT_CHANGE_FLAGS)
    for (i = 0;  i < wEntries;  i++)
      lpLogPal->palPalEntry[i].peFlags = bNewFlag;

  hPal = ::CreatePalette(lpLogPal); // re-use input param as output

  GlobalUnlock (hLogPal);
  GlobalFree (hLogPal);

  return hPal;
}

//---------------------------------------------------------------------
// Purpose:    Makes a copy of the given global memory block.  Returns
//             a handle to the new memory block (NULL on error).
//             Routine stolen verbatim out of ShowDIB.
// Parms:      h == Handle to global memory to duplicate.
// Returns:    Handle to new global memory block.
//---------------------------------------------------------------------
HANDLE WINAPI CDib::CopyHandle (HANDLE h)
{
  BYTE huge *lpCopy;
  BYTE huge *lp;
  HANDLE     hCopy;
  DWORD      dwLen;

  if (h == NULL)
    return NULL;

  dwLen = ::GlobalSize((HGLOBAL) h);

  if ((hCopy = (HANDLE) ::GlobalAlloc (GHND, dwLen)) != NULL)
    {
    lpCopy = (BYTE __huge *) ::GlobalLock((HGLOBAL) hCopy);
    lp     = (BYTE __huge *) ::GlobalLock((HGLOBAL) h);

    while (dwLen--)
      *lpCopy++ = *lp++;

    ::GlobalUnlock((HGLOBAL) hCopy);
    ::GlobalUnlock((HGLOBAL) h);
    }
  return hCopy;
}

//---------------------------------------------------------------------
// Purpose:    Renders an object for the clipboard.  The format is
//             specified in the "cf" variable (either CF_BITMAP,
//             CF_DIB, or CF_PALETTE). This "local" version is called
//             by the global version, which uses member variables to
//             supply the parameters, and also recursively by itself.
//
//             Originally stolen almost verbatim out of ShowDIB, I have
//             "Cplusplussed" it and used some MFC features.
//
//
// Parms:      hWndClip == Window clipboard belongs to, and where our
//                         image is stored).
//             cf       == Format to render (CF_BITMAP, CF_DIB, CF_PALETTE)
//             ptDIBSize== Size of the DIB in the given window.
//
// History:   Date      Reason
//             ???      Created
//            9/13/95   Modified for CDib class
//---------------------------------------------------------------------
HANDLE CDib::LocalRenderFormat(HWND hWndClip, int cf, CSize DIBSize)
{
  HANDLE    h = NULL;
  HBITMAP   hBitmap;
  HPALETTE  hPalette;        // Handle to the bitmap's palette.

  if (!hWndClip)
    return NULL;

  hPalette     = m_hPalette;
  hBitmap      = DIBToBitmap(m_hDIB, m_hPalette);

  switch (cf)
    {
    case CF_BITMAP:
      h = CropBitmap(hBitmap, hPalette, m_ClipRect, DIBSize);
    break;
    case CF_DIB:
      {
      HBITMAP hbm;

  // NOTE: For simplicity, use the display device to crop the bitmap.
  // This may result in a loss of color precision (if the display
  // device has less color precision than the DIB). This isn't usually
  // a problem, as users shouldn't really be editing images on devices
  // that can't display them.

  // Recursive call of this function...

      hbm = (HBITMAP)LocalRenderFormat(hWndClip, CF_BITMAP, DIBSize);
      if (hbm)
         {
         h = BitmapToDIB(hbm, hPalette);
         DeleteObject (hbm);
         }
      }
    break;
    case CF_PALETTE:
      if (hPalette)
        h = CopyPaletteChangingFlags(hPalette, 0);
    break;
    }
  return h;
}

// This gets called when an application wants to perform the 
// Edit - Paste action, and the system knows that our app
// has indicated it can paste the appropriate type of data
// onto the clipboard
///////////////////////////////////////////////////////////
HANDLE CDib::RenderFormat(UINT nFormat)
{
  HANDLE h;
  if (!m_hWndClip)
    return NULL;
  AfxGetApp()->BeginWaitCursor();
  h = LocalRenderFormat(m_hWndClip, nFormat, m_ClipSize);
  AfxGetApp()->EndWaitCursor();
  return h;
}

// This gets called when a user chooses Edit - Copy in our application.
// This routines signals Windows that certain data types area available,
// and saves some program state info to allow access to the data later
/////////////////////////////////////////////////////////////////////////////////////
void CDib::EditCopy(CWnd *pWnd, 
                    const CRect& SourceRegion, const CRect& ClipRegion)
{
  // Clean clipboard of contents, and tell it we can render
  //  a DIB, a DDB, and/or a palette.

  if (OpenClipboard (pWnd->GetSafeHwnd()))
    {
    EmptyClipboard();
    SetClipboardData(CF_DIB ,NULL);     // DIB's have their own palette
    SetClipboardData(CF_BITMAP  ,NULL); // but DDB's don't...
    SetClipboardData(CF_PALETTE ,NULL); // so offer to supply one
    CloseClipboard();

    // Set our globals to tell our app which child window
    // owns the clipboard, and the clipping rectangle at
    // the time of the copy.  If the clipping rectangle is
    // empty, then use the entire DIB window.

    m_hWndClip = pWnd->GetSafeHwnd();
    m_ClipRect = ClipRegion;
    m_ClipSize = SourceRegion.Size();
    }
}   

// Not implemented yet. In my application, I would need to scale up the
// bitmap info and put it back into data memory as 32-bit words, or
// whatever
//////////////////////////////////////////////////////////////////////////
void CDib::EditPaste(CWnd *pWnd, const CRect& DestRegion)
{
  // TODO: Add your command handler code here
}

LRESULT CDib::DoPaletteChanged(WPARAM wParam, CFrameWnd* pAppFrame, HWND hWnd, CDocument *pDoc)
{
  ASSERT(wParam != NULL);

  if (!m_hDIB || !m_hPalette)
    return(0L);
    
  CPalette  Pal, *pPal;
  pPal = CPalette::FromHandle(m_hPalette);

  if (pPal != NULL)
    {
    CClientDC appDC(pAppFrame);
    // All views but one should be a background palette.
    // wParam contains a handle to the active view, so the SelectPalette
    // bForceBackground flag is FALSE only if wParam == m_hWnd (this view)
    CPalette* oldPalette = appDC.SelectPalette(pPal, ((HWND)wParam) != hWnd);
    if (oldPalette != NULL)
      {
      UINT nColorsChanged = appDC.RealizePalette();
      if (nColorsChanged > 0)
        pDoc->UpdateAllViews(NULL);
      appDC.SelectPalette(oldPalette, TRUE);
      }
    else
      {
      TRACE0("\tSelectPalette failed in CDib::DoPaletteChanged\n");
      }
  }
  return 0L;
}
