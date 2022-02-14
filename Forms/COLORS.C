/* -----------------------------------------------------------------------
/
/   Colors.c
/
/ Copyright (c) 1988,  EG&G Instruments Inc.
/
  $Header:   C:/pvcs/logfiles/colors.c_v   1.0   27 Sep 1990 15:46:38   admax  $
  $Log:   C:/pvcs/logfiles/colors.c_v  $
 * 
 *    Rev 1.0   27 Sep 1990 15:46:38   admax
 * Initial revision.
/
/ ----------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "forms.h"
#include "formwind.h"
#include "barmenu.h"

#define KSI_NO_INDEX 0

#define   FILEOP_NONE       0
#define   FILEOP_READ       1
#define   FILEOP_WRITE      2


#define   COLORS_DEFAULT    0
#define   COLORS_MESSAGE    1
#define   COLORS_ERROR      2


#define   DGROUP_DO_STRINGS 1
#define   DGROUP_TOGGLES    2
#define   DGROUP_CODE       3
#define   DGROUP_GENERAL    4
#define   DGROUP_FORMS      5


#define   COLORGROUP_REGULAR   0
#define   COLORGROUP_REVERSE   1
#define   COLORGROUP_HIGHLIGHT 2
#define   COLORGROUP_SHADED    3
#define   COLORGROUP_ERROR     4

#define   BOX_ROW_OFFSET      1
#define   BOX_COLUMN_OFFSET   2
#define   BOX_ROWS          7
#define   BOX_COLUMNS       55

CDVHANDLE screen_handle;
CDVCAPABILITY screen;
MENUCONTEXT MenuFocus;
BOOLEAN scroll_refresh_only_flag;

unsigned int  default_form_attributes = (FORMATTR_FIRST_CHAR_ERASE);

int   background_color = BLUE;
int   foreground_color = BRT_WHITE;
int   group_index = 0;
int   color_set_index = 0;

int   number_of_color_sets = 1;

int   file_operation = FILEOP_NONE;
char    file_name[65] = "standard.clr";

char *  file_error_text[] = { "File I/O Error!", NULL };
char *  file_not_found_text[] = { "File not found!", NULL };
char *  file_not_ok_text[] = { "File doesn't seem to be color file!", NULL };
char *  file_write_verify_text[] = {
  "Write Color File? ",
  NULL
};

COLOR_SET ColorSets[] = {
{ { BRT_WHITE, BLUE },
  { BRT_WHITE, CYAN },
  { BRT_YELLOW, CYAN },
  { WHITE, BLUE },
  { BRT_WHITE, RED } },
{ { BRT_BLUE, WHITE },
  { WHITE, BLUE },
  { BRT_RED, WHITE },
  { BLACK, WHITE },
  { BRT_WHITE, RED } },
{ { BRT_WHITE, RED },
  { BRT_RED, WHITE },
  { BLACK, RED },
  { WHITE, RED },
  { BRT_WHITE, RED } }
};

COLOR_SET_RECORD FileColorSets[MAX_COLOR_SETS] = {
{ "Default Color Set",
  { { BRT_WHITE, BLUE },
    { BRT_WHITE, CYAN },
    { BRT_RED, BLUE },
    { WHITE, BLUE } ,
    { BRT_WHITE, RED } }
  }
};


char *  color_set_names[MAX_COLOR_SETS] = {
  FileColorSets[0].description
};

char * description_text = FileColorSets[0].description;

char *  group_options[] = {

  "Regular",
  "Reverse",
  "Highlight",
  "Shaded",
  "Error"
};

char *  color_options[] = {

  "Black",
  "Blue",
  "Green",
  "Red",
  "Cyan",
  "Purple",
  "Brown",
  "White (Grey)",
  "Bright Blue",
  "Bright Green",
  "Bright Red",
  "Bright Cyan",
  "Bright Purple",
  "Bright Yellow",
  "Bright White"
};


DATA DO_STRING_Reg[] = {
{ "Color", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
{ "Background", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
{ "Foreground", 0, DATATYP_STRING, DATAATTR_PTR, 0 },
{ "Color Group Used For:", 0, DATATYP_STRING, DATAATTR_PTR, 0 }
};

DATA FORMS_Reg[] = {
{ 0, 0, 0, 0, 0 }
};


DATA TOGGLES_Reg[] = {
{ group_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
{ color_options, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },
{ color_set_names, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 }
};

int draw_test_colors(void);
int select_group(void);
int select_index(void);
int set_desc_text_ptr(void);
int select_index(void);

EXEC_DATA CODE_Reg[] = {
{ draw_test_colors, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
{ select_group, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
{ select_index, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
{ set_desc_text_ptr, 0, DATATYP_CODE, DATAATTR_PTR, 0 },
{ select_index, 0, DATATYP_CODE, DATAATTR_PTR, 0 }
};


DATA GENERAL_Reg[] = {
{ &color_set_index, 0, DATATYP_INT, DATAATTR_PTR, 0 },
{ &group_index, 0, DATATYP_INT, DATAATTR_PTR, 0 },
{ &background_color, 0, DATATYP_INT, DATAATTR_PTR, 0 },
{ &foreground_color, 0, DATATYP_INT, DATAATTR_PTR, 0 },
{ file_name, 0, DATATYP_STRING, DATAATTR_PTR, 0 },
{ &file_operation, 0, DATATYP_INT, DATAATTR_PTR, 0 },
{ &number_of_color_sets, 0, DATATYP_INT, DATAATTR_PTR, 0 },
{ &description_text, 0, DATATYP_STRING, DATAATTR_PTR_PTR, 0 }
};


DATA * DataReg[] = {
  0,
  DO_STRING_Reg,
  TOGGLES_Reg,
  CODE_Reg,
  GENERAL_Reg
};


DATA_LIMIT DataLimitRegistry[] = {
  { 0, 0, 0 }, /* always zero */
