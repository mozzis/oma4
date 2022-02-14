#include <ctype.h>
#include <string.h>
#include <stddef.h> // NULL

#include "forms.h"
  
/* -----------------------------------------------------------------------
/
/  frmatfld.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00        1-8 May         1988
/  Worked on:  TLB      Version 1.01
/
*/ /*
  $Header:   J:/logfiles/forms/frmatfld.c_v   1.1   10 Jan 1992 13:31:24   cole  $
  $Log:   J:/logfiles/forms/frmatfld.c_v  $
 * 
 *    Rev 1.1   10 Jan 1992 13:31:24   cole
 * Add include <stddef.h>
 * 
 *    Rev 1.0   27 Sep 1990 15:40:46   admax
 * Initial revision.
*/ /*
/ ----------------------------------------------------------------------- */
  
// #define  NULL           0        // 5/29/90 DAI
// #define  NOT            !
// #define  TRUE           1
// #define  FALSE          0
  
#define fmt_input_char(x) ((x=='?') || (x=='~') || (x=='#'))
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void data_to_format_field()
{
   BOOLEAN  source_string_null_found = FALSE;
   BOOLEAN  input_position_found = FALSE;
   int      i;
   int      size;
   char     ch;
   char     fmt_ch;
   char     insert_ch;
   char *   format;
   char *   source;
  
   source = ((char *) Current.FieldDataPtr);
   format = ((char *) Current.FieldAltDataPtr);
   size = ((int) Current.Field->width);
  
   Current.Form->field_overfull_flag = FALSE;
   Current.Form->string_cursor_offset = 0;
   Current.Form->display_cursor_offset = 0;
  
   Current.Form->field_char_count = 0;
  
   for (i=0; i<size; i++)
   {
      if (source_string_null_found)
      {
         ch = SPACE;
      }
      else if ((ch = source[i]) == '\x00')
      {
         source_string_null_found = TRUE;
         ch = SPACE;
      }
  
      if ((fmt_ch = format[i]) == '\x00')
         break;
  
      switch (fmt_ch)
      {
         case '?':
            insert_ch = ch;
            input_position_found = TRUE;
            break;
  
         case '#':
            if (isdigit(ch))
               insert_ch = ch;
            else
               insert_ch = '0';
            input_position_found = TRUE;
            break;
  
         case '~':
            /*          if (isalpha(ch) || (ch == SPACE)) */
            if (isalpha(ch))
               insert_ch = ch;
            else
               insert_ch = SPACE;
            input_position_found = TRUE;
            break;
  
         default:
            insert_ch = fmt_ch;
      }
      Current.FieldString[Current.Form->field_char_count++] = insert_ch;
   }
   Current.FieldString[Current.Form->field_char_count] = 0;
  
   if ( ! input_position_found)
      set_bit(Current.Field->attrib, FLDATTR_DISPLAY_ONLY);
  
   set_bit(Current.Field->attrib, FLDATTR_NO_OVERFLOW_CHAR);
  
   update_display_string();
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
int data_from_format_field()
{
   strncpy((char *) Current.FieldDataPtr, Current.FieldString,
   Current.Form->field_char_count);
  
   ((char *) Current.FieldDataPtr)[Current.Form->field_char_count] = '\x00';
  
   return(FIELD_VALIDATE_SUCCESS);
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void format_field_cursor_left(char * format)
{
   int i;
  
   if (Current.Form->string_cursor_offset > 0)
   {
      --Current.Form->string_cursor_offset;
  
      i = (int) Current.Form->string_cursor_offset;
  
      for (; i>=0; i--)
      {
         if (fmt_input_char(format[i]))
         {
            Current.Form->string_cursor_offset = (unsigned char) i;
            break;
         }
      }
   }
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void format_field_cursor_right(char * format)
{
   int   i;
   int   limit = (int) (Current.Form->field_char_count - 1);
  
   if (Current.Form->string_cursor_offset < (unsigned char) limit)
   {
      i = (int) (++Current.Form->string_cursor_offset);
  
      for (; i<limit; i++)
      {
         if (fmt_input_char(format[i]))
         {
            Current.Form->string_cursor_offset = (unsigned char) i;
            break;
         }
      }
   }
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
BOOLEAN format_field_char_action(unsigned char key)
{
   BOOLEAN key_used_by_field = TRUE;
   static char * format;
  
   if (key)
   {
      if (key & KEYS_HIGH_BIT)
      {
         switch (key)
         {
            case KEY_HOME:
               Current.Form->string_cursor_offset = 0;
  
               if ( ! fmt_input_char(format[0]) )
                  format_field_cursor_right(format);
               break;
  
            case KEY_END:
            {
               int position = (Current.Form->field_char_count - 1);
  
               Current.Form->string_cursor_offset = (unsigned char) position;
  
               if ( ! fmt_input_char(format[position]))
                  format_field_cursor_left(format);
               break;
            }
            case KEY_BACKSPACE:
            case KEY_LEFT:
               format_field_cursor_left(format);
               break;
  
            case KEY_RIGHT:
               format_field_cursor_right(format);
               break;
            default:
               key_used_by_field = FALSE;
         }
      }
      else
      {
         char insert_ch = '\x00';
  
         switch ( format[Current.Form->string_cursor_offset] )
         {
            case '?':
               insert_ch = key;
               break;
  
            case '#':
               if (isdigit(key))
                  insert_ch = key;
               break;
  
            case '~':
               if (isalpha(key) || (key == SPACE))
                  insert_ch = key;
               break;
         }
         if (insert_ch != '\x00')
         {
            Current.FieldString[Current.Form->string_cursor_offset] = insert_ch;
            format_field_cursor_right(format);
         }
      }
      Current.Form->display_cursor_offset =
      Current.Form->string_cursor_offset;
      update_display_string();
      display_field_to_screen(TRUE);
   }
   else     /* function was called to do self-initialization */
   {
      format = ((char *) Current.FieldAltDataPtr);
   }
   return(key_used_by_field);
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void uses_format_fields()
{
   FIELD_CLASS * FieldClassPtr;
  
   FieldClassPtr = &FieldClassArray[FLDTYP_FORMAT];
  
   FieldClassPtr->TypeBit = (FTB_FORMAT | FTB_STRING);
   FieldClassPtr->field_char_action = format_field_char_action;
   FieldClassPtr->data_to_field = data_to_format_field;
   FieldClassPtr->data_from_field = data_from_format_field;
   FieldClassPtr->data_limit_action = NULL;
}
  
  
  

