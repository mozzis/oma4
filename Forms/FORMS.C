/* -----------------------------------------------------------------------
/
/  forms.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  This is a collection of routines to implement a forms package.
/  This module is the central core; support for various types of
/  fields and scrolling forms are in other modules.
/
/
*/ /*
  $Header:   J:/logfiles/forms/forms.c_v   1.12   07 Jan 1992 12:24:26   maynard  $
  $Log:   J:/logfiles/forms/forms.c_v  $
*
*/ /*
/ ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <dos.h>

#include "syserror.h"
#include "ksindex.h"
#include "omaform.h"
#include "forms.h"

#ifdef XSTAT
  #define PRIVATE
#else
  #define PRIVATE static
#endif

void erase_form(void);
void draw_form(void);
void run_LOGIC_field();
BOOLEAN  (*data_limit_action)(void);
void string_to_field();
SHORT string_from_field();
void (*ExitRestoreFunc)(void) = NULL;

/* -----------------------------------------------------------------------
/  purpose: saves position and other information for a field that has
/     been "zoomed".  This is used to restore the field to its original
/     size and position when it is "un-zoomed".
/ ----------------------------------------------------------------------- */

struct {
   USHORT attrib;
   UCHAR  row;
   UCHAR  column;
   UCHAR  width;
   UCHAR  form_column;
   UCHAR  actual_row;
   UCHAR  actual_column;
   SaveAreaInfo * screen;
} ZoomSave;

/* -----------------------------------------------------------------------
/  purpose:
/ ----------------------------------------------------------------------- */

BOOLEAN NoAutoPlay;

SHORT   active_locus = LOCUS_UNKNOWN;
BOOLEAN mouse_is_useable = FALSE;

/* -----------------------------------------------------------------------
/ purpose: take care of implementation specific keys
/ ----------------------------------------------------------------------- */

UCHAR (*UserKeyHandler)(UCHAR key) = NULL;
KEY_IDLE_CALLS *keyboard_idle = NULL;

/* -----------------------------------------------------------------------
/ purpose: implementation specific error handler
/ ----------------------------------------------------------------------- */

ERROR_CATEGORY (*error_handler)(ERROR_CATEGORY err, ...) = NULL;
void (*audible_error_handler)(void) = NULL;

/* -----------------------------------------------------------------------
/ purpose: Keystroke record and playback functions
/ ----------------------------------------------------------------------- */

void (*KSActivateFieldRecord)(SHORT GlobalFieldIndex, PCHAR DisplayString) = NULL;
void (*KSFieldFocusRecord)(SHORT FieldIndex) = NULL;
void (*KSActivateFormRecord)(SHORT FormIndex, BOOLEAN Nested) = NULL;
void (*KSDeActivateFormRecord)(void) = NULL;
void (*KSActivateMenuRecord)(SHORT MenuIndex, SHORT ItemIndex) = NULL;
void (*KSDeActivateMenuRecord)(void) = NULL;
void (*KSPlayNextMenuItem)(void) = NULL;
BOOLEAN *pKSRecord = NULL;
BOOLEAN *pKSPlayBack = NULL;
PCHAR * ppKSPlayFieldString = NULL;
PSHORT  pKSCharPosition = NULL;
PFLOAT  pPlayBackDelay = NULL;

/* -----------------------------------------------------------------------
/  function:   Does nothing.  Used as a placeholder for function pointers.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void noop(void)
{
}

SHORT int_noop(void)
{
   return(0);
}

BOOLEAN char_noop(UCHAR key)
{
   return(key ? FALSE : FALSE);
}

BOOLEAN limit_noop(void)
{
   return (FALSE);
}

FIELD_CLASS    FieldClassArray[16] = {
   { 0, 0, 0, 0, 0 },                     /* no field type of zero */
   { FTB_STRING,  default_char_action,
   string_to_field, string_from_field, NULL },
   { 0, char_noop, noop, int_noop, limit_noop },        /* filled in for each type */
   { 0, char_noop, noop, int_noop, limit_noop },        /* linked in, at runtime */
   { 0, char_noop, noop, int_noop, limit_noop },
   { 0, char_noop, noop, int_noop, limit_noop },
   { 0, char_noop, noop, int_noop, limit_noop },
   { 0, char_noop, noop, int_noop, limit_noop },
   { 0, char_noop, noop, int_noop, limit_noop },
   { 0, char_noop, noop, int_noop, limit_noop },
   { 0, char_noop, noop, int_noop, limit_noop },
   { 0, char_noop, noop, int_noop, limit_noop },
   { 0, char_noop, noop, int_noop, limit_noop },
   { 0, char_noop, noop, int_noop, limit_noop },
   { 0, char_noop, noop, int_noop, limit_noop },
   { 0, char_noop, noop, int_noop, limit_noop }
};

/* -----------------------------------------------------------------------
/  purpose: holds all transient information about the current
/     form and field context.  See forms.h for definitions.
/
/ ----------------------------------------------------------------------- */

FORM_CONTEXT   Current = {
  NULL, // FIELD *        Field;
  NULL, // FIELD_CLASS *  FieldClass;
  NULL, // DATA *         FieldData;
  NULL, // void *         FieldDataPtr;
  NULL, // DATA *         FieldAltData;
  NULL, // void *         FieldAltDataPtr;
  "\0", // char           FieldString[MAX_FIELD_SIZE + (MAX_FIELD_SIZE / 2) + 1];
  "\0", // char           FieldDisplayString[MAX_FIELD_SIZE + 1];
  NULL, // int            (*FieldVerifyFunction)(void *, char *);
  NULL, // BOOLEAN        (*FormInitFunction)(int);
  NULL, // BOOLEAN        (*FormExitFunction)(int);
  NULL, // COLOR_SET *    FormColorSet;
  NULL, // FORM *         Form;
  NULL, // SaveAreaInfo *;
  NULL  // FORM_CONTEXT * PreviousStackedContext;
};

/* -----------------------------------------------------------------------
/  purpose: This array is used as a character filter.  The ASCII
/     value of the character is used as an index into the array.
/     If the field type bit is set at the array position, the
/     character is passed on to the character handling routine
/     for the current field type.  If the bit is not set, the
/     character is ignored, as if it were never typed.
/     (note: FTBS_GENERAL_NUMERIC sets bits for all the numeric
/      field types to save space here.)
/ ----------------------------------------------------------------------- */

