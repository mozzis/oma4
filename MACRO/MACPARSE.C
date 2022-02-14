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
  / noname.c
/
/  Written by: TLB      Version 1.00
/  Worked on:  TLB      Version 1.01
/
/  .c is a .
/
/ ----------------------------------------------------------------------- */

BOOLEAN debug_flag = FALSE;

    int StateToIndex[]={0, 0, 1, 1, 2, 1, 4, 5, 6, 7, 4, 6, 6, 8, 6, 8, 9, 6, 
    6, 10, 9, 21, 22, 23, 24, 25, 26, 27, 25, 27, 28, 27, 31, 27, 32, 33, 27, 
    27, 27, 46, 27, 27, 47, 58, 27, 59, 27, 60, 63, 64, 27, 64, 27, 64, 68, 69
    , 70, 74, 75, 76, 69, 68, 74, 68, 74, 78, 68, 74, 74, 74, 80, 75, 84, 84, 
    94, 84, 94, 84, 94, 94, 84, 97, 98, 84, 84, 105, 106, 114, 106, 115, 116, 
    106, 117, 118, 136, 118, 137, 138, 139, 140, 141, 0, 0, 142, 0, 143, 144, 
    2, 145, 150, 7, 151, 152, 153, 160, 161, 162, 163, 164, 165, 166, 167, 168
    , 169, 170, 171, 172, 84, 173, 174, 84, 176, 177, 178, 174, 174, 179, 180
    , 181, 28, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194
    , 195, 196, 197, 198, 199, 209, 210, 211, 212, 213, 199, 153, 214, 215, 
    199, 216, 217, 218, 219, 220, 221, 222, 223, 224, 227, 145, 228, 0, 0, 2, 
    10, 15, 17, 21, 23, 25, 28, 30, 44, 48, 50, 52, 54, 56, 58, 60, 62, 65, 69
    , 0, 0, 73, 77, 80, 82, 84, 87, 89, 91, 93, 95, 99, 101, 103, 106, 108, 
    111, 115, 118, 121, 124, 0, 128, 130, 132, 135, 137, 149, 151, 160, 137, 
    137, 132, 149, 137, 171, 160, 160, 172, 149, 174, 179, 137, 160, 137, 137
    , 137, 182, 184, 184, 184, 137, 184, 160, 184, 137, 184, 160, 160, 137, 
    160, 193, 184, 184, 184, 137, 137, 195, 184, 137, 160, 198, 137, 160, 137
    , 58, 206, 137, 208, 210, 212, 215, 223, 225, 229, 231, 240, 246, 252, 256
    , 259, 261, 265, 271, 280, 282, 284, 289, 292, 296, 302, 307, 312, 316, 
    320, 324, 326, 330, 334, 338, 342, 346, 354, 358, 362, 366, 372, 381, 385
    , 394, 396, 400, 404, 408, 412, 416, 418, 421, 425, 429, 431, 440, 444, 
    450, 459, 463, 465, 467, 470};

    int ReductionProdFlag[]={0, 87, 7, 57, 0, 34, 39, 38, 18, 0, 64, 12, 15, 
    53, 17, 52, 0, 13, 14, 71, 50, 74, 0, 76, 67, 73, 77, 23, 72, 25, 0, 24, 
    69, 19, 0, 56, 26, 28, 27, 70, 21, 20, 0, 68, 30, 98, 32, 2, 0, 65, 29, 88
    , 31, 89, 0, 63, 0, 55, 93, 99, 62, 0, 47, 8, 54, 90, 5, 46, 37, 61, 0, 3
    , 101, 49, 59, 75, 78, 4, 58, 35, 42, 102, 60, 33, 79, 92, 83, 36, 0, 44, 
    40, 6, 100, 11, 81, 97, 43, 96, 41, 45, 95, 84, 85, 9, 86, 96, 51, 0, 0, 0
    , 16, 10, 0, 0, 0, 10, 0, 10, 10, 0, 0, 10, 0, 0, 10, 0, 10, 48, 0, 1, 82
    , 10, 0, 10, 0, 91, 10, 10, 10, 22, 10, 10, 10, 10, 10, 0, 10, 10, 10, 10
    , 0, 10, 0, 10, 10, 0, 10, 0, 10, 10, 10, 10, 10, 66, 80, 10, 10, 103, 0, 
    10, 10, 0, 10, 0, 10, 10, 94, 0, 0, 10};

    int ReductionStackPopCount[]={0, 1, 2, 2, 2, 2, 1, 1, 1, 3, 1, 1, 1, 1, 1
    , 3, 2, 1, 1, 2, 4, 1, 3, 2, 1, 1, 4, 1, 3, 1, 3, 1, 6, 1, 2, 1, 1, 1, 1, 
    2, 1, 1, 3, 5, 1, 1, 1, 3, 2, 2, 2, 2, 2, 2, 1, 1, 3, 1, 1, 4, 3, 1, 1, 1
    , 1, 1, 1, 1, 1, 1, 3, 6, 4, 6, 3, 1, 3, 3, 3, 3, 8, 1, 2, 8, 4, 1, 1, 1, 
    3, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 3, 1, 1, 2, 1, 1, 0, 1, 1, 1, 1, 0, 0
    , 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 3, 4, 0, 0, 0, 1, 3, 0, 
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0
    , 0, 3, 3, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 0};

    int ExpectedTokens[]={80, 0, 28, 31, 32, 47, 52, 62, 80, 0, 35, 51, 65, 76
    , 0, 20, 0, 51, 76, 81, 0, 11, 0, 17, 0, 11, 68, 0, 7, 0, 41, 43, 45, 53, 
    61, 64, 66, 73, 74, 75, 77, 78, 79, 0, 34, 51, 76, 0, 11, 0, 17, 0, 17, 0
    , 27, 0, 11, 0, 17, 0, 10, 0, 2, 5, 0, 34, 51, 76, 0, 34, 51, 76, 0, 2, 51
    , 76, 0, 17, 26, 0, 17, 0, 11, 0, 27, 54, 0, 80, 0, 18, 0, 81, 0, 50, 0, 
    51, 76, 81, 0, 17, 0, 10, 0, 11, 20, 0, 1, 0, 11, 37, 0, 51, 63, 76, 0, 2
    , 5, 0, 11, 37, 0, 5, 9, 0, 29, 30, 44, 0, 1, 0, 55, 0, 38, 48, 0, 7, 0, 1
    , 6, 17, 18, 19, 21, 39, 49, 59, 60, 67, 0, 69, 0, 4, 6, 12, 13, 14, 22, 
    23, 24, 0, 17, 27, 40, 42, 56, 57, 58, 70, 71, 72, 0, 0, 36, 0, 3, 8, 33, 
    46, 0, 51, 76, 0, 7, 0, 1, 6, 17, 18, 19, 21, 39, 67, 0, 25, 0, 4, 6, 0, 1
    , 17, 18, 19, 21, 39, 67, 0, 80, 0, 1, 0, 55, 0, 38, 48, 0, 2, 5, 11, 20, 
    36, 37, 68, 0, 7, 0, 29, 30, 44, 0, 69, 0, 4, 6, 12, 13, 14, 22, 23, 24, 0
    , 11, 20, 29, 30, 44, 0, 2, 5, 29, 30, 44, 0, 29, 30, 44, 0, 38, 48, 0, 69
    , 0, 29, 30, 44, 0, 11, 37, 29, 30, 44, 0, 11, 20, 36, 37, 68, 29, 30, 44
    , 0, 36, 0, 27, 0, 3, 8, 33, 46, 0, 51, 76, 0, 29, 30, 44, 0, 11, 37, 29, 
    30, 44, 0, 3, 8, 33, 46, 0, 3, 8, 33, 46, 0, 29, 30, 44, 0, 29, 30, 44, 0
    , 29, 30, 44, 0, 7, 0, 29, 30, 44, 0, 29, 30, 44, 0, 29, 30, 44, 0, 29, 30
    , 44, 0, 29, 30, 44, 0, 11, 20, 37, 68, 29, 30, 44, 0, 29, 30, 44, 0, 29, 
    30, 44, 0, 29, 30, 44, 0, 11, 68, 29, 30, 44, 0, 11, 20, 36, 37, 68, 29, 
    30, 44, 0, 29, 30, 44, 0, 11, 20, 36, 37, 68, 29, 30, 44, 0, 25, 0, 29, 30
    , 44, 0, 29, 30, 44, 0, 29, 30, 44, 0, 29, 30, 44, 0, 29, 30, 44, 0, 25, 0
    , 4, 6, 0, 29, 30, 44, 0, 29, 30, 44, 0, 25, 0, 11, 20, 36, 37, 68, 29, 30
    , 44, 0, 29, 30, 44, 0, 2, 5, 29, 30, 44, 0, 11, 20, 36, 37, 68, 29, 30, 
    44, 0, 29, 30, 44, 0, 17, 0, 80, 0, 51, 76, 0, 29, 30, 44, 0};

    int NextStateFromToken[]={312, 0, 11, 17, 18, 12, 14, 8, 312, 0, 98, 301, 
    96, 305, 0, 16, 0, 301, 305, 56, 0, 292, 0, 23, 0, 314, 320, 0, 26, 0, 33
    , 41, 40, 27, 31, 29, 36, 202, 38, 37, 44, 203, 46, 0, 92, 301, 305, 0, 
    298, 0, 39, 0, 97, 0, 302, 0, 43, 0, 55, 0, 307, 0, 22, 294, 0, 89, 301, 
    305, 0, 90, 301, 305, 0, 88, 301, 305, 0, 65, 209, 0, 60, 0, 59, 0, 24, 
    193, 0, 210, 0, 211, 0, 212, 0, 214, 0, 301, 305, 100, 0, 65, 0, 326, 0, 
    314, 20, 0, 335, 0, 314, 32, 0, 301, 85, 305, 0, 84, 286, 0, 314, 42, 0, 
    206, 205, 0, 224, 181, 180, 0, 21, 0, 187, 0, 6, 10, 0, 182, 0, 337, 334, 
    281, 61, 54, 35, 66, 309, 310, 311, 63, 0, 45, 0, 315, 317, 57, 64, 69, 68
    , 62, 67, 0, 281, 302, 194, 291, 101, 102, 94, 81, 104, 1, 0, 0, 87, 0, 
    323, 324, 330, 325, 0, 301, 305, 0, 190, 0, 337, 334, 281, 61, 54, 35, 66
    , 63, 0, 306, 0, 315, 317, 0, 337, 281, 61, 54, 35, 66, 63, 0, 341, 0, 225
    , 105, 226, 106, 227, 227, 107, 108, 108, 108, 108, 108, 108, 108, 109, 
    228, 110, 111, 111, 111, 229, 230, 112, 231, 231, 231, 231, 231, 231, 231
    , 231, 113, 114, 114, 115, 115, 115, 232, 116, 116, 117, 117, 117, 233, 
    118, 118, 118, 234, 235, 235, 119, 236, 120, 121, 121, 121, 237, 123, 123
    , 124, 124, 124, 239, 125, 125, 125, 125, 125, 126, 126, 126, 240, 241, 
    127, 128, 242, 243, 243, 243, 243, 129, 244, 244, 130, 131, 131, 131, 245
    , 132, 132, 133, 133, 133, 246, 243, 243, 243, 243, 134, 243, 243, 243, 
    243, 135, 136, 136, 136, 247, 137, 137, 137, 248, 138, 138, 138, 249, 250
    , 139, 140, 140, 140, 251, 141, 141, 141, 252, 142, 142, 142, 253, 143, 
    143, 143, 254, 144, 144, 144, 255, 145, 145, 145, 145, 146, 146, 146, 256
    , 147, 147, 147, 257, 148, 148, 148, 258, 149, 149, 149, 259, 150, 150, 
    151, 151, 151, 260, 152, 152, 152, 152, 152, 153, 153, 153, 261, 154, 154
    , 154, 262, 155, 155, 155, 155, 155, 156, 156, 156, 263, 264, 157, 158, 
    158, 158, 265, 159, 159, 159, 266, 160, 160, 160, 267, 161, 161, 161, 268
    , 162, 162, 162, 269, 264, 163, 270, 270, 164, 165, 165, 165, 271, 166, 
    166, 166, 272, 264, 167, 168, 168, 168, 168, 168, 169, 169, 169, 273, 170
    , 170, 170, 274, 171, 171, 172, 172, 172, 275, 173, 173, 173, 173, 173, 
    174, 174, 174, 276, 175, 175, 175, 277, 278, 176, 279, 177, 244, 244, 178
    , 179, 179, 179, 280};

    int PossibleStatesAfterReduction[]={0, 0, 227, 0, 0, 0, 0, 0, 0, 0, 276, 
    273, 263, 261, 260, 256, 246, 240, 239, 232, 0, 0, 0, 0, 0, 0, 0, 0, 181, 
    182, 0, 0, 0, 265, 266, 275, 268, 237, 233, 229, 267, 271, 274, 257, 259, 
    0, 0, 246, 260, 240, 239, 195, 256, 261, 263, 273, 276, 0, 0, 0, 245, 247
    , 0, 0, 245, 247, 248, 0, 0, 0, 224, 202, 203, 0, 0, 0, 278, 0, 214, 0, 
    246, 260, 239, 0, 246, 260, 240, 239, 256, 261, 263, 273, 276, 0, 257, 259
    , 0, 0, 265, 266, 267, 271, 257, 259, 0, 0, 265, 266, 267, 271, 274, 257, 
    259, 0, 0, 0, 0, 0, 276, 273, 263, 261, 260, 256, 246, 240, 239, 232, 265
    , 266, 267, 271, 274, 257, 259, 0, 0, 0, 0, 0, 0, 0, 0, 0, 226, 275, 229, 
    233, 237, 0, 0, 229, 230, 251, 252, 253, 245, 247, 248, 0, 232, 232, 233, 
    233, 234, 235, 236, 237, 238, 239, 239, 240, 240, 242, 255, 0, 245, 246, 
    246, 247, 248, 249, 251, 252, 253, 254, 255, 256, 256, 257, 258, 259, 260
    , 260, 261, 261, 262, 263, 263, 234, 249, 258, 277, 280, 254, 262, 269, 
    272, 0, 265, 266, 267, 268, 269, 271, 272, 273, 273, 274, 275, 275, 276, 
    276, 277, 242, 236, 0, 279, 280};

    int StateAfterReduction[]={217, 103, 4, 293, 283, 0, 9, 2, 220, 7, 75, 75
    , 75, 75, 75, 75, 75, 75, 75, 75, 86, 290, 19, 186, 295, 199, 184, 30, 5, 
    9, 3, 189, 208, 74, 76, 284, 284, 284, 284, 284, 78, 79, 82, 299, 304, 303
    , 192, 221, 188, 297, 218, 196, 70, 72, 73, 80, 83, 216, 34, 197, 328, 332
    , 322, 338, 328, 332, 47, 322, 91, 222, 285, 50, 52, 308, 313, 207, 48, 
    338, 71, 58, 221, 188, 218, 216, 221, 188, 297, 218, 70, 72, 73, 80, 83, 
    216, 299, 304, 303, 316, 74, 76, 78, 79, 299, 304, 303, 296, 74, 76, 78, 
    79, 82, 299, 304, 303, 321, 333, 336, 319, 215, 215, 215, 215, 215, 215, 
    215, 215, 215, 215, 74, 76, 78, 79, 82, 299, 304, 303, 318, 327, 99, 331, 
    198, 95, 93, 339, 287, 13, 15, 25, 28, 77, 303, 223, 289, 49, 51, 53, 328
    , 332, 47, 322, 216, 223, 199, 223, 223, 293, 122, 223, 208, 218, 223, 297
    , 223, 195, 329, 288, 223, 221, 223, 223, 223, 223, 223, 223, 223, 223, 
    223, 70, 223, 223, 223, 223, 188, 223, 72, 223, 223, 73, 223, 219, 183, 
    191, 204, 213, 185, 300, 200, 201, 340, 223, 223, 223, 223, 223, 223, 223
    , 80, 223, 223, 220, 223, 83, 223, 223, 195, 122, 289, 95, 223};


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

  while ((Uch == '=') || 
  (Uch == '>'))
   {
      Symbol[SymbolLength++] = Uch;
      nextch();
      Uch = toupper(ch);
   }
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

    case '(': GetSingletToken(1); break;
    case ')': GetSingletToken(2); break;
    case '*': GetSingletToken(3); break;
    case '+': GetSingletToken(4); break;
    case ',': GetSingletToken(5); break;
    case '-': GetSingletToken(6); break;
    case '.': GetSingletToken(7); break;
    case '/': GetSingletToken(8); break;
    case ';': GetSingletToken(11); break;
    case '=': GetSingletToken(22); break;
    case '[': GetSingletToken(80); break;
    case ']': GetSingletToken(81); break;
    case '$': Radix=16; nextch(); GetNumberConstantToken(); break;
    case ':':
    case '<':
    case '>':
      if (!GetOddballToken())
        {
        string_from_character(ch, char_str);
        macro_error(ERROR_SYNTAX, char_str);
        GetNextToken();                 // try again
        };
      break;
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

  
    InitSymbolTokenValue(":", 9);
    InitSymbolTokenValue(":=", 10);
    InitSymbolTokenValue("<", 12);
    InitSymbolTokenValue("<=", 13);
    InitSymbolTokenValue("<>", 14);
    InitSymbolTokenValue(">", 23);
    InitSymbolTokenValue(">=", 24);
    InitSymbolTokenValue("AND", 25);
    InitSymbolTokenValue("ARRAY", 26);
    InitSymbolTokenValue("BEGIN", 27);
    InitSymbolTokenValue("COUNT", 28);
    InitSymbolTokenValue("CS", 29);
    InitSymbolTokenValue("DC", 30);
    InitSymbolTokenValue("DESC", 31);
    InitSymbolTokenValue("DISPLAY", 32);
    InitSymbolTokenValue("DIV", 33);
    InitSymbolTokenValue("DO", 34);
    InitSymbolTokenValue("DOWNTO", 35);
    InitSymbolTokenValue("ELSE", 36);
    InitSymbolTokenValue("END", 37);
    InitSymbolTokenValue("EXPLICIT", 38);
    InitSymbolTokenValue("FALSE", 39);
    InitSymbolTokenValue("FOR", 40);
    InitSymbolTokenValue("FRAME", 41);
    InitSymbolTokenValue("IF", 42);
    InitSymbolTokenValue("MAX", 43);
    InitSymbolTokenValue("MC", 44);
    InitSymbolTokenValue("MIN", 45);
    InitSymbolTokenValue("MOD", 46);
    InitSymbolTokenValue("NAME", 47);
    InitSymbolTokenValue("NOEXPLICIT", 48);
    InitSymbolTokenValue("NOT", 49);
    InitSymbolTokenValue("OF", 50);
    InitSymbolTokenValue("OR", 51);
    InitSymbolTokenValue("PATH", 52);
    InitSymbolTokenValue("POINT_COUNT", 53);
    InitSymbolTokenValue("PROCEDURE", 54);
    InitSymbolTokenValue("PROGRAM", 55);
    InitSymbolTokenValue("READ", 56);
    InitSymbolTokenValue("READLN", 57);
    InitSymbolTokenValue("REPEAT", 58);
    InitSymbolTokenValue("SHL", 59);
    InitSymbolTokenValue("SHR", 60);
    InitSymbolTokenValue("SRC_COMP", 61);
    InitSymbolTokenValue("START_INDEX", 62);
    InitSymbolTokenValue("THEN", 63);
    InitSymbolTokenValue("TIME", 64);
    InitSymbolTokenValue("TO", 65);
    InitSymbolTokenValue("TRACK", 66);
    InitSymbolTokenValue("TRUE", 67);
    InitSymbolTokenValue("UNTIL", 68);
    InitSymbolTokenValue("VAR", 69);
    InitSymbolTokenValue("WHILE", 70);
    InitSymbolTokenValue("WRITE", 71);
    InitSymbolTokenValue("WRITELN", 72);
    InitSymbolTokenValue("X", 73);
    InitSymbolTokenValue("XMAX", 74);
    InitSymbolTokenValue("XMIN", 75);
    InitSymbolTokenValue("XOR", 76);
    InitSymbolTokenValue("XUNITS", 77);
    InitSymbolTokenValue("Y", 78);
    InitSymbolTokenValue("YUNITS", 79);
  for (i=0; i < NumberOfSymbolEntries; i++)
    if (SymbolTable[i].Token == 0)
      SymbolTable[i].Token = IDENT_TOKX;
}
