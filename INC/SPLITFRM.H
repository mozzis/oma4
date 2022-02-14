/***************************************************************************/
/* splitfrm.h                                                              */
/* standard method for using split form/graph screen                       */
/* Morris Maynard July 1993 Copyright (C) 1993 EG&G Instruments            */
/***************************************************************************/

#ifndef SPLITFRM_INCLUDED
#define SPLITFRM_INCLUDED

#ifndef EGGTYPE_INCLUDED
#include "eggtype.h"
#endif

#ifndef FORMTYPE_INCLUDED
#include "formtype.h"
#endif

BOOLEAN InitSplitForm(FORM * Form, BOOLEAN * InForm, int (*DoOnSwitch)(void));
SHORT SwitchToGraphMode(void);
BOOLEAN ExitSplitForm(BOOLEAN * InForm);
BOOLEAN InSplitForm(void);

#endif
