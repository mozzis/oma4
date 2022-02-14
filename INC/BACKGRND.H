/* -----------------------------------------------------------------------
/
/  backgrnd.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/BACKGRND.H 1     6/18/99 1:52a Maynard $
/  $Log: /OMA4000/Include/BACKGRND.H $
 * 
 * 1     6/18/99 1:52a Maynard
*/

#ifndef BACKGRND_INCLUDED
#define BACKGRND_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern char * BackGroundEntryName ;

extern char * BackGroundDesc ;

ERR_OMA CopyToBackGround(CHAR * FileName, USHORT Startindex,
                                USHORT Count);

void registerBackgroundForm(void);

#endif  // BACKGRND_INCLUDED
