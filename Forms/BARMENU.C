/* -----------------------------------------------------------------------
/
/  barmenu.c
/
/  Copyright (c) 1988,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00         26 June        1989
/  Worked on:  TLB      Version 1.01
/
/  barmenu.c is a collection of functions to support program control
/  menus.
/
*/ /*
$Header:   J:/logfiles/forms/barmenu.c_v   1.8   24 Jul 1991 15:09:02   cole  $
$Log:   J:/logfiles/forms/barmenu.c_v  $
 * 
 *    Rev 1.8   24 Jul 1991 15:09:02   cole
 * Added MENUITEM_CALL_FORMTABLE case in choose_menuitem() for running a
 *   form from a menu via a pointer to a pointer to the form.
 * 
 *    Rev 1.7   28 May 1991 13:51:08   cole
 * remove all extern's from .c files
 * 
 *    Rev 1.6   20 May 1991 13:27:08   maynard
 * Cosmetic changes
 * 
 *    Rev 1.5   19 May 1991 21:03:44   cole
 * cosmetics. fixed #includes -- <> instead of "". fixed bug by adding test
 * for MenuFocus.PriorContext == NULL before freeing it.
 * 
 *    Rev 1.4   14 May 1991 12:49:22   maynard
 * Deleted calls to heapmin
 * 
 *    Rev 1.3   17 Dec 1990 16:25:14   irving
 * Added heapmin call.
 * 
 *    Rev 1.1   08 Oct 1990 16:45:16   irving
 * Disable cursor output during functions so that the drawing may happen
 * faster
*
*    Rev 1.0   27 Sep 1990 15:39:04   admax
* Initial revision.
*/ /*
/ ----------------------------------------------------------------------- */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "fkeyfunc.h"
#include "barmenu.h"
#include "omaform.h"
#include "forms.h"

BOOLEAN continue_flag = TRUE;

MENUCONTEXT MenuFocus;

/* -----------------------------------------------------------------------
/
/  void draw_menuitem(use_item)
/
/  function:
/  requires:   (int) use_item - the index of the menuitem in the menu.
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void draw_menuitem(MENUITEM * ThisItem, UCHAR attrib, UCHAR select_attrib)
{
  CHAR text_column,
       SelChar = ThisItem->Text[ThisItem->SelectCharOffset],
       SelCol = ThisItem->SelectCharOffset;
  BOOLEAN OldCursorEnable;

  OldCursorEnable = GetMouseCursorEnable();
  MouseCursorEnable(FALSE);

  text_column = (MenuFocus.Column + ThisItem->Column);

  emit(SPACE, MenuFocus.Row, text_column++, attrib);

  display_string(ThisItem->Text, ThisItem->TextLen, MenuFocus.Row,
                 text_column, attrib);

  if (! (ThisItem->Control & MENUITEM_INACTIVE))
    {
    // don't draw any highlighted char if it's select index is -1
    if (SelCol != -1)
      emit(SelChar, MenuFocus.Row, (text_column + SelCol), select_attrib);
    }
  emit(SPACE, MenuFocus.Row, (text_column + ThisItem->TextLen), attrib);

  MouseCursorEnable(OldCursorEnable);
  replace_mouse_cursor();                             
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void execute_menuitem(MENUITEM * thisItem)
{
  if(thisItem->Control & MENUITEM_CALLS_FORM)   
    run_form(thisItem->SubMenu, &default_form_attributes, FALSE);
  else if(thisItem->Control & MENUITEM_CALLS_FORMTABLE)
    run_form(*((FORM **)thisItem->SubMenu), &default_form_attributes, FALSE);
  else if(thisItem->Control & MENUITEM_CALLS_FUNCTION)
    (*(thisItem->Action))(0);
}

// -----------------------------------------------------------------------
void shade_menuitem(char index)
{
  MENUITEM * ThisItem = &MenuFocus.ActiveMENU->ItemList[index];
  UCHAR attribute;

  attribute = set_attributes(MenuFocus.MenuColorSet->shaded.foreground,
  MenuFocus.MenuColorSet->shaded.background);

  draw_menuitem(ThisItem, attribute, attribute);
}

// -----------------------------------------------------------------------

void highlight_menuitem(char index)
{
   MENUITEM *    ThisItem = &MenuFocus.ActiveMENU->ItemList[index];
   unsigned char attribute;
   // unsigned char select_attribute;

   attribute = set_attributes(MenuFocus.MenuColorSet->reverse.foreground,
   MenuFocus.MenuColorSet->reverse.background);

   /* select_attribute =
   set_attributes(MenuFocus.MenuColorSet->highlight.foreground,
   MenuFocus.MenuColorSet->highlight.background);

   draw_menuitem(ThisItem, attribute, select_attribute);
   */
   draw_menuitem(ThisItem, attribute, attribute);
}

