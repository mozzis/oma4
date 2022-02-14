/* -----------------------------------------------------------------------
/
/  scrngss.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  This is a collection of routines to do screen I/O.  This module
/  supports GSS screen I/O, using GSS*GKS to write to the screen.
/
*/ /*
  $Header:   J:/logfiles/forms/scrngss.c_v   1.14   10 Jan 1992 13:40:06   cole  $
  $Log:   J:/logfiles/forms/scrngss.c_v  $
*/ /*
/ ----------------------------------------------------------------------- */

#include <stdlib.h>
#include <memory.h>
#include <malloc.h>     
#include <dos.h>
#include <stdio.h>      

#ifdef PROT
//    #include <os2_d16m.h>
#endif

#include "forms.h"
#include "coolstat.h"   

#define SWM_XOR        7
#define SWM_REPLACE    4

//BOOLEAN deferGraphCursor = FALSE;

#define  FOREGROUND(x) ((int) (x & 0xF))
#define  BACKGROUND(x) ((int) ((x >> 4) & 0xF))

#define NUM_EGA_COLORS 16
#define NUM_VGA_COLORS 256
#define NUM_GRAYS 64

extern CDVHANDLE screen_handle;
extern CDVHANDLE printer_handle;

extern CDVCAPABILITY screen;

SHORT text_cell_height, text_cell_width;
UCHAR screen_rows = DEFAULT_SCREEN_ROWS;
UCHAR screen_columns = DEFAULT_SCREEN_WIDTH;