SHORT FormsCTypeCheck[256] = {
   /* NUL */ 0,
   /* SOH */ 0,
   /* STX */ 0,
   /* ETX */ 0,
   /* EOT */ 0,
   /* ENQ */ 0,
   /* ACK */ 0,
   /* BEL */ 0,
   /* BS  */ 0, /* filtered out before using this table */
   /* HT  */ 0, /* filtered out before using this table */
   /* LF  */ 0,
   /* VT  */ 0,
   /* FF  */ 0,
   /* CR  */ 0, /* filtered out before using this table */
   /* SO  */ 0,
   /* SI  */ 0,
   /* DLE */ 0,
   /* DC1 */ 0,
   /* DC2 */ 0,
   /* DC3 */ 0,
   /* DC4 */ 0,
   /* NAK */ 0,
   /* SYN */ 0,
   /* ETB */ 0,
   /* CAN */ 0,
   /* EM  */ 0,
   /* SUB */ 0,
   /* ESC */ 0, /* filtered out before using this table */
   /* FS  */ 0,
   /* GS  */ 0,
   /* RS  */ 0,
   /* US  */ 0,
   /* SP  */ (FTB_STRING | FTB_TOGGLE),
   /* !   */ (FTB_STRING),
   /* "   */ (FTB_STRING),
   /* #   */ (FTB_STRING),
   /* $   */ (FTB_STRING),
   /* %   */ (FTB_STRING),
   /* &   */ (FTB_STRING),
   /* '   */ (FTB_STRING),
   /* (   */ (FTB_STRING),
   /* )   */ (FTB_STRING),
   /* *   */ (FTB_STRING | FTB_TOGGLE),
   /* +   */ (FTB_STRING | FTB_STD_FLT | FTB_SCL_FLT | FTB_INT | FTB_TOGGLE),
   /* ,   */ (FTB_STRING),
   /* -   */ (FTB_STRING | FTB_STD_FLT | FTB_SCL_FLT | FTB_INT| FTB_TOGGLE),
   /* .   */ (FTB_STRING | FTB_STD_FLT | FTB_SCL_FLT),
   /* /   */ (FTB_STRING | FTB_TOGGLE),
   /* 0   */ (FTB_STRING | FTB_TOGGLE | FTBS_GENERAL_NUMERIC),
   /* 1   */ (FTB_STRING | FTB_TOGGLE | FTBS_GENERAL_NUMERIC),
   /* 2   */ (FTB_STRING | FTB_TOGGLE | FTBS_GENERAL_NUMERIC),
   /* 3   */ (FTB_STRING | FTB_TOGGLE | FTBS_GENERAL_NUMERIC),
   /* 4   */ (FTB_STRING | FTB_TOGGLE | FTBS_GENERAL_NUMERIC),
   /* 5   */ (FTB_STRING | FTB_TOGGLE | FTBS_GENERAL_NUMERIC),
   /* 6   */ (FTB_STRING | FTB_TOGGLE | FTBS_GENERAL_NUMERIC),
   /* 7   */ (FTB_STRING | FTB_TOGGLE | FTBS_GENERAL_NUMERIC),
   /* 8   */ (FTB_STRING | FTB_TOGGLE | FTBS_GENERAL_NUMERIC),
   /* 9   */ (FTB_STRING | FTB_TOGGLE | FTBS_GENERAL_NUMERIC),
   /* :   */ (FTB_STRING),
   /* ;   */ (FTB_STRING),
   /* <   */ (FTB_STRING | FTB_TOGGLE),
   /* =   */ (FTB_STRING | FTB_TOGGLE),
   /* >   */ (FTB_STRING | FTB_TOGGLE),
   /* ?   */ (FTB_STRING),
   /* @   */ (FTB_STRING),
   /* A   */ (FTB_STRING | FTB_HEX_INT | FTB_TOGGLE),
   /* B   */ (FTB_STRING | FTB_HEX_INT | FTB_TOGGLE),
   /* C   */ (FTB_STRING | FTB_HEX_INT | FTB_TOGGLE),
   /* D   */ (FTB_STRING | FTB_HEX_INT | FTB_TOGGLE),
   /* E   */ (FTB_STRING | FTB_HEX_INT | FTB_TOGGLE | FTB_STD_FLT | FTB_SCL_FLT),
   /* F   */ (FTB_STRING | FTB_HEX_INT | FTB_TOGGLE),
   /* G   */ (FTB_STRING | FTB_SCL_FLT | FTB_TOGGLE),
   /* H   */ (FTB_STRING | FTB_TOGGLE),
   /* I   */ (FTB_STRING | FTB_TOGGLE),
   /* J   */ (FTB_STRING | FTB_TOGGLE),
   /* K   */ (FTB_STRING | FTB_SCL_FLT | FTB_TOGGLE),
   /* L   */ (FTB_STRING | FTB_TOGGLE),
   /* M   */ (FTB_STRING | FTB_SCL_FLT | FTB_TOGGLE),
   /* N   */ (FTB_STRING | FTB_TOGGLE),
   /* O   */ (FTB_STRING | FTB_TOGGLE),
   /* P   */ (FTB_STRING | FTB_TOGGLE),
   /* Q   */ (FTB_STRING | FTB_TOGGLE),
   /* R   */ (FTB_STRING | FTB_TOGGLE),
   /* S   */ (FTB_STRING | FTB_TOGGLE),
   /* T   */ (FTB_STRING | FTB_TOGGLE),
   /* U   */ (FTB_STRING | FTB_TOGGLE),
   /* V   */ (FTB_STRING | FTB_TOGGLE),
   /* W   */ (FTB_STRING | FTB_TOGGLE),
   /* X   */ (FTB_STRING | FTB_TOGGLE),
   /* Y   */ (FTB_STRING | FTB_TOGGLE),
   /* Z   */ (FTB_STRING | FTB_TOGGLE),
   /* [   */ (FTB_STRING),
   /* \   */ (FTB_STRING),
   /* ]   */ (FTB_STRING),
   /* ^   */ (FTB_STRING),
   /* _   */ (FTB_STRING),
   /* `   */ (FTB_STRING),
   /* a   */ (FTB_STRING | FTB_HEX_INT | FTB_TOGGLE),
   /* b   */ (FTB_STRING | FTB_HEX_INT | FTB_TOGGLE),
   /* c   */ (FTB_STRING | FTB_HEX_INT | FTB_TOGGLE),
   /* d   */ (FTB_STRING | FTB_HEX_INT | FTB_TOGGLE),
   /* e   */ (FTB_STRING | FTB_HEX_INT | FTB_STD_FLT | FTB_SCL_FLT | FTB_TOGGLE),
   /* f   */ (FTB_STRING | FTB_HEX_INT | FTB_TOGGLE),
   /* g   */ (FTB_STRING | FTB_TOGGLE),
   /* h   */ (FTB_STRING | FTB_TOGGLE),
   /* i   */ (FTB_STRING | FTB_TOGGLE),
   /* j   */ (FTB_STRING | FTB_TOGGLE),
   /* k   */ (FTB_STRING | FTB_TOGGLE),
   /* l   */ (FTB_STRING | FTB_TOGGLE),
   /* m   */ (FTB_STRING | FTB_SCL_FLT | FTB_TOGGLE),
   /* n   */ (FTB_STRING | FTB_SCL_FLT | FTB_TOGGLE),
   /* o   */ (FTB_STRING | FTB_TOGGLE),
   /* p   */ (FTB_STRING | FTB_SCL_FLT | FTB_TOGGLE),
   /* q   */ (FTB_STRING | FTB_TOGGLE),
   /* r   */ (FTB_STRING | FTB_TOGGLE),
   /* s   */ (FTB_STRING | FTB_TOGGLE),
   /* t   */ (FTB_STRING | FTB_TOGGLE),
   /* u   */ (FTB_STRING | FTB_SCL_FLT | FTB_TOGGLE),
   /* v   */ (FTB_STRING | FTB_TOGGLE),
   /* w   */ (FTB_STRING | FTB_TOGGLE),
   /* x   */ (FTB_STRING | FTB_TOGGLE),
   /* y   */ (FTB_STRING | FTB_TOGGLE),
   /* z   */ (FTB_STRING | FTB_TOGGLE),
   /* {   */ (FTB_STRING),
   /* |   */ (FTB_STRING),
   /* }   */ (FTB_STRING),
   /* ~   */ (FTB_STRING | FTB_TOGGLE),
   /* DEL */ 0, /* filtered out before using this table */
   /* Any after here have their high bit set */
   /* KEY_ENTER      */ FTB_SELECT,
   /* KEY_ESCAPE     */ 0,
   /* KEY_TAB        */ 0,
   /* KEY_BACK_TAB   */ 0,
   /* KEY_BACKSPACE  */ (FTB_STRING | FTBS_GENERAL_NUMERIC | FTB_TOGGLE | FTB_SELECT),
   /* KEY_DELETE     */ (FTB_STRING | FTBS_GENERAL_NUMERIC | FTB_SELECT),
   /* KEY_DELETE_FAR */ (FTB_STRING | FTBS_GENERAL_NUMERIC | FTB_TOGGLE),
   /* KEY_INSERT     */ 0,
   /* KEY_UP         */ 0,
   /* KEY_UP_FAR     */ 0,
   /* KEY_DOWN       */ 0,
   /* KEY_DOWN_FAR   */ 0,
   /* KEY_LEFT       */ (FTB_STRING | FTBS_GENERAL_NUMERIC | FTB_TOGGLE | FTB_SELECT),
   /* KEY_LEFT_FAR   */ 0,
   /* KEY_RIGHT      */ (FTB_STRING | FTBS_GENERAL_NUMERIC | FTB_TOGGLE | FTB_SELECT),
   /* KEY_RIGHT_FAR  */ 0,
   /* KEY_HOME       */ (FTB_STRING | FTBS_GENERAL_NUMERIC | FTB_TOGGLE | FTB_SELECT),
   /* KEY_HOME_FAR   */ 0,
   /* KEY_END        */ (FTB_STRING | FTBS_GENERAL_NUMERIC | FTB_TOGGLE | FTB_SELECT),
   /* KEY_END_FAR    */ 0,
   /* KEY_PG_UP      */ 0,
   /* KEY_PG_UP_FAR  */ 0,
   /* KEY_PG_DN      */ 0,
   /* KEY_PG_DN_FAR  */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /* KEY_F1         */ 0,
   /* KEY_F2         */ 0,
   /* KEY_F3         */ 0,
   /* KEY_F4         */ 0,
   /* KEY_F5         */ 0,
   /* KEY_F6         */ 0,
   /* KEY_F7         */ 0,
   /* KEY_F8         */ 0,
   /* KEY_F9         */ 0,
   /* KEY_F10        */ 0,
   /* KEY_F11        */ 0,
   /* KEY_F12        */ 0,
   /* KEY_F13        */ 0,
   /* KEY_F14        */ 0,
   /* KEY_F15        */ 0,
   /* KEY_F16        */ 0,
   /* KEY_F17        */ 0,
   /* KEY_F18        */ 0,
   /* KEY_F19        */ 0,
   /* KEY_F20        */ 0,
   /* KEY_F21        */ 0,
   /* KEY_F22        */ 0,
   /* KEY_F23        */ 0,
   /* KEY_F24        */ 0,
   /* KEY_F25        */ FTB_TOGGLE,
   /* KEY_F26        */ 0,
   /* KEY_F27        */ 0,
   /* KEY_F28        */ 0,
   /* KEY_F29        */ 0,
   /* KEY_F30        */ 0,
   /* KEY_F31        */ 0,
   /* KEY_F32        */ 0,
   /* KEY_F34        */ 0,
   /* KEY_F35        */ 0,
   /* KEY_F36        */ 0,
   /* KEY_F37        */ 0,
   /* KEY_F38        */ 0,
   /* KEY_F39        */ 0,
   /* KEY_F40        */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
   /*                */ 0,
};

/* -----------------------------------------------------------------------
/  function:   called at initialization time by a field type
/              which is not included in the character filter table
/              at compile-time.  Sets the type bit for the field
/              at each legal character for the field.  Legal
/              characters are taken from the string supplied
/              by the caller.
/  requires:   (USHORT) type_bit - the type bit representing
/              the field type
/              (UCHAR *) legal_char_set - a string of
/              characters that this field type will accept
/  returns:    (void)
/  side effects:  modifies the FormsCTypeCheck[] table
/
/ ----------------------------------------------------------------------- */

void legalize_chars_for_field(USHORT type_bit,
UCHAR * legal_char_set)
{
  USHORT index;

  while ((index = (USHORT) *legal_char_set++) != 0)
    FormsCTypeCheck[index] |= type_bit;
}

/* -----------------------------------------------------------------------
/  function:   All fields are displayed by this one function.
/              It calculates the field's real position on the
/              screen, picks the approprate color scheme, displays
/              the text, adds the optional overflow character,
/              and places the cursor if the caller requests it.
/              Four display primitives are called:
/                 set_attributes()
/                 display_string()
/                 emit()
/                 set_cursor()
/  requires:   (BOOLEAN) cursor_on - if TRUE, display cursor
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
void display_field_to_screen(BOOLEAN cursor_on)
{
  SHORT  row;
  SHORT  column;
  char   overflow_indicator;
  UCHAR  attribute;

  row = (SHORT) (Current.Form->row + Current.Field->row +
    Current.Form->display_row_offset);
  column = (SHORT) (Current.Form->column + Current.Field->column);

  if (Current.Field->attrib & FLDATTR_LIMIT_WARN)
    {
    attribute = set_attributes(Current.FormColorSet->error.foreground,
      Current.FormColorSet->error.background);
    }
  else if (Current.Field->attrib & FLDATTR_HIGHLIGHT)
    {
    attribute = set_attributes(Current.FormColorSet->highlight.foreground,
      Current.FormColorSet->highlight.background);
    }
  else if (Current.Field->attrib & FLDATTR_REV_VID)
    {
    attribute = set_attributes(Current.FormColorSet->reverse.foreground,
      Current.FormColorSet->reverse.background);
    }
  else if (Current.Field->attrib & FLDATTR_SHADED)
    {
    attribute = set_attributes(Current.FormColorSet->shaded.foreground,
      Current.FormColorSet->shaded.background);
    }
  else
    {
    attribute = set_attributes(Current.FormColorSet->regular.foreground,
      Current.FormColorSet->regular.background);
    }

  display_string(Current.FieldDisplayString, Current.Field->width,
    row, column, attribute);

  if(!((Current.Field->attrib & FLDATTR_NO_OVERFLOW_CHAR) ||
     (Current.Field->attrib & FLDATTR_DISPLAY_ONLY)))
    {
    if (Current.Field->type == FLDTYP_TOGGLE)
      show_toggle(row, (column + Current.Field->width), attribute);
    else
      {
      if (Current.Form->field_overfull_flag)
        {
        if (Current.Form->string_cursor_offset == 0)
          overflow_indicator = OVERFLOW_AT_START;
        else if (Current.Form->string_cursor_offset
          == Current.Form->field_char_count)
          overflow_indicator = OVERFLOW_AT_END;
        else
          overflow_indicator = OVERFLOW_IN_MIDDLE;
        }
      else
        overflow_indicator = OVERFLOW_NONE;

      show_overflow(overflow_indicator, row, (column + Current.Field->width),
        attribute);
      }
    }
}

/***********************************************************************/
/*                                                                     */
/* Cheat display                                                       */
/* display something in field without changing form context            */
/*                                                                     */
/***********************************************************************/
void cheat_display(FORM * Form, SHORT Field, CHAR * string)
{
  FIELD * pField = &Form->fields[Field];
  USHORT FldAtt = pField->attrib;
  UCHAR attrib, Overflow;
  COLOR_SET *Color = &ColorSets[Form->color_set_index];
  SHORT row = Form->row + pField->row + Form->display_row_offset,
        col = Form->column + pField->column,
        width = strlen(string);

  if (FldAtt & FLDATTR_LIMIT_WARN)
    attrib = set_attributes(Color->error.foreground, Color->error.background);
  else if (FldAtt & FLDATTR_HIGHLIGHT)
    attrib = set_attributes(Color->highlight.foreground, Color->highlight.background);
  else if (FldAtt & FLDATTR_REV_VID)
    attrib = set_attributes(Color->reverse.foreground, Color->reverse.background);
  else if (FldAtt & FLDATTR_SHADED)
    attrib = set_attributes(Color->shaded.foreground, Color->shaded.background);
  else
    attrib = set_attributes(Color->regular.foreground, Color->regular.background);
  display_string(string, width, row, col, attrib);

  if(!(pField->attrib & FLDATTR_NO_OVERFLOW_CHAR || pField->attrib & FLDATTR_DISPLAY_ONLY))
    {
    if (pField->type == FLDTYP_TOGGLE)
      show_toggle(row, (col + pField->width), attrib);
    else
      {
      if (Form->field_overfull_flag)
        {
        if (Form->string_cursor_offset == 0)
          Overflow = OVERFLOW_AT_START;
        else if (Form->string_cursor_offset == Form->field_char_count)
          Overflow = OVERFLOW_AT_END;
        else
          Overflow = OVERFLOW_IN_MIDDLE;
        }
      else
        Overflow = OVERFLOW_NONE;

      show_overflow(Overflow, row, col + pField->width, attrib);
      }
    }
}

