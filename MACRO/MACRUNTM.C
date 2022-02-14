/* -----------------------------------------------------------------------
/
/  macruntm.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/macro/macruntm.c_v   1.5   06 Jul 1992 12:51:48   maynard  $
/  $Log:   J:/logfiles/oma4000/macro/macruntm.c_v  $
/
*/

#include <malloc.h>
#include <string.h>

#include "macruntm.h"
#include "forms.h"
#include "basepath.h"
#include "crvheadr.h"  // LONGTYPE
#include "macnres.h"
#include "macrecor.h"
#include "mdatstak.h"
#include "macparse.h"
#include "macrores.h"  // procedureExec()
#include "macsymbl.h"
#include "syserror.h"  // ERROR_ALLOC_MEM
#include "omaerror.h"
#include "ksindex.h"   // FORM_NAME_SIZE
#include "omaform.h"   // COLORS_ERROR
#include "macres2.h"
#include "formwind.h"  // message_window
#include "macrofrm.h"  // CommandOutput

OPERATOR * IP;      /* IP stands for Instruction Pointer */
OPERATOR * MainRoutine;
  
OPERATOR * CodeSpace;     /* contains compiled instruction lists */
long       CodeSpaceSize = 0;
OPERATOR * EndOfProgram;
  
char *     StringTable;
INDEX      StringTableFreeOffset = 0;
INDEX      StringTableSize = DEFAULT_STRING_TABLE_SIZE;
  
int        Radix = 10;
char *     ParseLine;      /* source line */
int        CurrentLine;
int        TokenOffset;    /* index of next character in LINE */
int        ErrorOffset;    /* current token index in LINE */
  
/* the apply() routine will manage these */
int        NewDeclarations[MAX_DECLARATIONS_OF_TYPE];
int        DeclarationsOfType = 0;
  
DECL_INDEX MostRecentExecution;
DECL_INDEX MainProgramReference;
  
int        CurveRefClass;
int        IOListLength;
int        FunctionParameterCount = 0;
BOOLEAN    Compiling;
  
double     ForCount = 1.0;
DECL_INDEX FORVariableReference = INVALID_INDEX;
DECL_INDEX FORCountReference = INVALID_INDEX;
DECL_INDEX InvalidIndexRef;
  
SemanticRecord * SemanticStack;
int *      StateStack;
int        StackTop;
  
int        MacroStatus = PARSE_COMPLETE;
BOOLEAN    MacroRunProgram = FALSE;

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

/* handle run time errors */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SetErrorFlag()
{
  PCHAR Buf = NULL;
  CHAR FormBuf[FORM_NAME_SIZE + 1];
  CHAR LineBuf[24];

  MacroStatus = PARSE_ERROR;

  DeleteTempMathFiles();

  if (SymbolTable[MostRecentExecution].Type == TYPE_PROCEDURE)
    {
    // test for actual function used instead of index value
    INDEX exeListIndex =
      SymbolTable[MostRecentExecution].U.Procedure.ExecuteListIndex;

    if(isMacPlayMenu(exeListIndex) || isMacPlayForm(exeListIndex) ||
       isMacPlayField(exeListIndex))
      {
      memset(FormBuf, 0, FORM_NAME_SIZE + 1);
      Buf = FormBuf;
      getPlayName(exeListIndex, Buf, FORM_NAME_SIZE);
      }
    else
      Buf = SymbolTable[MostRecentExecution].Name;
    }
  else   // not a procedure call
    Buf = SymbolTable[MostRecentExecution].Name;

  if (strlen(Buf))
    {
    char * NearLocation[] = { "Error in or near:", NULL, NULL, NULL };

    NearLocation[1] = Buf;
    NearLocation[2] = &LineBuf[0];
    
    if (!MacroRunProgram)
      sprintf(LineBuf, "at program line #%d", CurrentLine);
    else if (IP)
      sprintf(LineBuf, "at program line #%d", IP->LineNumber);
    else
      LineBuf[0] = 0;


    message_window(NearLocation, COLORS_ERROR);
    }
}

