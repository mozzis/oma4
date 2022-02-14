//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 21:00:34  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/VIEW.CPV  $
// 
//    Rev 1.1   29 Sep 1995 21:00:34   MLM
// Add PVCS info tags
// 
//////////////////////////////////////////////////////////////////////////
// View.cpp : implementation of the CBasicView class
//

#include "stdafx.h"
#include "limits.h"
#include "basedisp.h"
#include "docuif.h"
#include "oma4driv.h"
#include "mainfrm.h"
#include "mydlgbar.h"
#include "cdib.h"
#include "rectinfo.h"
#include "View.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

short CELL_X = 4;
short CELL_Y = 4;

static BOOL JustDrawTracker = FALSE;
/////////////////////////////////////////////////////////////////////////////
// CBasicView

IMPLEMENT_DYNCREATE(CBasicView, CScrollView)

BEGIN_MESSAGE_MAP(CBasicView, CScrollView)
  //{{AFX_MSG_MAP(CBasicView)
  ON_WM_CREATE()
  ON_WM_ERASEBKGND()
  ON_WM_MOUSEMOVE()
  ON_WM_LBUTTONDOWN()
  ON_WM_SETCURSOR()
  ON_WM_DESTROY()
  ON_VBXEVENT(VBN_CLICK, IDC_BTNZFRM, OnClickBtnzfrm)
  ON_VBXEVENT(VBN_CLICK, IDC_BTNZOOM, OnClickBtnzoom)
  ON_VBXEVENT(VBN_CLICK, IDC_BTNVFLIP, OnClickBtnvflip)
  ON_VBXEVENT(VBN_CLICK, IDC_BTNHFLIP, OnClickBtnhflip)
  ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
  ON_WM_RENDERFORMAT()
  ON_WM_RENDERALLFORMATS()
  ON_MESSAGE(WM_DOREALIZE, OnDoRealize)
  ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
  ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
  ON_WM_KEYDOWN()
  ON_COMMAND(IDS_AUTOSCALE, OnAutoScale)
  ON_COMMAND(IDS_AUTOAUTO, OnAutoAuto)
  ON_UPDATE_COMMAND_UI(IDC_BTNZFRM, OnUpdateControl)
  ON_UPDATE_COMMAND_UI(IDS_AUTOAUTO, OnUpdateAutoAuto)
  ON_COMMAND(IDM_PROC1, OnProc1)
  ON_COMMAND(IDM_PROC2, OnProc2)
	ON_COMMAND(ID_AUTOSIZE_WINDOW, OnAutosizeWindow)
	ON_COMMAND(ID_OPT_1TO1, OnOpt1to1)
	ON_UPDATE_COMMAND_UI(ID_OPT_1TO1, OnUpdateOpt1to1)
  ON_UPDATE_COMMAND_UI(IDC_BTNVFLIP, OnUpdateControl)
  ON_UPDATE_COMMAND_UI(IDC_BTNHFLIP, OnUpdateControl)
  ON_UPDATE_COMMAND_UI(IDC_BTNZOOM, OnUpdateControl)
  ON_UPDATE_COMMAND_UI(IDS_AUTOSCALE, OnUpdateControl)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
  // Standard printing commands
  ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
  ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBasicView construction/destruction

CBasicView::CBasicView()
{
  DataMin = 0L;
  DataMax = 131072L;
  Briteness = 100;
  Contrast = 100;
  AutoAutoOn = FALSE;
  ZoomRectState = ZRectOff;
  XDrawDirection = XDrawRight;
  YDrawDirection = YDrawDown;
  m_pCDib = 0;
  m_bOneToOne = TRUE;
  SourceRect.SetRectEmpty();
  OutputRect.SetRectEmpty();
}

CBasicView::~CBasicView()
{
  if (m_pCDib)
    delete m_pCDib;
  m_pCDib = 0;
}

////////////////////////////////////////////////////////
// return long product of two shorts. Provided to
// improve readability of some following operations
static inline long DMult(short A, short B)
{
  return (long)((long)A * (long)B);
}