/*  { &min_int, &max_int, 0 }, */
};

FIELD   ColorSelectFormFields[] = {

/* 0 */  label_field(0,
           DGROUP_DO_STRINGS, 0,
           9, 15, 5),

/* 1 */  label_field(0,
           DGROUP_DO_STRINGS, 1,
           11, 10, 10),

/* 2 */  label_field(0,
           DGROUP_DO_STRINGS, 2,
           13, 10, 10),

/* 3 */  label_field(0,
           DGROUP_DO_STRINGS, 3,
           15, 8, 21),

/* 4 */  { FLDTYP_TOGGLE,
           FLDATTR_REV_VID,
           0,
           0,
           {DGROUP_GENERAL, 1 },
           {DGROUP_TOGGLES, 0},
           {DGROUP_CODE, 1},
           {0, 5},
           9, 22, 14,
           { -5, 1, 3, 1, 0, 0, 3, 1 } },

/* 5 */  { FLDTYP_TOGGLE,
           FLDATTR_REV_VID,
           0,
           0,
           { DGROUP_GENERAL, 2 },
           {DGROUP_TOGGLES, 1},
           {DGROUP_CODE, 0},
           {0, 8},
           11, 22, 15,
           { -6, 0, -1, 1, 0, 0, -1, 1 } },

/* 6 */  { FLDTYP_TOGGLE,
           FLDATTR_REV_VID,
           0,
           0,
           {DGROUP_GENERAL, 3},
           {DGROUP_TOGGLES, 1},
           {DGROUP_CODE, 0},
           {0, 15},
           13, 22, 15,
           { -7, 0, -1, 1, 0, 0, -1, 1 } },

/* 7 */  { FLDTYP_TOGGLE,
           FLDATTR_REV_VID,
           0,
           0,
           {DGROUP_GENERAL, 0},
           {DGROUP_TOGGLES, 2},
           {DGROUP_CODE, 3},
           {0, 1},
           16, 8, 39,
           { -8, 0, -1, -3, 0, 1, -1, -3 } },

/* 8 */  { FLDTYP_STRING,
           FLDATTR_REV_VID | FLDATTR_GET_DRAW_PERMISSION | FLDATTR_NO_OVERFLOW_CHAR,
           0,
           0,
           {DGROUP_GENERAL, 7},
           {0, 0},
           {0, 0},
           {39, 0},
           17, 8, 39,
           { -9, 0, -1, 0, 0, 0, -1, -4 } }
};

