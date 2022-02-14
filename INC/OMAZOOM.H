/* -----------------------------------------------------------------------
/
/  omazoom.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/OMAZOOM.H 1     6/18/99 1:53a Maynard $
*/

// make idempotent, add omazoom_init(), current_zoom_point().  RAC 16-MAR-90

#ifndef OMAZOOM_INCLUDED
#define OMAZOOM_INCLUDED

#ifndef CGIBIND_INCLUDED
   #define CGIBIND_INCLUDED
   #include <cgibind.h>
#endif

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

typedef struct _dCXY{
   float x;
   float y;
} dCXY;

#define ZOOM_CHOOSE_PT1    1
#define ZOOM_CHOOSE_PT2    2

extern SHORT ZoomState;
extern CXY cursor_loc;
extern BOOLEAN InitializingZoom;

// initialize so that the MoveCursor...() functions will work
void omazoom_init(void);

// return the X-Y coordinate of the current cursor location
dCXY current_zoom_point(void);

void DrawZoomBox(CRECT *ZoomRect, BOOLEAN Init);
void MoveCursorLeft(USHORT Dummy);
void MoveCursorRight(USHORT Dummy);
void MoveCursorUp(USHORT Dummy);
void MoveCursorDown(USHORT Dummy);
void Zoom(USHORT Dummy);
SHORT RestoreZoom(USHORT Dummy);

ERR_OMA ZoomToRect(SHORT Window, CRECT * ZoomRect);

// receive mouse motion from grafmous module
void zoomMouseMove(SHORT XPos, SHORT YPos);

void MacDoZoom(double zoomPts[4]);

void MacRestoreZoom(void);

#endif  // OMAZOOM_INCLUDED
