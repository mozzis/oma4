/**********************************************************************/
/*                                                                    */
/*       scanset.c                                                    */
/*       written by MLM 1/22/90                                       */
/*       forms and routines for scan setup of 1461                    */
/*       used by oma4000 software                                     */
/*                                                                    */
/*  Copyright (c) 1990,  EG&G Instruments Inc.                        */
/*
/  $Header:   J:/logfiles/oma4000/main/scanset.c_v   0.28   07 Jul 1992 17:11:02   maynard  $
/  $Log:   J:/logfiles/oma4000/main/scanset.c_v  $
*/
/**********************************************************************/

#include <stdlib.h> /* strtoul */

#include "rapset.h"
#include "omaform.h"
#include "oma4driv.h"
#include "detsetup.h"
#include "helpindx.h"
#include "ksindex.h"
#include "formtabs.h"
#include "omaerror.h"
#include "driverrs.h"
#include "forms.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

// private functions ------------------------------------------------------

PRIVATE BOOLEAN RapdaFormInit(void);
PRIVATE BOOLEAN RapdaFormExit(void);
PRIVATE BOOLEAN RegionFormInit(void);
PRIVATE BOOLEAN RegionFormExit(void);
PRIVATE BOOLEAN ChekRegionIdx(int index);

PRIVATE int form_set_Regions(void);
PRIVATE int form_set_CurrentRegion(void);
PRIVATE int form_X0_set(void);
PRIVATE int form_set_EndChnl(void);
PRIVATE int form_set_RegET(void);
PRIVATE int form_set_BaseET(void);
PRIVATE int form_set_ArrSize(void);

// ------------------------------------------------------------------------

PRIVATE int   form_Regions;
PRIVATE int   form_Tracks;
PRIVATE int   form_CurrRegion;
PRIVATE int   form_X0;
PRIVATE int   form_EndChnl;
PRIVATE float form_RegET;
PRIVATE float form_BaseET;
PRIVATE int   form_ActiveX;
PRIVATE int   form_ActiveY;
PRIVATE float frame_time;

static enum { DGROUP_DO_STRINGS=1, DGROUP_FORMS, DGROUP_CODE, DGROUP_RAPDA};

