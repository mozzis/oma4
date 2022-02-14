/* $Header: /OMA4000/Forms/FORMS.H 1     6/18/99 1:53a Maynard $ */
/* -----------------------------------------------------------------------
/
/  forms.h
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00        1-8 May         1988
/
*/ /*
/  $Log: /OMA4000/Forms/FORMS.H $
 * 
 * 1     6/18/99 1:53a Maynard
*/ /*
/ ----------------------------------------------------------------------- */

#ifndef FORMS_INCLUDED    // idempotent header file
#define FORMS_INCLUDED

#ifndef CGIBIND_INCLUDED
   #define CGIBIND_INCLUDED
   #include <cgibind.h>
#endif

#ifndef EGGTYPE_INCLUDED
  #include "eggtype.h"
#endif

#ifndef FORMTYPE_INCLUDED
   #include "formtype.h"
#endif

#define GSS_COLOR

#define char_fill(addr, count, ch) memset(addr, ch, count)

/* new name for this - ANSI C will have offsetof(), so if warning */
/* message appears, delete this macro */
//#define offsetof(type, field) ((SHORT)&(((type *)0)->field))

/* same as above - older versions used this name - delete later */
#define STRUCTOFFSET(type, field) ((SHORT)&(((type *)0)->field))

#define set_bit(bit_var, bit) (bit_var |= (bit))
#define reset_bit(bit_var, bit) (bit_var &= ~(bit))

#define DEFAULT_SCREEN_ROWS      25
#define DEFAULT_SCREEN_WIDTH     80

#define MAX_FIELD_SIZE           80
#define MAX_FORM_NEST_LEVEL      10

#define ASCII_BACKSPACE    8
#define CR                 13
#define SPACE              32
#define DELETE             127
#define ESCAPE             27

typedef enum ChoiceYesNoEscape
{
   CHOICE_YES = 1, CHOICE_NO = 0, CHOICE_ESCAPE = -1
} YesNoEscape ;

#define NO                 0
#define YES                1
#define CHOICE_ESCAPE      -1

#define KEYS_HIGH_BIT      0x80

#define KEY_ENTER          128
#define KEY_ESCAPE         129
#define KEY_TAB            130
#define KEY_BACK_TAB       131
#define KEY_BACKSPACE      132
#define KEY_DELETE         133
#define KEY_DELETE_FAR     134
#define KEY_INSERT         135
#define KEY_UP             136
#define KEY_UP_FAR         137
#define KEY_DOWN           138
#define KEY_DOWN_FAR       139
#define KEY_LEFT           140
#define KEY_LEFT_FAR       141
#define KEY_RIGHT          142
#define KEY_RIGHT_FAR      143
#define KEY_HOME           144
#define KEY_HOME_FAR       145
#define KEY_END            146
#define KEY_END_FAR        147
#define KEY_PG_UP          148
#define KEY_PG_UP_FAR      149
#define KEY_PG_DN          150
#define KEY_PG_DN_FAR      151
#define KEY_PLUS           152
#define KEY_MINUS          153

#define KEY_F1             154
#define KEY_F2             155
#define KEY_F3             156
#define KEY_F4             157
#define KEY_F5             158
#define KEY_F6             159
#define KEY_F7             160
#define KEY_F8             161
#define KEY_F9             162
#define KEY_F10            163
#define KEY_F11            164
#define KEY_F12            165
#define KEY_F13            166
#define KEY_F14            167
#define KEY_F15            168
#define KEY_F16            169
#define KEY_F17            170
#define KEY_F18            171
#define KEY_F19            172
#define KEY_F20            173
#define KEY_F21            174
#define KEY_F22            175
#define KEY_F23            176
#define KEY_F24            177
#define KEY_F25            178
#define KEY_F26            179
#define KEY_F27            180
#define KEY_F28            181
#define KEY_F29            182
#define KEY_F30            183
#define KEY_F31            184
#define KEY_F32            185
#define KEY_F33            186
#define KEY_F34            187
#define KEY_F35            188
#define KEY_F36            189
#define KEY_F37            190
#define KEY_F38            191
#define KEY_F39            192
#define KEY_F40            193   /* !!! */

