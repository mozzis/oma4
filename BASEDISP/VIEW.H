// View.h : interface of the CBasicView class
//
///////////p//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 20:57:02  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/VIEW.H_V  $
// 
//    Rev 1.1   29 Sep 1995 20:57:02   MLM
// Add PVCS info tags
//  
//////////////////////////////////////////////////////////////////////////

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "mytrack.h"

class CDocUif;
class CRectInfo;
class CDib;
enum XDrawDir;
enum YDrawDir;

class CBasicView : public CScrollView
{
protected: // create from serialization only
  CBasicView();
  DECLARE_DYNCREATE(CBasicView)

// Attributes
public:
  CDocUif* GetDocument();

// Operations
public:

// Implementation
public:
  virtual ~CBasicView();
  virtual void OnDraw(CDC* pDC);  // overridden to draw this view
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif
  static void __far __cdecl UpdateMouse(UINT, CPoint);
  virtual void ZoomRectOn(void);
  virtual void ZoomRectOff(void);
  virtual void DrawTracker(void);
  virtual BOOL IsAdjusted(void);
  void MapPointToDoc(CPoint& point, BOOL bFlip = TRUE);
  void MapRectToDoc(CRect& rect, BOOL bFlip = TRUE);
  void MapZRectToDoc(CRect& OutRect);
  void UnZoomSourceRect(void);
  enum ZRectState { ZRectOff = 0, ZRectOn };
private:
  MyTrack m_tracker;
  CDib  *m_pCDib;
  float DataMin, DataMax, 
        Briteness, Contrast;
  BOOL AutoAutoOn;
  BOOL m_bOneToOne;
  ZRectState ZoomRectState;
  CRect SourceRect;
  CRect OutputRect;
  XDrawDir XDrawDirection;    // flag for horiz draw direction
  YDrawDir YDrawDirection;    // flag for vert draw direction
  
  void SetDataValid(BOOL SetVal); // Set truth that bitmap data same as detector data
  BOOL GetDataValid(void);        // Get truth that bitmap data same as detector data
  void BriteAndContCalc(long *Brite, float *Cont);
  void ZoomToRect(void);
  void Crosses(const int CellSize);
  void RefreshRectInfo(void);
protected:
  // Printing support
  virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
  virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
  virtual void OnPrint(CDC *pDC, CPrintInfo *pInfo);
  virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Generated message map functions
protected:
  virtual void OnInitialUpdate(void);
  virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
  virtual void OnPrepareDC(CDC *pDC, CPrintInfo *pInfo = NULL);
  //{{AFX_MSG(CBasicView)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
  afx_msg void OnDestroy();
  afx_msg void OnClickBtnzfrm(UINT, int, CWnd*, LPVOID);
  afx_msg void OnClickBtnzoom(UINT, int, CWnd*, LPVOID);
  afx_msg void OnClickBtnvflip(UINT, int, CWnd*, LPVOID);
  afx_msg void OnClickBtnhflip(UINT, int, CWnd*, LPVOID);
  afx_msg void OnEditCopy();
  afx_msg void OnRenderFormat(UINT nFormat);
  afx_msg void OnRenderAllFormats(void);
  afx_msg LRESULT OnDoRealize(WPARAM wParam, LPARAM);
  afx_msg void OnEditPaste();
  afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnAutoScale(void);
  afx_msg void OnAutoAuto(void);
  afx_msg void OnUpdateControl(CCmdUI* pCmdUI);
  afx_msg void OnUpdateAutoAuto(CCmdUI* pCmdUI);
  afx_msg void OnProc1();
  afx_msg void OnProc2();
	afx_msg void OnAutosizeWindow();
	afx_msg void OnOpt1to1();
	afx_msg void OnUpdateOpt1to1(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in View.cpp
inline CDocUif* CBasicView::GetDocument()
   { return (CDocUif*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