static char OverwriteTextCursorData[] =
{  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static char InsertTextCursorData[] =
{  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

CXY CursorBMSize = {16,16};                  

CBMHANDLE bmOverwriteCursor, bmInsertCursor;

CMARKERTYPE TextCursor = CURSOR_TYPE_NORMAL; 

BOOLEAN TextCursorOn = FALSE;

static CXY LastTextCursorLoc;

/* -----------------------------------------------------------------------
/
/  UCHAR ColorTable[16]  change 15 to 16, RAC 4-APR-90
/
/  purpose:
/
/ ----------------------------------------------------------------------- */

CRGB EGAColorTable[NUM_EGA_COLORS] = {                
   {0,    0,    0    }, /* Black */
   {1000, 1000, 1000 }, /* Bright White */             
   {0,    500,  0    }, /* Green */
   {600,  0,    0    }, /* Red */
   {0,    500,  500  }, /* Cyan */
   {500,  0,    500  }, /* Purple */
   {500,  400,  0    }, /* Yellow (brown, really) */
   {500,  500,  500  }, /* White (grey?) */          
   {0,    0,    1000 }, /* Bright Blue */
   {0,    1000, 0    }, /* Bright Green */
   {1000, 0,    0    }, /* Bright Red */
   {0,    1000, 1000 }, /* Bright Cyan */
   {1000, 0,    1000 }, /* Bright Purple */
   {1000, 1000, 0    }, /* Bright Yellow */
   {0,    0,    600  }, /* Blue */                     
   {1000, 500,  0    }  /* Bright Orange */
};

CRGB ColorTable[NUM_VGA_COLORS] = {                
   {0,    0,    0    }, /* Black */
   {1000, 1000, 1000 }, /* Bright White */             
   {0,    500,  0    }, /* Green */
   {600,  0,    0    }, /* Red */
   {0,    500,  500  }, /* Cyan */
   {500,  0,    500  }, /* Purple */
   {500,  400,  0    }, /* Yellow (brown, really) */
   {500,  500,  500  }, /* White (grey?) */          
   {0,    0,    1000 }, /* Bright Blue */
   {0,    1000, 0    }, /* Bright Green */
   {1000, 0,    0    }, /* Bright Red */
   {0,    1000, 1000 }, /* Bright Cyan */
   {1000, 0,    1000 }, /* Bright Purple */
   {1000, 1000, 0    }, /* Bright Yellow */
   {0,    0,    600  }, /* Blue */                     
   {1000, 500,  0    }  /* Bright Orange */
};

CRGB HPColorTable[NUM_EGA_COLORS] = {                
/* 1 */ {0,    0,    0    }, /* Black */
/* 2 */ {1000, 1000, 1000 }, /* Bright White */             
/* 3 */ {0,    1000, 0    }, /* Green */
/* 4 */ {1000, 0,    0    }, /* Red */
/* 5 */ {0,    1000, 1000 }, /* Cyan */
/* 6 */ {1000, 0,    1000 }, /* Purple */
/* 7 */ {1000, 1000, 0    }, /* Yellow (brown, really) */
/* 8 */ {1000, 1000, 1000 }, /* White (grey?) */

/* 9 */ {0,    0,    1000 }, /* Bright Blue */
/*10 */ {0,    1000, 0    }, /* Bright Green */
/*11 */ {1000, 0,    0    }, /* Bright Red */
/*12 */ {0,    1000, 1000 }, /* Bright Cyan */
/*13 */ {1000, 0,    1000 }, /* Bright Purple */
/*14 */ {1000, 1000, 0    }, /* Bright Yellow */
/*15 */ {0,    0,    1000 }, /* Blue */                     
/*16 */ {1000, 1000,  0   }  /* Bright Orange */
};

/* -----------------------------------------------------------------------
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

UCHAR set_attributes(UCHAR fore, UCHAR back)
{
  return((UCHAR)((fore & 0xF) | ((back & 0xF) << 4)));
}


/* -----------------------------------------------------------------------
/  function:   converts screen rows to a GSS y coordinate.  (0,0) is
/              in the lower left corner, and screen row 0 is usually
/              considered to be at the top of the screen.  After the
/              row number is inverted, it is scaled by the height of
/              GSS characters.
/  requires:   (int) row - the row number
/  returns:    (int) - the GSS y coordinate
/  side effects:
/ ----------------------------------------------------------------------- */
int row_to_y(int row)
{
  return (screen_rows - row) * text_cell_height;
}

/* -----------------------------------------------------------------------
/  function:   converts a column number into a GSS x coordinate.
/  requires:   (int) column - the column number
/  returns:    (int) - the GSS x coordinate
/  side effects:
/
/ ----------------------------------------------------------------------- */
int column_to_x(int column)
{
  return (column * text_cell_width);
}

/* -----------------------------------------------------------------------
/  function:   converts a GSS y coordinate to screen row.  (0,0) is
/              in the lower left corner, and screen row 0 is usually
/              considered to be at the top of the screen.
/  requires:   (int) coord - the GSS y coordinate
/  returns:    (int) - the row number
/  side effects:
/ ----------------------------------------------------------------------- */
int y_to_row(int coord)
{
  return (screen_rows-1) - (coord / text_cell_height);
}

/* -----------------------------------------------------------------------
/  function:   converts a GSS x coordinate into a column number.
/  requires:   (int) coord - the GSS x coordinate
/  returns:    (int) - the column number
/  side effects:
/ ----------------------------------------------------------------------- */
int x_to_column(int coord)
{
  return (coord / text_cell_width);
}

void init_colors(CDVHANDLE device, int colors)
{
  SHORT i;

  colors = min(colors, NUM_VGA_COLORS);

  if (colors > NUM_EGA_COLORS)
    {
    SHORT minColor;
    FLOAT fcolors = (FLOAT)colors;

    if (device == screen_handle)
      {
      minColor = NUM_EGA_COLORS;
      fcolors -= (float)NUM_EGA_COLORS;
      }
    else
      minColor = 0;

    for (i = minColor; i < colors; i++)
      {
      ColorTable[i].Red   = 
      ColorTable[i].Green = 
      ColorTable[i].Blue  = (USHORT)((((FLOAT)(i-minColor)) / fcolors)
                             * 1000.0F);
      }
    }
  else
    {
    if (device == printer_handle)
      {
      for (i = 0; i < colors; i++)
        {
        ColorTable[i].Red   = HPColorTable[i].Red;
        ColorTable[i].Green = HPColorTable[i].Green;
        ColorTable[i].Blue  = HPColorTable[i].Blue;
        }
      }
    else
      {
      for (i = 0; i < colors; i++)
        {
        ColorTable[i].Red   = EGAColorTable[i].Red;
        ColorTable[i].Green = EGAColorTable[i].Green;
        ColorTable[i].Blue  = EGAColorTable[i].Blue;
        }
      }
    }
  CSetColorTable(device, 0, colors, ColorTable);
}

/* -----------------------------------------------------------------------
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void init_display_device(void)
{
  CATEXTCAP ScreenFont;

  CInqATextCap(screen_handle, &ScreenFont);

  screen_rows = (UCHAR)ScreenFont.Cells.Row;
  screen_columns = (UCHAR)ScreenFont.Cells.Col;

  text_cell_height = ScreenFont.CellWidth.y / ScreenFont.CellPositions.Row;
  text_cell_width  = ScreenFont.CellWidth.x / ScreenFont.CellPositions.Col;
 
  init_colors(screen_handle, screen.Colors);
}

/* -----------------------------------------------------------------------
/  function:   sets the cursor position on the screen.
/  requires:   (int) row - the row on the screen
/              (int) column - the column on the screen
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void set_cursor(int row, int column) 
{
   CPIXOPS SelMode;
   CXY CursorLoc;

   CursorLoc.x = column_to_x(column);
   CursorLoc.y = row_to_y(row + 1);

   // Set the drawing mode to XOR
   CSetWritingMode(screen_handle, CdXORs, &SelMode);

   // if cursor is showing, erase it from this previous position
   if (TextCursorOn)
      CPolymarker(screen_handle, 1, &LastTextCursorLoc);

   // output cursor to new position
   CPolymarker(screen_handle, 1, &CursorLoc);
   LastTextCursorLoc = CursorLoc;

   // Set the drawing mode to replace the current pixels
   CSetWritingMode(screen_handle, CReplace, &SelMode);

   TextCursorOn = TRUE;
}

//------------------------------------------------------------------------
static void setMarkerRep(enum CursorType ctype)
{
   CMARKERREPR markerRep;

   markerRep.Type      = CMK_UserDefined;
   markerRep.Color     = BRT_WHITE;
   markerRep.Height    = 0;      // dummy number for an unused parameter
   markerRep.HotSpot.x = 0;
   markerRep.HotSpot.y = 0;

   if (ctype == CURSOR_TYPE_OVERSTRIKE)
      markerRep.Handle = bmOverwriteCursor;
   else
      markerRep.Handle = bmInsertCursor;

   CSetMarkerRepr(screen_handle, & markerRep); 
}

/* -----------------------------------------------------------------------
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void set_cursor_type(enum CursorType ctype)
{
   CPIXOPS SelMode;

   if(TextCursorOn)
   {
      // Set the drawing mode to XOR

      CSetWritingMode(screen_handle, CdXORs, & SelMode);

      CPolymarker(screen_handle, 1, & LastTextCursorLoc);
   }

   // N.B. graphics mode changes the cursor type to a cross hair '+' symbol
   
   setMarkerRep(ctype);
   
   if(TextCursorOn)
   {
      CPolymarker(screen_handle, 1, & LastTextCursorLoc);

      // Set the drawing mode back to replace the current pixels

      CSetWritingMode(screen_handle, CReplace, & SelMode);
   }
}

void set_cursor_type_default(void)
//---------------------------------------------------------------------
{
   set_cursor_type( CURSOR_TYPE_NORMAL);
}

/* -----------------------------------------------------------------------
/  function: if cursor is showing, erase it.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
void erase_cursor(void)
{
   CPIXOPS SelMode;              

   // if cursor is showing, erase it from this previous position
   if (TextCursorOn)
   {
      // Set the drawing mode to XOR
      CSetWritingMode(screen_handle, CdXORs, &SelMode);

      CPolymarker(screen_handle, 1, &LastTextCursorLoc);

      // Set the drawing mode back to replace the current pixels
      CSetWritingMode(screen_handle, CReplace, &SelMode);
   }

   TextCursorOn = FALSE;
}

/* -----------------------------------------------------------------------
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void display_string(char * string, int len, int row, int column, int attrib)
{
   CXY ReqXY, SelXY;                   
   CCOLOR SelColor;

   erase_cursor();                     
   erase_mouse_cursor();               

   ReqXY.x = column_to_x(column);      
   ReqXY.y = row_to_y(row + 1);
   CSetATextPosition(screen_handle, ReqXY, &SelXY);  

   CSetATextColor(screen_handle, FOREGROUND(attrib), &SelColor);
   CSetBgColor(screen_handle, BACKGROUND(attrib), &SelColor);

   string[len] = 0;

   CAText(screen_handle, string, &SelXY);

   replace_mouse_cursor();             
}

/* -----------------------------------------------------------------------
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void emit(char character, int row, int column, int attrib)
{
  char  temp_string[2];

  temp_string[0] = character;
  display_string(temp_string, 1, row, column, attrib);
}

/* -----------------------------------------------------------------------
/  void show_overflow(char overflow_status, int row, int column, int attrib)
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void show_overflow(char overflow_status, int row, int column, int attrib)
{
   char        temp_string[2];
   CXY         char_box;
   CXY         arrow_tip[3];
   CXY         SelXY;                   
   CCOLOR SelColor;

   temp_string[1] = 0;

   char_box.x = column_to_x(column);
   char_box.y = row_to_y(row + 1);

   CSetATextPosition(screen_handle, char_box, &SelXY); 

   CSetATextColor(screen_handle, FOREGROUND(attrib), &SelColor);
   CSetBgColor(screen_handle, BACKGROUND(attrib), &SelColor);

   temp_string[0] = SPACE;
   CAText(screen_handle, temp_string, &SelXY);

   if (overflow_status != OVERFLOW_NONE)
   {
      int cell_width = (text_cell_width - screen.MinLineWidth);
      int cell_height = (text_cell_height - screen.MinLineWidth);
      int x_offset = (cell_width / 4);
      int y_offset = (cell_height / 6);
      int y_center = (cell_height / 2);

      
      CSetLineColor(screen_handle, FOREGROUND(attrib), &SelColor);

      arrow_tip[1].x = char_box.x;                 /* draw arrow shaft */
      arrow_tip[1].y = (char_box.y + y_center);
      arrow_tip[0].x = (arrow_tip[1].x + cell_width);
      arrow_tip[0].y = arrow_tip[1].y;
      CPolyline(screen_handle, 2, arrow_tip);

      if ((overflow_status == OVERFLOW_AT_END)
         || (overflow_status == OVERFLOW_IN_MIDDLE))
      {                                               /* draw left arrow tip */
         arrow_tip[0].x = (arrow_tip[1].x + x_offset);
         arrow_tip[0].y = (arrow_tip[1].y + y_offset);

         arrow_tip[2].x = (arrow_tip[1].x + x_offset);
         arrow_tip[2].y = (arrow_tip[1].y - y_offset);

         CPolyline(screen_handle, 3, arrow_tip);   
      }

      if ((overflow_status == OVERFLOW_AT_START)
         || (overflow_status == OVERFLOW_IN_MIDDLE))
      {                                             /* draw right arrow tip */
         arrow_tip[1].x = (char_box.x + cell_width);

         arrow_tip[0].x = (arrow_tip[1].x - x_offset);
         arrow_tip[0].y = (arrow_tip[1].y + y_offset);

         arrow_tip[2].x = (arrow_tip[1].x - x_offset);
         arrow_tip[2].y = (arrow_tip[1].y - y_offset);

         CPolyline(screen_handle, 3, arrow_tip);   
      }
   }
}

