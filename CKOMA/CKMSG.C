#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "primtype.h"
#ifdef USE_D16M
#include "mapphys.h"
#else
#include "himem.h"
#endif
#include "cache386.h"
#include "omaregs.h"
#include "monitor.h"

ULONG  OMA_phys_addr     = 0xC00000L;     /* OMA4 board physical address */
ULONG  OMA_memory_addr   = 0xC00000L;     /* 386 translated address */
USHORT OMA_port_addr     = 0x300;         /* addr of OMA4 CR0 */
USHORT string_len = 40;                   /* length of string to match */
char   message[81];
BOOLEAN quiet = FALSE, compare = TRUE;

#ifdef USE_D16M
char far * string_addr;
#else
char string_addr[81];
#endif


void access_startup_detector()
{
#ifdef USE_D16M
  ULONG tlong;  /* check if VCPI or DPMI present */
                
  if (tlong = D16MapPhysical(OMA_phys_addr, 0x200000, 1))
    OMA_memory_addr = tlong;
  else
#endif
    OMA_memory_addr = OMA_phys_addr;
}

void get_OMA4_string(char * string_addr, int len)
{
  cacheoff();
#ifndef USE_D16M
  read_high_memory(string_addr, OMA_memory_addr, len);
#else
  string_addr = D16SegAbsolute(OMA_memory_addr, 0);
#endif
  cacheon();
}

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
      case 'A':
        if (!(OMA_phys_addr = strtoul(&argv[0][2], &dummy, 16)))
          {
          printf("Bad address: %s\n", &argv[0][2]);
          err = TRUE;
          }
      break;
      case 'H':
      case '?':
        err = TRUE;
      break;
      case 'L':
        if (!(string_len = (unsigned)strtoul(&argv[0][2], &dummy, 10)))
          {
          printf("Bad string length: %s\n", &argv[0][2]);
          err = TRUE;
          }
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
    if (argc > 0 && !err)
      strcpy(message, *argv++);          /* at least 1 str on command line */
    else
      compare = FALSE;

  return(err);
}


int _cdecl main(int argc, char * argv[])
{
  if (argc > 1 && !get_args(argc, argv))
    {
    access_startup_detector();

    get_OMA4_string(string_addr, string_len);

    if (!quiet)
      printf("%s\n", string_addr);

    if (compare)
      return(strnicmp(message, string_addr, string_len));
    else
      return(0);
    }
  else
    {
    printf("CKMSG program - check OMA$ result messages\n"
           "\n"
           "Usage: CKMSG [-p# -a# -l# -q] [\"Message string\"]\n"
           "              -p#       port address\n"
           "              -a#       memory address\n"
           "              -l#       length of string\n"
           "              -q        don't print string from board\n"
           "\n"
           "Message string will be compared with string at memory address,\n"
           "and DOS errorlevel will be set if the strings do not match.\n"
           "If no string provided, no comparison is done\n");
    return(0x7F);
    }
}
