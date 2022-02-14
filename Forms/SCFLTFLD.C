/* -----------------------------------------------------------------------
/
/  scfltfld.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00        1-8 May         1988
/  Worked on:  TLB      Version 1.01
/
/
/  (fltfld.c must also be included with program to use this file.)
/
  $Header:   J:/logfiles/forms/scfltfld.c_v   1.4   28 Aug 1991 18:09:04   cole  $
  $Log:   J:/logfiles/forms/scfltfld.c_v  $
 * 
 *    Rev 1.4   28 Aug 1991 18:09:04   cole
 * delete include for omaform.h
 * 
 *    Rev 1.3   29 May 1991 17:32:28   maynard
 * Fix float conversions by only doing single or double type of math, 
 * as appropriate for type of field.
 * 
 *    Rev 1.2   28 May 1991 13:57:16   cole
 * remove all extern's from .c files
 * 
 *    Rev 1.1   13 Nov 1990 11:36:32   irving
 * Tried to make conversion error of float values less.  Didn't help
 * much
 * 
 *    Rev 1.0   27 Sep 1990 15:42:58   admax
 * Initial revision.
/ ----------------------------------------------------------------------- */
  
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "forms.h"
   
/* -----------------------------------------------------------------------
/
/  void scale_float_from_field()
/
/  function:   converts a string in the format "NNNN.NNN xx", where
/              NNNN.NNN is a valid floating point number and xx is
/              a one letter code (must be "m", "u", "n", or "p"),
/              into an appropriately scaled floating point number.
/              If the letter code is missing or not recognized, the
/              number is not scaled.
/  requires:   (char *) scale_str - the string to convert
/  returns:    (float) - the scaled value
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
int scale_float_from_field(void)
{
   char     scale_char = '\x00';
   char     ch;
   int      i;
   double   factor;
   double   dvalue;           
   float    fvalue;
  
   for (i=0; i < (int)Current.Form->field_char_count; i++)
   {
      ch = Current.FieldString[i];
  
      if ( ch == 'G' ||
         ch == 'M' ||
         ch == 'K' ||
         ch == 'm' ||
         ch == 'u' ||
         ch == 'n' ||
         ch == 'p' )
      {
         scale_char = ch;
         break;
      }
   }
   switch (scale_char)
   {
      case 'G':
         factor = FACTOR_FROM_GIGA;
         break;
      case 'M':
         factor = FACTOR_FROM_MEGA;
         break;
      case 'K':
         factor = FACTOR_FROM_KILO;
         break;
      case 'm':
         factor = FACTOR_FROM_MILLI;
         break;
      case 'u':
         factor = FACTOR_FROM_MICRO;
         break;
      case 'n':
         factor = FACTOR_FROM_NANO;
         break;
      case 'p':
         factor = FACTOR_FROM_PICO;
         break;
      default:
         factor = 1.0;
   }
  
   switch (Current.FieldData->type)
   {
      default:
      case DATATYP_FLOAT:
         sscanf(Current.FieldString, "%f", &fvalue);
         if (factor != 1.0) fvalue *= factor;
         *( (float *) Current.FieldDataPtr) = fvalue;
         break;
  
      case DATATYP_DOUBLE_FLOAT:
         sscanf(Current.FieldString, "%lf", &dvalue);
         if (factor != 1.0) dvalue *= factor;
         *( (double *) Current.FieldDataPtr) = dvalue;
         break;
   }
   return(FIELD_VALIDATE_SUCCESS);
}
  
  
/* -----------------------------------------------------------------------
/  function:   formats a floating field in the format "NNNN.NNN xx", where
/              NNNN.NNN is a number in floating point format and xx
/              is a one letter code (must be  "m", "n", "u", or "p"),
/              indicating the magnitude.
/  requires:
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void scale_float_to_field(void)
{
   char     scratch[40];
   char     tag = SPACE;
   char     ch;
   int      i;
   int      exponent = 1;
   double   factor = 1.0;
   double   dvalue;                       
   float    fvalue;
   int scrlen;          
  
  
   fvalue = *( (float *) Current.FieldDataPtr);
   dvalue = *( (double *) Current.FieldDataPtr);
   switch (Current.FieldData->type)
   {
      default:
      case DATATYP_FLOAT:
         // print here to help eliminate weird conversion results 
         sprintf(scratch, "%e", fvalue);
         break;
  
      case DATATYP_DOUBLE_FLOAT:
         // print here to help eliminate weird conversion results 
         sprintf(scratch, "%le", dvalue);
         break;
   }
   for (i=0; (ch = scratch[i]) != '\0'; i++)
   {
      if ( ch == 'e' )
      {
         exponent = atoi( &scratch[i+1] );
         break;
      }
   }
   if (exponent > 0)
   {
      if (exponent >= GIGA_EXPONENT )
      {
         factor = FACTOR_TO_GIGA;
         tag = 'G';
      }
      if (exponent >= MEGA_EXPONENT )
      {
         factor = FACTOR_TO_MEGA;
         tag = 'M';
      }
      else if (exponent >= KILO_EXPONENT )
      {
         factor = FACTOR_TO_KILO;
         tag = 'K';
      }
   }
   else if (exponent < 0)
   {
      if (exponent >= MILLI_EXPONENT )
      {
         factor = FACTOR_TO_MILLI;
         tag = 'm';
      }
      else if (exponent >= MICRO_EXPONENT )
      {
         factor = FACTOR_TO_MICRO;
         tag = 'u';
      }
      else if (exponent >=  NANO_EXPONENT )
      {
         factor = FACTOR_TO_NANO;
         tag = 'n';
      }
      else
      {
         factor = FACTOR_TO_PICO;
         tag = 'p';
      }
   }

   switch (Current.FieldData->type)
   {
      default:
      case DATATYP_FLOAT:
         if (factor != 1.0) fvalue *= factor;
         sprintf(scratch, "%.*f",
         (int) Current.Field->specific.fltfld.right_of_decimal, fvalue);
         break;
  
      case DATATYP_DOUBLE_FLOAT:
         if (factor != 1.0) dvalue *= factor;
         sprintf(scratch, "%.*lf",
            (int) Current.Field->specific.fltfld.right_of_decimal, dvalue);
         break;
   }
   strip_trailing_zeroes(scratch);
  
   scrlen = strlen(scratch);
  
   scratch[scrlen++] = SPACE;
   scratch[scrlen++] = tag;
   scratch[scrlen] = '\x00';
  
   string_to_field_string(scratch);
}
  
void uses_scale_float_fields(void)
{
   FIELD_CLASS * FieldClassPtr;
  
   FieldClassPtr = &FieldClassArray[FLDTYP_SCL_FLOAT];
  
   FieldClassPtr->TypeBit = FTB_SCL_FLT;
   FieldClassPtr->field_char_action = default_char_action;
   FieldClassPtr->data_to_field = scale_float_to_field;
   FieldClassPtr->data_from_field = scale_float_from_field;
   FieldClassPtr->data_limit_action = limit_float;
}