#define KEY_ALT_A          193   /* !!!!!!!!!!!!!!!!!!! */
#define KEY_ALT_B          194
#define KEY_ALT_C          195
#define KEY_ALT_D          196
#define KEY_ALT_E          197
#define KEY_ALT_F          198
#define KEY_ALT_G          199
#define KEY_ALT_H          200
#define KEY_ALT_I          201
#define KEY_ALT_J          202
#define KEY_ALT_K          203
#define KEY_ALT_L          204
#define KEY_ALT_M          205
#define KEY_ALT_N          206
#define KEY_ALT_O          207
#define KEY_ALT_P          208
#define KEY_ALT_Q          209
#define KEY_ALT_R          210
#define KEY_ALT_S          211
#define KEY_ALT_T          212
#define KEY_ALT_U          213
#define KEY_ALT_V          214
#define KEY_ALT_W          215
#define KEY_ALT_X          216
#define KEY_ALT_Y          217
#define KEY_ALT_Z          218

// Definitions for the Control keys. 
// Control C is a special MSDOS key.
#define KEY_CTRL_A 1
#define KEY_CTRL_B 2
#define KEY_CTRL_C 3  // special break key
#define KEY_CTRL_D 4
#define KEY_CTRL_E 5
#define KEY_CTRL_F 6
#define KEY_CTRL_G 7
#define KEY_CTRL_H 8
#define KEY_CTRL_I 9
#define KEY_CTRL_J 10
#define KEY_CTRL_K 11
#define KEY_CTRL_L 12
#define KEY_CTRL_M 13
#define KEY_CTRL_N 14
#define KEY_CTRL_O 15
#define KEY_CTRL_P 16
#define KEY_CTRL_Q 17
#define KEY_CTRL_R 18
#define KEY_CTRL_S 19
#define KEY_CTRL_T 20
#define KEY_CTRL_U 21
#define KEY_CTRL_V 22
#define KEY_CTRL_W 23
#define KEY_CTRL_X 24
#define KEY_CTRL_Y 25
#define KEY_CTRL_Z 26

#define KEY_

#define KEY_EXCEPTION      255

// -----------------------------------------------------------------------

#define FLDTYP_STRING               1
#define FLDTYP_STD_FLOAT            2
#define FLDTYP_SCL_FLOAT            3
#define FLDTYP_INT                  4
#define FLDTYP_UNS_INT              5
#define FLDTYP_HEX_INT              6
#define FLDTYP_TOGGLE               7
#define FLDTYP_LOGIC                8
#define FLDTYP_FORM                 9
#define FLDTYP_FORMAT               10
#define FLDTYP_SELECT               11
#define FLDTYP_SPIN                 12
#define FLDTYP_
#define FLDTYP_ROM_INT              15


#define FTB_STRING                  0x0001
#define FTB_STD_FLT                 0x0002
#define FTB_SCL_FLT                 0x0004
#define FTB_INT                     0x0008
#define FTB_UNS_INT                 0x0010
#define FTB_HEX_INT                 0x0020
#define FTB_TOGGLE                  0x0040
#define FTB_FORMAT                  0x0080
#define FTB_SELECT                  0x0100
#define FTB_SPIN                    0x0200
#define FTB_
#define FTB_ROM_INT                 0x8000

#define FTBS_GENERAL_NUMERIC        0x003E

#define FLDATTR_NONE                0

#define FLDATTR_RJ                  0x0001
#define FLDATTR_DISPLAY_ONLY        0x0002
#define FLDATTR_REV_VID             0x0004
#define FLDATTR_HIGHLIGHT           0x0008
#define FLDATTR_SHADED              0x0010
#define FLDATTR_LIMIT_WARN          0x0020
#define FLDATTR_GET_DRAW_PERMISSION 0x0040
#define FLDATTR_DRAW_PERMITTED      0x0080
#define FLDATTR_NO_OVERFLOW_CHAR    0x0100
#define FLDATTR_DISABLED            0x0200
#define FLDATTR_

#define DATATYP_VOID                0
#define DATATYP_STRING              1
#define DATATYP_STRING_ARRAY        2
#define DATATYP_FLOAT               3
#define DATATYP_DOUBLE_FLOAT        4
#define DATATYP_INT                 5
#define DATATYP_CHAR_INT            7
#define DATATYP_LONG_INT            8
#define DATATYP_CODE                9
#define DATATYP_FORM                10
#define DATATYP_

#define DATAATTR_NONE               0
#define DATAATTR_PTR                1
#define DATAATTR_PTR_PTR            2
#define DATAATTR_PTR_OFFS           3
#define DATAATTR_PTR_OFFS_PTR       4
#define DATAATTR_PTR_PTR_OFFS       5