// Called when the view is first connected with a 
// document
///////////////////////////////////////////////////////
void CBasicView::OnInitialUpdate(void)
{
  char buffer[40];
  CScrollView::OnInitialUpdate();
  // Put image dimensions into window title
  CDocUif *pDoc = GetDocument();
  CSize szDoc = pDoc->GetDocSize();
  CString Title = pDoc->GetTitle();
  wsprintf(buffer, " (%d X %d)", szDoc.cx, szDoc.cy);
  Title += buffer;
  pDoc->SetTitle(Title);

  if (theDlgBar)
    {
    theDlgBar->SetTextDMax(" ");
    theDlgBar->SetTextDMin(" ");
    }

  AutoAutoOn = FALSE;
  ZoomRectState = ZRectOff;
  Briteness = Contrast = -1;

  // Initialize the CRectTracker object
  m_tracker.m_nHandleSize = 8;
  m_tracker.m_nStyle = CRectTracker::dottedLine | 
                       CRectTracker::resizeInside;

  UnZoomSourceRect();
  SetScrollSizes(MM_TEXT, szDoc);
}

// Called if document data changes or if window uncovered
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{ 
  if (lHint) // if message originated with OnOpenDocument
    {
    CRect rect;  // resize frame to match image
    GetClientRect(&rect);
    OnSize(SIZE_RESTORED, rect.Width(), rect.Height());
    }

  if (AutoAutoOn)
    OnAutoScale();  
  if (m_bOneToOne)
    {
    CRect UpdateRect;
    GetClientRect(&UpdateRect);
    UpdateRect.IntersectRect(UpdateRect, SourceRect);
    InvalidateRect(&UpdateRect, FALSE); // Redraw all of client
    }
  else
    CScrollView::OnUpdate(pSender, lHint, pHint);
  RefreshRectInfo();           // update dialog box contents if up
}

// Only erase the background if the image does not fill the client area
// This allows faster display update if the window is sized to be the same
// as the image size.
/////////////////////////////////////////////////////////////////////////////
BOOL CBasicView::OnEraseBkgnd(CDC* pDC)
{
  CRect CliRect;
  GetClientRect(&CliRect);
  CSize CliSize = CliRect.Size();
  CSize OutSize = OutputRect.Size();
  // if the image does not fill the client area, erase client area
  if (CliSize.cx > OutSize.cx+1 || CliSize.cy > OutSize.cy+1)
    {
    CWnd* pParentWnd = GetParent();
    HBRUSH hBrush = (HBRUSH)pParentWnd->SendMessage(WM_CTLCOLOR,
      (WPARAM)pDC->m_hDC, MAKELONG(pParentWnd->m_hWnd, IDR_MAINFRAME));
    if (hBrush)
      pDC->FillRect(&CliRect, CBrush::FromHandle(hBrush));
    }
  return(1L); // indicate background erased
}

// adjust device context for printing or displaying
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnPrepareDC(CDC * pDC, CPrintInfo *pInfo)
{
  if (pInfo) // If displaying to printer
    {
    pDC->SetMapMode(MM_ANISOTROPIC);           // Need to control extents
    CScrollView::OnPrepareDC(pDC, pInfo);      // Call default first
    pDC->SetWindowOrg(SourceRect.TopLeft());   // Set coords for stretch to page
    pDC->SetWindowExt(SourceRect.Size());
    pDC->SetViewportOrg(0,0);                  // Set orientation
    OutputRect =                               // Try for best aspect ratio
      CRect(0, 0, pDC->GetDeviceCaps(HORZRES)-1,
                  pDC->GetDeviceCaps(VERTRES)-1);
    if (OutputRect.Width() * pDC->GetDeviceCaps(HORZRES) < 
        OutputRect.Height() * pDC->GetDeviceCaps(VERTRES))
      OutputRect.bottom = OutputRect.right;
    pDC->SetViewportExt(OutputRect.Size());
    }
  else  // screen output
    {
    CScrollView::OnPrepareDC(pDC, pInfo);      // Call default first
    pDC->SetMapMode(MM_TEXT);
    if (m_bOneToOne) // not zoom mode
      OutputRect = CRect(CPoint(0, 0), GetTotalSize());
    else
      GetClientRect(&OutputRect);   // zoom mode
    }
}               

