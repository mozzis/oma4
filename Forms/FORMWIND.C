/* -----------------------------------------------------------------------
/
/  formwind.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00         17 October     1989
/  Worked on:  TLB      Version 1.01
/              DAI      1/4/90  split message_window into
/                               put_up_message_window and
/                               release_message_window
*/ /*
  $Header:   J:/logfiles/forms/formwind.c_v   1.11   10 Jan 1992 13:28:58   cole  $
  $Log:   J:/logfiles/forms/formwind.c_v  $
 * 
 *    Rev 1.11   10 Jan 1992 13:28:58   cole
 * Add include device.h, forms.h; delete include plot.h, curvdraw.h
 * 
 *    Rev 1.10   24 Jul 1991 15:20:40   cole
 * Remove error() call from popupWindowBegin() -- caller should report the
 * error if a FALSE return.  Remove #include for cgibind.h
 * 
 *    Rev 1.9   24 Jun 1991 11:08:32   cole
 * new : popupWindowBegin(), popupWindowSetup(), popupWindowEnd().
 * moved popup_left_button_handler() here from mousefrm.c
 * modified functions to allow removal of all globals, now static in formwind.c
 * moved function prototypes for O
 * PRIVATE funcions here from formwind.h
 * moved struct OPTION definition here from forms.h -- only used in formwind.c
 * moved struct WINDOW definition here from formwind.h -- now an opaque type.
 * cosmetics.
 * 
 *    Rev 1.8   28 May 1991 13:53:10   cole
 * remove all extern's from .c files
 * 
 *    Rev 1.7   23 May 1991 15:02:26   maynard
 * Deleted big commented-out section.
 * 
 *    Rev 1.6   14 May 1991 12:49:28   maynard
 * Deleted calls to heapmin
 * 
 *    Rev 1.5   19 Jan 1991 16:35:52   irving
 * Finished up indexed and titled popup windows
 * 
 *    Rev 1.4   18 Jan 1991 09:58:04   irving
 * New indexed titled popups supported.
 *
 *    Rev 1.3   17 Dec 1990 16:26:50   irving
 * Added escape return status to choice_window calls
 *
 *    Rev 1.2   12 Oct 1990 13:34:16   irving
 * mouse support for message windows
 *
 *    Rev 1.1   05 Oct 1990 11:26:22   irving
 * changes for new text cursor
 *
 *    Rev 1.0   27 Sep 1990 15:38:00   admax
 * Initial revision.
*/ /*
/ ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>

#include "formwind.h"
#include "omaform.h"
#include "device.h"  // screen_handle
#include "forms.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

typedef struct ndxcolorset {
   COLOR_PAIR  colors[5];  /* and others as needed */
} INDEX_COLOR_SET;

typedef struct wind {
   USHORT Attrib;
   UCHAR Row;
   UCHAR Column;
   UCHAR SizeInRows;
   UCHAR SizeInColumns;
   SHORT TextStringSetCount;
   char ** TextStringSet;
   INDEX_COLOR_SET * ColorSet;
   SaveAreaInfo * SavedArea;
} WINDOW;

typedef struct {
   char * text;
   UCHAR text_length;
   UCHAR column_offset;
} OPTION;

char * BusyWorking[] = { "Working...", NULL };
char * BusyWorkingEsc[] = { "Working...", "Press <ESC> to abort", NULL };
char * BusyWorkingBreak[] = { "Working...", "Press CTRL-BREAK to abort", NULL };

char * BlkOverWritePrompt[] = {
   "Curves in the result range already exist",
   "Overwrite existing curves?",
   NULL
};
char * DataFileOverwritePrompt[] = {
  "This option will irretrievably ",
  "overwrite an existing data file. ",
  "Continue?",
  NULL
};

#define     MAX_CHOICES             10

#define     STRING_SET_GRANULES     32

static char * yes_no_choice_text[] = { "No", "Yes", NULL };

static UCHAR PopupChoiceRow;
static OPTION * PopupChoiceOptions = NULL;
static int PopupChoiceCount = 0;