#define FORMATTR_NONE               0

/* fixed form attributes */
#define FORMATTR_EXIT_ERASE       0x0001
#define FORMATTR_EXIT_RESTORE     0x0002
#define FORMATTR_SCROLLING        0x0004
#define FORMATTR_BORDER           0x0008
#define FORMATTR_INDEP            0x0010  // Nested form that can overrun
                                          // parent bounds.  10/16/90 DAI
                                          // Position will be given in
                                          // absolute row and column.
                                          // Exit will not remove parent
                                          // form.
#define FORMATTR_STICKY           0x0020  // Form won't exit if error exists
#define FORMATTR_FULLWIDTH        0x0040  // Form adjusts right edge to screen
#define FORMATTR_FULLHEIGHT       0x0080  // Form adjusts lower edge to screen
#define FORMATTR_FULLSCREEN       0x00C0  // Form adjusts size to screen

/* global changeable form attributes */
#define FORMATTR_OVERSTRIKE       0x0100
#define FORMATTR_NO_ENTER_VECTOR  0x0200
#define FORMATTR_FIRST_CHAR_ERASE 0x0400
#define FORMATTR_DEPENDENT_FORM   0x0800

/* local changeable form attributes */
#define FORMATTR_ZOOMED_FIELD     0x1000
#define FORMATTR_NESTED           0x2000
#define FORMATTR_VISIBLE          0x4000
#define FORMATTR_UNUSED_TRANSIENT 0x8000   /* not in use yet... */

#define FORMATTR_GLOBAL_BITS      0x0F00   /* subject to change... */

#define FORMSTAT_ACTIVE_FIELD     0
#define FORMSTAT_EXIT_THIS_FIELD  1
#define FORMSTAT_EXIT_THIS_FORM   2
#define FORMSTAT_EXIT_ALL_FORMS   3
#define FORMSTAT_SWITCH_MODE      4     /* switch between graph and menu */
#define FORMSTAT_EXIT_TO_MENU2    5
#define FORMSTAT_EXIT_TO_MENU1    6

#define EXIT_DEFAULT              0
#define EXIT_ENTER                1
#define EXIT_UP                   2
#define EXIT_DOWN                 3
#define EXIT_LEFT                 4
#define EXIT_RIGHT                5
#define EXIT_TAB_LEFT             6
#define EXIT_TAB_RIGHT            7
#define EXIT_NONE                 255

#define CURSOR_TYPE_NORMAL        0
#define CURSOR_TYPE_OVERSTRIKE    1
#define CURSOR_COLOR_NORMAL       BRT_WHITE

#define OVERFLOW_NONE             0
#define OVERFLOW_AT_START         1
#define OVERFLOW_IN_MIDDLE        2
#define OVERFLOW_AT_END           3

#define FIELD_VALIDATE_SUCCESS    -1
#define FIELD_VALIDATE_WARNING    -2
/*      FIELD_VALIDATE_ERROR is an offset into the field string */

#undef      BLACK
#undef      BLUE
#undef      GREEN
#undef      RED
#undef      CYAN
#undef      PURPLE
#undef      BROWN
#undef      WHITE

#ifdef GSS_COLOR
#define     BLACK           0
#define     BRT_WHITE       1
#define     GREEN           2
#define     RED             3
#define     CYAN            4
#define     PURPLE          5
#define     BROWN           6
#define     WHITE           7
#define     BRT_BLUE        8
#define     BRT_GREEN       9
#define     BRT_RED        10
#define     BRT_CYAN       11
#define     BRT_PURPLE     12
#define     BRT_YELLOW     13
#define     BLUE           14
#define     BRT_ORANGE     15
#else                            // straight DOS colors
#define     BLACK           0
#define     BLUE            1
#define     GREEN           2
#define     RED             3
#define     CYAN            4
#define     PURPLE          5
#define     BROWN           6
#define     WHITE           7
#define     BRT_BLUE        8
#define     BRT_GREEN       9
#define     BRT_RED        10
#define     BRT_CYAN       11
#define     BRT_PURPLE     12
#define     BRT_YELLOW     13
#define     BRT_WHITE      14
#define     BRT_ORANGE     15
#endif