// CBasicView drawing
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnDraw(CDC* pDC)
{
  CDocUif* pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  CSize szDib;
  long Offset;
  float Scale;

  // IsAdjusted() checks to see if brightness or contrast changed since the
  // last time the data was converted to a bitmap. If so, need to convert 
  // again to scale data down to 8-bits.
  // Updated() checks to see if new data arrived from detector or if image
  // was modified since the last conversion.
  // m_pCDib is a pointer to a custom DIB handler class.
  if (!m_pCDib || IsAdjusted() || pDoc->Updated()) // if no bitmap or need new one
    {
    if (m_pCDib)  // if old DIB exists delete it
      delete m_pCDib;

    szDib = pDoc->GetDocSize(); // if no document
    if (szDib == CSize(0, 0))  
      return;
      
    BriteAndContCalc(&Offset, &Scale);
    // Creating DIB performs data conversion to bitmap
    if (theMainFrame->GetDelayUpdate())
      AfxGetApp()->BeginWaitCursor();
    m_pCDib = new CDib(pDC, pDoc->GetDataPtr(), &szDib, Offset, Scale);
    pDoc->Updated(FALSE);
    if (theMainFrame->GetDelayUpdate())
      AfxGetApp()->EndWaitCursor();
    }
  
  if (!JustDrawTracker && m_pCDib)
    m_pCDib->Paint(pDC->GetSafeHdc(), &OutputRect, &SourceRect, XDrawDirection, 
                   YDrawDirection);

  // draw tracker (Zoom Rectangle) if turned on
  if (ZoomRectState == ZRectOn && !pDC->IsPrinting()) 
    m_tracker.Draw(pDC);
  JustDrawTracker = FALSE;

  // update value display at mouse position
  POINT lpt;
  GetCursorPos((POINT far *)&lpt); //gets mouse position in screen coordinates
  ScreenToClient(&lpt);
  OnMouseMove(0, lpt); // update value display since data may be different 
  RefreshRectInfo();
}

// CBasicView printing
// Set up print setup dialog box values
/////////////////////////////////////////////////////////////////////////////
BOOL CBasicView::OnPreparePrinting(CPrintInfo* pInfo)
{
  BOOL Success;

  if (theDlgBar && theDlgBar->IsVisible() && pInfo->m_bPreview)
    {
    theDlgBar->DelayHide();
    GetParentFrame()->RecalcLayout();
    }

  pInfo->m_pPD->m_pd.nFromPage = 1;
  pInfo->m_pPD->m_pd.nToPage = 1;
  pInfo->m_pPD->m_pd.nMinPage = 1;
  pInfo->m_pPD->m_pd.nMaxPage = 1;
  pInfo->m_pPD->m_pd.nCopies = 1;
  pInfo->m_nNumPreviewPages = 1;
  pInfo->m_nCurPage = 1;
  Success = DoPreparePrinting(pInfo); // default preparation
  return Success;
}

// Display hourglass cursor during printing
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnBeginPrinting(CDC* pDC, CPrintInfo* /*pInfo*/)
{                      
  AfxGetApp()->BeginWaitCursor();
}

// Printing - add title at top of printout
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnPrint(CDC *pDC, CPrintInfo *pInfo)
{
  CString szHeader = GetDocument()->GetTitle();
  CSize HeaderSize = pDC->GetTextExtent(szHeader, strlen(szHeader));
  // print the header at top center
  pDC->TextOut((OutputRect.Width() / 2) - (HeaderSize.cx / 2), HeaderSize.cy, szHeader);
  // subtract out from the draw rect the space used by the header
  OutputRect.top += HeaderSize.cy * 2;
  OnDraw(pDC);                  // Draw uses SourceRect
}

/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
  // Turn off hourglass cursor
  AfxGetApp()->EndWaitCursor();

  if (theDlgBar && pInfo->m_bPreview)
    {
    theDlgBar->DelayShow();
    GetParentFrame()->RecalcLayout();
    }
}

/////////////////////////////////////////////////////////////////////////////
// CBasicView diagnostics

#ifdef _DEBUG
void CBasicView::AssertValid() const
{
  CScrollView::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////
void CBasicView::Dump(CDumpContext& dc) const
{
  CScrollView::Dump(dc);
}

/////////////////////////////////////////////////////////////////////////////
CDocUif* CBasicView::GetDocument() // non-debug version is inline
{
  ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDocUif)));
  return (CDocUif*)m_pDocument;
}
#endif //_DEBUG
  
/////////////////////////////////////////////////////////////////////////////
// CBasicView message handlers


//////////////////////////////////////////////////////////////////////////////
int CBasicView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CScrollView::OnCreate(lpCreateStruct) == -1)
    return -1;                 

  // Setup for OMA data display
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
void CBasicView::OnDestroy()
{
  CScrollView::OnDestroy();
}

// Return truth that brightness or contrast were adjusted since
// the last redraw
//////////////////////////////////////////////////////////////////////////////
BOOL CBasicView::IsAdjusted(void)
{
  return (Briteness != theMainFrame->getBriteness() ||
         Contrast != theMainFrame->getContrast());
}

