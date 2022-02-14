/* -----------------------------------------------------------------------
/
/  maccomp.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/macro/maccomp.c_v   1.8   06 Jul 1992 12:50:10   maynard  $
/  $Log:   J:/logfiles/oma4000/macro/maccomp.c_v  $
 * 
 *    Rev 1.8   06 Jul 1992 12:50:10   maynard
 * Reinstate old handling of SIMPTYPE production, not same as VARIABLE
 * 
 *    Rev 1.7   12 Mar 1992 16:01:22   maynard
 * Changes to apply: so that if an undeclared identifier is found
 * in a program, a new symbol table entry is made for it, with type
 * determined by the initial letter of the identifier name; previous
 * behavior was the usual 'undefined symbol' error, which is OK for 
 * a compiled language but is inconvenient for a command line.  Also
 * combined case actions for VARH and SIMPTYPE since they were the same.
 * 
 * 
 * 
 *    Rev 1.6   09 Jan 1992 15:55:18   cole
 * Add include crvheadr.h ; delete include omatyp.h, oma35.h
 * 
 *    Rev 1.5   14 Oct 1991 10:38:22   cole
 * Minor changes to some macro_error() calls. Cosmetics.
 * 
 *    Rev 1.4   25 Jul 1991 08:25:40   cole
 * Added $Header and $Log comment lines.
/
/  ------------------------------------------------------------------------
*/
  
#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>
  
#include "maccomp.h"
#include "di_util.h"
#include "crvheadr.h"
#include "forms.h"
#include "macnres.h"
#include "macprodx.h"
#include "mdatstak.h"
#include "macsymbl.h"
#include "syserror.h"  // ERROR_ALLOC_MEM
#include "omaerror.h"
  
#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

