/* -----------------------------------------------------------------------
/
/  omamenu.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/omamenu.c_v   1.4   13 Jan 1992 14:41:56   cole  $
/  $Log:   J:/logfiles/oma4000/main/omamenu.c_v  $
 * 
 *    Rev 1.4   13 Jan 1992 14:41:56   cole
 * Change include's. Add savedGraphicsHotKeys[], setSpecialHotKey(),
 * clearSpecialHotKey(), saveGraphicsSpecialHotKeys(),
 * restoreGraphicsSpecialHotKeys().
 * 
 *    Rev 1.3   11 Nov 1991 14:42:10   maynard
 * Accum-B barmenu selection no longer calls Accum Form menu, just does
 * AccumBackFormInit (filename is always lastlive now)
 * 
 *    Rev 1.2   25 Sep 1991 12:36:56   cole
 * Add KSI_CALIB_MENU index to BaseCalibMenu. Put proper FormTable[] address
 * for spectrograph form into BaseCalibMenuItems.
 * 
 *    Rev 1.1   28 Aug 1991 16:43:04   cole
 * Moved FKeyItems[], FKey, ShiftFKey, ControlFKey, AltFKey from here to
 * fkeyfunc.c module.  Modified menus to use FormTable[] instead of form
 * addresses. Fix include's.
 * 
 *    Rev 1.0   25 Jul 1991 13:48:06   cole
 * Initial revision.
*/

#include <stddef.h>    // NULL
#include <string.h>    // NULL

#include "eggtype.h"
#include "omamenu.h"
#include "device.h"
#include "dosshell.h"
#include "fkeyfunc.h"
#include "graphops.h"
#include "helpindx.h"
#include "ksindex.h"
#include "tagcurve.h"
#include "runforms.h"
#include "formtabs.h"
#include "omaform.h"    // COLORS_MENU
#include "omaerror.h"   // ErrorBeepToggle
#include "forms.h"      // KEY_ENTER
#include "macruntm.h"
#include "handy.h"
#include "coolstat.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif


#define HKEY_ROW              0 
#define HKEY_ROW_SIZE         1
#define HKEY_ITEMS_PER_LINE   8


MENUITEM FilesMenuItems[] = {

   { "Directory", 0, 0, 0, &FormTable[KSI_FILE_DIR_FORM], NULL,
                            MENUITEM_CALLS_FORMTABLE, FILES_MENU_HBASE + 0 },
   { "Methods", 0, 0, 0, &FormTable[KSI_GET_METHOD_FORM], NULL,
                            MENUITEM_CALLS_FORMTABLE, FILES_MENU_HBASE + 1 },
   { "Translate", 0, 0, 0, &FormTable[KSI_FORMAT_FORM], NULL,
                            MENUITEM_CALLS_FORMTABLE, FILES_MENU_HBASE + 2 },
   { "DOS Shell", 1, 0, 0, NULL, do_DOS_commands, MENUITEM_CALLS_FUNCTION,
                                                      FILES_MENU_HBASE + 3 },
   { "Exit", 1, 0, 0, NULL, DoExit, MENUITEM_CALLS_FUNCTION,
                                                      FILES_MENU_HBASE + 4 }
};

struct menudef FilesMenu = {
   
   0, COLORS_MENU, sizeof(FilesMenuItems) / sizeof(FilesMenuItems[0]),
   FilesMenuItems, KSI_FILES_MENU
};

void set_menu_to_dosshell(void)
{
  if (MenuFocus.ActiveMENU == &FilesMenu)
    MenuFocus.ItemIndex = 3;
}

MENUITEM PlotMenuItems[] = {

   { "Setup", 0, 0, 0, &FormTable[KSI_PLOT_FORM], NULL,
      MENUITEM_CALLS_FORMTABLE, PLOT_MENU_HBASE + 0 },
   { "Screen", 1, 0, 0, NULL, PlotScreen, MENUITEM_CALLS_FUNCTION,
      PLOT_MENU_HBASE + 1 },
   { "Plotter", 0, 0, 0, NULL, PlotPlotter, MENUITEM_CALLS_FUNCTION,
      PLOT_MENU_HBASE + 2 },
   { "Printer", 1, 0, 0, NULL, PlotPrinter, MENUITEM_CALLS_FUNCTION,
      PLOT_MENU_HBASE + 3 }
};