#define GIGA_EXPONENT      9
#define MEGA_EXPONENT      6
#define KILO_EXPONENT      3
#define MILLI_EXPONENT     -3
#define MICRO_EXPONENT     -6
#define NANO_EXPONENT      -9
#define PICO_EXPONENT      -12

#define FACTOR_FROM_GIGA   1.0E+9
#define FACTOR_FROM_MEGA   1.0E+6
#define FACTOR_FROM_KILO   1.0E+3
#define FACTOR_FROM_MILLI  1.0E-3
#define FACTOR_FROM_MICRO  1.0E-6
#define FACTOR_FROM_NANO   1.0E-9
#define FACTOR_FROM_PICO   1.0E-12

#define FACTOR_TO_GIGA     1.0E-9
#define FACTOR_TO_MEGA     1.0E-6
#define FACTOR_TO_KILO     1.0E-3
#define FACTOR_TO_MILLI    1.0E+3
#define FACTOR_TO_MICRO    1.0E+6
#define FACTOR_TO_NANO     1.0E+9
#define FACTOR_TO_PICO     1.0E+12

#define MOUSE_LEFT_BUTTON  1
#define MOUSE_RIGHT_BUTTON 2
#define MOUSE_BUTTONS      3

#define LOCUS_UNKNOWN      0
#define LOCUS_MENUS        1
#define LOCUS_FORMS        2
#define LOCUS_APPLICATION  3
#define LOCUS_POPUP        4

#define  MAX_COLOR_SETS 4

typedef struct colorset_CI_pair {
   UCHAR  foreground;
   UCHAR  background;
} COLOR_PAIR;

struct colorset {

   COLOR_PAIR     regular;
   COLOR_PAIR     reverse;
   COLOR_PAIR     highlight;
   COLOR_PAIR     shaded;
   COLOR_PAIR     error;
   /* and others as needed */
} ;

// typedef struct colorset COLOR_SET in formtype.h

typedef struct colorsetrec {
   CHAR           description[40];
   COLOR_SET      set;
} COLOR_SET_RECORD;


enum { COLORS_DEFAULT, COLORS_MESSAGE, COLORS_ERROR, COLORS_MENU } ;

#define REGULAR_COLOR   0
#define REVERSE_COLOR   1
#define HIGHLIGHT_COLOR 2
#define SHADED_COLOR    3
#define ERROR_COLOR     4

#define MAX_COLOR       4

typedef int ERROR_CATEGORY;

extern ERROR_CATEGORY (* error_handler)(ERROR_CATEGORY err, ...);
extern void (* audible_error_handler)(void);

// index values for the ColorSets[] array
extern COLOR_SET ColorSets[ MAX_COLOR_SETS ] ;

extern unsigned default_form_attributes;

extern BOOLEAN deferGraphCursor;

extern UCHAR (*UserKeyHandler)(UCHAR key);
extern unsigned char (* UserKeyHandler)(unsigned char key);

#pragma pack(1)

// -----------------------------------------------------------------------

struct FltParam {
   UCHAR right_of_decimal;
   UCHAR dummy;
};

struct TglParam {
   UCHAR item_index;
   UCHAR total_items;
};

struct StrParam {
   UCHAR max_len;
   UCHAR dummy;
};

struct SelParam {
   UCHAR match_value;
   UCHAR dummy;
};

struct AnyParam {
   UCHAR one;
   UCHAR two;
};

struct data_index {
   UCHAR group;
   USHORT  item;
};

typedef struct field_def {

   UCHAR  type;
   USHORT attrib;
   SHORT  MacFieldIndex; // for keystroke record and playback
   // index into a file of macro commands
   // one of which will activate this field

   USHORT help_index;
   struct data_index primary_data;
   struct data_index alternate_data;
   struct data_index verifier;
   union {
      struct FltParam fltfld;
      struct TglParam tglfld;
      struct StrParam strfld;
      struct SelParam selfld;
      struct AnyParam anyfld;
   } specific;
   UCHAR row;
   UCHAR column;
   UCHAR width;
   CHAR exit_vectors[8]; /* default, enter, up, down, */
} FIELD;                 /* left, right, tab_left, tab_right */

// -----------------------------------------------------------------------

typedef struct data_def {
   void * pointer;
   SHORT  offset;
   UCHAR  type;
   UCHAR  attrib;
   UCHAR  limit_index;
} DATA;

typedef SHORT (* CODEFUNC)(void) ;

/* for CODE_Registries MLM */

