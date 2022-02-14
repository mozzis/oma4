#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <dos.h>
  
#include "forms.h"
  
/* -----------------------------------------------------------------------
/
/  scrnraw.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00          6 June        1988
/  Worked on:  TLB      Version 1.01
/
/  This is a collection of routines to do screen I/O.  This module
/  supports "Raw" I/O, that is, writing directly to video memory.
/  This is done to get the highest display speed.  Other modules
/  provide similar functions that display through BIOS, or through
/  graphics environments.  One BIOS routine is invoked here to set
/  the cursor position.
/
/  NOTE: this module calls assembler subroutines in module LVID.ASM.
/
  $Header:   C:/pvcs/logfiles/scrnraw.c_v   1.0   27 Sep 1990 15:44:18   admax  $
  $Log:   C:/pvcs/logfiles/scrnraw.c_v  $
 * 
 *    Rev 1.0   27 Sep 1990 15:44:18   admax
 * Initial revision.
/
/ ----------------------------------------------------------------------- */
  
// #define  NULL     0        // 5/29/90 DAI
// #define  NOT      !
// #define  TRUE     1
// #define  FALSE    0
  
/* these come from lvid.asm, which must be linked in when using this module */
extern VidMemWrite(void far * video_buffer, int total_columns,
int far * save_buffer, int row, int column, int cell_count);
extern VidMemRead(void far * video_buffer, int total_columns,
int far * save_buffer, int row, int column, int cell_count);
extern StringToVidMem(void far * video_buffer, int total_columns,
char far * string, int attrib, int row, int column, int len);
extern VidMemMove(void far * video_buffer, int total_columns,
int source_row, int source_column, int dest_row, int dest_column,
int cell_count);
  
/*       for single line bars all round
#define  VERTICAL_BAR_CHAR       179
#define  HORIZONAL_BAR_CHAR      196
#define  UL_CORNER_BAR_CHAR      218
#define  UR_CORNER_BAR_CHAR      191
#define  LL_CORNER_BAR_CHAR      192
#define  LR_CORNER_BAR_CHAR      217
*/
  
/* for double line horizontal, single line vertical */
#define  VERTICAL_BAR_CHAR       179
#define  HORIZONAL_BAR_CHAR      205
#define  UL_CORNER_BAR_CHAR      213
#define  UR_CORNER_BAR_CHAR      184
#define  LL_CORNER_BAR_CHAR      212
#define  LR_CORNER_BAR_CHAR      190
  
#define  BIOS_VIDEO_INT       0x10  /* actual interrupt number invoked */
#define  SET_CURSOR_SHAPE        1  /* BIOS Video Adapter Interrupt code */
#define  SET_CURSOR_POSITION     2  /* BIOS Video Adapter Interrupt code */
  
#define  MONO_CARD_ADDRESS    0x3b4 /* mono adapter I/O port base address */
#define  COLOR_CARD_ADDRESS   0x3d4 /* color adapter I/O port base address */
  
  
#define  RIGHT_ARROW_CHAR        26
#define  LEFT_ARROW_CHAR         27
#define  LR_ARROW_CHAR           29
  
  
#define  TOGGLE_FIELD_CHAR       17    // TLB 7/9/90
  
extern unsigned char    screen_rows;
extern unsigned char    screen_columns;
  
  
static void far * video_buffer;
static char cursor_vertical_trace_size = 7;
  
  
/* -----------------------------------------------------------------------
/
/  unsigned char  ColorTable[15]
/
/  purpose:
/
/ ----------------------------------------------------------------------- */
  
