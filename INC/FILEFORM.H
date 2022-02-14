//
// file fileform.h   RAC   December 6, 1989
//
// test file and functions for scrolling curve directory
/*
/  $Header: /OMA4000/Include/FILEFORM.H 1     6/18/99 1:52a Maynard $
*/

#ifndef FILEFORM_INCLUDED  // idempotent header file
#define FILEFORM_INCLUDED

// Put up a scrolling window for curve_directory, and a scrolling window
// corresponding to files on disk. Let the user add/delete to/from the
// curve directory.  Uses DOS environment variable "OMADATA" as the file
// specifier for the disk files to show, but only the first time it is
// called.  Thereafter, starts up with the last used file specifier.
// Defaults to ".\*.*" if there is no "OMADATA" environment variable.

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

// init FormTable with form addresses : UserForm, FileScrollForm,
//                                      ControlForm, CurveScrollForm
void registerScrlltstForms(void);

// set default filename filter string

void set_dataspec(char * fileSpec);

BOOLEAN is_special_name(char * name);

#endif // FILEFORM_INCLUDED