static WINDOW * PopupWindow = 0;
static WINDOW * BusyMessageWindow = 0;
// -----------------------------------------------------------------------
WINDOW * define_transient_window(UCHAR Row, UCHAR Column,
                                 UCHAR SizeInRows,
                                 UCHAR SizeInColumns,
                                 USHORT Attrib)
{
   WINDOW *    WindowPtr;

   if ((WindowPtr = malloc(sizeof(WINDOW))) != NULL)
   {
      memset(WindowPtr, 0, sizeof(WINDOW));

      WindowPtr->Row = Row;
      WindowPtr->Column = Column;
      WindowPtr->SizeInRows = SizeInRows;
      WindowPtr->SizeInColumns = SizeInColumns;
      WindowPtr->Attrib = Attrib;
   }
   return (WindowPtr);
}

// -----------------------------------------------------------------------

void destroy_transient_window(WINDOW * WindowPtr)
{
   free(WindowPtr);
   _heapmin();
}


// -----------------------------------------------------------------------
void attach_strings_to_window(WINDOW * WindowPtr, char ** StringSet)
{
   WindowPtr->TextStringSetCount = 0;
   WindowPtr->TextStringSet = StringSet;

   if(StringSet) {
      int count = 0;

      while(StringSet[ count ])
         count++;
                                         
      WindowPtr->TextStringSetCount = count;
   }
}

// -----------------------------------------------------------------------
void autosize_window(WINDOW * WindowPtr)
{
   int i;
   int width;
   int max_width = 0;
   UCHAR  required_columns;
   UCHAR  required_rows;

   for (i=0; i < WindowPtr->TextStringSetCount; i++)
   {
      width = strlen(WindowPtr->TextStringSet[i]);
      if (max_width < width)
         max_width = width;
   }
   required_columns = (UCHAR) (max_width + 2);
   required_rows = (UCHAR) (WindowPtr->TextStringSetCount + 2);

   if (WindowPtr->SizeInRows > required_rows)
      WindowPtr->SizeInRows = required_rows;

   if (WindowPtr->SizeInColumns > required_columns)
      WindowPtr->SizeInColumns = required_columns;
}

// -----------------------------------------------------------------------
void autocenter_window(WINDOW * WindowPtr)
{
   WindowPtr->Row =
   (UCHAR)((screen_rows / 2) - (WindowPtr->SizeInRows / 2));

   WindowPtr->Column =
   (UCHAR)((screen_columns / 2) - (WindowPtr->SizeInColumns / 2));
}

// -----------------------------------------------------------------------
void open_window(WINDOW * WindowPtr, UCHAR color_set_index)
{
   UCHAR attribute;

   erase_cursor();

   // moved here for OS/2 debugging
   // get protection error later on if don't have valid color set pointer
   WindowPtr->ColorSet = (INDEX_COLOR_SET *) &ColorSets[color_set_index];

   if ((WindowPtr->SavedArea = save_screen_area(WindowPtr->Row, WindowPtr->Column,
      WindowPtr->SizeInRows, WindowPtr->SizeInColumns)) != NULL)
   {
      // WindowPtr->ColorSet = (INDEX_COLOR_SET *) &ColorSets[color_set_index];

      attribute =
      set_attributes(WindowPtr->ColorSet->colors[REGULAR_COLOR].foreground,
      WindowPtr->ColorSet->colors[REGULAR_COLOR].background);
   }

   erase_screen_area(WindowPtr->Row, WindowPtr->Column,
   WindowPtr->SizeInRows, WindowPtr->SizeInColumns, attribute, TRUE);
}

// -----------------------------------------------------------------------
void close_window(WINDOW * WindowPtr)
{
   if ((WindowPtr->SavedArea != NULL))
      restore_screen_area(WindowPtr->SavedArea);
}

// -----------------------------------------------------------------------
PRIVATE void text_into_window(WINDOW * WindowPtr, char * string,
                              UCHAR row_offset,
                              UCHAR column_offset,
                              UCHAR color_index)
{
   UCHAR attribute;
   UCHAR string_len = (UCHAR) strlen(string);
   UCHAR max_column_offset;

   if (row_offset >= WindowPtr->SizeInRows)
      row_offset = (WindowPtr->SizeInRows - (char)1);

   if (string_len > WindowPtr->SizeInColumns)
      string_len = WindowPtr->SizeInColumns;

   max_column_offset = (WindowPtr->SizeInColumns - string_len);
   if (column_offset > max_column_offset)
      column_offset = max_column_offset;

   // RAC June 29, 1990.  Remove test for unsigned < 0, always FALSE
   //  if (((color_index < (UCHAR) 0) || (color_index > MAX_COLOR)))
   if (color_index > MAX_COLOR)
      color_index = REGULAR_COLOR;

   attribute =
   set_attributes(WindowPtr->ColorSet->colors[color_index].foreground,
   WindowPtr->ColorSet->colors[color_index].background);

   display_string(string, string_len, (WindowPtr->Row + row_offset),
                  (WindowPtr->Column + column_offset), attribute);
}

