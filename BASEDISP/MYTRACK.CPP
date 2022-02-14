/////////////////////////////////////////////////////////////////////////////
// MyTrack - private version of tracker rect.
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 21:00:34  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/MYTRACK.CPV  $
// 
//    Rev 1.1   29 Sep 1995 21:00:34   MLM
// Add PVCS info tags
// 
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "mytrack.h"

MyTrack::MyTrack()
{
}

MyTrack::~MyTrack()
{
}

/////////////////////////////////////////////////////////////////////////////
// MyTrack message handlers

void MyTrack::Limit(LPRECT lprect)
{
  if (lprect->right > m_LimitRect.right)
    lprect->right = m_LimitRect.right;
  if (lprect->left < m_LimitRect.left)
    lprect->left = m_LimitRect.left;
  if (lprect->top < m_LimitRect.top)
    lprect->top = m_LimitRect.top;
  if (lprect->bottom > m_LimitRect.bottom)
    lprect->bottom = m_LimitRect.bottom;
}

void MyTrack::SetLimit(const CRect& LimitRect)
{
  m_LimitRect = LimitRect;
}

BOOL MyTrack::Track(CWnd* pWnd, CPoint point, BOOL bAllowInvert, CWnd* pWndClipTo)
{
  return(CRectTracker::Track(pWnd, point, bAllowInvert, pWndClipTo));
}

BOOL MyTrack::TrackRubberBand(CWnd* pWnd, CPoint point, FNUPDATE, BOOL bAllowInvert)
{
  return CRectTracker::TrackRubberBand(pWnd, point, bAllowInvert);
}

// Called when tracker is adjusted in place with handles
/////////////////////////////////////////////////////////////////////////////
void MyTrack::AdjustRect(int handle, LPRECT lpRect)
{
  Limit(lpRect);
  CRectTracker::AdjustRect(handle, lpRect);
}

// Called when time to draw tracker, e.g. during rubber-banding
/////////////////////////////////////////////////////////////////////////////
void MyTrack::DrawTrackerRect(LPCRECT lpRect, CWnd* pWndClipTo, 
                              CDC* pDC, CWnd* pWnd)
{
  CRect LimRect = lpRect;
  Limit(LimRect);
//  ((CView *)GetParent())->OnMouseMove(0, LimRect.BottomRight()); 
  CRectTracker::DrawTrackerRect(LimRect, pWndClipTo, pDC, pWnd);
}

// Called after old rect is erased during tracker changing
/////////////////////////////////////////////////////////////////////////////
void MyTrack::OnChangedRect(const CRect& rectOld)
{
  Limit(rectOld);
  CRectTracker::OnChangedRect(rectOld);
}