// Convert values from frame window controls into image-dependent scaling values
////////////////////////////////////////////////////////////////////////////////
void CBasicView::BriteAndContCalc(long *Offset, float *Scale)
{
  float Diff = DataMax - DataMin, Factor;
  
  Briteness = theMainFrame->getBriteness(); // resets IsAdjusted() state
  Contrast = theMainFrame->getContrast();
  
  *Offset = (long)(DataMin + (Diff * ((Briteness / 100.0) - 1.0F)));
  Factor = Diff / (float)(COLOR_RANGE);
  *Scale = Factor + (Factor * ((Contrast / 100.0F) - 1.0F));
  
  ASSERT(*Scale != 0.0F);
}

// Set DataMax and DataMin from data - used later to calc Scale and Offset
// for data converstion to bitmap 
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnAutoScale(void)
{
  long TempMin = LONG_MAX, 
       TempMax = LONG_MIN;
  CDocUif * pDoc = GetDocument();
  ULONG __huge *pTemp, __huge *pData = pDoc->GetDataPtr();
  CSize szImg = pDoc->GetDocSize();
  short StartX, StartY, EndX, EndY;

  if (ZoomRectState == ZRectOn) // selection rectangle displayed
    {
    CRect Rect;
    MapZRectToDoc(Rect);  // convert client to image coords
    StartX = Rect.left;
    EndX = Rect.right;
    StartY = Rect.top;
    EndY = Rect.bottom;
    CELL_X = CELL_Y = 1;
    }
  else  // Source is whole image - note Source Rect
    {   // coordinates are always already in image space
    StartX = SourceRect.left;
    EndX = SourceRect.right;
    StartY = SourceRect.top;
    EndY = SourceRect.bottom;
    CELL_X = CELL_Y = 4;
    }

  if (StartX > szImg.cx) StartX = 0;
  if (EndX >= szImg.cx) EndX = szImg.cx - 1;
  if (StartY > szImg.cy) StartY = 0;
  if (EndY >= szImg.cy) EndY = szImg.cy - 1;

  for (short i = StartY; i < EndY; i += CELL_Y)
    {
    pTemp = &pData[DMult(i, szImg.cx)];
    for (short j = StartX; j < EndX; j += CELL_X)
      {
      long Temp = pTemp[j];
      if (Temp < TempMin) TempMin = Temp;
      if (Temp > TempMax) TempMax = Temp;
      }
    }

  // Use the dialog bar text boxes to
  // display max and min value found
  if (theDlgBar)
    {
    char buffer[40];
    wsprintf(buffer, "%ld", TempMax);
    theDlgBar->SetTextDMax(buffer);
    wsprintf(buffer, "%ld", TempMin);
    theDlgBar->SetTextDMin(buffer);
    }

  // fudge a little 
  DataMax = (float)TempMax * 1.01F;
  DataMin = (float)TempMin * .99F;
  
  theMainFrame->InitBriteAndCont();
  Briteness = Contrast = -1;      // Make IsAdjusted() return True
  Invalidate(FALSE);
}

// "Auto-AutoScale" - if on, recalc grayscale before every redraw
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnAutoAuto(void)
{
  if (AutoAutoOn)
      AutoAutoOn = FALSE;
  else
      AutoAutoOn = TRUE;
}

// Generic function to let controls be active - sometimes MFC must see override
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnUpdateControl(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(TRUE);
}

// Enable and check or uncheck the Auto-Autoscale menu item
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnUpdateAutoAuto(CCmdUI* pCmdUI)
{
  if (AutoAutoOn)
    pCmdUI->SetCheck(1);
  else
    pCmdUI->SetCheck(0);
}

// Update status bar display of X, Y and image pixel value 
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnMouseMove(UINT nFlags, CPoint point)
{
  char buffer[40];
  CDocUif *pDoc = GetDocument();
  CSize DocSize = pDoc->GetDocSize();
  CSize SrcSize = SourceRect.Size();  // Size of image part displayed
  ULONG __huge *pData = pDoc->GetDataPtr(); 

  if (SrcSize.cx <= 0 || SrcSize.cy <= 0)
    return;

  MapPointToDoc(point);

  if (point.x > SourceRect.right)
    point.x = SourceRect.right;
  if (point.x < 0)
    point.x = 0;
  if (point.y > SourceRect.bottom)
    point.y = SourceRect.bottom;
  if (point.y < 0)
    point.y = 0;

  // Display mouse X-position, Y-position, and the image value at that position
  // in frame windows status bar windows
  wsprintf(buffer, "X=%d", point.x);
  theMainFrame->StatusTextSet(buffer, 1);
  wsprintf(buffer, "Y=%d", point.y);
  theMainFrame->StatusTextSet(buffer, 2);
  pData = &pData[(long)((long)point.x + (long)((long)point.y * DocSize.cy))];
  wsprintf(buffer, "V=%ld", *pData);
  theMainFrame->StatusTextSet(buffer, 3);

  CScrollView::OnMouseMove(nFlags, point);
}