/* -----------------------------------------------------------------------
/  function:   UCHAR get_color_index(USHORT StringAttr)
/  requires:   (USHORT) StringAttr - can be any of the attributes
/                 STRATTR_REGULAR
/                 STRATTR_REV_VID
/                 STRATTR_HIGHLIGHT
/                 STRATTR_SHADED
/                 STRATTR_ERROR
/              along with other flags not associated with color
/
/  returns:    One of the color indices:
/                 REGULAR_COLOR
/                 REVERSE_COLOR
/                 HIGHLIGHT_COLOR
/                 SHADED_COLOR
/                 ERROR_COLOR
/  side effects:
/
/              1/15/91  DAI
/ ----------------------------------------------------------------------- */
PRIVATE UCHAR get_color_index(USHORT StringAttr)
{
   UCHAR i;

   for (i=0; i< MAX_COLOR; i++)
   {
      if ((1 << i) & (UCHAR) StringAttr)
         return (UCHAR) (i+1);
   }

   // if got this far, there is no special color condition
   return REGULAR_COLOR;
}

// -----------------------------------------------------------------------
PRIVATE void string_set_into_window(WINDOW * WindowPtr,
                                    UCHAR row_offset,
                                    int string_index, int string_count,
                                    PUSHORT attribute_set)
{
   int            i;
   UCHAR  column_offset = 1;
   char           buffer[85];
   int            line_width;
   UCHAR          color_index = REGULAR_COLOR;

   line_width = (int) (WindowPtr->SizeInColumns - column_offset - 1);

   for (i=0; i < string_count; i++)
   {
      if ((string_index + i) >= WindowPtr->TextStringSetCount)
         break;

      sprintf(buffer, "%-*s", line_width,
         WindowPtr->TextStringSet[string_index + i]);

      // get the color type for this line
      if (attribute_set != NULL)
         color_index = get_color_index(attribute_set[string_index + i]);

      text_into_window(WindowPtr, buffer, row_offset, column_offset,
         color_index);

      if (++row_offset >= WindowPtr->SizeInRows)
         break;
   }
}

