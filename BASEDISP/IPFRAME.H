// ipframe.h : interface of the CInPlaceFrame class
//
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 20:56:58  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/IPFRAME.H_V  $
// 
//    Rev 1.1   29 Sep 1995 20:56:58   MLM
// Add PVCS info tags
//  
//////////////////////////////////////////////////////////////////////////

class CInPlaceFrame : public COleIPFrameWnd
{
  DECLARE_DYNCREATE(CInPlaceFrame)
public:
  CInPlaceFrame();

// Attributes
public:

// Operations
public:

// Implementation
public:
  virtual ~CInPlaceFrame();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif
  virtual BOOL OnCreateControlBars(CWnd* pWndFrame, CWnd* pWndDoc);

protected:  // control bar embedded members
  CToolBar    m_wndToolBar;
  COleResizeBar   m_wndResizeBar;
  COleDropTarget m_dropTarget;

// Generated message map functions
protected:
  //{{AFX_MSG(CInPlaceFrame)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