void __far __cdecl CBasicView::UpdateMouse(UINT flags, CPoint point)
{
  // OnMouseMove(flags, point);
}

// Draw the tracker (Zoom Rect)
/////////////////////////////////////////////////////////////////////////////
void CBasicView::DrawTracker(void)
{
  if (ZoomRectState == ZRectOn)
    {
    CClientDC dc(this);
    OnPrepareDC(&dc);
    m_tracker.Draw(&dc);
    }
  RefreshRectInfo();
}

// The input point is in client coordinates, i.e. where 0,0 is the
// upper left corner of the visible area. The result is in image
// coordinates, i.e. 0,0 is the start of the image
// bFlip is used to tell whether to flip the image according to 
// display coordinates or DIB coordinates (i.e. EditCopy)
/////////////////////////////////////////////////////////////////////
void CBasicView::MapPointToDoc(CPoint& point, BOOL bFlip)
{
  CSize DocSize = GetDocument()->GetDocSize(); // Size of entire image
  CSize DstSize = OutputRect.Size();
  CSize SrcSize = SourceRect.Size();

  if (!DstSize.cx || !DstSize.cy)
    return;

  point.Offset(GetScrollPosition());

  // account for flipping and mirroring
  if (XDrawDirection == XDrawLeft)
    point.x = DocSize.cx - point.x;
  
  // DIB's on clipboard are upside down vs. display
  if ((YDrawUp == YDrawDirection && bFlip) || (YDrawDown == YDrawDirection  && !bFlip))
    point.y = DocSize.cy - point.y;

  float ratiox = (float)((float)SrcSize.cx / (float)DstSize.cx);
  float ratioy = (float)((float)SrcSize.cy / (float)DstSize.cy);

  point.x = (int)((float)point.x * ratiox);
  point.y = (int)((float)point.y * ratioy);

  point.Offset(SourceRect.TopLeft());
}

// Given a logical coordinate rectangle, map to image space
///////////////////////////////////////////////////////////
void CBasicView::MapRectToDoc(CRect& rect, BOOL bFlip)
{
  TRACE("InRect   X: %d - %d (%d), Y: %d - %d (%d)\n",
    rect.left,rect.right,rect.Width(),rect.top,rect.bottom,rect.Height());
  MapPointToDoc(rect.TopLeft(), bFlip);
  MapPointToDoc(rect.BottomRight(), bFlip);
  TRACE("OutRect  X: %d - %d (%d), Y: %d - %d (%d)\n",
    rect.left,rect.right,rect.Width(),rect.top,rect.bottom,rect.Height());
}

// fill return CRect with Zoom Rect coords in client space
/////////////////////////////////////////////////////////////////////////////
void CBasicView::MapZRectToDoc(CRect& rect)
{
  rect = m_tracker.m_rect;
  MapRectToDoc(rect);
}

// Reset Source and Zoom rectangle coordinates to values for full image
/////////////////////////////////////////////////////////////////////////////
void CBasicView::UnZoomSourceRect(void)
{
  CSize szImg = GetDocument()->GetDocSize();

  SourceRect.left = SourceRect.top = 0;
  SourceRect.right = szImg.cx - 1;
  SourceRect.bottom = szImg.cy - 1;
  m_tracker.m_rect = SourceRect;
}

//  Zoom image so only region in ZoomRect is displayed
/////////////////////////////////////////////////////////////////////////////
void CBasicView::ZoomToRect(void)
{
  if (ZRectOn == ZoomRectState)
    {
    // Want to set source rect to image coords of ZRect
    // be careful, mapZrectToDoc uses source rect
    // so map onto a temp, then copy it into source rect
    CRect SrcRect;
    MapZRectToDoc(SrcRect);
    SrcRect.NormalizeRect();
    m_tracker.m_rect = SourceRect = SrcRect;
    ZoomRectState = ZRectOff;
    }
  else // called as result of Zoom button with no rect on - means unzoom
    {
    UnZoomSourceRect(); 
    SetScrollSizes(MM_TEXT, GetDocument()->GetDocSize());
    }
  if (theDlgBar)
    theDlgBar->m_pBtnZfrm->SetNumProperty("Value", 0);
  Invalidate(FALSE);
}

