#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>

#include "oma4driv.h"
#include "fabsgraf.h"
#include "fabsdata.h"
#include "fabserrs.h"

#define LABELLEN 15

typedef struct
{
  char label[LABELLEN];
  SHORT tag;
} cmdblock;

static cmdblock commands[] = { {"ACTIVE",    DC_ACTIVE    },
                               {"AUDIO",     DC_PAUDIO    },
                               {"ACTIVEX",   DC_ACTIVEX   },
                               {"ACTIVEY",   DC_ACTIVEY   },
                               {"ADPREC",    DC_ADPREC    },
                               {"ANTIBLOOM", DC_ANTIBLOOM },
                               {"BYTES",     DC_BYTES     },
                               {"CONTROL",   DC_CONTROL   },
                               {"COOLLOCK",  DC_COOLLOCK  },
                               {"COOLONOFF", DC_COOLONOFF },
                               {"COOLSTAT",  DC_COOLSTAT  },
                               {"DAPROG",    DC_DAPROG    },
                               {"DELTAX",    DC_DELTAX    },
                               {"DELTAY",    DC_DELTAY    },
                               {"DETPORT",   DC_DETPORT   },
                               {"DMODEL",    DC_DMODEL    },
                               {"DTEMP",     DC_DTEMP     },
                               {"DERROR",    DC_DERROR    },
                               {"ET",        DC_ET        },
                               {"FREQ",      DC_FREQ      },
                               {"FTIME",     DC_FTIME     },
                               {"HIMEM",     DC_HIMEM     },
                               {"SCANS",     DC_I         },
                               {"ID",        DC_ID        },
                               {"IMODE",     DC_IMODE     },
                               {"J",         DC_J         },
                               {"K",         DC_K         },
                               {"MAXET",     DC_MAXET     },
                               {"MAXMEM",    DC_MAXMEM    },
                               {"MEM",       DC_MEM       },
                               {"MINET",     DC_MINET     },
                               {"SHIFTREG",  DC_OUTPUTREG },
                               {"PDELAY",    DC_PDELAY    },
                               {"PDELINC",   DC_PDELINC   },
                               {"PDELRANGE", DC_PDELRANGE },
                               {"PNTMODE",   DC_PNTMODE   },
                               {"POINT",     DC_POINT     },
                               {"POINTS",    DC_POINTS    },
                               {"PREPS",     DC_H         },
                               {"PWIDTH",    DC_PWIDTH    },
                               {"PLSR",      DC_PLSR      },
                               {"PTRIGNUM",  DC_PTRIGNUM  },
                               {"PTRIGSRC",  DC_PTRIGSRC  },
                               {"RUN",       DC_RUN       },
                               {"SAMEET",    DC_SAMEET    },
                               {"SCITC",     DC_SCITC     },
                               {"SCMP",      DC_SCMP      },
                               {"SHFTMODE",  DC_SHFTMODE  },
                               {"SHUTMODE",  DC_SHUTMODE  },
                               {"SPEED",     DC_SPEED     },
                               {"THERE",     DC_THERE     },
                               {"TRIGS",     DC_TRIGS     },
                               {"TRACK",     DC_TRACK     },
                               {"TRACKS",    DC_TRACKS    },
                               {"TRKMODE",   DC_TRKMODE   },
                               {"VER",       DC_VER       },
                               {"WFTC",      DC_WFTC      },
                               {"WFTO",      DC_WFTO      },
                               {"X0",        DC_X0        },
                               {"Y0",        DC_Y0        },
                               {"",          0            }
                             };

char * dtypes[] = {"No Detector",
            "Thompson 512 X 512",
            "Thompson Dual Channel",
            "EEV CCD",
            "RAPDA",
            "Thompson 1024 X 256",
            "Unknown Device"};

/********************************************/
/* connect to the detector                  */
/********************************************/
int ConnectDetector(USHORT port, ULONG address, ULONG size)
{
  float fparam;
  SHORT i;

  do   /* try to connect to detector and init till giveup or success*/
    {
    setup_detector_interface(port, address, size);

    GetParam(DC_THERE, &fparam); /* is board there? */

    if (!fparam) return ERR_DET_NOT_FOUND;

    GetParam(DC_DMODEL, &fparam);       /* is detector awake */

    if (!fparam)
      {
      at_printf(LN_DETID, "No detector found - try again?");
      i = getch();
      if (!i)
        i = getch();
      }
    }
  while ((i == 'Y' || i == 'y') && !(fparam < 1.0F));

  at_printf(LN_DETID, "Detector is %s",dtypes[(SHORT)fparam]);

  if (fparam < 1.0F)
    return ERR_DET_NO_ID;
  else
    return 0;
}

