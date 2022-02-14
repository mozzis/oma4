/* OMADATA - OMA4 Windows Data Manager DLL */

// Future directions:
// Add XlateData2BMP API:
//   BOOL FAR PASCAL _export XlateData2BMP(DHANDLE dHandle, VHP hpBMP, DSIZE dStart, DSIZE dAmount, 
//                                      DWORD Offset, float Scale /* char cType */);
// Takes dDataX X dDataY rectangle and copies it to hpBMP, mapping from cType down to 8-bits
// returns TRUE if successful
// dStart is offset to start, dAmount is X and Y points to copy
// Scale and Offset are used in mapping from a big word size to a small one - like from 32 to 8 bits.
// Of course this function will incorporate my 32-bit assembler routine (CopyBlock) in OMABLT.ASM.
// Once this function exists in the DLL, I can have my app call it instead of calling CopyBlock d
// directly.
// Then if I expand CopyBlock to deal with floats and words and bytes, I can start adding on the
// other file-type modules I have.
// Add GetData API:
//   BOOL FAR PASCAL _export GetData(DHANDLE dHandle, VHP hpData, DSIZE dStart, DSIZE dAmount, char cType);
// Takes dDataX X dDataY rectangle and copies it to hpData, changing from the source's type cType 
// to cType
// returns TRUE if successful
// You can get one point if you want, or megabytes.
// With these two functions, I could move all the references to DataPtr out of the application
// (although we keep the GetDataPointer function for high-performance math, etc.)
// Except DATMGR would then also need to handle lastlive as a source.
// Add IsSourceLive API:
//  extern "C" BOOLEAN FAR PASCAL _export GetSourceIsLive(DHANDLE dHandle);
// returns TRUE if source is lastlive data.
// Then OpenData source just needs to check if "lastlive" is the input name, and if so
// then call OMA4WIN to get the header parameters, including the data pointer, instead of
// reading from a file. And GetSource(was File)DataPointer would not do the disk read.
// In fact, there should be a "dirty" flag in the internal entry somewhere so that the data
// only gets read once (unless it is changed on disk somehow?)
// And finally, the GetParam & SetParam functions must pass on to OMA4WIN if the source is live.
  
#include <windows.h>
#include <windowsx.h>
//#include <stdio.h>
#include <string.h>
//#include <io.h>
//#include <assert.h>

#if (WINVER < 0x030a)
  #error WINVER bogus!!
#endif

#include "oma4blt.h"
#include "datmgr.h"
#include "datadir.h"
#include "wintcl.h"

// Local Data
//////////////////////////////////////////////////////////////////////////
DATADIR Directory;
HGLOBAL hDir, hEnt = NULL;

static const char *LastLiveName = "lastlive";