// Turn on display of the tracker (Zoom Rect)
/////////////////////////////////////////////////////////////////////////////
void CBasicView::ZoomRectOn(void)
{
  ZRectState OldState = ZoomRectState;
  ZoomRectState = ZRectOn;
  if (ZRectOn != OldState)       // if it wasn't on before
    {                            // display it within client region
    GetClientRect(&m_tracker.m_rect);
    m_tracker.m_rect.IntersectRect(m_tracker.m_rect, OutputRect);
    // initial size - centered just inside image
    m_tracker.m_rect.InflateRect(m_tracker.m_rect.Width() / -10, m_tracker.m_rect.Height() / -10);
    }
  else  // have to erase old tracker
    Invalidate(FALSE);
  DrawTracker();
}

// Turn off display of the tracker (Zoom Rect)
/////////////////////////////////////////////////////////////////////////////
void CBasicView::ZoomRectOff(void)
{
  ZRectState OldState = ZoomRectState;
  ZoomRectState = ZRectOff;
  m_tracker.m_rect = SourceRect; // so Edit Copy will use whole area
  if (ZRectOff != OldState)
    Invalidate(FALSE);
}

// Create the Zoom Rect when the left button is down in the image
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnLButtonDown(UINT nFlags, CPoint point)
{
  MyTrack tracker = m_tracker; // use a temporary tracker to save the old one
  CRect CliRect;
  GetClientRect(&CliRect);
  // Limit tracker rectangle to client area and to image area
  CliRect.IntersectRect(OutputRect, CliRect);
  m_tracker.SetLimit(CliRect);

   // if point not on tracker rect
  if (ZoomRectState == ZRectOff || 
      m_tracker.HitTest(point) == CRectTracker::hitNothing)
    {
    if (m_tracker.TrackRubberBand(this, point, CBasicView::UpdateMouse, TRUE))
      {
      ZoomRectState = ZRectOn;
      // turn on the "permanent" tracker rectangle
      if (theDlgBar)
        theDlgBar->m_pBtnZfrm->SetNumProperty("Value", (long)TRUE);
      DrawTracker();
      m_tracker.m_rect.NormalizeRect();
      }
    else
      m_tracker = tracker; // restore previous if error or empty
    } 
    // If ZoomRect was on and a mouse hit happened, just move
    // or size the existing ZoomRect.
  else if (ZoomRectState == ZRectOn && m_tracker.Track(this, point, FALSE))
    Invalidate(FALSE);
  else  // no mouse hit, do default action
    CView::OnLButtonDown(nFlags, point);
   RefreshRectInfo();
}

// Want to change cursor if it is over the Zoom (tracker) rect
/////////////////////////////////////////////////////////////////////////////
BOOL CBasicView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
  // forward to tracker
  CDocUif* pDoc = GetDocument();
  if (pWnd == this && ZoomRectState == ZRectOn && m_tracker.SetCursor(this, nHitTest))
    return TRUE;
  
  return CView::OnSetCursor(pWnd, nHitTest, message);
}

// Display or hide the Zoom (tracker) frame
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnClickBtnzfrm(UINT, int, CWnd*, LPVOID)
{
  if (theDlgBar && theDlgBar->m_pBtnZfrm->GetNumProperty("Value"))
    ZoomRectOn();
  else
    ZoomRectOff();
}

//  Respond to Zoom button - Zoom image so only region in ZoomRect is displayed
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnClickBtnzoom(UINT, int, CWnd*, LPVOID)
{
  ZoomToRect();
}

// Flip the image vertically
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnClickBtnvflip(UINT, int, CWnd*, LPVOID)
{
  if (YDrawDirection == YDrawDown) // toggle the flag
    YDrawDirection = YDrawUp;
  else
    YDrawDirection = YDrawDown;

  Invalidate(FALSE);  // Cause a repaint
}

// Flip the image horizontally
/////////////////////////////////////////////////////////////////////////////
void CBasicView::OnClickBtnhflip(UINT, int, CWnd*, LPVOID)
{
  if (XDrawDirection == XDrawRight) // toggle the flag
    XDrawDirection = XDrawLeft;
  else
    XDrawDirection = XDrawRight;

  Invalidate(FALSE);  // Cause a repaint
}

