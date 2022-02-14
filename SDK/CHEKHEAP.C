#include <stdio.h>
#include <alloc.h>

int chekheap(char * Message)
{
int heapstat;

#ifdef __TURBOC__
  static struct farheapinfo hi;
#else
  static struct _heapinfo hi;
  #define size _size
  #define ptr _pentry
  #define in_use _useflag
#endif

  if (Message)
    printf("\n%s\n", Message);

  printf("Address    Size        Status\n"
         "-------    ----        ------\n");
  heapstat = heapcheck();

  if (heapstat == _HEAPOK)
    {
    hi.ptr = NULL;
    while (farheapwalk(&hi) == _HEAPOK)
      {
      printf("%lp  %7.7lu     %s", hi.ptr, hi.size, hi.in_use ? "in use" : "free");
      printf("     \n");
      }
    }
  else
    {
    printf("Bad heap status: %d\n", heapstat);
    heapstat = _HEAPCORRUPT;
    }

#ifdef __TURBOC__
  printf("Memory in near heap: %u\n", coreleft());
  printf("Memory in far heap: %lu\n", farcoreleft());
#else
  printf("Memory in near heap: %u\n", _nmsize());
  printf("Memory in far heap: %lu\n", _fmsize());
#endif

  return heapstat;
}

