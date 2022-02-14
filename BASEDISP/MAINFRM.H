// mainfrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 20:56:58  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/MAINFRM.H_V  $
// 
//    Rev 1.1   29 Sep 1995 20:56:58   MLM
// Add PVCS info tags
//  
//////////////////////////////////////////////////////////////////////////

#define WM_DOREALIZE   (WM_USER + 0)

class CMyDlgBar;
class CRectInfo;

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
  CMainFrame();
  DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Implementation
private:
  CString OldStatusText;  // Save area for status message
  BOOL StTextSaved;

protected:  // control bar embedded members
  CStatusBar m_wndStatusBar;
  CToolBar   m_wndToolBar;
  CMyDlgBar  *m_pMyDlgBar;
  CRectInfo  *m_pRectInfo;
  BOOL       m_bDelayUpdate;

public:
  virtual ~CMainFrame();
  void UpdateView(void);
  void StatusTextSave(void);    // Saves 1st message in status bar
  void StatusTextRestore(void);  // Restores saved text
  void StatusTextSet(const char * NewText, UINT Index); // Sets indicator in status bar
  short getBriteness(void); // get briteness from dialog bar controls
  short getContrast(void); // get contrast from dialog bar controls
  void InitBriteAndCont(void);
  CRectInfo *GetRectInfoPtr(void);
  BOOL GetDelayUpdate(void) { return m_bDelayUpdate; }
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

 // Generated message map functions
 // Some of these actually come from the dialog bar (toobox)
 // but they show up here for some reason
protected:
  //{{AFX_MSG(CMainFrame)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnUpdateControl(CCmdUI* pCmdUI);
  afx_msg void OnMouseupBrite(UINT, int, CWnd*, LPVOID);
  afx_msg void OnChangeBrite(UINT, int, CWnd*, LPVOID);
  afx_msg void OnMouseupCont(UINT, int, CWnd*, LPVOID);
  afx_msg void OnChangeCont(UINT, int, CWnd*, LPVOID);
  afx_msg void OnClickBtnbrite(UINT, int, CWnd*, LPVOID);
  afx_msg void OnClickBtncont(UINT, int, CWnd*, LPVOID);
  afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnViewRectInfo();
  afx_msg LONG OnCloseRectInfo(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateViewRectInfo(CCmdUI* pCmdUI);
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnUpdateViewControls(CCmdUI* pCmdUI);
	afx_msg void OnViewControls();
	afx_msg void OnUpdateSetDelayUpdate(CCmdUI* pCmdUI);
	afx_msg void OnSetDelayUpdate();
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

extern CMainFrame * theMainFrame;