/* -----------------------------------------------------------------------
/  function:   Calls the erase_screen_area() display primitive
/              function to erase the area defined by the Current
/              form.
/  requires:   (UCHAR) attrib - from set_attributes()
/              (BOOLEAN) border - TRUE if a border will be drawn
/              around the form area.
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void erase_form_area(UCHAR attrib, BOOLEAN border)
{
  FORM * pForm = Current.Form;
  UCHAR row = pForm->row;

  if (pForm->size_in_rows == 1)   /* there's gotta be a better way... */
    row += pForm->display_row_offset;

  if (pForm->attrib & FORMATTR_SCROLLING && pForm->attrib & FORMATTR_BORDER)
    {
    erase_screen_area((UCHAR)(row-1), pForm->column,
                      (UCHAR)(pForm->size_in_rows + 2),
                      pForm->size_in_columns, attrib, border);
    }
  else
    {  
    erase_screen_area(row, pForm->column,
                      pForm->size_in_rows,
                      pForm->size_in_columns, attrib, border);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UCHAR currentFormStatus(void)
{
  return Current.Form->status;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setCurrentFormStatus(UCHAR newStatus)
{
  Current.Form->status = newStatus;
}

FORM * CurrentForm(void)
{
  return Current.Form;
}

/* -----------------------------------------------------------------------
/  function:   Each datum associated with a field is described by an
/              entry in a data "registry" table, which is unique to
/              each application.  Much of the description involves
/              how to realize the actual address of the datum.
/              A pointer is in the descriptor which may:
/                 directly reference the item
/                 reference another pointer
/                 add an offset to directly reference the item
/                 reference another pointer, then add an offset to it
/              Variations may be easily added if necessary.
/
/  requires:   (DATA *) descriptor - a pointer to a data descriptor structure
/  returns:    (void *) - a pointer to the datum (type casting will be
/                 done by the caller, appropriate to the data and field type)
/  side effects:
/     (Not really a side effect, but note that the OFFSet additions are
/        very Intel-processor specific!)
/ ----------------------------------------------------------------------- */

void * resolve_address(DATA * descriptor)
{
  switch ((SHORT) descriptor->attrib)
    {
    default:
    case DATAATTR_PTR:
      return(descriptor->pointer);

    case DATAATTR_PTR_PTR:
      return(*((void **) descriptor->pointer));

    case DATAATTR_PTR_OFFS:
      {
      void * tempptr;

      tempptr = descriptor->pointer;
      FP_OFF(tempptr) = (FP_OFF(tempptr) + descriptor->offset);
      return(tempptr);
      }
    case DATAATTR_PTR_OFFS_PTR: // case added Dec. 19, 1989. RAC
      {
      void *      tempptr;

      tempptr = descriptor->pointer;
      FP_OFF(tempptr) = (FP_OFF(tempptr) + descriptor->offset);
      return(*((void **) tempptr));
      }
    case DATAATTR_PTR_PTR_OFFS:
      {
      void *      tempptr;

      tempptr = *((void **) descriptor->pointer);
      FP_OFF(tempptr) = (FP_OFF(tempptr) + descriptor->offset);
      return(tempptr);
      }
    }
}

/* -----------------------------------------------------------------------
/  function:   calls the field type specific function to convert
/              data into text, then displays it on the screen.
/  requires:   (BOOLEAN) cursor_on - TRUE if cursor should be activated
/  returns:    (void)
/  side effects:  modifies FieldString and FieldDisplayString
/
/ ----------------------------------------------------------------------- */

void format_and_display_field(BOOLEAN cursor_on)
{
  (void)(*Current.FieldClass->data_to_field)();
  display_field_to_screen(cursor_on);
}


/* -----------------------------------------------------------------------
/  function:   calls the field type specific function to convert
/              the field text into the data format appropriate for
/              the field and data types, then may invoke two
/              optional validation routines.  One is a limiting
/              function which is general to the field type, but
/              uses limit values specific to the field.  The other
/              is a validation procedure specific to the field.
/  requires:   (void)
/  returns:    (int) - a number that will be indicate the status of
/              the field validation procedures.  If >= zero, this
/              value is the character offset within the field where
/              the field validation failed (often zero if no specific
/              offset can be determined).  FIELD_VALIDATE_WARNING and
/              FIELD_VALIDATE_SUCCESS are negative values.
/              Either one may be returned by the optional validation
/              routine.  If the optional data limiting function
/              does limit the entered value, FIELD_VALIDATE_WARNING
/              is returned.
/  side effects:  the datum associated with the field is changed to
/              reflect the entered value.
/
/ ----------------------------------------------------------------------- */

SHORT get_data_from_field(void)
{
  SHORT field_type_validation;
  SHORT limit_validation = FIELD_VALIDATE_SUCCESS;
  SHORT field_specific_validation = FIELD_VALIDATE_SUCCESS;

  field_type_validation = (*Current.FieldClass->data_from_field)();

  if (field_type_validation != FIELD_VALIDATE_SUCCESS)
    return(field_type_validation);
  else
    {
    if((Current.FieldClass->data_limit_action != NULL) &&
      (*Current.FieldClass->data_limit_action)())
      {
      limit_validation = FIELD_VALIDATE_WARNING;
      }

    if (Current.Field->verifier.group != 0)
      {
      DATA *   TempDataDescrip = &Current.Form->dataRegistry
        [Current.Field->verifier.group]
        [Current.Field->verifier.item];

      Current.FieldVerifyFunction =
        ((SHORT(*)(void *, char *)) resolve_address(TempDataDescrip));

      /* now do it ! */
      field_specific_validation =
        (*Current.FieldVerifyFunction)(Current.FieldDataPtr,
        Current.FieldString);
      }

    if(field_specific_validation != FIELD_VALIDATE_SUCCESS)
      return (field_specific_validation);
    else
      {
      if(limit_validation == FIELD_VALIDATE_WARNING)
        (*Current.FieldClass->field_char_action)(0); /* reset field */

      return(limit_validation);
      }
    }
}


/* -----------------------------------------------------------------------
/  function:   prepares the (FORM_CONTEXT) Current structure with
/              all of the addresses necessary to run a new field
/              using the Current.Form->field_index as a starting point.
/  requires:   (void)
/  returns:    (BOOLEAN) - TRUE if field initialized properly
/  side effects:  modifies the global Current structure
/
/ ----------------------------------------------------------------------- */

BOOLEAN init_field(void)
{
  Current.Field = &Current.Form->fields[Current.Form->field_index];

  Current.FieldClass = &FieldClassArray[Current.Field->type];

  if (Current.Field->primary_data.group != 0)
    {
    Current.FieldData = & Current.Form->dataRegistry
      [Current.Field->primary_data.group]
      [Current.Field->primary_data.item];
    Current.FieldDataPtr = resolve_address(Current.FieldData);
    }
  if (Current.Field->alternate_data.group != 0)
    {
    Current.FieldAltData = & Current.Form->dataRegistry
      [Current.Field->alternate_data.group]
      [Current.Field->alternate_data.item];
    Current.FieldAltDataPtr = resolve_address(Current.FieldAltData);
    }
  return(TRUE);
}


/* -----------------------------------------------------------------------
/  function:   prepares the (FORM_CONTEXT) Current structure with
/              all of the addresses necessary to run a new form
/              whose address is the parameter given by the caller.
/              If an initialization function was specified for the
/              form, it is invoked here.
/  requires:   (FORM *) new_form - the form to run (or display)
/  returns:    TRUE if problem
/  side effects:  modifies the global Current structure
/ ----------------------------------------------------------------------- */

BOOLEAN init_form(FORM * new_form)
{
  DATA *   TempDataDescrip;
  BOOLEAN ReturnVal = FALSE;

  Current.Form = new_form;

  if (Current.Form->init_function.group != 0)
    {
    TempDataDescrip = & Current.Form->dataRegistry
      [Current.Form->init_function.group]
      [Current.Form->init_function.item];
    Current.FormInitFunction =
      ((BOOLEAN (*)(SHORT)) resolve_address(TempDataDescrip));
    ReturnVal = (*Current.FormInitFunction)(0);  /* now do it ! */
    }

  if (Current.Form->exit_function.group != 0)
    {
    TempDataDescrip = &Current.Form->dataRegistry
                        [Current.Form->exit_function.group]
                        [Current.Form->exit_function.item];

    Current.FormExitFunction =
      ((BOOLEAN (*)(SHORT)) resolve_address(TempDataDescrip));
    }
  Current.FormColorSet = &ColorSets[Current.Form->color_set_index];

  return ReturnVal;
}

/* -----------------------------------------------------------------------
/  function:   takes any action required on exit.  The form area
/              may be erased, or what was on the screen before
/              may be restored, or the form may be left intact.
/              Calls the form exit function.
/  requires:   (void)
/  returns:    TRUE if need to keep form on screen, FALSE if form exit is OK
/  side effects:
/ ----------------------------------------------------------------------- */

BOOLEAN shutdown_form(void)
{
  if (Current.Form->attrib & FORMATTR_EXIT_ERASE)
    {
    erase_form_area(
      set_attributes(Current.FormColorSet->regular.foreground,
      Current.FormColorSet->regular.background), FALSE);
    }
  else if (Current.Form->attrib & FORMATTR_EXIT_RESTORE)
    {
    if(Current.SavedArea)
      restore_screen_area(Current.SavedArea);
    else if (ExitRestoreFunc)
      (*ExitRestoreFunc)();
    }
  return FALSE;
}

void SetExitRestoreFunc(void (*RestoreFunc)(void))
{
  ExitRestoreFunc = RestoreFunc;
}

/* -----------------------------------------------------------------------
/  function:   saves the current state of the forms system in order
/              to allow nested forms, or routines invoked from
/              the forms to run other forms.
/  requires:   (void)
/  returns:    (BOOLEAN) - TRUE if able to (malloc) push form context
/              if no memory available
/  side effects:
/ ----------------------------------------------------------------------- */

BOOLEAN push_form_context(void)  /* make copy of structure with pointer */
{                                /* to original */
  FORM_CONTEXT * saved;

  saved = (FORM_CONTEXT *) malloc(sizeof(FORM_CONTEXT));

  if (saved != NULL)
    {
    memcpy(saved, &Current, sizeof(FORM_CONTEXT));
    Current.PreviousStackedContext = saved;
    return(TRUE);
    }
  return(FALSE);
}

/* -----------------------------------------------------------------------
/  function:   restores the forms system context (...)
/  requires:   (void)
/  returns:    (void)
/  side effects:  modifies the Current structure
/ ----------------------------------------------------------------------- */

void pop_form_context(void)
{
  FORM_CONTEXT * saved = Current.PreviousStackedContext;

  if (saved != NULL)
    {
    memcpy(&Current, saved, sizeof(FORM_CONTEXT));
    free(saved);
    }
}

/* -----------------------------------------------------------------------
/  function:   called to display a nested form when the
/              draw_form_fields() routine finds a FORM field.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

PRIVATE void format_and_display_FORM_field(void)
{
  if (push_form_context())
    {
    FORM * DescendForm;

    DescendForm = (FORM *) Current.FieldDataPtr;

  if (!(DescendForm->attrib & FORMATTR_DEPENDENT_FORM) &&
      !(DescendForm->attrib & FORMATTR_INDEP))
    {
    DescendForm->row += Current.PreviousStackedContext->Form->row;
    DescendForm->column += Current.PreviousStackedContext->Form->column;
    DescendForm->attrib |= FORMATTR_DEPENDENT_FORM;
    }

    if (Current.Form->attrib & FORMATTR_SCROLLING)
      {
      DescendForm->display_row_offset = Current.Form->display_row_offset;
      DescendForm->virtual_row_index = Current.Form->virtual_row_index;
      }

    if (! init_form(DescendForm))
      draw_form();

    pop_form_context();
    }
}

/* -----------------------------------------------------------------------
/  function:   used to display the fields within a form, usually
/              when the form is first entered.  If a field is a
/              FORM field (nested sub-form entry point), the
/              sub-form is displayed.  There is one tricky bit
/              of business in this routine because of a problem
/              of which sub-forms or fields to draw when there
/              are several optional ones.  The free-form nature
/              of this forms system does not allow this routine
/              to automatically draw any conclusions on what should
/              or should not be displayed.  The form designer must
/              mark any fields whose display is optional with the
/              FLDATTR_GET_DRAW_PERMISSION attribute at compile-time.
/              If a field has this attribute, it will not be displayed
/              unless the FLDATTR_DRAW_PERMITTED attribute is also set.
/              If a LOGIC (executable code) field is used to decide
/              which field will be entered, it will be invoked here,
/              and the field it selects will have the FLDATTR_DRAW_PERMITTED
/              attribute temporarily set to ensure that it is shown.
/              Any logic field that does this should have the
/              FLDATTR_DRAW_PERMITTED attribute selected at compile-time
/              so that it will be invoked.
/
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void draw_form_fields(void)
{
  SHORT    i, saved_index = Current.Form->field_index;
  BOOLEAN  proceed;

  erase_cursor();

  if(Current.Form->number_of_fields)
    {
      for (i=0; i < Current.Form->number_of_fields; i++)
      {
      Current.Form->field_index = i;

      if (init_field())
        {
        if (Current.Field->attrib & FLDATTR_GET_DRAW_PERMISSION)
          proceed = (Current.Field->attrib & FLDATTR_DRAW_PERMITTED);
        else
          proceed = TRUE;

        if (proceed)
          {
          switch (Current.Field->type)
            {
            case FLDTYP_LOGIC:

              if (Current.Field->attrib & FLDATTR_DRAW_PERMITTED)
                {
                run_LOGIC_field();

                if ((Current.Form->field_index > i)
                  && (Current.Form->field_index
                    < Current.Form->number_of_fields))
                    {
                    FIELD * TargetField;

                    TargetField =
                      &Current.Form->fields[Current.Form->field_index];

                    set_bit(TargetField->attrib, FLDATTR_DRAW_PERMITTED);
                    }
                  }
            break;

            case FLDTYP_FORM:
              format_and_display_FORM_field();
              reset_bit(Current.Field->attrib, FLDATTR_DRAW_PERMITTED);
            break;

            default:
              format_and_display_field(FALSE);
            break;
            }
          }
        }
      }
    Current.Form->field_index = saved_index;
    init_field();
    }
}

/* -----------------------------------------------------------------------
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void draw_scroll_form_fields(void)
{
   Current.Form->field_index = 0;   /* call initial (logic) field */

   if(init_field())
     run_LOGIC_field();      /* if it's not a logic field, watch out! */
}


/* -----------------------------------------------------------------------
/  function:   draws the form box.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void erase_form(void)
{
  UCHAR attrib =
    set_attributes(Current.FormColorSet->regular.foreground,
                   Current.FormColorSet->regular.background);

  erase_form_area(attrib, (Current.Form->attrib & FORMATTR_BORDER));
}

/* -----------------------------------------------------------------------
/  function:   draws the form box, then the fields in it.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void draw_form(void)
{
  if (Current.Form->attrib & FORMATTR_VISIBLE)
    {
    MouseCursorEnable(FALSE);
    erase_form();

    if (Current.Form->attrib & FORMATTR_SCROLLING)
        draw_scroll_form_fields();
    else
        draw_form_fields();

    MouseCursorEnable(TRUE);
    replace_mouse_cursor();
    }
}

/* -----------------------------------------------------------------------
/  function:   called by update_display_string() to format as much of
/              the FieldString (which contains the full text of the
/              field) as will fit into the FieldDisplayString (which
/              contains the text image that will show in the field).
/              This function is called only when there is more text
/              in the FieldString than will fit in the field.
/  requires:   (int) offset - the offset from the start of the
/              FieldString to take text for the FieldDisplayString.
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

static void window_into_overfull_field(SHORT offset)
{
  memcpy(&(Current.FieldDisplayString), &(Current.FieldString[offset]),
  Current.Field->width);

  Current.FieldDisplayString[Current.Field->width] = 0;
}


/* -----------------------------------------------------------------------
/
/  static void window_into_field(void)
/
/  function:   called by update_display_string() to format the
/              FieldDisplayString with text from the FieldString.
/              Left or right justification is performed depending
/              on the field attributes.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

static void window_into_field(void)
{
  SHORT space_count = (Current.Field->width - Current.Form->field_char_count);

  if (Current.Field->attrib & FLDATTR_RJ)
    {
    if (space_count > 0)
      char_fill(&(Current.FieldDisplayString), space_count, SPACE);

    strncpy(&(Current.FieldDisplayString[space_count]), Current.FieldString,
      Current.Form->field_char_count);
    }
  else     /* assume left justification */
    {
    strncpy(Current.FieldDisplayString, Current.FieldString,
      Current.Form->field_char_count);

    if (space_count > 0)
      char_fill(&(Current.FieldDisplayString[Current.Form->field_char_count]),
        space_count, SPACE);
    }
  Current.FieldDisplayString[Current.Field->width] = 0;
}


