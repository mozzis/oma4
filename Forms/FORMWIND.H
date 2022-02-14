// FILE : formwind.h

#ifndef FORMWIND_INCLUDED
#define FORMWIND_INCLUDED

#include "eggtype.h"

typedef struct wind WINDOW;

extern char * BusyWorking[];
extern char * BusyWorkingEsc[];
extern char * BusyWorkingBreak[];

enum BusyAbortFlags { BAF_NONE, BAF_ESC, BAF_BRK };

extern char * BlkOverWritePrompt[];
extern char * DataFileOverwritePrompt[];

typedef SHORT (CharActFunct)(WINDOW * WindowPtr, 
                             UCHAR RowOffset, SHORT RowCount,
                             PUSHORT attribute_set, 
                             PUCHAR ItemRowCount, 
                             USHORT ItemCount,
                             USHORT ItemIndex, 
                             BOOLEAN Pointer, 
                             UCHAR Key);

WINDOW * define_transient_window(UCHAR Row, UCHAR Column, 
                                 UCHAR SizeInRows, UCHAR SizeInColumns, 
                                 USHORT Attrib);
void destroy_transient_window(WINDOW * WindowPtr);
void attach_strings_to_window(WINDOW * WindowPtr, char ** StringSet);
void autosize_window(WINDOW * WindowPtr);
void autocenter_window(WINDOW * WindowPtr);
void open_window(WINDOW * WindowPtr, UCHAR color_set_index);
void close_window(WINDOW * WindowPtr);
void manage_dynamic_window(WINDOW * WindowPtr, UCHAR row_offset,
   int delta_row_count);
void message_pause(void);
char ** start_new_string_set(int * elements);
char ** add_string_to_string_set(char ** current_set, char * string,
   int * index, int * elements);
void release_string_set(WINDOW * windowPtr);
BOOLEAN put_up_message_window(char ** message_text,
   UCHAR color_set_index, WINDOW ** MessageWindow);
WINDOW * release_message_window(WINDOW * MessageWindow);
BOOLEAN message_window(char ** message_text,
   UCHAR color_set_index);
int choice_window(char ** message_text, char ** choice_text,
   int default_choice, UCHAR color_set_index);
int yes_no_choice_window(char ** message_text, int default_choice,
   UCHAR color_set_index);

BOOLEAN title_message_window(PCHAR title_text, PCHAR *message_text,
   PUSHORT attribute_set, PUCHAR ItemRowCount, USHORT ItemCount,
   UCHAR MaxRows, UCHAR color_set_index, BOOLEAN Pointer,
   CharActFunct *CharAction);

BOOLEAN ShowBusyMsg(enum BusyAbortFlags AbFlag);
BOOLEAN EndBusyMsg(void);

// check popup options for mousefrm module
UCHAR popup_left_button_handler(UCHAR row,
                                        UCHAR column);

// functions to manage the popup window PopupWindow

BOOLEAN popupWindowBegin(void);
void popupWindowSetup(UCHAR row, UCHAR column,
                       UCHAR numRows, UCHAR numColumns);
void popupWindowEnd(void);

#endif  //  FORMWIND_INCLUDED
