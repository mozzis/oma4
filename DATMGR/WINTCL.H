// wintcl.h
// define functions for DATMGR DLL to read Hidris format data files

#ifndef WINTCL_INCLUDED
#define WINTCL_INCLUDED

#include "datmgr.h"
#include "datadir.h"

BOOL IsFileTCL(FILE * fhnd);
BOOL ReadTCLHeader(FILE * fhnd, LPENTRY pEntry);
BOOL ReadTCLData(FILE * fhnd, LPENTRY pEntry, DWORD szOffset, DWORD szData);
BOOL WriteTCLHeader(FILE * fhnd, LPENTRY pEntry);
BOOL WriteTCLData(FILE * fhnd, LPENTRY pEntry, DWORD szOffset, DWORD szData);

#endif
