#ifdef XSTAT
#define PRIVATE
#else
#define PRIVATE static
#endif

#ifdef _WINOMA_
#include <windows.h>
#endif

#include "oma4driv.h"
#include "cmdtbl.h"
#include "detsetup.h"
#include "driverrs.h"

/* table of commands for use by macro language and others      */
/* each entry is tag, string, set routine, get routine         */

#ifndef __WATCOMC__
PRIVATE int_cmd_entry _far int_cmd_tbl[] =
#else
PRIVATE int_cmd_entry int_cmd_tbl[] =
#endif
{ {DC_ACTIVE,     set_DaActive,          get_DaActive          },
  {DC_ACTIVEX,    set_ActiveX,           get_ActiveX           },
  {DC_ACTIVEY,    set_ActiveY,           get_ActiveY           },
  {DC_ADPREC,     set_PointSize,         get_PointSize         },
  {DC_ANTIBLOOM,  set_AntiBloom,         get_AntiBloom         },
  {DC_BYTES,      set_nothing,           get_Bytes             },
  {DC_CLR,        set_ClearMem,          get_nothing           },
  {DC_CONTROL,    set_ControlMode,       get_ControlMode       },
  {DC_COOLLOCK,   set_nothing,           get_CoolLocked        },
  {DC_COOLONOFF,  set_CoolOnOff,         get_CoolOnOff         },
  {DC_COOLSTAT,   set_nothing,           get_CoolStatus        },
  {DC_COOLTYPE,   set_nothing,           get_CoolerType        },
  {DC_DAMAX,      set_nothing,           get_DaMaxProg         },
  {DC_DAPROG,     set_DaProg,            get_DaProg            },
  {DC_DELTAX,     set_DeltaX,            get_DeltaX            },
  {DC_DELTAY,     set_DeltaY,            get_DeltaY            },
  {DC_DETPORT,    set_DetPort,           get_DetPort           },
  {DC_DMODEL,     set_DetectorType,      get_DetectorType      },
  {DC_DTEMP,      set_Temp,              get_Temp              },
  {DC_DERROR,     set_nothing,           get_Error             },
  {DC_EXPROWS,    set_ExposedRows,       get_ExposedRows       }, 
  {DC_FRAME,      set_nothing,           get_Frame             },
  {DC_FREQ,       set_Freq,              get_Freq              },
  {DC_H,          set_Preps,             get_Preps             },
  {DC_HIMEM,      set_nothing,           get_HiMem             },
  {DC_I,          set_Scans,             get_Scans             },
  {DC_ID,         set_nothing,           get_Ident             },
  {DC_IMODE,      set_IntensifierMode,   get_IntensifierMode   },
  {DC_INPIA,      set_nothing,           get_PIA_In            },
  {DC_J,          set_Mems,              get_Mems              },
  {DC_K,          set_Ignores,           get_Ignores           },
  {DC_L,          set_Lastscan,          get_Lastscan          },
  {DC_MAXMEM,     set_nothing,           get_MaxMem            },
  {DC_MEM,        set_Mem,               get_Mem               },
  {DC_OUTPIA,     set_PIA_Out,           get_PIA_Out           },
  {DC_OUTPUTREG,  set_OutputReg,         get_OutputReg         },
  {DC_OUTREG,     set_OutputReg,         get_OutputReg         },
  {DC_PLSR,       set_PulserType,        get_PulserType        },
  {DC_PAUDIO,     set_PulserAudio,       get_PulserAudio       },
  {DC_PNTMODE,    set_PointMode,         get_PointMode         },
  {DC_POINT,      set_CurrentPoint,      get_CurrentPoint      },
  {DC_POINTS,     set_Points,            get_Points            },
  {DC_PRESCAN,    set_Prescan,           get_Prescan,          },
  {DC_PTIME,      set_PixTime,           get_PixTime           },
  {DC_PTRIGNUM,   set_PulserTrigCount,   get_PulserTrigCount   },
  {DC_PTRIGSRC,   set_PulserTrigSrc,     get_PulserTrigSrc     },
  {DC_PTRIGTRSH,  set_PulserTrigThresh,  get_PulserTrigThresh  },
  {DC_REGIONS,    set_Regions,           get_Regions           },
  {DC_REGSIZE,    set_RegSize,           get_RegSize           },
  {DC_RUN,        start_OMA_DA,          get_DaActive          },
  {DC_SAMEET,     set_SameET,            get_SameET            },
  {DC_SCITC,      set_SourceCompMode,    get_SourceCompMode    },
  {DC_SHFTMODE,   set_ShiftMode,         get_ShiftMode         },
  {DC_SHUTMODE,   set_ShutterMode,       get_ShutterMode       },
  {DC_SPEED,      set_PixTime,           get_PixTime           },
  {DC_SRTREG,     set_nothing,           sort_regions          },
  {DC_STIME,      set_ShiftTime,         get_ShiftTime         },
  {DC_STOP,       stop_OMA_DA,           get_nothing           },
  {DC_STREAKMODE, set_StreakMode,        get_StreakMode        },
  {DC_THERE,      set_RealDetector,      get_RealDetector      },
  {DC_TRIGS,      set_MainTrigger,       get_MainTrigger       },
  {DC_TRACK,      set_CurrentTrack,      get_CurrentTrack      },
  {DC_TRACKS,     set_Tracks,            get_Tracks            },
  {DC_TRKMODE,    set_TrackMode,         get_TrackMode         },
  {DC_WFTC,       set_ShutterCloseSync,  get_ShutterCloseSync  },
  {DC_WFTO,       set_ShutterOpenSync,   get_ShutterOpenSync   },
  {DC_X0,         set_X0,                get_X0                },
  {DC_Y0,         set_Y0,                get_Y0                },
  {0,             0,                     0                     },
};

