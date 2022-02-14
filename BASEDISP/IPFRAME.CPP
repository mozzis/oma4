// ipframe.cpp : implementation of the CInPlaceFrame class
//
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 21:00:32  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/IPFRAME.CPV  $
// 
//    Rev 1.1   29 Sep 1995 21:00:32   MLM
// Add PVCS info tags
// 
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "basedisp.h"
#include "ipframe.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInPlaceFrame

IMPLEMENT_DYNCREATE(CInPlaceFrame, COleIPFrameWnd)

BEGIN_MESSAGE_MAP(CInPlaceFrame, COleIPFrameWnd)
  //{{AFX_MSG_MAP(CInPlaceFrame)
  ON_WM_CREATE()
  //}}AFX_MSG_MAP
  // Global help commands
  ON_COMMAND(ID_HELP_INDEX, COleIPFrameWnd::OnHelpIndex)
  ON_COMMAND(ID_HELP_USING, COleIPFrameWnd::OnHelpUsing)
  ON_COMMAND(ID_HELP, COleIPFrameWnd::OnHelp)
  ON_COMMAND(ID_DEFAULT_HELP, COleIPFrameWnd::OnHelpIndex)
  ON_COMMAND(ID_CONTEXT_HELP, COleIPFrameWnd::OnContextHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// arrays of IDs used to initialize control bars

// toolbar buttons - IDs are command buttons
static UINT BASED_CODE buttons[] =
{
  // same order as in the bitmap 'itoolbar.bmp'
  ID_FILE_NEW,
  ID_FILE_OPEN,
  ID_FILE_SAVE,
    ID_SEPARATOR,
  ID_EDIT_CUT,
  ID_EDIT_COPY,
  ID_EDIT_PASTE,
    ID_SEPARATOR,
  ID_FILE_PRINT,
  ID_APP_ABOUT,
  ID_CONTEXT_HELP,
    ID_SEPARATOR,
  IDS_AUTOSCALE,
  IDS_AUTOAUTO,
};

/////////////////////////////////////////////////////////////////////////////
// CInPlaceFrame construction/destruction

CInPlaceFrame::CInPlaceFrame()
{
}

CInPlaceFrame::~CInPlaceFrame()
{
}

int CInPlaceFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (COleIPFrameWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  if (!m_wndResizeBar.Create(this))
  {
    TRACE("Failed to create resize bar\n");
    return -1;      // fail to create
  }

  m_dropTarget.Register(this);

  return 0;
}

// OnCreateControlBars is called by the framework to create control
//  bars on the container application's windows.
BOOL CInPlaceFrame::OnCreateControlBars(CWnd* pWndFrame, CWnd* pWndDoc)
{
  // create toolbar on client's frame window
  if (!m_wndToolBar.Create(this) ||
    !m_wndToolBar.LoadBitmap(IDR_MAINFRAME) ||
    !m_wndToolBar.SetButtons(buttons,
      sizeof(buttons)/sizeof(UINT)))
    {
    TRACE("Failed to create toolbar\n");
    return -1;      // fail to create
    }

  // set owner to this window, so messages are delivered to correct app
  m_wndToolBar.SetOwner(this);
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CInPlaceFrame diagnostics

#ifdef _DEBUG
void CInPlaceFrame::AssertValid() const
{
  COleIPFrameWnd::AssertValid();
}

void CInPlaceFrame::Dump(CDumpContext& dc) const
{
  COleIPFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CInPlaceFrame commands
