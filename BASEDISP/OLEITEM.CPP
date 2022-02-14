// oleitem.cpp : Implementation for the CDispItem class.
//
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 21:00:34  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/OLEITEM.CPV  $
// 
//    Rev 1.1   29 Sep 1995 21:00:34   MLM
// Add PVCS info tags
// 
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "basedisp.h"
#include "docuif.h"
#include "oleitem.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// This is very close to being what is needed to provide "in-place" display
// of images from within other programs!!. To finish, need to:
//
//    1) Set the DIB and DDB clipboard formats in the constructor
//    2) Get a private "OMADATA" format and register it too
//    3) provide a serialize function to ship a header and data out
//       to a calling app for the OMADATA format; else just provide DIB
//    4) Provide a Draw function.
//
// I think that would be it! At least for a start...

/////////////////////////////////////////////////////////////////////////////
// CDispItem

IMPLEMENT_DYNAMIC(CDispItem, COleServerItem)

CDispItem::CDispItem(CDocUif *pContainerDoc)
  : COleServerItem(pContainerDoc, TRUE)
{
  // TODO: add one-time construction code here
  //  (eg, adding additional clipboard formats to the item's data source)
}

CDispItem::~CDispItem()
{
  // TODO: add cleanup code here
}

void CDispItem::Serialize(CArchive& ar)
{
  // CDispItem::Serialize will be called by the framework if
  // the item is copied to the clipboard.  This can happen automatically
  // through the OLE callback OnGetClipboardData.  A good default for
  // the embedded item is simply to delegate to the document's Serialize
  // function.  If you support links, you'd want to serialize just a
  // portion of the document.

  if (!IsLinkedItem())
  {
    CDocUif* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
//    pDoc->Serialize(ar);
  }
}

BOOL CDispItem::OnGetExtent(DVASPECT dwDrawAspect, CSize& rSize)
{
  // This implementation of CDispItem::OnGetExtent only handles
  //  the "content" aspect indicated by DVASPECT_CONTENT.

  if (dwDrawAspect != DVASPECT_CONTENT)
    return COleServerItem::OnGetExtent(dwDrawAspect, rSize);

  // CDispItem::OnGetExtent is called to get the extent in
  //  HIMETRIC units of the entire item.  The default implementation
  //  here simply returns a hard-coded number of units.

  CDocUif* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  rSize = pDoc->GetDocSize();
  CClientDC dc(NULL);

  // set a MM_LOENGLISH based on logical inches
  //  (we can't use MM_LOENGLISH because MM_LOENGLISH uses physical inches)
  dc.SetMapMode(MM_ANISOTROPIC);
  dc.SetViewportExt(dc.GetDeviceCaps(LOGPIXELSX), dc.GetDeviceCaps(LOGPIXELSY));
  dc.SetWindowExt(100, -100);
  dc.LPtoHIMETRIC(&rSize);

  return TRUE;
}

BOOL CDispItem::OnDraw(CDC* pDC, CSize& rSize)
{
  CDocUif* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  pDC->SetMapMode(MM_ANISOTROPIC);
  CSize sizeDoc = pDoc->GetDocSize();
  sizeDoc.cy = -sizeDoc.cy;
  pDC->SetWindowExt(sizeDoc);
  pDC->SetWindowOrg(0,0);

  return TRUE;
}

// CDispItem diagnostics

#ifdef _DEBUG
void CDispItem::AssertValid() const
{
  COleServerItem::AssertValid();
}

void CDispItem::Dump(CDumpContext& dc) const
{
  COleServerItem::Dump(dc);
}
#endif
