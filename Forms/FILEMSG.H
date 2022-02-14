// FILE filemsg.h    RAC   1-FEB-90
// Copyright (c) 1989,  EG&G Princeton Applied Research
  
// functions to read error/help messages from a file and put them on
// screen, also to read a color file
  
#ifndef FILEMSG_INCLUDED  // idempotent header file
#define FILEMSG_INCLUDED
  
#include <stdarg.h>    // va_list

#include "formtype.h"  // COLOR_SET
#include "formwind.h"  // WINDOW
  
// put up a message window on screen with text taken from a file.
BOOLEAN get_message_from_file(WINDOW *WindowPtr, const char * file_spec,
                              int message_index, va_list insert_args);
  
// put up a message window on screen with text taken from a file.
BOOLEAN va_file_message_window(const char * file_spec, int message_index,
                               unsigned char max_rows, 
                               unsigned char color_set_index, 
                               va_list insert_args);
  
// put up a message window on screen with text taken from a file.
BOOLEAN file_message_window(const char * file_spec, int message_index,
unsigned char max_rows, unsigned char color_set_index, ... );
  
// read color settings from a file into color_sets[]
BOOLEAN read_color_file(const char * file_spec, int max_color_sets,
                        COLOR_SET *color_sets);
  
#endif // FILEMSG_INCLUDED
