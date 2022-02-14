/* ----------------------------------------------------------------------
/
/   basepath.h
/
/   maintain the starting directory path for finding oma4000.xxx files
/
/  Copyright (c) 1991,  EG&G Princeton Applied Research
/
/  $Header: /OMA4000/Include/BASEPATH.H 1     6/18/99 1:52a Maynard $
/  $Log: /OMA4000/Include/BASEPATH.H $
 * 
 * 1     6/18/99 1:52a Maynard
 * 
 *    Rev 1.0   07 Jan 1992 11:46:54   cole
 * Initial revision.
/
/ ----------------------------------------------------------------------- */

#ifndef BASEPATH_INCLUDED
#define BASEPATH_INCLUDED

// There are three files which are supposed to be in the same subdirectory
// as the executing program.  One contains help text, one contains error
// text, and one contains colors data.
  
typedef enum { ERROR_FILE, HELP_FILE, COLOR_FILE, MACRO_FILE,
               METHOD_FILE, FIELD_FILE, FORM_FILE } BASE_FILE;
  

// full_pathname should be the directory where the oma4000.xxx files can be
// found.
// MUST call this first before calling base_path().
void set_basepath(char * full_pathname);
            
// Return a pointer to the file specifier of the file corresponding to
// base_file.  WARNING : Do NOT modify the returned string.  If modification
// is required, copy the string and then modify the copy.
  
char * base_path(BASE_FILE base_file);
 
#endif  // BASEPATH_INCLUDED
