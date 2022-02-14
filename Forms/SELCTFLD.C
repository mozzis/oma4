 
/* -----------------------------------------------------------------------
/
/  selctfld.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00        23  July     1989
/  Worked on:  TLB      Version 1.01        13  October  1989
/                       Version 1.02
/
  $Header:   J:/logfiles/forms/selctfld.c_v   1.1   10 Jan 1992 13:45:22   cole  $
  $Log:   J:/logfiles/forms/selctfld.c_v  $
 * 
 *    Rev 1.1   10 Jan 1992 13:45:22   cole
 * Add include <stddef.h>. delete commented out define's for NULL, NOT, TRUE,
 *   FALSE. delete commented out typedef int BOOLEAN.
 * 
 *    Rev 1.0   27 Sep 1990 15:43:36   admax
 * Initial revision.
/ ----------------------------------------------------------------------- */
  
#include <stddef.h>  /* NULL */

#include "forms.h"
 
/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void select_to_field()
{
   unsigned char  match_value;
   BOOLEAN        selected;
  
   if (Current.FieldData->type == DATATYP_LONG_INT)
   {
      match_value = (unsigned char) *( (long *) Current.FieldDataPtr);
   }
   else
   {
      match_value = (unsigned char) *( (int *) Current.FieldDataPtr);
   }
  
   if (Current.Field->specific.selfld.match_value == 0)
      selected = (match_value != 0);
   else
      selected = (Current.Field->specific.selfld.match_value == match_value);
  
   if (selected)
      reset_bit(Current.Field->attrib, FLDATTR_SHADED);
   else
      set_bit(Current.Field->attrib, FLDATTR_SHADED);
  
   string_to_field_string( (char *) Current.FieldAltDataPtr );
}
  
int select_from_field()
{
   return(FIELD_VALIDATE_SUCCESS);
}
  
  
/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
BOOLEAN select_field_char_action(unsigned char key)
{
   BOOLEAN  key_used_by_field = TRUE;
  
   if (key)
   {
      unsigned char  match_value;
      BOOLEAN        selected;
  
      if (Current.FieldData->type == DATATYP_LONG_INT)
      {
         match_value = (unsigned char) *( (long *) Current.FieldDataPtr);
      }
      else
      {
         match_value = (unsigned char) *( (int *) Current.FieldDataPtr);
      }
  
      if (Current.Field->specific.selfld.match_value == 0)
         selected = (match_value != 0);
      else
         selected = (Current.Field->specific.selfld.match_value == match_value);
  
  
      switch (key)
      {
         case KEY_ENTER:
            if (selected)
               selected = FALSE;
            else
               selected = TRUE;
            key_used_by_field = FALSE; /* we use it, but pretend we didn't */
            break;
  
         case KEY_BACKSPACE:
         case KEY_DELETE:
            selected = FALSE;
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
         default:
            key_used_by_field = FALSE;
      }
      if (selected)
         reset_bit(Current.Field->attrib, FLDATTR_SHADED);
      else
         set_bit(Current.Field->attrib, FLDATTR_SHADED);
  
      if (Current.Field->specific.selfld.match_value == 0)
         match_value = (unsigned char) selected;
      else
      {
         if (selected)
            match_value = Current.Field->specific.selfld.match_value;
         else
            match_value = 0;
      }
  
      if (Current.FieldData->type == DATATYP_LONG_INT)
      {
         *( (long *) Current.FieldDataPtr) = (long) match_value;
      }
      else
      {
         *( (int *) Current.FieldDataPtr) = (int) match_value;
      }
  
      display_field_to_screen(TRUE);
   }
   return(key_used_by_field);
}
  
  
void uses_select_fields()
{
   FIELD_CLASS *     FieldClassPtr;
  
   FieldClassPtr = &FieldClassArray[FLDTYP_SELECT];
  
   FieldClassPtr->TypeBit = FTB_SELECT;
   FieldClassPtr->field_char_action = select_field_char_action;
   FieldClassPtr->data_to_field = select_to_field;
   FieldClassPtr->data_from_field = select_from_field;
   FieldClassPtr->data_limit_action = NULL;
}
