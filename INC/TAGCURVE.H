// tagcurve.h               RAC Oct 4, 1990.
//
//  Copyright (c) 1990,  EG&G Instruments Inc.                             */
/*
/  $Header: /OMA4000/Include/TAGCURVE.H 1     6/18/99 1:53a Maynard $
*/

// functions for tagging, untagging, and drawing tagged curves.

#ifndef TAGCURVE_INCLUDED
#define TAGCURVE_INCLUDED

#ifndef CGIBIND_INCLUDED
   #define CGIBIND_INCLUDED
   #include <cgibind.h>
#endif

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern BOOLEAN ExpandedOnTagged;

void HotTagCurve   (USHORT Dummy);
void HotUntagCurve (USHORT Dummy);
void HotUntagAll   (USHORT Dummy);
void HotAddTagGroup(USHORT Dummy);

// redraw all the tagged curves in the current active window using a
// special tagged curve color.
void drawTagged(USHORT dummy);

// Return an appropriate color for drawing tagged curves, given the current
// curve drawing color.
CCOLOR taggedColor(CCOLOR currentColor);

void ExpandTaggedToggle(USHORT Dummy);

// init FormTable[] with TagCurveGroupForm, SaveTaggedForm
void registerTagcurveForms(void);

void MacExpandTagged(void);
void MacTagCurve(void);
void MacUntagCurve(void);
void MacRestoreFromExpand(void);

#endif  // TAGCURVE_INCLUDED
