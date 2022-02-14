#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

#include "oma4driv.h"

#define LABELLEN 15

ULONG *Data;
enum yesno {NO = 0, YES };
char prmfilename[] = "testdriv.prm";

typedef struct
{
  char label[LABELLEN];
  SHORT tag;
  FLOAT value;
  enum yesno Canset;
} cmdblock;

static cmdblock commands[] = { {"ACTIVE",    DC_ACTIVE,   1.00F,    NO  },
                               {"AUDIO",     DC_PAUDIO,   1.00F,    YES },
                               {"ACTIVEX",   DC_ACTIVEX,  512.00F,  NO  },
                               {"ACTIVEY",   DC_ACTIVEY,  512.00F,  NO  },
                               {"ADPREC",    DC_ADPREC,   1.00F,    NO  },
                               {"ANTIBLOOM", DC_ANTIBLOOM,50.0F,    YES },
                               {"BYTES",     DC_BYTES,    2048.00F, NO  },
                               {"CONTROL",   DC_CONTROL,  0.00F,    NO  },
                               {"COOLLOCK",  DC_COOLLOCK, 1.00F,    NO  },
                               {"COOLONOFF", DC_COOLONOFF,1.00F,    YES },
                               {"COOLSTAT",  DC_COOLSTAT, 1.00F,    NO  },
                               {"DAPROG",    DC_DAPROG,   1.00F,    YES },
                               {"DELTAX",    DC_DELTAX,   1.00F,    YES },
                               {"DELTAY",    DC_DELTAY,   1.00F,    YES },
                               {"DETPORT",   DC_DETPORT,  0x300F,   NO  },
                               {"DMODEL",    DC_DMODEL,   1.00F,    NO  },
                               {"DTEMP",     DC_DTEMP,    -20.0F,   YES },
                               {"DERROR",    DC_DERROR,   0.00F,    NO  },
                               {"ET",        DC_ET,       0.10F,    YES },
                               {"FREQ",      DC_FREQ,     60.0F,    YES },
                               {"FTIME",     DC_FTIME,    0.00F,    NO  },
                               {"HIMEM",     DC_HIMEM,    0.00F,    NO  },
                               {"SCANS",     DC_I,        4.00F,    YES },
                               {"ID",        DC_ID,       1560.0F,  NO  },
                               {"IMODE",     DC_IMODE,    1.0F,     YES },
                               {"J",         DC_J,        1.00F,    YES },
                               {"K",         DC_K,        0.00F,    YES },
                               {"MAXET",     DC_MAXET,    0.00F,    NO  },
                               {"MAXMEM",    DC_MAXMEM,   1.00F,    NO  },
                               {"MEM",       DC_MEM,      0.00F,    YES },
                               {"MINET",     DC_MINET,    0.01F,    NO  },
                               {"SHIFTREG",  DC_OUTPUTREG,0.00F,    YES },
                               {"PDELAY",    DC_PDELAY,   0.00F,    YES },
                               {"PDELINC",   DC_PDELINC,  0.00F,    YES },
                               {"PDELRANGE", DC_PDELRANGE,0.00F,    YES },
                               {"PNTMODE",   DC_PNTMODE,  0.00F,    YES },
                               {"POINT",     DC_POINT,    1.00F,    NO  },
                               {"POINTS",    DC_POINTS,   512.0F,   YES },
                               {"PREPS",     DC_H,        0.00F,    YES },
                               {"PWIDTH",    DC_PWIDTH,   0.010F,   YES },
                               {"PLSR",      DC_PLSR,     2.0F,     NO  },
                               {"PTRIGNUM",  DC_PTRIGNUM, 1.0F,     YES },
                               {"PTRIGSRC",  DC_PTRIGSRC, 2.0F,     YES },
                               {"RUN",       DC_RUN,      0.00F,    NO  },
                               {"SAMEET",    DC_SAMEET,   0.00F,    NO  },
                               {"SCITC",     DC_SCITC,    4.00F,    YES },
                               {"SCMP",      DC_SCMP,     0.00F,    NO  },
                               {"SHFTMODE",  DC_SHFTMODE, 0.00F,    YES },
                               {"SHUTMODE",  DC_SHUTMODE, 0.00F,    YES },
                               {"SPEED",     DC_SPEED,    0.00F,    YES },
                               {"THERE",     DC_THERE,    1.00F,    NO  },
                               {"TRIGS",     DC_TRIGS,    0.00F,    NO  },
                               {"TRACK",     DC_TRACK,    1.00F,    NO  },
                               {"TRACKS",    DC_TRACKS,   1.00F,    YES },
                               {"TRKMODE",   DC_TRKMODE,  0.00F,    YES },
                               {"VER",       DC_VER,      1.00F,    NO  },
                               {"WFTC",      DC_WFTC,     0.00F,    YES },
                               {"WFTO",      DC_WFTO,     0.00F,    YES },
                               {"X0",        DC_X0,       1.00F,    YES },
                               {"Y0",        DC_Y0,       128.0F,   YES },
                               {"",          0,        0.0F,     NO  }
                             };