// -----------------------------------------------------------------------

void unhighlight_menuitem(char index)
{
   MENUITEM *    ThisItem = &MenuFocus.ActiveMENU->ItemList[index];
   unsigned char attribute;
   unsigned char select_attribute;

   attribute = set_attributes(MenuFocus.MenuColorSet->regular.foreground,
   MenuFocus.MenuColorSet->regular.background);

   select_attribute =
   set_attributes(MenuFocus.MenuColorSet->highlight.foreground,
   MenuFocus.MenuColorSet->highlight.background);

   draw_menuitem(ThisItem, attribute, select_attribute);
}

/* -----------------------------------------------------------------------
/  void draw_menu()
/
/  function:   puts a menu onto the screen.
/  requires:   (int)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void draw_menu()
{
   char i;
   char first_active = -1;
   unsigned char column_offset = 0;
   unsigned char attribute;
   MENUITEM *    ThisItem;
   BOOLEAN OldCursorEnable;

   MenuFocus.MenuColorSet = &ColorSets[MenuFocus.ActiveMENU->ColorSetIndex];

   MenuFocus.ItemIndex = 0;

   attribute = set_attributes(MenuFocus.MenuColorSet->regular.foreground,
   MenuFocus.MenuColorSet->regular.background);

   OldCursorEnable = GetMouseCursorEnable();  
   MouseCursorEnable(FALSE);               

   erase_screen_area(MenuFocus.Row, MenuFocus.Column, 1, screen_columns,
   attribute, FALSE);

   for (i=0; i < MenuFocus.ActiveMENU->ItemCount; i++)
   {
      ThisItem = &MenuFocus.ActiveMENU->ItemList[i];

      if (ThisItem->TextLen == 0)
         ThisItem->TextLen = (char) strlen(ThisItem->Text);

      ThisItem->Column = column_offset;
      column_offset += (ThisItem->TextLen + 2);

      if (column_offset > screen_columns)
      {
         printf("\nMenu Overflowing screen!\n");
         break;
      }
      if (ThisItem->Control & MENUITEM_INACTIVE)
         shade_menuitem(i);
      else
      {
         unhighlight_menuitem(i);
         if (first_active == -1)
            first_active = i;
      }
   }
   if (first_active != -1)     /* what if ALL options are inactive?!? */
   {
      MenuFocus.ItemIndex = first_active;
   }
   highlight_menuitem(MenuFocus.ItemIndex);

   MouseCursorEnable(OldCursorEnable);             
   replace_mouse_cursor();                
}