/* -----------------------------------------------------------------------
/  function:   called to make sure the display image for the field
/              window is current after modifying the field string
/              or taking some other action, such as moving the cursor.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void update_display_string(void)
{
  FORM * pForm = Current.Form;

  if (pForm->field_overfull_flag)
    {
    SHORT offset;

    if (pForm->display_cursor_offset > pForm->string_cursor_offset)
      {
      offset = 0;
      pForm->display_cursor_offset = pForm->string_cursor_offset;
      }
    else
      {
      offset =
        (SHORT)(pForm->string_cursor_offset - pForm->display_cursor_offset);
      }
    window_into_overfull_field(offset);
    }
  else
    window_into_field();
}

/* -----------------------------------------------------------------------
/  function:   This is the workhorse function used by most field
/              type data-to-field routines to update the field text
/              strings after they have converted their specific data
/              type to a string.  It copies the source_string into
/              the FieldString, sets the variables associated with
/              the field text string, and also causes the display
/              string to be initialized.
/  requires:   (char *) source_string - the new text for the field.
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void string_to_field_string(char * source_string)
{
   UCHAR source_string_len;

  source_string_len = (UCHAR) strlen(source_string);

  if (Current.Field->type == FLDTYP_STRING)
    {
    /* If the max_len is zero, then don't change source_string_len. */
    UCHAR limit = Current.Field->specific.strfld.max_len;
    if((limit > 0) && (source_string_len > limit))
      source_string_len = limit - (char) 1;
    }

  strncpy(Current.FieldString, source_string, source_string_len);
  Current.FieldString[source_string_len] = 0;

  Current.Form->field_char_count = source_string_len;

  Current.Form->field_overfull_flag = (UCHAR)
    (Current.Form->field_char_count > Current.Field->width);

  if (Current.Form->field_overfull_flag)
    {
    Current.Form->string_cursor_offset = Current.Field->width;
    Current.Form->display_cursor_offset = Current.Field->width;
    }
  else /* fits into field ok... */
    {
    Current.Form->string_cursor_offset = source_string_len;

    if (Current.Field->attrib & FLDATTR_RJ)
      Current.Form->display_cursor_offset = Current.Field->width;
    else /* assume left justification */
      Current.Form->display_cursor_offset = Current.Form->field_char_count;
    }
  update_display_string();
}

/* -----------------------------------------------------------------------
/  function:   doesn't actually move the cursor, really it just tries
/              to increment the cursor offset variable, unless that
/              would extend past the end of the field window, or the
/              text.  Cursor offset is strictly a function of the width
/              of the field; another variable controls the offset into
/              the field text.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void display_cursor_right(void)
{
  UCHAR     max_display_cursor;

  if (Current.Field->attrib & FLDATTR_RJ)
    max_display_cursor = Current.Field->width;
  else
    {
    max_display_cursor =
      min(Current.Field->width, Current.Form->field_char_count);
    }

  if (++Current.Form->display_cursor_offset > max_display_cursor)
    {
    --Current.Form->display_cursor_offset;
    }
}

/* -----------------------------------------------------------------------
/  function:   usually called in response to a right-arrow key press.
/              If possible, the string cursor is moved to the right,
/              then display_cursor_right() is called to move the display
/              cursor to the right.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void cursor_right(void)
{
  if (++Current.Form->string_cursor_offset > Current.Form->field_char_count)
    {
    --Current.Form->string_cursor_offset;
    }
  display_cursor_right();
  update_display_string();

}

/* -----------------------------------------------------------------------
/  function:   doesn't actually move the cursor, really it just tries
/              to decrement the cursor offset variable, unless that
/              would extend past the left end of the field window, or the
/              text.  Cursor offset is strictly a function of the width
/              of the field; another variable controls the offset into
/              the field text.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void display_cursor_left(void)
{
  UCHAR  min_display_cursor;

  if ((Current.Field->attrib & FLDATTR_RJ) &&
     (!Current.Form->field_overfull_flag))
    {
    min_display_cursor =
      (Current.Field->width - Current.Form->field_char_count);
    }
  else
    min_display_cursor = 0;

  if (Current.Form->display_cursor_offset > min_display_cursor)
    {
    --Current.Form->display_cursor_offset;
    }
}

/* -----------------------------------------------------------------------
/  function:   usually called in response to a left-arrow key press.
/              If possible, the string cursor is moved to the left,
/              then display_cursor_left() is called to move the display
/              cursor to the left.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void cursor_left(void)
{
  if (Current.Form->string_cursor_offset > 0)
    --Current.Form->string_cursor_offset;

  display_cursor_left();
  update_display_string();
}

/* -----------------------------------------------------------------------
/  function:   force the cursor to the beginning of the field text.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void cursor_full_left(void)
{
  if (Current.Field->attrib & FLDATTR_RJ)
    {
    Current.Form->display_cursor_offset =
      max(((UCHAR) 0), (Current.Field->width - Current.Form->field_char_count));
    }
  else
    Current.Form->display_cursor_offset = 0;

  Current.Form->string_cursor_offset = 0;

  update_display_string();
}

/* -----------------------------------------------------------------------
/  function:   force the cursor to the end of the field text.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void cursor_full_right(void)
{
  if (Current.Field->attrib & FLDATTR_RJ)
    Current.Form->display_cursor_offset = Current.Field->width;
  else
    {
    Current.Form->display_cursor_offset =
      min(Current.Field->width, Current.Form->field_char_count);
    }

  Current.Form->string_cursor_offset = Current.Form->field_char_count;

  update_display_string();
}


/* -----------------------------------------------------------------------
/  function:   removes a character from the field text string
/              if possible.
/  requires:   (BOOLEAN) go_left - TRUE if the character that should
/              be deleted is to the left of the cursor, otherwise
/              the character at the cursor is deleted
/  returns:    (BOOLEAN) - TRUE if character successfully deleted
/  side effects:
/
/ ----------------------------------------------------------------------- */