char * dtypes[] = {"No Detector",
                 "Thompson 512 X 512",
                 "Thompson Dual Channel",
                 "EEV 1024 X 256",
                 "Reticon RAPDA",
                 "Thompson 1024 X 256",
                 "Thompson 1024 X 1024",
                 "Unknown Device" };

/* show the detector's version of the current settings */

void display_detector(void)
{
  USHORT i;
  FLOAT dummy;

  for (i = 0; commands[i].label[0] != '\0'; i++)
    {
    if (GetParam(commands[i].tag, &dummy))
      {
      fprintf(stderr, "Error while getting %s: TAG %x VAL %f\n",
                       commands[i].tag, dummy);
      fflush(stderr); /* else WATCOM won't print! */
      }
    else
      printf("Get %s = %f\n", commands[i].label, dummy);
    }
}

/* send all the current settings to the detector */                                  
void program_detector(void)
{
  USHORT i;

  for (i = 0; commands[i].label[0] != '\0'; i++)
    {
    if (commands[i].Canset == YES)
      {
      if (SetParam(commands[i].tag, commands[i].value))
        {
        FLOAT error;
        GetParam(DC_DERROR, &error);
        fprintf(stderr, "Error# %f while setting %s\n", error,
                                               commands[i].label);
        fflush(stderr);
        }
      printf("Set %s = %f\n", commands[i].label, commands[i].value);
      }
    }
}
                                  
void write_pram_file(void) /* write current settings to a file */
{
  USHORT i;
  FILE * outfile;

  if ((outfile = fopen(prmfilename, "w+t")) == NULL)
    {
    fprintf(stderr, "\nError opening prm file for output");
    fflush(stderr); /* else WATCOM won't print! */
    }
  else
    {
    printf("\nWriting output file %s", prmfilename);
    for (i = 0;commands[i].label[0] != '\0'; i++)
      {
      fprintf(outfile,"%-10.10s %-6.6g %s\n",
              commands[i].label,
              commands[i].value,
              commands[i].Canset ? "YES" : "NO ");
      }
    fclose(outfile);
    }
}

void read_pram_file(void)  /* read params from file if one exists */
{
  USHORT i, j = 0;
  static char testflg[] = "YES";
  static cmdblock test_block;
  FILE * infile;
  SHORT found, error = 0;

  if ((infile = fopen(prmfilename, "r+t")) == NULL)
    {
    fprintf(stderr, "\nError opening %s for input", prmfilename);
    fprintf(stderr, "\nInternal defaults will be used");
    fflush(stderr); /* else WATCOM won't print! */
    }
  else
    {
    printf("\nreading parameters from %s", prmfilename);
    while (! feof(infile) && (! error)) /* just keep trying to end of file */
      {
      if(fscanf(infile,"%s %g %s\n", /* format same as commands above */
                test_block.label,    /* except no quote marks, etc. */
                &(test_block.value),
                testflg) == 3)
        {
        j++, found = 0;
        for (i = 0;(!found) && (commands[i].label[0] != '\0'); i++)
          {
          if(!strnicmp(commands[i].label, test_block.label, LABELLEN))
            {
            found = 1, i--;
            }
          }
        if (found)
          {
          strncpy(commands[i].label, test_block.label, LABELLEN);
          commands[i].value = test_block.value;
          commands[i].Canset = !strnicmp(testflg, "Y", 1);
          }
        else
          error = 1;
        }
      else
        error = 1;
      }
    if (error)
      {
      fprintf(stderr, "\nCouldn't parse line %d containing %s",
                       j, test_block.label);
      fflush(stderr); /* else WATCOM won't print! */
      }
    fclose(infile);
    }
}
                                  
/********************************************/
/* wait for a keypress                      */
/********************************************/
void interval(void)
{
  fprintf(stderr, "************ Press any key ****************\n");
  fflush(stderr);
  getch();
}

/******************************************************/
/* get the OMA4 board params from the environment or  */
/* from the command line - command line predominates  */
/******************************************************/

