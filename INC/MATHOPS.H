/*
/  FILE : mathops.h
/
/  $Header: /OMA4000/Include/MATHOPS.H 1     6/18/99 1:53a Maynard $
/  $Log: /OMA4000/Include/MATHOPS.H $
 * 
 * 1     6/18/99 1:53a Maynard
 *
*/

#ifndef MATHOPS_INCLUDED
#define MATHOPS_INCLUDED

#ifndef _MAX_DRIVE
#include <stdlib.h>
#endif

/* indices tied to math form operand list */
typedef enum { PLUS = 0,
               SUB,
               MULTIPLY,
               DIVIDE,
               LOG,
               LN,
               ABSORB,
               DERIV,
               SMOOTH3,
               SMOOTH5,
               INTDIV,
               MOD,
               TRUNC,
               ROUND,
               ABS,
               AND,
               OR,
               XOR,
               BITNOT,
               SHL,
               SHR,
               EXP,
               SIN,
               COS,
               TAN,
               ATAN,
               ASIN,
               ACOS,
               ATAN2,
               EQUALTO,
               NOTEQUALTO,
               LESSTHAN,
               GREATERTHAN,
               LESSTHANEQ,
               GREATERTHANEQ };

typedef struct op_block {
  CHAR Spec[_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT];
  DOUBLE Constant;   /* holds value if block is not a curve set subset */
  USHORT BlkIndex;   /* curve directory index of block */
  USHORT Start;      /* first curve to use in block */
  USHORT Count;      /* count of curves to use in block */
  BOOLEAN IsSource;  /* way to tell which are source and which is dest */
  } OP_BLOCK;

typedef struct stats {
  DOUBLE Avg;
  DOUBLE Dev;
  DOUBLE Area;
  BOOLEAN DoCArea; // Flag whether to do corrected area
} STAT_STRUCT;

extern double MaxErrorVal;
extern double MinErrorVal;

#endif  //  MATHOPS_INCLUDED