/*************************************************************************/
/* set the detector settings for acquisition, and allocate memory for    */
/* data and post processing. Data1 and Data2 point to pointers; the      */
/* pointers are not to be initialized or changed by the calling routine  */
/* on return they will point to regions of memory each large enough to   */
/* hold one track.  The same goes for Result, except it will hold floats */
/*************************************************************************/
int SetupDetector(USHORT *Points, ULONG **Data1,
                  ULONG **Data2, float **Result)
{
  float fparam;
  USHORT DataSize;
  int err = ERR_NONE;

  /* set up the defaults */

  GetParam(DC_DERROR, &fparam);
  SetParam(DC_SHUTMODE, 1.0F);   /* force shutter closed */
  SetParam(DC_COOLONOFF, 0.0F);
  SetParam(DC_X0,     1.0F);
  SetParam(DC_Y0,     1.0F);
  SetParam(DC_DELTAX, 1.0F);
  SetParam(DC_DELTAY, 1.0F);
  SetParam(DC_TRACKS, 2.0F);
  GetParam(DC_ACTIVEX, &fparam);
  SetParam(DC_POINTS, fparam);
  SetParam(DC_TRKMODE, 1);
  SetParam(DC_TRACK, 1);
  SetParam(DC_Y0, 100);
  SetParam(DC_DELTAY, 50);
  SetParam(DC_TRACK, 2);
  SetParam(DC_Y0, 300);
  SetParam(DC_DELTAY, 50);
  SetParam(DC_ANTIBLOOM, 0.0F);
  SetParam(DC_WFTO, 0.0F);
  SetParam(DC_ET,     0.01F);
  SetParam(DC_SAMEET, 1.0F);
  SetParam(DC_PTIME, 1.0F);
  SetParam(DC_DAPROG, 1.0F);
  SetParam(DC_I, 1.0F);
  SetParam(DC_J, 1.0F);
  SetParam(DC_L, 0.0F);
  SetParam(DC_SHUTMODE, 2.0F);   /* force shutter open */

  GetParam(DC_DERROR, &fparam);

  GetParam(DC_POINTS, &fparam);
  DataSize = (USHORT)fparam * sizeof(float);
  *Result = malloc(DataSize);
  *Points = (USHORT)fparam;

  if (!*Result)
    {
    err = ERR_NO_MEMORY;
    printf("Out of memory for Result buffer\n");
    }
  else  
    {
    GetParam(DC_BYTES, &fparam);
    DataSize = (USHORT) fparam;

    *Data1 = malloc(DataSize);
    *Data2 = malloc(DataSize);

    if (!*Data1 || !*Data2)
      {
      err = ERR_NO_MEMORY;
    printf("Out of memory for Data buffers\n");
      }
    else
      {
      GetParam(DC_FTIME, &fparam);
      at_printf(LN_FRAMETIME, "Frame time is: %f", fparam);
      }
    }
  return err;
}

/*************************************************************************/
/*************************************************************************/
int DoCustomSetup(char * filename)
{
  FILE *infile;
  int i, j = 0, err = ERR_NONE;
  float fparam;
  char cmdbuf[LABELLEN];

  infile = fopen(filename, "rt");
  if (!infile)
    {
    printf("Can't open %s for input\n", filename);
    err = ERR_NO_FILE;
    }
  else
    {
    while(!feof(infile) && !err)
      {
      /* format same as commands above except no quote marks, etc. */
      if(fscanf(infile,"%s %g\n", cmdbuf, &fparam) == 2)
        {
        j++;
        for (i = 0; commands[i].tag != 0; i++)
          {
          if (!stricmp(cmdbuf, commands[i].label))
            break;
          }
        if (commands[i].tag)
          {
          err = SetParam(commands[i].tag, fparam);
          if (err)
            {
            printf("Error setting %s to %f\n", cmdbuf, fparam);
            err = ERR_DETSETUP;
            break;
            }
          }
        else /* didn't find command in table */
          {
          printf("Bad command in paramfile: %s\n", cmdbuf);
          err = ERR_DETSETUP;
          break;
          }
        }
      else /* scanf error */
        {
        printf("Couldn't scan line %d in paramfile", j+1);
        err = ERR_PARAMLINE;
        break;
        }
      } /* till end of paramfile */
    fclose(infile);
    GetParam(DC_FTIME, &fparam);
    at_printf(LN_FRAMETIME, "Frame time is: %f", fparam);
    }
  if (err)
    {
    printf("Press any key to halt\n");
    getch();
    }
  return err;
}

int StartData(void)
{
  return SetParam(DC_RUN, 1.0F);
}

int StopData(void)
{
  return SetParam(DC_STOP, 0.0F);
}

/* wait for ms milliseconds.  uses the system (BIOS) timer */

int wait_ms(ULONG ms)
{
  unsigned long mtime, newtime;
  struct timeb tBuf;

  ftime(&tBuf);
  mtime = tBuf.time * 1000 + tBuf.millitm;
  at_printf(LN_TRIGWAIT, "Waiting for trigger...");
  do
    {
    ftime(&tBuf);
    newtime = tBuf.time * 1000 + tBuf.millitm;
    }
  while (mtime - newtime < ms);
  return 0;
}

/* Acquire data and convert to absorbance in Result */
/* Return 1 if aborted, 0 if Normal */

int get_Data(ULONG *Track1, ULONG *Track2, short Count, float *Result)
{
  float dummy;
  static float lastscan = 0;
  int i;

  /* wait for either a new scan to complete, or a key to be hit */
  /* delay between reads so DAC isn't locked out of the OMA memory */

  do
    GetParam(DC_L, & dummy);
  while (dummy == lastscan && ! kbhit() && ! wait_ms(10));

  lastscan = dummy;
  at_erase(LN_TRIGWAIT);

  if (kbhit())
    {
    if (!getch()) /* throws away key from kbhit */
      getch();
    return 1;
    }
  else
    {
    ReadCurveFromMem(Track1, Count * sizeof(long), 0);
    ReadCurveFromMem(Track2, Count * sizeof(long), 1);

    /* calculate absorbance.  watch out for zeroes! */

    for (i = 0; i < Count; i++)
      {
      if (Track2[i] == 0.0f)
       Result[i] = 1.0F;
      else
        {
        Result[i] = (float)Track1[i] / (float)Track2[i];
        if (Result[i] != 0.0F)
         Result[i] = (float)log(Result[i]);
        }
      }
    }
  return 0;
}

