/* -----------------------------------------------------------------------
/
/  basepath.c
/
/  Copyright (c) 1991,  EG&G Instruments Inc.
/
/  remember the base directory for finding oma4000.xxx files
/
/  $Header:   J:/logfiles/oma4000/main/basepath.c_v   1.1   15 Jan 1992 09:03:42   cole  $
/  $Log:   J:/logfiles/oma4000/main/basepath.c_v  $
 * 
 *    Rev 1.1   15 Jan 1992 09:03:42   cole
 * Fix bug which caused oma4000.ers to appear as a file name in some error
 * messages instead of the correct file name.
 * 
 *    Rev 1.0   07 Jan 1992 11:52:46   cole
 * Initial revision.
/
*/

#include <stdlib.h>
#include <string.h>

#include "basepath.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

enum { MaxBasePath = 80 } ;

// start of the extension in BasePath
PRIVATE char * ext_start ;

// full path name of executing program, but the file extension is changed
PRIVATE char BasePath[ MaxBasePath ] ;

PRIVATE char errorFileName[ MaxBasePath ] ;

// N.B. The error file is handled specially.  Error messages may contain
// file names obtained from calls to base_path(). Since the error routine
// calls base_path() to obtain the name of the error file, this must not
// change the value returned from the previous call to base_path() or else
// the name of the error file may appear in the error message on screen
// instead of the correct file name.

/* -----------------------------------------------------------------------
/  function: Initialize BasePath to point to the proper string, should be
/     argv[0] from main().  This function MUST be called first, before
/     base_path().  It is assumed that a 3 character extension is part of
/     full_pathname.  DOS will only execute files with an extension of EXE,
/     COM, or BAT ; so this assumption should be valid. Also set up
/     errorFileName to the full path and name of the error file.
/ ----------------------------------------------------------------------- */
void set_basepath(char * full_pathname)
{
   char drive[_MAX_DRIVE],
        dir[_MAX_DIR],
        fname[_MAX_FNAME],
        fext[_MAX_EXT];

   _fullpath(BasePath, full_pathname, MaxBasePath);
   _splitpath(BasePath,drive,dir,fname,fext);
   strcpy(fext,".EXE");
   _makepath(BasePath,drive,dir,fname,fext);
   
   ext_start = BasePath + strlen(BasePath) - 3;

   // now set up errorFileName

   strcpy(ext_start, "ERS");
   strcpy(errorFileName, BasePath);
}

/* -----------------------------------------------------------------------
// The files must be in the same subdirectory as defined by the
// full_pathname argument to set_basepath().
/ ----------------------------------------------------------------------- */
char * base_path( BASE_FILE base_file )
{
   switch( base_file ) {
      case ERROR_FILE  :                              return errorFileName ;
      case HELP_FILE   : strcpy( ext_start, "HLP" ) ; return BasePath ;
      case COLOR_FILE  : strcpy( ext_start, "CLR" ) ; return BasePath ;
      case MACRO_FILE  : strcpy( ext_start, "MAC" ) ; return BasePath ;
      case METHOD_FILE : strcpy( ext_start, "MET" ) ; return BasePath ;
      case FIELD_FILE  : strcpy( ext_start, "FLD" ) ; return BasePath ;
      case FORM_FILE   : strcpy( ext_start, "FRM" ) ; return BasePath ;
   }
   return BasePath ; // default 
}

