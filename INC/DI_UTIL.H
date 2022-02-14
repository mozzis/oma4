/***************************************************************************/
/*  di_util.c                                                              */
/*                *** OMA 35 Version ***                                   */
/*                                                                         */
/*  copyright (c) 1988, EG&G Instruments Inc.                              */
/*                                                                         */
/***************************************************************************/
  
#ifndef DI_UTIL_INCLUDED
#define DI_UTIL_INCLUDED
  
#include <stdio.h>   /* FILE */

#include "eggtype.h"

SHORT mem_dump(PVOID, ULONG, FILE *);
SHORT lpprintf(char *);
VOID nullproc(VOID);
VOID SysWait(ULONG);

int InverseFloatCompare(float *, float *);
  
int FloatCompare(float *, float *);
  
int InverseUcharCompare(unsigned char *, unsigned char *);
  
int UcharCompare(unsigned char *, unsigned char *);
  
int InverseIntCompare(int *, int *);
  
int IntCompare(int *, int *);
  
int InverseUintCompare(unsigned *, unsigned *);
  
int UintCompare(unsigned *, unsigned *);
  
int InverseLongCompare(long *, long *);
  
int LongCompare(long *, long *);
  
int InverseUlongCompare(unsigned long *, unsigned long *);
  
int UlongCompare(unsigned long *, unsigned long *);
  
int InverseDoubleCompare(double *, double *);
  
int DoubleCompare(double *, double *);
  
int  StrCompare(CHAR * *, CHAR * *);
int  InverseStrCompare(CHAR * *, CHAR * *);
SHORT ParseFileName (CHAR *, CHAR *);
SHORT ParsePathAndName(CHAR *PathOut, CHAR *NameOut, CHAR *BufIn);

//SHORT MyDtoA(double, CHAR *, SHORT);
//SHORT MyStoA(SHORT, CHAR *);

BOOLEAN ConvertUnits(UCHAR, PDOUBLE, UCHAR, double, double);
  
//BOOLEAN MatchStringSpec(PCHAR, PCHAR);

DOUBLE StartStopWatch(PDOUBLE);
DOUBLE StopStopWatch(DOUBLE);
VOID StripExp(PCHAR, SHORT, BOOLEAN);
  
VOID ConvertTypes(void * Source, int SourceType,
                   void *Destination, int DestinationType);
  
// delete one character from a string
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void drop_char(char * ptr);

// delete leading + and 0's from exponent and number string
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int condense_float_string(char * string);

#endif