unsigned char  ColorTable[15] = {
   0x00,   /* Black */
   0x01,   /* Blue */
   0x02,   /* Green */
   0x04,   /* Red */
   0x03,   /* Cyan */
   0x05,   /* Purple */
   0x06,   /* Yellow (brown, really) */
   0x07,   /* White (grey?) */
   0x09,   /* Bright Blue */
   0x0A,   /* Bright Green */
   0x0C,   /* Bright Red */
   0x0B,   /* Bright Cyan */
   0x0D,   /* Bright Purple */
   0x0E,   /* Bright Yellow */
   0x0F    /* Bright White */
};
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
unsigned char set_attributes(unsigned char fore, unsigned char back)
{
   return( (ColorTable[fore] & 0xF) | ((ColorTable[back] & 0x7) << 4) );
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void init_display_device(void)
{                                                           /* 40:0063 */
   unsigned int far *   BIOS_VID_CARD_ADDR = (unsigned int far *) 0x400063L;
   unsigned char far *  BIOS_ROW_COUNT = (unsigned char far *) 0x400084L;
   unsigned char far *  BIOS_COLUMN_COUNT = (unsigned char far *) 0x40004aL;
   unsigned int   vid_card_addr;
  
   vid_card_addr = *BIOS_VID_CARD_ADDR;
  
   if (vid_card_addr == MONO_CARD_ADDRESS)
   {
      video_buffer = (void far *) 0xb0000000L;     /* B000:0000 */
      cursor_vertical_trace_size = 14;
   }
   else
   {
      video_buffer = (void far *) 0xb8000000L;     /* B800:0000 */
      cursor_vertical_trace_size = 7;
   }
  
   if (*BIOS_ROW_COUNT > 0)
      screen_rows = (*BIOS_ROW_COUNT + 1);   /* bios ROWS are zero based */
   else
      screen_rows = DEFAULT_SCREEN_ROWS;
  
   screen_columns = *BIOS_COLUMN_COUNT;
}
  
  
/* -----------------------------------------------------------------------
/
/  void set_cursor(int row, int column)
/
/  function:   calls the BIOS video services interrupt to set the
/              cursor position on the screen.
/  requires:   (int) row - the row on the screen
/              (int) column - the column on the screen
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void set_cursor(int row, int column)
{
   union REGS regs;
   unsigned int intrrp_status;
  
   regs.h.ah = SET_CURSOR_POSITION;
   regs.h.al = 0;
   regs.h.bh = 0;    /* video page number */
   regs.h.bl = 0;
   regs.h.dh = (char) row;
   regs.h.dl = (char) column;
  
   intrrp_status = int86(BIOS_VIDEO_INT, &regs, &regs);
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void emit(char character, int row, int column, int attrib)
{
   StringToVidMem(video_buffer, (int) screen_columns, &character, attrib,
   row, column, 1);
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void show_overflow(char overflow_status, int row, int column, int attrib)
{
   char     character;
  
   switch (overflow_status)
   {
      case OVERFLOW_NONE:
         character = SPACE;
         break;
      case OVERFLOW_AT_START:
         character = RIGHT_ARROW_CHAR;
         break;
      case OVERFLOW_IN_MIDDLE:
         character = LR_ARROW_CHAR;
         break;
      case OVERFLOW_AT_END:
         character = LEFT_ARROW_CHAR;
         break;
   }
  
   StringToVidMem(video_buffer, (int) screen_columns, &character, attrib,
   row, column, 1);
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
// TLB 7/9/90
void show_toggle(int row, int column, int attrib)
{
   char     character = TOGGLE_FIELD_CHAR;
  
   StringToVidMem(video_buffer, (int) screen_columns, &character, attrib,
   row, column, 1);
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void display_string(char * string, int len, int row, int column, int attrib)
{
   StringToVidMem(video_buffer, (int) screen_columns, string, attrib,
   row, column, len);
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void scroll_up(char row, char column, char n_rows, char n_columns)
{
   int   i;
  
   for (i=1; i < n_rows; i++)
   {
      VidMemMove(video_buffer, (int) screen_columns,
      (row+i), column, (row+(i-1)), column, n_columns);
   }
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void scroll_down(char row, char column, char n_rows, char n_columns)
{
   int   i;
  
   for (i=(n_rows-1); i > 0; i--)
   {
      VidMemMove(video_buffer, (int) screen_columns,
      (row+(i-1)), column, (row+i), column, n_columns);
   }
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void erase_screen_area(unsigned char row, unsigned char column,
unsigned char n_rows, unsigned char n_columns, unsigned char attrib,
BOOLEAN border)
{
   int   i;
   int   last_row = (row + (n_rows - 1));
   char  space_string[MAX_FIELD_SIZE];
  
   if (n_columns > MAX_FIELD_SIZE)
      n_columns = MAX_FIELD_SIZE;
  
   if (border)
   {
      char_fill(space_string, n_columns, SPACE);
      space_string[0] = VERTICAL_BAR_CHAR;
      space_string[ n_columns - 1 ] = VERTICAL_BAR_CHAR;
  
      for (i = (row + 1); i < last_row; i++)
      {
         StringToVidMem(video_buffer, (int) screen_columns, space_string,
         attrib, i, column, n_columns);
      }
      char_fill(space_string, n_columns, HORIZONAL_BAR_CHAR);
  
      space_string[0] = UL_CORNER_BAR_CHAR;
      space_string[ n_columns - 1 ] = UR_CORNER_BAR_CHAR;
      StringToVidMem(video_buffer, (int) screen_columns, space_string,
      attrib, row, column, n_columns);
  
      space_string[0] = LL_CORNER_BAR_CHAR;
      space_string[ n_columns - 1 ] = LR_CORNER_BAR_CHAR;
      StringToVidMem(video_buffer, (int) screen_columns, space_string,
      attrib, last_row, column, n_columns);
   }
   else
   {
      char_fill(space_string, n_columns, SPACE);
  
      for (i = row; i <= last_row; i++)
      {
         StringToVidMem(video_buffer, (int) screen_columns, space_string,
         attrib, i, column, n_columns);
      }
   }
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
unsigned int * save_screen_area(unsigned char row, unsigned char column,
unsigned char n_rows, unsigned char n_columns)
{
   int               i;
   int               buffer_size = (n_rows * n_columns);
   unsigned int *    save_buffer;
  
   save_buffer = (unsigned int *) malloc(buffer_size * sizeof(unsigned int));
  
   if (save_buffer != NULL)
   {
      for (i = 0; i < n_rows; i++)
      {
         VidMemRead(video_buffer, (int) screen_columns,
         &save_buffer[i * n_columns], (row+i), column, n_columns);
      }
   }
   return(save_buffer);
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void restore_screen_area(unsigned int * save_buffer, unsigned char row,
unsigned char column, unsigned char n_rows, unsigned char n_columns)
{
   int               i;
   int               buffer_size = (n_rows * n_columns);
  
   if (save_buffer != NULL)
   {
      for (i = 0; i < n_rows; i++)
      {
         VidMemWrite(video_buffer, (int) screen_columns,
         &save_buffer[i * n_columns], (row+i), column, n_columns);
      }
      free(save_buffer);
   }
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void set_cursor_type(int type)
{
   union REGS regs;
   unsigned int intrrp_status;
  
   regs.h.ah = SET_CURSOR_SHAPE;
   switch (type)
   {
      default:
      case CURSOR_TYPE_NORMAL:
         regs.h.ch = (cursor_vertical_trace_size - 1);
         regs.h.cl = cursor_vertical_trace_size;
         break;
      case CURSOR_TYPE_OVERSTRIKE:
         regs.h.ch = 0;
         regs.h.cl = cursor_vertical_trace_size;
         break;
   }
   intrrp_status = int86(BIOS_VIDEO_INT, &regs, &regs);
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void erase_cursor(void)
{
   set_cursor(screen_rows + 1, 0);
}
  
  