// -----------------------------------------------------------------------
PRIVATE void manage_indexed_dynamic_window(WINDOW * WindowPtr,
                                           UCHAR row_offset,
                                           SHORT row_count,
                                           PUSHORT attribute_set,
                                           PUCHAR ItemRowCount,
                                           USHORT ItemCount,
                                           BOOLEAN Pointer,
                                           SHORT (*CharAction)(WINDOW *,
                                                               UCHAR,
                                                               SHORT,
                                                               PUSHORT,
                                                               PUCHAR,
                                                               USHORT,
                                                               USHORT,
                                                               BOOLEAN,
                                                               UCHAR))
{
  SHORT    TopIndex = 0;
  SHORT    OffsetIndex = 0;
  UCHAR    key;
  UCHAR    centered_column =
    ((WindowPtr->SizeInColumns / (char)2) - (char)6);

  BOOLEAN  done = FALSE;
  BOOLEAN  movement = TRUE;
  BOOLEAN  movement_permitted;
  SHORT    PreviousLocus;
  USHORT   ItemIndex = 0;
  SHORT    Count;

  PopupWindow = WindowPtr;
  PopupChoiceCount = 0;
  PopupChoiceOptions = NULL;
  PreviousLocus = active_locus;
  active_locus = LOCUS_POPUP;

  if (ItemRowCount == NULL)
    {                          // set up to refer to bottom of window
    ItemCount = WindowPtr->TextStringSetCount;
    ItemIndex = row_count - 1;
    if (ItemIndex >= ItemCount)
      ItemIndex = ItemCount;
    }

  movement_permitted = (WindowPtr->TextStringSetCount > row_count);
  do
    {
    if (movement)
      {
      movement = FALSE;

      if (Pointer)
        {
        // put arrow into first two spaces of indexed string
        WindowPtr->TextStringSet[TopIndex + OffsetIndex][0] = '=';
        WindowPtr->TextStringSet[TopIndex + OffsetIndex][1] = '>';
        }
      string_set_into_window(WindowPtr, row_offset, TopIndex,
        row_count, attribute_set);

      if (Pointer)
        {
        // replace spaces in first two spaces of indexed string
        WindowPtr->TextStringSet[TopIndex + OffsetIndex][0] = ' ';
        WindowPtr->TextStringSet[TopIndex + OffsetIndex][1] = ' ';
        }
      if (movement_permitted)
        {
        if (TopIndex == (WindowPtr->TextStringSetCount - row_count))
          {
          text_into_window(WindowPtr, " End  ", WindowPtr->SizeInRows,
            centered_column, REVERSE_COLOR);
          }
        else
          {
          text_into_window(WindowPtr, " More ", WindowPtr->SizeInRows,
            centered_column, REVERSE_COLOR);
          }
        }
      }

    NoAutoPlay = TRUE;
    key = get_FORM_key_input();
    NoAutoPlay = FALSE;

    switch (key)
      {
      case KEY_UP:
        if (ItemIndex > 0)
          {
          if (ItemRowCount != NULL)
            {
            ItemIndex--;
            if (OffsetIndex == 0)
              TopIndex -= ItemRowCount[ItemIndex];
            else
              OffsetIndex -= ItemRowCount[ItemIndex];
            }
          else
            {
            if (TopIndex != 0)
              {
              TopIndex--;
              ItemIndex = TopIndex + (row_count - 1);
              if (ItemIndex >= ItemCount)
                ItemIndex = ItemCount;
              }
            }
          movement = TRUE;
          }
      break;
      case KEY_DOWN:
        if (ItemIndex < (ItemCount-1))
          {
          if (ItemRowCount != NULL)
            {
            OffsetIndex += ItemRowCount[ItemIndex];

            ItemIndex++;
            // make sure full item shows in window
            while ((OffsetIndex + ItemRowCount[ItemIndex]) >
              row_count)
              {
              TopIndex++;
              OffsetIndex--;
              }
            }
          else
            {
            TopIndex++;
            ItemIndex = TopIndex + (row_count - 1);
            if (ItemIndex >= ItemCount)
              ItemIndex = ItemCount;
            }
          movement = TRUE;
          }
      break;

      case KEY_PG_UP:
        if (movement_permitted)
          {
          if (ItemRowCount != NULL)
            {
            // need to get to the start of item closest to the 
            // page limit without skipping over any lines
            Count = 0;
            while ((Count < row_count) && (ItemIndex > 0))
              {
              ItemIndex--;
              Count += ItemRowCount[ItemIndex];
              }
            // back up to show the full item
            if (Count > row_count)
              {
              Count -= ItemRowCount[ItemIndex];
              ItemIndex++;
              }

            OffsetIndex -= Count;
            while (OffsetIndex < 0)
              {
              TopIndex--;
              OffsetIndex++;
              }
            }
          else
            {
            TopIndex -= row_count;
            if (TopIndex < 0)
              TopIndex = 0;
            ItemIndex = TopIndex + row_count - 1;
            if (ItemIndex >= ItemCount)
              ItemIndex = ItemCount;
            }
          movement = TRUE;
          }
      break;
      case KEY_PG_DN:
        if (movement_permitted && (ItemIndex < (ItemCount - 1)))
          {
          if (ItemRowCount != NULL)
            {
            // need to get to the start of item closest to the page 
            // limit without skipping over any lines
            Count = 0;
            while ((Count < row_count) &&
              ((TopIndex + Count) < (WindowPtr->TextStringSetCount -
              ItemRowCount[ItemIndex])))
              {
              Count += ItemRowCount[ItemIndex];
              ItemIndex++;
              }
            if (Count > row_count)    // back up to show the full item
              {
              ItemIndex--;
              Count -= ItemRowCount[ItemIndex];
              }

            OffsetIndex += Count;
            while ((OffsetIndex + ItemRowCount[ItemIndex])> row_count)
              {
              TopIndex++;
              OffsetIndex--;
              }
            }
          else
            {
            TopIndex += row_count;
            if (TopIndex > (WindowPtr->TextStringSetCount - row_count))
              TopIndex = (WindowPtr->TextStringSetCount - row_count);
            ItemIndex = TopIndex + (row_count - 1);
            if (ItemIndex >= ItemCount)
              ItemIndex = ItemCount;
            }
          movement = TRUE;
          }
      break;

      case KEY_HOME:
        if (TopIndex != 0)
          {
          TopIndex = 0;
          OffsetIndex = 0;
          ItemIndex = row_count - 1;
          if (ItemIndex >= ItemCount)
            ItemIndex = ItemCount;
          movement = TRUE;
          }
      break;
      case KEY_END:
        if (movement_permitted)
          {
          if (ItemRowCount != NULL)
            {
            // need to get o the start of item closest to the page limit
            // without skipping over any lines
            Count = 0;
            while (ItemIndex < (ItemCount-1))
              {
              Count += ItemRowCount[ItemIndex];
              ItemIndex++;
              }
            OffsetIndex += Count;
            while ((OffsetIndex + ItemRowCount[ItemIndex])> row_count)
              {
              TopIndex++;
              OffsetIndex--;
              }
            }
          else
            {
            TopIndex = (WindowPtr->TextStringSetCount - row_count);
            ItemIndex = TopIndex + (row_count - 1);
            if (ItemIndex >= ItemCount)
              ItemIndex = ItemCount;
            }
          movement = TRUE;
          }
      break;

      default:
        if (CharAction != NULL)
          {
          switch ((*CharAction)(WindowPtr, row_offset, row_count,
            attribute_set, ItemRowCount, ItemCount, ItemIndex,
            Pointer, key))
            {
            case KEY_EXCEPTION:
              done = TRUE;
            break;
            case 1:
              movement = TRUE;
            break;
            case 0:
            break;
            }
          }
        else                         // compatibility with earlier action
          if ((key == KEY_ESCAPE) || (key == KEY_ENTER))
          done = TRUE;
      }
    }
  while (! done);

  active_locus = PreviousLocus;
}

