#include <windows.h>
#include <lzexpand.h>                                            
#include <shellapi.h>
#include <stdio.h>
#include <io.h>
#include <string.h>

#include "resource.h"
                    
#define _countof(X) sizeof(X) / sizeof(X[0])
#define NUM_FILES 4
char szWinDirBuf[128];

char *Names[4][4] = {
                 {
                 "setup1.ex_",
                 "vbrun300.dl_",
                 "ver.dl_",
                 "setupkit.dl_",
                 },
                 {
                 "setup1.ex_",
                 "system\\vbrun300.dl_",
                 "system\\ver.dl_",
                 "system\\setupkit.dl_",
                 },
               };

BOOL CALLBACK SetupDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
  switch (message) 
    { 
    case WM_INITDIALOG: 
    return TRUE; 
 
    case WM_COMMAND: 
    return TRUE; 
    } 
  return FALSE; 
}
                   
static LPSTR FullPathName(LPSTR FileName)
{                                                  
  char szPath[128];
  memset(szPath, 0, sizeof(szPath));
  strcpy(szPath, szWinDirBuf);
  _fstrcat(szPath, FileName);
  return szPath;
}
                   
static BOOL FilesAreThere()
{
  for (int i = 0; i < _countof(Names); i++)
    {
    if (-1 == access(Names[0][i], 0))
      {
      char Msg[80];
      sprintf(Msg, "Missing File: %s", Names[0][i]);
      MessageBox(0, Msg, "Setup Error", MB_APPLMODAL | MB_ICONSTOP);
      return FALSE;
      }
    }
  return TRUE;
}

int PASCAL WinMain(HINSTANCE hinstCurrent, HINSTANCE hinstPrevious,
    LPSTR lpszCmdLine, int nCmdShow)
{
  MSG msg;

  if (hinstPrevious != NULL)           /* other instances?      */
    return FALSE;                      /* there can be only one! */

  OFSTRUCT ofStrSrc;
  OFSTRUCT ofStrDest;
  HFILE hfSrcFile, hfDstFile;
  int i;
  
  GetWindowsDirectory((LPSTR)szWinDirBuf, sizeof(szWinDirBuf));
  strcat(szWinDirBuf, "\\");

   if (!FilesAreThere())
     return 0;
             
  RECT rctWin, rctDlg;
  HWND hwndDsk = GetDesktopWindow();
  GetWindowRect(hwndDsk, &rctWin);
  HWND hwndDlg = 
    CreateDialog(hinstCurrent, MAKEINTRESOURCE(IDD_SETUP), NULL, (DLGPROC)SetupDlgProc);
  GetWindowRect(hwndDlg, &rctDlg);
  int x = ((rctWin.right - rctWin.left) - (rctDlg.right - rctDlg.left)) / 2;
  int y = ((rctWin.bottom - rctWin.top) - (rctDlg.bottom - rctDlg.top)) / 2;
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE); 
  ShowWindow(hwndDlg, SW_SHOW);
  /* Allocate internal buffers for the CopyLZFile function. */
    
  LZStart();
    
  /* Open, copy, and then close the files. */
  for (i = 0; i < NUM_FILES; i++) 
    {
    hfSrcFile = LZOpenFile(Names[0][i], &ofStrSrc, OF_READ);
    
    hfDstFile = LZOpenFile(FullPathName(Names[1][i]), &ofStrDest, OF_CREATE);
    CopyLZFile(hfSrcFile, hfDstFile);
    LZClose(hfSrcFile);
    LZClose(hfDstFile);
    }
    
  LZDone(); /* free the internal buffers */  
  
  // MessageBox(0, "Almost done", "Setup", MB_APPLMODAL | MB_OK);
  DestroyWindow(hwndDlg);
  ShellExecute(NULL, NULL, "setup1.exe", NULL, szWinDirBuf, SW_SHOWMAXIMIZED);
  return (int) msg.wParam;    /* return value of PostQuitMessage */
}