typedef struct {
   SHORT   (*pointer)(void);
   SHORT   offset;
   UCHAR   type;
   UCHAR   attrib;
   UCHAR   limit_index;
} EXEC_DATA;

/* use for code registries with logic or scroll form routines */
/* which must return type UCHAR instead of type int as above */

#define CAST_CHR2INT (SHORT (*)(void))

// -----------------------------------------------------------------------
struct form_def {

   SHORT     field_index;
   SHORT     previous_field_index;
   USHORT    attrib;
   CHAR      next_field_offset;
   UCHAR     exit_key_code;
   UCHAR     status;
   UCHAR     row;
   UCHAR     column;
   UCHAR     size_in_rows;
   UCHAR     size_in_columns;
   UCHAR     display_row_offset;     /* for scrolling forms */
   USHORT    virtual_row_index;      /* for scrolling forms */
   struct data_index init_function;
   struct data_index exit_function;
   UCHAR     color_set_index;
   UCHAR     string_cursor_offset;
   UCHAR     display_cursor_offset;
   UCHAR     field_char_count;
   UCHAR     field_overfull_flag;
   SHORT     number_of_fields;
   FIELD *   fields;
   SHORT     MacFormIndex;          // for keystroke record and playback
                                    // index into a file of macro commands
                                    // one of which will activeate this form
   DATA *            dataRegistry[6];
            // 0 is reserved because data_index.group == 0 is used to
            // indicate a "NULL" entry.

} ;

// typedef struct form_def FORM in formtype.h

/* -----------------------------------------------------------------------
/
/  FIELD_CLASS
/
/  purpose: used to define the behaviour of a field type.
/     It contains pointers to functions and the field type bit.
/     The field type bit is used for character filtering for input
/     fields (see FormsCTypeCheck[] below), which is one unique
/     characteristic of fields.  The functions define the other
/     unique characteristics, such as how data is translated to
/     and from the field string, how characters are dealt with, and
/     how input data values are verified (as a class - individual
/     fields also can have specific verification functions associated
/     with them).  The only pre-defined field type is the STRING type;
/     all others are linked in separately, and invoke a function to
/     initialize the appropriate entry in the table.  The field type
/     value is used as the index to this table.
/
/ ----------------------------------------------------------------------- */

// -----------------------------------------------------------------------

typedef struct fld_class {
   SHORT      TypeBit;
   BOOLEAN  (*field_char_action)(UCHAR);
   void     (*data_to_field)(void);
   SHORT      (*data_from_field)(void);
   BOOLEAN  (*data_limit_action)(void);

} FIELD_CLASS;

// -----------------------------------------------------------------------

struct data_limit_def {
   void *            low_limit_ptr;
   void *            high_limit_ptr;
   void *            min_delta_ptr;
} ;

// typedef struct data_limit_def DATA_LIMIT in formtype.h

// -----------------------------------------------------------------------


#define STRATTR_REGULAR    0
#define STRATTR_REV_VID    1
#define STRATTR_HIGHLIGHT  2
#define STRATTR_SHADED     4
#define STRATTR_ERROR      8
// other attributes such as underline, small, large, etc. may be defined
// later for use in generic string output

typedef struct save_area_info {
   CRECT       diagonal;
   CMINMAX     valid_col;
   CMINMAX     valid_row;
   USHORT      BitMap;
   void _huge *pixels;
} SaveAreaInfo;

/* -----------------------------------------------------------------------
/  char *   FieldString;
/
/  purpose: This string contains the textual image of the data associated
/     with the field.  When entering a field, the data is converted
/     into text with an appropriate format (determined by which
/     "data_to_string" routine is used) and placed into this FieldString.
/     If ENTER was typed to exit the field, the text in FieldString is
/     converted back into the original data type and stored.  Of course,
/     if the data is a string, it is simply copied to and from FieldString.
/
/  char *   FieldDisplayString;
/
/  purpose: This string is the actual string presented on the screen
/     in the field "window".  It contains as much of FieldString as
/     will fit in the field width, and is formatted according to field
/     attributes such as left or right justification.
/
/  These two strings are shared by all fields in all forms.
/ ----------------------------------------------------------------------- */