/* -----------------------------------------------------------------------
/  void redraw_menu()
/
/  function:   puts a menu onto the screen.
/  requires:   (int)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void redraw_menu()
{
  char i;
  UCHAR attrib;
  MENUITEM *    ThisItem;
  BOOLEAN OldCursorEnable;

  attrib = set_attributes(MenuFocus.MenuColorSet->regular.foreground,
    MenuFocus.MenuColorSet->regular.background);

  OldCursorEnable = GetMouseCursorEnable();
  MouseCursorEnable(FALSE);

  erase_screen_area(MenuFocus.Row, MenuFocus.Column, 1, screen_columns,
    attrib, FALSE);

  for (i = 0;i < MenuFocus.ActiveMENU->ItemCount;i++)
    {
    ThisItem = &MenuFocus.ActiveMENU->ItemList[i];

    if (ThisItem->Control & MENUITEM_INACTIVE)
      shade_menuitem(i);
    else
      unhighlight_menuitem(i);
    }
  highlight_menuitem(MenuFocus.ItemIndex);
  MouseCursorEnable(OldCursorEnable);
  replace_mouse_cursor();                
}

/* -----------------------------------------------------------------------
/  void UnselectifyMenu(void)
/
/  function: puts a menu onto the screen without its select characters
/            highlighted
/  requires: (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void UnselectifyMenu(void)
{
  char i, Temp;
  UCHAR attrib;
  MENUITEM *    ThisItem;
  BOOLEAN OldCursorEnable;

  OldCursorEnable = GetMouseCursorEnable();
  MouseCursorEnable(FALSE);

  attrib = set_attributes(MenuFocus.MenuColorSet->regular.foreground,
    MenuFocus.MenuColorSet->regular.background);

  erase_screen_area(MenuFocus.Row, MenuFocus.Column, 1, screen_columns,
    attrib, FALSE);

  for (i = 0;i < MenuFocus.ActiveMENU->ItemCount;i++)
    {
    ThisItem = &MenuFocus.ActiveMENU->ItemList[i];

    Temp = ThisItem->SelectCharOffset;
    ThisItem->SelectCharOffset = -1;
    if (ThisItem->Control & MENUITEM_INACTIVE)
      shade_menuitem(i);
    else
      unhighlight_menuitem(i);
    ThisItem->SelectCharOffset = Temp;
    }
  highlight_menuitem(MenuFocus.ItemIndex);
  MouseCursorEnable(OldCursorEnable);
  replace_mouse_cursor();                             
}

/* -----------------------------------------------------------------------
/  BOOLEAN previous_menuitem()
/
/  function:   moves back to the previous item in the menu.  That is,
/              it moves back to the nearest item that is a valid
/              choice;  it skips any item which has the NOTCHOICE
/              bit set in its control variable.  This bit usually
/              indicates that the item is an input field.  If there
/              are no more valid fields in this direction, the current
/              item remains selected.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

BOOLEAN previous_menuitem()
{
  int i;

  for (i=(MenuFocus.ItemIndex - 1); i >= 0; i--)
    {
    if (! (MenuFocus.ActiveMENU->ItemList[i].Control & MENUITEM_INACTIVE))
      {
      unhighlight_menuitem(MenuFocus.ItemIndex);
      MenuFocus.ItemIndex = (char) i;
      highlight_menuitem(MenuFocus.ItemIndex);
      return(TRUE);
      }
    }

  return(FALSE);
}

/* -----------------------------------------------------------------------
/  BOOLEAN next_menuitem()
/
/  function:   moves to the next item in the menu.  That is,
/              it moves forward to the nearest item that is a valid
/              choice;  it skips any item which has the MENUITEM_INACTIVE
/              bit set in its control variable.  This bit indicates
/              that the item is not available.  If there are no more
/              valid fields in this direction, the current item
/              remains selected.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

BOOLEAN next_menuitem()
{
  int i;

  for (i = MenuFocus.ItemIndex + 1;i < MenuFocus.ActiveMENU->ItemCount;i++)
    {
    if (! (MenuFocus.ActiveMENU->ItemList[i].Control & MENUITEM_INACTIVE))
      {
      unhighlight_menuitem(MenuFocus.ItemIndex);
      MenuFocus.ItemIndex = (char) i;
      highlight_menuitem(MenuFocus.ItemIndex);
      return(TRUE);
      }
    }
  return(FALSE);
}

// -----------------------------------------------------------------------

BOOLEAN select_item_by_character(unsigned char key)
{
  int           i;
  MENUITEM *    ThisItem;
  unsigned char match_char;

  key = (char)toupper(key);

  for (i=0; i < MenuFocus.ActiveMENU->ItemCount; i++)
    {
    ThisItem = &MenuFocus.ActiveMENU->ItemList[i];

    if (ThisItem->SelectCharOffset != -1)
      {
      match_char = (char)toupper(ThisItem->Text[ ThisItem->SelectCharOffset ]);

      if (key == match_char)
        {
        if (ThisItem->Control & MENUITEM_INACTIVE)
          return(FALSE);
        else
          {
          unhighlight_menuitem(MenuFocus.ItemIndex);
          MenuFocus.ItemIndex = (char) i;
          highlight_menuitem(MenuFocus.ItemIndex);
          return(TRUE);
          }
        }
      }
    }
  return(FALSE);
}

/* -----------------------------------------------------------------------
/  void previous_menu()
/
/  function:   If in a sub-menu, this function will exit the sub-menu
/              and return to the calling menu.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void previous_menu()
{
  if (MenuFocus.ActiveMENU->Control & MENU_IS_SUBMENU)
    {
    UCHAR attrib;

    attrib = set_attributes(MenuFocus.MenuColorSet->regular.foreground,
      MenuFocus.MenuColorSet->regular.background);

    erase_screen_area(MenuFocus.Row, MenuFocus.Column, 1, screen_columns,
      attrib, FALSE);

    reset_bit(MenuFocus.ActiveMENU->Control, MENU_IS_SUBMENU);

    memcpy(&MenuFocus, MenuFocus.PriorContext, sizeof(MENUCONTEXT));

    redraw_menu();

    if(MenuFocus.PriorContext)
      free(MenuFocus.PriorContext);
    }
}

/* -----------------------------------------------------------------------
/  void back_to_main_menu()
/
/  function:   peel off all layers of sub-menu until back to the main menu.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void back_to_main_menu()
{
  while (MenuFocus.ActiveMENU->Control & MENU_IS_SUBMENU)
    {
    previous_menu();
    }
}

/* -----------------------------------------------------------------------
/  void enter_submenu()
/
/  function:   if a menuitem is chosen that calls a sub-menu, this
/              function will set up the sub-menu and make it the
/              current menu context.  The context of the menu that is
/              active is saved, the sub-menu context is set up, and
/              the new sub-menu is drawn on the screen.  Also, the
/              bit which indicates that an item has been selected
/              from the menu is reset.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void enter_submenu()
{
  PVOID pTemp;

  // this will keep the base context pointing to a NULL PriorContext
  pTemp = (MENUCONTEXT *) malloc(sizeof(MENUCONTEXT));

  if (pTemp!= NULL)
    {
    MENUITEM * ThisItem;

    memcpy(pTemp, &MenuFocus, sizeof(MENUCONTEXT));
    MenuFocus.PriorContext = pTemp;

    ThisItem = &MenuFocus.ActiveMENU->ItemList[MenuFocus.ItemIndex];

    MenuFocus.Row++;

    MenuFocus.ActiveMENU = ThisItem->SubMenu;

    set_bit(MenuFocus.ActiveMENU->Control, MENU_IS_SUBMENU);

    draw_menu();
    }
}

// -----------------------------------------------------------------------

void choose_menuitem(void)
{
  MENUITEM * ThisItem = &MenuFocus.ActiveMENU->ItemList[MenuFocus.ItemIndex];

  if (! (ThisItem->Control & MENUITEM_INACTIVE))
    {
    if (ThisItem->Control & MENUITEM_CALLS_SUBMENU)
      {
      UnselectifyMenu();         // take select key highlights off of
      // current menu   
      enter_submenu();
      highlight_menuitem(MenuFocus.ItemIndex);
      }
    else if(   (ThisItem->Control & MENUITEM_CALLS_FORM)
      || (ThisItem->Control & MENUITEM_CALLS_FORMTABLE))
      {
      UnselectifyMenu();         // take select key highlights off of
      // current menu      
      if (pKSPlayBack != NULL)
        {
        if ((*pKSPlayBack == TRUE) || (*pKSPlayBack == -2))
          return;
        }
      if(ThisItem->Control & MENUITEM_CALLS_FORM)
        Current.Form->exit_key_code =
          run_form(ThisItem->SubMenu, &default_form_attributes, FALSE);
        else  // must be MENUITEM_CALLS_FORMTABLE
          Current.Form->exit_key_code =
          run_form(* ((FORM **) ThisItem->SubMenu),
          & default_form_attributes, FALSE);

      redraw_menu();
      }
    else if (ThisItem->Action != NULL)
      {
      UnselectifyMenu();         // take select key highlights off of
      erase_cursor();
      active_locus = LOCUS_APPLICATION;
      (*ThisItem->Action)(0); /* add dummy parameter to match definition */
      active_locus = LOCUS_MENUS;
      redraw_menu();
      }
    }
}