FORM  ColorSelectForm = {
      0,
      0,
      FORMATTR_EXIT_RESTORE | FORMATTR_BORDER | FORMATTR_FIRST_CHAR_ERASE,
      0, 0, 0,
      2, 10, 19, 59,
      0, 0, 
      { 0, 0 },
      { 0, 0 },
      COLORS_DEFAULT,
      0, 0, 0, 0,
      sizeof(ColorSelectFormFields) / sizeof(FIELD),
      ColorSelectFormFields,
      0,
      0,
      DO_STRING_Reg,
      TOGGLES_Reg,
      FORMS_Reg,
      CODE_Reg,
      0};

#define   COLOR_DESCRIP_TOGGLE_FIELD    7 /* CHANGE IF FORM CHANGES */

unsigned char * color_desc_toggle_counter(void)
{
  return(&ColorSelectForm.fields[COLOR_DESCRIP_TOGGLE_FIELD].specific.tglfld.total_items);
}
      
int set_desc_text_ptr(void)
{
  description_text = FileColorSets[color_set_index].description;
  return select_index();
}

int select_group(void)
{
  COLOR_SET *     ModifyColorSet;

  ModifyColorSet = &FileColorSets[color_set_index].set;

  switch (group_index)
  {
    case COLORGROUP_REGULAR:
      foreground_color = ModifyColorSet->regular.foreground;
      background_color = ModifyColorSet->regular.background;
      break;
    case COLORGROUP_REVERSE:
      foreground_color = ModifyColorSet->reverse.foreground;
      background_color = ModifyColorSet->reverse.background;
      break;
    case COLORGROUP_HIGHLIGHT:
      foreground_color = ModifyColorSet->highlight.foreground;
      background_color = ModifyColorSet->highlight.background;
      break;
    case COLORGROUP_SHADED:
      foreground_color = ModifyColorSet->shaded.foreground;
      background_color = ModifyColorSet->shaded.background;
      break;
    case COLORGROUP_ERROR:
      foreground_color = ModifyColorSet->error.foreground;
      background_color = ModifyColorSet->error.background;
      break;
  }
  draw_form_fields();
  init_field();
    
return(FIELD_VALIDATE_SUCCESS);
}

int draw_test_colors(void)
{
  char *        reg_text =  "    Regular Text    ";
  char *        rev_text =  " Reverse Video Text ";
  char *        high_text = "   Highlight Text   ";
  char *        shad_text = "    Shaded Text     ";
  char *        err_text =  "     Error Text     ";
  unsigned char form_row,
                form_col,
                attrib;
  COLOR_SET *   ModifyColorSet;

  ModifyColorSet = &FileColorSets[color_set_index].set;

  switch (group_index)
  {
    case COLORGROUP_REGULAR:
      ModifyColorSet->regular.foreground = foreground_color;
      ModifyColorSet->regular.background = background_color;
      break;
    case COLORGROUP_REVERSE:
      ModifyColorSet->reverse.foreground = foreground_color;
      ModifyColorSet->reverse.background = background_color;
      break;
    case COLORGROUP_HIGHLIGHT:
      ModifyColorSet->highlight.foreground = foreground_color;
      ModifyColorSet->highlight.background = background_color;
      break;
    case COLORGROUP_SHADED:
      ModifyColorSet->shaded.foreground = foreground_color;
      ModifyColorSet->shaded.background = background_color;
      break;
    case COLORGROUP_ERROR:
      ModifyColorSet->error.foreground = foreground_color;
      ModifyColorSet->error.background = background_color;
      break;
  }

  form_row = Current.Form->row + BOX_ROW_OFFSET;
  form_col = Current.Form->column + BOX_COLUMN_OFFSET;

  attrib = set_attributes(ModifyColorSet->regular.foreground,
    ModifyColorSet->regular.background);

  erase_screen_area( form_row, form_col, BOX_ROWS, BOX_COLUMNS, attrib, TRUE);

  {
  int len = strlen(reg_text);
  display_string(reg_text, len, (form_row + 1), (form_col + 2), attrib);
  }

  attrib = set_attributes(ModifyColorSet->reverse.foreground,
    ModifyColorSet->reverse.background);

  display_string(rev_text, strlen(rev_text),
    (form_row + 1),
    (form_col + 26), attrib);

  attrib = set_attributes(ModifyColorSet->highlight.foreground,
    ModifyColorSet->highlight.background);

  display_string(high_text, strlen(high_text),
    (form_row + 3),
    (form_col + 2), attrib);

  attrib = set_attributes(ModifyColorSet->shaded.foreground,
    ModifyColorSet->shaded.background);

  display_string(shad_text, strlen(shad_text),
    (form_row + 3),
    (form_col + 26), attrib);

  attrib = set_attributes(ModifyColorSet->error.foreground,
    ModifyColorSet->error.background);

  display_string(err_text, strlen(err_text),
    (form_row + 5),
    (form_col + 2), attrib);

  return(FIELD_VALIDATE_SUCCESS);
}