typedef struct frm_cntxt {

   FIELD *          Field;
   FIELD_CLASS *    FieldClass;
   DATA *           FieldData;
   void *           FieldDataPtr;
   DATA *           FieldAltData;
   void *           FieldAltDataPtr;
   CHAR             FieldString[MAX_FIELD_SIZE + (MAX_FIELD_SIZE / 2) + 1];
   CHAR             FieldDisplayString[MAX_FIELD_SIZE + 1];
   SHORT            (*FieldVerifyFunction)(void *, CHAR *);
   BOOLEAN          (*FormInitFunction)(SHORT);
   BOOLEAN          (*FormExitFunction)(SHORT);
   COLOR_SET *      FormColorSet;
   FORM *           Form;
   struct save_area_info *     SavedArea;
   struct frm_cntxt *   PreviousStackedContext;

} FORM_CONTEXT;

// -----------------------------------------------------------------------

/* macros to allow editing of forms without the form editor */

/* Use in exit vectors for exit from form */

#define FORM_EXIT_UP (-1)
#define FORM_EXIT_DN (255)

#define EXIT ((-1))
#define EXIT_DN ((254))

/* Use in exit vectors calc offset to a named field. */
#define exit_to( a, b ) ( b - a )

/* ---------------------------------------------------------------
field_set macro. The first parameter (a) is a compile time constant
(enum type recommended) which is the array index of the field you 
are defining. This provides three benefits: it is easy to set the
exit vectors for a field, it is (therefore) easier to rearrange a
form, and it's safer to do things which depend on the field position.

The rest of the parameters are identical to those FORMED
generates, except for exit vectors. For each exit vector,
the macro takes the array index (name) of the destination field for
that vector, once again using the compile time constant.
Thus instead of specifying an offset for each exit vector,
you can specify a destination field name.

Notice that I defined the constant EXIT above. It can be used for
any exit vector which is to exit the form.

To improve readability, when I use this macro I put each part of the 
field structure on a separate line. This is especially helpful since
in the macro you cannot use the {} brackets which normally delimit
the different fields in the FIELD structure definition.
----------------------------------------------------------------*/

#define field_set(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x) \
{ b, c, d, e, { f, g }, {h, i}, {j, k}, {l, m}, n, o, p, \
   { exit_to(a, q), exit_to(a, r), exit_to(a, s), exit_to(a, t), \
exit_to(a, u), exit_to(a, v), exit_to(a, w), exit_to(a, x) } }

/***************************************************************************
This is an example field definition using the macro:

enum {FIRSTFIELD=0, SECONDFIELD... DTYPE, ... };

field_set(DTYPE,                 // field name
FLDTYP_TOGGLE,                   // field type
FLDATTR_REV_VID,                 // field attributes
KSI_THIS_FIELD,                  // Keystroke index
DA_FORM_FIELD_HBASE + 1,         // field help index
DGROUP_1461, 13,                 // field data primary
DGROUP_TOGGLES, 2,               // field data alternate
DGROUP_CODE, 10,                 // field data verify routine
0, 4,                            // field specific parameter
1, 68, 8,                        // field row, column, length
EXIT, DTYPE, DAMODE, DTEMP,      // exit vectors
DAMODE, MEMS, DAMODE, MEMS ),    // where each name is another field name

****************************************************************************/

/* The label_field macro defines a label type field - a display only  */
/* string which will not be changed by the program.  Still supply a   */
/* field index constant, so it is easy to rearrange the form          */

#define label_field(a,b,c,d,e,f) \
{ FLDTYP_STRING, \
  FLDATTR_DISPLAY_ONLY, \
  KSI_NO_INDEX, \
  0,\
  { b, c }, \
  { 0, 0 }, \
  { 0, 0 }, \
  { 0, 0 }, \
  d, e, f,  \
  { 1, 1, 0, 0, 0, 0, 0, 0 } }

/* here is an example of using label_field():

  label_field(LBL_BLOCK1,         
              DGROUP_DOSTRING, 0,
              2, 2, 9),          

*/

// use this macro to define other display only fields

#define d_field_(a,b,c,d,e,f,g,h,i) \
{ b, c, 0, d, { e, f }, {0, 0}, {0, 0}, {0, 0},\
g, h, i, \
{ 1, 1, 0, 0, 0, 0, 0, 0 } }

/* Bob's old display only macro */

#define do_field_set(b,c,d,e,f,g,h,i,j,k,l,m,n,o) \
{ b, c, 0, d, { e, f }, {g, h}, {i, j}, {k, l}, m, n, o, \
{ 1, 1, 0, 0, 0, 0, 0, 0 } }

