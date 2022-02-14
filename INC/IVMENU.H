/* -----------------------------------------------------------------------
/
/  menu.h
/
/ Copyright (c) 1988,  EG&G Princeton Applied Research
/
/ Written by: TLB  Version 1.00    1-8 March  1988
/ Worked on:  TLB  Version 1.01    25 April   1988
/             TLB  Version 1.02    30 June    1988
/             TLB  Version 1.03    7 November 1988
/
/   menu.h defines constants and data structures for menus.
/
/ ----------------------------------------------------------------------- */

#define   MAX_ITEMS      25
#define   MAX_MENU_DEPTH 20

/*
Bit definitions for the MENUITEM.control field fit into the high byte;
the low byte is for an optional menuitem index.
This bits are OR'd together to define what actions should be taken 
when a menuitem is chosen, and select various other properties.
*/
#define SUBMENU       0x100    /* enter submenu if item is chosen */
#define DOFUNCTION    0x200    /* execute function if item is chosen */
#define GETFROM       0x400    /* get input from item named in low byte */
#define NOTCHOICE     0x800    /* this is not a selectable item   */
#define INPUT_SCL_FLT 0x1000   /* this item is a scaled flt. pt. input field */
#define INPUT_INT     0x2000   /* this item is a integer input field */
#define INPUT_FLT     0x4000   /* this item is floating pt. input field */
#define INPUT_STR     0x8000   /* this item is a string input field */

#define INPUT_MASK    0xF000   /* to mask out all but "input type" bits */

/* Bit definitions for the MENU.control field */

#define CHOICE_MADE   0x01     /* set at run time to indicate item chosen */
#define SINGLE_CHOICE 0x02     /* menu exits as soon as an item is chosen */
#define MULTI_CHOICE  0x04     /* menu allows several items to be chosen */
#define RETURN_INDEX  0x08     /* return index of submenu menuitem */
#define RADIO_BUTTON  0x10     /* used by CHOICEBOXes */

#define NEXT_POSITION   -1

#define NORMAL         0
#define HIGHLIGHT      1
#define CHOICE         2

#define STAY           0
#define UP             1
#define DOWN           2

#define KPM_OFF        0       /* Keystroke Programming Mode */
#define KPM_LEARN      1
#define KPM_PLAYBACK   2

#define MAX_DB_CHOICES 16


#define MEGA_EXPONENT      6
#define KILO_EXPONENT      3
#define MILLI_EXPONENT     -3
#define MICRO_EXPONENT     -6
#define NANO_EXPONENT      -9
#define PICO_EXPONENT      -12

#define FACTOR_FROM_MEGA   (float)1.0E+6
#define FACTOR_FROM_KILO   (float)1.0E+3
#define FACTOR_FROM_MILLI  (float)1.0E-3
#define FACTOR_FROM_MICRO  (float)1.0E-6
#define FACTOR_FROM_NANO   (float)1.0E-9
#define FACTOR_FROM_PICO   (float)1.0E-12

#define FACTOR_TO_MEGA     (float)1.0E-6
#define FACTOR_TO_KILO     (float)1.0E-3
#define FACTOR_TO_MILLI    (float)1.0E+3
#define FACTOR_TO_MICRO    (float)1.0E+6
#define FACTOR_TO_NANO     (float)1.0E+9
#define FACTOR_TO_PICO     (float)1.0E+12

/* structure to define a Display Box */

typedef struct DUMPDISP
{
   char      row;
   char      column;
   char      size_in_rows;
   char      size_in_columns;
   char      text_color;
   char      background_color;
   char      choice_color;
   int       last_x;
   int       last_y;
   int       bitmap_index;
   DESCARTES diagonal[2];

} DISPBOX ;


/* structure to define a Menu Item */

typedef struct DUMBITEM
{
   char *           text;
   char             row;
   char             column;
   int              control;
   struct menudef * sub_menu;      /* MENU * sub_menu */
   char *           string_ptr;
   void *           data_ptr;
   void             (*entry_action)(void);
   void             (*exit_action)(void);
} MENUITEM;

