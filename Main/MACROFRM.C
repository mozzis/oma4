/* -----------------------------------------------------------------------
/
/  macrofrm.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/macrofrm.c_v   0.14   06 Jul 1992 10:30:22   maynard  $
/  $Log:   J:/logfiles/oma4000/main/macrofrm.c_v  $
 * 
 *    Rev 0.14   06 Jul 1992 10:30:22   maynard
 * cast some items in form defs and field data defs to elim compiler warnings
 * 
 *    Rev 0.13   30 Mar 1992 11:49:00   maynard
 * Make a default static value for buflen and point pBuflen to it so pBuflen
 * will never be NULL and cause keystrokes programs to crash.
 * A displayMacroForm entry for use by ReadLn and WriteLn
 * 
 *    Rev 0.12   13 Jan 1992 13:54:32   cole
 * Change #include's. Move isCurrentFormMacroForm() to omaform.c
 * Add registerMacroForm() to put entry in NorecFormTable[].
 * 
 *    Rev 0.11   28 Aug 1991 16:11:46   cole
 * delete include for omamenu.h
 * 
 *    Rev 0.10   25 Jul 1991 10:49:36   cole
 * Change #include's. Add isCurrentFormMacroForm(), setCurrentFormToMacroForm().
 * Move MacroLineVerify() to here. Move FORM MacroForm and its data registries
 * to here and put the data registry addresses in the new dataRegistry[] FORM
 * field, data registries are now static local.
 * 
 *    Rev 0.9   28 May 1991 12:58:26   cole
 * removed all extern's from .c files
 * 
 *    Rev 0.8   19 Feb 1991 13:44:12   irving
 * Added pointer to the default curve set name.  Added stuf to 
 * make live work better during macro execution
 *
 *    Rev 0.7   10 Jan 1991 01:28:34   maynard
 * Incorporate Dwight's changes from 1.81 oma2000
 * Changes for OMA4 macro language support
 * Add temperature control, shutter control, et. al.
 *
 *    Rev 1.1   29 Oct 1990 12:15:38   irving
 * Changes for new cursors.
/
*/

#include <stdio.h>
#include <string.h>

#include "macrofrm.h"
#include "fkeyfunc.h"
#include "omaform.h"  // default_form_attributes
#include "cursor.h"
#include "live.h"
#include "macsymbl.h"
#include "ksindex.h"
#include "helpindx.h"
#include "macparse.h"
#include "forms.h"
#include "formtabs.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

PRIVATE UCHAR MacroLineVerify(char * FieldString, void * FieldData) ;

char CommandInput[80] = "";
char CommandOutput[80] = "";
int buflen = 79;      /* buflen for when macro form not on screen */
char DefaultName[81] = { "auto" }; /* */
int * pBufLen = &buflen;
PCHAR pDefaultName = DefaultName;

enum { DGROUP_DO_STRINGS = 1, DGROUP_CODE, DGROUP_GENERAL } ;

