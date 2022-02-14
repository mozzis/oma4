#include <stdlib.h>
#include <dos.h>
  
#include "forms.h"
  
/* -----------------------------------------------------------------------
/
/  mouseraw.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00          6 June        1988
/  Worked on:  TLB      Version 1.01
/
/  This is a collection of routines to do mouse input.  This module
/  supports "Raw" mouse input, that is, direct interrupt calls
/  to the mouse driver.  Other modules provide similar functions
/  from other mouse environments, such as GSS*GKI.
/
  $Header:   C:/pvcs/logfiles/forms/mouseraw.c_v   1.2   26 Oct 1990 10:37:22   irving  $
  $Log:   C:/pvcs/logfiles/forms/mouseraw.c_v  $
 * 
 *    Rev 1.2   26 Oct 1990 10:37:22   irving
 * added raw screen coordinates to sample_mouse_position.
 * Will be unused when not using GSS version
 * 
 *    Rev 1.1   08 Oct 1990 16:48:38   irving
 * Added dummy functions to complement changes in mousegss.c
 * 
 *    Rev 1.0   27 Sep 1990 15:42:26   admax
 * Initial revision.
/ ----------------------------------------------------------------------- */
  
// #define  NULL     0        // 5/29/90 DAI
// #define  NOT      !
// #define  TRUE     1
// #define  FALSE    0
  
#define  MOUSE_DRIVER_LOADED     -1 /*  */
  
#define  MOUSE_DRIVER_INT        0x33  /* actual interrupt number invoked */
#define  TEST_FOR_MOUSE_DRIVER      0  /*  */
#define  ENABLE_MOUSE_CURSOR        1  /*  */
#define  DISABLE_MOUSE_CURSOR       2  /*  */
#define  GET_POSITION_AND_BUTTON    3  /*  */
#define  SET_POSITION               4  /*  */
#define  LAST_BUTTON_DOWN_EVENT     5  /*  */
#define  LAST_BUTTON_UP_EVENT       6  /*  */
#define  LIMIT_MOUSE_X              7  /*  */
#define  LIMIT_MOUSE_Y              8  /*  */
#define  MOUSE9                     9  /*  */
#define  MOUSE_CURSOR_STYLE         10 /*  */
#define  MOUSE_RELATIVE_MOTION      11 /*  */
#define  MOUSE12                    12 /*  */
#define  MOUSE13                    13 /*  */
#define  MOUSE14                    14 /* these two have something to do with */
#define  MOUSE15                    15 /* loading interrupt service routines */
  
  
extern BOOLEAN    mouse_is_useable;
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
BOOLEAN init_mouse_device(void)
{
   union REGS regs;
  
   regs.x.ax = TEST_FOR_MOUSE_DRIVER;
   int86(MOUSE_DRIVER_INT, &regs, &regs);
  
   if (regs.x.ax == MOUSE_DRIVER_LOADED)
   {
      regs.x.ax = ENABLE_MOUSE_CURSOR;
      int86(MOUSE_DRIVER_INT, &regs, &regs);
  
      regs.x.ax = MOUSE_CURSOR_STYLE;
      regs.x.bx = 1;
      regs.x.cx = 4;
      regs.x.dx = 5;
      int86(MOUSE_DRIVER_INT, &regs, &regs);
      mouse_is_useable = TRUE;
   }
   else
   {
      mouse_is_useable = FALSE;
   }
   return(mouse_is_useable);
}
  
  
void shut_down_mouse_device(void)
{
   union REGS regs;
  
   if (mouse_is_useable)
   {
      regs.x.ax = DISABLE_MOUSE_CURSOR;
      int86(MOUSE_DRIVER_INT, &regs, &regs);
      mouse_is_useable = FALSE;
   }
}
  
void enable_mouse_cursor(int row, int column)
{
  
}
  
void disable_mouse_cursor(void)
{
  
}
  
unsigned int sample_mouse_position(int * row, int * column,
   int *XPos, int *YPos)      // 10/18/90 DAI
{
   union REGS regs;
  
   if (mouse_is_useable)
   {
      regs.x.ax = GET_POSITION_AND_BUTTON;
      int86(MOUSE_DRIVER_INT, &regs, &regs);
  
      *column = (regs.x.cx / 8);
      *row = (regs.x.dx / 8);
      *XPos = regs.x.cx;    
      *YPos = regs.x.dx;

      return (regs.x.bx & MOUSE_BUTTONS);
   }
   else
   {
      *column = 0;
      *row = 0;
      return (0);
   }
}

void erase_mouse_cursor(void)  
{

}

void replace_mouse_cursor() 
{

}

void set_mouse_cursor_type( USHORT )
{

}

void MouseCursorEnable( BOOLEAN )
{

}

BOOLEAN GetMouseCursorEnable( void )
{
   return TRUE;
}

