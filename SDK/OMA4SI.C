#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>

#include "oma4driv.h"
#include "driverrs.h"

static ULONG Data[1024];

typedef struct
{
  char label[15];
  SHORT tag;
} cmd_block;

static cmd_block commands[] = {
  {"ACTIVE",     DC_ACTIVE,    },
  {"ACTIVEX",    DC_ACTIVEX    },
  {"ACTIVEY",    DC_ACTIVEY    },
  {"ADPREC",     DC_ADPREC     },
  {"ANTIBLOOM",  DC_ANTIBLOOM  },
  {"BYTES",      DC_BYTES      },
  {"CLR",        DC_CLR        },
  {"CONTROL",    DC_CONTROL    },
  {"COOLLOCK",   DC_COOLLOCK   },
  {"COOLONOFF",  DC_COOLONOFF  },
  {"COOLSTAT",   DC_COOLSTAT   },
  {"COOLTYPE",   DC_COOLTYPE   },
  {"DAMAX",      DC_DAMAX      },
  {"DAPROG",     DC_DAPROG     },
  {"DELTAX",     DC_DELTAX     },
  {"DELTAY",     DC_DELTAY     },
  {"DETPORT",    DC_DETPORT    },
  {"DMODEL",     DC_DMODEL     },
  {"DTEMP",      DC_DTEMP      },
  {"DERROR",     DC_DERROR     },
  {"EXPROWS",    DC_EXPROWS    },           
  {"FRAME",      DC_FRAME      },
  {"FREQ",       DC_FREQ       },
  {"H",          DC_H          },
  {"HIMEM",      DC_HIMEM      },
  {"I",          DC_I          },
  {"ID",         DC_ID         },
  {"IMODE",      DC_IMODE      },
  {"INPIA",      DC_INPIA      },
  {"J",          DC_J          },
  {"K",          DC_K          },
  {"L",          DC_L          },
  {"MAXMEM",     DC_MAXMEM     },
  {"MEM",        DC_MEM        },
  {"OUTPIA",     DC_OUTPIA     },
  {"OUTPUTREG",  DC_OUTPUTREG  },
  {"OUTREG",     DC_OUTREG     },
  {"PLSR",       DC_PLSR       },
  {"PAUDIO",     DC_PAUDIO     },
  {"PNTMODE",    DC_PNTMODE    },
  {"POINT",      DC_POINT      },
  {"POINTS",     DC_POINTS     },
  {"PRESCAN",    DC_PRESCAN    },
  {"PTIME",      DC_PTIME      },
  {"PTRIGNUM",   DC_PTRIGNUM   },
  {"PTRIGSRC",   DC_PTRIGSRC   },
  {"PTRIGTRSH",  DC_PTRIGTRSH  },
  {"REGIONS",    DC_REGIONS    },
  {"REGSIZE",    DC_REGSIZE    },
  {"RUN",        DC_RUN        },
  {"SAMEET",     DC_SAMEET     },
  {"SCITC",      DC_SCITC      },
  {"SHFTMODE",   DC_SHFTMODE   },
  {"SHUTMODE",   DC_SHUTMODE   },
  {"SPEED",      DC_SPEED      },
  {"SRTREG",     DC_SRTREG     },
  {"STIME",      DC_STIME      },
  {"STOP",       DC_STOP       },
  {"STREAKMODE", DC_STREAKMODE },
  {"THERE",      DC_THERE      },
  {"TRIGS",      DC_TRIGS      },
  {"TRACK",      DC_TRACK      },
  {"TRACKS",     DC_TRACKS     },
  {"TRKMODE",    DC_TRKMODE    },
  {"WFTC",       DC_WFTC       },
  {"WFTO",       DC_WFTO       },
  {"X0",         DC_X0         },
  {"Y0",         DC_Y0         },
  {"ET",         DC_ET         },
  {"FTIME",      DC_FTIME      },
  {"MAXET",      DC_MAXET      },
  {"MINET",      DC_MINET      },
  {"PDELAY",     DC_PDELAY     },
  {"PDELINC",    DC_PDELINC    },
  {"PDELRANGE",  DC_PDELRANGE  },
  {"PWIDTH",     DC_PWIDTH     },
  {"PSTIME",     DC_PSTIME     },
  {"REGET",      DC_REGET      },
  {"SCMP",       DC_SCMP       },
  {"STTIME",     DC_STTIME     },
  {"VER",        DC_VER        },
  {"\0",         0          },
};

void list_commands(void)
{
  int i;

  for (i = 0;commands[i].label[0] != '\0'; i++)
    {
    printf("%s\n",commands[i].label);
    if (!((i+1) % 20))
      {
      fprintf(stderr, "Press any key\n");
      fflush(stderr);
      getch();
      }
    }
}

