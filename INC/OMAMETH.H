/***************************************************************************/
/*  OMAMETH.H                                                              */
/*                                                                         */
/*  Copyright (c) 1991,  EG&G Instruments Inc.                             */
/*                                                                         */
/*
*  $Header: /OMA4000/Include/OMAMETH.H 1     6/18/99 1:53a Maynard $
*
/***************************************************************************/

#ifndef OMAMETH_INCLUDED
#define OMAMETH_INCLUDED

#include <stdio.h>     // FILE
#include <stddef.h>    // offsetof()

#include "handy.h"     // DESCRIPTION_LENGTH

#ifndef DETINFO_INCLUDED
  #include "detinfo.h"
#endif

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

// These constants are based on the order of the option strings
// getmethodhdr_options in omaform1.c
#define METHOD_LOAD_FULL   0
#define METHOD_SAVE        1
#define METHOD_LOAD_DET    2
#define METHOD_LOAD_PLOT   3

struct mplot_flags {
    unsigned int live_fast : 1;
    unsigned int loop_colors : 1;
};

typedef struct mplot_flags MPLOT_FLAGS;

typedef struct
{
   char              Title[28];
   MPLOT_FLAGS       flags;
   char              XLegend[30];
   float             XMax;
   float             XMin;
   BOOLEAN           XAscending;
   USHORT            XUnits;
   char              YLegend[30];
   float             YMax;
   float             YMin;
   BOOLEAN           YAscending;
   USHORT            YUnits;
   char              ZLegend[30];
   float             ZMax;
   float             ZMin;
   BOOLEAN           ZAscending;
   USHORT            ZUnits;
   int               XZPercent;
   int               YZPercent;
   int               ZPosition;
   int               Style;
   BOOLEAN           PlotPeakLabels;      /* 166 */
} MET_PLOT;

#define STRUCT_VERSION 12

/* InterfaceType Field values */
#define   INTERFACE_NONE      0
#define   INTERFACE_AT_2000   1
#define   INTERFACE_PS2_2000  2     /* not detectable now */
#define   INTERFACE_1460      3
#define   INTERFACE_OMA88     4
#define   INTERFACE_AT_4000   5

struct method_header
{
   CHAR FileTypeID[FTIDLEN];
                                    /* string identifying file as DATA, */
                                    /* METHOD, etc. using fid constants */
                                    /* Other file types will start with:*/
                                    /*      "DAD EG&G PARC"             */
                                    /*      "PIXELMAP EG&G PARC"        */
   UCHAR      StructureVersion;
   USHORT     Length;               /* HDR_BASE_LENGTH + any group_tables */
   CHAR       User;                 /* a user number or character */
   CHAR       Description[DESCRIPTION_LENGTH];  /* 121 bytes */
   USHORT     FileCurveNum;         /* number of curves stored in this file */
   UCHAR      InterfaceType;        /* 0=none, 1=AT, 2=PS/2, 3 = 1460, */
                                    /* 4 = OMA88 5 = 4000/AT*/
   UCHAR      ActiveDetector;       /* detector interface number */
                                    /* nanometers */
   CHAR       DADName[FNAME_LENGTH];  /* file to look for DAD code */
                                    /* 202 */
   UCHAR      Normalize;            /* not 0 = curves are normalized */
   UCHAR      SpectrographUnits;    /* wavelength or wavenumber, */
   FLOAT      Excitation;           /* Raman Shift excitation wavelength */
                                    /* See OMA35.H for values */

   int        Spectrograph;         /* Model number of spectrograph */
   int        Grating;              /* Spectrograph grating  active */
                                    /* "Channel" corresponding to 
                                     * spectrograph reading. */
   FLOAT      GratingCenterChnl [3]; 
   FLOAT      dummy [19];         /* To take up space...*/

   FLOAT      RelPixelWidth;        /* Pixel width compensator */
   FLOAT      CalibCoeff[3][4];   /* X,Y,Z axis calibration  352 */
                                    /* coefficients for least squares */
                                    /* polynomial fitting */
   UCHAR      CalibUnits[3];
   CHAR       BackgrndName[FNAME_LENGTH]; /* filename for  432 */
                                            /* background data */
   CHAR       I0Name[FNAME_LENGTH];   /* 100 % Transmittance data file  509 */
                                        /* to be used for ABS, %T and */
                                        /* other calculations */
   CHAR       InputName[FNAME_LENGTH]; /* general purpose input file  586 */
   CHAR       OutputName[FNAME_LENGTH];/* general purpose output file 663 */
   FLOAT      YTInterval;           /* data point or scan interval.  
                                    /* Also set for NON-YT scans */
   FLOAT      YTPredelay;           /* delay before taking any measurements */
   USHORT     Pia[2];               /* starting PIA values */
   USHORT     SoftwareVersion;      /* Software Version that this file was */
                                    /* written under */
   SHORT      PlotWindowIndex;      /* 1..10 */
   SHORT      ActivePlotSetup;
   BOOLEAN    AutoScaleX;
   BOOLEAN    AutoScaleY;
   BOOLEAN    AutoScaleZ;           /*687 */
   SHORT      WindowPlotSetups[8];
   MET_PLOT   PlotInfo[8];          /*2015 */
   UCHAR      Reserved[100];
   CHAR       DetNumber;            /*2116 */
   DET_INFO * DetInfo;
};

/* typedef struct method_header METHDR;  */
/* typedef METHDR * PMETHDR;              */
/* typedef METHDR far * LPMETHDR;         */

#define SZMETHDR sizeof(METHDR)
#define HDR_BASE_LENGTH  offsetof(METHDR, DetInfo)

extern LPMETHDR InitialMethod;
   
ERR_OMA getInitialMethodFromFile(UCHAR interfaceType);
ERR_OMA saveMethodtoDefaultFile(void);
void DeAllocMetDetInfo(LPMETHDR pMethdr);
ERR_OMA DetInfoToMethod(LPMETHDR *ppMethdr);
void MethodToDetInfo(LPMETHDR pMethdr);

ERR_OMA FindMethdrSpace(LPMETHDR far *Methdr, USHORT length);
void DeAllocMethdr(LPMETHDR pMethdr);
ERR_OMA MethdrWrite(FILE *fhnd, const char * fName,
                            LPMETHDR methdr);
ERR_OMA MethdrRead(FILE *fhnd, const char * fName,
                           LPMETHDR far *methdr);


int getMethodModelNumber(LPMETHDR methdr);

VOID print_method(LPMETHDR);
  
#endif  //  OMAMETH_INCLUDED
