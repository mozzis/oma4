
#ifndef WINPHYS_INCLUDED
#define WINPHYS_INCLUDED

#include "eggtype.h"
typedef USHORT selector_t;
void __huge * GetProtectedPointer(ULONG linear);
ULONG MapPhysical(ULONG physical, ULONG limit);

void ReleaseDescriptors(void);

#endif