/* Handle parse time errors */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void macro_error(int errnum, char * id)
{
  int i;
  UCHAR cursor_offset;

  MacroStatus = PARSE_ERROR;

  if (SavedNumberOfSymbolEntries != INVALID_INDEX) /* backtrack var defs */
    {
    NumberOfSymbolEntries = SavedNumberOfSymbolEntries;
    SavedNumberOfSymbolEntries = INVALID_INDEX;
    }
  /* ~tilde is highest printable character */
  for (i=0; ((i < 79) && (ParseLine[i] <= '~')); i++)
    ;

  if (i > 0)  /* if parse line has printable characters */
    {
    if (!MacPlayBack) /* display parse line unless program was keystroke */
      {
      push_form_context();
      setCurrentFormToMacroForm();
      strncpy(CommandOutput, ParseLine, 79);
      CommandOutput[79] = '\0';
      Current.Form->field_index = 2;
      init_field();
      cursor_offset = Current.Form->display_cursor_offset;
      Current.Form->display_cursor_offset = (UCHAR) TokenOffset;
      format_and_display_field(TRUE);
      Current.Form->display_cursor_offset = cursor_offset;
      pop_form_context();
      }
    }

  error(errnum, id); /* only for errors with string params! */

  i = CurrentLine;   /* this assumes compile time error */
  
  if (i)
    error(ERROR_LINE_NUMBER, i);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int sizeof_Type(int Type)
{
  if (Type & POINTER_TO)
    return(sizeof(void *));

  if (Type == TYPE_BOOLEAN)
    Type = TYPE_WORD;

  switch (Type)
    {
    /*    case TYPE_BOOLEAN:  */
    case TYPE_INTEGER:
    case TYPE_WORD:
      return(sizeof(int));
    case TYPE_LONG_INTEGER:
      return(sizeof(long));
    case TYPE_BYTE:
      return(sizeof(char));
    case TYPE_REAL4:              
      return(sizeof(FLOAT));
    case TYPE_REAL:
      return(sizeof(double));

    default:          /* default to largest size */
    case TYPE_CURVE:
      return(sizeof(CURVE_REF));
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ResetIP()
{
  IP = EndOfProgram;   /* start at IP=beginning of free space */
}

// This function is also called for subroutines
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void RunProgram(OPERATOR * Initial_IP)
{
  OPERATOR *Saved_IP;
  BOOLEAN BranchFlag;
  BOOLEAN OldRunProgram = MacroRunProgram;

  Saved_IP = IP;
  IP = Initial_IP;

  MacroRunProgram = TRUE;
  while (MacroRunProgram)
    {
    switch (IP->Class)
      {
      case CLASS_EXECUTABLE:
        {
        DECLARATION * Execute = &SymbolTable[IP->Reference];

        MostRecentExecution = IP->Reference;

        switch (Execute->Type)
          {
          case TYPE_PROCEDURE:
            procedureExec(Execute->U.Procedure.ExecuteListIndex);
          break;

          case TYPE_USER_PROCEDURE:
            RunProgram(Execute->U.UserProcedure.Address);
          default:
          break;
          }
        }
      IP ++;
      break;

      case CLASS_BRANCH:
        ((char *)IP) = (((char *)IP) + ((BRANCH_OPERATOR *) IP)->JumpOffset);
      break;

      case CLASS_COND_BRANCH:
        PopFromDataStack((void *) &BranchFlag, TYPE_BOOLEAN);
        if (BranchFlag)
          ((char *) IP) = (((char *) IP) + sizeof(BRANCH_OPERATOR));
        else
          ((char *)IP) = (((char*) IP) + ((BRANCH_OPERATOR *)IP)->JumpOffset);
      break;

      case CLASS_EXECUTE_RETURN:
        MacroRunProgram = FALSE;
      break;

      default:
        {
        DECLARATION * Symbol = &SymbolTable[IP->Reference];

        switch (IP->Class)
          {
          case CLASS_SCALAR:
            PushToDataStack(&Symbol->U.Scalar, Symbol->Type, TRUE);
          break;

          case CLASS_STRING:
            PushToDataStack(Symbol->U.String.Pointer, Symbol->Type, TRUE);
          break;

          case CLASS_ARRAY:
            if (!getStackDepth() ||
                DataStackPeek(0) == (TYPE_STRING | POINTER_TO))
              {
               /* push pointer to array */
              PushToDataStack(Symbol->U.Array.Pointer, Symbol->Type, TRUE);
              }
            else                              /* push ptr to array element */
              {
              INDEX  CurrentIndex;
              char * Element;

              PopFromDataStack((void *) &CurrentIndex, TYPE_WORD);

              if (CurrentIndex > Symbol->U.Array.MaxIndex)
                {
                /* ARRAY OUT OF BOUNDS ERROR! */
                CurrentIndex = Symbol->U.Array.MaxIndex;
                error(ERROR_ARRAY_BOUNDS, Symbol->Name);
                SetErrorFlag();
                }
              else
                {
                Element = (((char *) Symbol->U.Array.Pointer)
                           + (sizeof_Type(Symbol->Type) * CurrentIndex));
                PushToDataStack(Element, Symbol->Type, TRUE);
                }

              }
          break;

          case CLASS_FILE:
          case CLASS_TEXTFILE:
            PushToDataStack(Symbol->U.File.FilePtr, Symbol->Type, TRUE);
          break;
          }
        }
      IP ++;
      }
    if (MacroStatus == PARSE_ERROR)
      MacroRunProgram = FALSE;
    }
  IP = Saved_IP;

  if (MacroStatus != PARSE_ERROR)        
    MacroRunProgram = OldRunProgram;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void RunMainProgram(void)
{
  InitDataStack();
  RunProgram(MainRoutine);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void ReleaseMacroLanguageMemory(void)
{
  forget(StringTable);
  forget(StateStack);
  forget(SemanticStack);
  releaseDataStack();
  forget(SymbolTable);
  forget(CodeSpace);
}

// return TRUE iff all memory allocated OK. FALSE ==> error condition
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN GetMacroLanguageMemory(void)
{
  BOOLEAN allocation_failure = FALSE;

  CodeSpaceSize = (long) (sizeof(OPERATOR) * DEFAULT_COMPILE_TOKEN_MAX);

  if(allocate(CodeSpace, OPERATOR, DEFAULT_COMPILE_TOKEN_MAX)  &&
    allocate(SymbolTable, DECLARATION, SymbolTableMaxEntries) &&
    allocateDataStack() &&
    allocate(SemanticStack, SemanticRecord, PARSER_STACK_SIZE) &&
    allocate(StateStack, int, PARSER_STACK_SIZE))

    if (! allocate(StringTable, char, DEFAULT_STRING_TABLE_SIZE))
      allocation_failure = TRUE;
    else
      erase(StringTable, char, DEFAULT_STRING_TABLE_SIZE);
  else
    allocation_failure = TRUE;

  if (allocation_failure)
    {
    ReleaseMacroLanguageMemory();
    error(ERROR_ALLOC_MEM);
    }
  return !allocation_failure;
}

// return FALSE iff error condition, TRUE means all is well
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN InitializeMacroLanguage(void)
{
  if (GetMacroLanguageMemory())
    {
    if(! ReadMacroSymbolFile(base_path(MACRO_FILE), &NumberOfSymbolEntries))
      {
      int i;
      DECLARATION * VarEntry;

      InitReservedKeywords();
      StringTableFreeOffset = 0;
      NumberOfReservedSymbolEntries = NumberOfSymbolEntries;
      EndOfProgram = CodeSpace;

      // allocate and initialize all predefined string variables
      for (i=0; i<NumberOfSymbolEntries; i++)
        {
        VarEntry = &SymbolTable[ i ];
        if ((VarEntry->Type == TYPE_STRING) && !VarEntry->U.String.Pointer)
          {
          VarEntry->U.String.Pointer =
            AddStringToTable("", (UCHAR)VarEntry->U.String.MaxLength);
          if (VarEntry->U.String.Pointer == NULL)
            {
            // error: no mem for str
            VarEntry->U.String.MaxLength = 0;
            return FALSE;
            }
          }
        }
      return TRUE;
      }
    }
  return FALSE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void ShutdownMacroLanguage(void)
{
  ReleaseMacroLanguageMemory();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN EnterMacroForm(void)
{
  BOOLEAN status;

  if (OpenFormRefFiles())        
    return FALSE;

  LocateMacroForm();                  /* set command line screen coords */
  status = InitializeMacroLanguage();

  pBufLen = &(SymbolTable[LookupSymbol("BUFLEN", NULL)].U.Scalar.Integer);
  pDefaultName = SymbolTable[LookupSymbol("DEF_NAME", NULL)].U.String.Pointer;

  StartNewParseOperation();
  return(status);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void LeaveMacroForm(void)
{
  pBufLen = &buflen;
  pDefaultName = DefaultName;
  ShutdownMacroLanguage();
  CloseFormRefFiles();
}