void get_params(SHORT argc, char ** argv, ULONG *address,
                                          ULONG *size,
                                          USHORT * port)
{
  char * endptr, *dummy;
  SHORT i;
  
  if (endptr = getenv("OMAADDR"))
    *address = strtoul(endptr, &dummy, 16);

  if (endptr = getenv("OMAPORT"))
    *port = (USHORT)strtoul(endptr, &dummy, 16);

  if (endptr = getenv("OMASIZE"))
    *size = strtoul(endptr, &dummy, 16);


  for (i = 1;i < argc; i++)
	  {
	  if (argv[i][0] == 'A' || argv[i][0] == 'a')
	    *address = strtoul(&argv[i][1], &dummy, 16);
	  else if (argv[i][0] == 'P' || argv[i][0] == 'p')
	    *port = (USHORT)strtoul(&argv[i][1], &dummy, 16);
	  else if (argv[i][0] == 'S' || argv[i][0] == 's')
	    *size = strtoul(&argv[i][1], &dummy, 16);
	  }
}

/********************************************/
/* connect to the detector                  */
/********************************************/
SHORT connect_detector(USHORT port, ULONG address, ULONG size)
{
  FLOAT real_detector, dmodel;
  SHORT i;

  do   /* try to connect to detector and init till giveup or success*/
    {
    setup_detector_interface(port, address, size);

    GetParam(DC_THERE, &real_detector); /* is board there? */
    GetParam(DC_DMODEL, &dmodel);       /* is detector awake */

    if (!real_detector || !dmodel)
      {
      fprintf(stderr, "\nNo detector found - try again?");
      fflush(stderr);
      i =  getch();
      }
    }
  while ((i == 'Y' || i == 'y') && (!real_detector || !dmodel));

  fprintf(stderr, "\nDetector is %s\n",dtypes[(SHORT)dmodel]);
  fflush(stderr);

  if (dmodel < 1.0F) return 1; else return 0;
}

/********************************************/
/* wait for data acquisition to be complete */
/********************************************/
void WaitActive(void)
{
  FLOAT active, i;

  do
    {
    GetParam(DC_ACTIVE, &active);
    GetParam(DC_I, &i);
    printf("%g *** %2.0f\r", active, i);
    }
  while (active);
}

/****************************************************/
/* print out data from the start and end of a curve */
/****************************************************/
void print_data(SHORT start_val, SHORT end_val, SHORT gap, char * message)
{
  SHORT i;
  printf("\n%s\n", message);

  for (i = start_val;i < end_val; i++)
    printf(" Data[%d]: \t %ld \t Data[%d]: \t %ld\n",
                   i, Data[i], (512-gap)+i, Data[(512-gap)+i]);
}

/*******************************************************************/
SHORT main(SHORT argc, char ** argv)
{
  FLOAT tracks, flength;
  USHORT port = 0x300;
  ULONG address = 0x800000L;
  ULONG size = 0x200000L;
  ULONG temp;
  USHORT length;

  read_pram_file();

  get_params(argc, argv, &address, &size, &port);

  printf("\naddress %lx size %lx port %x\n", address, size, port);

  if(connect_detector(port, address, size))
    {
    return (10);
    }

  program_detector();
 
  GetParam(DC_BYTES, &flength);

  length = (USHORT)flength;

  if (length == 0)
    {
    fprintf(stderr, "\nDetector reported 0 length\n");
    fflush(stderr);
    return (11);
    }

  Data = malloc(length);

  if (!Data)
    {
    fprintf(stderr, "\nNot enough memory!\n");
    fflush(stderr);
    return 1;
    }

  interval();

  write_pram_file();

  display_detector();

  interval();

  SetParam(DC_RUN, 0.0F);

  WaitActive();
  
  ReadCurveFromMem(Data, length, 0);

  print_data(0, 24, 24, "Start & End of first memory:");

  interval();

  ReadCurveFromMem(Data, length, 1);

  print_data(0, 24, 24, "Start & End of second memory:");

  interval();

  GetParam(DC_TRACKS, &tracks);
  ReadCurveFromMem(Data, length, (SHORT)tracks * 7);

  print_data(0, 24, 24, "Data before mem 8 acquisition:");

  SetParam(DC_MEM, 8.0F);
  SetParam(DC_RUN, 0.0F);

  WaitActive();

  ReadCurveFromMem(Data, length, 0);

  print_data(0, 24, 24, "Data after mem 8 acquisition:");

  ReadCurveFromMem(Data, length, 3); /* get track 4 */

  temp = Data[100];

  printf("Point 100 from track 4: %ld\n", temp);

  SetParam(DC_RUN, 0.0F);

  WaitActive();
  
  SetParam(DC_ET, 0.2F);

  ReadCurveFromMem(Data, length, 3); /* get track 4 */

  temp = Data[100];

  printf("Point 100 from track 4: %ld\n", temp);

  SetParam(DC_RUN, 0.0F);

  WaitActive();

  SetParam(DC_ET, 0.1F);
                        
  ReadCurveFromMem(Data, length, 3); /* get track 4 */

  temp = Data[100];

  printf("Point 100 from track 4: %ld\n", temp);

  return(0);

}
