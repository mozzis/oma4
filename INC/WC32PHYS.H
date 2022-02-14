#ifndef WC32PHYS
#define WC32PHYS

#include "eggtype.h"

typedef USHORT selector_t;

void far *GetProtectedPointer(ULONG linear);
void FreeSelector(void far *fPtr);
BOOLEAN InitDPMI(void);

#endif
