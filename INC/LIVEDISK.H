/* -----------------------------------------------------------------------
/
/  livedisk.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
*/

#ifndef LIVEDISK_INCLUDED
#define LIVEDISK_INCLUDED

// variables used in omaform1.c
extern int LiveDiskCount ;
extern int LiveDiskMode ;
extern char LiveDiskFileName[] ;

int GoLiveDisk( void );

#endif  // LIVEDISK_INCLUDED
