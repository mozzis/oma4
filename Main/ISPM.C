#include <dos.h>
#include <stdio.h>
#include "dos16.h"

extern int isD16M(void);
extern int _is_pm(void);

void main(void)
{
  printf("80286 is in %s mode\n", _is_pm() ? "protected" : "real");
  printf("DOS16M %s running\n", isD16M() ? "is" : "is not");

}