struct menudef PlotMenu = {

   0, COLORS_MENU, sizeof(PlotMenuItems) / sizeof(PlotMenuItems[0]),
   PlotMenuItems, KSI_PLOT_MENU
};

MENUITEM RunMenuItems[] =
{
   { "Setup", 0, 0, 0, &FormTable[KSI_DA_FORM], NULL,
                              MENUITEM_CALLS_FORMTABLE, RUN_MENU_HBASE + 0},
   { "Accum", 0, 0, 0, NULL, AccumFormInit, MENUITEM_CALLS_FUNCTION, 
                                                        RUN_MENU_HBASE + 1},
   { "Accum-B", 1, 0, 0, NULL, AccumBackFormInit, MENUITEM_CALLS_FUNCTION
                             | MENUITEM_INACTIVE, RUN_MENU_HBASE + 2 },
   { "Live", 0, 0, 0, &FormTable[KSI_LIVE_FORM], NULL,
                              MENUITEM_CALLS_FORMTABLE, RUN_MENU_HBASE + 3},
   { "Live to disk", 8, 0, 0, &FormTable[KSI_LIVE_DISK_FORM], NULL,
                              MENUITEM_CALLS_FORMTABLE, RUN_MENU_HBASE + 4},      
   { "Live-B", 1, 0, 0, &NorecFormTable[NOREC_LIVE_BACK_FORM], NULL,
          MENUITEM_CALLS_FORMTABLE | MENUITEM_INACTIVE, RUN_MENU_HBASE + 5},
   { "BackGround", 0, 0, 0, &FormTable[KSI_BACKGROUND_FORM], NULL,
                              MENUITEM_CALLS_FORMTABLE, RUN_MENU_HBASE + 6},
};
                                            
struct menudef RunMenu = {

   0, COLORS_MENU, sizeof(RunMenuItems) / sizeof(RunMenuItems[0]),
   RunMenuItems, KSI_RUN_MENU
};

MENUITEM MathMenuItems[] =
{
   { "Calculate", 0, 0, 0, &FormTable[KSI_MATH_FORM], NULL,
                         MENUITEM_CALLS_FORMTABLE, MATH_MENU_HBASE + 0 },
   { "Stats",     0, 0, 0, &FormTable[KSI_STAT_FORM], NULL,
                         MENUITEM_CALLS_FORMTABLE, MATH_MENU_HBASE + 1 },
   { "Baseline",  0, 0, 0, &FormTable[KSI_BASLN_FORM], NULL,
                         MENUITEM_CALLS_FORMTABLE, MATH_MENU_HBASE + 2 },
};

struct menudef MathMenu = {
   
   0, COLORS_MENU, sizeof(MathMenuItems) / sizeof(MathMenuItems[0]),
   MathMenuItems, KSI_MATH_MENU
};

    /* added BaseCalibMenu to allow 1235 */

MENUITEM BaseCalibMenuItems[] =
{
   { "X Calibration", 0, 0, 0, &FormTable[KSI_XCAL_FORM], NULL,
                            MENUITEM_CALLS_FORMTABLE, CALIB_HBASE + 10 },
   { "Y Calibration", 0, 0, 0, &FormTable[KSI_YCAL_FORM], NULL,
                            MENUITEM_CALLS_FORMTABLE, CALIB_HBASE + 10 },
   { "Spectrograph Control", 0, 0, 0, &FormTable[KSI_SPGRAPH_FORM], NULL,
                            MENUITEM_CALLS_FORMTABLE, CALIB_HBASE + 20 },
};

struct menudef BaseCalibMenu = {

   0, COLORS_MENU,
   sizeof(BaseCalibMenuItems) / sizeof(MENUITEM),
   BaseCalibMenuItems, KSI_CALIB_MENU
};