// -----------------------------------------------------------------------
void manage_dynamic_window(WINDOW * WindowPtr, UCHAR row_offset,
                           int delta_row_count)
{
   manage_indexed_dynamic_window(WindowPtr, row_offset,
      WindowPtr->SizeInRows + delta_row_count,
      NULL, NULL, 0, FALSE, NULL);
}

// -----------------------------------------------------------------------
PRIVATE void display_choice(WINDOW * WindowPtr, OPTION * options,
                             int option_index, UCHAR color_index)
{
   text_into_window(WindowPtr, options[option_index].text,
   WindowPtr->SizeInRows, options[option_index].column_offset,
   color_index);
}

// -----------------------------------------------------------------------
PRIVATE int offer_choices_in_window(WINDOW * WindowPtr,
                                     char ** choice_strings,
                                     int default_choice)
{
   int            i;
   int            choice_count = 0;
   int            current_choice;
   BOOLEAN        done = FALSE;
   UCHAR  key;
   UCHAR  offset = 0;
   UCHAR  start_offset;
   OPTION         options[MAX_CHOICES];
   int            PreviousLocus;

   erase_mouse_cursor();
   while ((choice_count < MAX_CHOICES)
      && (choice_strings[choice_count] != NULL))
   {
      options[choice_count].text = choice_strings[choice_count];
      options[choice_count].text_length = (UCHAR)
      strlen(options[choice_count].text);

      options[choice_count].column_offset = (offset + (UCHAR)1);
      offset += (options[choice_count].text_length + (UCHAR)2);
      choice_count++;
   }
   if (offset < WindowPtr->SizeInColumns)
      start_offset = ((WindowPtr->SizeInColumns - offset) / (UCHAR)2);
   else
      start_offset = 0;

   PopupWindow = WindowPtr;
   PopupChoiceCount = choice_count;
   if (PopupChoiceCount)
   {
      PopupChoiceRow = WindowPtr->Row + WindowPtr->SizeInRows - (UCHAR) 1;
      PopupChoiceOptions = options;
   }

   for (i=0; i<choice_count; i++)
   {
      options[i].column_offset += start_offset;
      display_choice(WindowPtr, options, i, REVERSE_COLOR);
   }

   if ((default_choice < 0) || (default_choice > (choice_count - 1)))
      default_choice = 0;
   current_choice = default_choice;

   replace_mouse_cursor();
   PreviousLocus = active_locus;
   active_locus = LOCUS_POPUP;

   do
   {
      display_choice(WindowPtr, options, current_choice, HIGHLIGHT_COLOR);

      NoAutoPlay = TRUE;

      key = get_FORM_key_input();

      NoAutoPlay = FALSE;

      if (key & KEYS_HIGH_BIT)
      {
         switch (key)
         {
            case KEY_RIGHT:
            case KEY_RIGHT_FAR:
            case KEY_TAB:
               if (current_choice < (choice_count - 1))
               {
                  display_choice(WindowPtr, options, current_choice,
                  REVERSE_COLOR);
                  current_choice++;
               }
               break;

            case KEY_LEFT:
            case KEY_LEFT_FAR:
            case KEY_BACK_TAB:
               if (current_choice > 0)
               {
                  display_choice(WindowPtr, options, current_choice,
                  REVERSE_COLOR);
                  current_choice--;
               }
               break;

            case KEY_ENTER:
               done = TRUE;
               break;

            case KEY_ESCAPE:
               // escape will return a -1

               current_choice = -1;    // bail out
               done = TRUE;
               break;
         }
      }
      else
      {
         key = (char)toupper(key);

         for (i=0; i<choice_count; i++)
         {
            if (key == (UCHAR)toupper(options[i].text[0]))
            {
               current_choice = i;
               done = TRUE;
               break;
            }
         }
      }
   }
   while (! done);

   PopupChoiceOptions = NULL;
   PopupChoiceCount = 0;
   active_locus = PreviousLocus;

   return(current_choice);
}

