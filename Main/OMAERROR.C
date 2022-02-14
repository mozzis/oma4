/* -----------------------------------------------------------------------
/
/  omaerror.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00         13 November    1989
/  Worked on:  TLB      Version 1.01
/              RAC                           31-JAN-90
/                   include filemsg.h, change arg to va_file_message_window()
/                   delete error_file_name[], no longer needed
/
/
/
/  $Header:   J:/logfiles/oma4000/main/omaerror.c_v   0.15   30 Mar 1992 12:56:44   maynard  $
/  $Log:   J:/logfiles/oma4000/main/omaerror.c_v  $
/
/ --------------------------------------------------------------------------
*/

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>   // strcpy(), strcat()
#include <stdio.h>    // printf()
#include <conio.h>    // cputs()
#include <float.h>

#include "omaerror.h"
#include "basepath.h"    // base_path()
#include "filemsg.h"    // va_file_message_window(), ERROR_FILE
#include "syserror.h"   // ERROR_ALLOC_MEM
#include "macruntm.h"   // SetErrorFlag()
#include "device.h"     // screen_handle
#include "oma4driv.h"    // get_Error()
#include "driverrs.h"   // NO_ERROR
#include "forms.h"      // pKSPlayBack

BOOLEAN BeepEnabled = TRUE;

#define COLORS_ERROR 2

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
BOOLEAN test_for_DOS_critical_error(char * device_or_file)
{
  BOOLEAN error_flag = FALSE;
#ifndef PROT
  ERR_OMA error_number;
  static char DriveLetter[2];

  if (DOS_int24_info)
    {
    switch ((int) (DOS_int24_info & 0xFF))
      {
      case DOSCRIT_WRITE_PROTECT:
        error_number = ERROR_ACCESS_DENIED;
      break;
      case DOSCRIT_DRIVE_NOT_READY:
        error_number = ERROR_DRIVE_NOT_READY;
      break;
      case DOSCRIT_SECTOR_NOT_FOUND:
        error_number = ERROR_SECTOR_NOT_FOUND;
      break;
      case DOSCRIT_DATA_CRC_ERROR:
        error_number = ERROR_BAD_SECTOR;
      break;
      case DOSCRIT_NO_PAPER:
        error_number = ERROR_NO_PAPER;
      break;
      case DOSCRIT_SEEK_ERROR:
        error_number = ERROR_SEEK;
      break;
      case DOSCRIT_UNKNOWN_UNIT:
        error_number = ERROR_BAD_DRIVE;
      break;
      case DOSCRIT_WRITE_FAULT:
        error_number = ERROR_WRITE;
      break;
      case DOSCRIT_READ_FAULT:
        error_number = ERROR_READ;
      break;
      case DOSCRIT_BAD_REQUEST:
      case DOSCRIT_UNKNOWN_MEDIA:
      case DOSCRIT_UNKNOWN_COMMAND:
      case DOSCRIT_GENERAL_FAILURE:
        error_number = ERROR_GENERAL_DISK_FAILURE;
      break;
      }
    DOS_int24_info = 0;

    DriveLetter[0] = (char)((dev_err_info & 0xFF) + 'A');
    error(error_number, DriveLetter);

    error_flag = TRUE;
    }
#endif
  return(error_flag);
}

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

ERR_OMA error(ERR_OMA error_number, ...)
{
  CRECT OldClipRect;
  CRECT ScreenArea; 

  if(error_number == ERROR_ALLOC_MEM)
    {
    char no_mem_string[] = "  Out of memory. \r\n";
    no_mem_string[ 1 ] = '\a';  // make noise
    cputs(no_mem_string);     // direct to screen
    }
  else
    {
    BOOLEAN message_result;
    va_list insert_args;

    va_start(insert_args, error_number);

    if(test_for_DOS_critical_error((char *)insert_args))
      return (error_number);
      
    ScreenArea.ll.x = 0;    
    ScreenArea.ll.y = 0;
    ScreenArea.ur.x = screen.LastVDCXY.x;
    ScreenArea.ur.y = screen.LastVDCXY.y;

    CInqClipRectangle(screen_handle, &OldClipRect); 
    CSetClipRectangle(screen_handle, ScreenArea);

    if ((pKSPlayBack != NULL) && (*pKSPlayBack != -2))
      {
      if (*pKSPlayBack)
        {
        *pKSPlayBack = -2;         // signal error
        SetErrorFlag();
        }
      }

    // Check the result of va_file_message_window()
    message_result = va_file_message_window(base_path(ERROR_FILE),
      error_number, MAX_MESSAGE_ROWS, COLORS_ERROR, insert_args);

    if(message_result) 
      {
      CSetClipRectangle(screen_handle, OldClipRect); 
      return error_number;           
      }
    else {
      char error_string[ 50 ];
      char error_number_string[ 20 ];
      strcpy(error_string, "  Out of memory.  Error number ");
      itoa(error_number, error_number_string, 10);
      strcat(error_string, error_number_string);
      strcat(error_string, "\r\n");
      error_string[ 1 ] = '\a';             // make noise
      cputs(error_string);
      }
    CSetClipRectangle(screen_handle, OldClipRect); 
    }
  // error message has been forced to the screen, wait for a key press
  cputs(" Press a key to continue.\r\n");      
  while(! kbhit());     // wait for a key press
  getch();                // read character, no echo

  if (pKSPlayBack != NULL)  
    {
    if (*pKSPlayBack)
      *pKSPlayBack = -2;    
    }
  return error_number;
}

/* -----------------------------------------------------------------------
/  function:   Provides a common method for sending beeps.  Can turn off
/              by setting BeepEnabled to FALSE.
*/

void ErrorBeep(void)
{
   if (BeepEnabled)
   {
#ifdef PROT
      DosBeep(1100, 200);
#else
      printf("\a");
#endif
   }
}

void ErrorBeepToggle(USHORT Dummy)
{
   BeepEnabled = !BeepEnabled;

   Dummy;
}

// puts up an error message box and then returns either
// FIELD_VALIDATE_SUCCESS or FIELD_VALIDATE_WARNING.
// Provide the return value from a detector drive call as its argument,
// which should be either NO_ERROR or DRIV_ERROR.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int errorCheckDetDriver(int detDriveReturnVal)
{
   FLOAT errorVal;

   if(detDriveReturnVal == NO_ERROR)
      return FIELD_VALIDATE_SUCCESS;

   GetParam(DC_DERROR, & errorVal);  // get the det driver error condition

   // report the error, but translate detector driver error number to
   // an oma4000 error number first.
   error((SHORT)errorVal);

   return FIELD_VALIDATE_WARNING;
}

/***************************************************************/
/* handle error returned when setjmp() catches exception error */
/***************************************************************/
void exception_error(int errnum)
{
  switch(errnum)
    {
    case 1:                  /* divide by zero */
      error(326);
    break;
    case 2:
      error(ERROR_BREAK_ABORT);    /* ctrl-c or ctrl-brk */
    break;
    case 3:
      error(ERROR_FLOATING_POINT); /* floating point error */
    break;
    }
}

