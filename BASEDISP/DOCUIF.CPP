// docuif.cpp : implementation of the CDocUif class
//
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 21:00:32  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/DOCUIF.CPV  $
// 
//    Rev 1.1   29 Sep 1995 21:00:32   MLM
// Add PVCS info tags
// 
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "basedisp.h"
#include "docuif.h"
#include "oleitem.h"
#include "oma4driv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

ULONG __huge * DataPtr;

/////////////////////////////////////////////////////////////////////////////
// CDocUif

IMPLEMENT_DYNCREATE(CDocUif, COleServerDoc)

BEGIN_MESSAGE_MAP(CDocUif, COleServerDoc)
  //{{AFX_MSG_MAP(CDocUif)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDocUif construction/destruction

CDocUif::CDocUif()
{
  EnableAutomation();
  AfxOleLockApp();
  m_Modified = FALSE;
}

CDocUif::~CDocUif()
{
  AfxOleUnlockApp();
}

BOOL CDocUif::OnNewDocument()
{
  if (!COleServerDoc::OnNewDocument())
    return FALSE;

  DocIsLastlive = TRUE;
  DataPtr = (ULONG __huge *)GetDataPointer(); 

  return TRUE;
}

BOOL CDocUif::OnOpenDocument( const char* pszPathName )
{
  ULONG __huge *pTemp;
  BOOL Success = FALSE;
  
  if (_fstrcmp("lastlive", pszPathName))
    {
    dHandle = OpenDataSource(pszPathName);
    if (dHandle >= 0)  // got valid data handle
      {                // don't lose old data pointer just yet
      pTemp = (ULONG __huge *)GetFileDataPointer(dHandle);
      if (pTemp)
        {
        DataPtr = pTemp;
        DocIsLastlive = FALSE;
        Success = TRUE;
        }
      }
    else
      {
      CString rString;
      AfxFormatString1(rString, IDS_ERRFILEOPEN, pszPathName);
      AfxMessageBox(rString, MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
      Success = FALSE;
      }
    }
  else
    {
    DataPtr = (ULONG __huge *)GetDataPointer();
    DocIsLastlive = TRUE;
    Success = (DataPtr != NULL);
    }
  if (Success)
    UpdateAllViews(NULL, -1L);
  return Success;
}

static inline DWORD DMult(short A, short B)
{
  return (DWORD)((DWORD)A * (DWORD)B);
}

BOOL CDocUif::OnSaveDocument(const char* pszPathName)
{
  BOOL Success = FALSE;
   
  if (DocIsLastlive) // not yet a disk file, must create it
    {
    dHandle = NewDataSource(pszPathName);
    if (dHandle >= 0)
      {                                    
      short IParam;
      float FParam;

      GetIntParam(DC_POINTS, &IParam); 
      SetIntHeaderParam(dHandle, DM_POINTS, IParam);
      GetIntParam(DC_TRACKS, &IParam); 
      SetIntHeaderParam(dHandle, DM_TRACKS, IParam);
      GetIntParam(DC_I, &IParam); 
      SetIntHeaderParam(dHandle, DM_I, IParam);
      GetIntParam(DC_DTEMP, &IParam); 
      SetIntHeaderParam(dHandle, DM_DTEMP, IParam);
      GetParam(DC_ET, &FParam); 
      SetHeaderParam(dHandle, DM_ET, FParam);
      GetIntParam(DC_DELTAX, &IParam); 
      SetIntHeaderParam(dHandle, DM_DELTAX, IParam);
      SetIntHeaderParam(dHandle, DM_DTYPE, 0x04);
      GetIntParam(DC_PTIME, &IParam); 
      SetIntHeaderParam(dHandle, DM_PTIME, IParam);
      GetIntParam(DC_J, &IParam); 
      SetIntHeaderParam(dHandle, DM_J, IParam);
      CSize szData = GetDocSize();
      if (!InsertData(dHandle, DataPtr, DMult(szData.cx, szData.cy)))
        {
        Success = SaveDataSource(dHandle);
        }
      }
    }
  else // file came from disk, save with new name
    {
    if (RenameDataSource(dHandle, pszPathName));
      {
      Success = SaveDataSource(dHandle);
      }
    }
  if (Success)
    SetModifiedFlag(FALSE);
  else
    {
    CString rString;
    AfxFormatString1(rString, IDS_ERRFILEOPEN, pszPathName);
    AfxMessageBox(rString, MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
    Success = FALSE;
    }
  return Success;
}

CSize CDocUif::GetDocSize(void)
{
  short cX, cY;

  if (DocIsLastlive)
    {
    GetIntParam(DC_POINTS, &cX);
    GetIntParam(DC_TRACKS, &cY);
    }
  else
    {
    GetIntHeaderParam(dHandle, DM_POINTS, &cX);
    GetIntHeaderParam(dHandle, DM_TRACKS, &cY);
    }
  
  return CSize(cX, cY);
}

BOOL CDocUif::DocIsLive(void)
{
  return (DocIsLastlive);
}                

//////////////////////////////////////////////////////////////////////////////
// use to force read of data from board if needed; until DATMGR can do it
BOOL CDocUif::Updated(void)
{
  short thisL;
  CSize DocSize = GetDocSize();

  // handle empty document
  if (DocSize.cx == 0 || DocSize.cy == 0)
    return(FALSE);

  if (DocIsLastlive)
    GetIntParam(DC_L, &thisL);
  else
    thisL = LastL;
    
  if (m_Updated)
    SetModifiedFlag(TRUE);
  return (LastL != thisL) || m_Updated;
}

void CDocUif::Updated(BOOL bUpdated)
{
  short thisL;
  CSize DocSize = GetDocSize();

  // handle empty document
  if (DocSize.cx == 0 || DocSize.cy == 0)
    return;

  // Get Live status
  
  if (DocIsLastlive) // if live, valid depends on scan counter
    GetIntParam(DC_L, &thisL);
  else
    thisL = LastL;
  
  if (!bUpdated) // if forcing to valid
    LastL = thisL;
  else        // if forcing invalid
    LastL = thisL - 1;

  m_Updated = bUpdated;
  if (bUpdated)
    SetModifiedFlag(TRUE);
}

ULONG __huge * CDocUif::GetDataPtr(void)
{
  return(DataPtr);
}

/////////////////////////////////////////////////////////////////////////////
// CDocUif server implementation

COleServerItem *CDocUif::OnGetEmbeddedItem(void)
{
  // OnGetEmbeddedItem is called by the framework to get the COleServerItem
  //  that is associated with the document.  It is only called when necessary.

  CDispItem *pItem = new CDispItem(this);
  ASSERT_VALID(pItem);
  return pItem;
}

/////////////////////////////////////////////////////////////////////////////
// CDocUif diagnostics

#ifdef _DEBUG
void CDocUif::AssertValid() const
{
  COleServerDoc::AssertValid();
}

void CDocUif::Dump(CDumpContext& dc) const
{
  COleServerDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDocUif commands