BOOLEAN delete_char_from_field(BOOLEAN go_left)
{
  char * field_at_cursor;
  SHORT  move_count;

  if (Current.Form->field_char_count > 0)
    {
    move_count =
      ((SHORT) Current.Form->field_char_count -
       (SHORT) Current.Form->string_cursor_offset);

    if (go_left)
      {
      if (Current.Form->string_cursor_offset == 0)
        return(FALSE);
      else
        Current.Form->string_cursor_offset--;
      }
    else
      {
      if (Current.Form->string_cursor_offset
        == Current.Form->field_char_count)
        return(FALSE);
      else
        move_count--;
      }

    if (move_count > 0)
      {
      field_at_cursor =
        &(Current.FieldString[Current.Form->string_cursor_offset]);
      memcpy(field_at_cursor, (field_at_cursor + 1), move_count);
      }

    Current.Form->field_char_count--;
    Current.FieldString[Current.Form->field_char_count] = 0;

    Current.Form->field_overfull_flag = (UCHAR)
      (Current.Form->field_char_count > Current.Field->width);

    return(TRUE);
    }
  else
    return(FALSE);
}


/* -----------------------------------------------------------------------
/  function:   deletes the character to the left of the cursor,
/              usually after the backspace key is pressed.  Any
/              modifications that have to be made to the string
/              text and cursor offsets are made depending on the
/              justification attributes and other factors.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void char_delete_left(void)
{
  if (delete_char_from_field(TRUE))
    {
    if (! (Current.Field->attrib & FLDATTR_RJ))
      {
      if (Current.Form->field_char_count == Current.Field->width)
        {
        Current.Form->display_cursor_offset =
          Current.Form->string_cursor_offset;
        }
      else
        {
        if (Current.Form->field_overfull_flag)
          {
          if (! ((char) Current.Field->width
            > ((char) Current.Form->string_cursor_offset
            - (char) Current.Form->display_cursor_offset)))
            {
            display_cursor_left();
            }
          }
        else
          display_cursor_left();
        }
      }
    update_display_string();
    }
}


/* -----------------------------------------------------------------------
/  function:   deletes the character at the cursor, usually after
/              the backspace key is pressed.  Any modifications
/              that have to be made to the string text and cursor
/              offsets are made depending on the justification
/              attributes and other factors.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void char_delete_at_cursor(void)
{
  if (delete_char_from_field(FALSE))
    {
    if (Current.Field->attrib & FLDATTR_RJ)
      {
      if (Current.Form->field_char_count == Current.Field->width)
        {
        Current.Form->display_cursor_offset =
          Current.Form->string_cursor_offset;
        }
      else
        {
        if (! Current.Form->field_overfull_flag)
          display_cursor_right();
        }
      }
    update_display_string();
    }
}


/* -----------------------------------------------------------------------
/  function:   deletes all text in the field at one blow.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void delete_entire_field(void)
{
  Current.Form->field_overfull_flag = FALSE;
  Current.Form->field_char_count = 0;
  Current.Form->string_cursor_offset = 0;
  Current.FieldString[0] = 0;               /* in case field is zoomed */

  if (Current.Field->attrib & FLDATTR_RJ)
    Current.Form->display_cursor_offset = Current.Field->width;
  else
    Current.Form->display_cursor_offset = 0;

  update_display_string();
}


/* -----------------------------------------------------------------------
/  function:   called to add a character to the field string at
/              the current position.  This job is complicated
/              by the OVERSTRIKE form attribute, which allows
/              new keys to replace the characters at the current
/              position instead of moving the following text
/              to make room for the new keys.
/  requires:   (UCHAR) key - the character to insert
/  returns:    (BOOLEAN) - TRUE if character successfully added to string
/  side effects:
/
/ ----------------------------------------------------------------------- */

BOOLEAN insert_char_into_field(UCHAR key)
{
  char *   field_at_cursor;
  BOOLEAN  at_last_position;

  at_last_position =
    (Current.Form->string_cursor_offset >= Current.Form->field_char_count);

  /* is there NO room for another character? */
  if ((Current.Form->field_char_count >= MAX_FIELD_SIZE) ||
    ((Current.Field->type == FLDTYP_STRING) &&
    (Current.Form->field_char_count >=
    (Current.Field->specific.strfld.max_len - (char) 1))))
    {
    if (Current.Form->attrib & FORMATTR_OVERSTRIKE)
      {                        /* in overstrike mode characters are */
      if (at_last_position)    /* not really "inserted" unless the  */
        return(FALSE);         /* cursor is at the end of the text  */
      }
    else
      return(FALSE);
    }
  field_at_cursor =
    &(Current.FieldString[Current.Form->string_cursor_offset]);

  if (Current.Form->attrib & FORMATTR_OVERSTRIKE)
    {
    if (at_last_position)
      Current.Form->field_char_count++;
    }
  else
    {
    if (!at_last_position)
      {
      memmove(field_at_cursor + 1, field_at_cursor,
              Current.Form->field_char_count -
              Current.Form->string_cursor_offset);
      }
    Current.Form->field_char_count++;
    }
  *field_at_cursor = key;

  Current.Form->string_cursor_offset++;
  Current.FieldString[Current.Form->field_char_count] = 0;

  Current.Form->field_overfull_flag = (UCHAR)
    (Current.Form->field_char_count > Current.Field->width);

  return(TRUE);
}


/* -----------------------------------------------------------------------
/  function:   called to put a key into the field.  If it can
/              be inserted into the field text, the cursor is
/              adjusted in the appropriate direction and the
/              field display string is updated.
/  requires:   (UCHAR) key - the character to insert
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void char_insert(UCHAR key)
{
  if (insert_char_into_field(key))
    {
    if (Current.Form->display_cursor_offset < Current.Field->width)
      {
      if (Current.Field->attrib & FLDATTR_RJ)
        {
        if((Current.Form->attrib & FORMATTR_OVERSTRIKE) ||
           (Current.Form->display_cursor_offset == 0))
          {
          display_cursor_right();
          }
        }
      else
        display_cursor_right();
      }
    update_display_string();
    }
}

/* -----------------------------------------------------------------------
/  function:   used by most field types (string, float, int, etc.)
/              that have no oddball editing requirements.  Takes
/              appropriate action for the various cursor and
/              editing keys, and inserts any characters.
/  requires:   (UCHAR) key - key from keyboard input
/              routine (which translates special keys into
/              single byte codes with the high bit set)
/              NOTE: if key value is NULL, this routine performs
/              self-initialization, as all field type character
/              action routines are expected to do
/  returns:    (BOOLEAN) - TRUE if character was recognized
/              and used by this routine
/  side effects:
/
/ ----------------------------------------------------------------------- */

BOOLEAN default_char_action(UCHAR key)
{
  BOOLEAN key_was_used = TRUE;
  static  BOOLEAN erase_trigger;
  SHORT   row;
  SHORT   column;

  if (key)
    {
    if (key & KEYS_HIGH_BIT)
      {
      switch (key)
        {
        case KEY_BACKSPACE:
           char_delete_left();
           break;
        case KEY_RIGHT:
           cursor_right();
           break;
        case KEY_LEFT:
           cursor_left();
           break;
        case KEY_HOME:
           cursor_full_left();
           break;
        case KEY_END:
           cursor_full_right();
           break;
        case KEY_DELETE:
           char_delete_at_cursor();
           break;
        case KEY_DELETE_FAR:
           delete_entire_field();
           break;
        default:
           key_was_used = FALSE;
        }
      if (key_was_used)
        erase_trigger = FALSE;
      }
    else
      {
      if (erase_trigger != FALSE)
        {
        delete_entire_field();
        erase_trigger = FALSE;
        }
      char_insert(key);
      }

    display_field_to_screen(TRUE);

    // Show the cursor position
    row = (SHORT) (Current.Form->row + Current.Field->row +
      Current.Form->display_row_offset);
    column = (SHORT) (Current.Form->column + Current.Field->column);
    set_cursor(row, (column + Current.Form->display_cursor_offset));
    }
  else     /* function was called to do self-initialization */
    {
    // Ignore the form attribute.  The default
    // will be to have ALL forms behave as if FORMATTR_FIRST_CHAR_ERASE
    // were set.
    // erase_trigger = Current.Form->attrib & FORMATTR_FIRST_CHAR_ERASE;
    erase_trigger = TRUE;
    }
  return(key_was_used);
}

/* -----------------------------------------------------------------------
/  function:   called after a failure to validate a field, this
/              routine positions the cursor to the offset within
/              the field supplied by the caller, presumably the
/              location of the problem.
/  requires:   (UCHAR) offset - offset into field text
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

PRIVATE void position_cursor_in_field(UCHAR offset)
{
   Current.Form->string_cursor_offset = offset;

  if (Current.Form->field_overfull_flag)
    Current.Form->display_cursor_offset = (Current.Field->width / (UCHAR) 2);
  else
    {
    if (Current.Field->attrib & FLDATTR_RJ)
      {
      Current.Form->display_cursor_offset =
        ((Current.Field->width - Current.Form->field_char_count)
        + offset);
      }
    else
      {
      Current.Form->display_cursor_offset =
        min(Current.Field->width, offset);
      }
    }
  update_display_string();
}


/* -----------------------------------------------------------------------
/  function:   switches between overstrike and insert mode.
/              The cursor is also modified to show which mode is
/              active (not all screen display primitive functions
/              support this feature).
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

static void toggle_insert_mode(void)
{
   if (Current.Form->attrib & FORMATTR_OVERSTRIKE)
   {
      reset_bit(Current.Form->attrib, FORMATTR_OVERSTRIKE);
      set_cursor_type(CURSOR_TYPE_NORMAL);
   }
   else
   {
      set_bit(Current.Form->attrib, FORMATTR_OVERSTRIKE);
      set_cursor_type(CURSOR_TYPE_OVERSTRIKE);
   }

}


/* -----------------------------------------------------------------------
/  function:   this is the field type specific function for
/              data-to-field for the STRING field type.
/              STRING fields are the only field type supported
/              directly by this module (forms.c); support for
/              other field types comes from other modules.
/              (accessories sold separately, batteries not included.)
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void string_to_field(void)
{
   string_to_field_string((char *) Current.FieldDataPtr);
}

/* -----------------------------------------------------------------------
/  function:   this is the field type specific function for
/              data-from-field for the STRING field type.
/              STRING fields are the only field type supported
/              directly by this module (forms.c); support for
/              other field types comes from other modules.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

SHORT string_from_field(void)
{
   UCHAR field_len;

   field_len = min(
   (Current.Field->specific.strfld.max_len - (UCHAR) 1), \
   Current.Form->field_char_count);

   strncpy((char *) Current.FieldDataPtr, Current.FieldString,
   (SHORT) field_len);

   ((char *) Current.FieldDataPtr)[field_len] = 0;

   return(FIELD_VALIDATE_SUCCESS);
}


/* -----------------------------------------------------------------------
/  function:   simply translates a key for a field exit into an
/              index into the exit vector array for the field.
/  requires:   (UCHAR) exit_code - the key value
/  returns:    (UCHAR) - the exit vector array index
/  side effects:
/
/ ----------------------------------------------------------------------- */

