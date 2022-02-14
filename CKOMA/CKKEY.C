#include <stdio.h>
#include <conio.h>

int main(int argc, char ** argv)
{
  char ch = 0;

  if (argc == 2)
    printf("%s", argv[1]);

  if(kbhit())
    {
    ch = (char)getch();

    if (ch > 'a' && ch < 'z' ) /* will uppercase letters in a-z range */
      ch -= ('a' - 'A');
    }

  return((int)ch);
}
