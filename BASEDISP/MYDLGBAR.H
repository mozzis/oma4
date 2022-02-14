// mydlgbar.h : header file
//
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 20:56:58  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/MYDLGBAR.H_V  $
// 
//    Rev 1.1   29 Sep 1995 20:56:58   MLM
// Add PVCS info tags
//  
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMyDlgBar window

class CMyDlgBar : public CDialogBar
{
  friend class CMainFrame;
  DECLARE_DYNCREATE(CMyDlgBar)
// Construction
public:
  CMyDlgBar();

// Form Data
public:
  enum { IDD = IDD_DLGBAR1 };
  CVBControl* m_pBtnCont;
  CVBControl* m_pBtnBrite;
  CVBControl* m_pContSlide;
  CVBControl* m_pBriteSlide;
  CVBControl* m_pBtnZoom;
  CVBControl* m_pBtnZfrm;
  CVBControl* m_pBtnHflip;
  CVBControl* m_pBtnVflip;
  int   Briteness;
  int   Contrast;

// Attributes
public:

// Operations
public:
  void SetTextDMax(const CString& Text);
  void SetTextDMin(const CString& Text);

// Implementation
public:
  virtual ~CMyDlgBar();
  short GetBrite(void);
  short GetCont(void);
  void InitControls(void);

protected:
CVBControl * GetCVBPtr(UINT nID);
// In spite of the fact that these are declared this way, and
// Class Wizard seems to know abut them, the messages never
// arrive here; instead, I have to catch them in the frame
// window.  Oh well - I have to declare handler functions
// here anyway, so I left them in MFC message map style.
protected:
  // Generated message map functions
  //{{AFX_MSG(CMyDlgBar)
  afx_msg void OnMouseupBrite(UINT, int, CWnd*, LPVOID);
  afx_msg void OnMouseupCont(UINT, int, CWnd*, LPVOID);
  afx_msg void OnClickBtnbrite(UINT, int, CWnd*, LPVOID);
  afx_msg void OnClickBtncont(UINT, int, CWnd*, LPVOID);
  afx_msg void OnUpdateControl(CCmdUI* pCmdUI);
  afx_msg void OnClose();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

extern CMyDlgBar * theDlgBar;