// -----------------------------------------------------------------------
void message_pause(void)
{
   UCHAR key;

   do
   {
      NoAutoPlay = TRUE;
      key = get_FORM_key_input();
      NoAutoPlay = FALSE;
   }
   while ((key != KEY_ENTER) && (key != KEY_ESCAPE));
}

// -----------------------------------------------------------------------
char ** start_new_string_set(int * elements)
{
   char ** new_string_set;
   int i;

   if ((new_string_set = malloc(sizeof(char *) *STRING_SET_GRANULES)) != NULL)
   {
      *elements = STRING_SET_GRANULES;
      for(i = 0; i < * elements; i ++)
         new_string_set[ i ] = NULL;
   }
   else
      *elements = 0;

   return(new_string_set);
}

// -----------------------------------------------------------------------
PRIVATE char ** extend_string_set(char ** current_set, int * elements)
{
   char ** extended_string_set;

   if ((extended_string_set =
     malloc(sizeof(char *) * (*elements + STRING_SET_GRANULES))) != NULL)
   {
      int i;

      memcpy(extended_string_set, current_set, (*elements * sizeof(char *)));
      free(current_set);

      for(i = * elements; i < * elements + STRING_SET_GRANULES; i ++)
         extended_string_set[ i ] = NULL;

      *elements += STRING_SET_GRANULES;
      return(extended_string_set);
   }
   else
      return(current_set);
}

// -----------------------------------------------------------------------
char ** add_string_to_string_set(char ** current_set, char * string,
int * index, int * elements)
{
   int      string_length = (strlen(string) + 1);
   char **  working_set = current_set;
   char *   new_string;

   if((* index) >= ((* elements) - 1)) // leave a last NULL pointer
   {
      int size_before = *elements;

      working_set = extend_string_set(current_set, elements);
      if (*elements == size_before)
         return(current_set);
   }
   if ((new_string = malloc(sizeof(char) * string_length)) != NULL)
   {
      strcpy(new_string, string);
      working_set[ (*index)++ ] = new_string;
   }
   return(working_set);
}

// -----------------------------------------------------------------------
void release_string_set(WINDOW * windowPtr)
{
   char ** current_set = windowPtr->TextStringSet;
   int i;

   for (i = 0; i < windowPtr->TextStringSetCount; i ++)
      free(current_set[ i ]);

   free(current_set);
}

// -----------------------------------------------------------------------
PRIVATE void title_into_window(WINDOW * WindowPtr, PCHAR title_text,
                                UCHAR color_index)
{
   UCHAR          column_offset;
   char           buffer[85];
   int            line_width;

   line_width = (int) (WindowPtr->SizeInColumns - 1);

   // center title in window
   column_offset = (UCHAR) ((line_width - strlen(title_text)) / 2);
   sprintf(buffer, "%s", title_text);

   text_into_window(WindowPtr, buffer, 0, column_offset,
      color_index);
}

