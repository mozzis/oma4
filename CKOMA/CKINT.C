#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include "primtype.h"
#include "omaregs.h"
#include "monitor.h"

USHORT OMA_port_addr     = 0x300;         /* addr of OMA4 CR0 */
BOOLEAN quiet = FALSE;

int _cdecl get_args(int argc, char *argv[])
{
  unsigned err = FALSE;
  void * dummy;
  char c;
  
  while (--argc > 0 && ((*++argv)[0] == '-'|| *argv[0] == '/'))
    {
    c = (char)toupper((int)argv[0][1]);
    switch (c)
      {                                  /* add more options as needed */
      case 'H':
      case '?':
        err = TRUE;
      break;
      case 'P':
        if (!(OMA_port_addr = (unsigned)strtoul(&argv[0][2], &dummy, 16)))
          {
          printf("Bad port address: %s\n", &argv[0][2]);
          err = TRUE;
          }
      break;
      case 'Q':
        quiet = TRUE;
      break;
      default:
        printf("ckmsg: illegal option %c\n", argv[0]);
      argc = 0;
      err = TRUE;
      }                                  /* end of switch */
    }
  return(err);
}

volatile OMAControlReg0 Creg0;
volatile OMAControlReg1 Creg1;

int _cdecl main(int argc, char * argv[])
{
  unsigned int test;

  if (!get_args(argc, argv))
    {
    Creg0.byte = (unsigned char)inp(OMA_port_addr);
    Creg0.bits.cooler_irq_enabled  = 1;
    outp(OMA_port_addr, Creg0.byte);

    Creg1.byte = (unsigned char)inp(OMA_port_addr+1);

    Creg1.wbits.host_attn_ack = 1;
    Creg1.wbits.cooler_attn_ack = 1;
    outp(OMA_port_addr+1, Creg1.byte);

    while (!kbhit())
      {
      Creg1.byte = (unsigned char)inp(OMA_port_addr+1);
      if (!quiet)
        printf("%2.2x   \r",Creg1.byte);
      test = (Creg1.rbits.cooler_irq_lo != 0) || (Creg1.rbits.host_attn != 0);
      if (test)
        break;
      }
    return(test);
    }
  else
    {
    printf("CKMSG program - check OMA$ result messages\n"
           "\n"
           "Usage: CKMSG [-p# -a# -l# -q] [\"Message string\"]\n"
           "              -p#       port address\n"
           "              -l#       length of string\n"
           "\n"
           "Message string will be compared with string at memory address,\n"
           "and DOS errorlevel will be set to 1 if the strings do not match\n");
    return(0x7F);
    }
}