int select_index(void)
{
  select_group();
  draw_test_colors();
  return(FIELD_VALIDATE_SUCCESS);
}


BOOLEAN write_color_file()
{
  BOOLEAN         status = FALSE;
  FILE *          bin_file;
  int           i;

  if ((bin_file = fopen(file_name, "wb")) != NULL)
  {
    fwrite(&number_of_color_sets, sizeof(int), 1, bin_file);

    for (i=0; i<number_of_color_sets; i++)
      fwrite( &FileColorSets[i], sizeof(COLOR_SET_RECORD), 1, bin_file);

    status = ( ! ferror(bin_file) );

    fclose(bin_file);
  }

  return(status);
}

BOOLEAN read_color_file()
{
  BOOLEAN         status = FALSE;
  FILE *          bin_file;
  int           i;

  if ((bin_file = fopen(file_name, "rb")) != NULL)
    {
    fread(&number_of_color_sets, sizeof(int), 1, bin_file);

    if (number_of_color_sets > 0 && number_of_color_sets <= MAX_COLOR_SETS)
      {
      for (i=0; i<number_of_color_sets; i++)
        {
        fread(&FileColorSets[i], sizeof(COLOR_SET_RECORD), 1, bin_file);
        color_set_names[i] = FileColorSets[i].description;
        }
      *(color_desc_toggle_counter()) = (unsigned char)number_of_color_sets;

      status = (!ferror(bin_file));
      }
    else
      {
      message_window(file_not_ok_text, COLORS_ERROR);
      }
    fclose(bin_file);
    }
  else
    {
    message_window(file_not_found_text, COLORS_ERROR);
    }
  return(status);
}

#define CGI_NOT_PRESENT -3003
#define CGI_NOT_TRANSIENT   -2978
#define DRIVERS_ALREADY_LOADED  -2977
#define MEMORY_TOO_SMALL_FOR_GSSCGI -3034
#define INSUFFICIENT_MEMORY -1
#define ALLOCATE_FAR_MEMORY(x) (char far *) halloc(x, sizeof (char))
#define FREE_FAR_MEMORY(x) hfree (x)

char far   *where;            /* Where to load drivers */
CCONFIGURATION config;        /* GSS*CGI Configuration structure */
int transient;                /* Transient-drivers-loaded flag */

/************************************************************************
 *
 * Load Drivers
 *
 ************************************************************************/

int load_drivers (void)
{
  auto int  err;      /* Error variable */
  auto long bytes_needed;   /* Memory required for driver load */

  err = 0;        /* Assume no error */

  if (((CLoadCgi ((char far *)0, 0L, &bytes_needed))))
    {
    switch ((err = CInqCGIError()))
      {
      case CGI_NOT_PRESENT:
        printf ("\n%s%s%s\n",
          "GSS*CGI is not presently in memory.  Please load it by running the GSS*CGI\n",
          "Device Driver Management Utility, DRIVERS.EXE.  Then try running this\n",
          "program again.\n");
      break;

      case DRIVERS_ALREADY_LOADED:
      case CGI_NOT_TRANSIENT:
          err = 0;    /* These conditions are okay */
          transient = CFalse;
      break;

      default:
        printf ("\n%s%d.\n%s\n",
          "The following unrecognized error was returned from Load CGI: ", err,
          "Please refer to the GSS*CGI Programmer's Guide for the appropriate action.\n");
      break;
      }
    }
  else if ((where = malloc((USHORT)bytes_needed)) != (char far *) 0)
    {
    CLoadCgi (where, bytes_needed, &bytes_needed);
    transient = CTrue;
    }
  else
    {
    printf ("\n%s%s%s%s\n",
      "The program was unable to allocate sufficient memory to load GSS*CGI\n",
      "device drivers.  Please load them manually, using the GSS*CGI Device\n",
      "Driver Management Utility, DRIVERS.EXE.  Or remove resident programs\n",
      "in order to make additional memory available.\n");
    err = INSUFFICIENT_MEMORY;
    }
    return (err);
}


