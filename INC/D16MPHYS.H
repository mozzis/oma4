// FILE : mapphys.h

#ifndef MAPPHYS_INCLUDED
#define MAPPHYS_INCLUDED

#ifndef EGGTYPE_INCLUDED
#include "eggtype.h"
#endif

/* MapPhysical -- given a physical region (physical address and
  a byte length), return a linear address that the program can use
  to access the region, after any necessary preparation.

  Ex: map special memory board 8MB for 64KB

    la = MapPhysical(0x800000, 0x10000, 0);
    if (! la)  error;
    ptr = D16SegAbsolute(la, 0);

  Limitations 3.94:
    Does not work with DPMI.
    D16SegAbsolute and some other library functions will not
      correctly handle linear addresses >16MB.  Large linear
      addresses are only likely to happen if D16MapPhysical
      is called several times in a program under VCPI.
    To make the code simpler, the physical region may not be
      larger than 4KB less than 4MB.
  Limitations 4.0:
    Works OK with DPMI now.          (mlm)
    Works for up to 8Mb block size.  (mlm)
    Always assume non-cacheable.     (mlm)
*/

unsigned long MapPhysical(
    unsigned long physical,   /* hardware address */
    unsigned long bytes);     /* number of bytes to map */

void _huge * GetProtectedPointer(ULONG address);

#endif  //  MAPPHYS_INCLUDED