/* -----------------------------------------------------------------------
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void show_toggle(int row, int column, int attrib)
{
   int cell_width;
   int cell_height;
   int quarter_y;
   int quarter_x;
   int y_center;
   char temp_string[2];
   CXY char_box;
   CXY shape[4];
   CXY SelXY;           
   CCOLOR SelColor;

   char_box.x = column_to_x(column);
   char_box.y = row_to_y(row + 1);

   CSetATextPosition(screen_handle, char_box, &SelXY);

   CSetATextColor(screen_handle, FOREGROUND(attrib), &SelColor);
   CSetBgColor(screen_handle, BACKGROUND(attrib), &SelColor);

   temp_string[0] = SPACE;
   temp_string[1] = 0;
   CAText(screen_handle, temp_string, &SelXY);      

   cell_width = (text_cell_width - screen.MinLineWidth);
   cell_height = (text_cell_height - screen.MinLineWidth);
   quarter_x = (cell_width / 4);
   quarter_y = (cell_height / 4);
   y_center = (cell_height / 2);

   shape[0].x = (char_box.x + quarter_x);
   shape[0].y = (char_box.y + y_center);
   shape[1].x = (char_box.x + (quarter_x * 3));
   shape[1].y = (char_box.y + (quarter_y * 3));
   shape[2].x = shape[1].x;
   shape[2].y = (char_box.y + quarter_y);
   shape[3].x = shape[0].x;
   shape[3].y = shape[0].y;

   CSetFillColor(screen_handle, FOREGROUND(attrib), &SelColor);
   CFillArea(screen_handle, 4, shape);       
}

/* -----------------------------------------------------------------------
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void scroll_up(char row, char column, char n_rows, char n_columns)
{
  CBMHANDLE   screen_bitmap;       
  CRECT       temp_xy;             
  CRECT       source_area;         
  CRECT       dest_area;           

  source_area.ll.x = column_to_x(column);
  source_area.ll.y = row_to_y(row + n_rows);
  source_area.ur.x = column_to_x(column + n_columns);
  source_area.ur.y = row_to_y(row + 1);

  dest_area.ll.x = source_area.ll.x;
  dest_area.ll.y = row_to_y(row + n_rows - 1);

  erase_mouse_cursor();

  CInqDrawingBitmap(screen_handle, &screen_bitmap, &temp_xy);
  CCopyBitmap(screen_handle, screen_bitmap, source_area, dest_area.ll);
  
  replace_mouse_cursor();      
}

/* -----------------------------------------------------------------------
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void scroll_down(char row, char column, char n_rows, char n_columns)
{
  CBMHANDLE   screen_bitmap;    
  CLINEREPR   linespec;
  CRECT       temp_xy;
  CRECT       source_area;
  CRECT       dest_area;

  CInqLineRepr(screen_handle, &linespec);
  
  source_area.ll.x = column_to_x(column);
  source_area.ll.y = row_to_y((row + n_rows) - 1);
  source_area.ur.x = column_to_x(column + n_columns);
  source_area.ur.y = row_to_y(row);

  dest_area.ll.x = source_area.ll.x;
  dest_area.ll.y = row_to_y(row + n_rows);

  erase_mouse_cursor();      
  CInqDrawingBitmap(screen_handle, &screen_bitmap, &temp_xy);
  CCopyBitmap(screen_handle, screen_bitmap, source_area, dest_area.ll);
  replace_mouse_cursor();      
}

/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void erase_screen_area(UCHAR row, UCHAR col,
                       UCHAR rows, UCHAR cols,
                       UCHAR attrib, BOOLEAN border)
{
  CRECT diagonal;
  CINTERIORFILL SelStyle;
  CCOLOR SelColor;

  erase_mouse_cursor();

  diagonal.ur.x = column_to_x(col + cols);
  diagonal.ll.x = column_to_x(col);
  diagonal.ur.y = row_to_y(row);
  diagonal.ll.y = row_to_y(row + rows);

  if ((unsigned)diagonal.ur.x > (unsigned)0x7FFF)
    diagonal.ur.x = 0x7FFF;

  if ((unsigned)diagonal.ur.y > (unsigned)0x7FFF)
    diagonal.ur.y = 0x7FFF;

  CSetFillInterior (screen_handle, CSolidFill, &SelStyle);
  CSetFillColor(screen_handle, BACKGROUND(attrib), &SelColor);
  CBar(screen_handle, diagonal);

  diagonal.ur.y = row_to_y(row);
  diagonal.ur.x = column_to_x(col + cols);

  if (border)
    {
    int half_width = ((text_cell_width / 2));
    int half_height = ((text_cell_height / 2));

    diagonal.ur.x -= half_width;
    diagonal.ur.y -= half_height;
    diagonal.ll.x += half_width;
    diagonal.ll.y += half_height;

    CSetFillInterior (screen_handle, CHollowFill, &SelStyle);
    CSetFillColor(screen_handle, FOREGROUND(attrib), &SelColor);
    CBar(screen_handle, diagonal);
    CSetFillInterior (screen_handle, CSolidFill, &SelStyle);
    CSetFillColor(screen_handle, BACKGROUND(attrib), &SelColor);
    }
  replace_mouse_cursor();
  displayCoolerStatus(TRUE);  
}

/* -----------------------------------------------------------------------
/  function: save an area of the screen if possible
/  requires: screen area to save, in alpha (text) row and column format
/  returns:  pointer to saved screen area info
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
DOUBLE XVirToPhys(void)
{
  return (DOUBLE)((DOUBLE)screen.LastVDCXY.x / (DOUBLE)screen.LastXY.x);
}

DOUBLE YVirToPhys(void)
{
  return(DOUBLE)((DOUBLE)screen.LastVDCXY.y / (DOUBLE)screen.LastXY.y);
}

/* -----------------------------------------------------------------------*/
void CalcSaveArea(SaveAreaInfo * Info, USHORT *width, USHORT *height)
{
  *width = Info->diagonal.ur.x - Info->diagonal.ll.x;
  *width = (USHORT)(((DOUBLE)*width / XVirToPhys()) + 0.5);

  *height = Info->diagonal.ur.y - Info->diagonal.ll.y;
  *height = (USHORT)(((DOUBLE)*height / YVirToPhys()) + 0.5);
}

