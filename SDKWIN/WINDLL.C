/* -----------------------------------------------------------------------
/
/  windll.c 
/  Startup and shutdown code for Windows implementation of OMA4 driver
/
/  Author: Morris Maynard
/  Copyright (c) 1994,  EG&G Instruments Inc.
/
/  $Header:
/  $Log:
/
------------------------------------------------------------------------*/
#ifdef XSTAT
#define PRIVATE
#else
#define PRIVATE static
#endif

#ifdef _WINOMA_ 
#include <windows.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "oma4driv.h"
#include "detsetup.h"
#include "driverrs.h"
#include "cntrloma.h"
#include "access4.h"

extern HINSTANCE hinstLib = NULL;

/********************************************************************************************/
/* Write a 32-bit integer to the DLL private .INI file; return TRUE if successful           */
/********************************************************************************************/
BOOL WriteLongHexInt(const char * File, const char * Section, const char * Entry, ULONG value)
{
  char Str[32];
  
  wsprintf(Str, "0x%lx", value);
  return WritePrivateProfileString(Section, Entry, Str, File);
}

/********************************************************************************************/
/* Display the values from the INI file in a message box; IsOk selects Info or STOP icon    */
/********************************************************************************************/
void DispMsg(const char * Title, USHORT port, ULONG addr, ULONG size, BOOL IsOK)
{           
  UINT Style;
  char Msg[140], /* be careful - Msg must be big enough to hold formatted string! */
       *MsgFormat = "Config Settings: Port Addr:   0x%3.3x\n"
                    "                 Base Addr:   0x%6.6lx\n"
                    "                 Memory Size: 0x%6.6lx";
                
  wsprintf(Msg, MsgFormat, port, addr, size);
  if (IsOK)
    Style = ( MB_SYSTEMMODAL | MB_ICONINFORMATION | MB_OK );
  else
    Style = ( MB_SYSTEMMODAL | MB_ICONSTOP | MB_OKCANCEL );

  MessageBox(NULL, (LPCSTR)Msg, (LPCSTR)Title, Style);
}

/********************************************************************************************/
/* Startup routine for the Windows DLL implementation of the OMA4 driver                    */
/* Gets values from the .INI file, then calls init_detector to load the monitor file, etc.  */
/********************************************************************************************/
int FAR PASCAL LibMain(HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize, LPSTR lpCmdLine)
{
  static const char * dConfigFile = "OMA4CFG.INI",
                    * dUserSect   = "User",
                    * dEntryVer   = "Verify",
                    * dConfigSect = "Detector 0",
                    * dEntryPort  = "Port Address",
                    * dEntryBase  = "Base Address",
                    * dEntrySize  = "Memory Size",
                    * dHiShift    = "Upper 2Meg";
  static char Str[40];
  USHORT port; 
  ULONG memaddr, memsize;
  BOOL verbose = FALSE;
  static BOOL WasInit = FALSE;
  
  if (!WasInit) /* only do this when DLL is first loaded */
    {
    hinstLib = hInstance;

    det_setup->det_addr = port = 
      (USHORT)GetPrivateProfileInt(dConfigSect, dEntryPort, 0x300, dConfigFile);
    GetPrivateProfileString(dConfigSect, dEntryBase, "0xc00000", Str, sizeof(Str), dConfigFile);
    det_setup->memory_base  = memaddr = strtoul(Str, NULL, 0);
    GetPrivateProfileString(dConfigSect, dEntrySize, "0x200000", Str, sizeof(Str), dConfigFile);
    det_setup->memory_size = memsize = strtoul(Str, NULL, 0);
    GetPrivateProfileString(dUserSect, dEntryVer, "True", Str, sizeof(Str), dConfigFile);
    _strupr(Str);
    if (strstr(Str, "TRUE") || strstr(Str, "YES") || strstr(Str, "1"))
       verbose = TRUE;
    GetPrivateProfileString(dConfigSect, dHiShift, "False", Str, sizeof(Str), dConfigFile);
    _strupr(Str);
    if (strstr(Str, "TRUE") || strstr(Str, "YES") || strstr(Str, "1"))
       hi_shift = TRUE;
       
    /* Write the current values to the init file.  In this way, the file will be created */
    /* if it does not already exist                                                      */                    
    
    if (WriteLongHexInt(dConfigFile, dConfigSect, dEntryPort, (ULONG)port) &&
        WriteLongHexInt(dConfigFile, dConfigSect, dEntryBase, memaddr) &&
        WriteLongHexInt(dConfigFile, dConfigSect, dEntrySize, memsize) &&
        WritePrivateProfileString(dUserSect, dEntryVer, verbose ? "True" : "False", dConfigFile) &&
        WritePrivateProfileString(dConfigSect, dHiShift, hi_shift ? "True" : "False", dConfigFile))
      {
      if (verbose)
        DispMsg(dConfigFile, port, memaddr, memsize, TRUE);
      }
    else
      {  
      DispMsg(dConfigFile, port, memaddr, memsize, FALSE);
      return 0;
      }                        
    detector_index = 0; /* start with first detector selected */
    access_startup_detector(memaddr, memsize);
    WasInit = TRUE;
    }
  else
    port = det_setup->det_addr;        /* Use current port address */
  
  // must return non-zero or Windows will unload DLL

  SetIntParam(DC_DETPORT, port);
  if (InitDetector() != ERROR_FAKEDETECTOR) /* if board is present */
    {
    return TRUE;  /* success */
    }
  else
//    return FALSE; /* fail, DLL unloads */
    return TRUE; /* success */
}

int CALLBACK WEP(nExitType)
{          
  ShutdownDetector();
  return 0;
}
