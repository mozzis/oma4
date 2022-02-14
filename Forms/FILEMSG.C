/* -----------------------------------------------------------------------
/
/  filemsg.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00            18 October     1989
/  Worked on:  TLB      Version 1.01
/
*/ /*
  $Header:   J:/logfiles/forms/filemsg.c_v   1.4   10 Jan 1992 13:25:46   cole  $
  $Log:   J:/logfiles/forms/filemsg.c_v  $
 * 
 *    Rev 1.4   10 Jan 1992 13:25:46   cole
 * Add include forms.h, delete include oma35.h
 * 
 *    Rev 1.3   24 Jun 1991 10:52:36   cole
 * removed all references to fields in struct WINDOW (modified some functions
 * in formwind.c). cosmetics.  struct WINDOW is now an opaque type.
 * 
 *    Rev 1.2   12 Oct 1990 13:34:40   irving
 * mouse support for message windows
 * 
 *    Rev 1.1   05 Oct 1990 11:19:18   irving
 * changes for new color table
 * 
 *    Rev 1.0   27 Sep 1990 15:39:54   admax
 * Initial revision.
*/ /*
/ ----------------------------------------------------------------------- */
  
// modified 1-FEB-90, RAC : added filemsg.h, moved read_color_file() from
//   OMA2000.C
  
#include <stdio.h>      // fopen()
#include <stdlib.h>     // atoi()
#include <string.h>     // strcpy()
  
#include "eggtype.h"
#include "filemsg.h"
#include "formwind.h"   // WINDOW
#include "forms.h"
  
BOOLEAN get_message_from_file(WINDOW * WindowPtr, const char * file_spec,
int message_index, va_list insert_args)
{
   FILE *   fileptr;
   BOOLEAN  message_found = FALSE;
   char **  message_string_array;
   int      string_array_elements;
   int      string_count = 0;
   int      string_length;
  
   message_string_array = start_new_string_set(&string_array_elements);
   if (string_array_elements == 0)
   {
      attach_strings_to_window(WindowPtr, NULL);
      return(FALSE);
   }
  
   if((fileptr = fopen(file_spec, "r")) != NULL)
   {
      char   file_line[128];
      char * marker;

      while (fgets(file_line, 128, fileptr) != NULL)
      {
         if ((marker = strstr(file_line, "@@@")) != NULL)
         {
            if (message_found)
               break;
            else
            {
               if (atoi(&marker[3]) == message_index)
                  message_found = TRUE;
            }
         }
         else if (message_found)
         {
            string_length = strlen(file_line);
            if (file_line[ (string_length - 1) ] == '\n')
               file_line[ (string_length - 1) ] = (char) 0; /* zap \n at end */
  
            if (strchr(file_line, '%') != NULL)
            {
               char format_string[128];
  
               vsprintf(format_string, file_line, insert_args);
  
               message_string_array =
               add_string_to_string_set(message_string_array,
               format_string, &string_count, &string_array_elements);
            }
            else
            {
               message_string_array =
               add_string_to_string_set(message_string_array,
               file_line, &string_count, &string_array_elements);
            }
         }
      }
      fclose(fileptr);
   }
  
   if (message_found && (string_count > 0))
      attach_strings_to_window(WindowPtr, message_string_array);
   else
      attach_strings_to_window(WindowPtr, NULL);
  
   return message_found;
}
  
BOOLEAN va_file_message_window(const char * file_spec, int message_index,
 UCHAR max_rows, UCHAR color_set_index, va_list insert_args)
{
   WINDOW * FileWindow;
   BOOLEAN  file_success = FALSE;

   erase_mouse_cursor();

   if ((max_rows < 4) || (max_rows > screen_rows))
      max_rows = screen_rows;
  
   FileWindow =
   define_transient_window(0, 0, max_rows, screen_columns, (char)0);
  
   if (FileWindow != NULL)
   {
      if (message_index == 0)
      {
         char *   problem_message[2];
         char *   text = "No Message Available";
  
         problem_message[0] = text;
         problem_message[1] = NULL;
  
         attach_strings_to_window(FileWindow, problem_message);
      }
      else if (get_message_from_file(FileWindow, file_spec, message_index,
         insert_args))
      {
         file_success = TRUE;
      }
      else
      {
         char *   problem_message[2];
         char     text[40];
  
         sprintf(text, "Message Number %d", message_index);
         problem_message[0] = text;
         problem_message[1] = NULL;
  
         attach_strings_to_window(FileWindow, problem_message);
      }
  
      autosize_window(FileWindow);
      autocenter_window(FileWindow);
  
      open_window(FileWindow, color_set_index);
  
      replace_mouse_cursor();

      manage_dynamic_window(FileWindow, 1, -2);
  
      close_window(FileWindow);
  
      if (file_success)
         release_string_set(FileWindow);
  
      destroy_transient_window(FileWindow);
      return(TRUE);
   }
   else
      return(FALSE);
}
  