MENUITEM MainMenuItems[] = { 
   { "Files",         0, 0, 0, &FilesMenu,      NULL,
                               MENUITEM_CALLS_SUBMENU,  MAIN_MENU_HBASE+0},
   { "Run",           0, 0, 0, &RunMenu,        NULL,
                               MENUITEM_CALLS_SUBMENU,  MAIN_MENU_HBASE+1},
   { "Plot",          0, 0, 0, &PlotMenu,       NULL,
                               MENUITEM_CALLS_SUBMENU,  MAIN_MENU_HBASE+2},
   { "Calibration",   0, 0, 0, &BaseCalibMenu,  NULL,
                               MENUITEM_CALLS_SUBMENU,  MAIN_MENU_HBASE+3},
   { "Math",          0, 0, 0, &MathMenu,       NULL,
                               MENUITEM_CALLS_SUBMENU,  MAIN_MENU_HBASE+4},
   { "Configuration", 1, 0, 0, &FormTable[KSI_CONFIG_FORM],     NULL,
                              MENUITEM_CALLS_FORMTABLE, MAIN_MENU_HBASE+5},
   { "Keystroke",     0, 0, 0, &FormTable[KSI_KEYSTROKE_FORM],  NULL,
                               MENUITEM_CALLS_FORMTABLE, MAIN_MENU_HBASE+7},
   { "Scan",          0, 0, 0, &FormTable[KSI_SCAN_SETUP],  NULL,
                              MENUITEM_CALLS_FORMTABLE, MAIN_MENU_HBASE+6},
};

MENU  MainMenu = {

   0, COLORS_MENU, sizeof(MainMenuItems) / sizeof(MainMenuItems[0]),
   MainMenuItems, KSI_MAIN_MENU
};