/***********************************************************************
 *
 * Find out what, if any, GSS*CGI configuration is in memory and proceed
 * to load it in, if it's not present.
 *
 ************************************************************************/

int load_configuration ()
{
  auto int    error;

  config.CGIPath = NULL;
  config.Where = NULL;
  config.Available = 0L;
  config.Required = 0L;
  error = 0;

  if (CCgiConfiguration (CStatusCGI, &config) == 0)
    {
    switch (config.Status)
      {
      case CNotLoaded:
        if (CCgiConfiguration(CLoadCGI, &config) &&
            (CInqCGIError() == MEMORY_TOO_SMALL_FOR_GSSCGI) &&
            ((config.Where = ALLOCATE_FAR_MEMORY(config.Required)) !=
             (char far *) 0))
          {
          config.Available = config.Required;
          if (CCgiConfiguration(CLoadCGI, &config))
            {
            error = CInqCGIError ();
            }
          }
        else
          error = CInqCGIError ();
      break;

      case CLoadedStatic:
      case CTransientLoaded:
      case CLoadedTSR:
      case CLoadedApp:
      break;                  /* No action necessary */

      case CTransient:
        error = load_drivers();
      break;
        }
    }
    return (error);
}


/************************************************************************
 *
 * Remove the GSS*CGI Configuration
 *
 ************************************************************************/

/**********************************************************************
 *
 * If the program loaded GSS*CGI device drivers with Load CGI, then
 * remove the drivers and release the memory used to contain them.
 *
 **********************************************************************/
void remove_drivers(void)
{
  if (transient == CTrue)
    {
    CKillCgi();
    if (where)
      FREE_FAR_MEMORY(where);
    }
}

/***********************************************************************
 *
 * If the GSS*CGI configuration was loaded by the application, then
 * remove it.
 *
 ************************************************************************/

int remove_configuration (void)
{
  int error;

  config.CGIPath = NULL;
  /* Save config.Where for call to FREE_FAR_MEMORY below */
  config.Available = 0L;
  config.Required = 0L;
  error = 0;

  if (CCgiConfiguration(CStatusCGI, &config) == 0)
    {
    switch (config.Status)
      {
      case CLoadedApp:
        if (CCgiConfiguration(CRemoveCGI, &config))
          {
          error = CInqCGIError();
          }
        FREE_FAR_MEMORY(config.Where); /* Release the configuration memory */
      break;

      case CTransientLoaded:
        remove_drivers();
      break;

      case CNotLoaded:
      case CLoadedStatic:
      case CTransient:
      case CLoadedTSR:
      break;                  /* No action necessary */
      }
    }
  return (error);
}

// open the screen and clear it
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int openAndClearScreen(CCOLOR markerColor)
{
  int returnVal;
  CDVOPEN screen_setup =
            {         
            CPreserveAspect,              // coordinate transform mode 1
            CLN_Solid,                    // initial line type
            1,                            // initial line color
            CMK_Plus,                     // marker type = Star
            0,                            // marker color, for graph cursor
            1,                            // initial graphics text font
            1,                            // initial graphics text color
            1,                            // initial fill interior style
            1,                            // initial fill style index
            1,                            // initial fill color index
            1,                            // prompting flag
            "DISPLAY"                     // driver link name
            };

  screen_setup.MarkerColor = markerColor;

  if (load_configuration() < 0)
    return (-1);

  returnVal = COpenWorkstation(&screen_setup, &screen_handle, &screen);

  if(returnVal != -1)
    CClearWorkstation(screen_handle);

  return returnVal;
}

int main(int argc, char ** argv)
{
  int err = openAndClearScreen(BRT_YELLOW);

  if(err != -1)
    {
    if (initialize_form_system())
      {
      uses_int_fields();
      uses_toggle_fields();

      if (argc > 1)
        strcpy(file_name, argv[1]);

      if (read_color_file())
        {
        run_form(&ColorSelectForm, &default_form_attributes, FALSE);

        if ( yes_no_choice_window(file_write_verify_text, YES,
          COLORS_MESSAGE ) == YES )
          {
          write_color_file();
          }
        }

      shutdown_form_system();
      }
    else
      printf("\nError from initialize_form_system()!");
    }
  else
    printf("\nError from GSS init: %x", err);

  remove_configuration();
  return(0);
}
