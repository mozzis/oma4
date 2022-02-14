/***************************************************************************/
/*  DETINFO.H                                                              */
/*                                                                         */
/*  Copyright (c) 1991,  EG&G Instruments Inc.                             */
/*                                                                         */
/*
 *  $Header: /OMA4000/Include/DETINFO.H 1     6/18/99 1:52a Maynard $
 * 
 *    Rev 1.0   07 Jan 1992 11:50:14   cole
 * Initial revision.
 * 
 ***************************************************************************/

#ifndef DETINFO_INCLUDED
#define DETINFO_INCLUDED

#include <stdio.h>  // FILE

#include "eggtype.h"

/* typedef struct detector_info DET_INFO; */

// allocate space for the indicated number of DET_INFO structures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DET_INFO * allocateDetInfoSpace(int numDetectors);

// i is the detector number 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA readDetInfoGroupsFromFile(DET_INFO * detInfo, int i,
                                         FILE * fhnd, const char * fName,
                                         ULONG * testLen);

// number of bytes needed to store detNumber DET_INFO structures, but not
// including GroupTables
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int detInfoBaseLen(int detNumber);

// return the size in bytes for the i'th detinfo groups
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int sizeOfDetInfoGroups(DET_INFO * detInfo, int i);

// write out det info stuff to a file
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA detInfoWriteToFile(DET_INFO * detInfo, int i, FILE * fhnd,
                                   const char * fName);


// deallocate the memory for the DET_INFO area which is actually a copy
// of info elsewhere in the system
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DeAllocDetInfo(DET_INFO * detInfo, USHORT detNumber);


// fill detector info from the detector driver. Set * detCount to the number
// of detectors
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA DetDriverToDetInfo(DET_INFO * * theInfo, int * detCount,
                                   USHORT * totalDetLen);

// load detector driver from detector info
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DetInfoToDetDriver(DET_INFO * detInfo, SHORT detNumber);

// functions for #ifdef DEBUG

// print out the detector information to stderr. detNumber is the number of
// detectors and detectorInfo points to an array of DET_INFO structures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void printMethodDetInfo(DET_INFO * detectorInfo, USHORT detNumber);

int getDetInfoModelNumber(DET_INFO * detinfo);

#endif  //  OMAMETH_INCLUDED
