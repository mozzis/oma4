/* $Header: /OMA4000/Include/FKEYFUNC.H 1     6/18/99 1:52a Maynard $ */
  
/* -----------------------------------------------------------------------
/
/  fkeyfunc.h
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: DAI      Version 1.00         21 Nov         1989
/
/  fkeyfunc.h prototypes functions for handling function key actions.
/
*/ /*
/ ----------------------------------------------------------------------- */
  
#ifndef FKEYFUNC_INCLUDED
#define FKEYFUNC_INCLUDED

#ifndef EGGTYPE_INCLUDED
#include "eggtype.h"
#endif

#include "barmenu.h"
  
extern MENUITEM far FKeyItems[];
extern MENU FKey;

extern USHORT CurrentShiftMode;

extern char * LiveCaptureStr;
extern char * GraphModeStr;

UCHAR (*UserKeyHandler)(UCHAR key);
  
SHORT ShowFKeys(MENU *);
UCHAR HandleFKey(UCHAR);
UCHAR HandleAltHotKey(UCHAR Key);
UCHAR HandleSpecialHotKey(UCHAR Key);
UCHAR OMAKey(UCHAR);
SHORT MenuModeToggle(USHORT Dummy);
UCHAR ShiftCheck(void);
void HotExit(USHORT Dummy);
void GetHelp(USHORT Dummy);
void GetPopCurveDir(USHORT Dummy);
  
UCHAR IsCursorInFKeys(SHORT Row, SHORT Column);

void reinit_FKeys(void);  // set up initial values for FKeyItems[1..9] and
                           // redisplay the function keys on screen

#endif  // FKEYFUNC_INCLUDED