// Initialize library on load
//////////////////////////////////////////////////////////////////////////
extern "C" int CALLBACK LibMain(HINSTANCE hinst, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{
  Directory.pEntries = NULL;
  Directory.ECount = 0;

  hEnt = GlobalAlloc(GMEM_DDESHARE | GMEM_FIXED, SZENTRY);
  if (!hEnt)
    return(0);      // Unload DLL, couldn't get memory for at least 1 entry
  GlobalFree(hEnt); // don't really allocate until needed
  hEnt = NULL;      // important flag for first NewHandle call
  return 1;         // return success
}

// make sure handle is in legal range: 0 <= dH < ECount
//////////////////////////////////////////////////////////////////////////
static BOOL GoodHandle(DHANDLE dHandle)
{
  return (dHandle < Directory.ECount && dHandle >= 0);
}

// Create a new handle to a data directory entry
//////////////////////////////////////////////////////////////////////////
static DHANDLE NewHandle(void) // DirEntry::DirEntry() { return this; }
{
  if (!hEnt) // if this is the first entry
    {
    hEnt = GlobalAlloc(GMEM_DDESHARE | GMEM_FIXED, SZENTRY);
    if (!hEnt)
      return(INVALID_HANDLE);  // Return error, couldn't get memory for at least 1 entry

    Directory.pEntries = (LPENTRY)GlobalLock(hEnt); // Create pointer to entry array
    if (!Directory.pEntries)   // Make sure lock was successful
      {
      GlobalFree(hEnt);        // Try to clean up
      return(INVALID_NOMEM);   // return error
      }
    Directory.ECount = 0;      // initial count on first call
    }
  else // all calls but the first come here
    {
    // resize the entry array
    LPENTRY pEntries = (LPENTRY)
      GlobalReAllocPtr(Directory.pEntries, SZENTRY * (Directory.ECount +1), GMEM_MOVEABLE);
    if (!pEntries)                 // couldn't increase the array size
      return(INVALID_NOMEM);       // return error
    Directory.pEntries = pEntries; // new pointer from ReAlloc
    }
  _fmemset(&Directory.pEntries[Directory.ECount], 0, SZENTRY);
  Directory.pEntries[Directory.ECount].RefCount = 1;  // This is 1st and only reference
  return Directory.ECount++;                          // return index before increment
}

// remove a data dir entry
//////////////////////////////////////////////////////////////////////////
static DERR DeleteHandle(DHANDLE dHandle) // DirEntry::~DirEntry() { }
{
  if (!GoodHandle(dHandle))
    return (ERR_DMGR_BADHANDLE);                         
                           
  Directory.pEntries[dHandle].RefCount -= 1;  // one less "open" reference
  if (Directory.pEntries[dHandle].RefCount)   // if other "opens" pending
    return(ERR_DMGR_OK);                      // don't really delete
    
  // if not the last entry, first move following entries                                     
  if (dHandle < Directory.ECount - 1)
    memmove(&Directory.pEntries[dHandle], &Directory.pEntries[dHandle+1], SZENTRY);
    
  Directory.ECount -= 1; // decrease entry count
  if (Directory.ECount)  // if there are any left
    {
    LPENTRY pEntries =  (LPENTRY)    // reduce the aray size
      GlobalReAllocPtr(Directory.pEntries, SZENTRY * (Directory.ECount +1), GMEM_MOVEABLE);
    if (!pEntries)
      return(ERR_DMGR_NOMEM);
    Directory.pEntries = pEntries;
    }
  else // in none left, free memory from (only) remaining entry
    {
    GlobalUnlock(hEnt);
    GlobalFree(hEnt);
    hEnt = NULL;               // flag for NewHandle
    Directory.pEntries = NULL; // easier bug checking
    }
  return(ERR_DMGR_OK);
}

// look at a file and try to tell who made it
//////////////////////////////////////////////////////////////////////////
static enum src_types IdentifyFile(FILE *fhnd)
{
  if (IsFileTCL(fhnd))
    return STYPE_OMA4TCL32;        // Hidris format
  return STYPE_INVALID;
}

// fill the DIR_ENTRY struct from an identified file
//////////////////////////////////////////////////////////////////////////
static BOOL LoadEntryFromFile(enum src_types SType, FILE *fhnd, LPENTRY pEntry)
{
  pEntry->SourceType = SType; // may make dependent on successful load later
  switch(SType)
    {
    case STYPE_OMA4TCL32:
      return ReadTCLHeader(fhnd, pEntry);
    }
  return FALSE;
}

// ship the data entry out to disk file
//////////////////////////////////////////////////////////////////////////
static BOOL SaveEntryToFile(enum src_types SType, FILE *fhnd, LPENTRY pEntry)
{
  pEntry->SourceType = SType; // may make dependent on successful load later
  switch(SType)
    {
    case STYPE_OMA4TCL32:
      if (WriteTCLHeader(fhnd, pEntry))
        return TRUE;
      return WriteTCLData(fhnd, pEntry, 0L, -1);
    }
  return FALSE;
}


// look in the directory for a file with a given name
//////////////////////////////////////////////////////////////////////////
static DHANDLE SearchDirForName(LPCSTR lpName)
{
  int i;
  LPENTRY pEntry;
  
  if (!Directory.ECount)
    return(INVALID_HANDLE);
  
  for (i = 0; i < Directory.ECount; i++)
    {
    pEntry = &Directory.pEntries[i];
    if (!strcmpi(lpName, pEntry->Name))
      break;
    }
   if (i < Directory.ECount)
     return i;
   return(INVALID_HANDLE);
}

// API function - find if an open source is live data
//////////////////////////////////////////////////////////////////////////
extern "C" BOOL FAR PASCAL _export GetSourceIsLive(DHANDLE dHandle)
{
  LPENTRY pEntry;

  if (!GoodHandle(dHandle))
    return(FALSE);
  
  pEntry = &Directory.pEntries[dHandle];
  return(!_fstrcmp(pEntry->Name, LastLiveName));
}
                                          
// API function - get pointer to data
//////////////////////////////////////////////////////////////////////////
extern "C" void __huge * PASCAL _export GetFileDataPointer(DHANDLE dHandle)
{
  FILE * dFile;
  enum src_types sType;
  LPENTRY pEntry;
  DWORD szData = -1;
  
  if (!GoodHandle(dHandle))
    return NULL;
                          
  pEntry = &Directory.pEntries[dHandle];
  
  dFile = pEntry->hFile;
  sType = pEntry->SourceType;
  szData = -1L;       // signal read ALL the data
  
  switch (sType)
    {
    case STYPE_OMA4TCL32:
      ReadTCLData(dFile, pEntry, 0L, szData);
      return pEntry->pData;
    break;
    }
  return NULL;
}

// API function - start access to data from disk or live
//////////////////////////////////////////////////////////////////////////
extern "C" DHANDLE FAR PASCAL _export OpenDataSource(LPCSTR lpName)
{
  LPENTRY pEntry;
  DHANDLE retHandle;
  FILE * fhnd;
  enum src_types SType;

  if (!lpName)
    return(INVALID_HANDLE);   // Return error, name pointer was NULL

  retHandle = SearchDirForName(lpName);
  
  if (retHandle >= 0)         // if a valid handle, return it
    {
    Directory.pEntries[retHandle].RefCount++; // one more "open" reference
    return(retHandle);        // Done
    }
    
  retHandle = NewHandle();    // Else create a new handle
  if (retHandle < 0)          // If no handle could be made
    return(retHandle);        // return error
            
  pEntry = &Directory.pEntries[retHandle];
            
  _fstrcpy(pEntry->Name, lpName); // save name
  fhnd = fopen(lpName, "rb");
  if (!fhnd)                  // if couldn't open file
    {
    DeleteHandle(retHandle);  // assume retHandle still valid
    return(INVALID_NOFILE);   // Return error, file not found
    }
    
  pEntry->hFile = fhnd;
  
  SType = IdentifyFile(fhnd); // See if EG&G type file
  if (SType == STYPE_INVALID || SType == STYPE_OTHER)
    {
    fclose(fhnd);
    DeleteHandle(retHandle);  // assume retHandle still valid
    return(INVALID_FILETYPE); // Return error, filetype not recognized
    }
    
  if (LoadEntryFromFile(SType, fhnd, pEntry))
    {
    fclose(fhnd);
    DeleteHandle(retHandle);  // assume retHandle still valid
    return(INVALID_FILEREAD); // Return error, file read error
    }
  return(retHandle);          // Successful return
}
 
// API function - create new data source
//////////////////////////////////////////////////////////////////////////
DHANDLE FAR PASCAL _export NewDataSource(LPCSTR lpName)
{
  LPENTRY pEntry;
  DHANDLE retHandle;

  if (!lpName)
    return(INVALID_HANDLE);   // Return error, name pointer was NULL

  retHandle = SearchDirForName(lpName);
  
  if (retHandle >= 0)         // if a valid handle, return it
    {
    Directory.pEntries[retHandle].RefCount++; // one more "open" reference
    return(retHandle);        // Done
    }
    
  retHandle = NewHandle();    // Else create a new handle
  if (retHandle < 0)          // If no handle could be made
    return(retHandle);        // return error
  pEntry = &Directory.pEntries[retHandle];
  pEntry->SourceType = STYPE_OMA4TCL32; // Default for now         
  _fstrcpy(pEntry->Name, lpName); // save name
  pEntry->hFile = 0;

  return(retHandle);        // Done
    
}

// API function - put data in data source
//////////////////////////////////////////////////////////////////////////
BOOL FAR PASCAL _export InsertData(DHANDLE dHandle, void __huge * hpData, DWORD szData)
{
  DATA_BLOCK __huge * hpDBlock;
  LPENTRY pEntry;
  size_t SizeX, SizeY, szWord;
  DWORD Size;
  BOOL Fail = TRUE;
  
  if (!hpData || !GoodHandle(dHandle))
    return TRUE;
  
  pEntry = &Directory.pEntries[dHandle];

  if (szData = -1) // Save "All"
    {
    szWord = (pEntry->Method.DACSetup.DataType & 0x0F);
    SizeX = pEntry->Method.ScanSetup.Points * szWord;
    SizeY = pEntry->Method.ScanSetup.Tracks;
    Size = szData = DWORD((DWORD)SizeX * (DWORD)SizeY);
    }
  else
    Size = szData;

  pEntry->MCount += 1;
  Size += (DWORD)SZDBLOCK;

  if (!pEntry->pData)
    {
    pEntry->hData = GlobalAlloc(GPTR | GMEM_DDESHARE, Size);
    if (!pEntry->hData)
      return (TRUE);
    hpDBlock = (DATA_BLOCK __huge *)GlobalLock(pEntry->hData);
    if (!hpDBlock)
      {
      GlobalFree(pEntry->hData);
      return (TRUE);
      }
    }
  else
    {
     hpDBlock = (DATA_BLOCK __huge *) ((DWORD)(pEntry->pData) - (DWORD)SZDBLOCK);
     hpDBlock = (DATA_BLOCK __huge *)
        GlobalReAllocPtr(hpDBlock, Size, GMEM_MOVEABLE);
     if (!hpDBlock)
       return TRUE;
     }

  pEntry->pData = (void __huge *)(&(((char __huge *)hpDBlock)[SZDBLOCK]));
 
  hpDBlock->MemData = FALSE;                     // TRUE if data is in (OMA4) memory
  hpDBlock->szData = Size - SZDBLOCK;            // size of Y Data in bytes
  
  for (DWORD i = 0; i < szData; i++)
    ((char __huge *)pEntry->pData)[i] = ((char __huge *)hpData)[i];
  return FALSE;
}

// API function - write data in source to storage
//////////////////////////////////////////////////////////////////////////
BOOL FAR PASCAL _export SaveDataSource(DHANDLE dHandle)
{
  LPENTRY pEntry;
  DHANDLE retHandle;
  FILE *fhnd;
  enum src_types SType;

  if (!GoodHandle(dHandle))
    return(INVALID_HANDLE);   // Return error, name pointer was NULL

  pEntry = &Directory.pEntries[dHandle];
  if (!pEntry)
    return(TRUE); // something really worng, Null entry ptr.
    
  fhnd = pEntry->hFile;

  if (fhnd)
    fhnd = freopen(pEntry->Name, "w+b", fhnd);
  else
    fhnd = fopen(pEntry->Name, "w+b");
  
  if (!fhnd)                  // if couldn't open file
    return(TRUE);             // Return error
    
  pEntry->hFile = fhnd;
  SType = pEntry->SourceType;
  
  if (SaveEntryToFile(SType, fhnd, pEntry))
    {
    fclose(fhnd);
    return TRUE; // Return error, file read error
    }
  fclose(fhnd);
  return(retHandle);          // Successful return
}

// API function - change name of data source in internal directory
//////////////////////////////////////////////////////////////////////////
BOOL FAR PASCAL _export RenameDataSource(DHANDLE dHandle, LPCSTR lpszName)
{
  DHANDLE OldHandle;
  LPENTRY pEntry;

  if (!GoodHandle(dHandle))
    return(TRUE);   // Return error, name pointer was NULL
                                 
  OldHandle = SearchDirForName(lpszName);
  
  if (!GoodHandle(OldHandle)) // no other source has this name
    {
    pEntry = &Directory.pEntries[dHandle];
    strcpy(pEntry->Name, lpszName);
    return FALSE;
    }
  else
    {
    if (OldHandle == dHandle)
      return FALSE; // Same name, same slot - NOP
    else
      return TRUE;  // Name already exists in different slot - error.
    }
  return TRUE;
}
 

// API function - get parameter from data source header
//////////////////////////////////////////////////////////////////////////
DERR FAR PASCAL _export GetIntHeaderParam(DHANDLE dHandle, enum fparam Param, short far *retval)
{
  LPENTRY pEntry;
  UNI_METHOD _far * pMeth;

  if (!GoodHandle(dHandle))
    return (ERR_DMGR_BADHANDLE);                         

  pEntry = &Directory.pEntries[dHandle];
  pMeth = &pEntry->Method;
  switch (Param) 
    {
    case DM_ACTIVEX:        /* 02 */
      *retval = pMeth->DetStructSetup.ActiveX;
      break;
    case DM_ACTIVEY:        /* 03 */
      *retval = pMeth->DetStructSetup.ActiveX;
      break;
    case DM_ADPREC:         /* 05 */
      break;
    case DM_ANTIBLOOM:      /* 06 */
      break;
    case DM_BYTES:          /* 07 */
      break;
    case DM_COOLSTAT:       /* 09 */
      break;
    case DM_COOLLOCK:       /* 10 */
      break;
    case DM_COOLONOFF:      /* 11 */
      break;
    case DM_COOLTYPE:       /* 12 */
      break;
    case DM_CONTROL:        /* 13 */
      break;
    case DM_DAMAX:          /* 14 */
      break;
    case DM_DAPROG:         /* 15 */
      break;
    case DM_DELTAX:         /* 16 */
      *retval = pMeth->ScanSetup.XMap.Delta[0];
      break;
    case DM_DELTAY:         /* 17 */
      *retval = pMeth->ScanSetup.YMap.Delta[0];
      break;
    case DM_DETPORT:        /* 18 */
      break;
    case DM_DMODEL:         /* 19 */
      break;
    case DM_DTEMP:          /* 20 */
      *retval = pMeth->AuxSetup.Temperature;
      break;
    case DM_DTYPE:          /* 20 */
      *retval = pMeth->DACSetup.DataType;
      break;
    case DM_DERROR:         /* 21 */
      break;
    case DM_EXPROWS:        /* 22 */
      break;
    case DM_FRAME:          /* 23 */
      break;
    case DM_FREQ:           /* 24 */
      break;
    case DM_H:              /* 25 */
      break;
    case DM_HIMEM:          /* 26 */
      break;
    case DM_I:              /* 27 */
      *retval = pMeth->DACSetup.Scans;
      break;
    case DM_ID:             /* 28 */
      break;
    case DM_IMODE:          /* 29 */
      break;
    case DM_INPIA:          /* 30 */
      break;
    case DM_J:              /* 31 */
      *retval = pMeth->DACSetup.Mems;
      break;
    case DM_K:              /* 32 */
      break;
    case DM_L:              /* 33 */
      break;
    case DM_MAXMEM:         /* 34 */
      break;
    case DM_MEM:            /* 35 */
      break;
    case DM_MASK:           /* 36 */
      break;
    case DM_OUTPUTREG:      /* 37 */
      break;
    case DM_OUTREG:         /* 38 */
      break;
    case DM_OUTPIA:         /* 39 */
      break;
    case DM_PAUDIO:         /* 40 */
      break;
    case DM_PLSR:           /* 41 */
      break;
    case DM_PNTMODE:        /* 42 */
      break;
    case DM_POINT:          /* 43 */
      break;
    case DM_POINTS:         /* 44 */
      *retval = pEntry->Method.ScanSetup.Points;
    break;
    case DM_PRESCAN:        /* 45 */
      break;
    case DM_PTIME:          /* 46 */
      *retval = pMeth->AuxSetup.PixTimeIndex;
      break;
    case DM_PTRIG:          /* 47 */
      break;
    case DM_PTRIGNUM:       /* 48 */
      break;
    case DM_PTRIGSRC:       /* 49 */
      break;
    case DM_PTRIGTRSH:      /* 50 */
      break;
    case DM_REGIONS:        /* 51 */
      break;
    case DM_REGSIZE:        /* 52 */
      break;
    case DM_RUN:            /* 53 */
      break;
    case DM_SAMEET:         /* 54 */
      break;
    case DM_SCITC:          /* 55 */
      break;
    case DM_SHFTMODE:       /* 56 */
      break;
    case DM_SHUTMODE:       /* 57 */
      break;
    case DM_SPEED:          /* 58 */
      break;
    case DM_SRTREG:         /* 59 */
      break;
    case DM_STIME:          /* 60 */
      break;
    case DM_STOP:           /* 61 */
      break;
    case DM_STREAKMODE:     /* 62 */
      break;
    case DM_SUBT:           /* 63 */
      break;
    case DM_THERE:          /* 64 */
      break;
    case DM_TRACK:          /* 65 */
      break;
    case DM_TRACKS:         /* 66 */
      *retval = pEntry->Method.ScanSetup.Tracks;
    break;
    case DM_TRIGS:          /* 67 */
      break;
    case DM_TRKMODE:        /* 68 */
      break;
    case DM_WFTC:           /* 69 */
      break;
    case DM_WFTO:           /* 70 */
      break;
    case DM_X0:             /* 71 */
      break;
    case DM_Y0:             /* 72 */
      break;
  default:
      return (ERR_DMGR_BADPARAM);
    }
  return (ERR_DMGR_OK);
}

// API function - set parameter in data source header
//////////////////////////////////////////////////////////////////////////
DERR FAR PASCAL _export SetIntHeaderParam(DHANDLE dHandle, enum fparam Param, short setval)
{
  LPENTRY pEntry;
  UNI_METHOD _far * pMeth;

  if (!GoodHandle(dHandle))
    return (ERR_DMGR_BADHANDLE);                         

  pEntry = &Directory.pEntries[dHandle];
  pMeth = &pEntry->Method;
  switch (Param) 
    {
    case DM_ACTIVEX:        /* 02 */
      pMeth->DetStructSetup.ActiveX = setval;
      break;
    case DM_ACTIVEY:        /* 03 */
      pMeth->DetStructSetup.ActiveX = setval;
      break;
    case DM_ADPREC:         /* 05 */
      break;
    case DM_ANTIBLOOM:      /* 06 */
      break;
    case DM_BYTES:          /* 07 */
      break;
    case DM_COOLSTAT:       /* 09 */
      break;
    case DM_COOLLOCK:       /* 10 */
      break;
    case DM_COOLONOFF:      /* 11 */
      break;
    case DM_COOLTYPE:       /* 12 */
      break;
    case DM_CONTROL:        /* 13 */
      break;
    case DM_DAMAX:          /* 14 */
      break;
    case DM_DAPROG:         /* 15 */
      break;
    case DM_DELTAX:         /* 16 */
      pMeth->ScanSetup.XMap.Delta[0] = setval;
      break;
    case DM_DELTAY:         /* 17 */
      pMeth->ScanSetup.YMap.Delta[0] = setval;
      break;
    case DM_DETPORT:        /* 18 */
      break;
    case DM_DMODEL:         /* 19 */
      break;
    case DM_DTEMP:          /* 20 */
      pMeth->AuxSetup.Temperature = setval;
      break;
    case DM_DTYPE:
      pMeth->DACSetup.DataType = (dat_type)setval;
      break;
    case DM_DERROR:         /* 21 */
      break;
    case DM_EXPROWS:        /* 22 */
      break;
    case DM_FRAME:          /* 23 */
      break;
    case DM_FREQ:           /* 24 */
      break;
    case DM_H:              /* 25 */
      break;
    case DM_HIMEM:          /* 26 */
      break;
    case DM_I:              /* 27 */
      pMeth->DACSetup.Scans = setval;
      break;
    case DM_ID:             /* 28 */
      break;
    case DM_IMODE:          /* 29 */
      break;
    case DM_INPIA:          /* 30 */
      break;
    case DM_J:              /* 31 */
      pMeth->DACSetup.Mems = setval;
      break;
    case DM_K:              /* 32 */
      break;
    case DM_L:              /* 33 */
      break;
    case DM_MAXMEM:         /* 34 */
      break;
    case DM_MEM:            /* 35 */
      break;
    case DM_MASK:           /* 36 */
      break;
    case DM_OUTPUTREG:      /* 37 */
      break;
    case DM_OUTREG:         /* 38 */
      break;
    case DM_OUTPIA:         /* 39 */
      break;
    case DM_PAUDIO:         /* 40 */
      break;
    case DM_PLSR:           /* 41 */
      break;
    case DM_PNTMODE:        /* 42 */
      break;
    case DM_POINT:          /* 43 */
      break;
    case DM_POINTS:         /* 44 */
      pEntry->Method.ScanSetup.Points = setval;
    break;
    case DM_PRESCAN:        /* 45 */
      break;
    case DM_PTIME:          /* 46 */
      pMeth->AuxSetup.PixTimeIndex = setval;
      break;
    case DM_PTRIG:          /* 47 */
      break;
    case DM_PTRIGNUM:       /* 48 */
      break;
    case DM_PTRIGSRC:       /* 49 */
      break;
    case DM_PTRIGTRSH:      /* 50 */
      break;
    case DM_REGIONS:        /* 51 */
      break;
    case DM_REGSIZE:        /* 52 */
      break;
    case DM_RUN:            /* 53 */
      break;
    case DM_SAMEET:         /* 54 */
      break;
    case DM_SCITC:          /* 55 */
      break;
    case DM_SHFTMODE:       /* 56 */
      break;
    case DM_SHUTMODE:       /* 57 */
      break;
    case DM_SPEED:          /* 58 */
      break;
    case DM_SRTREG:         /* 59 */
      break;
    case DM_STIME:          /* 60 */
      break;
    case DM_STOP:           /* 61 */
      break;
    case DM_STREAKMODE:     /* 62 */
      break;
    case DM_SUBT:           /* 63 */
      break;
    case DM_THERE:          /* 64 */
      break;
    case DM_TRACK:          /* 65 */
      break;
    case DM_TRACKS:         /* 66 */
      pEntry->Method.ScanSetup.Tracks = setval;
    break;
    case DM_TRIGS:          /* 67 */
      break;
    case DM_TRKMODE:        /* 68 */
      break;
    case DM_WFTC:           /* 69 */
      break;
    case DM_WFTO:           /* 70 */
      break;
    case DM_X0:             /* 71 */
      break;
    case DM_Y0:             /* 72 */
      break;
  default:
      return (ERR_DMGR_BADPARAM);
    }
  return (ERR_DMGR_OK);
}

// API function - set parameter in data source header using floating point
//////////////////////////////////////////////////////////////////////////
DERR FAR PASCAL _export SetHeaderParam(DHANDLE dHandle, enum fparam Param, float setval)
{
  LPENTRY pEntry;
  UNI_METHOD _far * pMeth;

  if (!GoodHandle(dHandle))
    return (ERR_DMGR_BADHANDLE);                         

  pEntry = &Directory.pEntries[dHandle];
  pMeth = &pEntry->Method;
  switch (Param) 
    {
    case DM_ACTIVEX:        /* 02 */
      pMeth->DetStructSetup.ActiveX = (short)setval;
      break;
    case DM_ACTIVEY:        /* 03 */
      pMeth->DetStructSetup.ActiveX = (short)setval;
      break;
    case DM_ADPREC:         /* 05 */
      break;
    case DM_ANTIBLOOM:      /* 06 */
      break;
    case DM_BYTES:          /* 07 */
      break;
    case DM_COOLSTAT:       /* 09 */
      break;
    case DM_COOLLOCK:       /* 10 */
      break;
    case DM_COOLONOFF:      /* 11 */
      break;
    case DM_COOLTYPE:       /* 12 */
      break;
    case DM_CONTROL:        /* 13 */
      break;
    case DM_DAMAX:          /* 14 */
      break;
    case DM_DAPROG:         /* 15 */
      break;
    case DM_DELTAX:         /* 16 */
      pMeth->ScanSetup.XMap.Delta[0] = (short)setval;
      break;
    case DM_DELTAY:         /* 17 */
      pMeth->ScanSetup.YMap.Delta[0] = (short)setval;
      break;
    case DM_DETPORT:        /* 18 */
      break;
    case DM_DMODEL:         /* 19 */
      break;
    case DM_DTEMP:          /* 20 */
      pMeth->AuxSetup.Temperature = (short)setval;
      break;
    case DM_DTYPE:
      pMeth->DACSetup.DataType = (dat_type)setval;
      break;
    case DM_DERROR:         /* 21 */
      break;
    case DM_EXPROWS:        /* 22 */
      break;
    case DM_FRAME:          /* 23 */
      break;
    case DM_FREQ:           /* 24 */
      break;
    case DM_H:              /* 25 */
      break;
    case DM_HIMEM:          /* 26 */
      break;
    case DM_I:              /* 27 */
      pMeth->DACSetup.Scans = (short)setval;
      break;
    case DM_ID:             /* 28 */
      break;
    case DM_IMODE:          /* 29 */
      break;
    case DM_INPIA:          /* 30 */
      break;
    case DM_J:              /* 31 */
      pMeth->DACSetup.Mems = (short)setval;
      break;
    case DM_K:              /* 32 */
      break;
    case DM_L:              /* 33 */
      break;
    case DM_MAXMEM:         /* 34 */
      break;
    case DM_MEM:            /* 35 */
      break;
    case DM_MASK:           /* 36 */
      break;
    case DM_OUTPUTREG:      /* 37 */
      break;
    case DM_OUTREG:         /* 38 */
      break;
    case DM_OUTPIA:         /* 39 */
      break;
    case DM_PAUDIO:         /* 40 */
      break;
    case DM_PLSR:           /* 41 */
      break;
    case DM_PNTMODE:        /* 42 */
      break;
    case DM_POINT:          /* 43 */
      break;
    case DM_POINTS:         /* 44 */
      pEntry->Method.ScanSetup.Points = (short)setval;
    break;
    case DM_PRESCAN:        /* 45 */
      break;
    case DM_PTIME:          /* 46 */
      pMeth->AuxSetup.PixTimeIndex = (short)setval;
      break;
    case DM_PTRIG:          /* 47 */
      break;
    case DM_PTRIGNUM:       /* 48 */
      break;
    case DM_PTRIGSRC:       /* 49 */
      break;
    case DM_PTRIGTRSH:      /* 50 */
      break;
    case DM_REGIONS:        /* 51 */
      break;
    case DM_REGSIZE:        /* 52 */
      break;
    case DM_RUN:            /* 53 */
      break;
    case DM_SAMEET:         /* 54 */
      break;
    case DM_SCITC:          /* 55 */
      break;
    case DM_SHFTMODE:       /* 56 */
      break;
    case DM_SHUTMODE:       /* 57 */
      break;
    case DM_SPEED:          /* 58 */
      break;
    case DM_SRTREG:         /* 59 */
      break;
    case DM_STIME:          /* 60 */
      break;
    case DM_STOP:           /* 61 */
      break;
    case DM_STREAKMODE:     /* 62 */
      break;
    case DM_SUBT:           /* 63 */
      break;
    case DM_THERE:          /* 64 */
      break;
    case DM_TRACK:          /* 65 */
      break;
    case DM_TRACKS:         /* 66 */
      pEntry->Method.ScanSetup.Tracks = (short)setval;
    break;
    case DM_TRIGS:          /* 67 */
      break;
    case DM_TRKMODE:        /* 68 */
      break;
    case DM_WFTC:           /* 69 */
      break;
    case DM_WFTO:           /* 70 */
      break;
    case DM_X0:             /* 71 */
      break;
    case DM_Y0:             /* 72 */
      break;
    case DM_ET:
      pMeth->ShutterSetup.ExposeTime = setval;
      break;
    case DM_FTIME:          /*  74 */
      break;
    case DM_MAXET:          /*  75 */
      break;
    case DM_MINET:          /*  76 */
      break;
    case DM_PDELAY:         /*  77 */
      break;
    case DM_PDELINC:        /*  78 */
      break;
    case DM_PDELRANGE:      /*  79 */
      break;
    case DM_PWIDTH:         /*  80 */
      break;
    case DM_PSTIME:         /*  81 */
      break;
    case DM_REGET:          /*  82 */
      break;
    case DM_SCMP:           /*  83 */
      break;
    case DM_STTIME:         /*  84 */
      break;
    case DM_VER:            /*  85 */
      break;
  default:
      return (ERR_DMGR_BADPARAM);
    }
  return (ERR_DMGR_OK);
}


// Calculate nearest multiple of 4 of input
/////////////////////////////////////////////////////////////////////////////
static inline short DWAlign(short Bytes)
{
  return ((Bytes + 3) / 4) * 4;
}

// API function - get data from source and translate into bitmap
//////////////////////////////////////////////////////////////////////////////
BOOL FAR PASCAL _export XlateData2BMP(DHANDLE dHandle, HANDLE *hpBMP, DSIZE dStart, DSIZE dAmount, 
                                      DWORD Offset, float Scale /* char cType */)
{
  if (!GoodHandle(dHandle))
    return(FALSE);

  long BlockSize = (long)((long)dAmount.x_ * (long)dAmount.y_);
  if (IsBadHugeWritePtr(hpBMP, BlockSize))

    return(FALSE);

  LPENTRY pEntry = &Directory.pEntries[dHandle];
  long WordSize = pEntry->Method.DACSetup.DataType & 0x0F; // bytes per data point

  if (!pEntry->pData || IsBadHugeReadPtr(pEntry->pData, BlockSize * WordSize))
    return(FALSE);
    
  CopyBlock((char __far *)hpBMP,            // pointer to destination bitmap
       (ULONG __far *)(pEntry->pData),          // pointer to source data
       dAmount.x_,             // Width of data in pixels
       dAmount.y_,             // Height of data in lines
       DWAlign(dAmount.x_),    // Size of Dest scan line in bytes
       dAmount.x_ * WordSize,  // Size of Src scan line in bytes
       Offset,                 // Brightness adjustment
       Scale /*, cType */);    // Contrast adjustment
  return(TRUE);
}

//  switch (SType)
//    {
//    case STYPE_OMA4DATA:      // if file has data, get pointer to it 
//    case STYPE_OMA4DAD:          
//    case STYPE_OMA3DATA:         
//    case STYPE_DIF:        
//    case STYPE_JCAMP:      
//    case STYPE_OMA4V11DATA:      
//    case STYPE_OMA4TCL32:        
//    case STYPE_OMATIFF8:         
//    case STYPE_OMAM1470A:        
//    case STYPE_OMAM1470B:        
//    case STYPE_OMAM1470C:        
//      pEntry->pData = GetFileDataPointer(retHandle);
//      break;
//    case STYPE_OMA4METHOD:    // Method files don't have data
//    case STYPE_OMA3METHOD:       
//    case STYPE_OMA35METHOD:      
//    case STYPE_OMA4V11METHOD:    
//    default:
//      break;
//    }