PRIVATE BOOLEAN NoExplicit = TRUE;  /* Whether new vars must be defined 1st */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
long RelativeCodeOffset(OPERATOR * Location)
{
  return((long) Location - (long) CodeSpace);
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE OPERATOR * CodeAddressFromOffset(long Location)
{
  return((OPERATOR *) (((char *) CodeSpace) + Location));
}
  
// test for program too large error
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void TestCompileOverflow(void)
{
  if (RelativeCodeOffset(IP) >= CodeSpaceSize)
    {
    macro_error(ERROR_PROGRAM_TOO_LARGE, "");
    Compiling = FALSE;
    }
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
PRIVATE void MarkCompileLocation(void)
{
  long RelOff = RelativeCodeOffset(IP);

  PushToDataStack((void *) &RelOff, TYPE_LONG_INTEGER, FALSE);
}
  
/* -----------------------------------------------------------------------
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:  Be careful, because this and the
/  PushLoopReference() / PopLoopReference() functions both
/  use the data stack... make sure that pushes and pops are correctly
/  sequenced so the two don't get mixed up!
/
/ ----------------------------------------------------------------------- */
  
PRIVATE long RetrieveMarkedLocation(void)
{
  long Location;

  PopFromDataStack((void *) &Location, TYPE_LONG_INTEGER);
  return Location;
}
  
/* -----------------------------------------------------------------------
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:  Be careful, because this and the
/  MarkCompileLocation() / RetrieveMarkedLocation() functions both
/  use the data stack... make sure that pushes and pops are correctly
/  sequenced so the two don't get mixed up!
/
/ ----------------------------------------------------------------------- */
  
PRIVATE void PushLoopReference(void)
{
  PushToDataStack((void *) &FORVariableReference, TYPE_WORD, FALSE);
  PushToDataStack((void *) &FORCountReference, TYPE_WORD, FALSE);
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void PopLoopReference(void)
{
  PopFromDataStack((void *) &FORCountReference, TYPE_WORD);
  PopFromDataStack((void *) &FORVariableReference, TYPE_WORD);
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
PRIVATE void FixupBranchJumpOffset(long BranchOpLocation, long Destination)
{
  BRANCH_OPERATOR * OpPtr;

  OpPtr = (BRANCH_OPERATOR *) CodeAddressFromOffset(BranchOpLocation);

  OpPtr->JumpOffset = ((int) (Destination - BranchOpLocation));
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
PRIVATE long LocationAfterBranchOp(long BranchOpLocation)
{
  return (BranchOpLocation + (long) sizeof(BRANCH_OPERATOR));
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
PRIVATE void CompileCondBranch(void)
{
  ((BRANCH_OPERATOR *)IP)->Class = CLASS_COND_BRANCH;
  ((BRANCH_OPERATOR *)IP)->JumpOffset = sizeof(BRANCH_OPERATOR); /* dummy */
  // SkipBranch();
  ((char *) IP) = (((char *) IP) + sizeof(BRANCH_OPERATOR));
  TestCompileOverflow();
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
PRIVATE void CompileBranch(void)
{
  ((BRANCH_OPERATOR *) IP)->Class = CLASS_BRANCH;
  ((BRANCH_OPERATOR *) IP)->JumpOffset = sizeof(BRANCH_OPERATOR); /* dummy */
  // SkipBranch();
  ((char *) IP) = (((char *) IP) + sizeof(BRANCH_OPERATOR));
  TestCompileOverflow();
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
PRIVATE void CompileOperator(DECL_INDEX Ref)
{
  IP->Class = SymbolTable[Ref].Class;
  IP->Reference = Ref;
  IP ++;
  IP->LineNumber = CurrentLine;
  TestCompileOverflow();
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
PRIVATE void CompileRETURN(void)
{
  IP->Class = CLASS_EXECUTE_RETURN;
  IP ++;
  IP->LineNumber = CurrentLine;
  TestCompileOverflow();
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
void FORGET_ALL(void)
{
  NumberOfSymbolEntries = NumberOfReservedSymbolEntries;
  StringTableFreeOffset = 0;
  EndOfProgram = CodeSpace;
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
void init_sem(void)
{
  int invalid_index_var = INVALID_INDEX;

  InitDataStack();
  CurveRefClass = CLASS_UNKNOWN;
  IOListLength = 0;
  Compiling = FALSE;
  NoExplicit = TRUE;
  FORVariableReference = INVALID_INDEX;
  FORCountReference =
    CreateConstantSymbol(CLASS_SCALAR, TYPE_REAL, 0, (void *) &ForCount);
  InvalidIndexRef = CreateConstantSymbol(CLASS_SCALAR, TYPE_LONG_INTEGER,
                                     INT_TOKX, (void *) &invalid_index_var);
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN IdentMustBeNew(DECLARATION * Entry)
{
  if(Entry->Type == (UCHAR) TYPE_NEWLY_CREATED)
    return TRUE;

  macro_error(ERROR_SYMBOL_REDEFINED, Entry->Name);
  return FALSE;
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN IdentMustExist(DECLARATION * Entry)
{
  if(Entry->Type != (UCHAR) TYPE_NEWLY_CREATED)
    return TRUE;

  macro_error(ERROR_SYMBOL_UNKNOWN, Entry->Name);
  return FALSE;
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void apply(int ProductionFlag, SemanticRecord * ReductionSemRec)
{
  long BranchOpLocation;
  long Destination;
  int i;

  switch (ProductionFlag)
    {
    case EXECUTENOW:
      Compiling = FALSE;
      if (MacroStatus != PARSE_ERROR)
        {
        MacroStatus = PARSE_COMPLETE;
        }
    break;

    case PROGDECL:
      if (IdentMustBeNew(&SymbolTable[SemanticStack[StackTop].Reference]))
        {
        MainProgramReference = SemanticStack[StackTop].Reference;

        SymbolTable[MainProgramReference].Class = CLASS_EXECUTABLE;
        SymbolTable[MainProgramReference].Type = TYPE_USER_PROCEDURE;

        Compiling = TRUE;
        EndOfProgram = CodeSpace;
        ResetIP();
        }
    break;

    case IMMSTART:
      Compiling = TRUE;
      ResetIP();
      MainRoutine = IP;
    break;

    case IMMEND:
      CompileRETURN();
    break;

    /*    case PBLOCK:
      break; */
    case PBEGIN:
      SymbolTable[MainProgramReference].U.UserProcedure.Address = IP;
      MainRoutine = IP;
    break;

    case PDECL:
      if (IdentMustBeNew(&SymbolTable[ SemanticStack[StackTop].Reference ]))
        {
        DECLARATION *     ProcedureEntry;

        ProcedureEntry = &SymbolTable[SemanticStack[StackTop].Reference];

        ProcedureEntry->Class = CLASS_EXECUTABLE;
        ProcedureEntry->Type = TYPE_USER_PROCEDURE;
        ProcedureEntry->U.UserProcedure.Address = IP;
        }
    break;

    case PROGEND:
      CompileRETURN();
      if (MacroStatus == PARSE_ERROR)
        EndOfProgram = CodeSpace;
      else
        EndOfProgram = IP;   // allow program to be re-run (reserve space)
    break;

    case PBEND:
      CompileRETURN();
    break;

    case VARST:
      DeclarationsOfType = 0;
      SavedNumberOfSymbolEntries = NumberOfSymbolEntries;
    break;

    case VAREND:
      SavedNumberOfSymbolEntries = INVALID_INDEX;
    break;

    case VITEM:
      if (DeclarationsOfType > 0)
        {
        DECLARATION * VarEntry;
        UCHAR Type = (UCHAR)
          SymbolTable[SemanticStack[StackTop-1].Reference].U.Scalar.Integer;

        for (i=0; i<DeclarationsOfType; i++)
          {
          VarEntry = &SymbolTable[ NewDeclarations[i] ];
          VarEntry->Type = Type;
          if (Type == TYPE_STRING)
            {
            VarEntry->Class = CLASS_STRING;
            VarEntry->U.String.Pointer =
              AddStringToTable("", DEFAULT_STRING_LEN);
            if (VarEntry->U.String.Pointer == NULL)
              {
              VarEntry->U.String.MaxLength = 0; /* error: no mem for str */
              }
            else
              VarEntry->U.String.MaxLength = DEFAULT_STRING_LEN;
            }
          else
            {
            VarEntry->Class = CLASS_SCALAR;
            }
          }
        DeclarationsOfType = 0;
        }
    break;

    case NEWIDENT1:
    case NEWIDENT2:
      IdentMustBeNew(&SymbolTable[SemanticStack[StackTop].Reference]);
    break;

    case ARRAYTYPE:
      if (DeclarationsOfType > 0)
        {
        DECLARATION * VarEntry;
        UCHAR Type = (UCHAR)
          SymbolTable[SemanticStack[StackTop].Reference].U.Scalar.Integer;
        int tempE = SemanticStack[ StackTop - 3 ].Reference;
        int Elements = SymbolTable[ tempE ].U.Scalar.Integer;

        for (i=0; i<DeclarationsOfType; i++)
          {
          VarEntry = &SymbolTable[ NewDeclarations[i] ];

          VarEntry->Type = Type;
          VarEntry->Class = CLASS_ARRAY;

          VarEntry->U.Array.Pointer = malloc(Elements * sizeof_Type(Type));

          if (VarEntry->U.Array.Pointer == NULL)
            {
            VarEntry->U.Array.MaxIndex = 0; // error: no mem for array
            macro_error(ERROR_ALLOC_MEM, "");
            }
          else
            VarEntry->U.Array.MaxIndex = Elements;
          }
        DeclarationsOfType = 0; /* so VITEM does not undo our work */
        }
    break;
  
    case FORTO:
    case FORDOWNTO:
      CompileOperator(SemanticStack[StackTop-2].Reference); /* ':=' */
      MarkCompileLocation();
    break;

    case FORVARID:
      FORVariableReference = SemanticStack[StackTop].Reference;
      CompileOperator(SemanticStack[StackTop].Reference);
      * ReductionSemRec = SemanticStack[StackTop];
    break;

    case FORUP: // move increment to end of loop body
      CompileOperator(FORVariableReference);
      CompileOperator(LookupSymbol(">=", NULL));
      MarkCompileLocation();
      CompileCondBranch();
      PushLoopReference();
    break;

    case FORDOWN:  // move decrement to end of loop body
      CompileOperator(FORVariableReference);
      CompileOperator(LookupSymbol("<=", NULL));
      MarkCompileLocation();
      CompileCondBranch();
      PushLoopReference();
    break;

    case FOREND:      // move increment to end of loop body
      PopLoopReference();
      CompileOperator(FORVariableReference);
      CompileOperator(FORVariableReference);
      CompileOperator(FORCountReference);
      CompileOperator(LookupSymbol("+", NULL));
      CompileOperator(LookupSymbol(":=", NULL));

      BranchOpLocation = RetrieveMarkedLocation();
      FixupBranchJumpOffset(BranchOpLocation,
        LocationAfterBranchOp(RelativeCodeOffset(IP)));      /* DO */
      BranchOpLocation = RelativeCodeOffset(IP);
      CompileBranch();
      Destination = RetrieveMarkedLocation(); /* back to TO */
      FixupBranchJumpOffset(BranchOpLocation, Destination);
    break;

    case DFOREND:  // 4/10/90 TLB - move decrement to end of loop body
      PopLoopReference();
      CompileOperator(FORVariableReference);
      CompileOperator(FORVariableReference);
      CompileOperator(FORCountReference);
      CompileOperator(LookupSymbol("-", NULL));
      CompileOperator(LookupSymbol(":=", NULL));

      BranchOpLocation = RetrieveMarkedLocation();
      FixupBranchJumpOffset(BranchOpLocation,
        LocationAfterBranchOp(RelativeCodeOffset(IP)));      /* DO */
      BranchOpLocation = RelativeCodeOffset(IP);
      CompileBranch();
      Destination = RetrieveMarkedLocation(); /* back to DOWNTO */
      FixupBranchJumpOffset(BranchOpLocation, Destination);
    break;

    case REPSTART:
    case WHSTART:
      MarkCompileLocation();
    break;

    case WHILEDO:
    case THENT:
      MarkCompileLocation();
      CompileCondBranch();
    break;

    case ELSET:
      MarkCompileLocation();
      CompileBranch();
    break;

    case IFTH:
      BranchOpLocation = RetrieveMarkedLocation();
      FixupBranchJumpOffset(BranchOpLocation, RelativeCodeOffset(IP));
    break;

    case IFTHELSE:
      BranchOpLocation = RetrieveMarkedLocation();
      FixupBranchJumpOffset(BranchOpLocation, RelativeCodeOffset(IP));  /* ELSE */
      Destination = LocationAfterBranchOp(BranchOpLocation);
      BranchOpLocation = RetrieveMarkedLocation();
      FixupBranchJumpOffset(BranchOpLocation, Destination); /* THEN */
    break;

    case REPUNTILDONE:
      BranchOpLocation = RelativeCodeOffset(IP);
      CompileCondBranch();
      Destination = RetrieveMarkedLocation(); /* back to REPEAT */
      FixupBranchJumpOffset(BranchOpLocation, Destination);
    break;

    case WHILEDONE:
      BranchOpLocation = RetrieveMarkedLocation();
      FixupBranchJumpOffset(BranchOpLocation,
        LocationAfterBranchOp(RelativeCodeOffset(IP)));      /* DO */
      BranchOpLocation = RelativeCodeOffset(IP);
      CompileBranch();
      Destination = RetrieveMarkedLocation(); /* back to WHILE */
      FixupBranchJumpOffset(BranchOpLocation, Destination);
    break;

    /* case CASEFIELD:
      break;
    case CASELIST:
    break;
    case CASEITEM:
    break;
    */
    case READWRL:
      {
      int Ref;

      Ref = CreateConstantSymbol(CLASS_SCALAR, TYPE_INTEGER, INT_TOKX,
                                 (void *) &IOListLength);
      CompileOperator(Ref);         /* constant: I/O list length */
      CompileOperator(SemanticStack[StackTop-3].Reference); /* I/O op */
      }
    break;

    case RWREAD:
    case RWWRITE:
    case RWREADLN:
    case RWWRITELN:
      IOListLength = 0;
      * ReductionSemRec = SemanticStack[ StackTop ];
    break;

	 case EXPLICIT:
	   NoExplicit = FALSE;
	 break;

	 case NOEXPLIC:
	   NoExplicit = TRUE;
	 break;

    case PROCSTMT:  /* procedure called alone as a Statement */
      if (SymbolTable[SemanticStack[StackTop].Reference].Attrib &
         SYMATTR_RETURNS_VALUE)
        {
        CompileOperator(LookupSymbol("DROP", NULL));
        }
    break;

    case RVAL_PROC:
      if (!(SymbolTable[SemanticStack[StackTop].Reference].Attrib &
            SYMATTR_RETURNS_VALUE))
        macro_error(ERROR_NO_RETURN_VALUE,
          SymbolTable[SemanticStack[StackTop].Reference].Name);
    break;

    case PROCCALL:
      {
      DECLARATION * Symbol;
      int Ref;
      DECL_INDEX ProcRef;

      ProcRef = SemanticStack[StackTop-1].Reference;
      Symbol = &SymbolTable[ProcRef];
      if (IdentMustExist(Symbol))
        {

        /* if a special lookup function, save the subindex */
        if((Symbol->U.Procedure.SubIndex != INVALID_INDEX) &&
            (Symbol->Type != TYPE_USER_PROCEDURE))
          {
          Ref = CreateConstantSymbol(CLASS_SCALAR, TYPE_INTEGER, INT_TOKX,
                                    (void *) &(Symbol->U.Procedure.SubIndex));
          CompileOperator(Ref); /* constant: Special procedure index */
          }
        switch (Symbol->Type)
          {
          case TYPE_PROCEDURE:
            if (FunctionParameterCount != Symbol->U.Procedure.ParameterCount)
              macro_error(ERROR_PARAM_COUNT, Symbol->Name);
          break;

          case TYPE_USER_PROCEDURE:
            if (FunctionParameterCount > 0)
              macro_error(ERROR_PARAM_COUNT, Symbol->Name);
          break;

          default:
            macro_error(ERROR_UNKNOWN_PROC, Symbol->Name);
          break;
          }
        CompileOperator(ProcRef);
        * ReductionSemRec = SemanticStack[ StackTop - 1 ];
        }
      }
    break;
  
    case PROCLISTST:
      FunctionParameterCount = 0;
    break;

    case PROCLIST1:
    case PROCLIST2:
      FunctionParameterCount++;
      * ReductionSemRec = SemanticStack[ StackTop ];
    break;

    case IOLIST1:
    case IOLIST2:
      IOListLength++;

    case LESS:
    case GTR:
    case LEQ:
    case GEQ:
    case EQ:
    case NEQ:
      * ReductionSemRec = SemanticStack[ StackTop ];
    break;

    case ASSIGN:
      if (SymbolTable[SemanticStack[StackTop-2].Reference].Attrib &
          SYMATTR_CONSTANT)
        macro_error(ERROR_MODIFY_CONSTANT,
                    SymbolTable[SemanticStack[StackTop-2].Reference].Name);
  
    case OROP:   /* these ops are binary */
    case XOROP:
    case ANDOP:
    case RELOP:
    case ADD:
    case SUB:
    case MPY:
    case QUOT:
    case MODULO:
    case DIVIDE:
  
    case NOTOP:   /* these ops are unary */
    case SHLEFT:
    case SHRIGHT:
      CompileOperator(SemanticStack[StackTop-1].Reference);
    break;
  
    case NEG: /* otherwise it would compile "-" (subtract) */
      {
      int Ref;
      long MinusOne = (-1L);
  
      Ref = CreateConstantSymbol(CLASS_SCALAR, TYPE_LONG_INTEGER,
      INT_TOKX, (void *) &MinusOne);
      CompileOperator(Ref);         /* constant: -1 */
      CompileOperator(LookupSymbol("*", NULL));
      }
    break;

    case VARID:
      * ReductionSemRec = SemanticStack[ StackTop - 1 ];
      CompileOperator(SemanticStack[StackTop-1].Reference);
    break;
  
#ifdef NEWPLOT      
    case PLOTEND:
#endif
    case CURVEEND:
    case CURVEVAR:
    case TSIMP:
      * ReductionSemRec = SemanticStack[ StackTop ];
    break;
  
    case VARH:
      {
      DECL_INDEX Ref = SemanticStack[StackTop].Reference;
      DECLARATION * VarEntry = &SymbolTable[ Ref ];
      if(VarEntry->Type == (UCHAR)TYPE_NEWLY_CREATED)
        {
        if(NoExplicit)
          {
          /* if variable does not exist, create it */
          /* assign it a type based on the first character of */
          /* the variable name */
          
          int LeadChar = toupper(VarEntry->Name[0]);

          VarEntry = &SymbolTable[ Ref ];
          if (    LeadChar < 'I')
            VarEntry->Type = TYPE_REAL4;
          else if (LeadChar < 'O')
            VarEntry->Type = TYPE_LONG_INTEGER;
          else if (LeadChar < 'V')
            VarEntry->Type = TYPE_REAL;
          else
            VarEntry->Type = TYPE_WORD;
          VarEntry->Class = CLASS_SCALAR;
          }
        else
          macro_error(ERROR_SYMBOL_UNKNOWN, VarEntry->Name);
        }

      * ReductionSemRec = SemanticStack[ StackTop ];
      }
    break;

    case SIMPTYPE:
      IdentMustExist(&SymbolTable[ SemanticStack[StackTop].Reference ]);
      * ReductionSemRec = SemanticStack[ StackTop ];
    break;

    case LITSTRING:  /* this may change... */
    case CONST:
      * ReductionSemRec = SemanticStack[ StackTop ];
      CompileOperator(SemanticStack[StackTop].Reference);
    break;

    case CURVEST:
      CurveRefClass = CLASS_UNKNOWN;
    break;

    case MEMCRV:
    case DISPCRV:
      {
      int Ref;
  
      if (CurveRefClass == CLASS_UNKNOWN)
        CurveRefClass = CLASS_CURVE;
  
      Ref = CreateConstantSymbol(CLASS_SCALAR, TYPE_INTEGER, INT_TOKX,
                                (void *) &CurveRefClass);
      CompileOperator(Ref);         // constant: curve set reference type
  
      CompileOperator(SemanticStack[StackTop-1].Reference);
      * ReductionSemRec = SemanticStack[ StackTop - 1 ];
      }
    break;

    case CRVSET:
      {
      int Ref;

      if (CurveRefClass == CLASS_UNKNOWN)
        CurveRefClass = CLASS_CURVESET;

      Ref = CreateConstantSymbol(CLASS_SCALAR, TYPE_INTEGER, INT_TOKX,
                                 (void *) &CurveRefClass);
      CompileOperator(Ref);         // constant: curve set reference type
  
      CompileOperator(SemanticStack[StackTop-1].Reference);
      * ReductionSemRec = SemanticStack[ StackTop - 1 ];
      }
    break;
  
#ifdef NEWPLOT
    case PLTSET:
      {
      int Ref, class = CLASS_PLOTREF;
  
      Ref = CreateConstantSymbol(CLASS_SCALAR, TYPE_INTEGER, INT_TOKX,
                                 (void *) &class);
      CompileOperator(Ref);         // constant: plot set reference type
  
      CompileOperator(SemanticStack[StackTop-1].Reference);
      * ReductionSemRec = SemanticStack[ StackTop - 1 ];
      }
    break;
#endif

    case CVS_STARTIND:
    case CVS_COUNT:
    case CVS_NAME:
    case CVS_PATH:
    case CVS_DESC:
    case CVS_DISP:
      CurveRefClass = CLASS_CURVESETHDR;
      CompileOperator(InvalidIndexRef); /* constant: dummy curve index */
      CompileOperator(SemanticStack[StackTop].Reference);
    break;

    case CVS_ONLY:
      CompileOperator(InvalidIndexRef); /* constant: dummy curve index */
    break;

    case CV_ONLY:
      if (CurveRefClass == CLASS_UNKNOWN)
        CurveRefClass = CLASS_CURVE;
    break;
  
    case CV_POINTCNT:
    case CV_TIME:
    case CV_SCMP:
    case CV_TRACK:
    case CV_FRAME:
    case CV_MIN:
    case CV_MAX:
    case CV_XMIN:
    case CV_XMAX:
    case CV_XUNITS:
    case CV_YUNITS:
      CurveRefClass = CLASS_CURVEHDR;

#ifdef NEWPLOT
    case PLT_XMIN:
    case PLT_XMAX:
    case PLT_YMIN :
    case PLT_YMAX :
    case PLT_ZMIN :
    case PLT_ZMAX :
    case PLT_XUNIT:
    case PLT_YUNIT:
    case PLT_ZUNIT:
    case PLT_TITLE:
    case PLT_XLABL:
    case PLT_YLABL:
    case PLT_ZLABL:
    case PLT_ZLABL:
    case PLT_ZXPCT:
    case PLT_ZYPCT:
    case PLT_LNSTY:
    case PLT_AUTOX:
    case PLT_AUTOY:
    case PLT_AUTOZ:
#endif
      CompileOperator(SemanticStack[StackTop].Reference);
    break;

    case CV_XPOINT:
    case CV_YPOINT:
      CurveRefClass = CLASS_POINT;
      CompileOperator(SemanticStack[StackTop-1].Reference);
    break;

    case CTRUE:
      {
      int Ref;
      unsigned int TrueVal = 1;
  
      Ref = CreateConstantSymbol(CLASS_SCALAR, TYPE_BOOLEAN, INT_TOKX,
                                 &TrueVal);
  
      ReductionSemRec->Token = INT_TOKX;
      ReductionSemRec->Type = TYPE_BOOLEAN;
      ReductionSemRec->Reference = Ref;
      }
    break;

    case CFALSE:
      {
      int Ref;
      unsigned int FalseVal = 0;
  
      Ref = CreateConstantSymbol(CLASS_SCALAR, TYPE_BOOLEAN, INT_TOKX,
                                 &FalseVal);
  
      ReductionSemRec->Token = INT_TOKX;
      ReductionSemRec->Type = TYPE_BOOLEAN;
      ReductionSemRec->Reference = Ref;
      }
    break;
    }
}
