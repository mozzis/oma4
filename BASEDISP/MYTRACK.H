// mytrack.h : header file
//
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 20:56:58  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/MYTRACK.H_V  $
// 
//    Rev 1.1   29 Sep 1995 20:56:58   MLM
// Add PVCS info tags
//  
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// MyTrack - version of CRectTracker MFC class

typedef void (__cdecl __far FNUPDATE)(UINT, CPoint);

class MyTrack :  public CRectTracker
{
// Construction
public:
  MyTrack();

// Attributes
private:
  CRect m_LimitRect;
public:
// Operations
private:
  void Limit(LPRECT lprect);
public:
  BOOL Track(CWnd* pWnd, CPoint point, BOOL bAllowInvert = FALSE, CWnd* pWndClipTo = NULL);
  BOOL TrackRubberBand(CWnd*, CPoint, FNUPDATE *, BOOL bAllowInvert = TRUE);
  virtual void DrawTrackerRect(LPCRECT lpRect, CWnd* pWndClipTo, CDC* pDC, CWnd* pWnd);
  virtual void OnChangedRect(const CRect& rectOld);
  virtual void AdjustRect(int handle, LPRECT lpRect);
  void SetLimit(const CRect& LimitRect);

// Implementation
public:
  virtual ~MyTrack();
};

/////////////////////////////////////////////////////////////////////////////
