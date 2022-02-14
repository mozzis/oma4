/* -----------------------------------------------------------------------
/
/  macskel.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/macro/macskel.c_v   0.14   09 Jan 1992 16:46:32   cole  $
/  $Log:   J:/logfiles/oma4000/macro/macskel.c_v  $
 * 
 *    Rev 0.14   09 Jan 1992 16:46:32   cole
 * Change #include's.
 * 
 *    Rev 0.13   25 Jul 1991 08:32:14   cole
 * Added $Header and $Log comment lines.
/
*/

#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include "macparse.h"
#include "omaform.h"
#include "di_util.h"
#include "crvheadr.h"
#include "forms.h"
#include "macruntm.h"
#include "macnres.h"
#include "macprodx.h"
#include "formwind.h"
#include "maccomp.h"
#include "macsymbl.h"
#include "syserror.h"  // ERROR_OPEN
#include "omaerror.h"

/* -----------------------------------------------------------------------
/
{##
  begin
    rmargin:=78;
    indent:=2;
    string_quote:='"';
    quoted_string:='\"';
    debugging := false;
    writeln('/ ', progname, '.c');
    end;
  ##}
/
/  Written by: TLB      Version 1.00
/  Worked on:  TLB      Version 1.01
/
/  .c is a .
/
/ ----------------------------------------------------------------------- */

BOOLEAN debug_flag = FALSE;

