#include <bios.h>
#include <stdio.h>
#include <conio.h>

#define CMD_PORT 0x70
#define VAL_PORT 0x71

int main(void)
{
  unsigned int err = 0;
  unsigned long memcmos = 0L, membios = 0L;

  outp(CMD_PORT, 0x17);     /* addresses 17 and 18 of the CMOS store */
  memcmos = inp(VAL_PORT);  /* the size of physical extended memory */
  outp(CMD_PORT, 0x18);
  memcmos |= (inp(VAL_PORT) << 8);

  _asm                      /* just to be safe, also ask the BIOS */
    {
    mov ax,0x8800;
    int 0x15;
    mov word ptr membios, ax;
    }

  /* The memory size returned is the number of 1k blocks above 1 Mb */
  /* Need to convert to highest extended memory address */

  memcmos = (memcmos << 10);
  membios = (membios << 10);

  printf("Extended memory (CMOS): %lx\n", memcmos);
  printf("Extended memory (BIOS): %lx\n", membios);
  
  if (membios > memcmos) /* Use highest value to be safe */
    memcmos = membios;

  memcmos = (memcmos + 0x100000) & 0xe00000;

  err = (unsigned)(memcmos / 0x200000);
  if (err > 6) err = 0;

  printf("Address OMA controller at: %lx\n", memcmos);
  
  return((int)err);
}