void prt_err(int err)
{
  char * tptr;

  switch (err)
    {
    default:
    case NO_ERROR:
      tptr="NO ERROR"; break;
    case DRIV_ERROR:
      tptr="DRIV ERROR"; break;
    case ERROR_FAKEDETECTOR:
      tptr="FAKEDETECTOR ERROR"; break;
    case ERROR_NOSUCH_PARAM:
      tptr="NOSUCH PARAM ERROR"; break;
    case ERROR_MONITOR_VER:
      tptr="MONITOR VER ERROR"; break;
    case ERROR_DA_ACTIVE:
      tptr="DA ACTIVE ERROR"; break;
    case ERROR_BAD_STRUCTURE:
      tptr="BAD STRUCTURE ERROR"; break;
    case ERROR_TOO_LONG:
      tptr="TOO LONG ERROR"; break;
    case ERROR_NO_SERIAL_CODE:
      tptr="NO SERIAL_CODE ERROR"; break;
    case ERROR_NO_CODE_SPACE:
      tptr="NO CODE_SPACE ERROR"; break;
    case ERROR_NO_CLEAN_CODE:
      tptr="NO CLEAN_CODE ERROR"; break;
    case ERROR_DETECTOR_TIMEOUT:
      tptr="DETECTOR TIMEOUT ERROR"; break;
    case ERROR_POINT_NUMBER:
      tptr="POINT NUMBER ERROR"; break;
    case ERROR_TRACK_NUMBER:
      tptr="TRACK NUMBER ERROR"; break;
    case ERROR_EXPOSE_TIME:
      tptr="EXPOSE TIME ERROR"; break;
    case ERROR_NOSUCH_DAPROG:
      tptr="NOSUCH DAPROG ERROR"; break;
    case ERROR_TOO_MANY_MEMORIES:
      tptr="TOO MANY_MEMORIES ERROR"; break;
    case ERROR_NO_LIVE_DATA:
      tptr="NO LIVE_DATA ERROR"; break;
    case ERROR_SCAN_MISMATCH:
      tptr="SCAN MISMATCH ERROR"; break;
    case ERROR_ILLEGAL_RANGE:
      tptr="ILLEGAL RANGE ERROR"; break;
    case ERROR_VALUE_INCORRECT:
      tptr="VALUE INCORRECT ERROR"; break;
    case ERROR_ADJUSTED_TRACK:
      tptr="ADJUSTED TRACK ERROR"; break;
    case ERROR_ADJUSTED_POINT:
      tptr="ADJUSTED POINT ERROR"; break;
    case ERROR_MONITOR_LOAD:
      tptr="MONITOR LOAD ERROR"; break;
    case OVERLAP_ERR:
      tptr="OVERLAP ERR"; break;
    case SIZE_ERR:
      tptr="SIZE ERR"; break;
    case OUT_OF_ARRAY_ERR:
      tptr="OUT OF_ARRAY_ERR"; break;
    case UNSCANNED_PXL_ERR:
      tptr="UNSCANNED PXL_ERR"; break;
    case SHORT_ET_ERR:
      tptr="SHORT ET_ERR"; break;
    case ET_ERR:
      tptr="ET ERR"; break;
    case ET_RANGE_ERR:
      tptr="ET RANGE_ERR"; break;
    case ARRAY_SIZE_ERR:
      tptr="ARRAY SIZE_ERR"; break;
    }
  fprintf(stderr, "%s\n", tptr);
  fflush(stderr);

}

int main(int argc, char ** argv)
{
  SHORT i, pnum, err = 0;
  USHORT port = 0x300;
  float param, dummy;
  char user[80];
  char parm[20];
  ULONG address = 0x200000L;
  char * endptr;

  if (endptr = getenv("OMAADDR"))
    address = strtoul(endptr, NULL, 16);
  if (endptr = getenv("OMAPORT"))
    port = (unsigned)strtoul(endptr, NULL, 16);

  for (i = 1;i < argc; i++)
    {
    if (argv[i][0] == 'A' || argv[i][0] == 'a')
      address = strtoul(&(argv[i][1]), &endptr, 16);
    else if (argv[i][0] == 'P' || argv[i][0] == 'p')
      port = (unsigned)strtoul(&(argv[i][1]), &endptr, 16);
    }
  printf("Address %lx, port %x\n",address,port);
  err = setup_detector_interface(port, address, 0x200000L);
  prt_err(err);

  SetParam(DC_DAPROG, (float)1);
  do
    {
    pnum = 0;
    printf("\nEnter a Command: ");
    fflush( stdin );
    gets(user);
    endptr = strtok(user," ");
    if (endptr != NULL)
      {
      pnum = 1;
      strcpy(parm, endptr);
      endptr = strtok(NULL," ");
      if (endptr != NULL)
        {
        dummy = (float)strtod(endptr, &endptr);
        pnum = 2;
        }
      }
    if (!strcmpi(parm,"?"))
      {
      list_commands();
      }
    else if (!strcmpi(parm,"DUMP"))
      {
      char * bum;
      SHORT count, i;
      endptr = strtok(NULL, " ");
      if (endptr != NULL)
        {
        count = (unsigned)strtoul(endptr, &bum, 10);
        printf("Dump %d points of curve %f\n", count, dummy);
        GetParam(DC_POINTS, &param);
        ReadCurveFromMem(Data, (SHORT)param * 4, (SHORT)dummy);
        for (i = 0;i < count; i++)
          {
          printf("%d : %lu\n", i, Data[i]);
          }
        }
      }
    else if (!strcmpi(parm,"READSYS"))
      {
      SHORT i;
      float param;

      for (i = 0;commands[i].label[0] != '\0'; i++)
        {
        GetParam(commands[i].tag, &param);
        printf("%s = %f\n",commands[i].label, param);
        if (!((i+1) % 20))
          {
          printf("Press any key\n");
          getch();
          }
        }
      }
    else if (pnum)
      {
      for (i = 0;commands[i].label[0] != '\0'; i++)
        {
        if (! strcmpi(user, commands[i].label))
          {
          if (pnum == 2)
            err = SetParam(commands[i].tag, dummy);
          else
            {
            err = GetParam(commands[i].tag, &dummy);
            printf("%s = %f\n",commands[i].label, dummy);
            }
          break;
          }
        }
      if (commands[i].label[0] == 0)  /* If 0, commands didn't have string. */
        printf ("Illegal Command: %s\n", user); /* Sound a warning... */
      else if (err)
        {
        GetParam(DC_DERROR, &dummy);
        err = (int)dummy;
        printf("Error: %d ", err);
        prt_err(err);
        err = 0;
        }
      }
    }
  while (pnum);
  return(0);
}

