/* $Header: /OMA4000/Include/SYSERROR.H 1     6/18/99 1:53a Maynard $ */
/* -----------------------------------------------------------------------
/
/  syserror.h
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: DAI      Version 1.00         1  December    1989
/
*/ /*
 * 
 *    Rev 1.0   27 Sep 1990 16:52:46   irving
 * Initial revision.
*/ /*
/ ----------------------------------------------------------------------- */
  
#ifndef SYSERROR_INCLUDED
#define SYSERROR_INCLUDED
  
// error code 0 means no error, see eggtype.h

/* 1-89:  real primal errors */
#define  ERROR_ALLOC_MEM                1
#define  ERROR_FLOATING_POINT           2

/* 90-99: Process errrors */
#define  ERROR_DOSSHELL                 90
  
/* 300-399:  Disk I/O errors */
  
#define  ERROR_OPEN                     300
#define  ERROR_CLOSE                    301
#define  ERROR_READ                     302
#define  ERROR_WRITE                    303
#define  ERROR_BAD_FILENAME             304
#define  ERROR_BAD_DIRNAME              305
#define  ERROR_BAD_DRIVE                306
#define  ERROR_ACCESS_DENIED            307
#define  ERROR_SHARING                  308
#define  ERROR_DEV_OPEN                 309
#define  ERROR_DEV_CLOSE                310
#define  ERROR_DEV_READ                 311
#define  ERROR_DEV_WRITE                312
  
/* (320-3??: DOS Disk I/O Critical errors) */
  
#define  ERROR_DRIVE_NOT_READY          320
#define  ERROR_SEEK                     321
#define  ERROR_SECTOR_NOT_FOUND         322
#define  ERROR_BAD_SECTOR               323
#define  ERROR_GENERAL_DISK_FAILURE     324
#define  ERROR_BREAK_ABORT              325
  
/* 400-499:  I/O errors (other than Disk or GPIB I/O) */
  
#define  ERROR_DEV_IOCTL                400
  
#define  ERROR_NO_PAPER                 410
  
/* 500-599:  Operating System errors (other than I/O) */
  
/* 600-699:  GPIB errors */
#define  ERROR_GPIB_OPEN                600
#define  ERROR_GPIB_NO_DEVICE           601
#define  ERROR_GPIB_TIMEOUT             602
#define  ERROR_GPIB_SERIAL_POLL         603
  
#define  ERROR_GENERIC                 9999
  
struct crit_err {
   unsigned    error_code    : 8;
   unsigned    read_or_write : 1;
   unsigned    disk_location : 2;
   unsigned    not_used      : 4;
   unsigned    disk_or_other : 7;
};
  
union crit_err_map {
   struct crit_err   bits;
   unsigned int      word;
} DOS_CRITICAL_ERROR;
  
/* AH values: */
/*    bit 7 - 0 = disk error, 1 = other I/O error */
/*    bits 1,2 - disk location where error occurred */
/*       00 - DOS area */
/*       01 - File Allocation Table (FAT) */
/*       10 - directory */
/*       11 - file area */
/*    bit 0 - 0 = read error, 1 = write error */
  
/* DI values (DOS error code): */
#define  DOSCRIT_WRITE_PROTECT            0     /*  0 - write-protected */
#define  DOSCRIT_UNKNOWN_UNIT             1     /*  1 - unknown unit */
#define  DOSCRIT_DRIVE_NOT_READY          2     /*  2 - drive not ready */
#define  DOSCRIT_UNKNOWN_COMMAND          3     /*  3 - unknown command */
#define  DOSCRIT_DATA_CRC_ERROR           4     /*  4 - data error (bad CRC) */
#define  DOSCRIT_BAD_REQUEST              5     /*  5 - bad request structure length */
#define  DOSCRIT_SEEK_ERROR               6     /*  6 - seek error */
#define  DOSCRIT_UNKNOWN_MEDIA            7     /*  7 - unknown media type */
#define  DOSCRIT_SECTOR_NOT_FOUND         8     /*  8 - sector not found */
#define  DOSCRIT_NO_PAPER                 9     /*  9 - printer out of paper */
#define  DOSCRIT_WRITE_FAULT             10     /* 10 - write fault */
#define  DOSCRIT_READ_FAULT              11     /* 11 - read fault */
#define  DOSCRIT_GENERAL_FAILURE         12     /* 12 - general failure */
/* (including unformatted diskette) */
  
#endif
