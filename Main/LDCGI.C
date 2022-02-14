#include <stdio.h>
#include <malloc.h>
#include <conio.h>
#include <cgi_def.h>


static int screen_setup[] = {
                              0, /* coordinate transform mode   */
                              1, /* initial line type           */
                              2, /* initial line color          */
                              3, /* initial marker type         */
                              1, /* initial marker color        */
                              1, /* initial graphics text font  */
                              1, /* initial graphics text color */
                              0, /* initial fill interior style */
                              0, /* initial fill style index    */
                              3, /* initial fill color index    */
                              1, /* prompting flag              */
                              'D','I','S','P','L','A','Y',' '
                             };

int screen_handle;
int screen[66];

int main( int argc, char *argv[])
{
  int i, cgi_loaded = 0;

  long required = 0L;
  long avail = 0L;
  unsigned char far * where = NULL;

  required = cgi_load(where, avail);
  if (required != -1)
    {
    printf("Need %ld bytes for CGI\n",required);
    i = getch();
    where = malloc((unsigned)required);
    if (where)
      {
      avail = cgi_load (where, required);
      if (avail != -1) cgi_loaded = 1;
      }
    }
  if (cgi_loaded)
    {
    screen[11] = 1;

    i = v_opnwk( screen_setup, &screen_handle, screen );

    if( i != -1 )
        v_clrwk( screen_handle ) ;
    }
  else
    printf("GSS Error %d\n", vq_error());
  return 0;
}
