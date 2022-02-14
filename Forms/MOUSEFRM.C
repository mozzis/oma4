/* -----------------------------------------------------------------------
/
/  mousefrm.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00   6  October  1989
/  Worked on:  TLB      Version 1.01
/
*/ /*
  $Header:   J:/logfiles/forms/mousefrm.c_v   1.10   28 Aug 1991 18:06:14   cole  $
  $Log:   J:/logfiles/forms/mousefrm.c_v  $
*/ /*
/ ----------------------------------------------------------------------- */

#include <sys\types.h>
#include <sys\timeb.h>

#include <stdio.h>

#include "ksindex.h"
#include "formwind.h"
#include "barmenu.h"
#include "forms.h"

typedef struct field_ref   // added to accomodate searching scroll forms
{
  int Index;
  int ScrollRowOffset;
} FIELD_REF;

typedef struct menu_ref   // added to accomodate searching menus
{
  CHAR ActivatingChar;
  UCHAR MenuLevel;
} MENU_REF;

unsigned char (*application_mouse_service)(unsigned int buttons,
               int row, int column, int XPos, int YPos, BOOLEAN *LeftUp,
               BOOLEAN *RightUp, BOOLEAN *BothUp, BOOLEAN *LeftDrag,
               BOOLEAN *RightDrag, BOOLEAN *BothDrag);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN is_cursor_in_menus(UCHAR row, UCHAR column, MENU_REF *pMenuRef)
{
  MENUCONTEXT *TempFocus;
  MENUITEM *pMenuItem;
  SHORT i;
  unsigned char  last_column;
  BOOLEAN Found = FALSE;

  TempFocus = &MenuFocus;

  while ((TempFocus != NULL) && !Found)
    {
    if (row == (unsigned char) TempFocus->Row)
      {
      for (i=0; (i < TempFocus->ActiveMENU->ItemCount) && !Found; i++)
        {
        pMenuItem = &TempFocus->ActiveMENU->ItemList[i];

        if (! (pMenuItem->Control & MENUITEM_INACTIVE))
          {
          last_column = pMenuItem->Column + pMenuItem->TextLen + (char)2;

          if (((char)column >= pMenuItem->Column) &&
            (column < last_column))
            {
            pMenuRef->ActivatingChar =
              pMenuItem->Text[ pMenuItem->SelectCharOffset ];
            Found = TRUE;
            }
          }
        }
      }

    if (!Found)
      TempFocus = TempFocus->PriorContext;
    }

  if (TempFocus != NULL)     // Found a menu item
    {
    if (TempFocus->Row == 1)
      pMenuRef->MenuLevel = 2;
    else
      pMenuRef->MenuLevel = 1;
    }
  else
    pMenuRef->ActivatingChar = KEY_EXCEPTION;

  return Found;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned char menus_left_button_handler(unsigned char row, unsigned char col)
{
  MENU_REF MenuRef;

  /* check to see if any menu level */
  if (! is_cursor_in_menus(row, col, &MenuRef))
    return KEY_EXCEPTION;

  switch (MenuRef.MenuLevel)
    {
    case 1:
      Current.Form->status = FORMSTAT_EXIT_TO_MENU1;
    break;

    case 2:
      Current.Form->status = FORMSTAT_EXIT_TO_MENU2;
    break;
    }
  return (UCHAR) MenuRef.ActivatingChar;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FIELD_REF is_cursor_in_this_forms_fields(FORM * pForm, UCHAR row, UCHAR col)
{
  FIELD *  pField;
  int      i;
  int      field_min_column;
  int      field_max_column;
  FIELD_REF FoundField = {-1, 0};

  /* is form currently visible? */
  if ((pForm->attrib & FORMATTR_VISIBLE) || (pForm->attrib & FORMATTR_SCROLLING))
    {
  /* is cursor within the form area? */
    if(((row >= pForm->row) &&
      (row <= (pForm->row + pForm->size_in_rows))) &&
      ((col >= pForm->column) &&
      (col <= (pForm->column + pForm->size_in_columns))))
  /* yes! look at all fields within form */
      {
      for (i=(pForm->number_of_fields - 1); i>=0; i--)
        {
        pField = &pForm->fields[i];
  /* is field valid to activate? */
        if(!((pField->attrib & FLDATTR_DISPLAY_ONLY) ||
          ((pField->attrib & FLDATTR_GET_DRAW_PERMISSION) &&
          (!(pField->attrib & FLDATTR_DRAW_PERMITTED))) ||
          (pField->type == FLDTYP_LOGIC) ||
          (pField->type == FLDTYP_FORM) ||
          (pField->attrib & FLDATTR_DISABLED)))
          {
  /* is cursor on this field's row? */
          if((row == (pForm->row + pField->row)) ||
            (pForm->attrib & FORMATTR_SCROLLING))
            {
            field_min_column = (pField->column + pForm->column);
            field_max_column = (field_min_column + pField->width);
  /* is cursor within columns of field? */
            if(((int) col >= field_min_column) &&
              ((int) col <= field_max_column) &&
              !(pField->attrib & FLDATTR_DISPLAY_ONLY))
              {
  /* YES! found it! */
              FoundField.ScrollRowOffset = row - pForm->row;
              FoundField.Index = i;
              return(FoundField);
              }
            }
          }  // check field type
        }     // for loop

  /* if in a scroll form find the first actionable field to focus on */
        if((FoundField.Index == -1) && (pForm->attrib & FORMATTR_SCROLLING))
          {
          for(i=0; i<pForm->number_of_fields && FoundField.Index == -1; i++)
            {
            pField = &pForm->fields[i];
            if(!((pField->attrib & FLDATTR_DISPLAY_ONLY) ||
                (pField->type == FLDTYP_LOGIC) ||
                (pField->type == FLDTYP_FORM)))
              {
              FoundField.Index = i;
              FoundField.ScrollRowOffset = row - pForm->row;
              }
            }
          }
        }  // if in form
      }     // if form visible
    return(FoundField);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN is_cursor_in_child_form(FIELD_REF * routing_list, FORM * formptr,
                                unsigned char row, unsigned char column)
{
   int            i;
   FIELD_REF      field_index;
   BOOLEAN        found_target = FALSE;
   FORM *         DescendForm;
   FIELD *        DescendField;

   /* do recursive search of subforms */

   for (i=(formptr->number_of_fields - 1); i>=0; i--)
   {
      DescendField = &formptr->fields[i];

      if ((DescendField->type == FLDTYP_FORM)
         && (DescendField->primary_data.group != 0))
      {
         DescendForm = resolve_address(& formptr->dataRegistry
         [DescendField->primary_data.group]
         [DescendField->primary_data.item]);

         if ((DescendForm->attrib & FORMATTR_VISIBLE) ||
              (DescendForm->attrib & FORMATTR_SCROLLING))
         {
            routing_list[0].Index++;   /* take tentative step forward */
            routing_list[ routing_list[0].Index ].Index = i;

            field_index =
            is_cursor_in_this_forms_fields(DescendForm, row, column);

            if (field_index.Index == -1)
            {
               if (is_cursor_in_child_form(routing_list, DescendForm,
                  row, column))
               {
                  found_target = TRUE;    /* found at a lower level */
                  break;
               }
               else
                  routing_list[0].Index--;      /* false step - retreat */
            }
            else
            {
               if (DescendForm->attrib & FORMATTR_SCROLLING)
                  routing_list[ routing_list[0].Index ].ScrollRowOffset =
                     field_index.ScrollRowOffset;
               routing_list[0].Index++;
               routing_list[ routing_list[0].Index ] = field_index;
               found_target = TRUE;
               break;
            }
         }
      }
   }
   return (found_target);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN is_cursor_in_other_form_field(FIELD_REF* routing_list,
                                    unsigned char row, unsigned char column)
{
   FIELD_REF         field_index;
   BOOLEAN           found_target = FALSE;
   FORM_CONTEXT *    form_context_stack_ptr;
   FORM *            AscendForm;

   /* search for field: first in children of this form... */

   found_target =
   is_cursor_in_child_form(routing_list, Current.Form, row, column);

   if (! found_target)  /* then go back to farthest ancestor, */
   {                       /* search all descendents (besides current) */
      AscendForm = NULL;
      form_context_stack_ptr = &Current;

      while (((form_context_stack_ptr =
         form_context_stack_ptr->PreviousStackedContext) != NULL)
         && (form_context_stack_ptr->Form != NULL)
         && ((form_context_stack_ptr->Form->attrib & FORMATTR_VISIBLE) ||
              (form_context_stack_ptr->Form->attrib & FORMATTR_SCROLLING)))
      {
         AscendForm = form_context_stack_ptr->Form;
         routing_list[0].Index++;
         routing_list[ routing_list[0].Index ].Index = -1;
      }
      if (AscendForm != NULL)
      {
         field_index =
         is_cursor_in_this_forms_fields(AscendForm, row, column);

         if (field_index.Index != -1)
         {
            routing_list[0].Index++;
            routing_list[ routing_list[0].Index ] = field_index;
            found_target = TRUE;
         }
         else
         {
            found_target =
              is_cursor_in_child_form(routing_list, AscendForm, row, column);
         }
      }
      if (form_context_stack_ptr)
        {
        if (! form_context_stack_ptr->Form->attrib & FORMATTR_SCROLLING)
          set_bit(Current.Form->attrib, FORMATTR_VISIBLE);
        }
   }
   return (found_target);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN descend_into_child_form(FIELD_REF field_index)
{
   FORM * DescendForm;
   BOOLEAN ReturnVal;
   int OldFieldIndex;
   USHORT TempIndex;

   OldFieldIndex = Current.Form->field_index;
   Current.Form->field_index = field_index.Index;

   if (init_field())
   {
      DescendForm = (FORM *) Current.FieldDataPtr;
      if (DescendForm->attrib & FORMATTR_SCROLLING)
      {
         DescendForm->virtual_row_index =
            DescendForm->virtual_row_index -
            (USHORT) DescendForm->display_row_offset +
            (USHORT) field_index.ScrollRowOffset;
         DescendForm->display_row_offset =
            (UCHAR) field_index.ScrollRowOffset;
      }
      if (ReturnVal = setup_for_nested_form_entry(
                  (FORM *) Current.FieldDataPtr,
                  &Current.Form->attrib))
      {              // error initializing form 
         Current.Form->field_index = OldFieldIndex;
         init_field();
      }
      else
      {
         TempIndex = Current.Form->MacFormIndex;

         if ((KSActivateFormRecord != NULL) &&
              (TempIndex != KSI_NO_INDEX))
         {
            if (*pKSRecord)
               (*KSActivateFormRecord)(TempIndex, TRUE); // nested form
         }
      }

      return ReturnVal;
   }
   else
      return(FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UCHAR new_form_and_field_context(FIELD_REF * routing_list)
{
   int i;
   BOOLEAN FormInitError = FALSE;
   USHORT TempIndex;

   unzoom_field();
   (*Current.FieldClass->data_to_field)();
   reset_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
   display_field_to_screen(FALSE);

   TempIndex = Current.Form->MacFormIndex;

   for (i=1; i<routing_list[0].Index; i++)
   {
      if (routing_list[i].Index < 0)      /* move back to parent form */
      {
         if ((KSDeActivateFormRecord != NULL) &&
               (TempIndex != KSI_NO_INDEX))
         {
            if (*pKSRecord == TRUE)
               (*KSDeActivateFormRecord)();
         }

         reset_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
         format_and_display_field(FALSE);
         shutdown_form();
         exit_from_nested_form(); /* performs pop_form_context() */
         FormInitError = FALSE;
      }
      else                          /* move forward to child form */
      {
         if (! descend_into_child_form(routing_list[i]))
         {
            FormInitError = FALSE;
            break;
         }
         else
            FormInitError = TRUE;
      }
      TempIndex = Current.Form->MacFormIndex;
   }
   /* !!! form may be in undetermined state if error descending!! */
   if (! FormInitError)
   {
      Current.Form->field_index = routing_list[ routing_list[0].Index ].Index;

      if (Current.Form->attrib & FORMATTR_SCROLLING)
      {
         Current.Form->virtual_row_index =
            Current.Form->virtual_row_index - Current.Form->display_row_offset +
            routing_list[ routing_list[0].Index ].ScrollRowOffset;
         Current.Form->display_row_offset =
            (UCHAR) routing_list[ routing_list[0].Index ].ScrollRowOffset;
         if ((*Current.FormInitFunction)(Current.Form->virtual_row_index))
         {
            Current.Form->virtual_row_index = -1;
            Current.Form->display_row_offset = 0;
            Current.Form->field_index = 0;   // reset to starting condition
      
            do
            {
               reset_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
               shutdown_form();
               exit_from_nested_form();      /* performs pop_form_context() */
            }
            while (Current.Form->attrib & FORMATTR_NESTED);
         }
      }
   }

   init_field();
   if ((Current.Field->type == FLDTYP_FORM) ||
        (Current.Field->type == FLDTYP_LOGIC))
   {
      Current.Form->field_index = 0;
      init_field();
      return KEY_EXCEPTION;        // will redraw forms fields
   }

   (*Current.FieldClass->field_char_action)(0);
   set_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
   reset_bit(Current.Field->attrib, FLDATTR_LIMIT_WARN);
   format_and_display_field(TRUE);

   Current.Form->status = FORMSTAT_ACTIVE_FIELD;

   return (UCHAR) FormInitError;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned char forms_left_button_handler(unsigned char row,
                                        unsigned char column)
{
  FIELD_REF field_index;
  FIELD_REF routing_list[25];
  UCHAR Found = FALSE;

  routing_list[0].Index = 0;

  /* search for field: first in this form... */
  field_index = is_cursor_in_this_forms_fields(Current.Form, row, column);

  if (field_index.Index != -1)
    {
    if (field_index.Index == Current.Form->field_index)
      {
      if (Current.Form->attrib & FORMATTR_SCROLLING)
        {
        /* reset old highlighted position */
        reset_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
        format_and_display_field(TRUE);

        /* set focus on new line */
        Current.Form->virtual_row_index =
          Current.Form->virtual_row_index -
          Current.Form->display_row_offset +
          field_index.ScrollRowOffset;
        Current.Form->display_row_offset =
          (UCHAR) field_index.ScrollRowOffset;
        if ((*Current.FormInitFunction)(Current.Form->virtual_row_index))
          {
          Current.Form->virtual_row_index = -1;
          Current.Form->display_row_offset = 0;

          do
            {
            reset_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
            shutdown_form();
            exit_from_nested_form();  /* performs pop_form_context() */
            }
          while (Current.Form->attrib & FORMATTR_NESTED);
          }
        init_field();
        set_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
        format_and_display_field(TRUE);
        }
      return(KEY_ENTER);
      }
    else
      {
      routing_list[0].Index++;
      routing_list[ routing_list[0].Index ] = field_index;
      /* simple job, here */
      Found = new_form_and_field_context(routing_list);
      if (Found == KEY_EXCEPTION)
        return Found;     // signal replay of form
      else
        Found = (UCHAR) ! Found;
      }
    }
  else if (is_cursor_in_other_form_field(routing_list, row, column))
    {
    Found = new_form_and_field_context(routing_list);
    if (Found == KEY_EXCEPTION)
      return KEY_EXCEPTION;     // signal replay of form
    else
      Found = (UCHAR) ! Found;
    }

  if (Found)
    {
    if ((Current.Field->type == FLDTYP_LOGIC) || // one key push on select
      (Current.Field->type == FLDTYP_SELECT) ||// and other logic fields
      (Current.Field->type == FLDTYP_FORM))
      return KEY_ENTER;
    else
      return(0);
    }
  else
    return KEY_EXCEPTION;

}

#define WAIT_INTERVAL 170   /* in milliseconds */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN DebounceMouseKey()
{
   struct timeb now;
   static struct timeb allow_repeat_after = { 0, 0 };

   ftime(&now);

   if ((allow_repeat_after.time > now.time) ||
      ((allow_repeat_after.time == now.time) &&
      (allow_repeat_after.millitm > now.millitm)))
      return(FALSE);

   allow_repeat_after.millitm = (now.millitm + WAIT_INTERVAL);
   allow_repeat_after.time = now.time + (now.millitm / 1000);
   now.millitm %= 1000;
   return TRUE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned char forms_mouse_service(void)
{
  unsigned int   buttons;
  int            row;
  int            column;
  int            XPos, YPos;
  unsigned char  return_key_value = (unsigned char) 0;
  static BOOLEAN LeftUp = TRUE;
  static BOOLEAN RightUp = TRUE;
  static BOOLEAN BothUp = TRUE;
  static BOOLEAN LeftDrag = FALSE;
  static BOOLEAN RightDrag = FALSE;
  static BOOLEAN BothDrag = FALSE;
  BOOLEAN TempLeftUp = TRUE;
  BOOLEAN TempRightUp = TRUE;
  BOOLEAN TempBothUp = TRUE;

  buttons = sample_mouse_position(&row, &column, &XPos, &YPos);

  /* in general, action should take place only when previous state */
  /* had no button */

  if (!((buttons & MOUSE_LEFT_BUTTON) || (buttons & MOUSE_RIGHT_BUTTON)))
    BothUp = TRUE;

  if (! (buttons & MOUSE_LEFT_BUTTON))
    LeftUp = TRUE;

  if (! (buttons & MOUSE_RIGHT_BUTTON))
    RightUp = TRUE;

  if ((active_locus == LOCUS_APPLICATION) && application_mouse_service)
    {
    return_key_value = (*application_mouse_service)(buttons, row, column,
      XPos, YPos, &LeftUp, &RightUp, &BothUp, &LeftDrag, &RightDrag,
      &BothDrag);
    return(return_key_value);
    }

  if (buttons && (RightUp || LeftUp))
    {
    if (! DebounceMouseKey())
      return 0;

    if (BothUp) // not in two key mode
      {
      if ((buttons & MOUSE_LEFT_BUTTON) && (buttons & MOUSE_RIGHT_BUTTON))
        {
        return_key_value = KEY_RIGHT;
        TempRightUp = FALSE;
        TempLeftUp = FALSE;
        TempBothUp = FALSE;
        }
      else if ((buttons & MOUSE_LEFT_BUTTON) && LeftUp)
        {
        TempLeftUp = FALSE;
        switch (active_locus)
          {
          case LOCUS_MENUS:
            return_key_value =
              menus_left_button_handler((UCHAR)row, (UCHAR)column);
            Current.Form->exit_key_code = return_key_value;
          break;

          case LOCUS_FORMS:
            return_key_value =
              forms_left_button_handler((UCHAR)row, (UCHAR)column);

            if (return_key_value == KEY_EXCEPTION)
              {
              return_key_value =
                menus_left_button_handler((UCHAR)row, (UCHAR)column);
              }
          break;

          case LOCUS_POPUP:
            return_key_value =
              popup_left_button_handler((UCHAR)row, (UCHAR)column);
          break;
          }
        }
      else if ((buttons & MOUSE_RIGHT_BUTTON) && RightUp)
        {
        TempRightUp = FALSE;
        return_key_value = KEY_ESCAPE;
        }
      }
    else   // still possibly in two key mode, previously let up only one key
      {    // act as if just did both keys
      if ((buttons & MOUSE_RIGHT_BUTTON) && (buttons & MOUSE_LEFT_BUTTON))
        {
        TempRightUp = FALSE;
        TempLeftUp = FALSE;
        return_key_value = KEY_RIGHT;
        }
      TempBothUp = FALSE;
      }

    if (return_key_value == KEY_EXCEPTION)
      {
      return_key_value = (*application_mouse_service)(buttons, row,
        column, XPos, YPos, &LeftUp, &RightUp, &BothUp, &LeftDrag,
        &RightDrag, &BothDrag);

      if (return_key_value == KEY_EXCEPTION)
        {
        Current.Form->status = FORMSTAT_EXIT_THIS_FIELD;
        return_key_value = KEY_ESCAPE;
        }
      }
    else
      {
      LeftUp = TempLeftUp;
      RightUp = TempRightUp;
      BothUp = TempBothUp;
      }
    }
   return(return_key_value);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void uses_mouse_input()
{
  if (init_mouse_device())
    {
    poll_mouse_event = forms_mouse_service;
    }
  else 
    {
    poll_mouse_event = NULL;
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void done_using_mouse_input()
{
  poll_mouse_event = NULL;

  shut_down_mouse_device();
}