//////////////////////////////////////////////////////////////////////////
//// Clipboard support

// Signal clipboard that data is ready, and
// save the rectangle pointing to the data
//////////////////////////////////////////////////////////////////////////
void CBasicView::OnEditCopy()
{
  CRect ClipRect = m_tracker.m_rect;
  CSize DocSize = GetDocument()->GetDocSize();
  CRect SrcRect(CPoint(0,0), DocSize);
  if (ClipRect != SourceRect)
    MapRectToDoc(ClipRect, TRUE);
  // DIBs are stored upside down from the way they are displayed!
  // So must flip the rect to the opposite side of the image
  ClipRect.top = DocSize.cy - ClipRect.top;
  ClipRect.bottom = DocSize.cy - ClipRect.bottom;
  ClipRect.NormalizeRect();
  RefreshRectInfo();
  m_pCDib->EditCopy(this, SrcRect, ClipRect);
}

// Additional transfer possibilities:
// OnEditCopy:
// UINT CFormat = RegisterClipboardFormat(lpszFormatName);
// SetClipboardData(h_ClipData = GlobalLock(pData), CFormat);
// Then also worry about maintaining h_ClipData.

// When another app invokes Paste, this supplies the data to the clipboard
////////////////////////////////////////////////////////////////////////////
void CBasicView::OnRenderFormat(UINT nFormat)
{
  HANDLE hClipBoardData;

  hClipBoardData = m_pCDib->RenderFormat(nFormat);

  if (hClipBoardData)
    SetClipboardData(nFormat, hClipBoardData);
}

// When this app is closed with clipboard data pending, 
// this flushes the data out to the clipboard
////////////////////////////////////////////////////////////////////////////
void CBasicView::OnRenderAllFormats(void)
{
  HANDLE hClipBoardData;

  if (OpenClipboard())
    {
     EmptyClipboard();
     if (hClipBoardData = m_pCDib->RenderFormat(CF_DIB))
       SetClipboardData(CF_DIB, hClipBoardData);
     if (hClipBoardData = m_pCDib->RenderFormat(CF_BITMAP))
       SetClipboardData(CF_BITMAP, hClipBoardData);
     if (hClipBoardData = m_pCDib->RenderFormat(CF_PALETTE))
       SetClipboardData(CF_PALETTE, hClipBoardData);
     CloseClipboard();
     }
}

// No Edit Paste action yet!!
//////////////////////////////////////////////////////////////////////////
void CBasicView::OnEditPaste()
{
}

void CBasicView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(FALSE /* ::IsClipboardFormatAvailable(CF_BITMAP) */);
}

void CBasicView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  // TODO: Add your message handler code here and/or call default
  
  CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

// Put little white crosses into image at regular intervals
// Sets data at the cross location to DataMax+1%. Since the 
// Autoscale routine always backs off the display max by 5%,
// this guarantees that the crosses will apppear as white.
// WARNING - this modifies the data!!!!
//////////////////////////////////////////////////////////////////////////
void CBasicView::Crosses(const int CellSize)
{
  CDocUif *pDoc = GetDocument();
  if (!pDoc)
    return;
  CSize DocSize = pDoc->GetDocSize();
  if (DocSize.cx <=0 || DocSize.cy <= 0)
    return;
  ULONG __huge * pData = pDoc->GetDataPtr();

  for (int i = CellSize;i < DocSize.cy;i+=CellSize)
    {
    for (int j = CellSize;j < DocSize.cx-5;j+=CellSize)
      {
      long offset = (long)((long)j+(long)i*DocSize.cx);
      for (long k = offset - 5;k < offset + 5;k++) // do the horizontal line
        {
        pData[k] = (ULONG)(DataMax + DataMax/100);
        }
      if (i < DocSize.cy-5) // do the vertical line
        {
        for (k=-5;k <= 5;k++)
          {
          pData[offset + k*DocSize.cx] = (ULONG)(DataMax + DataMax/100);
          }
        }
      }
    } 
  pDoc->SetModifiedFlag(); // So framework will warn to save upon exit
  pDoc->Updated(TRUE);     // So new bitmap is generated
  Invalidate(FALSE);       // So repaint occurs.
}

// Do the crosses every 10 pixels
//////////////////////////////////////////////////////////////////////////
void CBasicView::OnProc1()
{
  Crosses(10);
}
// Do the crosses every 100 pixels
//////////////////////////////////////////////////////////////////////////
void CBasicView::OnProc2()
{
  Crosses(100);
}