/***************************************************************************/
//  Keyboard idle function stack

typedef struct key_idle_calls{
   UCHAR  (*current_handler)(void);
   void *          prev_handler;
} KEY_IDLE_CALLS;

typedef UCHAR KeyIdleFunc( void ) ;

extern UCHAR screen_rows;
extern UCHAR screen_columns;
extern SHORT                active_locus;
extern BOOLEAN            mouse_is_useable;
extern FIELD_CLASS        FieldClassArray[];
extern FORM_CONTEXT       Current;
extern SHORT                FormsCTypeCheck[];
extern KEY_IDLE_CALLS     *keyboard_idle;

/* -----------------------------------------------------------------------
/
/  FUNCTION DECLARATIONS
/
/ ----------------------------------------------------------------------- */

/* forms.c */

void   (*context_sensitive_help)(SHORT help_index);
extern void (*KSActivateFieldRecord)(SHORT GlobalFieldIndex,
                                    CHAR * DisplayString);
extern void (*KSFieldFocusRecord)(SHORT FormFieldIndex);

extern void (*KSActivateFormRecord)(SHORT FormIndex, BOOLEAN Nested);
extern void (*KSDeActivateFormRecord)(void);
extern void (*KSActivateMenuRecord)(SHORT MenuIndex, SHORT ItemIndex);
extern void (*KSDeActivateMenuRecord)(void);
extern void (*KSPlayNextMenuItem)(void);

extern BOOLEAN *pKSRecord;
extern BOOLEAN *pKSPlayBack;
extern CHAR * * ppKSPlayFieldString;
extern SHORT * pKSCharPosition;
extern float * pPlayBackDelay;

extern CMARKERTYPE TextCursor;
extern BOOLEAN TextCursorOn;

void legalize_chars_for_field(USHORT type_bit,
                              UCHAR * legal_char_set);
void noop(void);
SHORT int_noop(void);
BOOLEAN char_noop(UCHAR key);
BOOLEAN limit_noop(void);
void display_field_to_screen(BOOLEAN cursor_on);
void update_display_string(void);
void erase_form_area(UCHAR attrib, BOOLEAN border);
UCHAR currentFormStatus(void);
void setCurrentFormStatus(UCHAR newStatus);
FORM * CurrentForm(void);
void * resolve_address(DATA * descriptor);
void format_and_display_field(BOOLEAN cursor_on);
SHORT get_data_from_field(void);
BOOLEAN init_field(void);
BOOLEAN init_form(FORM * new_form);
BOOLEAN shutdown_form(void);
BOOLEAN push_form_context(void);
void pop_form_context(void);
void draw_form_fields(void);
void draw_form(void);
void string_to_field_string(CHAR * source_string);
void cursor_right(void);
void cursor_left(void);
void cursor_full_left(void);
void cursor_full_right(void);
void char_delete_left(void);
void char_delete_at_cursor(void);
void delete_entire_field(void);
void char_insert(UCHAR key);
BOOLEAN default_char_action(UCHAR key);
UCHAR exit_code_to_vector_index(UCHAR exit_code);
CHAR exit_code_to_vector(UCHAR exit_code);
void find_next_field_index(void);
void unzoom_field(void);
BOOLEAN setup_for_form_entry(FORM * this_form,
USHORT * global_attributes);
BOOLEAN setup_for_nested_form_entry(FORM * DescendForm,
USHORT * global_attributes);
SHORT   StandardVerify(UCHAR key);
void run_input_field(void);
void run_D_O_field(void);
void exit_from_nested_form(void);
UCHAR core_run_form(FORM * this_form,
USHORT * global_attributes,
BOOLEAN ResumeOperation);
UCHAR run_form(FORM * this_form, USHORT * global_attributes,
BOOLEAN ResumeOperation);
BOOLEAN initialize_form_system(void);
void shutdown_form_system(void);
void disable_field(FORM * Form, SHORT Index, BOOLEAN UnReverse);
void enable_field(FORM * Form, SHORT Index, BOOLEAN UnReverse);
BOOLEAN display_random_field(FORM *Form, SHORT FieldIndex);
void cheat_display(FORM * Form, SHORT Field, CHAR * string);
void playFieldForMacro(void);

/* fltfld.c */
void strip_trailing_zeroes(char * num_string);
BOOLEAN limit_float(void);
void uses_float_fields(void);