/* structure to define a Menu */

struct menudef
{
   char *    header_text;
   char      row;
   char      column;
   char      text_color;
   char      background_color;
   char      highlight_color;
   char      total_items;
   char      default_item;
   int *     item_limit_variable;
   char      control;
   DISPBOX * box;
   MENUITEM  item[MAX_ITEMS];
};
typedef struct menudef MENU;

/* structure to define a menu history entry */
struct backmenu
{
   MENU *    previous;
   char      previous_menuitem;
};



/* structure to define a choice from a choice box */
typedef struct _CHOICE
{
   int     row;
   int     column;
   char    choose_char;
   char *  text;
   int     textlen;
} ;

/* structure to define a choice box */

typedef struct CHOICEBOX
{
   int       control;
   int       selected_bits;
   int       textline_count;
   char **   textlines;
   int       choice_count;
   struct _CHOICE * choices;
   DISPBOX   box;
} ;

extern  void str_to_display_field(char *display_str,char *in_str);
extern  void display_field_to_str(char *out_str,char *display_str);
extern  float scale_str_to_value(char *scale_str);
extern  void value_to_scale_str(double value,char *scale_str,char *units_str);
extern  void initialize_input_field(int item_index);
extern  int row_to_y(int row);
extern  int column_to_x(int column);
extern  int y_to_row(int coord);
extern  int x_to_column(int coord);
/* int save_screen_area(struct DESCARTES *box_diagonal); */
/* void restore_screen_area(int index,struct DESCARTES *box_diagonal); */
extern  void select_DB_default_colors(struct DUMPDISP *box);
extern  void draw_DB(struct DUMPDISP *box);
extern  struct DUMPDISP *create_transient_DB(int row,int column,int size_rows,int size_columns);
extern  void erase_DB_interior(struct DUMPDISP *box);
extern  void destroy_DB(struct DUMPDISP *box);
extern  void destroy_transient_DB(struct DUMPDISP *box);
extern  void text_into_DB(struct DUMPDISP *box,int row,int column,char *textptr,int highlight_flag);
extern  void draw_menuitem(int use_item);
extern  void highlight_menuitem(int use_item);
extern  void draw_menu(int column_offset,int row_offset);
extern  int legal_char(char key);
extern  int end_of_text(char *string,int total_len);
extern  void get_data_from_input_field(int use_item);
extern  int read_keyboard(int * direction, char * key);
extern  int edit_menuitem(void );
extern  void previous_menuitem(void );
extern  void next_menuitem(void );
extern  void previous_menu(void );
extern  void exit_to_previous_menu(void );
extern  void return_index_to_calling_item(void );
extern  void back_to_main_menu(void );
extern  void make_menus_invisible(void );
extern  void make_menus_visible(void );
extern  void F3_menu_action(void );
extern  void enter_submenu(void );
extern  int get_from_input_field(void );
extern  void choose_menuitem(void );
extern  void reselect_option(void );
extern  void DoExit(void );
extern  void SetItemIndex(void );
extern  void refresh_menu_input_fields(void );
extern  int jump_to_menuitem(int item);
extern  void execute_function_key_action(int function_index);
extern  void set_DISPBOX_colors_to_BW(void );
extern  void set_MENU_colors_to_BW(struct menudef *target_menu);
extern  void init_menu(struct menudef *base_menu);
extern  void execute_menu(struct menudef *base_menu,int GSS_handle,struct work_out *GSS_dev_parameters);
extern  int pause(void );
extern  int simple_message_box(int row,int column,char *textptr);
extern  int string_input_field(struct DUMPDISP *box,char *field_string,char *string,int row,int column);
extern  void draw_choice_item(struct CHOICEBOX *chbox, struct _CHOICE *item,int highlight_flag);
extern  void display_all_items(struct CHOICEBOX *chbox,int select);
extern  void display_choice_box(struct CHOICEBOX * chbox,int select);
extern  int run_choice_box(struct CHOICEBOX *chbox,int legal_option_bits);
extern  void destroy_choice_box(struct CHOICEBOX *chbox);
