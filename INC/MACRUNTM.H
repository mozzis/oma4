/* -----------------------------------------------------------------------
/
/  macruntm.h
/
/  Written by: TLB      Version 1.00      25 February    1990
/
/  $Header: /OMA4000/Include/MACRUNTM.H 1     6/18/99 1:53a Maynard $
/
/ ----------------------------------------------------------------------- */
  
#ifndef MACRUNTM_INCLUDED
#define MACRUNTM_INCLUDED

#include <stdio.h>  // FILE

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

#define PARSE_COMPLETE  0
#define PARSE_START     1
#define PARSE_ACTIVE    2
#define PARSE_ERROR     3
  
/* Size of code space - maximum number of operators & operands compiled */
#define     DEFAULT_COMPILE_TOKEN_MAX        3000 /* was 1000 - mlm */
  
/* Size of Symbol Table - maximum number of identifiers */
#define     DEFAULT_MAX_SYMBOLS              3500 /* was 1500 - mlm */
  
/* Identifier names cannot be longer than this */
#define     MAX_IDENT_LEN                    20
  
/* Space in bytes for declared (user) strings and constant strings */
#define     DEFAULT_STRING_TABLE_SIZE        6144
  
/* The maximum depth for compile-time stack used by the parser */
#define     PARSER_STACK_SIZE                80
  
/* The most declarations for each type that can precede a single
": TYPE" construction */
#define     MAX_DECLARATIONS_OF_TYPE         20
  
/* until something better is done... */
#define     DEFAULT_STRING_LEN               80
  
#define     INVALID_INDEX     (-1)
#define     EOS '\0'          /* marks end of string */
#define     EOFCH  (-1)       /* marks end of file */
#define     EOLCH '\0'        /* end of line character */
#define     STRING_QUOTE '\'' /* character delimiting quoted strings */
  
#define allocate(ptr, type, count) \
((ptr = (type *) malloc(sizeof(type) * count)) != NULL)
  
#define erase(ptr, type, count) memset(ptr, 0, (sizeof(type) * count))
  
#define forget(ptr) if (ptr != NULL)   free(ptr);ptr = NULL;
  
#define set_bit(bit_var, bit) (bit_var |= (bit))
#define reset_bit(bit_var, bit) (bit_var &= ~(bit))
  
/* xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
  
#define     POINTER_TO              0x80

#define     TYPE_INTEGER            SHORTTYPE
#define     TYPE_WORD               USHORTTYPE
#define     TYPE_LONG_INTEGER       LONGTYPE
#define     TYPE_REAL4              FLOATTYPE
#define     TYPE_REAL               DOUBLETYPE
#define     TYPE_BYTE               UCHARTYPE
#define     TYPE_BOOLEAN            USHORTTYPE
/* #define     TYPE_BOOLEAN   0x11 */
  
#define     TYPE_CURVE              0x40
  
#define     TYPE_STRING             0x41
  
#define     TYPE_PROCEDURE          0x42
#define     TYPE_USER_PROCEDURE     0x43
  
#define     TYPE_FILE               0x44
#define     TYPE_TEXTFILE           0x45

#ifdef      NEWPLOT
#define     TYPE_PLOT               0x50
#endif
  
