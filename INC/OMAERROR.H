/* $Header: /OMA4000/Include/OMAERROR.H 1     6/18/99 1:53a Maynard $ */
/* -----------------------------------------------------------------------
/
/  omaerror.h
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00         13 November    1989
/
/* ----------------------------------------------------------------------- */

#ifndef OMAERROR_INCLUDED
#define OMAERROR_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern unsigned int DOS_int24_info;  /* note these are defined in */
extern dev_err_info;                 /* int24.asm */
void prepare_int24_trap(void);

#define MAX_MESSAGE_ROWS   14

/* This value is currently used only by the popup curve set handler */
/* It may be used for help and error messages at a later date */
#define MAX_MESSAGE_LEN    52           

/* error code 0 means no error, defined in eggtype.h */
/* #define ERROR_NONE                     0           */

/* 100-199:  Curve Directory Management errors */

#define  ERROR_TOO_MANY_CURVES          100
#define  ERROR_CURVE_NUM                101
#define  ERROR_IMPROPER_FILETYPE        102
#define  ERROR_BAD_CURVE_BLOCK          103
#define  ERROR_POINT_NUM                104
#define  ERROR_READ_CURVE_HEADER        105
#define  ERROR_CORRUPT_FILE             106 
#define  ERROR_OLD_VERSION              107  
#define  ERROR_CURVESET_NOT_FOUND       108
#define  ERROR_POINT_INDEX              109

/* 200-299:  Data Acquisition errors */

#define  ERROR_ACQ                      200

/* detector errors -- corresponding to the errors defined in driverrs.h */

#define  ERROR_DETECTOR_OFFSET      700
#define  ERROR_FAKEDETECTOR         700
#define  ERROR_NOSUCH_PARAM         701
#define  ERROR_MONITOR_VER          702 /* Monitor version wrong */
#define  ERROR_DA_ACTIVE            704
#define  ERROR_BAD_STRUCTURE        705 /* didn't find start or end addrs */
#define  ERROR_TOO_LONG             706 /* Template was too long */
#define  ERROR_NO_SERIAL_CODE       707 /* No code given for serial reg read*/
#define  ERROR_NO_CODE_SPACE        708 /* No address given for code output */
#define  ERROR_NO_CLEAN_CODE        709 /* No address for clean shift reg */
#define  ERROR_DETECTOR_TIMEOUT     710 /* No answer to GO or STOP command */
#define  ERROR_POINT_NUMBER         711 /* CurrentPoint was > ActiveX */
#define  ERROR_TRACK_NUMBER         712 /* CurrentTrack was > ActiveY */
#define  ERROR_EXPOSE_TIME          713 /* ET out of range            */
#define  ERROR_NOSUCH_DAPROG        714 /* The DA program doesn't exist */
#define  ERROR_TOO_MANY_MEMORIES    715 /* Number of memories too large */
#define  ERROR_NO_LIVE_DATA         716
#define  ERROR_MONITOR_LOAD         722 /* couldn't load monitor file */
#define  ERROR_ODD_TEMP             731 /* Temp entered was odd */
#define  ERROR_NOFASTSHIFT          732 /* Detector doesn't have fast shiftline capability */

#define IBERR_FILE_OFFSET 1500          /* 1500 to 1516 in OMA4000.ERS */
/* iberr = 9 is an undocumented GPIB error code */
#define UNDOCUMENTED_OFFSET ( IBERR_FILE_OFFSET + 9 )
#define MAX_IBERR_NUMBER 16           /* 0 thru 16 are documented */

/* for downfile.c */
#define ERROR_FILE_OFFSET 1600
#define ERR_RESPONSE_WRITE    ERROR_FILE_OFFSET
#define ERR_COMMAND_OPEN    ( ERROR_FILE_OFFSET + 1 )
#define ERR_RESPONSE_OPEN   ( ERROR_FILE_OFFSET + 2 )
#define ERR_NO_COMMAND_EXEC ( ERROR_FILE_OFFSET + 3 )
#define ERR_COMMAND_CLOSE   ( ERROR_FILE_OFFSET + 4 )
#define ERR_RESPONSE_CLOSE  ( ERROR_FILE_OFFSET + 5 )
#define ERR_NO_CALLBACK     ( ERROR_FILE_OFFSET + 6 )
#define ERR_NO_MONITOR_FILE ( ERROR_FILE_OFFSET + 7 )
#define ERR_NO_SCAN_FILE    ( ERROR_FILE_OFFSET + 8 )
#define ERR_NO_DAC_FILE     ( ERROR_FILE_OFFSET + 9 )

