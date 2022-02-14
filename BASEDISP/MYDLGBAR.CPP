// mydlgbar.cpp : implementation file
//
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 21:00:32  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/MYDLGBAR.CPV  $
// 
//    Rev 1.1   29 Sep 1995 21:00:32   MLM
// Add PVCS info tags
// 
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "basedisp.h"
#include "mydlgbar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

const long nMax = 200;
CMyDlgBar * theDlgBar;

/////////////////////////////////////////////////////////////////////////////
// CMyDlgBar

// Note: The CDialogBar base class seems to be a troubled creature, 
// or else my implementation of it is faulty. For some reason the 
// control and VBX messages never reach the handlers in the dialog 
// bar module; instead, I have to catch its messages in the frame 
// window and pass them along to this module. Long talks to Microsoft
//  support did not produce an answer. I still use the message map 
// stuff to declare the handlers which remain in this module. However,
// the control messages don't ever show up here. Instead, the message
// goes first to the frame window, and if a handler is not provided, 
// the message gets routed (by the MFC library) right on through to 
// the active view. The rule I followed is that if a control stores 
// data here locally then frame window will call the handler here; 
// later, the view will query the dialog bar (s.b. query the frame 
// window but I cheated) for the data value. If, instead, the control 
// just represents an action which the view must handle anyway, I let 
// the active view pick it up directly. The bottom line is not to put
// event handlers here, but rather in either the frame window or the
// view module. Someday I'll junk the dialog bar and implement the toolbox
// as a modeless or something.

IMPLEMENT_DYNCREATE(CMyDlgBar, CDialogBar)

CMyDlgBar::CMyDlgBar()
  : CDialogBar()
{
  theDlgBar = this;
  m_bAutoDelete = TRUE;
}

CMyDlgBar::~CMyDlgBar()
{
}

BEGIN_MESSAGE_MAP(CMyDlgBar, CDialogBar)
  //{{AFX_MSG_MAP(CMyDlgBar)
  ON_VBXEVENT(VBN_CHANGE, IDC_BRITE, OnMouseupBrite)
  ON_VBXEVENT(VBN_MOUSEUP, IDC_BRITE, OnMouseupBrite)
  ON_VBXEVENT(VBN_MOUSEUP, IDC_CONT, OnMouseupCont)
  ON_VBXEVENT(VBN_CLICK, IDC_BTNBRITE, OnClickBtnbrite)
  ON_VBXEVENT(VBN_CLICK, IDC_BTNCONT, OnClickBtncont)
  ON_UPDATE_COMMAND_UI(IDC_BTNBRITE, OnUpdateControl)
  ON_UPDATE_COMMAND_UI(IDC_BTNCONT, OnUpdateControl)
  ON_WM_CLOSE()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

short CMyDlgBar::GetBrite(void)
{
  return (short)Briteness;
}

short CMyDlgBar::GetCont(void)
{
  return (short)Contrast;
}
           
CVBControl * CMyDlgBar::GetCVBPtr(UINT nID)
{
  return (CVBControl *)((CWnd *)GetDlgItem(nID));
} 

// This function always gets the pointers to the VBX's because
// there isn't a convenient time to do it earlier, though it needs
// to be done only once          
void CMyDlgBar::InitControls(void)
{
  Briteness = Contrast = nMax / 2;

  // Get pointers to VBX's
  m_pBtnBrite = GetCVBPtr(IDC_BTNBRITE);
  m_pBriteSlide = GetCVBPtr(IDC_BRITE);

  m_pBtnCont = GetCVBPtr(IDC_BTNCONT);
  m_pContSlide = GetCVBPtr(IDC_CONT);
                          
  m_pBtnZoom = GetCVBPtr(IDC_BTNZOOM);
  m_pBtnZfrm = GetCVBPtr(IDC_BTNZFRM);

  m_pBtnHflip = GetCVBPtr(IDC_BTNHFLIP);
  m_pBtnVflip = GetCVBPtr(IDC_BTNVFLIP);

  // Set sliders to middle position
  m_pBriteSlide->SetNumProperty("Value", Briteness, 0);
  m_pContSlide->SetNumProperty("Value", Contrast, 0);
} 

/////////////////////////////////////////////////////////////////////////////
// CMyDlgBar message handlers

void CMyDlgBar::OnMouseupBrite(UINT, int, CWnd*, LPVOID)
{
  Briteness = (int)m_pBriteSlide->GetNumProperty("Value", 0);
}

void CMyDlgBar::OnMouseupCont(UINT, int, CWnd*, LPVOID)
{
  Contrast = (int)m_pContSlide->GetNumProperty("Value", 0);
}

void CMyDlgBar::OnClickBtnbrite(UINT, int, CWnd*, LPVOID)
{
  Briteness = nMax / 2;  // Set briteness to neutral (=100)
  m_pBriteSlide->SetNumProperty("Value", Briteness, 0);
}

void CMyDlgBar::OnClickBtncont(UINT, int, CWnd*, LPVOID)
{
  Contrast = nMax / 2;  // Set briteness to neutral (=100)
  m_pContSlide->SetNumProperty("Value", Contrast, 0);
}

void CMyDlgBar::SetTextDMax(const CString& Text)
{
  SetDlgItemText(IDC_DMAX, Text);
}

void CMyDlgBar::SetTextDMin(const CString& Text)
{
  SetDlgItemText(IDC_DMIN, Text);
}

// Generic function to let controls be active - sometimes MFC must see override
/////////////////////////////////////////////////////////////////////////////
void CMyDlgBar::OnUpdateControl(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(TRUE);
}

void CMyDlgBar::OnClose()
{
  DestroyWindow();
//  CDialogBar::OnClose();
}
