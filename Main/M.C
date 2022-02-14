#include <stdio.h>
#include <conio.h>

int main(int argc, char ** argv)
{
  char ch;

  if (argc < 2)
    return(0);

  printf("%s",argv[1]);

  while (!kbhit());

  ch = (char)getche();

  if (ch > 'a' && ch < 'z' )
    ch -= ('a' - 'A');
  ch -= ('A' - 1);

  printf("\n");

  return((int)ch);
}
