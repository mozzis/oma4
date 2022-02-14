/* -----------------------------------------------------------------------
/
/  mathform.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/mathform.c_v   1.10  06 Jul 1992 10:32:48   maynard  $
/  $Log:   J:/logfiles/oma4000/main/mathform.c_v  $
*/
  
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
  
#include "mathform.h"
#include "tempdata.h"
#include "di_util.h"
#include "curvedir.h"
#include "mathops.h"
#include "helpindx.h"
#include "ksindex.h"    
#include "fileform.h"
#include "formwind.h"
#include "macres2.h"   // DoBinMath()
#include "syserror.h"  // ERROR_ALLOC_MEM
#include "omaerror.h"
#include "formtabs.h"
#include "omaform.h"  // VerifyFileName()
#include "crventry.h"
#include "crvheadr.h"
#include "curvbufr.h"
#include "forms.h"
#include "points.h"
#include "plotbox.h"  
#include "splitfrm.h"

#include "curvdraw.h"  /* Replot() */
#include "cursor.h"     /* ActiveWindow */

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

/* data for plotmode/menumode switching */

BOOLEAN InMathForm = FALSE;     

DOUBLE MaxErrorVal = MAXFLOAT;
DOUBLE MinErrorVal = MINFLOAT;
USHORT min_count, max_count;

struct op_block Src1  = { "", 0.0, -1, 0, 1, TRUE };
struct op_block Src2  = { "", 0.0, -1, 0, 1, TRUE };
struct op_block Dst   = { "", 0.0, -1, 0, 1, FALSE };
  
// last curve specified.
#define CURLAST(x) (x->Start + x->Count - 1)
  
PRIVATE char * operator_toggle[35]= { "   +  ",   /* PLUS,        */
                                      "   -  ",   /* SUB,         */
                                      "   *  ",   /* MULTIPLY,    */
                                      "   /  ",   /* DIVIDE,      */
                                      " log ",    /* LOG,         */
                                      "  ln  ",   /* LN,          */
                                      "absorb",   /* ABSORB,      */
                                      "deriv",    /* DERIV,       */
                                      "smooth",   /* SMOOTH3,     */
                                      "SMOOTH",   /* SMOOTH5,     */
                                      "INTDIV",   /* INTDIV,      */
                                      " MOD  ",   /* MOD,         */
                                      "TRUNC ",   /* TRUNC,       */
                                      "ROUND ",   /* ROUND,       */
                                      " ABS  ",   /* ABS,         */
                                      " AND  ",   /* AND,         */
                                      "  OR  ",   /* OR,          */
                                      " XOR  ",   /* XOR,         */
                                      "  ~   ",   /* BITNOT,      */
                                      " SHL  ",   /* SHL,         */
                                      " SHR  ",   /* SHR,         */
                                      " EXP  ",   /* EXP,         */
                                      " SIN  ",   /* SIN,         */
                                      " COS  ",   /* COS,         */
                                      " TAN  ",   /* TAN,         */
                                      " ATAN ",   /* ATAN,        */
                                      " ASIN ",   /* ASIN,        */
                                      " ACOS ",   /* ACOS,        */
                                      "ATAN2 ",   /* ATAN2,       */
                                      "   =  ",   /* EQUALTO,     */
                                      "  <>  ",   /* NOTEQUALTO,  */
                                      "   <  ",   /* LESSTHAN,    */
                                      "   >  ",   /* GREATERTHAN, */
                                      "  <=  ",   /* LESSTHANEQ,  */
                                      "  >=  " }; /* GREATERTHANEQ*/

PRIVATE int operator = 0;
static  int dummy_select;
  
