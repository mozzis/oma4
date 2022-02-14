/* -----------------------------------------------------------------------
/
/  pltsetup.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/PLTSETUP.H 1     6/18/99 1:53a Maynard $
 * 
 *    Rev 1.0   24 Jul 1991 08:55:28   cole
 * Initial revision.
/
*/

#ifndef PLTSETUP_INCLUDED
#define PLTSETUP_INCLUDED

#ifndef CGIBIND_INCLUDED
   #define CGIBIND_INCLUDED
   #include <cgibind.h>
#endif

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern int autoscale_x;
extern int autoscale_y;
extern int autoscale_z;
extern int window_style;

void InitPlotColors(PLOTBOX * pPlotBox);

// always uses InitialMethod
void InitializePlotSetupFields(CRECT *);

void InitPlotBox(PLOTBOX *);
int  SetFormFromPlotBox(int Dummy);

// scale axes as per axis min and max values in the plot box x,y,z axis data
void scalePlotbox(PLOTBOX * pPlotBox);

// add plot setup form to FormTable[]
void registerPlotSetupForm(void);

#endif   // PLTSETUP_INCLUDED
  