static DATA MacDO_STRING_Reg[] = {

   /* 0 */ { "<", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 1 */ { ">", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
} ;

static DATA MacGENERAL_Reg[] = {

   /* 0  */ { CommandInput, 0, DATATYP_STRING, DATAATTR_PTR, 0 },
   /* 1  */ { CommandOutput, 0, DATATYP_STRING, DATAATTR_PTR, 0 },
} ;

static EXEC_DATA MacCODE_Reg[] = {

   /* 0 */ { CAST_CHR2INT MacroLineVerify, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
} ;

static FIELD MacroFormFields[] = {

   { FLDTYP_STRING,
     FLDATTR_DISPLAY_ONLY | FLDATTR_REV_VID,
     KSI_NO_INDEX,
     0,
     { DGROUP_DO_STRINGS, 1 },      // ">"
     {0, 0},
     {0, 0},
     {0, 0},
     0, 0, 1,
     { 1, 0, 0, 0, 0, 0, 0, 0 } },

   { FLDTYP_STRING,
     FLDATTR_DISPLAY_ONLY | FLDATTR_REV_VID,
     KSI_NO_INDEX,
     0,
     { DGROUP_DO_STRINGS, 0 },      // "<"
     {0, 0},
     {0, 0},
     {0, 0},
     1, 0, 1,
     { 1, 0, 0, 0, 0, 0, 0, 0 } },

   { FLDTYP_STRING,
     FLDATTR_DISPLAY_ONLY | FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
     KSI_NO_INDEX,
     0,
     { DGROUP_GENERAL, 1 },         // CommandOutput
     {0,  0},
     {0,  0},
     {79, 0},
     1, 1, 79,
     { 1, 1, 1, 1, 1, 1, 1, 1 } },

   { FLDTYP_STRING, FLDATTR_REV_VID | FLDATTR_NO_OVERFLOW_CHAR,
     KSI_NO_INDEX,
     MACROFRM_HBASE,
     { DGROUP_GENERAL, 0 },         // CommandInput
     {0, 0},
     {DGROUP_CODE, 0 },             // MacroLineVerify
     {79, 0},
     0, 1, 79,
     { -4, 0, 0, 0, 0, 0, 0, 0 } },
};

static FORM MacroForm = {
  0, 0,
  FORMATTR_FIRST_CHAR_ERASE | FORMATTR_VISIBLE,
  0, 0, 0,
  23, 0, 2, 80,
  0, 0,
  { 0, 0 },
  { 0, 0 },
  COLORS_DEFAULT,    
  0, 0, 0, 0,
  sizeof( MacroFormFields ) / sizeof( MacroFormFields[0] ),
  MacroFormFields,
  KSI_NO_INDEX,
  0, MacDO_STRING_Reg, (DATA *)MacCODE_Reg, MacGENERAL_Reg, 0, 0
};

void LocateMacroForm(void)
{
  MacroForm.row = (UCHAR)(screen_rows - 2);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void RunMacroForm( USHORT Dummy )
{
   FKeyItems[1].Action = SwitchModeLiveKey;
   FKeyItems[39].Control |= MENUITEM_INACTIVE;
   
   LocateMacroForm();

   if (active_locus == LOCUS_MENUS)
      UnselectifyMenu();

   if (EnterMacroForm())
   {
      run_form(&MacroForm, &default_form_attributes, FALSE);
      LeaveMacroForm();
   }

   if (active_locus == LOCUS_APPLICATION)
   {
      SetGraphCursorType(CursorType);
      FKeyItems[1].Action = GoLive;
   }

   FKeyItems[39].Control &= ~MENUITEM_INACTIVE;
   ShowFKeys( &FKey );
   if (active_locus == LOCUS_MENUS)
      redraw_menu();

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE UCHAR MacroLineVerify(char * FieldString, void * FieldData)
{
  SHORT StrLen;
  UCHAR status = FIELD_VALIDATE_SUCCESS;

  /* don't do StartNewParseOperation() if more than one line allowed! */
  StartNewParseOperation();

  StrLen = strlen(CommandInput);
  CommandInput[StrLen] = (CHAR)EOFCH;
  CommandInput[StrLen+1] = '\0';
  ParseSingleLine(CommandInput);   /* all parsing and compiling done here */
  CommandInput[StrLen] = '\0';

  switch (MacroStatus)
    {
    case PARSE_COMPLETE:
      RunMainProgram();    /* program executed here */
    break;
    default:
    case PARSE_ERROR:
      status = (UCHAR) ErrorOffset;
    }
  /* cause field to erase next time around */
  default_char_action(NIL);
  return( status );
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void registerMacroForm( void )
{
   NorecFormTable[NOREC_MACRO_FORM] = &MacroForm ;
}


void displayMacroForm(void)
{
  setup_for_form_entry(&MacroForm, &default_form_attributes);
}