/* -----------------------------------------------------------------------*/
SaveAreaInfo * save_screen_area(UCHAR row, UCHAR col,
                                UCHAR rows, UCHAR cols)
{
  SaveAreaInfo *remember;
  ULONG        save_size;
  USHORT       width, height;
  CRECT        temp_xy;
  CBMHANDLE    screen_bitmap;

  erase_mouse_cursor();

  if ((remember = malloc(sizeof(SaveAreaInfo))) != NULL)
    {
    remember->diagonal.ur.x = column_to_x(col + cols) + (screen.MinLineWidth * 6);
    remember->diagonal.ur.y = row_to_y(row) + (screen.MinLineWidth * 6);
    remember->diagonal.ll.x = column_to_x(col);
    remember->diagonal.ll.y = row_to_y(row + rows);

    if (remember->diagonal.ll.x < 0)
      remember->diagonal.ll.x = 0;

    if ((USHORT)remember->diagonal.ur.x > (USHORT)0x7FFF)
      remember->diagonal.ur.x = 0x7FFF;

    if ((USHORT)remember->diagonal.ur.y > (USHORT)0x7FFF)
      remember->diagonal.ur.y = 0x7FFF;

    CInqDrawingBitmap(screen_handle, &screen_bitmap, &temp_xy);
    /* create a bitmap to save the area that will be covered by the menu */
    if (CCreateBitmap(screen_handle, remember->diagonal, CFullDepth,
                      &remember->BitMap) != -1)
      {
      /* select the newly created bitmap as the drawing bitmap */
      CSelectDrawingBitmap(screen_handle, remember->BitMap);
      /* copy the specified area from the screen into the new bitmap */
      CCopyBitmap(screen_handle, screen_bitmap, remember->diagonal,
        remember->diagonal.ll);
      /* reset so that screen bitmap is where new drawing will go */
      CSelectDrawingBitmap(screen_handle, screen_bitmap);
      remember->pixels = NULL;
      }
    else
      {
      CalcSaveArea(remember, &width, &height);

      save_size = (ULONG)((ULONG)width * (ULONG)height);

      remember->pixels = halloc(save_size, 1);
      if (remember->pixels == NULL)
        {
        free(remember);
        return(NULL);
        }
      remember->BitMap = -1;
      if (CInqBytePixels(screen_handle,
                        remember->diagonal.ll,
                        width,
                        height,
                        &remember->valid_col,
                        &remember->valid_row,
                        (void far *)remember->pixels) == -1)
        {
        hfree(remember->pixels);
        free(remember);
        return(NULL);
        }
      remember->valid_col.Max++;
      }

    }
  replace_mouse_cursor();
  return(remember);
}
  
