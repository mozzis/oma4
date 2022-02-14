#include <ctype.h>
#include <string.h>
#include <stddef.h>  // NULL

#include "forms.h"
  
/* -----------------------------------------------------------------------
/
/  tglfld.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB   Version 1.00  1-8 May   1988
/  Worked on:  RAC   Version 1.01   26 June  1990
/              TLB   Version 1.02    9 July  1990
/
*/ /*
  $Header:   J:/logfiles/forms/tglfld.c_v   1.1   10 Jan 1992 13:47:08   cole  $
  $Log:   J:/logfiles/forms/tglfld.c_v  $
 * 
 *    Rev 1.1   10 Jan 1992 13:47:08   cole
 * Add include <stddef.h>
 * 
 *    Rev 1.0   27 Sep 1990 15:44:32   admax
 * Initial revision.
*/ /*
/ ----------------------------------------------------------------------- */
  
/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void toggle_to_field(void)
{
   unsigned char index;
  
   if (Current.FieldData->type == DATATYP_LONG_INT)
   {
      index = (unsigned char) *( (long *) Current.FieldDataPtr);
   }
   else
   {
      index = (unsigned char) *( (int *) Current.FieldDataPtr);
   }
   if (index > Current.Field->specific.tglfld.total_items)
      index = Current.Field->specific.tglfld.total_items;
  
   Current.Field->specific.tglfld.item_index = index;
  
   string_to_field_string(((char **) Current.FieldAltDataPtr)[index]);
  
   Current.Form->display_cursor_offset = 0;
   Current.Form->string_cursor_offset = 0;
}
  
int toggle_from_field(void)
{
   if (Current.FieldData->type == DATATYP_LONG_INT)
   {
      *( (long *) Current.FieldDataPtr) =
      (long) Current.Field->specific.tglfld.item_index;
   }
   else
   {
      *( (int *) Current.FieldDataPtr) =
      (int) Current.Field->specific.tglfld.item_index;
   }
   return(FIELD_VALIDATE_SUCCESS);
}
  
/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
int match_start_of_string(char * full_string, char * part_string)
{
  char		full;
  char		part;
  int		matches = 0;

  if ( (*part_string == 0) || (*full_string == 0) )
    return matches;

  do
    {
    /* need to ignore all spaces */
    full = *full_string++;
    if (full != ' ')
      {
      do
        {
        part = *part_string++;
        }
      while ( (part == ' ') && (*part_string != 0) );
      if (part != ' ')
        {
        if ( toupper(full) == toupper(part) )
          matches++;
        else
          break;
        }
      }
    }
  while ( (*full_string != 0) && (*part_string != 0) );

	return(matches);
}
  
BOOLEAN toggle_field_char_action(unsigned char key)
{
  static int     match_str_count;
  static char    match_str[41];
  struct TglParam * Toggle = &(Current.Field->specific.tglfld);

  if (key)
    {  // allow user to select toggle item by
       // typing in text - field will "hunt" for closest match
    if (isalnum(key) || ispunct(key))
      {
      char *         item_string;
      unsigned char  best_match_index,  i;
      int            match_count, try_count, best_match_count;
      BOOLEAN        good_match;

      if (match_str_count >= 40)
        match_str_count = 0;       // reset to prevent overflow

      try_count = 0;
      good_match = FALSE;
      do
        {
        best_match_count = 0;

        match_str[match_str_count++] = key; // add to compare string
        match_str[match_str_count] = 0;

        for (i=0; i<Toggle->total_items; i++)
          {
          item_string = ((char **) Current.FieldAltDataPtr)[i];

          match_count = match_start_of_string(item_string, match_str);
          if (match_count > best_match_count)
            {
            best_match_index = i;
            best_match_count = match_count;
            }
          }
        if (best_match_count == match_str_count)
          {
          Toggle->item_index = best_match_index;
          good_match = TRUE;
          match_str_count = best_match_count;
          }
        else
          {
          match_str_count = 0;
          }
        try_count++;
        }
      while ((try_count < 2) && ( ! good_match ));
      }
    else
      {
      match_str_count = 0;

      switch (key)
        {
        case KEY_DELETE_FAR:
        case KEY_HOME:
          Toggle->item_index = 0;
        break;

        case KEY_END:
          Toggle->item_index = 0;  /* no break; */
        case KEY_BACKSPACE:
        case KEY_LEFT:
          if( Toggle->item_index == 0 )
            Toggle->item_index = (Toggle->total_items - (char) 1);
          else
            Toggle->item_index -- ;
        break;

        default:
          if ( ++Toggle->item_index >=  Toggle->total_items)
            Toggle->item_index = 0;
        }
      }

    string_to_field_string(((char **)Current.FieldAltDataPtr)[Toggle->item_index]);

    Current.Form->display_cursor_offset = 0;
    Current.Form->string_cursor_offset = 0;
    display_field_to_screen(TRUE);
    }
  else   /* function was called to do self-initialization */
    {
    match_str_count = 0;
    }
  return(TRUE);
}
  
void uses_toggle_fields(void)
{
  FIELD_CLASS *     FieldClassPtr;
 
  FieldClassPtr = &FieldClassArray[FLDTYP_TOGGLE];
 
  FieldClassPtr->TypeBit = FTB_TOGGLE;
  FieldClassPtr->field_char_action = toggle_field_char_action;
  FieldClassPtr->data_to_field = toggle_to_field;
  FieldClassPtr->data_from_field = toggle_from_field;
  FieldClassPtr->data_limit_action = NULL;
}