// Update the information in the info dialog
//////////////////////////////////////////////////////////////////////////
void CBasicView::RefreshRectInfo(void)
{
  CRectInfo *pRectInfo = theMainFrame->GetRectInfoPtr();
  
  if (!pRectInfo)
    return;

  CRect ClipRect = m_tracker.m_rect;

  if (ZRectOn == ZoomRectState)
    MapRectToDoc(ClipRect);

  if (pRectInfo)
    {
    pRectInfo->m_SRC_TLX = SourceRect.left;
    pRectInfo->m_SRC_TLY = SourceRect.top;
    pRectInfo->m_SRC_BRX = SourceRect.right;
    pRectInfo->m_SRC_BRY = SourceRect.bottom;
    pRectInfo->m_SRC_SIZX = SourceRect.Size().cx;
    pRectInfo->m_SRC_SIZY = SourceRect.Size().cy;
    pRectInfo->m_CLR_TLX = ClipRect.left;
    pRectInfo->m_CLR_TLY = ClipRect.top;
    pRectInfo->m_CLR_BRX = ClipRect.right;
    pRectInfo->m_CLR_BRY = ClipRect.bottom;
    pRectInfo->m_CLR_SIZX = ClipRect.Size().cx;
    pRectInfo->m_CLR_SIZY = ClipRect.Size().cy;
    pRectInfo->UpdateData(PUT_DATA);
    }
}

// Called when some other app has changed the palette so 
// this app gets a chance to change it back
// I am passing base class pointers to derived class objects,
// one must be careful what actions the called routine attempts.
///////////////////////////////////////////////////////////////////
LRESULT CBasicView::OnDoRealize(WPARAM wParam, LPARAM)
{
  CMainFrame* pAppFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
  ASSERT(pAppFrame->IsKindOf(RUNTIME_CLASS( CMainFrame )));
  CDocUif* pDoc = GetDocument();
  // Call the CDib object
  if (m_pCDib)
    return (m_pCDib->DoPaletteChanged(wParam, pAppFrame, m_hWnd, pDoc));
  else
    return(0L);
}

// set display window to be exact size of image contained
// well, actually will shrink window if its too big.
void CBasicView::OnAutosizeWindow()
{
  WINDOWPLACEMENT wndpl;
  CDocUif *pDoc = (CDocUif *)GetDocument();
  CSize Size = pDoc->GetDocSize();

  if (Size == CSize(0,0))
    return;

  wndpl.length = sizeof(WINDOWPLACEMENT);
  if(GetWindowPlacement(&wndpl))
    {
    wndpl.rcNormalPosition.right = wndpl.rcNormalPosition.left + Size.cx;
    wndpl.rcNormalPosition.bottom = wndpl.rcNormalPosition.top + Size.cy;
    SetWindowPlacement(&wndpl);
    GetParentFrame()->RecalcLayout();
    ResizeParentToFit();
    }
}

void CBasicView::OnOpt1to1()
{
  CDocUif *pDoc = (CDocUif *)GetDocument();
  CSize Size = pDoc->GetDocSize();

  m_bOneToOne = (m_bOneToOne == FALSE);
  if (!m_bOneToOne) // Go to zoom mode
    SetScaleToFitSize(Size);
  else // go to 1:1 mode
    SetScrollSizes(MM_TEXT, Size);

  GetParentFrame()->RecalcLayout();
  ResizeParentToFit();
}

void CBasicView::OnUpdateOpt1to1(CCmdUI* pCmdUI)
{
  pCmdUI->SetCheck(m_bOneToOne);
}

// limit window size to doc size for 1:1 case
void CBasicView::OnSize(UINT nType, int cx, int cy)
{
  BOOL bChanged = FALSE;
  CDocUif *pDoc = (CDocUif *)GetDocument();
  if (pDoc)
    {
    CSize Size = pDoc->GetDocSize();

    if (Size.cx == 0 || Size.cy == 0)
      return;
    if (cx > Size.cx)
      cx = Size.cx, bChanged = TRUE;
    if (cy > Size.cy)
      cy = Size.cy, bChanged = TRUE;
    if (bChanged)
      {
      if (m_bOneToOne)
        SetScrollSizes(MM_TEXT, CSize(cx, cy));
      else
        SetScaleToFitSize(Size);
      GetParentFrame()->RecalcLayout();
      ResizeParentToFit();
      }
    }
	CScrollView::OnSize(nType, cx, cy);
}