#ifndef __WATCOMC__
PRIVATE flt_cmd_entry far flt_cmd_tbl[] = {
#else
PRIVATE flt_cmd_entry flt_cmd_tbl[] = {
#endif
  {DC_ET,         set_ExposeTime,        get_ExposeTime        },
  {DC_FTIME,      set_flt_nothing,       get_FrameTime         },
  {DC_MAXET,      set_flt_nothing,       get_MaxET             },
  {DC_MINET,      set_flt_nothing,       get_MinET             },
  {DC_PDELAY,     set_PulserDelay,       get_PulserDelay       },
  {DC_PDELINC,    set_PulserDelayInc,    get_PulserDelayInc    },
  {DC_PDELRANGE,  set_PulserDelayRange,  get_PulserDelayRange  },
  {DC_PWIDTH,     set_PulserWidth,       get_PulserWidth       },
  {DC_PSTIME,     set_flt_nothing,       get_PrescanTime       },
  {DC_REGET,      set_RegionET,          get_RegionET          },
  {DC_SCMP,       set_flt_nothing,       get_SourceComp        },
  {DC_STTIME,     set_flt_nothing,       get_StreakTime        },
  {DC_VER,        set_flt_nothing,       get_Version           },
  {0,             0,                     0                     },
};

#if defined(_DEBUG) && defined(_WINOMA_) && FALSE
typedef struct 
  {
  enum det_command cmd;
  char *name;
  } dbg_cmd;

dbg_cmd dbg_cmds[] = { { DC_ACTIVE,    "DC_ACTIVE"      },
                       { DC_ACTIVEX,   "DC_ACTIVEX"     },
                       { DC_ACTIVEY,   "DC_ACTIVEY"     },
                       { DC_ADD,       "DC_ADD"         },
                       { DC_ADPREC,    "DC_ADPREC"      },
                       { DC_ANTIBLOOM, "DC_ANTIBLOOM"   },
                       { DC_BYTES,     "DC_BYTES"       },
                       { DC_CLR,       "DC_CLR"         },
                       { DC_COOLSTAT,  "DC_COOLSTAT"    },
                       { DC_COOLLOCK,  "DC_COOLLOCK"    },
                       { DC_COOLONOFF, "DC_COOLONOFF"   },
                       { DC_COOLTYPE,  "DC_COOLTYPE"    },
                       { DC_CONTROL,   "DC_CONTROL"     },
                       { DC_DAMAX,     "DC_DAMAX"       },
                       { DC_DAPROG,    "DC_DAPROG"      },
                       { DC_DELTAX,    "DC_DELTAX"      },
                       { DC_DELTAY,    "DC_DELTAY"      },
                       { DC_DETPORT,   "DC_DETPORT"     },
                       { DC_DMODEL,    "DC_DMODEL"      },
                       { DC_DTEMP,     "DC_DTEMP"       },
                       { DC_DERROR,    "DC_DERROR"      },
                       { DC_EXPROWS,   "DC_EXPROWS"     },
                       { DC_FRAME,     "DC_FRAME"       },
                       { DC_FREQ,      "DC_FREQ"        },
                       { DC_H,         "DC_H"           },
                       { DC_HIMEM,     "DC_HIMEM"       },
                       { DC_I,         "DC_I"           },
                       { DC_ID,        "DC_ID"          },
                       { DC_IMODE,     "DC_IMODE"       },
                       { DC_INPIA,     "DC_INPIA"       },
                       { DC_J,         "DC_J"           },
                       { DC_K,         "DC_K"           },
                       { DC_L,         "DC_L"           },
                       { DC_MAXMEM,    "DC_MAXMEM"      },
                       { DC_MEM,       "DC_MEM"         },
                       { DC_MASK,      "DC_MASK"        },
                       { DC_OUTPUTREG, "DC_OUTPUTREG"   },
                       { DC_OUTREG,    "DC_OUTREG"      },
                       { DC_OUTPIA,    "DC_OUTPIA"      },
                       { DC_PAUDIO,    "DC_PAUDIO"      },
                       { DC_PLSR,      "DC_PLSR"        },
                       { DC_PNTMODE,   "DC_PNTMODE"     },
                       { DC_POINT,     "DC_POINT"       },
                       { DC_POINTS,    "DC_POINTS"      },
                       { DC_PRESCAN,   "DC_PRESCAN"     },
                       { DC_PTIME,     "DC_PTIME"       },
                       { DC_PTRIG,     "DC_PTRIG"       },
                       { DC_PTRIGNUM,  "DC_PTRIGNUM"    },
                       { DC_PTRIGSRC,  "DC_PTRIGSRC"    },
                       { DC_PTRIGTRSH, "DC_PTRIGTRSH"   },
                       { DC_REGIONS,   "DC_REGIONS"     },
                       { DC_REGSIZE,   "DC_REGSIZE"     },
                       { DC_RUN,       "DC_RUN"         },
                       { DC_SAMEET,    "DC_SAMEET"      },
                       { DC_SCITC,     "DC_SCITC"       },
                       { DC_SHFTMODE,  "DC_SHFTMODE"    },
                       { DC_SHUTMODE,  "DC_SHUTMODE"    },
                       { DC_SPEED,     "DC_SPEED"       },
                       { DC_SRTREG,    "DC_SRTREG"      },
                       { DC_STIME,     "DC_STIME"       },
                       { DC_STOP,      "DC_STOP"        },
                       { DC_STREAKMODE,"DC_STREAKMODE", },
                       { DC_SUBT,      "DC_SUBT"       },
                       { DC_THERE,     "DC_THERE"      },
                       { DC_TRACK,     "DC_TRACK"      },
                       { DC_TRACKS,    "DC_TRACKS"     },
                       { DC_TRIGS,     "DC_TRIGS"      },
                       { DC_TRKMODE,   "DC_TRKMODE"    },
                       { DC_WFTC,      "DC_WFTC"       },
                       { DC_WFTO,      "DC_WFTO"       },
                       { DC_X0,        "DC_X0"         },
                       { DC_Y0,        "DC_Y0"         },
                       { DC_ET,        "DC_ET"         },
                       { DC_FTIME,     "DC_FTIME"      },
                       { DC_MAXET,     "DC_MAXET"      },
                       { DC_MINET,     "DC_MINET"      },
                       { DC_PDELAY,    "DC_PDELAY"     },
                       { DC_PDELINC,   "DC_PDELINC"    },
                       { DC_PDELRANGE, "DC_PDELRANGE"  },
                       { DC_PWIDTH,    "DC_PWIDTH"     },
                       { DC_PSTIME,    "DC_PSTIME"     },
                       { DC_REGET,     "DC_REGET"      },
                       { DC_SCMP,      "DC_SCMP"       },
                       { DC_STTIME,    "DC_STTIME"     },
                       { DC_VER,       "DC_VER"        },
};

#define DEBUG_GET \
  { \
  char Msg[80]; \
  wsprintf(Msg, " Getting: %s", dbg_cmds[cmd-1].name); \
  MessageBox(NULL, Msg, "Debugging OMA4Win", MB_OK); \
  }
#define DEBUG_SET \
  { \
  char Msg[80]; \
  wsprintf(Msg, " Setting: %s", dbg_cmds[cmd-1].name); \
  MessageBox(NULL, Msg, "Debugging OMA4Win", MB_OK); \
  }
#else
#define DEBUG_GET
#define DEBUG_SET
#endif // #ifdef _DEBUG

/****************************************************************/
/*                                                              */
/* generalized interface to detector set routines               */
/*                                                              */
/****************************************************************/

#ifndef __WATCOMC__
pfgint find_get_cmd(enum int_command cmd)
#else
pfgint find_get_cmd(int cmd)
#endif
{
  SHORT i;

  for (i = 0; (int_cmd_tbl[i].cmd_tag != 0) &&
              (int_cmd_tbl[i].cmd_tag != cmd); i++);

  return (int_cmd_tbl[i].get_routine);
}
              
#ifndef __WATCOMC__
pfgflt find_flt_get_cmd(enum flt_command cmd)
#else
pfgflt find_flt_get_cmd(int cmd)
#endif
{
  SHORT i;

  for (i = 0;(flt_cmd_tbl[i].cmd_tag != 0) &&
             (flt_cmd_tbl[i].cmd_tag != cmd);i++);

  return (flt_cmd_tbl[i].get_routine);
}

#ifndef __WATCOMC__
SHORT get_int_detect_param(enum int_command cmd, SHORT *prm)
#else
SHORT get_int_detect_param(int cmd, SHORT *prm)
#endif
{
  pfgint command = find_get_cmd(cmd);

  if(! command)
    return DRIV_ERROR;

  DEBUG_GET
  return((*command)(prm));
}

#ifndef __WATCOMC__
SHORT get_flt_detect_param(enum flt_command cmd, float *prm)
#else
SHORT get_flt_detect_param(int cmd, float *prm)
#endif
{
  pfgflt command = find_flt_get_cmd(cmd);

  if(! command)
    return DRIV_ERROR;
   
  DEBUG_GET
  return((*command)(prm));
}

#ifndef __WATCOMC__
pfsint find_set_cmd(enum int_command cmd)
#else
pfsint find_set_cmd(int cmd)
#endif
{
  SHORT i;

  for (i = 0; (int_cmd_tbl[i].cmd_tag != 0) &&
              (int_cmd_tbl[i].cmd_tag != cmd); i++);

  return (int_cmd_tbl[i].set_routine);
}
              
#ifndef __WATCOMC__
pfsflt find_flt_set_cmd(enum flt_command cmd)
#else
pfsflt find_flt_set_cmd(int cmd)
#endif
{
  SHORT i;

  for (i = 0; (flt_cmd_tbl[i].cmd_tag != 0) &&
               (flt_cmd_tbl[i].cmd_tag != cmd); i++);

  return (flt_cmd_tbl[i].set_routine);
}
              
#ifndef __WATCOMC__
SHORT set_int_detect_param(enum int_command cmd, SHORT prm)
#else
SHORT set_int_detect_param(int cmd, SHORT prm)
#endif
{
  pfsint command = find_set_cmd(cmd);

  if(! command)
    return DRIV_ERROR;
   
  DEBUG_SET
  return((*command)(prm));
}
  
#ifndef __WATCOMC__
SHORT set_flt_detect_param(enum flt_command cmd, FLOAT prm)
#else
SHORT set_flt_detect_param(int cmd, FLOAT prm)
#endif
{
  pfsflt command = find_flt_set_cmd(cmd);

  if(! command)
    return DRIV_ERROR;
   
  DEBUG_SET
  return((*command)(prm));
}