MENUITEM AltHotKeyItems[] = {

     { "Alt-A", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 0
     { "Beep On/Off", 0, 0, 0, NULL, ErrorBeepToggle,               
                                            MENUITEM_CALLS_FUNCTION, 0 }, // 1
     { "Alt-C", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 2
     { "Alt-D", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 3
     { "Alt-E", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 4
     { "Alt-F", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 5
     { "Go Cursor", 0, 0, 0, &FormTable[KSI_CURSOR_GOTO_FORM], NULL,
                       MENUITEM_CALLS_FORMTABLE | MENUITEM_INACTIVE, 0 }, // 6
     { "Alt-H", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 7
     { "Alt-I", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 8
     { "Alt-J", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 9
     { "Alt-K", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 10
     { "Alt-L", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 11
     { "Alt-M", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 12
     { "Alt-N", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 13
     { "Alt-O", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 14
     { "Peaks", 0, 0, 0, NULL, HotAutoPeakStart,
          MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 9/14/90 DAI// 15
     { "Alt-Q", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 16
     { "Replot", 0, 0, 0, NULL, HotPlotWindow,
                                            MENUITEM_CALLS_FUNCTION, 0 }, // 17
     { "Screen", 0, 0, 0, NULL, HotPlotScreen,
                                            MENUITEM_CALLS_FUNCTION, 0 }, // 18
     { "Tag", 0, 0, 0, NULL, HotTagCurve,
                        MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 19
     { "Untag", 0, 0, 0, NULL, HotUntagCurve,
                        MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 20
     { "Alt-V", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 21
     { "Alt-W", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 22
     { "Exit", 1, 0, 0, NULL, HotExit, MENUITEM_CALLS_FUNCTION, 0 },     // 23
     { "Alt-Y", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 24
     { "Alt-Z", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                  MENUITEM_INACTIVE, 0 }, // 25
};

MENU AltHotKey = {

   0, COLORS_MENU, sizeof(AltHotKeyItems) / sizeof(AltHotKeyItems[0]),
   AltHotKeyItems, KSI_ALT_KEY_HOT
};

MENUITEM    CtrlHotKeyItems[] = {
   { "Ctrl-A", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 0
   { "Ctrl-B", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 1
   // NOTE : control C is a special MSDOS "break" key
   { "Cancel", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 },// 2
   { "Ctrl-D", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 3
   { "Ctrl-E", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 4
   { "Ctrl-F", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 5
   { "Ctrl-G", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 6
   { "Ctrl-H", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 7
   { "Ctrl-I", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 8
   { "Ctrl-J", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 9
   { "Ctrl-K", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 10
   { "Ctrl-L", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 11
   { "Ctrl-M", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 12
   { "Ctrl-N", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 13
   { "Ctrl-O", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 14
   { "Ctrl-P", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 15
   { "Ctrl-Q", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 16
   { "Ctrl-R", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 17
   { "Ctrl-S", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 18
   { "Tag Group", 0, 0, 0, &FormTable[KSI_TAG_CURVES_FORM],
               NULL, MENUITEM_CALLS_FORMTABLE | MENUITEM_INACTIVE, 0 }, // 19
   { "Untag All", 0, 0, 0, NULL, HotUntagAll,
                      MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 20
   { "Ctrl-V", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 21
   { "Ctrl-W", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 22
   { "Ctrl-X", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 23
   { "Ctrl-Y", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 24
   { "Ctrl-Z", 0, 0, 0, NULL, NULL, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 25

   { "(Ins  <X>", 0, 0, 0, NULL, ExpandXAxis, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 24
   { "Del  >X<", 0, 0, 0, NULL, ContractXAxis, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 24
   { "PgUp <Y>", 0, 0, 0, NULL, ExpandYAxis, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 25
   { "PgDn >Y<)", 0, 0, 0, NULL, ContractYAxis, MENUITEM_CALLS_FUNCTION |
                                                MENUITEM_INACTIVE, 0 }, // 25
};

MENU CtrlHotKey = {
   
   0, COLORS_MENU, sizeof(CtrlHotKeyItems) / sizeof(CtrlHotKeyItems[0]),
   CtrlHotKeyItems, KSI_CTRL_KEY_HOT
};

MENUITEM    SpecialHotKeyItems[] =        
{
/*ENTER     */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 0
/*ESCAPE    */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 1
/*TAB       */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 2
/*BACK_TAB  */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 3
/*BACKSPACE */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 4
/*DELETE    */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 5
/*DELETE_FAR*/ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 6
/*INSERT    */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 7
/*UP        */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 8
/*UP_FAR    */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 9
/*DOWN      */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 10
/*DOWN_FAR  */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 11
/*LEFT      */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 12
/*LEFT_FAR  */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 13
/*RIGHT     */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 14
/*RIGHT_FAR */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 15
/*HOME      */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 16
/*HOME_FAR  */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 17
/*END       */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 18
/*END_FAR   */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 19
/*PG_UP     */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 20
/*PG_UP_FAR */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 21
/*PG_DN     */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 22
/*PG_DN_FAR */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 23
/*PLUS      */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 24
/*MINUS     */ { NULL, 0, 0, 0, NULL, NULL,
               MENUITEM_CALLS_FUNCTION | MENUITEM_INACTIVE, 0 }, // 25
};

MENU SpecialHotKey = {
   
   0, COLORS_MENU,
   sizeof(SpecialHotKeyItems) / sizeof(SpecialHotKeyItems[0]),
   SpecialHotKeyItems, KSI_SPC_KEY_HOT
};

static MENU *CurrentHKeyMenu = &MainMenu;

// save area for LEFT, RIGHT, UP, DOWN, ENTER, ESCAPE SpecailHotKeyItems
PRIVATE MENUITEM savedGraphicsHotKeys[6];

// attach a function to a special hot key and enable it.  Key codes are
// defined in forms.h -- KEY_ENTER, KEY_ESCAPE, etc.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setSpecialHotKey(int key, void (* hotKeyFunction)(USHORT))
{
   SpecialHotKeyItems[key - KEY_ENTER].Action   =   hotKeyFunction;
   SpecialHotKeyItems[key - KEY_ENTER].Control &= ~ MENUITEM_INACTIVE;
}

// clear a special hot key entry to NULL funcion.  key argument values are
// defined in forms.h -- KEY_ENTER, KEY_ESCAPE, etc.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void clearSpecialHotKey(int key)
{
   SpecialHotKeyItems[key - KEY_ENTER].Action   = NULL;
   SpecialHotKeyItems[key - KEY_ENTER].Control |= MENUITEM_INACTIVE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void saveGraphicsSpecialHotKeys(void)
{
   savedGraphicsHotKeys[0] = SpecialHotKeyItems[KEY_LEFT   - KEY_ENTER];
   savedGraphicsHotKeys[1] = SpecialHotKeyItems[KEY_RIGHT  - KEY_ENTER];
   savedGraphicsHotKeys[2] = SpecialHotKeyItems[KEY_UP     - KEY_ENTER];
   savedGraphicsHotKeys[3] = SpecialHotKeyItems[KEY_DOWN   - KEY_ENTER];
   savedGraphicsHotKeys[4] = SpecialHotKeyItems[KEY_ESCAPE - KEY_ENTER];
   savedGraphicsHotKeys[5] = SpecialHotKeyItems[KEY_ENTER  - KEY_ENTER];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void restoreGraphicsSpecialHotKeys(void)
{
   SpecialHotKeyItems[KEY_LEFT   - KEY_ENTER] = savedGraphicsHotKeys[0];
   SpecialHotKeyItems[KEY_RIGHT  - KEY_ENTER] = savedGraphicsHotKeys[1];
   SpecialHotKeyItems[KEY_UP     - KEY_ENTER] = savedGraphicsHotKeys[2];
   SpecialHotKeyItems[KEY_DOWN   - KEY_ENTER] = savedGraphicsHotKeys[3];
   SpecialHotKeyItems[KEY_ESCAPE - KEY_ENTER] = savedGraphicsHotKeys[4];
   SpecialHotKeyItems[KEY_ENTER  - KEY_ENTER] = savedGraphicsHotKeys[5];
}

void select_which_scanmenu(BOOLEAN isRapda)
{
  if (isRapda)
    MainMenuItems[7].SubMenu = &FormTable[KSI_RAPDA_FORM];
  else
    MainMenuItems[7].SubMenu = &FormTable[KSI_SCAN_SETUP];
}

void init_menu_focus(void)
{
  MenuFocus.Row          = HKEY_ROW;
  MenuFocus.Column       = 0;
  MenuFocus.SizeInRows   = HKEY_ROW_SIZE;
  MenuFocus.ItemIndex    = 0;
  MenuFocus.MenuColorSet = &ColorSets[MenuFocus.ActiveMENU->ColorSetIndex];
  MenuFocus.PriorContext = NULL;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT ShowHKeys(MENU *HKeyMenu)
{
  MENUCONTEXT OldContext = MenuFocus;
  SHORT       i;
  UCHAR       column_offset = 0;
  UCHAR       attribute;
  MENUITEM *  ThisItem;
  SHORT       ColumnInc = screen_columns / HKEY_ITEMS_PER_LINE;
  CRECT       OldClipRect;
  CFILLREPR   FillRep;
  int         shadeItem;

  CurrentHKeyMenu = HKeyMenu;

  CInqFillRepr(screen_handle, &FillRep);

  erase_mouse_cursor();
  MouseCursorEnable(FALSE);

  CInqClipRectangle(screen_handle, &OldClipRect);
  setClipRectToFullScreen();

  MenuFocus.Row          = HKEY_ROW;
  MenuFocus.Column       = 0;
  MenuFocus.SizeInRows   = HKEY_ROW_SIZE;
  MenuFocus.ItemIndex    = 0;
  MenuFocus.ActiveMENU   = HKeyMenu;
  MenuFocus.MenuColorSet = &ColorSets[MenuFocus.ActiveMENU->ColorSetIndex];
  MenuFocus.PriorContext = NULL;

  attribute = set_attributes(MenuFocus.MenuColorSet->regular.foreground,
    MenuFocus.MenuColorSet->regular.background);

  erase_screen_area(MenuFocus.Row, MenuFocus.Column, MenuFocus.SizeInRows,
    screen_columns, attribute, FALSE);

  for (i=0; i < MenuFocus.ActiveMENU->ItemCount; i++)
    {
    ThisItem = &MenuFocus.ActiveMENU->ItemList[i];

    if (ThisItem->Text != NULL)
      {
      if (!(ThisItem->SubMenu || ThisItem->Action))
        continue;

      if (ThisItem->TextLen == 0)
        ThisItem->TextLen = (char) strlen(ThisItem->Text);

      /* Wrap To next line */    
      if ((column_offset + ThisItem->TextLen) >= (UCHAR)screen_columns)
        {
        column_offset = 0;
        MenuFocus.Column = 0;
        MenuFocus.Row++;
        }

      ThisItem->Column = (unsigned char) column_offset;

      shadeItem = ThisItem->Control &MENUITEM_INACTIVE;

      if (shadeItem)
        shade_menuitem((unsigned char) i);
      else
        unhighlight_menuitem((unsigned char) i);
      }

    column_offset += ThisItem->TextLen + 2;
    }

  MenuFocus = OldContext;

  CSetClipRectangle(screen_handle, OldClipRect);

  MouseCursorEnable(TRUE);
  replace_mouse_cursor();

  CSetFillRepr(screen_handle, &FillRep);

  displayCoolerStatus(TRUE);

  return FALSE;
}