{##
   var K, S, COUNT, UD: integer;   {parser tables}
   begin
     indent:=4;

     write('int StateToIndex[]={0, ');
     for k:=ldim(statex) to udim(statex)-1 do
       write(statex[k], ', ');
     writeln(statex[udim(statex)], '};');
     writeln;

     write('int ReductionProdFlag[]={0, ');
     for k:=ldim(map) to udim(map)-1 do
       write(map[k], ', ');
     writeln(map[udim(map)], '};');
     writeln;

     write('int ReductionStackPopCount[]={0, ');
     for k:=ldim(popno) to udim(popno)-1 do
       write(popno[k], ', ');
     writeln(popno[udim(popno)], '};');
     writeln;

     write('int ExpectedTokens[]={');
     for k:=ldim(toknum) to udim(toknum)-1 do
       write(toknum[k], ', ');
     writeln(toknum[udim(toknum)], '};');
     writeln;

     write('int NextStateFromToken[]={');
     for k:=ldim(tostate) to udim(tostate)-1 do
       write(tostate[k], ', ');
     writeln(tostate[udim(tostate)], '};');
     writeln;

     write('int PossibleStatesAfterReduction[]={');
     for k:=ldim(stk_state) to udim(stk_state)-1 do
       write(stk_state[k], ', ');
     writeln(stk_state[udim(stk_state)], '};');
     writeln;

     write('int StateAfterReduction[]={');
     for k:=ldim(stk_tostate) to udim(stk_tostate)-1 do
       write(stk_tostate[k], ', ');
     writeln(stk_tostate[udim(stk_tostate)], '};');
     writeln;

     if debugging then begin
        {These are for printing tokens in parser stack dumps.}
      write('char *tokchar[]={"", ');
       string_quote:='"';
       for k:=1 to all_toks do begin
         if (tokens[k] = '?') then write('"\\"')   {c needs extra backslash for literal backslash}
         else write(qstring(tokens[k]));
         if k<all_toks then write(', ')
         else writeln('};');
         end;
       writeln;

       write('int prodx[]={0, ');
       for k:=ldim(prodx) to udim(prodx)-1 do
         write(prodx[k], ', ');
       writeln(prodx[udim(prodx)], '};');
       writeln;

       write('int prods[]={0, ');
       for k:=ldim(prods) to udim(prods)-1 do
         write(prods[k], ', ');
       writeln(prods[udim(prods)], '};');
       writeln;

       write('int insym[]={0, ');
       for k:=ldim(insym) to udim(insym)-1 do
         write(insym[k], ', ');
       writeln(insym[udim(insym)], '};');
       writeln;

       end;   {of debugging}
     end;
   ##}

static BOOLEAN              TokenValid = FALSE;
static SemanticRecord       TokenSemRec;  // current semantics assoc. with token
static SemanticRecord       ReductionSemRec;
static BOOLEAN              WithinComment = FALSE;
static char                 ch;           // next character from input file
static int                  token;        // Next token in input list
static int line_cnt;          // number of source lines (for error report)
static int                  CurrentState;

static CHAR *CompileMessage[] = { " Compiling ", NULL };

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

// -----------------------------------------------------------------------

PRIVATE void NewToken(int Type, int TokenValue, int Reference)
{
  TokenSemRec.Token = TokenValue;
  TokenSemRec.Type = Type;
  TokenSemRec.Reference = Reference;
  TokenValid = TRUE;
}

PRIVATE void TokenRead(void)
{
  TokenValid = FALSE;
}

// LEXICAL ANALYZER STARTS HERE

// -----------------------------------------------------------------------

PRIVATE void nextch(void)
{
  ch = ParseLine[TokenOffset];

  if ((ch != EOFCH) && (ch != EOLCH))
    TokenOffset++;                    // don't move past an eof mark
}

// -----------------------------------------------------------------------

PRIVATE void backch(void)
{
  if (TokenOffset > 1)
    {
    TokenOffset--;
    ch = ParseLine[TokenOffset];
    }
}

// -----------------------------------------------------------------------

PRIVATE void SkipOnlyWhiteSpace(void)
{
  while (ch == ' '|| ch == '\t' || ch == '\f')   // for control chars
    nextch();
}

PRIVATE void SkipCommentText(void)
{
  while ((ch != EOLCH) && (ch != '}'))
    nextch();

  if (ch == '}')
    {
    WithinComment = FALSE;
    nextch();
    }
}

   // comments will be bracketed in {} braces

PRIVATE void SkipWhiteSpace(void)
{
  do
    {
    if (ch == '{')
      WithinComment = TRUE;

    if (WithinComment)
      SkipCommentText();

    SkipOnlyWhiteSpace();
    }
  while (ch == '{');
}

// -----------------------------------------------------------------------

PRIVATE BOOLEAN gather_number_string(char * NumberString)
{
  int         Digits = 0;
  BOOLEAN     UseDigit;
  BOOLEAN     ExpSeen = FALSE;
  BOOLEAN     DotSeen = FALSE;

  do
    {
    if (Radix == 16)
      UseDigit = isxdigit(ch);  // only legal chars for hex numbers
    else if (isdigit(ch))       // valid decimal digit?
      UseDigit = TRUE;
    else if (ch == '.')         // dot? only if not already seen
      {
      if (DotSeen)
        UseDigit = FALSE;
      else
        {
        DotSeen = TRUE;
        UseDigit = TRUE;
        }
      }
    else if (toupper(ch) == 'E')     // Exponent? only if not already seen
      {
      if (ExpSeen)
        UseDigit = FALSE;
      else
        {
        ExpSeen = TRUE;
        UseDigit = TRUE;
        }
      }
    else if ((ch == '+') || (ch == '-'))   // only if Exponent seen
      UseDigit = ExpSeen;
    else
      UseDigit = FALSE;                // all other characters not used

    if (UseDigit)
      {
      NumberString[Digits++]=ch;
      nextch();
      }
    }
  while (UseDigit);

  NumberString[Digits] = '\0';

  return ((Radix != 16) && (ExpSeen || DotSeen));    // floating point?
}

// -----------------------------------------------------------------------

PRIVATE void GetNumberConstantToken(void)
{
  char           NumberString[60];
  char *         dummy;
  long           long_conversion;
  double         float_conversion;
  DECL_INDEX     Ref;

  if (gather_number_string(NumberString))
    {
    float_conversion = atof(NumberString);
    Ref = CreateConstantSymbol(CLASS_SCALAR, TYPE_REAL, REAL_TOKX,
      (void *) &float_conversion);

    NewToken(TYPE_REAL, REAL_TOKX, Ref);
    }
  else
    {
    long_conversion = strtol(NumberString, &dummy, Radix);
    Ref = CreateConstantSymbol(CLASS_SCALAR, TYPE_LONG_INTEGER, INT_TOKX,
      (void *) &long_conversion);

    NewToken(TYPE_LONG_INTEGER, INT_TOKX, Ref);
    }
  // unable to create constant - symbol table probably full
  if (Ref == INVALID_INDEX)
    macro_error(ERROR_SYMTAB_FULL, "(CONSTANT)");
}

// -----------------------------------------------------------------------

PRIVATE DECL_INDEX DoSymbolDeclaration(char * Symbol)
{
  DECL_INDEX Ref;

  if ((Ref = CreateSymbol(Symbol)) == INVALID_INDEX)
    macro_error(ERROR_SYMTAB_FULL, Symbol);
  else
    {
    NewDeclarations[DeclarationsOfType++] = Ref;

    SymbolTable[Ref].Token = IDENT_TOKX;
    SymbolTable[Ref].Type = TYPE_NEWLY_CREATED;
    }
  return(Ref);
}

// -----------------------------------------------------------------------

PRIVATE void GetSymbolToken(void)
{
  register int SymbolLength;
  char Symbol[MAX_IDENT_LEN + 1];
  DECL_INDEX Ref;
  SHORT FIndex = INVALID_INDEX; // form index for keystroke
                                // playback

  SymbolLength = 0;

  while (isalpha(ch) || isdigit(ch) || (ch == '_'))
    {
    if (SymbolLength < MAX_IDENT_LEN)
      Symbol[SymbolLength++] = (char)toupper(ch);

    nextch();
    }
  Symbol[SymbolLength] = NIL;

  if ((Ref = LookupSymbol(Symbol, &FIndex)) == INVALID_INDEX)
    Ref = DoSymbolDeclaration(Symbol);

  if (Ref != INVALID_INDEX)
    {
    // don't write form index if not a form !!!
    if (FIndex != INVALID_INDEX)
      SymbolTable[Ref].U.Procedure.SubIndex = FIndex;
    NewToken(SymbolTable[Ref].Type, SymbolTable[Ref].Token, Ref);
    }
}

// -----------------------------------------------------------------------

PRIVATE void GetStringToken(void)
{
  BOOLEAN           end_of_string;
  char              TestString[80+1];  // fix this!
  char *            StringLocation;
  DECL_INDEX        Ref;
  int               i = 0;

  nextch();                            // get past the first quote mark
  do
    {
    // note delimiters
    while ((ch != EOFCH) && (ch != EOLCH) && (ch != STRING_QUOTE))
      {
      TestString[i++]=ch;                   // fetch string from infile
      nextch();
      }
    end_of_string = TRUE;
    if (ch == STRING_QUOTE)     // peek ahead to see if quote is repeated
      {
      nextch();
      if (ch == STRING_QUOTE)
        {
        end_of_string = FALSE;   // Yes it was, insert quote char itself
        TestString[i]=ch;
        nextch();
        }
      }
    else if ((ch == EOFCH) || (ch == EOLCH))
      {
      macro_error(ERROR_STRING_UNTERMINATED, "");
      }
    }
  while (!end_of_string);

  TestString[i++]=EOS;            // append terminator

  StringLocation = NULL;

  if (i > 1)                          //  i will equal one if null string
    StringLocation = FindStringInTable(TestString);

  if (StringLocation == NULL)
    {
    StringLocation =
      AddStringToTable(TestString, (unsigned char) i);
    }
  if (StringLocation == NULL)
    {
    macro_error(ERROR_TOO_MANY_STRINGS, "");
    Ref = INVALID_INDEX;
    }
  else
    {
    Ref = CreateConstantSymbol(CLASS_STRING, TYPE_STRING, STR_TOKX,
      (void *) StringLocation);

    if (Ref == INVALID_INDEX)
      macro_error(ERROR_SYMTAB_FULL, "(STRING CONSTANT)");
    }

  NewToken(TYPE_STRING, STR_TOKX, Ref);
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// This recognizes all those non-alphanumeric tokens that
// are such that the first character is a prefix of
// some other token.  These are the `is_mult_char' tokens.
// The strategy is to collect all the characters that
// can follow the first character of such tokens, then
// search for the resulting string in the symbol table.
// We test the longest such sequence first, then backtrack
// one character at a time until a longest token is found
// or a failure occurs.  Failure means a lexical error.
// This token class is loaded into the symbol table in the
// INITTABLES procedure.
// ----------------------------------------------------------------------

PRIVATE BOOLEAN GetOddballToken(void)
{
   char              Uch, Symbol[MAX_IDENT_LEN + 1];
   DECL_INDEX        Ref;
   int               SymbolLength = 0;
   SHORT             Extras;

   SymbolLength=0;
   Symbol[SymbolLength++]=ch;
   nextch();
   Uch = toupper(ch);

   TokenSemRec.Token = STOP_TOKX; // just in case -- should never see daylight

{## var K, UD: integer;
    begin
      indent:=2;
      string_quote:='''';   {C designation for single char}
      ud:=udim(mult_char_fol);
      if ud >= ldim(mult_char_fol) then begin
        write('while (');
        for k := ldim(mult_char_fol) to ud do begin
          write('(Uch == ');
          if (mult_char_fol[k] = '?') then write(qstring('\\'))
          else write(qstring(mult_char_fol[k]));
          if k < ud then writeln(') || ');
          end;
        writeln('))');
        end
      else copy(false);
      end;  ##}
   {
      Symbol[SymbolLength++] = Uch;
      nextch();
      Uch = toupper(ch);
   }
{## ##}
   Symbol[SymbolLength]=NIL;     // SymbolLength is characters in string

  for(;;)
    {
    if ((Ref = LookupSymbol(Symbol, &Extras)) == INVALID_INDEX)
      {
      if (SymbolLength>=1)
        {
        Symbol[--SymbolLength]='\0';
        backch();
        }
      else
        return FALSE;
      }
    else                                // found the token
      {
      SymbolTable[Ref].U.Procedure.SubIndex = Extras;
      NewToken(SymbolTable[Ref].Type, SymbolTable[Ref].Token, Ref);
      return TRUE;
      }
    }
}

PRIVATE void GetSingletToken(int TokenValue)
{
   DECL_INDEX        Ref;
   char              Symbol[2];
   SHORT             Extras;

   Symbol[0] = ch;
   Symbol[1] = '\0';

   if ((Ref = LookupSymbol(Symbol, &Extras)) != INVALID_INDEX)
   {
      SymbolTable[Ref].U.Procedure.SubIndex = Extras;
      NewToken(SymbolTable[Ref].Type, TokenValue, Ref);
   }
   else
      NewToken(0, TokenValue, INVALID_INDEX);

   nextch();
}

// -----------------------------------------------------------------------

{## copy(debugging) ##}
{## ##}

PRIVATE void string_from_character(char format_ch, char * string)
{
  if (isprint(format_ch))
    {
    string[0] = format_ch;
    string[1] = '\0';
    }
  else
    {
    sprintf(string, "'\\0x%x'", format_ch);
    }
}

PRIVATE BOOLEAN GetNextToken(void)
{
  char char_str[15];

  if (TokenValid)
    return(TokenSemRec.Token != EOL_TOKX);

  TokenSemRec.Type = TYPE_DONT_CARE;                    // default case
  SkipWhiteSpace();
  ErrorOffset = TokenOffset;

  if (isalpha(ch))
    {
    GetSymbolToken();
    }
  else if (isdigit(ch))
    {
    Radix = 10;

    GetNumberConstantToken();
    }
  else switch (ch)
    {
    case STRING_QUOTE:
      GetStringToken();
    break;

{## copy(debugging) ##}
    case '!':
      debug_flag = (!debug_flag);
      nextch();
      GetNextToken();
    break;

{##   {the following generates inline case transfers for
         singlet tokens -- those not starting with an alphanumeric, and
         of length = 1}
    var K, L, U: integer;
    begin
      indent:=4;
      string_quote:='''';
      for k:=1 to term_toks do begin
        if is_singlet[k] then begin  {these can be recognized instantly
                                      as a single character}
        if (tokens[k] = '?') then
        writeln('case ', '''', '\\', '''', ': GetSingletToken(', k,
          '); break;')
        else
        writeln('case ', qstring(tokens[k]), ': GetSingletToken(', k,
          '); break;')
        end
      end;

    { Add code to recognize numeric base specifiers }

    writeln('case ''$'': Radix=16; nextch(); GetNumberConstantToken(); break;');

    {the following generates inline case transfers for the multiplet tokens
      -- those not starting with an alphanumeric, and of length > 1 }

    l:=ldim(mult_char_pfx);
    u:=udim(mult_char_pfx);
    if u>=l then begin   {don't do anything if there's no need}
      for k:=l to u do
        if (mult_char_pfx[k] = '?') then writeln('case ', qstring('\\'),':')
        else writeln('case ', qstring(mult_char_pfx[k]), ':');
      copy(true);
      end
    else copy(false);
    end;
##}
      if (!GetOddballToken())
        {
        string_from_character(ch, char_str);
        macro_error(ERROR_SYNTAX, char_str);
        GetNextToken();                 // try again
        };
      break;
{## ##}
      default:
         if (ch == EOFCH)
           NewToken(TYPE_DONT_CARE, STOP_TOKX, 0);
         else if (ch == EOLCH)
           {
           NewToken(TYPE_DONT_CARE, EOL_TOKX, 0); // accept an end-of-line token
           }
         else
           {
           string_from_character(ch, char_str);
           macro_error(ERROR_SYNTAX, char_str);
           nextch();
           GetNextToken();                 // try again
           }
         break;
  }                                // end switch
  return(TokenSemRec.Token != EOL_TOKX);
}

// -----------------------------------------------------------------------

PRIVATE void init_lex(void)
{
   TokenValid = FALSE;
   TokenOffset = 0;
   ErrorOffset = 0;
   nextch();                                  // fetch 1st char
}

// -----------------------------------------------------------------------

PRIVATE void parse_pushread(int State, SemanticRecord * SemPtr)
{
   if(StackTop >= PARSER_STACK_SIZE)
      macro_error(ERROR_EXPR_TOO_COMPLEX, "");
   else {
      SemanticStack[ ++ StackTop ] = * SemPtr;
      StateStack[ StackTop ] = State;
   }
}

// ----------------------------------------------------
// writes the print name of the TX'th token,
// returning the number of characters output.
// ----------------------------------------------------

{## copy(debugging) ##}

char * NameThatToken(int TokenValue)
{
   DECL_INDEX        i;

   if ((TokenValue>=1) && (TokenValue<=ALL_TOKS))
   {
      return(tokchar[TokenValue]);
   }
   else
   {
      for (i=0; i < NumberOfSymbolEntries; i++)
      {
         if (SymbolTable[i].Token == TokenValue)
            return(SymbolTable[i].Name);
      }
   }
   return("????");
}

void wrtok(int tx)
{
   fprintf(stdprn,  " %s", NameThatToken(tx));
}

void dump_sem(int indent, SemanticRecord * semrec)
{
   char * name;

   if (semrec != NULL)
   {
      if (semrec->Reference != INVALID_INDEX)
         name = SymbolTable[semrec->Reference].Name;
      else
         name = "";

      fprintf(stdprn, " ");
      while (indent-- > 0)
         fprintf(stdprn, " ");
      if (semrec->Type == TYPE_BOOLEAN)
         semrec->Type = TYPE_WORD;
      switch (semrec->Type)
      {
         case TYPE_INTEGER:
            fprintf(stdprn,  "INTEGER: >>%s<<", name);
            break;
         case TYPE_WORD:
            fprintf(stdprn,  "WORD: >>%s<<", name);
            break;
         case TYPE_LONG_INTEGER:
            fprintf(stdprn,  "LONG INT: >>%s<<", name);
            break;
         case TYPE_REAL4:
            fprintf(stdprn,  "REAL4: >>%s<<", name);
            break;
         case TYPE_REAL:
            fprintf(stdprn,  "REAL: >>%s<<", name);
            break;
         case TYPE_BYTE:
            fprintf(stdprn,  "BYTE: >>%s<<", name);
            break;
         case TYPE_BOOLEAN:
            fprintf(stdprn,  "BOOL: >>%s<<", name);
            break;
         case TYPE_CURVE:
            fprintf(stdprn,  "CURVE: >>%s<<", name);
            break;
         case TYPE_STRING:
            fprintf(stdprn,  "STRING: >>%s<<", name);
            break;
         case TYPE_PROCEDURE:
            fprintf(stdprn,  "PROCEDURE: >>%s<<", name);
            break;
         case TYPE_USER_PROCEDURE:
            fprintf(stdprn,  "USER PROC: >>%s<<", name);
            break;
         default:
            fprintf(stdprn,  "????");
            break;
      }
   }
}

void wrprod(int prx)
{
  prx = prodx[prx];
  wrtok(prods[prx]);
  fprintf(stdprn, " ->");
  while (prods[++prx] != 0)
    {
    wrtok(prods[prx]);
    }
}

void stk_dump(char * ActionID, int * StateStack, int StackTop,
                     int CurrentState)
{
  int sx, tl, ll;
  int count;

  fprintf(stdprn, "%s - State (%d)", ActionID, CurrentState);
  if (CurrentState >= READSTATE)
    {
    fprintf(stdprn, " - on token:", TokenSemRec.Token);
    wrtok(TokenSemRec.Token);
    fprintf(stdprn, "\n");
    }
  else                 // reduce state
    {
    fprintf(stdprn, ", RULE: ");
    wrprod(CurrentState);
    fprintf(stdprn, "\n");
    if (StackTop > 15)
      {
      fprintf(stdprn, "  ###\n");
      ll = StackTop-15;
      }
    else
      ll = 1;
    for (sx = ll; sx <= StackTop; sx++)
      {
      fprintf(stdprn, " %2d ", StackTop-sx);
      wrtok(insym[(sx == StackTop) ? CurrentState : StateStack[sx+1] ]);
      count = MAXTOKLEN - tl + 1;
      while (count-- > 0)
        fprintf(stdprn, " ");
      dump_sem(0, &SemanticStack[sx]);
      fprintf(stdprn, "\n");
      }
    }
}
{## ##}

// -----------------------------------------------------------------------

void StartNewParseOperation(void)
{
   StackTop = -1;
   ReductionSemRec.Type = TYPE_DONT_CARE;
   parse_pushread(STK_STATE_1, &ReductionSemRec);
   CurrentState = START_STATE;
   CurrentLine = 0;
   MacroStatus = PARSE_START;
   init_sem();
}

// -----------------------------------------------------------------------
void ParseSingleLine(char * ParseText)
{
  int index;

  ParseLine = ParseText;
  init_lex();

   // can also leave loop from "break;" if EOL_TOKX seen
  while ((CurrentState != 0) && (MacroStatus != PARSE_ERROR))
    {
    MacroStatus = PARSE_ACTIVE;
    if (CurrentState < READSTATE)               // a reduce state
      {
      ReductionSemRec.Type = TYPE_DONT_CARE;
      ReductionSemRec.Reference = INVALID_INDEX;
{## copy(debugging) ##}
      if (debug_flag)
        stk_dump("*REDUCE*", StateStack, StackTop, CurrentState);
{## ##}
      // contains production flags; if not zero, apply the semantics action
      if (ReductionProdFlag[CurrentState] != 0)
        apply(ReductionProdFlag[CurrentState], &ReductionSemRec);

      if (ReductionStackPopCount[CurrentState] == 0)  // empty production
        {
        parse_pushread(PossibleStatesAfterReduction[StateToIndex[CurrentState]],
               &ReductionSemRec);
        CurrentState = StateAfterReduction[StateToIndex[CurrentState]];
        }
      else
        {
        StackTop -= ReductionStackPopCount[CurrentState] - 1;

        if (((ReductionStackPopCount[CurrentState] == 1) &&
              (ReductionSemRec.Type != TYPE_DONT_CARE)) ||
              (ReductionStackPopCount[CurrentState] != 1))
          {
          SemanticStack[ StackTop ] = ReductionSemRec;
          }

        index = StateToIndex[CurrentState];       // compute the GOTO state
        CurrentState = StateStack[StackTop];
        while ((PossibleStatesAfterReduction[index] != CurrentState) &&
                (PossibleStatesAfterReduction[index] != 0))
          index++;
        CurrentState = StateAfterReduction[index];
        }
      }
    else if (CurrentState < LOOKSTATE)               // a read state
      {
{## copy(debugging) ##}
      if (debug_flag)
        stk_dump("*READ*", StateStack, StackTop, CurrentState);
{## ##}
      if (!GetNextToken())          // need next token now
        break;                      // EOL_TOKX seen, leave loop

      index = StateToIndex[CurrentState];
      while ((ExpectedTokens[index] != 0) &&
             (ExpectedTokens[index] != TokenSemRec.Token))
        {
        index++;
        }
      if (ExpectedTokens[index] == 0)
        {
        if (TokenSemRec.Reference > 0)
          {
          macro_error(ERROR_SYNTAX, SymbolTable[TokenSemRec.Reference].Name);
          }
        else
          {
          macro_error(ERROR_SYNTAX, "(UNKNOWN_SYMBOL)");
          }
{## copy(debugging) ##}

        fprintf(stdprn, "\nFound >>");
        wrtok(TokenSemRec.Token);
        fprintf(stdprn, "<< expected:");
        for (index = StateToIndex[CurrentState]; ExpectedTokens[index] != 0;
            index++)
          {
          wrtok(ExpectedTokens[index]);
          }
        fprintf(stdprn, "\n");
{## ##}
        MacroStatus = PARSE_ERROR;
        CurrentState = 0;          // BOMB OUT RIGHT HERE!
        }
      else
        {
        parse_pushread(CurrentState, &TokenSemRec);
        CurrentState = NextStateFromToken[index];
        TokenRead();
        }
      }
    else                                           // lookahead state
      {
{## copy(debugging) ##}
      if (debug_flag)
        stk_dump("*LOOKAHEAD*", StateStack, StackTop, CurrentState);
{## ##}
      if (!GetNextToken())
        break;                  // EOL_TOKX seen, leave loop
      index = StateToIndex[CurrentState];
      while ((ExpectedTokens[index] != 0) &&
           (ExpectedTokens[index] != TokenSemRec.Token))
        {
        index++;
        }
      CurrentState = NextStateFromToken[index];
      }
    }
}

// -----------------------------------------------------------------------
void ReadAndParseSourceFile(char * filename)
{
  FILE *      ProgFile;
  char        file_line[128];
  WINDOW *    MessageWindow;

  if ((ProgFile = fopen(filename, "r")) != NULL)
    {
    put_up_message_window(CompileMessage, COLORS_MESSAGE,  &MessageWindow);

    StartNewParseOperation();
    
    while ((fgets(file_line, 128, ProgFile) != NULL) &&
           (MacroStatus != PARSE_ERROR))
      {
      CurrentLine += 1;
      if (file_line[strlen(file_line) - 1] == '\n')
        file_line[strlen(file_line) - 1] = '\0';  // remove '\n' from end
      ParseSingleLine(file_line);
      }
    fclose(ProgFile);

    if (MacroStatus != PARSE_ERROR)
      {
      file_line[0] = EOFCH;   // feed END_OF_FILE character to parser
      file_line[1] = NIL;
      ParseSingleLine(file_line);
      }

    if (MessageWindow != NULL)
      release_message_window(MessageWindow);
    }
  else
    {
    if (ParseLine == NULL)
      {
      sprintf(file_line, "RUN('%s')", filename);
      ParseLine = file_line;
      }
    macro_error(ERROR_OPEN, filename);
    }
}

// -----------------------------------------------------------------------

// PARSE INITIALIZATION

PRIVATE void InitSymbolTokenValue(char * Sym, int TokenValue)
{
  DECL_INDEX Ref;
  SHORT Dummy;

  if ((Ref = LookupSymbol(Sym, &Dummy)) == INVALID_INDEX)
    {
    Ref = CreateSymbol(Sym);
    }
  if (Ref != INVALID_INDEX)
    SymbolTable[Ref].Token = TokenValue;
}

// -----------------------------------------------------------------------
void InitReservedKeywords(void)
{
  int i;

  {##
    var K: integer;
  begin
    string_quote:='"';
  for k:=1 to term_toks do
    if like_ident[k] or is_mult_char[k] then
      writeln('InitSymbolTokenValue(', qstring(tokens[k]), ', ', k, ');');
  end;
##}
  for (i=0; i < NumberOfSymbolEntries; i++)
    if (SymbolTable[i].Token == 0)
      SymbolTable[i].Token = IDENT_TOKX;
}
