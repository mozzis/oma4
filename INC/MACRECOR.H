/* -----------------------------------------------------------------------
/
/  macrecor.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/MACRECOR.H 1     6/18/99 1:53a Maynard $
*/

#ifndef MACRECOR_INCLUDED
#define MACRECOR_INCLUDED
  
#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern BOOLEAN MacRecord;
extern BOOLEAN MacPlayBack;
extern FLOAT PlayBackDelay;
extern char MacRecordFileName[];
extern SHORT KeyStrokeMode;
extern SHORT MostRecentMenuIndex;
extern SHORT MostRecentFormIndex;
extern SHORT MostRecentFieldIndex;

void getPlayName(unsigned int exeListIndex, char Buf[], int formNameSize);

ERR_OMA ReadFormString(SHORT FormIndex, PCHAR Buf);
ERR_OMA ReadFieldString(SHORT FieldIndex, PCHAR Buf,
                                       PCHAR *DisplayString);
ERR_OMA ReadMenuString(SHORT MenuIndex, PCHAR Buf);
void MacRecordString(PCHAR String);
void MacPlayMenu(void);
void MacPlayForm(void);
void MacPlayField(void);
void MacFocusOnField(void);
void LeaveForm(void);
void LeaveMenu(void);
  
BOOLEAN OpenFormRefFiles(void);
void CloseFormRefFiles(void);
int StartKeyStroke(void);
void StopKeyStroke(USHORT Dummy);
SHORT LookupFormFieldMenu(PCHAR Name, PSHORT FldIndex);
SHORT LookupMenu(PCHAR MenuStr, PSHORT FldIndex);
SHORT LookupForm(PCHAR FormStr, PSHORT FldIndex);
SHORT LookupField(PCHAR FieldStr, SHORT FormIndex, PSHORT FldIndex);
  
SHORT RunKeyStrokePlay(void);
  
void ShowLocalInfo(USHORT DUMMY);

// connect the forms system to keystroke recording
// ------------------------------------------------------------------------
void init_keystroke_fields(void);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN isKeyStrokeRecordOn(void);

#endif   // MACRECOR_INCLUDED
