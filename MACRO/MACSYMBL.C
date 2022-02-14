/* -----------------------------------------------------------------------
/
/  macsymbl.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/macro/macsymbl.c_v   0.14   09 Jan 1992 16:48:08   cole  $
/  $Log:   J:/logfiles/oma4000/macro/macsymbl.c_v  $
 * 
 *    Rev 0.14   09 Jan 1992 16:48:08   cole
 * Change #include'
 * 
 *    Rev 0.13   14 Oct 1991 11:35:16   cole
 * Add macro_error() call in AddStringToTable().
 * 
 *    Rev 0.12   09 Sep 1991 13:46:56   cole
 * Add include filestuf.h
 * 
 *    Rev 0.11   25 Jul 1991 08:34:00   cole
 * Added $Header and $Log comment lines.
/
*/

#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>
  
#include "macsymbl.h"
#include "di_util.h"
#include "macnres.h"
#include "macrecor.h"
#include "syserror.h" // ERROR_OPEN
#include "omaerror.h"
#include "filestuf.h" // fidMacroOMA4
#include "crvheadr.h"
#include "handy.h"    // FTIDLEN
  
#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif
  
DECLARATION * SymbolTable;      /* contains 'symbol table' w/variables */
  
int NumberOfSymbolEntries = 0;
int NumberOfReservedSymbolEntries = 0;
int SavedNumberOfSymbolEntries = -1;
int SymbolTableMaxEntries = DEFAULT_MAX_SYMBOLS;
 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA ReadMacroSymbolFile(PCHAR MacFileName, PSHORT SymbolCount)
{
  FILE *hFile;
  CHAR FileIDStr[FTIDLEN];
  CHAR VersionStr[VERLEN];

  hFile = fopen(MacFileName, "rb");
  if (hFile == NULL)
    {
    error(ERROR_OPEN, MacFileName);
    return ERROR_OPEN;
    }

  if (fread(FileIDStr, 1, FTIDLEN, hFile) != FTIDLEN)
    {
    error(ERROR_READ, MacFileName);
    fclose(hFile);
    return ERROR_OPEN;
    }

  if (strcmp(fidMacroOMA4, FileIDStr))
    {
    error(ERROR_IMPROPER_FILETYPE, MacFileName);
    fclose(hFile);
    return ERROR_OPEN;
    }

  if (fread(VersionStr, 1, VERLEN, hFile) != VERLEN)
    {
    error(ERROR_READ, MacFileName);
    fclose(hFile);
    return ERROR_OPEN;
    }

  if (fread(SymbolCount, sizeof(SHORT), 1, hFile) != 1)
    {
    error(ERROR_READ, MacFileName);
    fclose(hFile);
    return ERROR_OPEN;
    }

  if (fread(SymbolTable, sizeof(DECLARATION), *SymbolCount, hFile) !=
    (USHORT) *SymbolCount)
    {
    error(ERROR_READ, MacFileName);
    fclose(hFile);
    return ERROR_OPEN;
    }

  fclose(hFile);
  return ERROR_NONE;
}

// -----------------------------------------------------------------------
DECL_INDEX LookupSymbol(char * Name, PSHORT pFIndex)
{
  DECL_INDEX i;
  
  /* slow, brute force method; could be speeded up */
  for (i = 0; i < NumberOfSymbolEntries; i++)
    {
    if (strncmp(SymbolTable[i].Name, Name, MAX_IDENT_LEN) == 0)
      return(i);
    }
  i= -1;
 
  if (pFIndex != NULL)
    i = LookupFormFieldMenu(Name, pFIndex);
 
  return(i);
}
  
// -----------------------------------------------------------------------
DECL_INDEX ForceCreateSymbol(char * Name)
{
  DECL_INDEX SymbolIndex;

  if (NumberOfSymbolEntries < SymbolTableMaxEntries)
    {
    SymbolIndex = NumberOfSymbolEntries++;
    erase(&SymbolTable[SymbolIndex], DECLARATION, 1);
    strcpy(SymbolTable[SymbolIndex].Name, Name);
    }
  else
    macro_error(ERROR_SYMTAB_FULL, Name);

  return(SymbolIndex);
}
  
// -----------------------------------------------------------------------
DECL_INDEX CreateSymbol(char * Name)
{
  DECL_INDEX SymbolIndex;
  SHORT Dummy;

  if (LookupSymbol(Name, &Dummy) != INVALID_INDEX)
    macro_error(ERROR_SYMBOL_REDEFINED, Name);
  else
    {
    SymbolIndex = ForceCreateSymbol(Name);
    }
  return(SymbolIndex);
}
  
