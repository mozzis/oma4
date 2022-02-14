/* -----------------------------------------------------------------------
/
/  cursor.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/CURSOR.H 1     6/18/99 1:52a Maynard $
*/

#ifndef CURSOR_INCLUDED
#define CURSOR_INCLUDED

#ifndef CGIBIND_INCLUDED
   #define CGIBIND_INCLUDED
   #include <cgibind.h>      // CMARKERTYPE
#endif

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern SHORT ActiveWindow;
extern SHORT ActivePlotIndex;
extern SHORT CursorMode;
extern USHORT CursorInc;
extern CMARKERTYPE CursorType;

// max time in milliseconds to be a repeat hit.
extern const USHORT KeyRepeatTm;

#define CURSORMODE_NORMAL  0
#define CURSORMODE_LIVE    1
#define CURSORMODE_ACCUM   2
#define CURSORMODE_PAUSED  3
#define CURSORMODE_ZOOM    4
#define CURSORMODE_SELECT  5
#define CURSORMODE_TAGGED  6
#define CURSORMODE_ACTIVE  7

#define SK_LEFT_ARROW      0x4B00
#define SK_LEFT_ARROW      0x4B00
#define SK_RIGHT_ARROW     0x4D00
#define SK_RIGHT_ARROW     0x4D00
#define SK_UP_ARROW        0x4800
#define SK_UP_ARROW        0x4800
#define SK_DOWN_ARROW      0x5000
#define SK_DOWN_ARROW      0x5000

#define LOCATION_CURSOR_PAD 0xE0

typedef struct
{
   FLOAT X;
   FLOAT Y;
   FLOAT Z;
   SHORT EntryIndex;
   USHORT FileCurveNumber;
   USHORT PointIndex;
   USHORT TotalCurves;
   USHORT UTDisplayCurve;     // Untagged mode display curve number
                              // use in tagged mode to keep track of the
                              // current curve
   USHORT UTTotalCurves;      // Untagged mode display curve count
                              // use in tagged mode to keep track of the
                              // current curve
} CURSORSTAT;

extern CURSORSTAT CursorStatus[];

void TmpSwap(float *, float *);
void UpdateWindowNumberStat(SHORT);
void UpdateCursorXStat(SHORT);
void UpdateCursorYStat(SHORT);
void UpdateCursorZStat(SHORT);
void UpdateCursorBlkNameStat(SHORT);
void UpdateCursorBlkNumberStat(SHORT);
void UpdateCursorMode(SHORT);

void ForceCursorIntoWindow(SHORT Window);
void InitCursorStatus(SHORT);
void SetGraphCursorType(CMARKERTYPE CursorType);
void InitCursor(SHORT, CMARKERTYPE);
void processNewCursorPos(SHORT window, unsigned keyStroke);
void MoveCursorXPos(SHORT, BOOLEAN);
void MoveCursorZPos(SHORT, BOOLEAN, BOOLEAN);

int new_active_window(int new_window);
void restore_active_window(int old_window);

ERR_OMA JumpCursor(FLOAT XVal, FLOAT ZVal);
CXY GetCurrentCursorPosition(SHORT Mode);

void DisplayGraphCursor(CXY CursorLoc);
void RemoveGraphCursor(void);
void DisplayActiveGraphCursor(void); // display graph cursor in the active
                                        // window
BOOLEAN pointOnGraphCursor(SHORT xPos, SHORT yPos);

// move the cursor for the macro language
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA moveCursorMacro(float xVal, float zVal);

void SetCursorPos(SHORT, FLOAT, FLOAT, FLOAT);
void CenterCursorXPos(void);

// CCURHANDLE GetCursorHandle(void);
//void ReleaseCursor(CCURHANDLE);
//void RestoreCursor(CCURHANDLE);

#endif
