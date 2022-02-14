/*************************************************************************/
/*  DOSSHELL.C                                                           */
/*                                                                       */
/*     spawn a DOS shell as a new process                                */
/*                                                                       */
/*************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <malloc.h>    // heapmin()

#include "dosshell.h"
#include "fkeyfunc.h"  // ShowNormalFKeys()
#include "forms.h"     // erase_screen_area()
#include "device.h"    // setScreenColors()
#include "curvdraw.h"  // PutUpPlotBoxes()
#include "omamenu.h"   // ShowHKeys()
#include "omaform.h"   // isFormGraphWindow()
#include "omaerror.h"  // error()
#include "syserror.h"  // ERROR_DOSSHELL
#include "graphops.h"  // HotPlotScreen()

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

int ShellCmd(char * cmd_string)
{
  int status = 0, form = 0;
  FORM_CONTEXT * temp;

  CEnterCTextMode(screen_handle);   /* enter cursor mode */
  _heapmin();
  status = system(cmd_string);     /* do DOS command */
  CExitCTextMode(screen_handle);   /* re-enter graphics mode */

  erase_screen_area(0, 0, 2, 80,
    (unsigned char) ((ColorSets[COLORS_DEFAULT].regular.background << 4) |
    ColorSets[COLORS_DEFAULT].regular.foreground) , FALSE);

  if (MenuFocus.ActiveMENU)
    draw_menu();
  if (Current.Form)
    {
    init_form(Current.Form);
    if (Current.Form->attrib & FORMATTR_VISIBLE)
      draw_form();  /* draw current (e.g. macro) form */
    }

  temp = Current.PreviousStackedContext; /* redraw previous nested forms */

  while (temp)
    {                                    /* form may be fieldless - */
    if (temp->Form->fields)              /* e.g. graph part of split */
      {                                  /* form */
      form = 1;
      push_form_context();
      Current.Form = temp->Form;         /* call form init function and */
      init_form(temp->Form);             /* reset any DRAW_PERMITTED bits*/
      if (Current.Form->attrib & FORMATTR_VISIBLE)
        draw_form();
      pop_form_context();
      }
    temp = temp->PreviousStackedContext;
    }
  if ((!form) && plotAreaShowing())      /* don't redraw plot if form is */
    HotPlotScreen(0);                    /* a split form - init_form did */

  ShowHKeys(&MainMenu); 
  ShowFKeys(&FKey);  

  if (status == -1)
    error(ERROR_DOSSHELL, cmd_string);

  return(status);
}

// returns 0 for success, -1 for failure
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int newDOSPrompt(const char * first, const char * second)
{
  // NOTE : After shelling to DOS the first time, getenv() for the DOS
  // prompt will return a pointer to DOS_prompt[] because that is where
  // the environment string for PROMPT is stored.  If the new prompt
  // string were to be built directly into DOS_prompt[], then
  // strncat(first, ...)   might overwrite the characters that second
  // points to.  Therefore, it is necessary to construct the new prompt
  // string in newPrompt first, before copying it into DOS_prompt.

  enum { MaxPromptLen = 200 };

  char newPrompt[ MaxPromptLen + 1 ];

  // putenv() REQUIRES a static or a global
  static char DOS_prompt[ MaxPromptLen + 1 ];

  strcpy(newPrompt, "PROMPT=") ;

  if(first)
    strncat(newPrompt, first,  MaxPromptLen - strlen(newPrompt)) ;

  if(second)
    strncat(newPrompt, second, MaxPromptLen - strlen(newPrompt)) ;

  strcpy(DOS_prompt, newPrompt) ;

  return putenv(DOS_prompt) ;  
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void do_DOS_commands(unsigned Dummy)
{
  char *  dosname;
  char *  current_prompt_ptr = getenv("PROMPT") ;
  char *  savedPrompt = 0;
  char *  newPrompt   = "$p$g";
  BOOLEAN printPrompt = FALSE;

  set_menu_to_dosshell();

  if(current_prompt_ptr)
    savedPrompt = strdup(current_prompt_ptr) ;

  if(savedPrompt)
    newPrompt = savedPrompt;
  else if(current_prompt_ptr)
    newPrompt = current_prompt_ptr;

  if( newDOSPrompt( "Type \"exit\" to return to OMA4000 program$_",
    newPrompt))
    {
    // try to restore the original prompt
    if(savedPrompt)
      (void) newDOSPrompt(current_prompt_ptr, 0) ;

    // print new prompt if can't get it into the DOS environment
    printPrompt = TRUE;
    }

  if(dosname = getenv("COMSPEC"))
    {
    if(printPrompt)
      printf("\nType \"exit\" to return to OMA4000 program.\n") ;

    ShellCmd(dosname);
    }

  if(savedPrompt)
    {
    (void) newDOSPrompt(savedPrompt, 0) ;
    free(savedPrompt) ;
    }
}
