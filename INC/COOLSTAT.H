// FILE coolstat.h     RAC   April 17, 1991.

// maintain cooler status on the screen.

#ifndef COOLSTAT_INCLUDED
#define COOLSTAT_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

// execute the next function in the keyboard idle loop chain
unsigned char lowLevelKeyboardIdle(void);

// start cooler status display by adding updateCoolerStat() to the
// keyboard idle chain.  Can only add to the chain once.
void startCoolStat(void);

// disable cooler status checking, updateCoolerStat() will still be
// called by the keyboard idle chain but it will do nothing.
void disableCoolStat(void);

// enable cooler status checking.
void enableCoolStat(void);

// check cooler status and update window display if it has changed or if
// force_display is TRUE.
void displayCoolerStatus(BOOLEAN forceDisplay);

#endif  // COOLSTAT_INCLUDED
