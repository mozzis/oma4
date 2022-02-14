/* -----------------------------------------------------------------------
/
/  barmenu.h
/
/  Copyright (c) 1988,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00         26 June        1988
/  Worked on:  TLB      Version 1.01
/              DAI                           22 Nov         1989
/              RAC                            1 Feb         1990
/                       made this header file idempotent, #include forms.h
/
/  barmenu.h defines constants and data structures for bar menus.
/
/ $Header: /OMA4000/Forms/BARMENU.H 1     6/18/99 1:53a Maynard $
/  $Log: /OMA4000/Forms/BARMENU.H $
 * 
 * 1     6/18/99 1:53a Maynard
 * 
 *    Rev 1.2   09 Jan 1992 15:31:40   cole
 * Add include formtype.h, primtype.h ; delete include forms.h
 * 
 *    Rev 1.1   25 Jul 1991 14:23:46   cole
 * Add #define for MENUITEM_CALLS_FORMTABLE
 * 
 *    Rev 0.8   24 Jul 1991 14:57:24   cole
 * Added #define for MENUITEM_CALLS_FORMTABLE
 * 
 *    Rev 0.7   10 Jan 1991 02:05:30   maynard
 * Incorporate Dwight's changes from 1.81 oma2000
 * Changes for OMA4 macro language support
 * Add temperature control, shutter control, et. al.
 * 
 *    Rev 1.0   17 Dec 1990 16:32:36   irving
 * Initial revision.
*
*    Rev 1.0   12 Jul 1989 11:26:58   tomb
* Initial revision.
/
/ -----------------------------------------------------------------------
*/

#ifndef BARMENU_INCLUDED   // idempotent header file
#define BARMENU_INCLUDED

#include "formtype.h"      // COLOR_SET

#define     MENU_IS_SUBMENU            0x01
#define     MENU_                      0x02


#define     MENUITEM_CALLS_SUBMENU     0x01
#define     MENUITEM_CALLS_FUNCTION    0x02
#define     MENUITEM_INACTIVE          0x04
#define     MENUITEM_                  0x08
#define     MENUITEM_CALLS_FORM        0x10
#define     MENUITEM_CALLS_FORMTABLE   0x20

#pragma  pack(1)

typedef struct menudef
{
  unsigned char        Control;
  char                 ColorSetIndex;
  char                 ItemCount;
  struct menuitemdef * ItemList;
  int                  MacMenuIndex; /* for keystroke record and playback     */
                                     /* index into a file of macro commands   */
                                     /* one of which will activeate this menu */
} MENU;

/* structure to define a Menu Item */

typedef void (MenuAction)(unsigned int);

typedef struct menuitemdef
{
   char *Text;
   char SelectCharOffset;
   char Column;
   char TextLen;
   void *SubMenu;
   MenuAction *Action;
   unsigned char Control;
   unsigned int help_index;
} MENUITEM;

/* structure to define a Menu */

typedef struct menu_context
{
   char                    Row;
   char                    Column;
   char                    SizeInRows;
   char                    ItemIndex;
   MENU *                  ActiveMENU;
   COLOR_SET *             MenuColorSet;
   struct menu_context *   PriorContext;
} MENUCONTEXT;

#pragma  pack()

extern MENUCONTEXT MenuFocus;

void shade_menuitem( char );
void highlight_menuitem( char );
void unhighlight_menuitem( char );
void draw_menu( void );
void previous_menu( void );
void redraw_menu( void );  
void UnselectifyMenu( void );
BOOLEAN select_item_by_character(unsigned char key);
void choose_menuitem( void );
void DoExit( unsigned short Dummy );
void execute_menu(MENU *, char, char, char, char );
void execute_menuitem( MENUITEM * thisItem );

#endif   // BARMENU_INCLUDED