/* -----------------------------------------------------------------------
/  function:   restore previously saved screen area
/  requires:   pointer to saved screen area info.  may be null.
/  returns:    truth that area was restored
/  side effects:
/ ----------------------------------------------------------------------- */
  
BOOLEAN restore_screen_area(SaveAreaInfo * save_buffer)
{
  BOOLEAN retval = FALSE;
  USHORT save_width, save_height;

  if (save_buffer)
    {
    erase_mouse_cursor();

    if (save_buffer->pixels == NULL)
      {
      /* copy from saved area bitmap onto the screen */
      CCopyBitmap(screen_handle, save_buffer->BitMap,
                  save_buffer->diagonal, save_buffer->diagonal.ll);

      /* delete the saved area bitmap */
      CDeleteBitmap(screen_handle, save_buffer->BitMap);
      retval = TRUE;
      }
    else
      {
      CalcSaveArea(save_buffer, &save_width, &save_height);

        /* copy from saved area bitmap onto the screen */

      if (CBytePixels(screen_handle,
                      save_buffer->diagonal.ll,
                      save_width,
                      save_height,
                      save_buffer->valid_col,
                      save_buffer->valid_row,
                      (void far *)save_buffer->pixels) != -1)
        retval = TRUE;
    
      hfree(save_buffer->pixels);
      }
    free(save_buffer);
    replace_mouse_cursor();
    }
  return(retval);
}

