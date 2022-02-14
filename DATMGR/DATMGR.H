/* DATMGR.H */
/* API defintions for OMA4 Data Manager */

#ifndef DATMGR_INCLUDED
#define DATMGR_INCLUDED

#include "eggtype.h"

typedef short int DHANDLE;
typedef short int DERR;

enum fparam  { DM_ACTIVEX,        /* 02 */
               DM_ACTIVEY,        /* 03 */
               DM_ADPREC,         /* 05 */
               DM_ANTIBLOOM,      /* 06 */
               DM_BYTES,          /* 07 */
               DM_COOLSTAT,       /* 09 */
               DM_COOLLOCK,       /* 10 */
               DM_COOLONOFF,      /* 11 */
               DM_COOLTYPE,       /* 12 */
               DM_CONTROL,        /* 13 */
               DM_DAMAX,          /* 14 */
               DM_DAPROG,         /* 15 */
               DM_DELTAX,         /* 16 */
               DM_DELTAY,         /* 17 */
               DM_DETPORT,        /* 18 */
               DM_DMODEL,         /* 19 */
               DM_DTYPE,
               DM_DTEMP,          /* 20 */
               DM_DERROR,         /* 21 */
               DM_EXPROWS,        /* 22 */
               DM_FRAME,          /* 23 */
               DM_FREQ,           /* 24 */
               DM_H,              /* 25 */
               DM_HIMEM,          /* 26 */
               DM_I,              /* 27 */
               DM_ID,             /* 28 */
               DM_IMODE,          /* 29 */
               DM_INPIA,          /* 30 */
               DM_J,              /* 31 */
               DM_K,              /* 32 */
               DM_L,              /* 33 */
               DM_MAXMEM,         /* 34 */
               DM_MEM,            /* 35 */
               DM_MASK,           /* 36 */
               DM_OUTPUTREG,      /* 37 */
               DM_OUTREG,         /* 38 */
               DM_OUTPIA,         /* 39 */
               DM_PAUDIO,         /* 40 */
               DM_PLSR,           /* 41 */
               DM_PNTMODE,        /* 42 */
               DM_POINT,          /* 43 */
               DM_POINTS,         /* 44 */
               DM_PRESCAN,        /* 45 */
               DM_PTIME,          /* 46 */
               DM_PTRIG,          /* 47 */
               DM_PTRIGNUM,       /* 48 */
               DM_PTRIGSRC,       /* 49 */
               DM_PTRIGTRSH,      /* 50 */
               DM_REGIONS,        /* 51 */
               DM_REGSIZE,        /* 52 */
               DM_RUN,            /* 53 */
               DM_SAMEET,         /* 54 */
               DM_SCITC,          /* 55 */
               DM_SHFTMODE,       /* 56 */
               DM_SHUTMODE,       /* 57 */
               DM_SPEED,          /* 58 */
               DM_SRTREG,         /* 59 */
               DM_STIME,          /* 60 */
               DM_STOP,           /* 61 */
               DM_STREAKMODE,     /* 62 */
               DM_SUBT,           /* 63 */
               DM_THERE,          /* 64 */
               DM_TRACK,          /* 65 */
               DM_TRACKS,         /* 66 */
               DM_TRIGS,          /* 67 */
               DM_TRKMODE,        /* 68 */
               DM_WFTC,           /* 69 */
               DM_WFTO,           /* 70 */
               DM_X0,             /* 71 */
               DM_Y0,             /* 72 */
               DM_INT_CMDS,      /* 73 */
               DM_ET = DM_INT_CMDS,/* 73 */
               DM_FTIME,          /*  74 */
               DM_MAXET,          /*  75 */
               DM_MINET,          /*  76 */
               DM_PDELAY,         /*  77 */
               DM_PDELINC,        /*  78 */
               DM_PDELRANGE,      /*  79 */
               DM_PWIDTH,         /*  80 */
               DM_PSTIME,         /*  81 */
               DM_REGET,          /*  82 */
               DM_SCMP,           /*  83 */
               DM_STTIME,         /*  84 */
               DM_VER,            /*  85 */
               DM_NUM_CMDS        /*  86 */
         };

enum dm_strings { DMS_NAME = 0 };

enum hndlerrs { INVALID_HANDLE = -1,   // return from handle allocate functions
                INVALID_NOMEM = -2, 
                INVALID_NOFILE = -3, 
                INVALID_FILETYPE = -4,
                INVALID_FILEREAD = -5 };

enum dmgr_errs { ERR_DMGR_OK = 0,
                 ERR_DMGR_BADHANDLE,
                 ERR_DMGR_NOMEM,
                 ERR_DMGR_BADPARAM };

typedef struct dsize_struct {
   SHORT x_;
   SHORT y_;
} DSIZE;

typedef void __huge * VPH;

/* API definitions */
#ifdef __cplusplus
extern "C" {
#endif
BOOL FAR PASCAL _export GetSourceIsLive(DHANDLE dHandle);
DERR FAR PASCAL _export GetHeaderParam(DHANDLE, enum fparam, float far *);
DERR FAR PASCAL _export SetHeaderParam(DHANDLE, enum fparam, float);
DERR FAR PASCAL _export GetIntHeaderParam(DHANDLE, enum fparam, short far *);
DERR FAR PASCAL _export SetIntHeaderParam(DHANDLE, enum fparam, short);
DERR FAR PASCAL _export GetHeaderString(DHANDLE, enum dmstring, char far *);
DERR FAR PASCAL _export SetHeaderString(DHANDLE, enum dmstring, char far *);
DERR FAR PASCAL _export ReadDataCurve(DHANDLE dHandle, void FAR *pData, SHORT bytes, USHORT curve);
DERR FAR PASCAL _export WriteDataCurve(DHANDLE dHandle, void FAR *pData, SHORT bytes, USHORT curve);
DHANDLE FAR PASCAL _export CreateDataSource(LPCSTR lpName, LPCSTR lpDesc);
DHANDLE FAR PASCAL _export OpenDataSource(LPCSTR lpName);
DHANDLE FAR PASCAL _export NewDataSource(LPCSTR lpName);
BOOL FAR PASCAL _export InsertData(DHANDLE, void __huge *, DWORD);
BOOL FAR PASCAL _export SaveDataSource(DHANDLE);
BOOL FAR PASCAL _export RenameDataSource(DHANDLE, LPCSTR);
DERR FAR PASCAL _export CloseDataSource(DHANDLE dHandle);
DERR FAR PASCAL _export ReSaveDataSource(DHANDLE dHandle, LPCSTR lpName);
void __huge * PASCAL _export GetFileDataPointer(DHANDLE dHandle);
#ifdef __cplusplus
  }
#endif

#endif