// -----------------------------------------------------------------------
PRIVATE BOOLEAN put_up_title_message_window(PCHAR title_text,
                                            PCHAR * message_text,
                                            PUSHORT attribute_set,
                                            UCHAR MaxRows,
                                            UCHAR color_set_index,
                                            BOOLEAN Dynamic,
                                            WINDOW * *MessageWindow)
{
   *MessageWindow = define_transient_window(0, 0, MaxRows, screen_columns, 0);

   if (*MessageWindow != NULL)
   {
      attach_strings_to_window(*MessageWindow, message_text);
      autosize_window(*MessageWindow);
      autocenter_window(*MessageWindow);

      open_window(*MessageWindow, color_set_index);

      if (title_text != NULL)
         title_into_window(*MessageWindow, title_text, HIGHLIGHT_COLOR);

      if (! Dynamic)    // will redraw in manage_dynamic_indexed_window
         string_set_into_window(*MessageWindow, 1,
            0, (*MessageWindow)->SizeInRows - 2, attribute_set);

      return TRUE;
   }
   return FALSE;
}

// -----------------------------------------------------------------------
//   Split off from message_window
// -----------------------------------------------------------------------
BOOLEAN put_up_message_window(char ** message, UCHAR colorset, WINDOW **Window)
{
   return put_up_title_message_window(NULL, message, NULL,
                                      screen_rows, colorset, FALSE, Window);
}

BOOLEAN ShowBusyMsg(enum BusyAbortFlags AbFlag)
{
  char ** WorkingMsg;

  if (!BusyMessageWindow)  /* If Busy Window is not open */
    {
    switch (AbFlag)
      {
      default:
      case BAF_NONE:
        WorkingMsg = BusyWorking;
        break;
      case BAF_ESC:
        WorkingMsg = BusyWorkingEsc;
        break;
      case BAF_BRK:
        WorkingMsg = BusyWorkingBreak;
        break;
      }
    return put_up_message_window(WorkingMsg, COLORS_MESSAGE, &BusyMessageWindow);
    }
  else
    return(TRUE); /* Busy Window already open */
}


// Erase the screen area of a message window if there is one and return
// a NULL pointer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
WINDOW * release_message_window(WINDOW * MessageWindow)
{
   if(MessageWindow)
   {
      CRECT OldClipRect;
      CRECT ScreenArea;

      ScreenArea.ll.x = 0;
      ScreenArea.ll.y = 0;
      ScreenArea.ur.x = screen.LastVDCXY.x;
      ScreenArea.ur.y = screen.LastVDCXY.y;

      CInqClipRectangle(screen_handle, &OldClipRect);
      CSetClipRectangle(screen_handle, ScreenArea);

      close_window(MessageWindow);

      destroy_transient_window(MessageWindow);

      CSetClipRectangle(screen_handle, OldClipRect);
   }
   return 0;
}

BOOLEAN EndBusyMsg(void)
  {
  if (BusyMessageWindow)                
    BusyMessageWindow = release_message_window(BusyMessageWindow);
  }

// -----------------------------------------------------------------------
BOOLEAN message_window(char ** message_text, UCHAR color_set_index)
{
   return title_message_window(NULL, message_text, NULL, NULL, 0,
      screen_rows, color_set_index, FALSE, NULL);
}

// -----------------------------------------------------------------------
int choice_window(char ** message_text, char ** choice_text,
   int default_choice, UCHAR color_set_index)
{
   WINDOW * ChoiceWindow;
   int      choice = default_choice;
   CRECT OldClipRect;
   CRECT ScreenArea;

   ScreenArea.ll.x = 0;
   ScreenArea.ll.y = 0;
   ScreenArea.ur.x = screen.LastVDCXY.x;
   ScreenArea.ur.y = screen.LastVDCXY.y;

   CInqClipRectangle(screen_handle, &OldClipRect);
   CSetClipRectangle(screen_handle, ScreenArea);

   ChoiceWindow =
   define_transient_window(0, 0, screen_rows, screen_columns, 0);

   if (ChoiceWindow != NULL)
   {
      attach_strings_to_window(ChoiceWindow, message_text);
      autosize_window(ChoiceWindow);
      autocenter_window(ChoiceWindow);

      open_window(ChoiceWindow, color_set_index);

      string_set_into_window(ChoiceWindow, 1,
      0, ChoiceWindow->TextStringSetCount, NULL);

      choice =
      offer_choices_in_window(ChoiceWindow, choice_text, default_choice);

      close_window(ChoiceWindow);

      destroy_transient_window(ChoiceWindow);
   }
   CSetClipRectangle(screen_handle, OldClipRect);
   return(choice);
}

