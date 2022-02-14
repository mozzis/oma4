// rectinfo.cpp : implementation file
//
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 21:00:34  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/RECTINFO.CPV  $
// 
//    Rev 1.1   29 Sep 1995 21:00:34   MLM
// Add PVCS info tags
// 
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "basedisp.h"
#include "rectinfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRectInfo dialog


CRectInfo::CRectInfo(CWnd* pParent /*=NULL*/)
  : CDialog(CRectInfo::IDD, pParent)
{
  //{{AFX_DATA_INIT(CRectInfo)
  m_CLR_BRX = 0;
  m_CLR_BRY = 0;
  m_CLR_SIZX = 0;
  m_CLR_SIZY = 0;
  m_CLR_TLX = 0;
  m_CLR_TLY = 0;
  m_SRC_BRX = 0;
  m_SRC_BRY = 0;
  m_SRC_SIZX = 0;
  m_SRC_SIZY = 0;
  m_SRC_TLX = 0;
  m_SRC_TLY = 0;
  //}}AFX_DATA_INIT
  Create(IDD_RECTINFO, pParent);
}

void CRectInfo::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CRectInfo)
  DDX_Text(pDX, IDC_CLR_BRX, m_CLR_BRX);
  DDX_Text(pDX, IDC_CLR_BRY, m_CLR_BRY);
  DDX_Text(pDX, IDC_CLR_SIZX, m_CLR_SIZX);
  DDX_Text(pDX, IDC_CLR_SIZY, m_CLR_SIZY);
  DDX_Text(pDX, IDC_CLR_TLX, m_CLR_TLX);
  DDX_Text(pDX, IDC_CLR_TLY, m_CLR_TLY);
  DDX_Text(pDX, IDC_SRC_BRX, m_SRC_BRX);
  DDX_Text(pDX, IDC_SRC_BRY, m_SRC_BRY);
  DDX_Text(pDX, IDC_SRC_SIZX, m_SRC_SIZX);
  DDX_Text(pDX, IDC_SRC_SIZY, m_SRC_SIZY);
  DDX_Text(pDX, IDC_SRC_TLX, m_SRC_TLX);
  DDX_Text(pDX, IDC_SRC_TLY, m_SRC_TLY);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRectInfo, CDialog)
  //{{AFX_MSG_MAP(CRectInfo)
    ON_WM_CLOSE()
    ON_COMMAND(ID_APP_EXIT, OnAppExit)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRectInfo message handlers
void CRectInfo::PostNcDestroy()
{
  delete(this);
}

void CRectInfo::OnClose()
{
  GetParent()->PostMessage(WM_RECTINFOCLS);
  DestroyWindow();
}

void CRectInfo::OnOK()
{
  GetParent()->PostMessage(WM_RECTINFOCLS);
  DestroyWindow();
}

void CRectInfo::OnCancel()
{
  GetParent()->PostMessage(WM_RECTINFOCLS);
  DestroyWindow();
}

void CRectInfo::OnAppExit()
{
  GetParent()->PostMessage(WM_RECTINFOCLS);
  DestroyWindow();
}
