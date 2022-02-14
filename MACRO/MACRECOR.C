/* -----------------------------------------------------------------------
/
/  macrecor.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/macro/macrecor.c_v   0.22   06 Jul 1992 12:51:12   maynard  $
/  $Log:   J:/logfiles/oma4000/macro/macrecor.c_v  $
 * 
 *    Rev 0.22   06 Jul 1992 12:51:12   maynard
 * Try to improve keystroke behavor in scrolling forms - if index is greater
 * than screen height of form, then scroll down, etc.  Allow more general
 * keystroke programs.
 * 
 * Make sure FKeys show when done with playback, since now it is possible for
 * the command line to be displayed by keystroke programs
 * 
*/

#include <string.h>
#include <malloc.h>

#include "macrecor.h"
#include "barmenu.h"
#include "omaform.h"    // Menu1
#include "di_util.h"    // SysWait()
#include "crvheadr.h"
#include "handy.h"      // DOSFILESIZE
#include "macruntm.h"
#include "cursor.h"     // RemoveGraphCursor()
#include "ksindex.h"    // NUM_FORMS
#include "helpindx.h"   // GRAPH_HBASE
#include "formwind.h"   // message_pause()
#include "mdatstak.h"
#include "macrores.h"   // isMacPlayMenu()
#include "macsymbl.h"   // SymbolTable
#include "macparse.h"
#include "syserror.h"   // ERROR_OPEN
#include "omaerror.h"
#include "fkeyfunc.h"   // FKeyItems
#include "formtabs.h"   // FormTable[]
#include "basepath.h"
#include "forms.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

FILE *hMacRecordFile;

PRIVATE FILE *hMacFormFile = NULL;       
PRIVATE FILE *hMacFieldFile = NULL;

CHAR MacRecordFileName[FNAME_LENGTH];

#define MODE_RECORD  0
#define MODE_PLAY    1

SHORT KeyStrokeMode = 0;

PRIVATE PCHAR PlayFieldStr = NULL;                
PRIVATE SHORT PlayStrPos = 0;

BOOLEAN MacRecord = FALSE;
BOOLEAN MacPlayBack = FALSE;
FLOAT PlayBackDelay = (FLOAT) 0;

SHORT MacMenuSymbol;
SHORT MacFormSymbol;
SHORT MacFieldSymbol;

SHORT MostRecentMenuIndex = 0;
SHORT MostRecentFormIndex = 0;
SHORT MostRecentFieldIndex = 0;

char * FileOverwritePrompt[] = {"This option will irretrievably ",
   "overwrite an existing file. ",
   "Continue?",
NULL};

typedef struct {
   PVOID PrevHistory;
   SHORT CurForm;
} FORM_HISTORY;                        

PRIVATE FORM_HISTORY *LastForm;         

typedef struct {
   PVOID PrevHistory;
   FORM *pForm;
} FORM_STACK;