PRIVATE DATA STRINGS_Reg[] = {
   /* 0 */  { "#Regions:",       0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 1 */  { "#Tracks:",        0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 2 */  { "Region #",        0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 3 */  { "Start Chnl.",     0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 4 */  { "End Chnl.",       0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 5 */  { "Region ET",       0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 6 */  { "Base ET",         0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 7  */ { "Array Size",      0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 8  */ { "X",               0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 9  */ { "Frame Time",      0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 10 */ { "sec.",            0, DATATYP_STRING, DATAATTR_PTR, 0 },
} ;

PRIVATE FORM RapdaForm, 
             RegionScrollForm,
             RegionBoxForm;
               
PRIVATE DATA RAPFORMS_Reg[] = {
   { &RapdaForm,        0, DATATYP_VOID, DATAATTR_PTR, 0 },
   { &RegionScrollForm, 0, DATATYP_VOID, DATAATTR_PTR, 0 },
   { &RegionBoxForm,    0, DATATYP_VOID, DATAATTR_PTR, 0 },
};

PRIVATE DATA RapSet_Reg[] = {

/* 0 */   { &form_Regions,   0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 1 */   { &form_Tracks,    0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 2 */   { &form_CurrRegion,0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 3 */   { &form_X0,        0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 4 */   { &form_EndChnl,   0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 5 */   { &form_RegET,     0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
/* 6 */   { &form_BaseET,    0, DATATYP_FLOAT, DATAATTR_PTR, 0 },
/* 7 */   { &form_ActiveX,   0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 8 */   { &form_ActiveY,   0, DATATYP_INT, DATAATTR_PTR, 0 },
/* 9 */   { &frame_time,     0, DATATYP_FLOAT, DATAATTR_PTR, 0 },        
};

EXEC_DATA Action_Registry[] = {
 /*0 */ {CAST_CHR2INT scroll_entry_field,      0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*1 */ {CAST_CHR2INT scroll_up_field,         0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*2 */ {CAST_CHR2INT scroll_down_field,       0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*3 */ {CAST_CHR2INT ChekRegionIdx,           0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*4 */ {form_set_Regions,        0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*5 */ {form_set_CurrentRegion,  0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*6 */ {form_X0_set,             0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*7 */ {form_set_EndChnl,        0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*8 */ {form_set_RegET,          0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*9 */ {form_set_BaseET,         0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*10*/ {form_set_ArrSize,        0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*11*/ {RapdaFormInit,           0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*12*/ {RapdaFormExit,           0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*13*/ {RegionFormInit,          0, DATATYP_CODE, DATAATTR_PTR, 0},
 /*14*/ {RegionFormExit,          0, DATATYP_CODE, DATAATTR_PTR, 0},
} ;

/* field names - RapdaForm */

static enum { REGIONS_LABEL, REGBOX_FORM, TRACKS_LABEL, REGNUM_LABEL,
              X0_LABEL,
              REGSIZE_LABEL, REGET_LABEL, BASET_LABEL, BETUNITS_LABEL,
              ARRSIZE_LABEL, BYLABEL, FTIME_LABEL, FUNITS_LABEL,
              
              F_REGIONS, F_TRACKS, F_REGION_FORM, F_BASET,
              F_ACTIVEX, F_ACTIVEY, F_FTIME
            };

/* field names - RegionScrollForm */

static enum { REG_INIT_LOGIC, REG_UP_LOGIC, REG_DN_LOGIC,
              REG_SEC_LABEL, F_REGNUM, F_X0, F_REGSIZE, F_REGET
            };
  
PRIVATE FIELD RapdaFormFields[] = {

   label_field(REGIONS_LABEL,
   DGROUP_DO_STRINGS, 0,       /*  "#Regions" */
   1, 4, 8),

   label_field(TRACKS_LABEL,
   DGROUP_DO_STRINGS, 1,       /*  "#Tracks" */
   1, 56, 7),

   d_field_(REGBOX_FORM,
   FLDTYP_FORM,
   FLDATTR_DISPLAY_ONLY,
   0,                               
   DGROUP_FORMS, 2,            /* draws box around region scroll form */
   3, 1, 0),                       

   label_field(REGNUM_LABEL,
   DGROUP_DO_STRINGS, 2,       /*  "Region #" */
   4, 4, 8),

   label_field(X0_LABEL,
   DGROUP_DO_STRINGS, 3,       /*  "Start Chnl." */
   4, 15, 11),

   label_field( REGSIZE_LABEL,
   DGROUP_DO_STRINGS, 4,       /*  "#Channels" */
   4, 28, 9),

   label_field( REGET_LABEL,
   DGROUP_DO_STRINGS, 5,       /*  "Region ET" */
   4, 40, 9),

   label_field( BASET_LABEL,
   DGROUP_DO_STRINGS, 6,       /*  "Base ET" */
   4, 56, 7),

   label_field( BETUNITS_LABEL,
   DGROUP_DO_STRINGS, 10,      /*  "sec." */
   4, 72, 4),

   label_field( ARRSIZE_LABEL,
   DGROUP_DO_STRINGS, 7,       /*  "Array Size" */
   16, 4, 10),

   label_field( BY_LABEL,
   DGROUP_DO_STRINGS, 8,       /*  "X" */
   16, 21, 1),

   label_field( FTIME_LABEL,
   DGROUP_DO_STRINGS, 9,       /*  "Frame Time" */
   18, 4, 10),

   label_field(FUNITS_LABEL,
   DGROUP_DO_STRINGS, 10,      /*  "sec." */
   18, 23, 4),

   field_set(F_REGIONS,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID,
   KSI_REGIONS,
   RAPDA_HBASE + 0,
   DGROUP_RAPDA, 0,            /* form_Regions */
   0, 0,
   DGROUP_CODE, 4,             /* form_set_Regions */
   0, 0,                       
   1, 13, 4,
   EXIT, F_REGIONS, F_FTIME, F_REGION_FORM,
   F_FTIME, F_REGION_FORM, F_ACTIVEX, F_REGION_FORM),

   field_set(F_TRACKS,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_DISPLAY_ONLY,
   KSI_TRACKS,
/*    RAPDA_HBASE + 1, */
   0 ,
   DGROUP_RAPDA, 1,            /* form_Tracks */
   0, 0,
   0, 0,
   0, 0,
   1, 64, 4,
   1, 1, 0, 0,
   0, 0, 0, 0),

   field_set(F_REGION_FORM,
   FLDTYP_FORM,
   FLDATTR_NONE,
   KSI_NO_INDEX,
   0,
   DGROUP_FORMS, 1,            /* RegionScrollForm */
   0, 0, 0, 0, 0, 0,
   8, 4, 1,
   EXIT, F_REGION_FORM, F_REGIONS, F_ACTIVEX,
   F_REGIONS, F_BASET, F_REGIONS, F_BASET),

   field_set(F_BASET,
   FLDTYP_SCL_FLOAT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_BASET,
   RAPDA_HBASE + 1,
   DGROUP_RAPDA, 6,            /* base ET */
   0, 0,
   DGROUP_CODE, 9,             /* form_set_BaseET */
   4, 0,
   4, 64, 8,
   EXIT, F_BASET, F_REGIONS, F_ACTIVEX,
   F_REGION_FORM, F_ACTIVEX, F_REGION_FORM, F_ACTIVEX),

   field_set(F_ACTIVEX,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID,
   KSI_ARRSIZE,
   RAPDA_HBASE + 2,
   DGROUP_RAPDA, 7,            /* form_ActiveX */
   0, 0,
   DGROUP_CODE, 10,            /* form_set_ArrSize */
   0, 0,
   16, 15, 4,
   EXIT, F_ACTIVEX, F_REGION_FORM, F_REGIONS,
   F_BASET, F_REGIONS, F_REGION_FORM, F_REGIONS),

   field_set(F_ACTIVEY,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_RAPDA, 8,            /* form_ActiveY */
   0, 0,
   0, 0,
   0, 0,
   16, 22, 4,
   1, 1, 0, 0,
   0, 0, 0, 0),

   field_set(F_FTIME,
   FLDTYP_SCL_FLOAT,
   FLDATTR_RJ | FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_RAPDA, 9,            /* frame_time */
   0, 0,
   0, 0,
   2, 0,
   18, 15, 8,
   1, 1, 0, 0,
   0, 0, 0, 0),
};

PRIVATE FORM  RapdaForm = {
   0, 0,
   FORMATTR_EXIT_RESTORE | FORMATTR_VISIBLE |
   FORMATTR_BORDER | FORMATTR_FULLSCREEN,
   0, 0, 0,
   2, 0, 21, 80,
   0, 0,
   {DGROUP_CODE, 11},
   {DGROUP_CODE, 12},
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(RapdaFormFields) / sizeof(FIELD),
   RapdaFormFields,
   KSI_RAPDA_FORM,
   0,
   STRINGS_Reg,
   RAPFORMS_Reg,
   (DATA *)Action_Registry,
   RapSet_Reg,
   0
} ;

/* A form with no fields used to produce a box */

PRIVATE FORM RegionBoxForm =
  {
  0,                                   /* field_index; */
  0,                                   /* previous_field_index; */
  FORMATTR_BORDER | FORMATTR_VISIBLE,  /* attrib; */
  0,                                   /* next_field_offset; */
  0,                                   /* exit_key_code; */
  0,                                   /* status; */
  5, 3, 11, 51,                        /* row col rows cols */
  0, 0,                                /* row_offset & index - scrolling forms */
  { 0, 0 },                            /* init_function; */
  { 0, 0 },                            /* exit_function; */
  COLORS_DEFAULT,                      /* color_set_index; */
  0, 0, 0, 0,                          /* cursor control data  */
  0,                                   /* number_of_fields; */
  0,                                   /* &fields; */
  KSI_NO_INDEX,                        /* MacFormIndex for keystroke macros */
  0, 0, 0, 0, 0, 0                     /* dataRegistry[6]; (0 means no entry) */
  };

PRIVATE FIELD RegionFormFields[] = {

   field_set(REG_INIT_LOGIC,
   FLDTYP_LOGIC,
   FLDATTR_NONE,
   KSI_NO_INDEX,
   0,
   DGROUP_CODE, 0,
   0, 0, 0, 0, 0, 0,
   0, 1, 1,
   EXIT, F_REGNUM, REG_INIT_LOGIC, REG_INIT_LOGIC,
   REG_INIT_LOGIC, REG_INIT_LOGIC, REG_INIT_LOGIC, REG_INIT_LOGIC ),

   field_set(REG_UP_LOGIC,
   FLDTYP_LOGIC,
   FLDATTR_NONE,
   KSI_NO_INDEX,
   0,
   DGROUP_CODE, 1,
   0, 0, 0, 0, 0, 0,
   0, 3, 1,
   EXIT, F_REGNUM, REG_UP_LOGIC, REG_UP_LOGIC,
   REG_UP_LOGIC, REG_UP_LOGIC, REG_UP_LOGIC, REG_UP_LOGIC ),

   field_set(REG_DN_LOGIC,
   FLDTYP_LOGIC,
   FLDATTR_NONE,
   KSI_NO_INDEX,
   0,
   DGROUP_CODE, 2,
   0, 0, 0, 0, 0, 0,
   0, 5, 1,
   EXIT, F_REGNUM, REG_DN_LOGIC, REG_DN_LOGIC,
   REG_DN_LOGIC, REG_DN_LOGIC, REG_DN_LOGIC, REG_DN_LOGIC ),

   label_field(REG_SEC_LABEL,
   DGROUP_DO_STRINGS, 10,
   0, 45, 4),

   field_set(F_REGNUM,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_DISPLAY_ONLY,
   KSI_NO_INDEX,
   0,
   DGROUP_RAPDA, 2, /* form_CurrRegion */
   0, 0,
   DGROUP_CODE, 5, 
   0, 0,
   0, 1, 3,
   F_X0, F_X0, F_REGNUM, F_REGNUM,
   F_REGNUM, F_REGNUM, F_REGNUM, F_REGNUM ),

   field_set(F_X0,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID,
   KSI_REG_X0,
   RAPDA_HBASE + 3,
   DGROUP_RAPDA, 3, /* form_X0 */
   0, 0,
   DGROUP_CODE, 6,  /* form_X0_set */
   0, 0,
   0, 13, 4,
   EXIT, F_X0, REG_UP_LOGIC, REG_DN_LOGIC,
   F_REGNUM, F_REGSIZE, EXIT, F_REGSIZE ),

   field_set(F_REGSIZE,
   FLDTYP_UNS_INT,
   FLDATTR_RJ | FLDATTR_REV_VID,
   KSI_REGSIZE,
   RAPDA_HBASE + 4,
   DGROUP_RAPDA, 4,  /* form_EndChnl */
   0, 0,
   DGROUP_CODE, 7,   /* form_set_EndChnl */
   0, 0,
   0, 26, 4,
   EXIT, F_REGSIZE, REG_UP_LOGIC, REG_DN_LOGIC,
   F_X0, F_REGET, F_X0, F_REGET ),

   field_set(F_REGET,
   FLDTYP_SCL_FLOAT,
   FLDATTR_RJ | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
   KSI_REGET,
   RAPDA_HBASE + 5,
   DGROUP_RAPDA, 5, 
   0, 0,
   DGROUP_CODE, 8, /* form_set_RegET */
   4, 0,
   0, 36, 9,
   EXIT, F_REGET, REG_UP_LOGIC, REG_DN_LOGIC,
   F_REGSIZE, EXIT_DN, F_REGSIZE, EXIT_DN ),

};

PRIVATE FORM RegionScrollForm = {
   0, 0,                                    /* field indices */
   FORMATTR_VISIBLE | FORMATTR_SCROLLING |  /* attribs */
   FORMATTR_STICKY,
   0,                                       /* next field offset */
   0,                                       /* exit key code */
   0,                                       /* status */
   6, 4, 8, 49,                             /* row col height width */
   0,                                       /* display row offset */
   0,                                       /* virtual row index */
   {DGROUP_CODE, 3},                        /* Init function */
   {DGROUP_CODE, 14},                       /* Exit function */
   COLORS_DEFAULT,
   0, 0, 0, 0,
   sizeof(RegionFormFields) / sizeof(FIELD),
   RegionFormFields, KSI_REGSCROLL_FORM,
   0,                                       /* data registries */
   STRINGS_Reg,
   RAPFORMS_Reg,
   (DATA *)Action_Registry,
   RapSet_Reg,
   0
};

char dummy_data[] = { 5, 5, 5, 5, 5, 5, 5 };
void * regoffset = &RegionScrollForm.dataRegistry;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
PRIVATE BOOLEAN RapdaFormInit(void)
{
  get_ActiveX(&form_ActiveX);
  get_ActiveY(&form_ActiveY);
  get_Tracks(&form_Tracks);

  get_CurrentPoint(&form_CurrRegion); /* get current region */
  get_X0(&form_X0);                   /* get X0, start channel. */
  get_RegSize(&form_EndChnl);         /* get the RegionSize, and */
  if (form_EndChnl)
    form_EndChnl += form_X0 - 1;      /* Convert to end channel. */
  get_Regions(&form_Regions);         /* get #regions */
  get_RegionET(&form_RegET);          /* get Region ET */
  get_FrameTime(&frame_time);

  get_ExposeTime(&form_BaseET);
  return FALSE ;
}

PRIVATE BOOLEAN RapdaFormExit(void)
{
  return FALSE ;
}

PRIVATE BOOLEAN RegionFormInit(void)
{
  return FALSE ;
}

PRIVATE BOOLEAN RegionFormExit(void)
{
  int temp = sort_regions(&form_Regions);
  display_random_field(&RapdaForm, F_REGIONS);
  draw_form() ;
  return (temp);
}

PRIVATE BOOLEAN ChekRegionIdx(int index)
{
  if (index < form_Regions)
    {
    errorCheckDetDriver( set_CurrentPoint( form_CurrRegion = index + 1 ));
    get_X0(&form_X0);                 /* get X0, start channel. */
    get_RegSize(&form_EndChnl);       /* get the RegionSize, and */
    if (form_EndChnl)
      form_EndChnl += form_X0 - 1;    /* Convert to end channel. */
    get_RegionET(&form_RegET);        /* get Region ET */
    }
  return (index >= form_Regions);
}

PRIVATE void redraw_rapda_form(void)
{
  draw_form() ;
}

/*  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
PRIVATE int form_set_Regions(void)
{
  /* should call driver to set NumRegions and return status from driver */
   int temp = errorCheckDetDriver( set_Regions (form_Regions));

   redraw_rapda_form();
   return temp;
}

  /*   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
PRIVATE int form_set_CurrentRegion(void)
{
  int err = errorCheckDetDriver( set_CurrentPoint( form_CurrRegion ) ) ;
  get_X0(&form_X0);                   /* get X0, start channel. */
  get_RegSize(&form_EndChnl);         /* get the RegionSize, and */
  if (form_EndChnl)
    form_EndChnl += form_X0 - 1;      /* Convert to end channel. */
  return err ;
}

PRIVATE void update_ftime(void)
{
  get_FrameTime(&frame_time);
  display_random_field(&RapdaForm, F_FTIME);
}

  /*   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
PRIVATE int form_X0_set(void)
{
  int RegionSize, temp;

  RegionSize = max (8, (form_EndChnl - form_X0 + 1));
  temp = set_X0(form_X0);
  temp = set_RegSize(RegionSize);
  update_ftime();
  return errorCheckDetDriver(temp);
}

  /*   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
PRIVATE int form_set_EndChnl(void)
{
  /* should call driver to set RegSize and return status from driver */
  int RegionSize, temp;
  RegionSize = form_EndChnl ? (form_EndChnl - form_X0 + 1) : 0;
  temp = set_RegSize(RegionSize);
  update_ftime();
  return errorCheckDetDriver(temp);
}

/*  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
PRIVATE int form_set_RegET(void)
{
  /* should call driver to set RegET and return status from driver */
  int temp = errorCheckDetDriver(set_RegionET(form_RegET));

  return (temp);
}

PRIVATE int form_set_BaseET(void)
{
  /* should call driver to set BaseET and return status from driver */
  set_ExposeTime(form_BaseET);
  get_ExposeTime(&form_BaseET);
  update_ftime();
  draw_form() ;
  return(FIELD_VALIDATE_SUCCESS);
}

PRIVATE int form_set_ArrSize(void)
{
  int temp = errorCheckDetDriver(set_ActiveX(form_ActiveX));

/*
 printf ("Rapda:\n");
  for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
      printf ("Region %i, start %i, number %i, offset %i, n %i\n",
             i,
             RAPDA_SETUP.RapdaReg[i].StartPixel,
             RAPDA_SETUP.RapdaReg[i].Number,
             RAPDA_SETUP.RapdaReg[i].DataOffset,
             (int)RAPDA_SETUP.RapdaReg[i].n);
*/
  redraw_rapda_form() ;
  return temp ;
}

/*  init FormTable[] with form addresses */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void registerRapdaForms( void )
{
   FormTable[KSI_RAPDA_FORM] = &RapdaForm ;
   FormTable[KSI_REGSCROLL_FORM] = &RegionScrollForm ;
}
