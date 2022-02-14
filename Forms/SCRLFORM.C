#include <stddef.h>  // NULL

#include "forms.h"
  
/* -----------------------------------------------------------------------
/
/  scrlform.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00        1-8 May         1988
/  Worked on:  TLB      Version 1.01
/
*/ /*
  $Header:   J:/logfiles/forms/scrlform.c_v   1.2   10 Jan 1992 13:37:24   cole  $
  $Log:   J:/logfiles/forms/scrlform.c_v  $
*/ /*
/ ----------------------------------------------------------------------- */
  
BOOLEAN scroll_refresh_only_flag = FALSE;
  
  
/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
UCHAR scroll_entry_field(void)
{
  SHORT  index;
  SHORT  virtual_index_offset;
  UCHAR  old_display_row;
  UCHAR  attribute;
  BOOLEAN at_least_one_line;

  at_least_one_line = FALSE;

  if ((USHORT) Current.Form->display_row_offset >
    Current.Form->virtual_row_index)
    {
    Current.Form->display_row_offset =
      (UCHAR) Current.Form->virtual_row_index;
    }

  if ((*Current.FormInitFunction)(Current.Form->virtual_row_index) )
    {
    Current.Form->display_row_offset = 0;
    Current.Form->virtual_row_index = 0;
    }

  attribute = set_attributes(Current.FormColorSet->regular.foreground,
    Current.FormColorSet->regular.background);

  erase_form_area(attribute, (Current.Form->attrib & FORMATTR_BORDER));

  old_display_row = Current.Form->display_row_offset;

  virtual_index_offset = ((int) Current.Form->virtual_row_index -
    (int) Current.Form->display_row_offset);
  
  for (index = 0; index < (int) Current.Form->size_in_rows; index++)
    {
    Current.Form->display_row_offset = (UCHAR) index;

    if (!(*Current.FormInitFunction)(virtual_index_offset + index))
      {
      draw_form_fields();

      at_least_one_line = TRUE;
      }
    else
      break;
    }

  Current.Form->display_row_offset = old_display_row;

  if (at_least_one_line)
    {
    (*Current.FormInitFunction)(Current.Form->virtual_row_index);
    init_field();
    return(EXIT_ENTER);
    }
  else
    {
    init_field();
    return(EXIT_DEFAULT);
    }
}
  
/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
BOOLEAN scroll_up_action()
{
  if (Current.Form->virtual_row_index > 0)
    {
    --Current.Form->virtual_row_index;

    if ( !(*Current.FormInitFunction)(Current.Form->virtual_row_index) )
      {
      if (Current.Form->display_row_offset > 0)
        {
        --Current.Form->display_row_offset;
        }
      else
        {
        scroll_down(Current.Form->row, Current.Form->column,
          Current.Form->size_in_rows, Current.Form->size_in_columns);
        }
      draw_form_fields();
      init_field();
      return(TRUE);
      }
    else
      {
      ++Current.Form->virtual_row_index;
      (*Current.FormInitFunction)(Current.Form->virtual_row_index);
      }
    }
  return(FALSE);
}

/* -----------------------------------------------------------------------
/ ----------------------------------------------------------------------- */
UCHAR scroll_up_field()
{
  UCHAR index;

  index = exit_code_to_vector_index(Current.Form->exit_key_code);

  if ((!scroll_up_action()) || (index == EXIT_UP))
    {
    Current.Form->field_index = Current.Form->previous_field_index;
    return(EXIT_NONE);
    }
  else
    return(index);
}
  
  
/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
BOOLEAN scroll_down_action()
{
  ++Current.Form->virtual_row_index;

  if ( !(*Current.FormInitFunction)(Current.Form->virtual_row_index) )
    {
    if (Current.Form->display_row_offset < (Current.Form->size_in_rows - (char) 1))
      {
      ++Current.Form->display_row_offset;
      }
    else
      {
      scroll_up(Current.Form->row, Current.Form->column,
        Current.Form->size_in_rows, Current.Form->size_in_columns);
      }
    draw_form_fields();
    init_field();
    return(TRUE);
    }
  else
    {
    --Current.Form->virtual_row_index;
    (*Current.FormInitFunction)(Current.Form->virtual_row_index);
    }
  return(FALSE);
}
  
/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
  
UCHAR scroll_down_field()
{
  UCHAR  index;

  index = exit_code_to_vector_index(Current.Form->exit_key_code);

  if ((!scroll_down_action()) || (index == EXIT_DOWN))
    {
    Current.Form->field_index = Current.Form->previous_field_index;
    return(EXIT_NONE);
    }
  else
    return(index);
}

/* -----------------------------------------------------------------------
// logic field function for use by scrolling form.  Used to determine
// whether the scrolling form is really being entered by the user and
// should retain the input focus or whether it should just be refreshed and
// not retain the input focus.
/ ----------------------------------------------------------------------- */
UCHAR refresh_scroll_only(FORM_CONTEXT * FrmCntxt)
{
  if (scroll_refresh_only_flag)
    return(EXIT_ENTER);
  else
    return(EXIT_DEFAULT);
}
  
/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void set_scroll_form_index(FORM * scroll_form, USHORT new_index)
{
  scroll_form->virtual_row_index = new_index;

  if (scroll_form->virtual_row_index >= (USHORT) scroll_form->size_in_rows)
    scroll_form->display_row_offset = (scroll_form->size_in_rows - (char)1);
  else
    scroll_form->display_row_offset = (UCHAR) new_index;
}

/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
  
USHORT index_of_scroll_form(FORM * scroll_form)
{
  return(scroll_form->virtual_row_index);
}
  
/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void show_active_field(FORM * TargetForm, SHORT field_index)
{
  SHORT save_index;

  if (push_form_context())
    {
    init_form(TargetForm);

    if (Current.Form->attrib & FORMATTR_SCROLLING)
      {
      if (Current.Form->init_function.group != 0)
        (*Current.FormInitFunction)(Current.Form->virtual_row_index);
      }

    save_index = Current.Form->field_index;
    Current.Form->field_index = field_index;
    init_field();

    set_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);
    format_and_display_field(FALSE);
    reset_bit(Current.Field->attrib, FLDATTR_HIGHLIGHT);

    Current.Form->field_index = save_index;
    pop_form_context();
    }
}
  
/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
  
void redraw_scroll_form(FORM * pForm, USHORT new_index, SHORT active_field)
{
  USHORT attrib;

  scroll_refresh_only_flag = TRUE;

  set_scroll_form_index(pForm, new_index);

  attrib = pForm->attrib & FORMATTR_NESTED;
  pForm->attrib &= (~FORMATTR_NESTED);
  pForm->field_index = 0;
  run_form(pForm, NULL, FALSE);
  pForm->attrib |= attrib;

  if (active_field != -1)
    show_active_field(pForm, active_field);

  scroll_refresh_only_flag = FALSE;
}


//void redraw_scroll_form(FORM * pForm, USHORT new_index, SHORT active_field)
//{
//  USHORT attrib;
//
//  scroll_refresh_only_flag = TRUE;
//
//  set_scroll_form_index(pForm, new_index);
//
//  attrib = pForm->attrib;
//  reset_bit(pForm->attrib, FORMATTR_NESTED);
//  pForm->field_index = 0;
//  run_form(pForm, NULL, FALSE);
//  pForm->attrib = attrib;
//
//  if (active_field != -1)
//    show_active_field(pForm, active_field);
//
//  scroll_refresh_only_flag = FALSE;
//}