// -----------------------------------------------------------------------

BOOLEAN activate_menu_option(MENU * menu_ptr, unsigned char option_number,
                             BOOLEAN activate)
{
  MENUITEM *  ThisItem;

  if ((char) option_number >= MenuFocus.ActiveMENU->ItemCount)
    return(FALSE);

  ThisItem = &MenuFocus.ActiveMENU->ItemList[option_number];

  if (activate)
    {
    reset_bit(ThisItem->Control, MENUITEM_INACTIVE);
    }
  else  /* (deactivate) */
    {
    if (MenuFocus.ItemIndex == (char)option_number) /* get away from here... */
      {
      if (! next_menuitem())
        previous_menuitem();
      }
    set_bit(ThisItem->Control, MENUITEM_INACTIVE);
    }
  redraw_menu();

  return(TRUE);
}

/* -----------------------------------------------------------------------
/  void DoExit()
/
/  function:   called as a function for a menuitem labelled "Exit".
/              This is provided to allow an alternate exit method
/              from the F10 key.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void DoExit(USHORT Dummy)      
{
   back_to_main_menu();
   continue_flag = FALSE;
   Current.Form->status = FORMSTAT_EXIT_ALL_FORMS;

   Dummy;               
}

/* -----------------------------------------------------------------------
/  void execute_menu(base_menu)
/
/  function:   This is the main function for the menus, the control
/              center.  It is controlled by keys typed at the keyboard.
/              The actions of the various keys:
/
/              Key            Action
/
/  requires:   (MENU *) base_menu - the main menu for the application
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void execute_menu(MENU * base_menu, char row, char col, char rows, char FirstChar)     
{
  unsigned char key;
  unsigned char attribute;
  SHORT TempIndex;

  /* get around starting over in menu system.  Keystroke Macro may leave */
  /* user in middle of form or menu and he will want to stay there */
  if (FirstChar != -1)
    {
    MenuFocus.Row = row;
    MenuFocus.Column = col;
    MenuFocus.SizeInRows = rows;
    MenuFocus.ActiveMENU = base_menu;
    draw_menu();

    attribute = set_attributes(MenuFocus.MenuColorSet->regular.foreground,
      MenuFocus.MenuColorSet->regular.background);

    if (MenuFocus.SizeInRows > 1)
      erase_screen_area((UCHAR) (MenuFocus.Row + 1),
        MenuFocus.Column,(UCHAR) (MenuFocus.SizeInRows - 1),
        screen_columns, attribute, FALSE);
    }

    continue_flag = TRUE;

    while (continue_flag &&
      (Current.Form->status != FORMSTAT_EXIT_ALL_FORMS) &&
      (Current.Form->status != FORMSTAT_SWITCH_MODE))
      {
      Current.Form->status = FORMSTAT_ACTIVE_FIELD;
      
      /* check to take up with current menu or go to the current form */
      if (FirstChar == -1)
        {
        // assume that the menus will not have a previous form
        while (Current.PreviousStackedContext != NULL)
          {
          Current.Form->exit_key_code =
            run_form(Current.Form, &default_form_attributes, TRUE);
          }
        /* exit to leftover (current) menu */
        FirstChar = 0;
        active_locus = LOCUS_MENUS;
        draw_menu();

        // go back to the top of the menu loop in case a big exit happened
        // while running the form
        continue;
        }

      active_locus = LOCUS_MENUS;

      if (FirstChar)
        {
        key = FirstChar;
        FirstChar = '\0';
        }
      else
        key = get_FORM_key_input();

      if (key & KEYS_HIGH_BIT)
        {
        switch (key)
          {
          case KEY_ENTER:
          case KEY_DOWN:
            if (KSActivateMenuRecord != NULL)
              {
              if (*pKSRecord)
                {
                TempIndex = MenuFocus.ActiveMENU->MacMenuIndex;
                (*KSActivateMenuRecord)(TempIndex, MenuFocus.ItemIndex);
                }
              }
            choose_menuitem();
          break;

          case KEY_ESCAPE:
          case KEY_UP:
            if (KSDeActivateMenuRecord != NULL)
              {
              if (*pKSRecord)
                (*KSDeActivateMenuRecord)();
              }
            previous_menu();
          break;

          case KEY_LEFT:
          case KEY_BACKSPACE:
          case KEY_BACK_TAB:
            previous_menuitem();
          break;

          case KEY_RIGHT:
          case KEY_TAB:
            next_menuitem();
          break;

          default:
            if (UserKeyHandler)
              Current.Form->status = (*UserKeyHandler)(key);
          break;
          }
        }
      else if ((Current.Form->status != FORMSTAT_EXIT_TO_MENU1) &&
        (Current.Form->status != FORMSTAT_EXIT_TO_MENU2))
        {
        if (select_item_by_character(key))
          {
          if (KSActivateMenuRecord != NULL)
            {
            if (*pKSRecord)
              {
              TempIndex = MenuFocus.ActiveMENU->MacMenuIndex;
              (*KSActivateMenuRecord)(TempIndex, MenuFocus.ItemIndex);
              }
            }
          choose_menuitem();
          }
        else
          {
          if (UserKeyHandler)
            Current.Form->status = (*UserKeyHandler)(key);
          }
        }

      if (Current.Form->status == FORMSTAT_EXIT_TO_MENU1)
        {
        if (KSDeActivateMenuRecord != NULL)
          {
          if (*pKSRecord)
            (*KSDeActivateMenuRecord)();
          }
        previous_menu();
        FirstChar = Current.Form->exit_key_code;
        }
      else if (Current.Form->status == FORMSTAT_EXIT_TO_MENU2)
        FirstChar = Current.Form->exit_key_code;
      }

    if (KSDeActivateMenuRecord != NULL)
      {
      if (*pKSRecord == TRUE)
        (*KSDeActivateMenuRecord)();
      }

    if (Current.Form->status == FORMSTAT_SWITCH_MODE)
      back_to_main_menu();
}

