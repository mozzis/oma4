// FILE : coolstat.c    RAC  April 17, 1991

// maintain an on screen cooler status display by adding an update cooler
// status function to the keyboard idle loop.  The update function can be
// disabled/enabled.  When disabled it does nothing.  Disabling does NOT
// remove the function from the keyboard idle loop.

#include <time.h> /* clock */
#include <stdlib.h>   // NULL

#include "coolstat.h"
#include "forms.h"
#include "oma4000.h"
#include "device.h"
#include "oma4driv.h"
#include "detsetup.h" // InitStartDetector
#include "cursor.h"

static int updateEnabled = FALSE;  // start out disabled

void enableCoolStat(void) { updateEnabled = TRUE; }
//---------------------------------------------------------------------

void disableCoolStat(void) { updateEnabled = FALSE; }
//---------------------------------------------------------------------

static void drawCoolerStatus(enum CoolerLockedStatus coolerLock,
                             enum CoolerErrorStatus coolerError)
{
  int StartColumn = screen_columns - 9;  // display at far right,
  const int TheRow = 1;                  // top row of screen
  CXY TextPt;
  CCOLOR SelColor;
  char * string;
  int fcolor;  // foreground text color
  int bcolor;  // background color
  static char * labels[ ] = { "No Detect",
                              "   Locked",
                              " Unlocked",
                               "HeatExch",
                               "EmptyDwr",
                               "OverDiff",
                               "Too High",
                               "No Power" };

  TextPt.x = column_to_x(StartColumn);
  TextPt.y = row_to_y(TheRow);
  CSetATextPosition(screen_handle, TextPt, &TextPt);

  if(coolerError == COOL_OK) // just display either locked or unlocked
    {
    switch(coolerLock)
      {
      case FAKE :     bcolor = BLACK;
                      fcolor = BRT_WHITE;
                      string = labels[ 0 ]; // No Detect
                      break;
      case LOCKED :   bcolor = BRT_GREEN;
                      fcolor = BLACK;
                      string = labels[ 1 ];  // Locked
                      break;
      case UNLOCKED : bcolor = BRT_YELLOW;
                      fcolor = BLACK;
                      string = labels[ 2 ];  // Unlocked
                      break;
      }
    CSetATextColor(screen_handle, fcolor, & SelColor);
    CSetBgColor(screen_handle, bcolor, & SelColor);
    CAText(screen_handle, string, & TextPt);
    }
  else // there is an error condition, put fake/lock/unlock color in first
       // character position only.
    {
    switch(coolerLock)
      {
      case FAKE :     bcolor = BLACK     ; break;
      case LOCKED :   bcolor = BRT_GREEN ; break;
      case UNLOCKED : bcolor = BRT_YELLOW; break;
      }
    CSetBgColor(screen_handle, bcolor, & SelColor);
    CAText(screen_handle, " ", & TextPt);
    
    // now put up the error condition string
    CSetATextColor(screen_handle, BLACK, & SelColor);
    CSetBgColor(screen_handle, BRT_RED, & SelColor);
    TextPt.x = column_to_x(StartColumn + 1);
    CSetATextPosition(screen_handle, TextPt, & TextPt);

    switch(coolerError)
      {
      case HEAT_EXCHANGE  : string = labels[ 3 ]; break; // HeatExch
      case TOO_HIGH       : string = labels[ 6 ]; break; // Too High
      case DEWAR_EMPTY    : string = labels[ 4 ]; break; // EmptyDwr
      case DIFF_EXCEEDED  : string = labels[ 5 ]; break; // OverDiff
      case UNKNOWN_COOLER : string = labels[ 7 ];
      SetParam(DC_STOP, 0);
      InitStartDetector(); break; //  Unknown
      }
    CAText(screen_handle, string, & TextPt);
  }
}

#define HEAT_EXCH 1

// check cooler status and update window display if it has changed or if
// force_display is TRUE. Do not update unless enabled.

void displayCoolerStatus(BOOLEAN forceDisplay)
{
  // always display the first time called
  static BOOLEAN firstTime = TRUE;
  static enum CoolerLockedStatus lockStat;
         enum CoolerLockedStatus newLockStat;
  static enum CoolerErrorStatus  errorStat;
         enum CoolerErrorStatus  newErrorStat;

  float  cooler_return_status;
 
  if(! updateEnabled) return;

  GetParam(DC_COOLLOCK, &cooler_return_status);
  newLockStat = (int) cooler_return_status;

  GetParam(DC_COOLSTAT, &cooler_return_status);
  newErrorStat = (unsigned) cooler_return_status;

  if(forceDisplay || firstTime || (errorStat != newErrorStat) ||
      (lockStat != newLockStat))
    {
    firstTime = FALSE;
    lockStat = newLockStat;
    errorStat = newErrorStat;
    drawCoolerStatus(lockStat, errorStat);
    }

  if (CursorMode == CURSORMODE_ACTIVE)
    {
    GetParam(DC_ACTIVE, &cooler_return_status);
    if (cooler_return_status == 0)
      UpdateCursorMode(0);
    }
}

// don't update status unless enabled, but still allow the other functions
// in the keyboard idle loop to execute.
static unsigned char updateCoolerStatus(void)
{
  enum { WAIT_INTERVAL = 1800 };

  static long next_showing = 0L;

  if(updateEnabled)
    {
    static long now;
    
    now = clock(); /* returns ms since pgm start */

    if(next_showing <= now)
      {
      next_showing = now + WAIT_INTERVAL;
      displayCoolerStatus(FALSE);
      }
    }
  return lowLevelKeyboardIdle();
}

// start cooler status display by adding updateCoolerStat() to the
// keyboard idle chain.
void startCoolStat(void)
{
  static KEY_IDLE_CALLS keyIdleCoolStat = { NULL, NULL };

  if(keyIdleCoolStat.current_handler) return;

  keyIdleCoolStat.current_handler = updateCoolerStatus;
  keyIdleCoolStat.prev_handler = keyboard_idle;
  keyboard_idle = & keyIdleCoolStat;
  displayCoolerStatus(TRUE);

}