BOOLEAN file_message_window(const char * file_spec, int message_index,
  UCHAR max_rows, UCHAR color_set_index, ...)
{
   va_list  insert_args;
  
   va_start(insert_args, color_set_index);
  
   return(va_file_message_window(file_spec, message_index, max_rows,
   color_set_index, insert_args));
}
  
BOOLEAN read_color_file(const char * file_spec, int max_color_sets,
                        COLOR_SET *color_sets)
{
   BOOLEAN              status = FALSE;
   FILE *               bin_file;
   COLOR_SET_RECORD     color_record;
   int                  i;
   int                  number_of_color_sets;
  
   if ((bin_file = fopen(file_spec, "rb")) != NULL)
   {
      fread(&number_of_color_sets, sizeof(int), 1, bin_file);
  
      if ((number_of_color_sets > 0)
         && (number_of_color_sets <= max_color_sets))
      {
         for (i=0; i<number_of_color_sets; i++)
         {
            fread(&color_record, sizeof(COLOR_SET_RECORD), 1, bin_file);
            memcpy(&color_sets[i], &color_record.set, sizeof(COLOR_SET));
#ifdef GSS_COLOR
            /* remap DOS color indices to GSS colors */ // 10/4/90 DAI
            if (color_sets[i].regular.foreground == BLUE)
               color_sets[i].regular.foreground = BRT_WHITE;
            else if (color_sets[i].regular.foreground == BRT_WHITE)
               color_sets[i].regular.foreground = BLUE;

            if (color_sets[i].regular.background == BLUE)
               color_sets[i].regular.background = BRT_WHITE;
            else if (color_sets[i].regular.background == BRT_WHITE)
               color_sets[i].regular.background = BLUE;

            if (color_sets[i].reverse.foreground == BLUE)
               color_sets[i].reverse.foreground = BRT_WHITE;
            else if (color_sets[i].reverse.foreground == BRT_WHITE)
               color_sets[i].reverse.foreground = BLUE;

            if (color_sets[i].reverse.background == BLUE)
               color_sets[i].reverse.background = BRT_WHITE;
            else if (color_sets[i].reverse.background == BRT_WHITE)
               color_sets[i].reverse.background = BLUE;

            if (color_sets[i].highlight.foreground == BLUE)
               color_sets[i].highlight.foreground = BRT_WHITE;
            else if (color_sets[i].highlight.foreground == BRT_WHITE)
               color_sets[i].highlight.foreground = BLUE;

            if (color_sets[i].highlight.background == BLUE)
               color_sets[i].highlight.background = BRT_WHITE;
            else if (color_sets[i].highlight.background == BRT_WHITE)
               color_sets[i].highlight.background = BLUE;

            if (color_sets[i].shaded.foreground == BLUE)
               color_sets[i].shaded.foreground = BRT_WHITE;
            else if (color_sets[i].shaded.foreground == BRT_WHITE)
               color_sets[i].shaded.foreground = BLUE;

            if (color_sets[i].shaded.background == BLUE)
               color_sets[i].shaded.background = BRT_WHITE;
            else if (color_sets[i].shaded.background == BRT_WHITE)
               color_sets[i].shaded.background = BLUE;

            if (color_sets[i].error.foreground == BLUE)
               color_sets[i].error.foreground = BRT_WHITE;
            else if (color_sets[i].error.foreground == BRT_WHITE)
               color_sets[i].error.foreground = BLUE;

            if (color_sets[i].error.background == BLUE)
               color_sets[i].error.background = BRT_WHITE;
            else if (color_sets[i].error.background == BRT_WHITE)
               color_sets[i].error.background = BLUE;
#endif
         }
  
         status = (! ferror(bin_file));
      }
      fclose(bin_file);
   }
  
   return(status);
}