/* scfltfld.c */
void uses_scale_float_fields(void);

/* intfld.c */
BOOLEAN limit_int(void);
void uses_int_fields(void);

/* unintfld.c */
BOOLEAN limit_uns_int(void);
void uses_uns_int_fields(void);

/* hxintfld.c */
void uses_hex_int_fields(void);

/* rmintfld.c */
void uses_rom_int_fields(void);

/* tglfld.c */
void uses_toggle_fields(void);

/* frmatfld.c */
void uses_format_fields(void);

/* selctfld.c */
void uses_select_fields(void);

/* keyibmpc.c */
extern BOOLEAN NoAutoPlay;
UCHAR (*poll_mouse_event)(void);
UCHAR get_FORM_key_input(void);
BOOLEAN keyboardEscape(void);
UCHAR lowLevelKeyboardIdle(void);
void forceKeyIdleLoop(KeyIdleFunc * newFunction);
void restoreKeyIdleLoop(void);

/* mousefrm.c */

extern UCHAR (*application_mouse_service)(USHORT buttons,
   int row, int column, int XPos, int YPos, 
   BOOLEAN *LeftUp, BOOLEAN *RightUp,
   BOOLEAN *BothUp, BOOLEAN *LeftDrag, BOOLEAN *RightDrag,
   BOOLEAN *BothDrag);
UCHAR menus_left_button_handler(UCHAR row, UCHAR column);
UCHAR forms_left_button_handler(UCHAR row, UCHAR column);
void uses_mouse_input(void);
void done_using_mouse_input(void);
BOOLEAN DebounceMouseKey(void);

/* scrlform.c */
extern BOOLEAN scroll_refresh_only_flag;
UCHAR scroll_entry_field(void);
UCHAR scroll_up_field(void);
UCHAR scroll_down_field(void);
UCHAR refresh_scroll_only(FORM_CONTEXT * FrmCntxt);
void set_scroll_form_index(FORM * scroll_form, USHORT new_index);
USHORT index_of_scroll_form(FORM * scroll_form);
void show_active_field(FORM * TargetForm, int field_index);
void redraw_scroll_form(FORM * scroll_form, USHORT new_index, int active_field);

/* mouseraw.c || mousegss.c */

void startup_mouse(void);
BOOLEAN init_mouse_device(void);
void shut_down_mouse_device(void);
USHORT sample_mouse_position(int * row, int * column, int * XPos, int * YPos);

#ifdef CGIBIND_INCLUDED

void erase_mouse_cursor(void);
void replace_mouse_cursor(void);
void set_mouse_cursor_type(unsigned Type);
void MouseCursorEnable(BOOLEAN On);
BOOLEAN GetMouseCursorEnable(void);

#endif  // CGIBIND_INCLUDED

/* scrnraw.c || scrngss.c */

UCHAR set_attributes(UCHAR fore, UCHAR back);
void init_colors(CDVHANDLE device, int colors);
void init_display_device(void);
void set_cursor(int row, int column);
void set_cursor_type_default(void);
void emit(char character, int row, int column, int attrib);
void show_overflow(char overflow_status, int row, int column, int attrib);
void display_string(char * string, int len, int row, int column, int attrib);
void scroll_up(char row, char column, char n_rows, char n_columns);
void scroll_down(char row, char column, char n_rows, char n_columns);
BOOLEAN scroll_up_action(void);
BOOLEAN scroll_down_action(void);

void erase_screen_area(UCHAR row, UCHAR column,
UCHAR n_rows, UCHAR n_columns, UCHAR attrib, BOOLEAN border);
struct save_area_info * save_screen_area(UCHAR row, UCHAR column,
                                         UCHAR n_rows, UCHAR n_columns);
BOOLEAN restore_screen_area(struct save_area_info * save_buffer);
void SetExitRestoreFunc(void(*)(void));
void InitTextCursors(void);
void set_cursor_type(enum CursorType);
void erase_cursor(void);
int row_to_y(int row);
int column_to_x(int Column);
int y_to_row(int coord);
int x_to_column(int coord);
int DumpKeyBuffer(unsigned ScanKey);
void run_LOGIC_field(void);
void string_to_field(void);
int string_from_field(void);
void show_toggle(int row, int column, int attrib);
CXY GetTextCursorLoc(short *Row, short *Column, BOOLEAN *On) ;

#endif     // FORMS_INCLUDED