/* for mathform.c */
#define ERROR_MF_OFFSET 1700
#define ERROR_CURVE_CONST        ERROR_MF_OFFSET
#define ERROR_SPLIT_RANGE        ERROR_MF_OFFSET + 1
#define ERROR_NO_OPERAND_CURVE   ERROR_MF_OFFSET + 2
#define ERROR_MATRIX_SIZE        ERROR_MF_OFFSET + 3
#define ERROR_SOURCE_OVERWRITE   ERROR_MF_OFFSET + 4

/* for calib.c */
#define ERROR_CAL_OFFSET            1800
#define ERROR_CAL_TOO_MANY_POINTS   ERROR_CAL_OFFSET
#define ERROR_CAL_TOO_FEW_POINTS    ERROR_CAL_OFFSET + 1
#define ERROR_CAL_NOT_MONOTONIC     ERROR_CAL_OFFSET + 2
#define ERROR_CAL_NOT_UNIQUE_Y      ERROR_CAL_OFFSET + 3
#define ERROR_CAL_PIXEL_UNITS       ERROR_CAL_OFFSET + 4 
#define ERROR_CONVERT_UNITS         ERROR_CAL_OFFSET + 5

/* for spgraph */
#define ERROR_1235_OFFSET           1850
#define ERROR_1235_FAIL             1850
#define ERROR_1235_NOANS            1851

/* for scrlltst.c */
#define ERROR_FILEFORM_OFFSET       1900
#define ERROR_NOT_UNIQUE_CURVE      ERROR_FILEFORM_OFFSET
#define ERROR_NO_SPECIAL_OVERWRITE  ERROR_FILEFORM_OFFSET + 1
#define ERROR_NO_INDEX_OVERWRITE    ERROR_FILEFORM_OFFSET + 2
#define ERROR_NO_SPECIAL_DELETE     ERROR_FILEFORM_OFFSET + 3

/* for the macro language */
/* run-time errors */
#define ERROR_MACRO_OFFSET          2000
#define ERROR_BAD_ASSN_RTYPE        ERROR_MACRO_OFFSET
#define ERROR_READ_ONLY_LTYPE       ERROR_MACRO_OFFSET + 1
#define ERROR_BAD_PARAM_TYPE        ERROR_MACRO_OFFSET + 2
#define ERROR_BAD_PARAM_RANGE       ERROR_MACRO_OFFSET + 3
#define ERROR_MACRO_DEBUG           ERROR_MACRO_OFFSET + 4
#define ERROR_STACK_EMPTY           ERROR_MACRO_OFFSET + 5
#define ERROR_STACK_FULL            ERROR_MACRO_OFFSET + 6
#define ERROR_ARRAY_BOUNDS          ERROR_MACRO_OFFSET + 7
/* compile-time errors */
#define ERROR_SYMTAB_FULL           ERROR_MACRO_OFFSET + 8
#define ERROR_SYMBOL_UNKNOWN        ERROR_MACRO_OFFSET + 9
#define ERROR_SYMBOL_REDEFINED      ERROR_MACRO_OFFSET + 10
#define ERROR_STRING_UNTERMINATED   ERROR_MACRO_OFFSET + 11
#define ERROR_TOO_MANY_STRINGS      ERROR_MACRO_OFFSET + 12
#define ERROR_SYNTAX                ERROR_MACRO_OFFSET + 13
#define ERROR_EXPR_TOO_COMPLEX      ERROR_MACRO_OFFSET + 14
#define ERROR_PARAM_COUNT           ERROR_MACRO_OFFSET + 15
#define ERROR_NO_RETURN_VALUE       ERROR_MACRO_OFFSET + 16
#define ERROR_UNKNOWN_PROC          ERROR_MACRO_OFFSET + 17
#define ERROR_MODIFY_CONSTANT       ERROR_MACRO_OFFSET + 18
#define ERROR_MAC_CLOSE_FAIL        ERROR_MACRO_OFFSET + 19
#define ERROR_MAC_WRITE_FAIL        ERROR_MACRO_OFFSET + 20
#define ERROR_MAC_FILE_SEEK_FAIL    ERROR_MACRO_OFFSET + 21
#define ERROR_MAC_READ_FAIL         ERROR_MACRO_OFFSET + 22
#define ERROR_PROGRAM_TOO_LARGE     ERROR_MACRO_OFFSET + 23
#define ERROR_LINE_NUMBER           ERROR_MACRO_OFFSET + 24

