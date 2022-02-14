/***************************************************************************/
/*  crvhdr35.h                                                             */
/*                                                                         */
/*  copyright (c) 1988, EG&G Instruments Inc.                              */
/*                                                                         */
/*  Functions to handle oma35 curve headers for use by oma4000             */
/*
/  $Header: /OMA4000/Include/CRVHDR35.H 1     6/18/99 1:52a Maynard $
/***************************************************************************/
  
#ifndef CRVHDR35_INCLUDED
#define CRVHDR35_INCLUDED

#include <stdio.h>   // FILE
#include "eggtype.h"

// read an old oma35 type curve header from file, convert to oma4000 curve
// header type and put in DstCurvehdr.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA readConvertCurveHeader1(CURVEHDR *DstCurvehdr, FILE *fhSource);

// return the size of a CURVEHDR1 type in bytes.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int curveHdr1Size(void);

#endif  // CRVHDR35_INCLUDED
