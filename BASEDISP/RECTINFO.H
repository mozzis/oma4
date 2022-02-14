// rectinfo.h : header file
//
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 20:56:58  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/RECTINFO.H_V  $
// 
//    Rev 1.1   29 Sep 1995 20:56:58   MLM
// Add PVCS info tags
//  
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CRectInfo dialog

#define WM_RECTINFOCLS WM_USER + 10

#define PUT_DATA FALSE
#define GET_DATA TRUE

class CRectInfo : public CDialog
{
// Construction
public:
	CRectInfo(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CRectInfo)
	enum { IDD = IDD_RECTINFO };
	int		m_CLR_BRX;
	int		m_CLR_BRY;
	int		m_CLR_SIZX;
	int		m_CLR_SIZY;
	int		m_CLR_TLX;
	int		m_CLR_TLY;
	int		m_SRC_BRX;
	int		m_SRC_BRY;
	int		m_SRC_SIZX;
	int		m_SRC_SIZY;
	int		m_SRC_TLX;
	int		m_SRC_TLY;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CRectInfo)
  virtual void PostNcDestroy();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAppExit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