/* for the keystroke macro record and playback */
/* run-time errors */
#define ERROR_KEYSTROKE_OFFSET      2100
#define ERROR_RECORD                ERROR_KEYSTROKE_OFFSET + 0
#define ERROR_PLAY                  ERROR_KEYSTROKE_OFFSET + 1
#define ERROR_INCORRECT_MODE        ERROR_KEYSTROKE_OFFSET + 2
#define ERROR_NOT_MENU_FORM_ITEM    ERROR_KEYSTROKE_OFFSET + 3
#define ERROR_NOT_IN_FORM           ERROR_KEYSTROKE_OFFSET + 4

/* for autopeak.c */
#define ERROR_AUTOPEAK_OFFSET       2200
#define ERROR_CURVE_PEAKS_FULL      ERROR_AUTOPEAK_OFFSET + 0

/* for wintags.c */
#define ERROR_WINTAGS_OFFSET        2300
#define ERROR_WINTAGS_NO_MEM        ERROR_WINTAGS_OFFSET + 0

/* for plot setup form */
#define ERROR_WINDOW_NOT_VISIBLE    2400
#define ERROR_BAD_HEAP              2410

/* background subtract errors */
#define ERROR_BACKGROUND_OFFSET     2500
#define ERROR_BKG_TO_BKG            ERROR_BACKGROUND_OFFSET + 0
#define ERROR_NO_REFERENCE_DATA     ERROR_BACKGROUND_OFFSET + 1
#define ERROR_DUAL_TRACK_MODE       ERROR_BACKGROUND_OFFSET + 2

/* for compatibility with earlier error handler */
void PrintErrMsg( char * string1, ERR_OMA ErrorNumber, char * string2);

/* -----------------------------------------------------------------------
/  function:   Provides a common method for displaying error
/              messages in pop-up windows.  The error number
/              is used to find the associated message in the
/              error message file.  These messages may optionally
/              have embedded formatting commands starting with '%',
/              which are used in the exact same way as printf()
/              uses them.  One minor difference is that there
/              may be multi-line messages in the file, but you
/              don't have to do anything special; the parameters
/              are used as the formatting commands are found.
/              If there is no need for parameters, simply give
/              the error number.
/  requires:   (ERR_OMA) error_number - the number
/              associated with the message in the error message
/              file.
/              { ... } - (optional) parameters of any type, to
/              match the '%' formatting commands in the specific
/              message.
/  returns:    (void)
/  side effects:  consumes a fair bit of memory, so don't use
/              it to report out-of-memory errors!
/
/ ----------------------------------------------------------------------- */
ERR_OMA error(ERR_OMA error_number, ... );

/* -----------------------------------------------------------------------
/  function:   When DOS has a problem with an I/O device that
/              is difficult to handle, it calls interrupt 0x24
/              to report it (unlike a real operating system,
/              which would simply return an error to the calling
/              program).  The default int 24 service routine pops
/              up the infamous "Abort, Retry, Fail?" message, which
/              effectively allows the user to crash the program.
/              The better way to handle this interrupt is to save
/              the error information, then tell DOS to fail the
/              I/O operation.  A routine to do this is provided
/              in the module INT24.ASM.  test_for_DOS_critical_error()
/              is called after any DOS I/O function fails.
/              The critical error information is tested (if nonzero,
/              a critical error happened), and a report is made
/              indicating what caused the problem.
/              (NOTE: for this function to work, the INT24.ASM
/              module must be linked in to the program, and the
/              prepare_int24_trap() initialization function must
/              be called.)
/
/  requires:   (char *) device_or_file - a string containing the
/              device and/or file name the operation was done on.
/
/  returns:    (BOOLEAN) - TRUE if critical error detected.
/
/  side effects:  resets the global variable DOS_int24_info to zero.
/
/ ----------------------------------------------------------------------- */

BOOLEAN test_for_DOS_critical_error(char * device_or_file);
void ErrorBeep( void );       
void ErrorBeepToggle( USHORT Dummy );

/* puts up an error message box and then returns either */
/* FIELD_VALIDATE_SUCCESS or FIELD_VALIDATE_WARNING. */
/* Provide the return value from a detector drive call as its argument, */
/* which should be either NO_ERROR or DRIV_ERROR. */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
int errorCheckDetDriver( int detDriveReturnVal ) ;

/***************************************************************/
/* handle error returned when setjmp() catches exception error */
/***************************************************************/
void exception_error(int);

#endif
