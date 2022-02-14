#include <stdio.h>
#include <stdlib.h>
#ifdef USE_D16M
#include <dos16.h>
#include "mapphys.h"
#else
#include "himem.h"
#endif

void useage(void)
{
  printf("\nEMPOKE stores 16 bit number at a physical address"
         "\nUseage:"
         "\nEMPOKE <32 bit hex address> <16 bit hex value>\n");
}

int main(int argc, char far ** argv)
{
  unsigned long paddress;
  unsigned int value;
  #ifdef USE_D16M
  unsigned long address;
  unsigned far * pvalue;
  #endif

  if (argc < 3)
    {
    useage();
    return(1);
    }

  paddress = strtoul(argv[1], NULL, 16);
  value = (unsigned) strtoul(argv[2], NULL, 16);

  #ifdef USE_D16M
  if (address = D16MapPhysical(paddress, 0x200000, 1))
    paddress = address;
  #endif

  printf("Store %x at %lx", value, paddress);

  #ifdef USE_D16M

  pvalue = D16SegAbsolute(paddress, 0);
  *pvalue = value;
  D16SegCancel(pvalue);

  #else
  write_high_memory((unsigned int far *)&value, paddress, sizeof(value));
  #endif
}

