#include <stdio.h>
#include <dos16.h>
#include "mapphys.h"

void main(void)
{
  unsigned long phys_address = 0x810000;
  unsigned long map_address = 0x810000;
  FILE * outfile;
  char far * outptr;

  if (!(map_address = D16MapPhysical( phys_address, 0xFFFF, 1 )))
    map_address = phys_address;

  outptr = D16SegAbsolute(map_address,0);

  outfile = fopen("monitor.bin","w+b");

  fwrite(outptr, 1, 0x8000, outfile);
}
