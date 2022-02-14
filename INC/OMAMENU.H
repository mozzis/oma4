/* -----------------------------------------------------------------------
/
/  omamenu.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/OMAMENU.H 1     6/18/99 1:53a Maynard $
 * 
 *    Rev 1.0   24 Jul 1991 08:53:02   cole
 * Initial revision.
*/

#ifndef OMAMENU_INCLUDED
#define OMAMENU_INCLUDED
  
#include "barmenu.h"  // MENU
  
extern MENU MainMenu;
extern MENU FilesMenu;
extern MENU PlotMenu;
extern MENU RunMenu;
extern MENU MathMenu;
extern MENU AltHotKey;
extern MENU SpecialHotKey;
extern MENU CtrlHotKey;

extern MENUITEM far AltHotKeyItems[];
extern MENUITEM far SpecialHotKeyItems[];
extern MENUITEM far CtrlHotKeyItems[];
extern MENUITEM far RunMenuItems[];

// attach a function to a special hot key and enable it.  Key codes are
// defined in forms.h -- KEY_ENTER, KEY_ESCAPE, etc.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setSpecialHotKey(int key, void (* hotKeyFunction)(USHORT));
  
// clear a special hot key entry to NULL funcion.  key argument values are
// defined in forms.h -- KEY_ENTER, KEY_ESCAPE, etc.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void clearSpecialHotKey(int key);

// save the special hot key items corresponding to KEY_LEFT, KEY_RIGHT,
// KEY_UP, KEY_DOWN, KEY_ECAPE, and KEY_ENTER
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void saveGraphicsSpecialHotKeys(void);

// restore the special hot key items saved by saveGraphicsSpecialHotKeys()
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void restoreGraphicsSpecialHotKeys(void);

// select whether RAPDA or CCD version of scan menu is used
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void select_which_scanmenu(BOOLEAN isRapda);

SHORT ShowHKeys(MENU *HKeyMenu);

// set menu selection to DOS shell so the menu is redrawn correctly after 
// shelling to DOS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void set_menu_to_dosshell(void);

void init_menu_focus(void);

#endif    // OMAMENU_INCLUDED