// -----------------------------------------------------------------------
PRIVATE BOOLEAN CompareByType(void * First, void * Second, int Type)
{
  if (Type == TYPE_BOOLEAN)
    Type = TYPE_WORD;
  switch (Type)
    {
    case TYPE_INTEGER:
    case TYPE_WORD:
      return(*((unsigned int *) First) == *((unsigned int *) Second));
    case TYPE_LONG_INTEGER:
      return(*((unsigned long *) First) == *((unsigned long *) Second));
    case TYPE_BYTE:
      return(*((UCHAR *) First) == *((UCHAR *) Second));
    case TYPE_REAL4:
      return(*((FLOAT *) First) == *((FLOAT *) Second));
    case TYPE_REAL:
      return(*((double *) First) == *((double *) Second));
  
    case TYPE_STRING:
      return(strcmp(((char *) First), ((char *) Second)) == 0);
  
    default:
      return(FALSE);
   }
}
  
// -----------------------------------------------------------------------
PRIVATE void StoreByType(void * Destination, void * Source, int Type)
{
  if (Type == TYPE_BOOLEAN)
    Type = TYPE_WORD;

  switch (Type)
    {
    case TYPE_INTEGER:
    case TYPE_WORD:
      *((unsigned int *) Destination) = *((unsigned int *) Source);
    break;
    case TYPE_LONG_INTEGER:
      *((unsigned long *) Destination) = *((unsigned long *) Source);
    break;
    case TYPE_BYTE:
      *((UCHAR *) Destination) = *((UCHAR *) Source);
    break;
    case TYPE_REAL4:
      *((FLOAT *) Destination) = *((FLOAT *) Source);
    break;
    case TYPE_REAL:
      *((double *) Destination) = *((double *) Source);
    break;
   }
}
  
// -----------------------------------------------------------------------
INDEX FindIdenticalConstantSymbol(int Class, int Type, void * DataPtr)
{
  INDEX i;
  void *SymbolDataPtr;

  /* for (i=0; i < NumberOfSymbolEntries; i++) */
  /* there won't be any useful constants in the reserved symbols */
  for (i=((USHORT) NumberOfReservedSymbolEntries+1);
       i < (USHORT) NumberOfSymbolEntries; i++)
    {
    if ((SymbolTable[i].Class == (UCHAR) Class) &&
        (SymbolTable[i].Type == (UCHAR) Type) &&
        (SymbolTable[i].Name[0] == NIL))
      {
      if (Class == CLASS_SCALAR)
        SymbolDataPtr = (void *) &SymbolTable[i].U.Scalar;
      else if (Class == CLASS_STRING)
        SymbolDataPtr = (void *) SymbolTable[i].U.String.Pointer;
      else
        return(INVALID_INDEX);

      if (CompareByType(SymbolDataPtr, DataPtr, Type))
        return(i);
      }
    }
  return(INVALID_INDEX);
}
  
// -----------------------------------------------------------------------
INDEX CreateConstantSymbol(int Class, int Type, int Token, void * DataPtr)
{
  INDEX SymbolIndex;

  if (Class != CLASS_SCALAR && Class != CLASS_STRING)
    return(INVALID_INDEX);

  SymbolIndex = FindIdenticalConstantSymbol(Class, Type, DataPtr);

  if (SymbolIndex == (INDEX)INVALID_INDEX)
    {
    SymbolIndex = ForceCreateSymbol("");
    if (SymbolIndex != (INDEX)INVALID_INDEX)
      {
      SymbolTable[SymbolIndex].Class = (UCHAR) Class;
      SymbolTable[SymbolIndex].Type = (UCHAR) Type;
      SymbolTable[SymbolIndex].Token = Token;
      set_bit(SymbolTable[SymbolIndex].Attrib, SYMATTR_CONSTANT);

      if (Class == CLASS_SCALAR)
        StoreByType(&SymbolTable[SymbolIndex].U.Scalar, DataPtr, Type);
      else if (Class == CLASS_STRING)
        SymbolTable[SymbolIndex].U.String.Pointer = (char *)DataPtr;
      }
    }
  return(SymbolIndex);
}
  
// -----------------------------------------------------------------------
char * FindStringInTable(char * TestString)
{
  INDEX i;
  
  /* add 2 bytes for len byte and null byte */  
  for (i = 0;i <= StringTableFreeOffset; i += (INDEX)(2 + StringTable[i]))
    {
    if (strcmp(&StringTable[i+1], TestString) == 0)
      {
      return(&StringTable[i+1]);
      }
    }
  return(NULL);
}
  
// -----------------------------------------------------------------------
char * AddStringToTable(char * SourceString, UCHAR MaxLength)
{
  char * StringPtr;

  if ((StringTableFreeOffset + ((INDEX) MaxLength + 2)) < StringTableSize)
    {
    ((UCHAR *) StringTable)[StringTableFreeOffset++] = MaxLength;
    StringPtr = &StringTable[StringTableFreeOffset];
    if (MaxLength > 0)
      strncpy(StringPtr, SourceString, (int) MaxLength);
    else
      *StringPtr = NIL;
    StringTableFreeOffset += (INDEX)(MaxLength + 1); /* add space for '\0' */
    StringTable[StringTableFreeOffset] = 0;
    }
  else
    {
    StringPtr = NULL;
    macro_error(ERROR_TOO_MANY_STRINGS, "");
    }
  return(StringPtr);
}

