/* -----------------------------------------------------------------------
/
/  mousegss.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00          6 June           1988
/  Worked on:  TLB      Version 1.01
/
/  This is a collection of routines to do mouse input.  This module
/  supports "gss" mouse input, that is, input mouse information
/  from the  GSS*GKI mouse input device.
/
  $Header:   J:/logfiles/forms/mousegss.c_v   1.7   10 Jan 1992 13:35:46   cole  $
  $Log:   J:/logfiles/forms/mousegss.c_v  $
 * 
 *    Rev 1.7   10 Jan 1992 13:35:46   cole
 * Add include device.h, delete include curvdraw.h
 * 
 *    Rev 1.6   24 Jul 1991 15:28:34   cole
 * Remove #include for cgibind.h
 * 
 *    Rev 1.5   28 May 1991 13:56:00   cole
 * remove all extern's from .c files
 * 
 *    Rev 1.4   26 Oct 1990 10:37:58   irving
 * Added raw screen coordinate parametersto sample_mouse_position
 *
 *    Rev 1.3   12 Oct 1990 13:42:14   irving
 * cleaned up some clutter
 *
 *    Rev 1.2   08 Oct 1990 16:46:04   irving
 * Added replace and erase mouse cursor plus a couple of functions
 * to use for a less transient mouse erasure.
 *
 *    Rev 1.1   05 Oct 1990 11:26:56   irving
 * added positioning and removal functions for mouse cursor
 *
 *    Rev 1.0   27 Sep 1990 15:42:10   admax
 * Initial revision.
/ ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "device.h"  // screen_handle
#include "forms.h"

static CXY LastMouseCursorPos = {0, 0};

static BOOLEAN MouseCursorOn = TRUE;

CDVHANDLE mouse_handle;

CDVCAPABILITY mouse;

#ifdef PROT

CDVOPEN mouse_setup = {
   0,1,1,3,1,1,1,0,0,1,1, "GIN"
};

#else

CDVOPEN mouse_setup = {        
   1,1,1,1,1,1,1,1,1,1,1, "MOUSE"
};

#endif

void startup_mouse( void )
{
  COpenWorkstation( & mouse_setup, & mouse_handle, & mouse ) ;
}


/* -----------------------------------------------------------------------*/
BOOLEAN init_mouse_device(void)
{
   CXY            position;
   int            status;
   CSAMPLELOCATOR Locator;
   CYESNO         SampleAvailable;

   status = CSampleLocator( mouse_handle, position, &SampleAvailable,
                            &Locator );

   mouse_is_useable = (status == 0);

   return(mouse_is_useable);
}


void shut_down_mouse_device(void)
{
}


void enable_mouse_cursor(int row, int column)
{
   /* v_dspcur(screen_handle, column_to_x(column), row_to_y(row)); */
}

void disable_mouse_cursor(void)
{
   /* v_rmcur(screen_handle); */
}

unsigned int sample_mouse_position(int * row, int * column, int *XPos, int *YPos)
{
   CXY               new_position;
   unsigned int      pressed;
   unsigned int      released;
   unsigned int      buttons;
   CSAMPLELOCATOR Locator;
   CYESNO         SampleAvailable;

   if (mouse_is_useable)
   {
      CSampleLocator( mouse_handle, LastMouseCursorPos, &SampleAvailable,
                            &Locator );

      new_position = Locator.Position;
      *XPos = new_position.x;
      *YPos = new_position.y;
      pressed = Locator.Pressed;
      released = Locator.Released;
      buttons = Locator.KeyState;

      *column = x_to_column(new_position.x);
      *row = y_to_row(new_position.y);

      if ( (LastMouseCursorPos.x != new_position.x) ||
           (LastMouseCursorPos.y != new_position.y) )
      {
         if (MouseCursorOn)
            CDisplayCursor(screen_handle, new_position );

         LastMouseCursorPos = new_position;
      }

      return (buttons & MOUSE_BUTTONS);
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
   CRemoveCursor(screen_handle);    // turn off mouse cursor
}


void replace_mouse_cursor()
{
   int Dummy;
   static CXY CursorLoc = {5000, 5000};

   if (! MouseCursorOn)
      return;

   CDisplayCursor(screen_handle, LastMouseCursorPos );
   sample_mouse_position( &Dummy, &Dummy, &Dummy, &Dummy );
}

void set_mouse_cursor_type( USHORT Type )
{
   CCURHANDLE hCursor;

   hCursor.CursorHandle = Type;
   CSelectGCursor( screen_handle, hCursor ); // mouse cursor
}

void MouseCursorEnable( BOOLEAN On )
{
   MouseCursorOn = On;
}

BOOLEAN GetMouseCursorEnable( void )
{
   return MouseCursorOn;
}

