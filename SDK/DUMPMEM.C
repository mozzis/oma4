#include <stdio.h>
#include <stdlib.h>
#include "himem.h"

char buffer[16];

void get_params( int argc, char ** argv, unsigned long *address,
                                         unsigned long *size)
{
  char * endptr;
  int i;
  
  for (i = 1;i < argc; i++)
	{
	if (argv[i][0] == 'A' || argv[i][0] == 'a')
	  *address = strtoul(&(argv[i][1]), &endptr, 16);
	else if (argv[i][0] == 'S' || argv[i][0] == 's')
	  *size = strtoul(&(argv[i][1]), &endptr, 16);
	}

}

int main(int argc, char **argv)
{
  int i, j, chunks;
  unsigned long address = 0x200000L;
  unsigned long size = 0x000200L;
  unsigned long tmpaddr;

  get_params( argc, argv, &address, &size);

  chunks = (int) (size / (unsigned long)(sizeof(buffer)));

  for (i = 0;i < chunks; i++)
    {
    tmpaddr = address +  ((unsigned long)i * (unsigned long)sizeof(buffer));
    read_high_memory(buffer, tmpaddr, sizeof(buffer));
    printf("\n%-6.6lx: ", tmpaddr);

    for (j = 0; j < sizeof(buffer); j += 2)
      {
      printf("%-4.4x ", ((int far *)(buffer))[j/2]);
      }
    }
  return(0);
}