UCHAR exit_code_to_vector_index(UCHAR exit_code)
{
   UCHAR  exit_vector;

   switch (exit_code)
   {
      case KEY_ESCAPE:     exit_vector = EXIT_DEFAULT;   break;
      case KEY_ENTER:      exit_vector = EXIT_ENTER;     break;
      case KEY_UP:         exit_vector = EXIT_UP;        break;
      case KEY_DOWN:       exit_vector = EXIT_DOWN;      break;
      case KEY_LEFT_FAR:   exit_vector = EXIT_LEFT;      break;
      case KEY_RIGHT_FAR:  exit_vector = EXIT_RIGHT;     break;
      case KEY_BACK_TAB:   exit_vector = EXIT_TAB_LEFT;  break;
      case KEY_TAB:        exit_vector = EXIT_TAB_RIGHT; break;
      default:             exit_vector = EXIT_NONE;
   }
   return(exit_vector);
}

/* -----------------------------------------------------------------------
/  function:   find the offset to the next active field,
/              from the exit vector array for THIS field,
/              depending on which key was pressed.
/              NOTE: an offset of zero means no exit is taken
/              from the current field.
/  requires:   (UCHAR) exit_code - the key value
/  returns:    (signed char) - the exit vector value, which is
/              an +/- offset to the next active field.
/  side effects:
/
/ ----------------------------------------------------------------------- */

signed char exit_code_to_vector(UCHAR exit_code)
{
   UCHAR  vector_index;

   vector_index = exit_code_to_vector_index(exit_code);

   if (vector_index == EXIT_NONE)
      return(0);
   else
      return(Current.Field->exit_vectors[vector_index]);
}


/* -----------------------------------------------------------------------
/  function:   adds the offset from the field exit vector to the
/              current field index to find out where to go next.
/              The exit vector value is a positive or negative
/              relative offset from the current field index.
/              if the index+offset result lies outside the form
/              (i.e. less than zero or greater than the number
/              of fields in the form), the form status is changed
/              to cause this form to be exited.  Otherwise, the
/              current field index is updated.  This will cause
/              a jump to the appropriate field the next time
/              init_field() is called, which will be soon after
/              leaving this routine.
/  requires:   (void)
/  returns:    (void)
/  side effects:  MAY CAUSE EXIT FROM CURRENT FORM.
/
/ ----------------------------------------------------------------------- */

void find_next_field_index(void)
{
  if((Current.Form->status != FORMSTAT_EXIT_ALL_FORMS) &&
     (Current.Form->status != FORMSTAT_SWITCH_MODE) &&
     (Current.Form->status != FORMSTAT_EXIT_TO_MENU1) &&
     (Current.Form->status != FORMSTAT_EXIT_TO_MENU2))
     {
    SHORT new_field_index;

    new_field_index =
      (Current.Form->field_index + ((SHORT)Current.Form->next_field_offset));

    if ((new_field_index < 0) ||
      (new_field_index >= Current.Form->number_of_fields))
      {
      Current.Form->status = FORMSTAT_EXIT_THIS_FORM;
      }
    else
      {
      Current.Form->previous_field_index =
        Current.Form->field_index;

      Current.Form->field_index = new_field_index;
      }
    }
}

/* -----------------------------------------------------------------------
/  function:   Since this forms system allows more text to be
/              entered into a field than can be displayed by
/              most normal width fields, there is a need for
/              some way to see the entire field text at once.
/              This routine causes to field to "zoom" out to
/              full screen width.  The field may be edited
/              normally while zoomed.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

static void zoom_field(void)
{
   if (! (Current.Form->attrib & FORMATTR_ZOOMED_FIELD))
   {
      set_bit(Current.Form->attrib, FORMATTR_ZOOMED_FIELD);

      ZoomSave.attrib = Current.Field->attrib;
      ZoomSave.row = Current.Field->row;
      ZoomSave.column = Current.Field->column;
      ZoomSave.width = Current.Field->width;
      ZoomSave.form_column = Current.Form->column;

      reset_bit(Current.Field->attrib, FLDATTR_RJ);

      Current.Field->column = 0;
      Current.Field->width = 79;
      Current.Form->column = 0;

      ZoomSave.actual_column = 0;
      ZoomSave.actual_row =
      (Current.Form->row + Current.Field->row
      + Current.Form->display_row_offset);

      ZoomSave.screen =
      save_screen_area(ZoomSave.actual_row, ZoomSave.actual_column,
      (UCHAR) 1,
      ((UCHAR)(Current.Field->width + (UCHAR) 1)));
      /* the +1 is for the extra char at end of field */

      string_to_field_string(Current.FieldString);
      display_field_to_screen(TRUE);
   }
}

/* -----------------------------------------------------------------------
/  function:   collapses a "zoomed" field back to normal width.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void unzoom_field(void)
{
   if (Current.Form->attrib & FORMATTR_ZOOMED_FIELD)
   {
      if (ZoomSave.screen != NULL)
      {
         reset_bit(Current.Form->attrib, FORMATTR_ZOOMED_FIELD);

         restore_screen_area(ZoomSave.screen);
         /* the +1 is for the extra char at end of field */

         Current.Field->attrib = ZoomSave.attrib;
         /*    Current.Field->row = ZoomSave.row; */
         Current.Field->column = ZoomSave.column;
         Current.Field->width = ZoomSave.width;
         Current.Form->column = ZoomSave.form_column;

         string_to_field_string(Current.FieldString);
         display_field_to_screen(TRUE);
      }
   }
}

/*******************************************************************/

SHORT StandardVerify(UCHAR key)
{
   SHORT verify_result;

   verify_result = get_data_from_field();

   if ((verify_result < (UCHAR)0)
      || (verify_result > (SHORT) Current.Form->field_char_count))
   {
      if (verify_result == FIELD_VALIDATE_WARNING)
      {
         set_bit(Current.Field->attrib, FLDATTR_LIMIT_WARN);
         if (audible_error_handler != NULL)
            (*audible_error_handler)();
      }
      else
         reset_bit(Current.Field->attrib, FLDATTR_LIMIT_WARN);

      (*Current.FieldClass->data_to_field)();
      /* re-initialize field (behave as if newly entered) */
      (*Current.FieldClass->field_char_action)(0);

      if (KSActivateFieldRecord != NULL)
      {
         if ((verify_result != FIELD_VALIDATE_WARNING) &&
            (*pKSRecord == TRUE) &&
            (Current.Field->MacFieldIndex != KSI_NO_INDEX))
            (*KSActivateFieldRecord)
            (Current.Field->MacFieldIndex,
            Current.FieldDisplayString);
      }

      if ((! (Current.Form->attrib & FORMATTR_NO_ENTER_VECTOR))
         && ((Current.Form->next_field_offset =
         exit_code_to_vector(key)) != 0))
      {
         Current.Form->status = FORMSTAT_EXIT_THIS_FIELD;
         find_next_field_index();
      }
      else
      {
         display_field_to_screen(TRUE);
      }

   }
   else
   {
      position_cursor_in_field((UCHAR) verify_result);
      set_bit(Current.Field->attrib, FLDATTR_LIMIT_WARN);
      display_field_to_screen(TRUE);
      reset_bit(Current.Field->attrib, FLDATTR_LIMIT_WARN);
      /* raise error signal */
      if (audible_error_handler != NULL)
         (*audible_error_handler)();
   }
   return verify_result;
}

/* -----------------------------------------------------------------------
/  function:   This is the central operator for fields that accept
/              input.  This runs the field from the time it is
/              entered by the user until the time it is exited.
/              It accepts and processes each keystroke until
/              the form status is no longer FORMSTAT_ACTIVE_FIELD
/              for any reason.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void run_input_field(void)
{
  BOOLEAN potential_field_exit;
  BOOLEAN key_used_by_field;
  UCHAR   key;
  BOOLEAN EmptyScroll = FALSE;
  USHORT  TempIndex;

  /* call to initialize */
  (*Current.FieldClass->field_char_action)(0);

  set_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
  reset_bit(Current.Field->attrib, FLDATTR_LIMIT_WARN);
  format_and_display_field(TRUE);

  // Record focus only for scroll forms for now, should also work for
  // any type of form if wanted later.
  if (KSFieldFocusRecord != NULL)
    {
    if ((*pKSRecord == TRUE) &&
      (Current.Form->attrib & FORMATTR_SCROLLING))
      (*KSFieldFocusRecord) (Current.Form->virtual_row_index);
    }

  do
    {
    potential_field_exit = FALSE;
    key_used_by_field = FALSE;

    key = get_FORM_key_input();

    if (FormsCTypeCheck[key] & Current.FieldClass->TypeBit)
      {
      key_used_by_field = (*Current.FieldClass->field_char_action)(key);
      }

    if (!key_used_by_field)
      {
      switch (key)
        {
        case KEY_ENTER:
          {
          unzoom_field();

          if (Current.FieldData->type == DATATYP_FORM)
            {
            if(setup_for_nested_form_entry(((FORM *)Current.FieldDataPtr), &Current.Form->attrib))
             ;

            /* check to make sure that the form initialization came */
            /* out OK before recording an active form */
            if (Current.Form->status == FORMSTAT_ACTIVE_FIELD)
              TempIndex = Current.Form->MacFormIndex;
            else
              TempIndex = KSI_NO_INDEX;     /* don't record activation */

            if((KSActivateFormRecord != NULL) && (TempIndex != KSI_NO_INDEX))
              {
              /* don't record a scroll refresh */
              if(*pKSRecord && !((Current.Form->attrib & FORMATTR_SCROLLING) &&
                scroll_refresh_only_flag))
                (*KSActivateFormRecord)(TempIndex, TRUE); /* nested form */
              }
            potential_field_exit = TRUE;
            Current.Form->status = FORMSTAT_EXIT_THIS_FIELD;
            }
          else
            StandardVerify(key);

          if (pKSPlayBack != NULL)
            {
            // possible states: TRUE, FALSE, -1 (just starting),
            // -2 (just got error)
            if ((*pKSPlayBack == TRUE) || (*pKSPlayBack == (USHORT)-2))
              Current.Form->status = FORMSTAT_EXIT_THIS_FIELD;
            }

          // erase cursor if it is showing so that it can be used
          // for a visual clue as to when the field has been
          // accepted and subject to first key erase.
          erase_cursor();
          }

        case KEY_ESCAPE:
        case KEY_TAB:
        case KEY_BACK_TAB:
        case KEY_UP:
        case KEY_DOWN:
        case KEY_RIGHT_FAR:
        case KEY_LEFT_FAR:
        case KEY_RIGHT:
        case KEY_LEFT:
          potential_field_exit = TRUE;
        break;

        case KEY_PLUS:
          zoom_field();
          key = 0;
        break;

        case KEY_MINUS:
          unzoom_field();
          key = 0;
        break;

        case KEY_PG_UP:
          key = KEY_ESCAPE;
          unzoom_field();
          Current.Form->status = FORMSTAT_EXIT_THIS_FORM;
        break;

        case KEY_ALT_E:
          if (Current.Form->attrib & FORMATTR_NO_ENTER_VECTOR)
            reset_bit(Current.Form->attrib, FORMATTR_NO_ENTER_VECTOR);
          else
            set_bit(Current.Form->attrib, FORMATTR_NO_ENTER_VECTOR);
          key = 0;
        break;

        case KEY_ALT_D:
          if (Current.Form->attrib & FORMATTR_FIRST_CHAR_ERASE)
            reset_bit(Current.Form->attrib, FORMATTR_FIRST_CHAR_ERASE);
          else
            set_bit(Current.Form->attrib, FORMATTR_FIRST_CHAR_ERASE);
          key = 0;
        break;

        case KEY_INSERT:
          toggle_insert_mode();
          key = 0;
        break;

        default:
          if (UserKeyHandler)
            Current.Form->status = (*UserKeyHandler)(key);
        break;
        }
      }
    /* check to see if scroll form has any active lines */
    /* If not, fake a tab */
    if ((Current.Form->attrib & FORMATTR_SCROLLING) &&
        (Current.Form->virtual_row_index == (USHORT)-1))
      {
      key = KEY_TAB;
      Current.Form->next_field_offset = 0;
      Current.Form->field_index = Current.Form->number_of_fields;
      Current.Form->status = FORMSTAT_EXIT_THIS_FORM;
      EmptyScroll = TRUE;
      Current.Form->virtual_row_index = 0;
      }

    if (Current.Form->status == FORMSTAT_ACTIVE_FIELD)
      {
      if (potential_field_exit)
        {
        /**** if the field offset is not zero, move to the new field ****/
        loop_through_field:
        if ((Current.Form->next_field_offset = exit_code_to_vector(key)))
          {
          unzoom_field();

          Current.Form->status = FORMSTAT_EXIT_THIS_FIELD;
          (*Current.FieldClass->data_to_field)();
          find_next_field_index();

          if (Current.Form->fields[Current.Form->field_index].attrib &
            FLDATTR_DISABLED)
            goto loop_through_field;
          }
        }
      }
    }
  while (Current.Form->status == FORMSTAT_ACTIVE_FIELD);

  if (! EmptyScroll)
    {
    reset_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
    display_field_to_screen(FALSE);
    }

  if (Current.Form->status == FORMSTAT_EXIT_THIS_FIELD)
    Current.Form->status = FORMSTAT_ACTIVE_FIELD;

  Current.Form->exit_key_code = key;
}