#define     TYPE_DONT_CARE    0     /* to prevent automatic data conversion */
#define     TYPE_NEWLY_CREATED      (-1)
  
  
/* xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
  
#define     CLASS_UNKNOWN                 0
  
#define     CLASS_EXECUTABLE              1
#define     CLASS_EXECUTE_RETURN          2
#define     CLASS_BRANCH                  3
#define     CLASS_COND_BRANCH             4
  
#define     CLASS_SCALAR                  5
#define     CLASS_ARRAY                   6
#define     CLASS_STRING                  7
#define     CLASS_CURVESET                8
#define     CLASS_CURVESETHDR             9
#define     CLASS_CURVE                  10
#define     CLASS_CURVEHDR               11
#define     CLASS_POINT                  12
#define     CLASS_FILE                   13
#define     CLASS_TEXTFILE               14
#ifdef NEWPLOT
#define     CLASS_PLOTREF                15
#endif
  
/* xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
// for CURVE_REF.Point_Or_HeaderItem

#define     CURVE_POINTY            0
#define     CURVE_POINTX            1
#define     CURVE_START_INDEX       2
#define     CURVE_COUNT             3
#define     CURVE_NAME              4
#define     CURVE_PATH              5
#define     CURVE_DESC              6
#define     CURVE_DISPLAY           7
#define     CURVE_POINT_COUNT       8
#define     CURVE_TIME              9
#define     CURVE_YMIN              10
#define     CURVE_YMAX              11
#define     CURVE_XMIN              12
#define     CURVE_XMAX              13
#define     CURVE_XUNITS            14
#define     CURVE_YUNITS            15
#define     CURVE_SCMP              16
#define     CURVE_FRAME             17
#define     CURVE_TRACK             18


#ifdef NEWPLOT
// for PLOT_REF.Item
#define     PLOT_XMIN               19
#define     PLOT_XMAX               20
#define     PLOT_YMIN               21
#define     PLOT_YMAX               22
#define     PLOT_ZMIN               23
#define     PLOT_ZMAX               24
#define     PLOT_XUNIT              25
#define     PLOT_YUNIT              26
#define     PLOT_ZUNIT              27
#define     PLOT_TITLE              28
#define     PLOT_XLABL              29
#define     PLOT_YLABL              30
#define     PLOT_ZLABL              31
#define     PLOT_ZLABL              32
#define     PLOT_ZXPCT              33
#define     PLOT_ZYPCT              34
#define     PLOT_LNSTY              35
#define     PLOT_AUTOX              36
#define     PLOT_AUTOY              37
#define     PLOT_AUTOZ              38
#endif

  
/* xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
  
#define     CURVESET_CS             1
#define     CURVESET_DC             2
#define     CURVESET_MC             3
  
/* xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
  
#define     SYMATTR_NONE               0
#define     SYMATTR_CONSTANT           0x01  /* cannot be LValue */
#define     SYMATTR_RETURNS_VALUE      0x02  /* can be RValue */
#define     SYMATTR_                   0x04  /*  */
  
/* xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
  
typedef SHORT DECL_INDEX;
typedef USHORT INDEX;
  
/* xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
/* Instruction Stream Data Structures */
  
typedef struct oper {
   UCHAR Class;
   INDEX Reference;
   USHORT LineNumber;
} OPERATOR;
  
typedef struct {
   UCHAR Class;
   int JumpOffset;
} BRANCH_OPERATOR;
  
/* xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
/* Semantic Stack Data Structure */
  
typedef struct semrec
{
   int Type;
   int Token;
   DECL_INDEX     Reference;
} SemanticRecord;
 
/* xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */
  
typedef struct {
   int                  ReferenceType;
   INDEX                CurveSetIndex;
   INDEX                CurveIndex;
   INDEX                PointIndex;
   INDEX                Point_Or_HeaderItem;
} CURVE_REF;

extern OPERATOR *          IP;
extern OPERATOR *          MainRoutine;
extern OPERATOR *          CodeSpace;
extern long                CodeSpaceSize;
extern OPERATOR *          EndOfProgram;
extern char *              StringTable;
extern INDEX               StringTableFreeOffset;
extern INDEX               StringTableSize;
extern int                 Radix;
extern char *              ParseLine;
extern int                 CurrentLine;
extern int                 TokenOffset;
extern int                 ErrorOffset;
extern int                 NewDeclarations[];
extern int                 DeclarationsOfType;
extern DECL_INDEX          MostRecentExecution;
extern DECL_INDEX          MainProgramReference;
extern int                 CurveRefClass;
extern int                 IOListLength;
extern int                 FunctionParameterCount;
extern BOOLEAN             Compiling;
extern double              ForCount;
extern DECL_INDEX          FORVariableReference;
extern DECL_INDEX          FORCountReference;
extern DECL_INDEX          InvalidIndexRef;
extern SemanticRecord *    SemanticStack;
extern int *               StateStack;
extern int                 StackTop;
extern int                 MacroStatus;
extern BOOLEAN             MacroRunProgram;
  
void SetErrorFlag(void);
void macro_error(int errnum, char * id);
int sizeof_Type(int Type);
void ResetIP(void);
void RunProgram(OPERATOR * Initial_IP);
void RunMainProgram(void);
BOOLEAN EnterMacroForm(void);
void LeaveMacroForm(void);
int checkMacroOpen(void);

#endif  // MACRUNTM_INCLUDED
