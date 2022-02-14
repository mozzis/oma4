/* -----------------------------------------------------------------------
/
/  omaform.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/OMAFORM.H 1     6/18/99 1:53a Maynard $
*/

#ifndef OMAFORM_INCLUDED
#define OMAFORM_INCLUDED
  
#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif
  
#include "formtype.h"

#define SCAN_MENU_COUNT 8

extern unsigned default_form_attributes;

extern DATA_LIMIT *DataLimitRegistry;
  
extern char * MenuModeStr;
extern char * TagExpandStr;
extern char * TagShrinkStr;
extern char * TagSaveStr;

BOOLEAN TempChangeCursorType(SaveAreaInfo ** SavedArea);
void TempRestoreCursorType(BOOLEAN RemGCursor, SaveAreaInfo ** SavedArea);

// return TRUE iff theForm is the special GraphWindow form
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN isFormGraphWindow(FORM * theForm);

BOOLEAN isPrevFormGraphWindow(void);

// return TRUE iff theForm is the special Menu1 form
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN isFormMenu1(FORM * theForm);
BOOLEAN isPrevFormMenu1(void);

// make special form Menu1 the current form and set active status
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setCurrentFormToMenu1(void);

// make special form GraphWindow the current form
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setCurrentFormToGraphWindow(void);

// make MacroForm the current form
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setCurrentFormToMacroForm(void);

// return TRUE iff the current form is the macro form
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN isCurrentFormMacroForm(void);

// Prepare for special graph (with form on screen at the same time) to
// exit to the form.  Returns result of set_attributes() call.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UCHAR specialGrafPrepareForExit(void);

// Return from a special graph (with form on screen at the same time) to
// the on screen form.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void specialGrafReturnToForm(void);

// return TRUE iff a plot is showing on screen at the same time as the form
BOOLEAN isPlotShowingForm(int macFormIndex);

// return TRUE iff the plot area is showing
BOOLEAN plotAreaShowing(void);

// put form addresses into FormTable[]
void registerOmaform1Forms(void);

// Verify that a string may be a proper file name
// Changes Current.FieldDataPtr to upper case and may prepend a path.
int VerifyFileName(void);

// verify that the current field string is a good non-reserved file name 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int VerifyWritableFileName(void);

// same as VerifyFileName() except recognizes special names like lastlive,
// etc. without changing them to upper case or prepending a path.
int specialVerifyFileName(void);

// put up a message window with help message from the help file
void form_help_from_file(int help_index);

#endif    // OMAFORM_INCLUDED
