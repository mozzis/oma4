/* -----------------------------------------------------------------------
/
/  macsymbl.h
/
/  $Header: /OMA4000/Include/MACSYMBL.H 1     6/18/99 1:53a Maynard $
/
/ ----------------------------------------------------------------------- */
  
#ifndef MACSYMBL_INCLUDED
#define MACSYMBL_INCLUDED

#include "macruntm.h"  // DECL_INDEX

typedef union {
   int                  Integer;
   long                 LongInteger;
   unsigned int         Word;
   double               Real;
   unsigned char        Byte;
   unsigned int         Boolean;
} SCALARDECL;
  
typedef struct {
   int                  MaxLength;     /* len of zero means string constant */
   char *               Pointer;
} STRINGDECL;
  
typedef struct {
   INDEX                MaxIndex;
   void *               Pointer;
} ARRAYDECL;
  
typedef struct {
   FILE *               FilePtr;
} FILEDECL;
  
typedef struct {
   int                  ParameterCount;
   INDEX                ExecuteListIndex;
   int                  SubIndex;            // 5/25/90 DAI
} PROCEDUREDECL;
  
typedef struct {
   OPERATOR *           Address;
} USER_PROCED_DECL;
  
typedef struct {
   unsigned char     Class;
   unsigned char     Type;
   unsigned char     Attrib;
   char              Name[MAX_IDENT_LEN];
   int               Token;
   union {
      PROCEDUREDECL     Procedure;
      USER_PROCED_DECL  UserProcedure;
      SCALARDECL        Scalar;
      STRINGDECL        String;
      ARRAYDECL         Array;
      FILEDECL          File;
   }                 U;
} DECLARATION;
 
#define VERLEN 4

extern DECLARATION * SymbolTable ;

extern int NumberOfSymbolEntries ;
extern int NumberOfReservedSymbolEntries ;
extern int SavedNumberOfSymbolEntries ;
extern int SymbolTableMaxEntries ;

ERR_OMA ReadMacroSymbolFile( PCHAR MacFileName, PSHORT SymbolCount );
DECL_INDEX LookupSymbol(char * Name, PSHORT pFIndex);
DECL_INDEX ForceCreateSymbol(char * Name);
DECL_INDEX CreateSymbol(char * Name);
INDEX FindIdenticalConstantSymbol(int Class, int Type, void * DataPtr);
INDEX CreateConstantSymbol(int Class, int Type, int Token, void * DataPtr);
char * FindStringInTable(char * TestString);
char * AddStringToTable(char * SourceString, unsigned char MaxLength);
  
#endif  // MACSYMBL_INCLUDED
