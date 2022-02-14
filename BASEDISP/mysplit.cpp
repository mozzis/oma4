#include <stdafx.h>
#include "mysplitwnd.h"

void CMySplitWnd::DrawAllSplitBars(CDC* pDC, int cxInside, int cyInside)
{
	ASSERT_VALID(this);

	CRect rect;
	GetClientRect(rect);
	for (int col = 0; col < m_nCols - 1; col++)
	{
		rect.left += m_pColInfo[col].nCurSize + CX_BORDER;
		rect.right = rect.left + m_cxSplitter;
		if (rect.right > cxInside)
			break;      // stop if not fully visible
		OnDrawSplitter(pDC, splitBar, rect);
		rect.left = rect.right + CX_BORDER;
	}

	GetClientRect(rect);
	for (int row = 0; row < m_nRows - 1; row++)
	{
		rect.top += m_pRowInfo[row].nCurSize + CY_BORDER;
		rect.bottom = rect.top + m_cySplitter;
		if (rect.bottom > cyInside)
			break;      // stop if not fully visible
		OnDrawSplitter(pDC, splitBox, rect);
		rect.top = rect.bottom + CY_BORDER;
	}
}