PRIVATE FORM_STACK *LastPlayedForm;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void getPlayName(unsigned int exeListIndex, char Buf[], int formNameSize)
{
  ULONG FileOffset;

  if(isMacPlayMenu(exeListIndex) || isMacPlayForm(exeListIndex))
    {
    int mostRecentIndex;

    if(isMacPlayMenu(exeListIndex))
      mostRecentIndex = MostRecentMenuIndex;
    else
      mostRecentIndex = MostRecentFormIndex;

    FileOffset = (ULONG) (mostRecentIndex) * formNameSize;
    if (fseek(hMacFormFile, FileOffset, SEEK_SET))
      error(ERROR_SEEK, base_path(FORM_FILE));
    else
      {
      if (fread(Buf, formNameSize, 1, hMacFormFile) != 1)
        error(ERROR_READ, base_path(FORM_FILE));
      }
    }
  else if(isMacPlayField(exeListIndex))
    {
    FileOffset = ((ULONG)(MostRecentFieldIndex-1) * formNameSize) +
      (ULONG) (NUM_FORMS * sizeof(BOUNDS)) +
      (ULONG) sizeof(int);

    if (fseek(hMacFieldFile, FileOffset, SEEK_SET))
      error(ERROR_SEEK, base_path(FIELD_FILE));
    else
      {
      if (fread(Buf, formNameSize, 1, hMacFieldFile) != 1)
        error(ERROR_READ, base_path(FIELD_FILE));
      }
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA ReadFormString(SHORT FormIndex, PCHAR Buf)
{
  ULONG FileOffset;

  FileOffset = (FormIndex) * FORM_NAME_SIZE;

  if (fseek(hMacFormFile, FileOffset, SEEK_SET) != 0) {
    error(ERROR_SEEK, base_path(FORM_FILE));
    return ERROR_SEEK;
    }

  if (fread(Buf, FORM_NAME_SIZE, 1, hMacFormFile) != 1) {
    error(ERROR_READ, base_path(FORM_FILE));
    return ERROR_READ;
    }

  Buf[FORM_NAME_SIZE] = '\0';

  return ERROR_NONE;
}

PRIVATE void MacActivatedForm(SHORT FormIndex, BOOLEAN Nested)
{
  CHAR CommandBuffer[FORM_NAME_SIZE + 1];
  ERR_OMA err;

   
  if ((err = ReadFormString(FormIndex, CommandBuffer)) == ERROR_NONE)
    {
    if (Nested)
      {
      if(fprintf(hMacRecordFile, "%s(TRUE);\n", CommandBuffer) == 0)
        error(err = ERROR_WRITE, MacRecordFileName);
      }
    else
      {
      if(fprintf(hMacRecordFile, "%s(FALSE);\n", CommandBuffer) == 0)
        error(err = ERROR_WRITE, MacRecordFileName);
      }

    if (err != ERROR_NONE)
      {
      error(ERROR_RECORD);
      MacRecord = FALSE;
      }
    }
  else
    {
    error(ERROR_RECORD);
    MacRecord = FALSE;
    }
}

PRIVATE void MacDeActivatedForm()
{
  if (fprintf(hMacRecordFile, "LEAVE_FORM();\n") == 0)
    {
    error(ERROR_WRITE, MacRecordFileName);
    error(ERROR_RECORD);
    MacRecord = FALSE;
    }
}

ERR_OMA ReadMenuString(SHORT MenuIndex, PCHAR Buf) 
{
  ULONG FileOffset;

  FileOffset = (MenuIndex-1) * FORM_NAME_SIZE;

  if(fseek(hMacFormFile, FileOffset, SEEK_SET) != 0) {
    error(ERROR_SEEK, base_path(FORM_FILE));
    return ERROR_SEEK;
    }

  if(fread(Buf, FORM_NAME_SIZE, 1, hMacFormFile) != 1) {
    error(ERROR_READ, base_path(FORM_FILE));
    return ERROR_READ;
    }

  Buf[FORM_NAME_SIZE] = '\0';

  return ERROR_NONE;
}

PRIVATE void MacActivatedMenu(SHORT MenuIndex, SHORT ItemIndex)
{
  ERR_OMA err;
  CHAR CommandBuffer[FORM_NAME_SIZE + 1];

  if ((err = ReadMenuString(MenuIndex, CommandBuffer)) == ERROR_NONE)
    {
    if (fprintf(hMacRecordFile, "%s('%s');\n", CommandBuffer,
      MenuFocus.ActiveMENU->ItemList[ItemIndex].Text) == 0)
      {
      error(ERROR_WRITE, MacRecordFileName);
      error(ERROR_RECORD);
      MacRecord = FALSE;
      return;
      }
    }
  else
    {
    error(ERROR_RECORD);
    MacRecord = FALSE;
    }
}

PRIVATE void MacDeActivatedMenu()
{
  if (fprintf(hMacRecordFile, "LEAVE_MENU();\n") == 0)
    {
    error(ERROR_WRITE, MacRecordFileName);
    error(ERROR_RECORD);
    MacRecord = FALSE;
    return;
    }
}

ERR_OMA ReadFieldString(SHORT FieldIndex, PCHAR Buf,   
                                                 PCHAR *DisplayString)
{
  ULONG FileOffset;

  FileOffset = ((FieldIndex - 1) * FORM_NAME_SIZE) +
    (NUM_FORMS * sizeof(BOUNDS)) + sizeof(int);

  if (fseek(hMacFieldFile, FileOffset, SEEK_SET) != 0) {
    error(ERROR_SEEK, base_path(FIELD_FILE));
    return ERROR_SEEK;
    }

  if (fread(Buf, FORM_NAME_SIZE, 1, hMacFieldFile) != 1) {
    error(ERROR_READ, base_path(FIELD_FILE));
    return ERROR_READ;
    }

  Buf[FORM_NAME_SIZE] = '\0';

  /* strip off leading spaces */
  while (**DisplayString == ' ') (*DisplayString)++;

  /* strip off trailing spaces */
  while ((*DisplayString)[strlen(*DisplayString) - 1] == ' ')
    (*DisplayString)[strlen(*DisplayString) - 1] = '\0';

  return ERROR_NONE;
}

PRIVATE void MacActivatedField(SHORT GlobalFieldIndex, PCHAR DisplayString)
{
  CHAR CommandBuffer[FORM_NAME_SIZE + 1];
  PCHAR LocalString = DisplayString;

  if ((ReadFieldString(GlobalFieldIndex, CommandBuffer,
    &LocalString)) != ERROR_NONE)
    {
    error(ERROR_RECORD);
    MacRecord = FALSE;
    return;
    }

  if (fprintf(hMacRecordFile, "%s('%s');\n", CommandBuffer,
    LocalString) == 0)
    {
    error(ERROR_WRITE, MacRecordFileName);
    error(ERROR_RECORD);
    MacRecord = FALSE;
    }
}

PRIVATE void MacFocusedOnField(SHORT FormFieldIndex)
{
  if (fprintf(hMacRecordFile,"FOCUS_FIELD(%d);\n", FormFieldIndex) == 0)
    {
    error(ERROR_WRITE, MacRecordFileName);
    error(ERROR_RECORD);
    MacRecord = FALSE;
    }
}

// record any string into record file
void MacRecordString(PCHAR String)     
{
  if (fprintf(hMacRecordFile, String) == 0)
    {
    error(ERROR_WRITE, MacRecordFileName);
    error(ERROR_RECORD);
    MacRecord = FALSE;
    }
}

SHORT LookupFormFieldMenu(PCHAR Name, PSHORT FldIndex)
{
  SHORT ReturnVal = INVALID_INDEX;

  strupr(Name);           // make uppercase
  if (!strncmp(Name, "FORM", 4))
    {
    ReturnVal = LookupForm(Name, FldIndex);
    }
  else if (!strncmp(Name, "MENU", 4))
    ReturnVal = LookupMenu(Name, FldIndex);
  else if (!strncmp(Name, "FLD", 3))
    ReturnVal = LookupField(Name, LastForm->CurForm, FldIndex);

  return ReturnVal;
}

SHORT FindActiveMenuItem(MENU *ActiveMenu, PCHAR ItemStr)
{
  SHORT i;

  for (i=0; i<ActiveMenu->ItemCount; i++)
    {
    if (stricmp(ItemStr, ActiveMenu->ItemList[i].Text) == 0)
      return i;
    }
   return -1;
}

void MacPlayMenu()
{
  PCHAR ItemStr;
  ERR_OMA err = ERROR_NONE;

  if (PopFromDataStack(&MostRecentMenuIndex, TYPE_INTEGER) != TYPE_INTEGER)
    err = ERROR_MACRO_DEBUG;
  else if ((MostRecentMenuIndex > (NUM_MENUS + NUM_FORMS)) ||
    (MostRecentMenuIndex <= NUM_FORMS))
    err = ERROR_MACRO_DEBUG;
  if (err)
    {
    error(err, "Menu Playback");
    SetErrorFlag();
    return;
    }

  if ((PopFromDataStack(&ItemStr, TYPE_STRING | POINTER_TO) &
    ~POINTER_TO) != TYPE_STRING)
    {
    error(ERROR_BAD_PARAM_TYPE, 1);
    SetErrorFlag();
    return;
    }

  /* must be in correct menu before making choice */
  if (MostRecentMenuIndex != MenuFocus.ActiveMENU->MacMenuIndex)
    error(ERROR_INCORRECT_MODE, "MENU LEVEL");

  if (MostRecentMenuIndex == KSI_MAIN_MENU)
    {
    if (active_locus != LOCUS_MENUS)
      {
      // switch cursor type or call error
      if (active_locus == LOCUS_FORMS)
        {
        // if instead set status to EXIT_FORM,
        // get error, "BUFLEN" is unknown procedure
        // (ERROR_UNKNOWN_PROC)
        error(ERROR_INCORRECT_MODE, "FORMS");
        SetErrorFlag();
        return;
        }
      if (active_locus == LOCUS_APPLICATION)
        {
        active_locus = LOCUS_MENUS;
        /* delete the graphing cursor and restore the menu cursor */
        RemoveGraphCursor();
        }
      }
    MenuFocus.Row = 0;
    }
  else
    MenuFocus.Row = 1;

  active_locus = LOCUS_MENUS;

  MenuFocus.Column = 0;
  unhighlight_menuitem(MenuFocus.ItemIndex);

  MenuFocus.ItemIndex = (CHAR) FindActiveMenuItem(MenuFocus.ActiveMENU,
    ItemStr);
  if (MenuFocus.ItemIndex == -1)
    {
    error(ERROR_NOT_MENU_FORM_ITEM);
    SetErrorFlag();
    return;
    }

  highlight_menuitem(MenuFocus.ItemIndex);

  choose_menuitem();

  SysWait((ULONG) (PlayBackDelay * (FLOAT) 1000));    
}

void MacPlayForm()
{
  ERR_OMA err = ERROR_NONE;
  FORM *pForm;
//  BOOLEAN DrawFinished;
//  BOOLEAN repeat_shutdown;
  BOOLEAN Nested, Found;
  FORM_STACK *pTemp;
  SHORT i;
  DATA *FormData;
  void *TestForm;

  if (PopFromDataStack(&MostRecentFormIndex, TYPE_INTEGER) != TYPE_INTEGER)
    err = ERROR_MACRO_DEBUG;
  else if((MostRecentFormIndex > NUM_FORMS) || (MostRecentFormIndex <= 0))
    err = ERROR_MACRO_DEBUG;

  if (err)
    {
    error(err, "Form Playback");
    SetErrorFlag();
    return;
    }
  if (PopFromDataStack(&Nested, TYPE_BOOLEAN) != TYPE_BOOLEAN)
    {
    error(ERROR_BAD_PARAM_TYPE, 1);
    SetErrorFlag();
    return;
    }

  pForm = FormTable[MostRecentFormIndex];

  // check in case of a previous automatic exit from a form because of
  // no scroll entries, bad initialization, etc.
  pTemp = LastPlayedForm;
  if ((LastPlayedForm = malloc(sizeof(FORM_STACK))) == NULL)
    {
    error(ERROR_ALLOC_MEM);
    SetErrorFlag();
    return;
    }

  LastPlayedForm->PrevHistory = pTemp;
  LastPlayedForm->pForm = pForm;

  /* Check to see if current form */
  if (pForm == Current.Form)
    return;

  if (! Nested)
    {
    /* This section from run_form */
    if (! push_form_context()) {
      SetErrorFlag();
      return;
      }
    }
  else if (active_locus != LOCUS_FORMS) {

    // if instead, set current form status to EXIT_THIS_FORM,
    // get error, "BUFLEN" is unknown procedure
    // (ERROR_UNKNOWN_PROC)
    error(ERROR_NOT_IN_FORM);
    SetErrorFlag();
    return;
    }
  /* End of section from run_form */

  if (active_locus != LOCUS_FORMS)
    {
    if (active_locus == LOCUS_APPLICATION)
      {
      active_locus == LOCUS_FORMS;
      /* delete the graphing cursor and restore the menu cursor */
      RemoveGraphCursor();
      }

    if (err)
      {
      SetErrorFlag();
      return;
      }

    active_locus = LOCUS_FORMS;
    }

  if (! Nested)
    {
    /* This section from core_run_form */
    setup_for_form_entry(pForm, &default_form_attributes);
//    Current.Form->field_index = 0;
    }
  else
    {
    if (! (pForm->attrib & FORMATTR_INDEP))
      {
      Found = FALSE;
      /* find the initializing field */
      for (i=0; (i<Current.Form->number_of_fields) && !Found; i++)
        {
        if (Current.Form->fields[i].type == FLDTYP_FORM)
          {
          FormData = & Current.Form->dataRegistry
            [Current.Form->fields[i].primary_data.group]
            [Current.Form->fields[i].primary_data.item];
          TestForm = resolve_address(FormData);

          if (TestForm == pForm)
            {
            Found = TRUE;
            Current.Form->field_index = i;
            init_field();
            }
          }
        }

      if (! Found)
        {
        error(ERROR_NOT_MENU_FORM_ITEM);
        SetErrorFlag();
        return;
        }
      }

    if (setup_for_nested_form_entry(pForm, &Current.Form->attrib))
      run_D_O_field();
    }

//  // draw scrolls and do logic fields
//  DrawFinished = FALSE;
//  do
//    {
//    if (init_field())
//      {
//      // bypass this section if starting nested form         
//      if ((! Nested) || ! (Current.Form->attrib & FORMATTR_INDEP))
//        {
//        Current.Form->status = FORMSTAT_ACTIVE_FIELD;
//
//        if (Current.Field->attrib & FLDATTR_DISPLAY_ONLY)
//          run_D_O_field();
//        else if (Current.Field->type == FLDTYP_LOGIC)
//          run_LOGIC_field();
//        else if (Current.Field->type == FLDTYP_FORM)
//          {
//          if (setup_for_nested_form_entry(
//            ((FORM *) Current.FieldDataPtr), &Current.Form->attrib))
//            {
//            run_D_O_field();
//            }                        
//          exit_from_nested_form(); /* want to remain in form designated */
//          }                        /* by keystroke program */
//        else
//          DrawFinished = TRUE;
//        }        // end of Nested Bypass
//
//      Nested = FALSE;
//
//      do
//        {
//        repeat_shutdown = FALSE;
//
//        if (Current.Form->status == FORMSTAT_EXIT_THIS_FORM)
//          {
//          if (Current.Form->exit_function.group != 0)
//            {
//            /* call exit function */
//            if ((*Current.FormExitFunction)(0))
//              Current.Form->status = FORMSTAT_ACTIVE_FIELD;
//            /* if need to rerun form, set ACTIVE_FIELD */
//            }
//
//          if (Current.Form->attrib & FORMATTR_NESTED)
//            exit_from_nested_form(); /* performs pop_form_context() */
//          }
//        }
//      while (repeat_shutdown);
//      }
//    }
//  while ((Current.Form->status == FORMSTAT_ACTIVE_FIELD) && !DrawFinished);

  init_field();

  SysWait((ULONG) (PlayBackDelay * (FLOAT) 1000));    
}

void MacPlayField()
{
  ERR_OMA err = ERROR_NONE;
  SHORT i;

  if (PopFromDataStack(&MostRecentFieldIndex, TYPE_INTEGER) != TYPE_INTEGER)
    err = ERROR_MACRO_DEBUG;
  else if((MostRecentFieldIndex > NUM_FIELDS) || (MostRecentFieldIndex <= 0))
    err = ERROR_MACRO_DEBUG;
  if (err)
    {
    error(err, "Field Playback");
    SetErrorFlag();
    return;
    }

  for (i=0; i<Current.Form->number_of_fields; i++)
    {
    if (MostRecentFieldIndex == Current.Form->fields[i].MacFieldIndex)
      {
      Current.Form->field_index = i;
      break;
      }
    }

  if (i==Current.Form->number_of_fields)
    {
    error(ERROR_NOT_MENU_FORM_ITEM, "Field Playback");
    SetErrorFlag();
    return;
    }

  if ((PopFromDataStack(&PlayFieldStr, TYPE_STRING | POINTER_TO) &
    ~POINTER_TO) != TYPE_STRING)
    {
    error(ERROR_BAD_PARAM_TYPE, 1);
    SetErrorFlag();
    return;
    }

  PlayStrPos = 0;
  if (init_field())
    {
    Current.Form->status = FORMSTAT_ACTIVE_FIELD;

    if (Current.Field->attrib & FLDATTR_DISPLAY_ONLY)
      run_D_O_field();
    else if (Current.Field->type == FLDTYP_LOGIC)
      run_LOGIC_field();
    else if (Current.Field->type == FLDTYP_FORM)
      {
      if (setup_for_nested_form_entry(
        ((FORM *) Current.FieldDataPtr), &Current.Form->attrib))
        run_D_O_field();
      }
    else
      run_input_field();

    SysWait((ULONG) (PlayBackDelay * (FLOAT) 1000));
    }
  else
    SetErrorFlag();
}

// set the active focus on the scroll form row which is on the stack
// Only for scroll forms for now
void MacFocusOnField()
{
  ERR_OMA err = ERROR_NONE;
  SHORT FormFieldIndex;

  reset_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
  display_field_to_screen(FALSE);

  if (! PopScalarFromDataStack(&FormFieldIndex, TYPE_INTEGER))
    {
    error(ERROR_BAD_PARAM_TYPE, 1);
    SetErrorFlag();
    return;
    }

  MostRecentFieldIndex = Current.Form->fields[FormFieldIndex].MacFieldIndex;

  if (Current.Form->attrib & FORMATTR_SCROLLING) /* if scroll form */
    {
    int i;
    if (FormFieldIndex > MostRecentFieldIndex)   /* see if scroll down */
      {
      for (i= FormFieldIndex - MostRecentFieldIndex; i > 0; i--)
        if (!scroll_down_action())                /* Yes, if didn't work */
          break;
      FormFieldIndex -= i;                      /* undo it */
      }
    else
      {
      for (i= MostRecentFieldIndex - FormFieldIndex; i > 0; i--)
        if (!scroll_up_action())                 /* Y, if didn't work */
          break;
      FormFieldIndex += i;                     /* undo it */
      }
    }
  else
    set_scroll_form_index(Current.Form, FormFieldIndex);

  if ((*Current.FormInitFunction)(Current.Form->virtual_row_index))
    SetErrorFlag();
  else
    {
    set_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
    reset_bit(Current.Field->attrib, FLDATTR_LIMIT_WARN);
    format_and_display_field(TRUE);
    SysWait((ULONG) (PlayBackDelay * (FLOAT) 1000));
    }
}

void LeaveForm()
{
  PVOID pTemp;

  // check in case of an automatic exit from a form because of no scroll
  // entries, bad initialization, etc.
  if (Current.Form != LastPlayedForm->pForm)
    {
    if (LastPlayedForm == NULL)
      {
      error(ERROR_PLAY, "");
      SetErrorFlag();
      return;
      }

    pTemp = LastPlayedForm;
    LastPlayedForm = LastPlayedForm->PrevHistory;
    free(pTemp);
    return;
    }

  Current.Form->status = FORMSTAT_EXIT_THIS_FORM;
  if (init_field())
    {
    reset_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
    display_field_to_screen(FALSE);
    if (Current.Form->attrib & FORMATTR_NESTED)
      exit_from_nested_form(); /* performs pop_form_context() */
    else
      {

      if (Current.Form->exit_function.group != 0)
        {
        /* call exit function */
        if ((*Current.FormExitFunction)(0))
          {
          /* if need to rerun form, set ACTIVE_FIELD */
          Current.Form->status = FORMSTAT_ACTIVE_FIELD;
          SetErrorFlag();
          return;
          }
        }
      shutdown_form();
      /* return to the previous form */
      pop_form_context();
      }
    }
  if(isFormMenu1(Current.Form))
    active_locus = LOCUS_MENUS;
  else if (isFormGraphWindow(Current.Form))
    active_locus = LOCUS_APPLICATION;

  if (active_locus == LOCUS_FORMS)
    init_field();

  /* pop new pointer to last activated form */
  if (LastPlayedForm != NULL)
    {
    pTemp = LastPlayedForm;
    LastPlayedForm = LastPlayedForm->PrevHistory;
    free(pTemp);
    }

  /* pop index of last compiled form */
  if (LastForm != NULL)
    {
    pTemp = LastForm->PrevHistory;
    free(LastForm);
    LastForm = pTemp;
    }

  SysWait((ULONG) (PlayBackDelay * (FLOAT) 1000));    
}

void LeaveMenu()
{
  previous_menu();
  SysWait((ULONG) (PlayBackDelay * (FLOAT) 1000));    
}

// Open up the files containing the form, menu and field names and indices
// for keystroke playback or to run macro

BOOLEAN OpenFormRefFiles(void)
{
  if ((hMacFormFile = fopen(base_path(FORM_FILE), "rb")) == NULL)
    {
    error(ERROR_OPEN, base_path(FORM_FILE));
    return TRUE;
    }

  if ((hMacFieldFile = fopen(base_path(FIELD_FILE), "rb")) == NULL)
    {
    fclose(hMacFormFile);
    hMacFormFile = NULL;
    error(ERROR_OPEN, base_path(FIELD_FILE));
    return TRUE;
    }
  _heapmin();

  return FALSE;
}

void CloseFormRefFiles(void)
{
  if (hMacFormFile != NULL)
    {
    fclose(hMacFormFile);
    hMacFormFile = NULL;
    }
  if (hMacFieldFile != NULL)
    {
    fclose(hMacFieldFile);
    hMacFieldFile = NULL;
    }
}

int StartKeyStroke(void)
{
  CHAR Buf[FNAME_LENGTH];

  /* check for good file name */
  if (ParseFileName(Buf, MacRecordFileName) != 2)
    {
    error(ERROR_BAD_FILENAME, Buf);
    return FIELD_VALIDATE_WARNING;
    }

  strcpy(MacRecordFileName, Buf);

  /* check to see if file of this name is present */
  hMacRecordFile = fopen(Buf, "r");
  if (KeyStrokeMode == MODE_RECORD)
    {
    if (hMacRecordFile != 0)
      {
      if (yes_no_choice_window(FileOverwritePrompt, 0,
        COLORS_MESSAGE) != YES)
        {
        fclose(hMacRecordFile);
        return FIELD_VALIDATE_SUCCESS;
        }

      fclose(hMacRecordFile);
      }

    if ((hMacRecordFile = fopen(MacRecordFileName, "w+")) == NULL)
      {
      error(ERROR_OPEN, MacRecordFileName);
      return FIELD_VALIDATE_WARNING;
      }

    if (OpenFormRefFiles())
      {
      fclose(hMacRecordFile);
      return FIELD_VALIDATE_WARNING;
      }
    }
  else
    {
    if (hMacRecordFile == 0)
      {
      error(ERROR_OPEN, MacRecordFileName);
      return FIELD_VALIDATE_WARNING;
      }
    fclose(hMacRecordFile);
    }

  if (KeyStrokeMode == MODE_RECORD)
    {
    Current.Form->status = FORMSTAT_EXIT_THIS_FORM;
    MacRecord = -1;   // -1 shows the start of record so that 'Go' is not
    // recorded.  Will be changed to TRUE by forms.c
    }
  else
    {
    MacPlayBack = -1;
    Current.Form->status = FORMSTAT_SWITCH_MODE;
    }

  FKeyItems[32].Control &= ~ MENUITEM_INACTIVE;

  return FIELD_VALIDATE_SUCCESS;
}

void StopKeyStroke(USHORT Dummy)        
{
  CloseFormRefFiles();
  fclose(hMacRecordFile);

  if (MacPlayBack)              // stop execution
    MacroStatus = PARSE_ERROR;

  MacRecord = 0;
  MacPlayBack = 0;
  FKeyItems[32].Control |= MENUITEM_INACTIVE;

  Dummy;
}

PRIVATE SHORT LookupString(FILE *hFile, ULONG StartOffset, SHORT MaxEntries,
                            PCHAR String)
{
  CHAR Buf[2][FORM_NAME_SIZE];
  BOOLEAN Found = FALSE;
  SHORT i;

  memset(Buf[0], 0, FORM_NAME_SIZE);
  strncpy(Buf[0], String, FORM_NAME_SIZE);
  /* go to start of menu names */
  if (fseek(hFile, StartOffset, SEEK_SET))
    {
    error(ERROR_SEEK, base_path(FORM_FILE));
    return INVALID_INDEX;
    }

  for (i = 0; (i < MaxEntries) && (! Found); i++)
    {
    if (fread(Buf[1], FORM_NAME_SIZE, 1, hFile) != 1)
      {
      error(ERROR_READ, base_path(FORM_FILE));
      return INVALID_INDEX;
      }
    if (! strncmp(Buf[0], Buf[1], FORM_NAME_SIZE))
      Found = TRUE;
    }

  if (! Found)
    return INVALID_INDEX;
  else return i;
}

SHORT LookupMenu(PCHAR MenuStr, PSHORT FldIndex)
{
   *FldIndex = LookupString(hMacFormFile,
                            (ULONG) FORM_NAME_SIZE * (ULONG) NUM_FORMS,
                            NUM_MENUS, MenuStr) + NUM_FORMS;
   return MacMenuSymbol;
}

SHORT LookupForm(PCHAR FormStr, PSHORT FldIndex)
{
  FORM_HISTORY *pTemp;

  *FldIndex = LookupString(hMacFormFile, 0L, NUM_FORMS, FormStr) -1;
  if (*FldIndex != INVALID_INDEX)
    {
    pTemp = LastForm;
    LastForm = malloc(sizeof(FORM_HISTORY));
    if (LastForm == NULL)
      {
      error(ERROR_ALLOC_MEM, "");
      return INVALID_INDEX;
      }

    LastForm->PrevHistory = pTemp;
    LastForm->CurForm = *FldIndex;
    }

  return MacFormSymbol;
}

SHORT LookupField(PCHAR FieldStr, SHORT FormIndex, PSHORT FldIndex) 
{
  CHAR Buf[FORM_NAME_SIZE];
  BOUNDS FormFieldRange;
  ULONG Offset;
  BOOLEAN FirstTry = TRUE;
  BOOLEAN Found = FALSE;

  if (FormIndex == 0)
    FormIndex = 1;

  do
    {
    /* first try the current form range information to decrease the number */
    /* of field names to search                                            */
    Offset = ((FormIndex) * sizeof(BOUNDS)) + sizeof(int);
    if (fseek(hMacFieldFile, Offset, SEEK_SET))
      {
      error(ERROR_SEEK, base_path(FIELD_FILE));
      return INVALID_INDEX;
      }
      
    /* read upper and lower bound of field indices for form */
    if (fread(&FormFieldRange, sizeof(BOUNDS), 1, hMacFieldFile) != 1)
      {
      error(ERROR_READ, base_path(FIELD_FILE));
      return INVALID_INDEX;
      }

    *FldIndex = FormFieldRange.Lower;
    Offset = ((*FldIndex - 1) * FORM_NAME_SIZE) +
      (NUM_FORMS * sizeof(BOUNDS)) + sizeof(int);
    if (fseek(hMacFieldFile, Offset, SEEK_SET))
      {
      error(ERROR_SEEK, base_path(FIELD_FILE));
      return INVALID_INDEX;
      }

    /* look for it in current block */
    do
      {
      if (fread(Buf, FORM_NAME_SIZE, 1, hMacFieldFile) != 1)
        {
        error(ERROR_READ, base_path(FIELD_FILE));
        return INVALID_INDEX;
        }
      if (! strncmp(FieldStr, Buf, FORM_NAME_SIZE))
        Found = TRUE;
      if (!Found)
        (*FldIndex)++;
      }
    while ((*FldIndex <= FormFieldRange.Higher) && (! Found));

    /* check to see if need to look in all other blocks */
    if (! Found)
      {
      if (FirstTry)
        {
        FormIndex = 1;     // go back to first form block
        FirstTry = FALSE;
        }
      else
        FormIndex++;
      }
    }
  while ((FormIndex <= NUM_FORMS) && !Found);

  if (Found)
    {
    return MacFieldSymbol;
    }
  else
    {
    *FldIndex = INVALID_INDEX;
    return INVALID_INDEX;
    }
}

// free up the allocated form history chain            
PRIVATE void FreeFormHistory(void)
{
  PVOID pTemp;

  while (LastForm != NULL)
    {
    pTemp = LastForm->PrevHistory;
    free(LastForm);
    LastForm = pTemp;
    }
  while (LastPlayedForm != NULL)
    {
    pTemp = LastPlayedForm->PrevHistory;
    free(LastPlayedForm);
    LastPlayedForm = pTemp;
    }

}

SHORT RunKeyStrokePlay()
{
  SHORT i;
  SHORT ReturnKey = 0;

  LastForm = malloc(sizeof(FORM_HISTORY));
  if (LastForm == NULL)
    {
    error(ERROR_ALLOC_MEM);
    return ReturnKey;
    }

  LastForm->PrevHistory = NULL;
  LastForm->CurForm = 0;

  LastPlayedForm = NULL;

  MacPlayBack = TRUE;
  Current.Form->status = FORMSTAT_ACTIVE_FIELD;

  if (EnterMacroForm())
    {
    InitDataStack();

    /* find the menu and form field symbols */
    for (i=NumberOfSymbolEntries-1; i >= 0; i--)
      {
      if (strcmp(SymbolTable[i].Name, "MENU_REF") == 0)
        MacMenuSymbol = i;
      if (strcmp(SymbolTable[i].Name, "FORM_REF") == 0)
        MacFormSymbol = i;
      if (strcmp(SymbolTable[i].Name, "FIELD_REF") == 0)
        MacFieldSymbol = i;
      }

      /* in macskel.c (macparse.c) */
    ReadAndParseSourceFile(MacRecordFileName);
    if (MacroStatus == PARSE_COMPLETE)
      RunMainProgram();    /* program executed here */

    /* make sure that execution does not resume after this!! */
    IP[1].Class = CLASS_EXECUTE_RETURN;
    IP[2].Class = CLASS_EXECUTE_RETURN;

    if (Current.Form->status != FORMSTAT_EXIT_ALL_FORMS)
      ReturnKey = -1;
    LeaveMacroForm();
    }

  FreeFormHistory();
  MacPlayBack = FALSE;

  /* turn off stop play back */
  FKeyItems[32].Control |= MENUITEM_INACTIVE;

  ShowFKeys(&FKey);

  return ReturnKey;
}


void ShowLocalInfo(USHORT Dummy)       
{
  WINDOW *    MessageWindow;
  CHAR HelpString[80];
  CHAR FormString[80];
  CHAR FieldString[160];
  PCHAR MessageStrings[4];
  CHAR Buf[80];
  ERR_OMA err;
  BOOLEAN LocalOpen = FALSE;
  PCHAR LocalString;

  switch (active_locus)
    {
    case LOCUS_FORMS:
      if (hMacFormFile == NULL)
        {
        if ((hMacFormFile = fopen(base_path(FORM_FILE), "rb")) == NULL)
          {
          error(ERROR_OPEN, base_path(FORM_FILE));
          return;
          }
        if ((hMacFieldFile = fopen(base_path(FIELD_FILE), "rb")) == NULL)
          {
          CloseFormRefFiles();
          error(ERROR_OPEN, base_path(FIELD_FILE));
          return;
          }
        LocalOpen = TRUE;
        }

      sprintf(HelpString, "Help index: %d", Current.Field->help_index);
      if ((err = ReadFormString(Current.Form->MacFormIndex, Buf)) !=
        ERROR_NONE)
        {
        error(err, base_path(FORM_FILE));
        return;
        }

      if (Current.Form->attrib & FORMATTR_NESTED)
        sprintf(FormString, "Form call: %s(TRUE);", Buf);
      else
        sprintf(FormString, "Form call: %s(FALSE);", Buf);
      FormString[79] = '\0';       // make sure that it's not too long

      LocalString = Current.FieldDisplayString;
      if ((err = ReadFieldString(Current.Field->MacFieldIndex, Buf,
        &LocalString)) != ERROR_NONE)
        {
        if (LocalOpen)
          CloseFormRefFiles();
        error(err, base_path(FIELD_FILE));
        return;
        }
      sprintf(FieldString, "Field call: %s('%s');", Buf, LocalString);
      FieldString[79] = '\0';       // make sure that it's not too long

      MessageStrings[0] = HelpString;
      MessageStrings[1] = FormString;
      MessageStrings[2] = FieldString;
      MessageStrings[3] = NULL;

      if (LocalOpen)
        CloseFormRefFiles();
    break;

    case LOCUS_MENUS:
      if (hMacFormFile == NULL)  // check to see if files already opened
        {
        if ((hMacFormFile = fopen(base_path(FORM_FILE), "rb")) == NULL)
          {
          error(ERROR_OPEN, base_path(FORM_FILE));
          return;
          }
        LocalOpen = TRUE;
        }

      sprintf(HelpString, "Help index: %d",
        MenuFocus.ActiveMENU->ItemList[MenuFocus.ItemIndex].help_index);
      if ((err = ReadMenuString(
        MenuFocus.ActiveMENU->MacMenuIndex, Buf)) != ERROR_NONE)
        {
        if (LocalOpen)
          CloseFormRefFiles();
        error(err, base_path(FORM_FILE));
        return;
        }
      sprintf(FieldString, "Menu item call: %s('%s');\n", Buf,
        MenuFocus.ActiveMENU->ItemList[MenuFocus.ItemIndex].Text);
      FormString[79] = '\0';       // make sure that it's not too long

      MessageStrings[0] = HelpString;
      MessageStrings[1] = FieldString;
      MessageStrings[2] = NULL;

      if (LocalOpen)
        CloseFormRefFiles();

    break;

    case LOCUS_APPLICATION:
      sprintf(HelpString, "Help index: %d", GRAPH_HBASE);
      MessageStrings[0] = HelpString;
      MessageStrings[1] = "Access string: ENTER_GRAPH();";
      MessageStrings[2] = NULL;
    break;
    }

  put_up_message_window(MessageStrings, COLORS_MESSAGE, &MessageWindow);
  if (MessageWindow != NULL)
    {
    message_pause();
    release_message_window(MessageWindow);
    }
}

// connect the forms system to keystroke recording
// ------------------------------------------------------------------------
void init_keystroke_fields(void)
{
  KSActivateFieldRecord  = MacActivatedField ;  
  KSFieldFocusRecord     = MacFocusedOnField ;        
  KSActivateFormRecord   = MacActivatedForm  ;  
  KSDeActivateFormRecord = MacDeActivatedForm;  
  KSActivateMenuRecord   = MacActivatedMenu  ;  
  KSDeActivateMenuRecord = MacDeActivatedMenu;

  pKSRecord           = & MacRecord    ;
  pKSPlayBack         = & MacPlayBack  ;
  ppKSPlayFieldString = & PlayFieldStr ;       
  pKSCharPosition     = & PlayStrPos   ;           
  pPlayBackDelay      = & PlayBackDelay;          
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN isKeyStrokeRecordOn(void)
{
   return pKSRecord && * pKSRecord;
}
