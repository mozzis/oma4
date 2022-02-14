// Copyright (C) 1998 Mayn Idea Software
// All rights reserved
// basedisp.cpp : Defines the class behaviors for the application.
//
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 21:00:32  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/BASEDISP.CPV  $
// 
//    Rev 1.1   29 Sep 1995 21:00:32   MLM
// Add PVCS info tags
// 
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "basedisp.h"
#include "mainfrm.h"
#include "docuif.h"
#include "View.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBasedispApp

BEGIN_MESSAGE_MAP(CBasedispApp, CWinApp)
  //{{AFX_MSG_MAP(CBasedispApp)
  ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
  //}}AFX_MSG_MAP
  // Standard file based document commands
  ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
  ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
  // Standard print setup command
  ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
  // Other commands
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBasedispApp construction

CBasedispApp::CBasedispApp()
{
  // TODO: add construction code here,
  // Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBasedispApp object

CBasedispApp NEAR theApp;

/////////////////////////////////////////////////////////////////////////////
// CBasedispApp initialization

static BOOL NEAR PASCAL SetRegKey(LPCSTR lpszKey, LPCSTR lpszValue)
{
  if (::RegSetValue(HKEY_CLASSES_ROOT, lpszKey, REG_SZ,
      lpszValue, lstrlen(lpszValue)) != ERROR_SUCCESS)
  {
    TRACE1("Warning: registration database update failed for key '%Fs'\n",
      lpszKey);
    return FALSE;
  }
  return TRUE;
}

BOOL CBasedispApp::InitInstance()
{

  // Initialize OLE 2.0 libraries
  if (!AfxOleInit())
  {
    AfxMessageBox(IDP_OLE_INIT_FAILED);
    return FALSE;
  }
  // Standard initialization
  // If you are not using these features and wish to reduce the size
  //  of your final executable, you should remove from the following
  //  the specific initialization routines you do not need.

  SetDialogBkColor();        // Set dialog background color to gray
  LoadStdProfileSettings();  // Load standard INI file options (including MRU)
  EnableVBX();               // Initialize VBX support
  // Register the application's document templates.  Document templates
  //  serve as the connection between documents, frame windows and views.

  CSingleDocTemplate* pDocTemplate1 = new CSingleDocTemplate(
    IDR_MAINFRAME,
    RUNTIME_CLASS(CDocUif),
    RUNTIME_CLASS(CMainFrame),     // main SDI frame window
    RUNTIME_CLASS(CBasicView));
  AddDocTemplate(pDocTemplate1);
  // Add the following code.
  // Registration Type Name
//  CString strFileTypeName;
//  pDocTemplate->GetDocString(strFileTypeName, CSingleDocTemplate::regFileTypeName);
  // Add another file extension to registration database.
  // If you double-click a .HID or .TIF file in File Manager it
  // automatically starts this application.
  //
  // SetRegKey adds a file extension to the Registration Database
//  SetRegKey(".HID", strFileTypeName);
  m_pMainWnd->DragAcceptFiles(TRUE);
  EnableShellOpen();
  RegisterShellFileTypes();

  // Parse the command line to see if launched as OLE server
  if (RunEmbedded() || RunAutomated())
    {
    // Application was run with /Embedding or /Automation.  Don't show the
    //  main window in this case.
    return TRUE;
    }

  if (m_lpCmdLine[0] == '\0')
    {
    // create a new (empty) document
    OnFileNew();
    }
  else
    {
    // open an existing document
    OpenDocumentFile(m_lpCmdLine);
    }
  return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
  CAboutDlg();

// Dialog Data
  //{{AFX_DATA(CAboutDlg)
  enum { IDD = IDD_ABOUTBOX };
  //}}AFX_DATA

// Implementation
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //{{AFX_MSG(CAboutDlg)
    // No message handlers
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
  //{{AFX_DATA_INIT(CAboutDlg)
  //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAboutDlg)
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
  //{{AFX_MSG_MAP(CAboutDlg)
    // No message handlers
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CBasedispApp::OnAppAbout()
{
  CAboutDlg aboutDlg;
  aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// VB-Event registration
// (calls to AfxRegisterVBEvent will be placed here by ClassWizard)

//{{AFX_VBX_REGISTER_MAP()
  UINT NEAR VBN_CHANGE = AfxRegisterVBEvent("CHANGE");
  UINT NEAR VBN_MOUSEUP = AfxRegisterVBEvent("MOUSEUP");
  UINT NEAR VBN_CLICK = AfxRegisterVBEvent("CLICK");
//}}AFX_VBX_REGISTER_MAP

/////////////////////////////////////////////////////////////////////////////
// CBasedispApp commands
