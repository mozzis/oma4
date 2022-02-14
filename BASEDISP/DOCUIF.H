// docuif.h : interface of the CDocUif class
//
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 20:56:58  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/DOCUIF.H_V  $
// 
//    Rev 1.1   29 Sep 1995 20:56:58   MLM
// Add PVCS info tags
//  
//////////////////////////////////////////////////////////////////////////
                     
#include "..\datmgr\datmgr.h"
                     
class CDispItem;

class CDocUif : public COleServerDoc
{
protected: // create from serialization only
  CDocUif(void);
  DECLARE_DYNCREATE(CDocUif)

// Attributes
public:
  CSize GetDocSize(void);
  CDispItem *GetEmbeddedItem(void)
    { return (CDispItem *)COleServerDoc::GetEmbeddedItem(); }
  BOOL DocIsLive(void); // Get lastlive flag for document
  BOOL Updated(void);
  void Updated(BOOL);
  ULONG __huge * GetDataPtr(void); 
protected:
  DHANDLE dHandle;    // DATMGR handle to data source
  BOOL DocIsLastlive; // Temp till DATMGR knows about lastlive
  BOOL m_Modified;
  BOOL m_Updated;
  short LastL;
  ULONG __huge * DataPtr;

// Operations
public:

// Implementation
public:
  virtual ~CDocUif(void);
//  virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
  virtual void AssertValid(void) const;
  virtual void Dump(CDumpContext& dc) const;
#endif

protected:
  void InitDocument(void);
  virtual BOOL OnNewDocument(void);
  virtual BOOL OnOpenDocument(const char* pszPathName);
  virtual BOOL OnSaveDocument(const char* pszPathName);
  COleServerItem * OnGetEmbeddedItem(void);

// Generated message map functions
protected:
  //{{AFX_MSG(CDocUif)
      // NOTE - the ClassWizard will add and remove member functions here.
      //    DO NOT EDIT what you see in these blocks of generated code !
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
