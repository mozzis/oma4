#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "forms.h"
  
/* -----------------------------------------------------------------------
/  spinfld.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: MLM   Version 1.00  13 Aug 1993
/
*/ /*
  $Header:   J:/logfiles/forms/spinfld.c_v   1.1   13 Aug 1993 13:47:08  maynard
  $Log:   J:/logfiles/forms/spinfld.c_v  $
/ ----------------------------------------------------------------------- */
  
/******************************************************************/  
void spin_to_field()
{
  unsigned char index;
  char spintext[32];

  if (Current.FieldData->type == DATATYP_LONG_INT)
    {
    index = (unsigned char) *((long *) Current.FieldDataPtr);
    }
  else
    {
    index = (unsigned char) *((int *) Current.FieldDataPtr);
    }
  if (index > Current.Field->specific.tglfld.total_items)
    index = Current.Field->specific.tglfld.total_items;

  Current.Field->specific.tglfld.item_index = index;
  sprintf(spintext, "%d", index);

  string_to_field_string(spintext);

  Current.Form->display_cursor_offset = 0;
  Current.Form->string_cursor_offset = 0;
}
  
/******************************************************************/  
int spin_from_field()
{
  if (Current.FieldData->type == DATATYP_LONG_INT)
    {
    *((long *) Current.FieldDataPtr) =
      (long) Current.Field->specific.tglfld.item_index;
    }
  else
    {
    *((int *) Current.FieldDataPtr) =
      (int) Current.Field->specific.tglfld.item_index;
    }
  return(FIELD_VALIDATE_SUCCESS);
}
  
/******************************************************************/  
BOOLEAN spin_field_char_action(unsigned char key)
{
  if (key)
    {
    switch (key)
      {
      case KEY_DELETE_FAR:
      case KEY_HOME:
        Current.Field->specific.tglfld.item_index = 0;
      break;

      case KEY_END:
        Current.Field->specific.tglfld.item_index = 0;  /* no break; */
      case KEY_BACKSPACE:
      case KEY_LEFT:
        if(Current.Field->specific.tglfld.item_index == 0)
          {
          Current.Field->specific.tglfld.item_index =
            (Current.Field->specific.tglfld.total_items - (char) 1);
          }
        else
          {
          Current.Field->specific.tglfld.item_index --;
          }
      break;

      default:
        if (++Current.Field->specific.tglfld.item_index
          >=  Current.Field->specific.tglfld.total_items)
          {
          Current.Field->specific.tglfld.item_index = 0;
          }
      }

    string_to_field_string(((char **) Current.FieldAltDataPtr)[
      Current.Field->specific.tglfld.item_index]);

    Current.Form->display_cursor_offset = 0;
    Current.Form->string_cursor_offset = 0;
    display_field_to_screen(TRUE);
    }
  return(TRUE);
}
  
void uses_spin_fields()
{
   FIELD_CLASS *     FieldClassPtr;
  
   FieldClassPtr = &FieldClassArray[FLDTYP_TOGGLE];
  
   FieldClassPtr->TypeBit = FTB_TOGGLE;
   FieldClassPtr->field_char_action = spin_field_char_action;
   FieldClassPtr->data_to_field = spin_to_field;
   FieldClassPtr->data_from_field = spin_from_field;
   FieldClassPtr->data_limit_action = NULL;
}