/* -----------------------------------------------------------------------
/  function:   since Display Only fields are not really "run",
/              this function simply passes execution on to the
/              next field
/
/ ----------------------------------------------------------------------- */

void run_D_O_field(void)
{
   if (!(Current.Field->attrib & FLDATTR_GET_DRAW_PERMISSION) ||
       (Current.Field->attrib & FLDATTR_DRAW_PERMITTED))
   format_and_display_field(FALSE);
   Current.Form->next_field_offset =
   Current.Field->exit_vectors[EXIT_DEFAULT];
   find_next_field_index();
}


/* -----------------------------------------------------------------------
/  function:   when execution is passed to a FORM field, this routine
/              provides the linkage to the sub-form, entering the
/              sub-form when the FORM field is entered, and exiting
/              the FORM field when the sub-form is exited.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
void exit_from_nested_form(void)
{
  FORM * DescendForm;

  DescendForm = Current.Form;

  if (DescendForm->attrib & (FORMATTR_INDEP | FORMATTR_STICKY))
    {
    if (DescendForm->exit_function.group != 0)
      {
      /* call exit function */
      if ((*Current.FormExitFunction)(0))
        {     // exit failed
        DescendForm->status = FORMSTAT_ACTIVE_FIELD;
        return;
        }
      }
    shutdown_form();
    }

  reset_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
  reset_bit(DescendForm->attrib, FORMATTR_NESTED);
  pop_form_context();

  if ((!(DescendForm->attrib & FORMATTR_INDEP)) &&
    (DescendForm->attrib & FORMATTR_EXIT_ERASE))
    {
    UCHAR row = DescendForm->row;

    if (DescendForm->size_in_rows == 1)
      row += DescendForm->display_row_offset;

    erase_screen_area(row, DescendForm->column,
      DescendForm->size_in_rows,
      DescendForm->size_in_columns,
      set_attributes(Current.FormColorSet->regular.foreground,
      Current.FormColorSet->regular.background), FALSE);
    }

//Current.Form->attrib =
//  ((Current.Form->attrib & ~(FORMATTR_GLOBAL_BITS))
//  | (DescendForm->attrib & FORMATTR_GLOBAL_BITS));

  Current.Form->exit_key_code = DescendForm->exit_key_code;

  if (!(DescendForm->attrib & FORMATTR_INDEP))
    {
    Current.Form->next_field_offset =
      exit_code_to_vector(Current.Form->exit_key_code);

    find_next_field_index();
    }
  else
    Current.Form->status = FORMSTAT_ACTIVE_FIELD;

  if (DescendForm->status == FORMSTAT_EXIT_ALL_FORMS)
    Current.Form->status = FORMSTAT_EXIT_ALL_FORMS;
  else if (DescendForm->status == FORMSTAT_SWITCH_MODE)
    Current.Form->status = FORMSTAT_SWITCH_MODE;
  else if (DescendForm->status == FORMSTAT_EXIT_TO_MENU1)
    Current.Form->status = FORMSTAT_EXIT_TO_MENU1;
  else if (DescendForm->status == FORMSTAT_EXIT_TO_MENU2)
    Current.Form->status = FORMSTAT_EXIT_TO_MENU2;
}


/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void run_LOGIC_field()
{
  if (Current.Field->primary_data.group != 0)
    {
    if (Current.FieldData->type == DATATYP_CODE)
      {
      UCHAR  (*LOGIC_function)(FORM_CONTEXT * frmc);
      UCHAR  exit_vector;

      LOGIC_function = ((UCHAR (*)()) Current.FieldDataPtr);

      exit_vector = (*LOGIC_function)(&Current);

      if (exit_vector != EXIT_NONE)
        {
        Current.Form->next_field_offset =
          Current.Field->exit_vectors[exit_vector];

        find_next_field_index();
        }
      }
    }
}


/* -----------------------------------------------------------------------
/  requires:
/  returns:
/  side effects:
/ ----------------------------------------------------------------------- */

BOOLEAN setup_for_form_entry(FORM * this_form, USHORT * global_attributes)
{
  BOOLEAN ReturnVal;

  if (this_form->attrib & FORMATTR_FULLHEIGHT)
    {
    this_form->row = (UCHAR)((screen_rows - this_form->size_in_rows) / 2);
    this_form->attrib &= (~FORMATTR_FULLHEIGHT);
    }
  
  if (this_form->attrib & FORMATTR_FULLWIDTH)
    {
    this_form->column =
      (UCHAR)((screen_columns - this_form->size_in_columns) / 2);
    this_form->attrib &= (~FORMATTR_FULLWIDTH);
    }

  if (ReturnVal = init_form(this_form))
    {
    // erase scroll forms if initialization returned no entries
    if (Current.Form->attrib & FORMATTR_SCROLLING)
      {
      erase_form_area(
        set_attributes(Current.FormColorSet->regular.foreground,
        Current.FormColorSet->regular.background), FALSE);
      Current.Form->virtual_row_index = -1;
      Current.Form->display_row_offset = 0;
      }
    return ReturnVal;
    }

  if (Current.Form->attrib & FORMATTR_EXIT_RESTORE)
    {
    if ((Current.SavedArea = save_screen_area(Current.Form->row,
      Current.Form->column, Current.Form->size_in_rows,
      Current.Form->size_in_columns)))
      {
      /* signal out of memory if savedarea is null */

      if (global_attributes)
        {
        Current.Form->attrib =
          ((Current.Form->attrib & ~(FORMATTR_GLOBAL_BITS))
          | (*global_attributes & FORMATTR_GLOBAL_BITS));
        }
      }
    }

  Current.Form->status = FORMSTAT_ACTIVE_FIELD;

  draw_form();      /* if scroll form, it will be drawn later */

  return ReturnVal;
}


/* -----------------------------------------------------------------------
/  requires:
/  returns:
/  side effects:
/
/ ----------------------------------------------------------------------- */

BOOLEAN setup_for_nested_form_entry(FORM * DescendForm,
                                    USHORT * global_attributes)
{
  if (push_form_context())
    {
    DescendForm->exit_key_code = Current.Form->exit_key_code;

    if ((Current.Form->attrib & FORMATTR_SCROLLING) &&
       !(DescendForm->attrib & FORMATTR_INDEP))
      {
      DescendForm->display_row_offset = Current.Form->display_row_offset;
      DescendForm->virtual_row_index =  Current.Form->virtual_row_index;
      }
    set_bit(DescendForm->attrib, FORMATTR_NESTED);

    // check for bad initialization
    if (setup_for_form_entry(DescendForm, global_attributes))
      {
      exit_from_nested_form();
      Current.Form->field_index = 0;   // restart calling form
      init_field();
      return TRUE;
      }

    return FALSE;
    }
  else
      return(TRUE);
}

/* -----------------------------------------------------------------------
/  function:
/  requires:
/  returns:
/  side effects:
/
/ ----------------------------------------------------------------------- */