// -----------------------------------------------------------------------
int yes_no_choice_window(char ** message_text, int default_choice,
   UCHAR color_set_index)
{
   return(choice_window(message_text, yes_no_choice_text, default_choice,
   color_set_index));
}


// -----------------------------------------------------------------------
BOOLEAN title_message_window(PCHAR title_text, PCHAR *message_text,
                             PUSHORT attribute_set,
                             PUCHAR ItemRowCount,
                             USHORT ItemCount,
                             UCHAR MaxRows,
                             UCHAR color_set_index,
                             BOOLEAN Pointer,
                             SHORT (*CharAction)(WINDOW * WindowPtr,
                                                 UCHAR RowOffset,
                                                 SHORT RowCount,
                                                 PUSHORT attribute_set,
                                                 PUCHAR ItemRowCount,
                                                 USHORT ItemCount,
                                                 USHORT ItemIndex,
                                                 BOOLEAN Pointer,
                                                 UCHAR Key))
{
  WINDOW *    MessageWindow;
  CRECT OldClipRect;
  BOOLEAN ReturnVal = FALSE;
  CRECT ScreenArea;

  ScreenArea.ll.x = 0;
  ScreenArea.ll.y = 0;
  ScreenArea.ur.x = screen.LastVDCXY.x;
  ScreenArea.ur.y = screen.LastVDCXY.y;

  CInqClipRectangle(screen_handle, &OldClipRect);
  CSetClipRectangle(screen_handle, ScreenArea);

  if (put_up_title_message_window(title_text, message_text,
     attribute_set, MaxRows, color_set_index, TRUE, &MessageWindow))
    {
    manage_indexed_dynamic_window(MessageWindow, 1,
                   (int) (MessageWindow->SizeInRows - 2),
                   attribute_set, ItemRowCount, ItemCount, Pointer,
                   CharAction);

    MessageWindow = release_message_window(MessageWindow);
    ReturnVal = TRUE;
    CSetClipRectangle(screen_handle, OldClipRect);
    }
  else
    CSetClipRectangle(screen_handle, OldClipRect);

  return ReturnVal;
}

// moved here from mousefrm.c  RAC 6/23/91
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UCHAR popup_left_button_handler(UCHAR row,
                                        UCHAR column)
{
   SHORT i;
   UCHAR ReturnKey = KEY_EXCEPTION;

   if ((PopupChoiceOptions != NULL) && PopupChoiceCount &&
        (row == PopupChoiceRow))
   {
      column -= PopupWindow->Column;
      for (i=0; i<PopupChoiceCount; i++)
      {
         if ((column >= PopupChoiceOptions[i].column_offset) &&
              (column < PopupChoiceOptions[i].column_offset +
                        PopupChoiceOptions[i].text_length))
            ReturnKey = PopupChoiceOptions[i].text[0];
      }
   }
   else
   {
      if (PopupChoiceOptions == NULL)// return an enter if no choice available
      {                              // and mouse was clicked in popup window
         if ((row >= PopupWindow->Row) &&
            (row < PopupWindow->Row + PopupWindow->SizeInRows))
            ReturnKey = KEY_ENTER;
      }
   }
   return ReturnKey;
}

// allocate a window for PopupWindow. return TRUE if alloc OK.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN popupWindowBegin(void)
{
   PopupWindow = malloc(sizeof(WINDOW));

   if(! PopupWindow)
      return FALSE;
   
   return TRUE;
}

// initialize PopupWindow
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void popupWindowSetup(UCHAR row,     UCHAR column,
                       UCHAR numRows, UCHAR numColumns)
{
   PopupWindow->Row = row;
   PopupWindow->Column = column;
   PopupWindow->SizeInRows = numRows;
   PopupWindow->SizeInColumns = numColumns;
}

// deallocate PopupWindow
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void popupWindowEnd(void)
{
   free(PopupWindow);
   PopupWindow = NULL;
}

