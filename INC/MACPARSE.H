/* -----------------------------------------------------------------------
/
/  macparse.h
/
/  $Header: /OMA4000/Include/MACPARSE.H 1     6/18/99 1:53a Maynard $
 * 
/
/ ----------------------------------------------------------------------- */
  
#ifndef MACPARSE_INCLUDED
#define MACPARSE_INCLUDED

void StartNewParseOperation(void);
void ParseSingleLine(char * ParseText);
void ReadAndParseSourceFile(char * filename);
void InitReservedKeywords(void);
  
#endif  // MACPARSE_INCLUDED
