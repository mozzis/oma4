/*
/ FILE : runforms.h     RAC 6/05/91
/
/            copyright (c) 1991, EG&G Instruments Inc. 
/
/    some of the forms that are used under the "RUN" menu bar option :
/    live to disk, live, accum.
/    run setup form and background form are in their own modules.
/
/
 *    Rev 1.0   13 Jun 1991 09:11:46   cole
 * Initial revision.
/
*/

#ifndef RUNFORMS_INCLUDED
#define RUNFORMS_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern CHAR * GoLiveBackStr;
extern CHAR * GoAccumBackStr;
extern CHAR * GoLiveStr;
extern CHAR * GoAccumStr;
extern CHAR * StopLiveStr;

extern BOOLEAN AccumFormInit(void);
extern BOOLEAN LiveFormInit(void);
extern BOOLEAN AccumBackFormInit(void);
ERR_OMA CheckFilterStatus(void);

/* for use by macros */
BOOLEAN SetFilter(enum FilterModes Mode, char * RefName);

// initialize FormTable[] with LIVE_TO_DISK_FORM, LiveForm, LiveBackForm,
//                             AccumForm, AccumBackForm
void registerRUNFORMS(void);

#endif  //  RUNFORMS_INCLUDED
