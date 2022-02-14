// basedisp.h : main header file for the BASEDISP application
//
//////////////////////////////////////////////////////////////////////////
//   This revision:  $Revision:   1.1  $
//   Last check-in   $Date:   29 Sep 1995 20:56:58  $
//   Last changes by $Author:   MLM  $
//   Changes:
//   $Log:   S:/SOFTWARE/WINX20/BASEDISP/SOURCE/BASEDISP.H_V  $
// 
//    Rev 1.1   29 Sep 1995 20:56:58   MLM
// Add PVCS info tags
//  
//////////////////////////////////////////////////////////////////////////


// $
#ifndef __AFXWIN_H__
  #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CBasedispApp:
// See basedisp.cpp for the implementation of this class
//

class CBasedispApp : public CWinApp
{
public:
  CBasedispApp();

// Overrides
  virtual BOOL InitInstance();

// Implementation

  //{{AFX_MSG(CBasedispApp)
  afx_msg void OnAppAbout();
    // NOTE - the ClassWizard will add and remove member functions here.
    //    DO NOT EDIT what you see in these blocks of generated code !
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// VB-Event extern declarations

//{{AFX_VBX_REGISTER()
  extern UINT NEAR VBN_CHANGE;
  extern UINT NEAR VBN_MOUSEDOWN;
  extern UINT NEAR VBN_MOUSEUP;
  extern UINT NEAR VBN_CLICK;
//}}AFX_VBX_REGISTER

/////////////////////////////////////////////////////////////////////////////