void InitTextCursors()
{
   CBMHANDLE bmScreen;
   CRECT box;                       
   CRECT xy;
   CMINMAX val_wid, val_hgt;
   float PixToVDC[2];

   CInqDrawingBitmap(screen_handle,&bmScreen, &box);    

   val_wid.Min = 0;
   val_wid.Max = (CursorBMSize.x / 2) - 1;
   val_hgt.Min = 0;
   val_hgt.Max = CursorBMSize.y - 1;

   PixToVDC[0] = (float) screen.LastVDCXY.x /(float)screen.LastXY.x;
   PixToVDC[1] = (float) screen.LastVDCXY.y /(float)screen.LastXY.y;
   xy.ll.x = 0;
   xy.ll.y = 0;
   xy.ur.x = CursorBMSize.x * (int) PixToVDC[0];   
   xy.ur.y = CursorBMSize.y * (int) PixToVDC[1];
   CCreateBitmap(screen_handle, xy, CFullDepth, &bmInsertCursor);
   CCreateBitmap(screen_handle, xy, CFullDepth, &bmOverwriteCursor);
   CSelectDrawingBitmap(screen_handle, bmInsertCursor);

   CBytePixels(screen_handle, xy.ll,              
   CursorBMSize.x, CursorBMSize.y,
   val_wid, val_hgt, InsertTextCursorData);

   CSelectDrawingBitmap(screen_handle, bmOverwriteCursor);

   CBytePixels(screen_handle, xy.ll,              
   CursorBMSize.x, CursorBMSize.y,
   val_wid, val_hgt, OverwriteTextCursorData);

   /* SELECT THE SCREEN BITMAP.  CREATE THE CURSOR. */
   CSelectDrawingBitmap(screen_handle, bmScreen);   
}

CXY GetTextCursorLoc(short *Row, short *Column, BOOLEAN *On) 
{
   *Row = y_to_row(LastTextCursorLoc.y);
   *Column = x_to_column(LastTextCursorLoc.x);
   *On = TextCursorOn;
   return LastTextCursorLoc;
}