static DATA DO_STRING_Reg[] = {
   { "Operand 1",   0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 0
   { "Curve Start", 0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 1
   { "Count",       0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 2
   { "Operator",    0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 3
   { "Operand 2",   0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 4
   { "Result",      0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 5
   { "Calculate",   0, DATATYP_STRING, DATAATTR_PTR, 0 },  // 6
};

PRIVATE SHORT RunMath(void* field_data, char * field_string);
PRIVATE SHORT VerifySrc1Blk(void);
PRIVATE SHORT VerifySrc2Blk(void);
PRIVATE SHORT VerifyCurveCount(void);
PRIVATE BOOLEAN InitMathForm(void);
PRIVATE BOOLEAN ExitMathForm(void);

static EXEC_DATA CODE_Reg[] = {
   { CAST_CHR2INT RunMath, 0, DATATYP_CODE, DATAATTR_PTR, 0 },    /* 0 */
   { VerifySrc1Blk, 0, DATATYP_CODE, DATAATTR_PTR, 0 },           /* 1 */
   { VerifySrc2Blk, 0, DATATYP_CODE, DATAATTR_PTR, 0 },           /* 2 */
   { VerifyWritableFileName, 0, DATATYP_CODE, DATAATTR_PTR, 0 },  /* 3 */
   { VerifyCurveCount, 0, DATATYP_CODE, DATAATTR_PTR, 0 },        /* 4 */
   { InitMathForm, 0, DATATYP_CODE, DATAATTR_PTR, 0 },            /* 5 */
   { ExitMathForm, 0, DATATYP_CODE, DATAATTR_PTR, 0 },            /* 6 */
};
 
static DATA DATA_Reg[] = {

  { &Src1.Spec,  0, DATATYP_STRING, DATAATTR_PTR, 0 }, // 0 file name / const
  { &Src1.Start, 0, DATATYP_INT, DATAATTR_PTR, 0 },    // 1 start curve
  { &Src1.Count, 0, DATATYP_INT, DATAATTR_PTR, 0 },    // 2 curve count
  { &operator,   0, DATATYP_INT, DATAATTR_PTR, 0 },    // 3 operator toggle val
  { &Src2.Spec,  0, DATATYP_STRING, DATAATTR_PTR, 0 }, // 4 op 2 file name / constant
  { &Src2.Start, 0, DATATYP_INT, DATAATTR_PTR, 0 },    // 5 op 2 low range limit
  { &Src2.Count, 0, DATATYP_INT, DATAATTR_PTR, 0 },    // 6 op 2 high range limit
  { &Dst.Spec,   0, DATATYP_STRING, DATAATTR_PTR, 0 }, // 7 result file name
  { &Dst.Start,  0, DATATYP_INT, DATAATTR_PTR, 0 },    // 8 result low range limit
  { &Dst.Count,  0, DATATYP_INT, DATAATTR_PTR, 0 },    // 9 result high range limit
  { &dummy_select, 0, DATATYP_INT, DATAATTR_PTR, 0 },  // 10 dummy calculate variable
};
  
static DATA TOGGLE_Reg[] = {
   { operator_toggle, 0, DATATYP_STRING_ARRAY, DATAATTR_PTR, 0 },   // 0
};
  
// names for indexing into the DataRegistry.
enum DATAREGISTRY_ACCESS { DGROUP_DOSTRING = 1, DGROUP_CODE,
                           DGROUP_DATA,         DGROUP_TOGGLE
};
  
// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// Start of field and form declarations
  
enum {LBL_BLOCK1,
      LBL_CS1,
      LBL_CC1,
      LBL_OPERATOR,
      LBL_BLOCK2,
      LBL_CS2,
      LBL_CC2,
      LBL_RESULT,
      LBL_CS3,
      LBL_CC3,
      FLD_BLOCK1,
      FLD_CS1,
      FLD_CC1,
      FLD_OPERATOR,
      FLD_BLOCK2,
      FLD_CS2,
      FLD_CC2,
      FLD_RESULT,
      FLD_CS3,
      FLD_CC3,
      FLD_CALC
      };

FIELD MathFormFields[] = {

  label_field(LBL_BLOCK1,
              DGROUP_DOSTRING, 0,
              2, 2, 9),
  label_field(LBL_CS1,
              DGROUP_DOSTRING, 1,
              2, 49, 11),
  label_field(LBL_CC1,
              DGROUP_DOSTRING, 2,
              2, 67, 5),
  label_field(LBL_OPERATOR,
              DGROUP_DOSTRING, 3,
              4, 30, 8),
  label_field(LBL_BLOCK2,
              DGROUP_DOSTRING, 4,
              6, 2, 9),
  label_field(LBL_CS2,
              DGROUP_DOSTRING, 1,
              6, 49, 11),
  label_field(LBL_CC2,
              DGROUP_DOSTRING, 2,
              6, 67, 5),
  label_field(LBL_RESULT,
              DGROUP_DOSTRING, 5,
              8, 2, 6),
  label_field(LBL_CS3,
              DGROUP_DOSTRING, 1,
              8, 49, 11),
  label_field(LBL_CC3,
              DGROUP_DOSTRING, 2,
              8, 67, 5),

  field_set(FLD_BLOCK1,
            FLDTYP_STRING,                   /* (0:10) */
            FLDATTR_REV_VID,
            KSI_MATH_OP1_NAME,
            MATHFORM_FIELD_HBASE + 0,
            DGROUP_DATA, 0,
            0, 0,
            DGROUP_CODE, 1,
            80, 0,
            2, 12, 36,
            EXIT, FLD_BLOCK1, FLD_CALC, FLD_OPERATOR,
            FLD_BLOCK1, FLD_CS1, FLD_CALC, FLD_CS1),

  field_set(FLD_CS1,
            FLDTYP_UNS_INT,
            FLDATTR_REV_VID,
            KSI_MATH_OP1_START,              /* (0:11) */
            MATHFORM_FIELD_HBASE + 1,
            DGROUP_DATA, 1,
            0, 0,
            0, 0,
            0, 0,
            2, 61, 5,
            EXIT, FLD_CS1, FLD_CALC, FLD_OPERATOR,
            FLD_BLOCK1, FLD_CC1, FLD_BLOCK1, FLD_CC1),

  field_set(FLD_CC1,
            FLDTYP_UNS_INT,
            FLDATTR_REV_VID,
            KSI_MATH_OP1_COUNT,              /* (0:12) */
            MATHFORM_FIELD_HBASE + 2,
            DGROUP_DATA, 2,
            0, 0,
            DGROUP_CODE, 4,
            0, 0,
            2, 73, 5,
            EXIT, FLD_CC1, FLD_CALC, FLD_OPERATOR,
            FLD_CC1, FLD_CC1, FLD_CS1, FLD_OPERATOR),

  field_set(FLD_OPERATOR,
            FLDTYP_TOGGLE,
            FLDATTR_REV_VID,
            KSI_MATH_OPERATOR,               /* (0:13) */
            MATHFORM_FIELD_HBASE + 3,        /* operator_toggle */
            DGROUP_DATA, 3,
            DGROUP_TOGGLE, 0,
            0, 0,
            0, 35,
            4, 39, 6,
            EXIT, FLD_OPERATOR, FLD_BLOCK1, FLD_BLOCK2,
            FLD_OPERATOR, FLD_OPERATOR, FLD_BLOCK1, FLD_BLOCK2),

  field_set(FLD_BLOCK2,
            FLDTYP_STRING,
            FLDATTR_REV_VID,                 /* (0:14) */
            KSI_MATH_OP2_NAME,
            MATHFORM_FIELD_HBASE + 4,
            DGROUP_DATA, 4,
            0, 0,
            DGROUP_CODE, 2,
            80, 0 ,
            6, 12, 36,
            EXIT, FLD_BLOCK2, FLD_OPERATOR, FLD_RESULT,
            FLD_BLOCK2, FLD_CS2, FLD_OPERATOR, FLD_CS2),

  field_set(FLD_CS2,
            FLDTYP_UNS_INT,
            FLDATTR_REV_VID,
            KSI_MATH_OP2_START,              /* (0:15) */
            MATHFORM_FIELD_HBASE + 5,
            DGROUP_DATA, 5,
            0, 0,
            0, 0,
            0, 0,
            6, 61, 5,
            EXIT, FLD_CS2, FLD_OPERATOR, FLD_RESULT,
            FLD_BLOCK2, FLD_CC2, FLD_BLOCK2, FLD_CC2),

  field_set(FLD_CC2,
            FLDTYP_UNS_INT,
            FLDATTR_REV_VID,
            KSI_MATH_OP2_COUNT,              /* (0:16) */
            MATHFORM_FIELD_HBASE + 6,
            DGROUP_DATA, 6,
            0, 0,
            DGROUP_CODE, 4,
            0, 0,
            6, 73, 5,
            EXIT, FLD_CC2, FLD_CC1, FLD_RESULT,
            FLD_CC2, FLD_CC2, FLD_CS2, FLD_RESULT),

  field_set(FLD_RESULT,
            FLDTYP_STRING,
            FLDATTR_REV_VID,                 /* (0:17) */
            KSI_MATH_RES_NAME,
            MATHFORM_FIELD_HBASE + 7,
            DGROUP_DATA, 7,
            0, 0,
            DGROUP_CODE, 3,
            80, 0 ,
            8, 12, 36,
            EXIT, FLD_RESULT, FLD_BLOCK2, FLD_CALC,
            FLD_RESULT, FLD_CS3, FLD_CC2, FLD_CS3),

  field_set(FLD_CS3,
            FLDTYP_UNS_INT,
            FLDATTR_REV_VID,
            KSI_MATH_RES_START,              /* (0:18) */
            MATHFORM_FIELD_HBASE + 8,
            DGROUP_DATA, 8,
            0, 0,
            0, 0,
            0, 0,
            8, 61, 5,
            EXIT, FLD_CS3, FLD_BLOCK2, FLD_CALC,
            FLD_RESULT, FLD_CS3, FLD_RESULT, FLD_CALC),

  field_set(FLD_CC3,
            FLDTYP_UNS_INT,
            FLDATTR_DISPLAY_ONLY,
            KSI_MATH_RES_COUNT,              /* (0:19) */
            MATHFORM_FIELD_HBASE + 9,
            DGROUP_DATA, 9,
            0, 0,
            0, 0,
            0, 0,
            8, 73, 5,
            FLD_CC3,FLD_CC3,FLD_CC3,FLD_CC3,
            FLD_CC3,FLD_CC3,FLD_CC3, FLD_CC3),

  field_set(FLD_CALC,
            FLDTYP_SELECT,
            FLDATTR_REV_VID,
            KSI_MATH_GO,                     /* (0:20) */
            MATHFORM_FIELD_HBASE + 10,
            DGROUP_DATA, 10,
            DGROUP_DOSTRING, 6 ,
            DGROUP_CODE, 0,
            0, 0,
            10, 35, 9,
            EXIT,FLD_CALC, FLD_RESULT, FLD_BLOCK1,
            FLD_CALC, FLD_CALC, FLD_CC3, FLD_BLOCK1),
};
  
static FORM MathForm =
{
   /* 1st 2 fields are for run-time storage                           */
   0,                          /* int           field_index;          */
   0,                          /* int           previous_field_index; */
   FORMATTR_BORDER |           /* unsigned int  attrib;               */
   FORMATTR_FIRST_CHAR_ERASE | 
   FORMATTR_EXIT_RESTORE |
   FORMATTR_FULLWIDTH |
   FORMATTR_VISIBLE,
   0,                          /* signed char   next_field_offset;    */
   0,                          /* unsigned char exit_key_code;        */
   0,                          /* unsigned char status;               */
   2,0,                        /* unsigned char row, column;          */
   12,                         /* unsigned char size_in_rows;         */
   80,                         /* unsigned char size_in_columns;      */
   /* Next 2 fields are for scrolling forms                           */
   0,                          /* unsigned char display_row_offset;   */
   0,                          /* unsigned int  virtual_row_index;    */
   {DGROUP_CODE, 5},           /* data_index    init_function;        */
   {DGROUP_CODE, 6},           /* data_index    exit_function;        */
   COLORS_DEFAULT,             /* unsigned char color_set_index;      */
   0,                          /* unsigned char string_cursor_offset; */
   0,                          /* unsigned char display_cursor_offset;*/
   0,                          /* unsigned char field_char_count;     */
   0,                          /* unsigned char field_overfull_flag;  */
   sizeof(MathFormFields) / sizeof(MathFormFields[0]) ,
   MathFormFields,             /* FIELD *       fields;               */
   /* Next field is for keystroke record and playback,                */
   /* an index into a file of macro commands                          */
   /* one of which will activate this form                            */
   KSI_MATH_FORM,               /* int           MacFormIndex;        */
   0, DO_STRING_Reg, (DATA *)CODE_Reg, DATA_Reg, TOGGLE_Reg, 0
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN InitMathForm(void)
{
  /* display plot screen *and* Math Form */
  return InitSplitForm(&MathForm, &InMathForm, NULL);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN ExitMathForm(void)
{
  return ExitSplitForm(&InMathForm);
}

/* redraw form after a change which affects more than one field */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void RedoForm(void)
{
  int Index;

  Index = Current.Form->field_index;
  draw_form();
  Current.Form->field_index = Index;
  format_and_display_field(FALSE);
}

/* return TRUE if the selected operator needs only one operand */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN is_single(void)
{
  switch(operator)
    {
    case PLUS:
    case SUB:
    case MULTIPLY:
    case DIVIDE:
    case ABSORB:
    case INTDIV:
    case MOD:
    case AND:
    case OR:
    case XOR:
    case SHL:
    case SHR:
    case EXP:
    case EQUALTO:
    case NOTEQUALTO:
    case LESSTHAN:
    case GREATERTHAN:
    case LESSTHANEQ:
    case GREATERTHANEQ:
      return FALSE;
    case LOG:
    case LN:
    case SMOOTH3:
    case SMOOTH5:
    case DERIV:
    case TRUNC:
    case ROUND:
    case ABS:
    case BITNOT:
    case SIN:
    case COS:
    case TAN:
    case ATAN:
    case ASIN:
    case ACOS:
    case ATAN2:
    default:
      return TRUE;
    }
}

// Automatically calculate the number of curves that should be in the result
// curve set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void MathFixCount(void)
{
  USHORT TestCount;

  if (Src1.Count == 0)                        /* First src must be a curve */
    Src1.Count = 1;
  
  TestCount = Src2.Count;
  
  if (is_single() || (Src2.Count))
    TestCount = 1;
  
  if (TestCount > Src1.Count)
    Dst.Count = TestCount / Src1.Count;
  else if (TestCount)
    Dst.Count = Src1.Count / TestCount;
  else
    Dst.Count = 1;

  return;
}

/* Called by form field entry for on curve count field */
PRIVATE SHORT VerifyCurveCount(void)
{
  MathFixCount();
  display_random_field(&MathForm, FLD_CC1);
  display_random_field(&MathForm, FLD_CC2);
  display_random_field(&MathForm, FLD_CC3);
  return FIELD_VALIDATE_SUCCESS;
}

// verify that the string is a good constant value
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int VerifyConst(CHAR String[])
{
  BOOLEAN Spaces = FALSE;
  BOOLEAN Exp = FALSE;
  BOOLEAN Decimal = FALSE;
  BOOLEAN Sign = FALSE;
  SHORT StrLen, i;
  CHAR *TempPos;

  /* ignore leading spaces */
  StrLen = strlen(String);
  i=0;
  while ((i < StrLen) && (String[i] == ' '))
    i++;

  if (i == StrLen)
    {
    error(ERROR_CURVE_CONST);
    return (i);
    }
  /* check for floating point number */
  /* check for nonnumeric characters */
  for (i=i; i<StrLen; i++)
    {
    if ((String[i] < '0') ||
      (String[i] > '9'))
      {
      /* check for extra spaces */
      if (String[i] == ' ')
        Spaces = TRUE;
      else /* a non-space, non-numeric, or repeat '.' or 'e' */
        {
        if (Spaces) /* intervening spaces not allowed */
          {
          error(ERROR_CURVE_CONST);
          return i;
          }
        else if (String[i] == '.')
          {
          Sign = TRUE;  /* don't want a sign after decimal point */
          if (Decimal)
            {
            error(ERROR_CURVE_CONST);
            return i;
            }
          else
            Decimal = TRUE;
          }
        else if ((String[i] & 0xDF) == 'E')
          {
          Sign = FALSE; /* can do another sign */
          if (Exp)
            {
            error(ERROR_CURVE_CONST);
            return i;
            }
          else
            Exp = TRUE;
          }
        else if ((String[i] == '-') || (String[i] == '+'))
          {
          if (Sign)
            {
            error(ERROR_CURVE_CONST);
            return i;
            }
          else
            Sign = TRUE;
          }
        else /* unrecognized character */
          {
          error(ERROR_CURVE_CONST);
          return i;
          }
        } /* non space or non_numeric */
      }
    }
  sprintf(String, "%-15.10g", (float) atof(String));
  /* strip off trailing spaces */
  if (TempPos = strchr(String, ' '))
    *TempPos = '\0';

  return FIELD_VALIDATE_SUCCESS;
}
  
// verify that a curve name and range is in the curve directory
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE SHORT VerifyCurveBlk(OP_BLOCK * Op)
{
  CHAR Name[_MAX_FNAME+_MAX_EXT];
  CHAR Path[_MAX_DIR+_MAX_DRIVE];
  SHORT OverlapCheck;

  /* expand the file block name and check to see if it is not a directory */
  /* 'special' files have no path */
  if(is_special_name(Op->Spec))
    {
    Path[0] = '\0';
    strcpy(Name, Op->Spec);
    }
  else if (ParsePathAndName(Path, Name, Op->Spec) != 2)
    return FIELD_VALIDATE_WARNING;  /* Illegal file spec */

  /* check to see that the curve range is present */
  OverlapCheck = CheckCurveBlkOverLap(Path, Name, Op->Start, CURLAST(Op),
                                      &Op->BlkIndex);
  switch (OverlapCheck)
    {
    case BADNAME:        /* Not a legal name or not found */
      return FIELD_VALIDATE_WARNING;
    case SPLITRANGE:
      error(ERROR_SPLIT_RANGE, Op->Start, CURLAST(Op));
      return FIELD_VALIDATE_WARNING;
    case RANGEOK:        /* Set is an existing contiguous range - success! */
      strcpy(Op->Spec, Path);
      strcat(Op->Spec, Name);
      return FIELD_VALIDATE_SUCCESS;
    case DISJOINT:
    case OVERLAPCAT:   /* Source exists, but curves specified overlap */
    case NOOVERLAPCAT: /* another curve block */
    default:
      error(ERROR_BAD_CURVE_BLOCK, Op->BlkIndex);
      return FIELD_VALIDATE_WARNING;
    }
  return FIELD_VALIDATE_SUCCESS;
}

// verify that a name is in the curve directory or is a constant
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE SHORT VerifyBlkOrConst(OP_BLOCK * Op)
{
  int ReturnVal;

  Op->Constant = MAXFLOAT;/* set default conditions as flags */
  ReturnVal = VerifyCurveBlk(Op);
  if (ReturnVal != FIELD_VALIDATE_SUCCESS)
    {
    ReturnVal = VerifyConst(Op->Spec);
    if (ReturnVal == FIELD_VALIDATE_SUCCESS)
      {
      Op->Constant = atof(Op->Spec);
      Op->Start = 0;
      Op->Count = 1;
      }
    }

  if (ReturnVal == FIELD_VALIDATE_SUCCESS)
    MathFixCount();
 
  return ReturnVal;
}

// when user enters new string in Operand 1, ensure it is a legal curve set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE SHORT VerifySrc1Blk(void)
{
  SHORT ReturnVal;

  Src2.Start = 0;
  Src2.Count = 1;

  ReturnVal = VerifyBlkOrConst(&Src1);
  if (Src1.BlkIndex != NOT_FOUND && ReturnVal == FIELD_VALIDATE_SUCCESS)
    {
    Src1.Start = MainCurveDir.Entries[Src1.BlkIndex].StartIndex;
    Src1.Count = MainCurveDir.Entries[Src1.BlkIndex].count;
    }
  else
    Src1.Count = 1;

  if (Dst.Count < Src1.Count)
    Dst.Count = Src1.Count;
  
  RedoForm();
  return ReturnVal;
}

// when user enters new string in Operand 2, ensure that it is a legal curve
// set or a constant value
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE SHORT VerifySrc2Blk(void)
{
  SHORT ReturnVal;

  Src2.Start = 0;
  Src2.Count = 1;

  ReturnVal = VerifyBlkOrConst(&Src2);
  if (Src2.BlkIndex != NOT_FOUND && ReturnVal == FIELD_VALIDATE_SUCCESS)
    {
    Src2.Start = MainCurveDir.Entries[Src2.BlkIndex].StartIndex;
    Src2.Count = MainCurveDir.Entries[Src2.BlkIndex].count;
    }

  if (Dst.Count < Src2.Count)
    Dst.Count = Src2.Count;
  RedoForm();
  
  return ReturnVal;
}

/***************************************************************************/
/* Return FALSE if source exists as a contiguous curve block and does not  */
/* overlap another curve block, or if the Spec is a legal constant         */
/* Converts values returned from form routines to TRUE/FALSE               */
/***************************************************************************/

PRIVATE BOOLEAN CheckSrcBlock(OP_BLOCK *Src)
{
  if (VerifyBlkOrConst(Src) != FIELD_VALIDATE_SUCCESS)
    return TRUE;

  return FALSE;
}

/********************************************************************/
/* before calculation is done, make sure that the destination block */
/* can be a legal, contiguous curve set.  OvrlapCk returns info on  */
/* whether the set needs to be created, partially deleted, etc.     */
/* function returns FALSE if curve set could be created or if it    */
/* exists and is acceptable                                         */
/********************************************************************/
PRIVATE BOOLEAN CheckDstBlock(OP_BLOCK *Dst, CHAR * Name, CHAR * Path,
                              SHORT * OvrlapCk)
{
  Dst->Constant = MAXFLOAT;/* set default condition */

  /* expand the file block name and check to see if it is not a directory */
  /* 'special' files have no path */
  if(is_special_name(Dst->Spec))
    {
    Path[0] = '\0';
    strcpy(Name, Dst->Spec);
    }
  else if(ParsePathAndName(Path, Name, Dst->Spec) != 2)
    {
    /* must be a proper name if it is the result curve block */
      error(ERROR_CURVESET_NOT_FOUND, Dst->Spec);
      Current.Form->field_index = FLD_RESULT;
      return TRUE;  /* flag error */
    }

  /* check to see that the curve range is present */
  *OvrlapCk = CheckCurveBlkOverLap(Path, Name, Dst->Start, CURLAST(Dst),
                                   &Dst->BlkIndex);
  switch (*OvrlapCk)
    {
    case BADNAME:  /* if result entry, will create curve block later */
    break;

    case SPLITRANGE:
      error(ERROR_SPLIT_RANGE, Dst->Start, CURLAST(Dst));
      Current.Form->field_index = FLD_CS3;
      return TRUE;

    case DISJOINT:
      error(ERROR_BAD_CURVE_BLOCK, Dst->BlkIndex);
      Current.Form->field_index = FLD_CS3;
      return TRUE;

    case RANGEOK:
    case OVERLAPCAT:
    case NOOVERLAPCAT:
    break;
    }
  return FALSE;
}


/**********************************************************************/
/* See if source blocks can be combined an integral number of times,  */
/* and do another calculation of the result curve count               */
/**********************************************************************/
PRIVATE BOOLEAN AreSrcRangesOK(USHORT *CurveCount, OP_BLOCK ** CompareBlock)
{
  /* Base the result block on the largest valid operand block entry */
  *CompareBlock = &Src1;
  *CurveCount = Src1.Count;

  if (!is_single())
    {
    if (Src2.BlkIndex != NOT_FOUND)
      {
      if (*CurveCount < Src2.Count)
        {
        *CompareBlock = &Src2;
        *CurveCount = Src2.Count;
        }
      /* check that curve ranges may be combined an integral # of times */
      if (*CurveCount % Src1.Count)
        {
        error(ERROR_MATRIX_SIZE);
        Current.Form->field_index = FLD_CS1;
        return FALSE;
        }
      }
    }

  /* check to see if there is a valid operand curve entry */
  if (!*CurveCount)
    {
    error(ERROR_NO_OPERAND_CURVE);
    Current.Form->field_index = FLD_BLOCK1;
    return FALSE;  /* return with error flag */
    }

  /* make sure that result range is correct size */
  if (*CurveCount != Dst.Count)
    {
    error(ERROR_MATRIX_SIZE);
    Current.Form->field_index = FLD_CS3;
    return FALSE;
    }
  return TRUE;
}

/* check out the operands and result curve set, and if all is well, do the */
/* calculation and display the result                                      */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE BOOLEAN RunMathProc(void)
{
  CHAR Name[DOSFILESIZE + 1],
       Path[DOSPATHSIZE + 1];
  USHORT CurveCount, i;
  OP_BLOCK * CompareBlock;
  SHORT OverlapCheck;
  WINDOW * MessageWindow;
  ERR_OMA err = ERROR_NONE;

  /* see if 1st operand is an existing curve, with Start and Count OK */
  if (CheckSrcBlock(&Src1)) // || Src1.BlkIndex == -1)
    {
    Current.Form->field_index = FLD_BLOCK1;
    return TRUE;
    }

  /* If a second operand is needed, see if it is an existing curve, */
  /* with Start and Count OK, or can be a constant value */
  if (!is_single())   
    {
      if (CheckSrcBlock(&Src2))
      {
      Current.Form->field_index = FLD_BLOCK2;
      return TRUE;
      }
    }

  if (Src1.BlkIndex == NOT_FOUND && Src2.BlkIndex == NOT_FOUND)
    {
    Current.Form->field_index = FLD_BLOCK2;
    return TRUE;
    }

  /* See if source ranges can be combined an integer number of times */
  if (!AreSrcRangesOK(&CurveCount, &CompareBlock))
    return TRUE;

  /* see if destination is an legal curve, with Start and Count OK */
  if (CheckDstBlock(&Dst, Name, Path, &OverlapCheck))
    return TRUE;

  /* if destination already exists, may have to delete it */
  if (OverlapCheck == RANGEOK || OverlapCheck == OVERLAPCAT)
    {
    CURVE_ENTRY * pEntry = &MainCurveDir.Entries[Dst.BlkIndex];
    ULONG SrcSz, DstSz;
    int delcurves;

    /* put up overwrite warning */
    if (yes_no_choice_window(BlkOverWritePrompt,0,COLORS_MESSAGE) != YES)
      return FALSE; /* return without error */

    /* get size in bytes of largest source and dest for comparison */
    if(GetWholeCurveSetSz(Dst.BlkIndex, Dst.Start, Dst.Count, &DstSz, FALSE))
      return TRUE;
    if(GetWholeCurveSetSz(CompareBlock->BlkIndex, CompareBlock->Start,
                          CompareBlock->Count, &SrcSz, FALSE))
      return TRUE;

    /* calculate how many curves to delete */
    if (CURLAST((&Dst)) >= (pEntry->StartIndex + pEntry->count))
      delcurves = pEntry->StartIndex + pEntry->count - Dst.Start;
    else
      delcurves = Dst.Count;

    /* but if source and dest are same size, then don't delete */
    if (DstSz == SrcSz)
      delcurves = 0, CurveCount = 0;

    /* delete curves which are to be overwritten */
    if (DelMultiTempCurve(&MainCurveDir, Dst.BlkIndex, Dst.Start, delcurves))
      return TRUE;
    }

  put_up_message_window(BusyWorkingEsc, COLORS_MESSAGE, &MessageWindow);

  /* if the result curve set did not exist, create it */
  if (OverlapCheck == BADNAME)
    {
    Dst.Start = 0;
    Dst.Count = CurveCount;

    if (CreateTempFileBlk(&MainCurveDir, &Dst.BlkIndex, Name,
                          Path, "", 0, 0L, 0, NULL, OMA4DATA, 0))
      return TRUE;
    }

  if (CurveCount)
    {
    CURVEHDR *pCurvehdr = calloc(1, sizeof(CURVEHDR));
    if (pCurvehdr == NULL)
      {
      error(ERROR_ALLOC_MEM);
      return TRUE;
      }

    /* add curves as needed to the result curve block */
    for (i = 0; i < CurveCount; i++)
      {
      if (err = ReadTempCurvehdr(&MainCurveDir, CompareBlock->BlkIndex,
                                CompareBlock->Start + i, pCurvehdr))
        break;  /* if err, quit loop, free curvehdr, and return */

      pCurvehdr->DataType = FLOATTYPE;
      pCurvehdr->CurveCount = 1;
      pCurvehdr->MemData = FALSE;

      if (err = AddCurveSpaceToTempBlk(&MainCurveDir, Dst.BlkIndex,
        Dst.Start + i, 1, pCurvehdr))
        break;  /* if err, quit loop, free curvehdr, and return */
      }
    free(pCurvehdr);
    }

  if (!err)
    {
    clearAllCurveBufs();    /* in case live data being sourced */

    if (operator == DERIV)
      DoBinom(&Src1, &Dst, 0);
    else if (operator == SMOOTH3)
      DoBinom(&Src1, &Dst, 1);
    else if (operator == SMOOTH5)
      DoBinom(&Src1, &Dst, 2);
    else
      {
      /* convert to old operand representation for DoBinMath */
      SHORT Blocks[3] =  { Src1.BlkIndex, Src2.BlkIndex, Dst.BlkIndex };
      USHORT Starts[3] = { Src1.Start, Src2.Start, Dst.Start };
      USHORT Counts[3] = { Src1.Count, Src2.Count, Dst.Count };
      DOUBLE Constant[2] = { Src1.Constant, Src2.Constant };

      DoBinMath(&MainCurveDir, Blocks, Starts, Counts, Constant, operator, TRUE);
      }

    /* don't display sources in CAL_PLOTBOX anymore, but do display result */
    MainCurveDir.Entries[Src1.BlkIndex].DisplayWindow &= ~(1 << CAL_PLOTBOX);
    if (Src2.BlkIndex != NOT_FOUND)
      MainCurveDir.Entries[Src2.BlkIndex].DisplayWindow &= ~(1 << CAL_PLOTBOX);
    MainCurveDir.Entries[Dst.BlkIndex].DisplayWindow |= (1 << CAL_PLOTBOX);
    }

  if (MessageWindow != NULL)
    release_message_window(MessageWindow);

  if (!err)
    Replot(ActiveWindow);  /* display the result curve */

  return (err != 0);
}

/* respond to the Calculate control on the math form */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE int RunMath(void* field_data, char * field_string)
{
   if (RunMathProc())
      return FIELD_VALIDATE_WARNING;
   else
      return FIELD_VALIDATE_SUCCESS;
}

// put & MathForm into FormTable[]
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void registerMathForm(void)
{
   FormTable[KSI_MATH_FORM] = &MathForm;
}
