#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <time.h>
#include <sys/timeb.h>

#include "fabsgraf.h"
#include "fabsdata.h"

#ifdef __BORLANDC__
int _stklen = 0x8000;
#endif

char * CustomFile = NULL;

/******************************************************/
/* get the OMA4 board params from the environment or  */
/* from the command line - command line predominates  */
/******************************************************/

void get_params(SHORT argc, char ** argv, ULONG *address, ULONG *size,
                                          USHORT *port)
{
  char * endptr;
  SHORT i;
  
  endptr = getenv("OMAADDR");
  if (endptr)
	  *address = strtoul(endptr, NULL, 16);
  endptr = getenv("OMAPORT");
  if (endptr)
	  *port = (unsigned)strtoul(endptr, NULL, 16);
  endptr = getenv("OMASIZE");
  if (endptr)
	  *size = strtoul(endptr, NULL, 16);

  for (i = 1;i < argc; i++)
	  {
    if (argv[i][0] == '-')
      {
	    if (toupper(argv[i][1]) == 'A')
	      *address = strtoul(&(argv[i][1]), &endptr, 16);
	    else if (toupper(argv[i][1]) == 'P')
	      *port = (unsigned)strtoul(&(argv[i][1]), &endptr, 16);
	    else if (toupper(argv[i][1]) == 'S')
	      *size = strtoul(&(argv[i][1]), &endptr, 16);
	    else if (toupper(argv[i][1]) == 'F')
        CustomFile = &argv[i][2];
      else
        printf("Bad command line argument: %s\n", &argv[i][1]);
      }
	  }
}

#ifdef __WATCOMC__
char * _strtime(char * tstr)
{
  time_t now = time(NULL);
  struct tm *nptr = localtime(&now);
  strftime(tstr, 9, "%H:%M:%S", nptr);
  return tstr;
}
#endif

int main(int argc, char ** argv)
{
  ULONG *Data1, *Data2, address=0x800000, size=0x200000;
  float *Result;
  USHORT Points, errorcode, port=0x300;
  LONG sctime, newtime, itime, sccount;
  struct timeb tBuf;
  char tstr[12];

  get_params(argc, argv, &address, &size, &port);

  if (InitGraf())
    return(1);

  /* make sure detector is present and can be identified */

  if (ConnectDetector(port, address, size))
    {
    DeInitGraf();
    return(1);
    }

  /* set up the detector for acquisition; includes allocation of */
  /* memory for data and calculation                             */

  if (SetupDetector(&Points, &Data1, &Data2, &Result) ||
      InitPlot(Points))    /* also scale and draw plot */
    {
    DeInitGraf();
    printf("No detector found!\n");
    return 1;
    }

  if (CustomFile)
    {
    if (DoCustomSetup(CustomFile))
      {
      DeInitGraf();
      return(1);
      }
    }

  at_printf(LN_PRESS_KEY, "Press any key to start acquistion.");
  if (!getch())
    getch();

  if (StartData())
    {
    errorcode = 2;
    }
  else
    {
    at_printf(LN_PRESS_KEY, "Press any key to abort acquistion.");

    itime = 1;               /* print status every itime seconds */
    ftime(&tBuf);            /* get the start time, save in sctime */
    sctime = tBuf.time;
    sccount = 0L;            /* initialize count of cycles per itime */

    at_printf(LN_TIMER+1, "Start Time: %s", _strtime(tstr));
    do
      {
      sccount++;             /* count acqcuire/plot cycles */
      errorcode = get_Data(Data1, Data2, Points, Result);

      if (!errorcode)
        errorcode = plot_Data(Result, Points);

      ftime (&tBuf);          /* get updated time */
      newtime = tBuf.time;

      if (newtime - sctime >= itime)
        {
        at_printf(LN_TIMER, "%s  Cycles: %ld", _strtime(tstr), sccount);
        sctime = newtime;
        sccount = 0L;
        }
      }
    while (!errorcode);       /* errorcode set on problem or keypress */
  
    StopData();
    }

  /* closes down the graphics system */
  DeInitGraf();
  if (errorcode == 2)
    printf("Error starting detector! (cycle %ld)\n", sccount);
  return errorcode;
}