UCHAR core_run_form(FORM * pForm, USHORT * global_attrib,
                    BOOLEAN ResumeOperation)
{
  BOOLEAN repeat_shutdown;
  USHORT  TempIndex, TfieldIdx;

  if (!ResumeOperation)
    {
    if (setup_for_form_entry(pForm, global_attrib))
      return(Current.Form->exit_key_code);
    }

  if (Current.Form->attrib & FORMATTR_OVERSTRIKE)
    set_cursor_type(CURSOR_TYPE_OVERSTRIKE);
  else
    set_cursor_type(CURSOR_TYPE_NORMAL);

  TempIndex = pForm->MacFormIndex;
  if ((KSActivateFormRecord != NULL) && (TempIndex != KSI_NO_INDEX))
    {
    if (*pKSRecord &&
      ! ((pForm->attrib & FORMATTR_SCROLLING) &&
      scroll_refresh_only_flag))
      (*KSActivateFormRecord)(TempIndex, FALSE);
    }

  TfieldIdx = 0;
  do
    {
    if (init_field())
      {
      TfieldIdx++;
      Current.Form->status = FORMSTAT_ACTIVE_FIELD;

      if (Current.Field->attrib & FLDATTR_DISPLAY_ONLY)
        {
        run_D_O_field();
        }
      else if (Current.Field->type == FLDTYP_LOGIC)
        {
        run_LOGIC_field();
        }
      else if (Current.Field->type == FLDTYP_FORM)
        {
        if (setup_for_nested_form_entry(
          ((FORM *) Current.FieldDataPtr), &Current.Form->attrib))
          {
          run_D_O_field();
          }

        /* check to make sure that the form initialization came */
        /* out OK before recording an active form */
        if (Current.Form->status == FORMSTAT_ACTIVE_FIELD)
          TempIndex = Current.Form->MacFormIndex;
        else
          {
          TempIndex = KSI_NO_INDEX;     // don't record activation
          Current.Form->status = FORMSTAT_ACTIVE_FIELD;
          }

        if ((KSActivateFormRecord != NULL) &&
          (TempIndex != KSI_NO_INDEX))

          {
          // don't record a scroll refresh
          if(*pKSRecord && !((Current.Form->attrib & FORMATTR_SCROLLING) &&
             scroll_refresh_only_flag))
            (*KSActivateFormRecord)(TempIndex, TRUE); // nested form
          }
        }
      else
        run_input_field();

      do
        {
        repeat_shutdown = FALSE;

        if ((Current.Form->status == FORMSTAT_EXIT_ALL_FORMS) ||
          (Current.Form->status == FORMSTAT_EXIT_THIS_FORM) ||
          (Current.Form->status == FORMSTAT_SWITCH_MODE) ||
          (Current.Form->status == FORMSTAT_EXIT_TO_MENU1) ||
          (Current.Form->status == FORMSTAT_EXIT_TO_MENU2))
          {
          if ((Current.Form->exit_function.group != 0) &&
            ! (Current.Form->attrib & FORMATTR_INDEP))
            {
            /* call exit function */
            if ((*Current.FormExitFunction)(0))
              Current.Form->status = FORMSTAT_ACTIVE_FIELD;
            /* if need to rerun form, set ACTIVE_FIELD */
            }

          if (Current.Form->attrib & FORMATTR_NESTED)
            {

            if ((KSDeActivateFormRecord != NULL) &&
              (TempIndex != KSI_NO_INDEX))
              {
              if ((*pKSRecord == TRUE) &&
                ! ((Current.Form->attrib & FORMATTR_SCROLLING) &&
                scroll_refresh_only_flag))
                (*KSDeActivateFormRecord)();
              }

            exit_from_nested_form(); /* performs pop_form_context() */
            repeat_shutdown = TRUE;
            }
          }
        }
      while (repeat_shutdown);
      }
    TempIndex = Current.Form->MacFormIndex;
    }
  while (Current.Form->status == FORMSTAT_ACTIVE_FIELD);

  if (global_attrib != NULL)
    {
    *global_attrib =
      ((*global_attrib & ~(FORMATTR_GLOBAL_BITS))
      | (Current.Form->attrib & FORMATTR_GLOBAL_BITS));
    }

  shutdown_form();

  if ((KSDeActivateFormRecord != NULL) &&
    (TempIndex != KSI_NO_INDEX))
    {
    if ((*pKSRecord == TRUE) &&
      ! ((Current.Form->attrib & FORMATTR_SCROLLING) &&
      scroll_refresh_only_flag))
      (*KSDeActivateFormRecord)();
    // don't want to record the first field and form exit after starting
    else if (*pKSRecord == (USHORT)-1)
      *pKSRecord = TRUE;
    }
  return(Current.Form->exit_key_code);
}


/* -----------------------------------------------------------------------
/  requires:
/  returns:
/  side effects:
/
/ ----------------------------------------------------------------------- */

#ifdef DEBUG_ATTRIBS
#include <bios.h>

#define     WAIT_INTERVAL     4

UCHAR debug_show_attributes(void)
{
   long           now;
   static long    next_showing = 0L;
   char           attrib_char;
   char           string[30];
   UCHAR  row = Current.Form->row;
   UCHAR  column = Current.Form->column;
   UCHAR  attribute;

   _bios_timeofday(_TIME_GETCLOCK, &now);

   if (next_showing > now)
      return(NULL);

   attribute = set_attributes(Current.FormColorSet->highlight.foreground,
   Current.FormColorSet->highlight.background);
   next_showing = (now + (long) WAIT_INTERVAL);


   if (Current.Form->attrib & FORMATTR_EXIT_ERASE)
      attrib_char = 'E';
   else
      attrib_char = 'e';
   emit(attrib_char, row, column, attribute);
   column++;
   if (Current.Form->attrib & FORMATTR_EXIT_RESTORE)
      attrib_char = 'R';
   else
      attrib_char = 'r';
   emit(attrib_char, row, column, attribute);
   column++;
   if (Current.Form->attrib & FORMATTR_NO_ENTER_VECTOR)
      attrib_char = 'N';
   else
      attrib_char = 'n';
   emit(attrib_char, row, column, attribute);
   column++;
   if (Current.Form->attrib & FORMATTR_FIRST_CHAR_ERASE)
      attrib_char = 'F';
   else
      attrib_char = 'f';
   emit(attrib_char, row, column, attribute);
   column++;
   if (Current.Form->attrib & FORMATTR_VISIBLE)
      attrib_char = 'V';
   else
      attrib_char = 'v';
   emit(attrib_char, row, column, attribute);
   column++;

   sprintf(string, " %d %d %p", Current.Form->field_index,
   Current.Form->number_of_fields, Current.Form);
   display_string(string, strlen(string), row, column, attribute);

   return(NULL);
}

#endif


/* -----------------------------------------------------------------------
/
/  function:   This is the function the application program calls
/              to execute a form.
/  requires:   (FORM *) this_form - a pointer to the form
/              (USHORT *) global_attributes - a pointer
/              to a variable where the form attribute bits can be
/              stored which are global across all forms.  These
/              bits are masked so that bits unique to each form
/              are not included.  If this feature is not desired,
/              call with a NULL pointer.
/
/              ResumeOperation - Added so that if a keystroke macro stops
/                 in the middle of using a form, the initializing sequence
/                 will not be done again.  Assumes that macro keeps track
/                 of all forms levels by pushing and popping current state
/                 as needed.
/  returns:    (UCHAR) - the last key pressed in the form;
/              i.e., the key that caused the form to exit.
/  side effects:
/
/ ----------------------------------------------------------------------- */

UCHAR run_form(FORM * pForm, USHORT * global_attrib, BOOLEAN ResumeOperation)
{
  UCHAR exit_key, ReturnStatus;
  SHORT previous_locus;

#ifdef DEBUG_ATTRIBS
  keyboard_idle = debug_show_attributes;
#endif

  if (! ResumeOperation)
    {
    if (! push_form_context())
      {
      /* signal out of memory error ! */
      if (error_handler)
        (*error_handler)(1); /* ERROR_ALLOC_MEM */
      return(KEY_ESCAPE);
      }

    previous_locus = active_locus;
    active_locus = LOCUS_FORMS;

    exit_key = core_run_form(pForm, global_attrib,
      ResumeOperation);
    active_locus = previous_locus;
    }
  else
    {
    exit_key = core_run_form(pForm, global_attrib,
      ResumeOperation);
    }

  /* If need to exit nested loops, this will set the Current.Form->status */
  /* to show the exit */
  ReturnStatus = 0;
  if((Current.Form->status == FORMSTAT_EXIT_ALL_FORMS) ||
     (Current.Form->status == FORMSTAT_SWITCH_MODE) ||
     (Current.Form->status == FORMSTAT_EXIT_TO_MENU1) ||
     (Current.Form->status == FORMSTAT_EXIT_TO_MENU2))
    ReturnStatus = Current.Form->status;

  /* return to the previous form */
  pop_form_context();

  if (ReturnStatus)
    Current.Form->status = ReturnStatus;

#ifdef DEBUG_ATTRIBS
  keyboard_idle = NULL;
#endif

  return(exit_key);
}

/* -----------------------------------------------------------------------
/  function:   calls the screen primitive initialization routine,
/              and any other setup required before the application
/              begins to use the forms.
/  requires:   (void)
/  returns:    (BOOLEAN) - TRUE if form system initialized properly
/  side effects:
/
/ ----------------------------------------------------------------------- */

BOOLEAN initialize_form_system(void)
{
  init_display_device();

  InitTextCursors();
  set_cursor_type(CURSOR_TYPE_NORMAL);

#ifdef CGIBIND_INCLUDED
  set_mouse_cursor_type(CArrow);
#endif

  return(TRUE);
}

/* -----------------------------------------------------------------------
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void shutdown_form_system(void)
{
  set_cursor_type(CURSOR_TYPE_NORMAL);
}

/* -----------------------------------------------------------------------
/  function: disables field; when user tries to go to field, the next
/            field along the exit vector path will be selected instead,
/            and if desired, redisplays field with the reverse video
/            attribute removed to indicate it is disabled
/  requires: FORM *    - pointer to form where field belongs
/            Index     - array index of field list for form
/            UnReverse - boolean, whether to remove reverse video attrib
/                        for field.
/  returns:  void
/ ------------------------------------------------------------------------ */
void disable_field(FORM * Form, SHORT Index, BOOLEAN UnReverse)
{
  if (Form && Form->fields)
    {
    Form->fields[Index].attrib |= FLDATTR_DISABLED;
    if (UnReverse)
      {
      Form->fields[Index].attrib &= ~FLDATTR_REV_VID;
      if (Form == Current.Form)
        display_random_field(Form, Index);
      }
    }
}

/* -----------------------------------------------------------------------
/  function: enables field; removes the DISABLED attribute, and if desired
/            redisplays field in reverse video to indicate it is enabled
/  requires: FORM *    - pointer to form where field belongs
/            Index     - array index of field list for form
/            UnReverse - boolean, whether to remove reverse video attrib
/                        for field.
/  returns:  void
/ ------------------------------------------------------------------------ */
void enable_field(FORM * Form, SHORT Index, BOOLEAN UnReverse)
{
  if (Form && Form->fields)
    {
    Form->fields[Index].attrib &= ~FLDATTR_DISABLED;
    if (UnReverse)
      {
      Form->fields[Index].attrib |= FLDATTR_REV_VID;
      if (Form == Current.Form)
        display_random_field(Form, Index);
      }
    }
}

/* -----------------------------------------------------------------------
/  function:  Updates the display of a field on a form that has already
/             been initialized.
/  requires:
/  returns:    (BOOLEAN) - TRUE if field initialized properly
/  side effects:
/
/ ----------------------------------------------------------------------- */

BOOLEAN display_random_field(FORM *Form, SHORT FieldIndex)
{
  USHORT OldFieldIndex;

  if (!push_form_context())
    return FALSE;

  Current.Form = Form;

  OldFieldIndex = Current.Form->field_index;
  Current.Form->field_index = FieldIndex;
  init_field();
  format_and_display_field(FALSE);

  Current.Form->field_index = OldFieldIndex;
  init_field();

  pop_form_context();
  if (Current.Form->number_of_fields)
    init_field();
  return TRUE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void playFieldForMacro()
{
  Current.Form->status = FORMSTAT_ACTIVE_FIELD;

  if (Current.Field->attrib & FLDATTR_DISPLAY_ONLY)
    run_D_O_field();

  else if (Current.Field->type == FLDTYP_LOGIC)
    run_LOGIC_field();

  else if (Current.Field->type == FLDTYP_FORM)
    {
    if (setup_for_nested_form_entry(((FORM *) Current.FieldDataPtr),
      &Current.Form->attrib))
      run_D_O_field();
    }
  else
    run_input_field();
}

/**************************************************************************/
/*                   end of file                                          */
/**************************************************************************/
