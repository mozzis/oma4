// scribitm.h : Prototypes for the CDispItem class.
//
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 20:56:58  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/OLEITEM.H_V  $
// 
//    Rev 1.1   29 Sep 1995 20:56:58   MLM
// Add PVCS info tags
//  
//////////////////////////////////////////////////////////////////////////

class CDocUif;

/////////////////////////////////////////////////////////////////////////////
// CScribbleEmbeddedItem

class CDispItem : public COleServerItem
{
  DECLARE_DYNAMIC(CDispItem)

// Constructors
public:
  CDispItem(CDocUif* pContainerDoc);

// Attributes
  CDocUif* GetDocument() const
    { return (CDocUif*)COleServerItem::GetDocument(); }

// Implementation
public:
  ~CDispItem();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif
  virtual BOOL OnDraw(CDC* pDC, CSize& rSize);
  virtual BOOL OnGetExtent(DVASPECT dwDrawAspect, CSize& rSize);

protected:
  virtual void Serialize(CArchive& ar);   // overridden for document i/o
};
