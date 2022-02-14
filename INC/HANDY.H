/***************************************************************************/
/*  HANDY.H                                                                */
/*                                                                         */
/*      Various system constants and variables for DOS programs.           */
/*                                                                         */
/*  last changed:                                                          */
/*                                                                         */
/*           1/3/89     DI       Changed MINFLOAT and MAXFLOAT to give a   */
/*                               product of the maximum float value,       */
/*                               also did same for MIN... and MAXDOUBLE    */
/*          11/21/89    DI       Adapted from OMA35.H                      */
/*
/   $Header: /OMA4000/Include/HANDY.H 1     6/18/99 1:52a Maynard $
 *                                                                         *
/***************************************************************************/
  
#ifndef HANDY_INCLUDED
#define HANDY_INCLUDED
  
#include <time.h>
  
typedef struct tm TM;                 /* time structure */
  
#define SEGSIZE 0x10000L
  
#define SZFLOAT sizeof(float)
#define SZLONG  sizeof(long)

#define MAXFLOAT 3.4E19
#define MINFLOAT -3.4E19
  
#define MAXDOUBLE 1.7E154
#define MINDOUBLE -1.7E154
  
#define MAX_DOUBLE_DIFF (1e-9)  /* 1e-9, for double float comparisons */
#define MAX_FLOAT_DIFF  (1e-9)  /* 1e-9, for single float comparisons */

/* shift status port address and flags for DOS */
enum { RIGHTSHIFT = 1, LEFTSHIFT = 2, CNTRL_FLAG = 4, ALT_FLAG = 8 };

#define SHIFT_STATUS_ADDR  0x00400017

#define PLANCK       6.626176e-34 /* Planck's constant in Joules * seconds */
#define LIGHTSPEED   2.99792458e8 /* speed of light in meters/sec */

#define  PI  3.1415926535

enum { DOSFILESIZE = 12, DOSPATHSIZE = 64 };

enum { FNAME_LENGTH = DOSFILESIZE + DOSPATHSIZE + 1 };

enum { DESCRIPTION_LENGTH = 81 };

enum { FTIDLEN = 40 };

#endif         /* HANDY_INCLUDED */
