/* -----------------------------------------------------------------------
/
/  fltfld.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00        1-8 May         1988
/  Worked on:  TLB      Version 1.01
/
  $Header:   J:/logfiles/forms/fltfld.c_v   1.2   28 May 1991 13:51:56   cole  $
  $Log:   J:/logfiles/forms/fltfld.c_v  $
 * 
 *    Rev 1.2   28 May 1991 13:51:56   cole
 * remove all extern's from .c files
 * 
 *    Rev 1.1   13 Nov 1990 10:50:38   irving
 * Tried to make conversion error on floats less.  Only helped a little.
 * 
 *    Rev 1.0   27 Sep 1990 15:40:14   admax
 * Initial revision.
/
/ ----------------------------------------------------------------------- */
  
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "omaform.h"
#include "forms.h"
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void strip_trailing_zeroes(char * num_string)
{
   int num_string_len = (strlen(num_string) - 1 );
  
   for (; num_string_len > 1; num_string_len--)
   {
      if (num_string[num_string_len] == '0'
         && num_string[num_string_len - 1] != '.')
      {
         num_string[num_string_len] = (char) 0;
      }
      else
         break;
   }
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void float_to_field()
{
   char     scratch[40];
   double   dvalue;           // 11/6/90 DAI
   float    fvalue;           // 11/6/90 DAI
  
   switch (Current.FieldData->type)
   {
      default:
      case DATATYP_FLOAT:
         fvalue = *( (float *) Current.FieldDataPtr);
         // print here to help eliminate weird conversion results //11/6/90 DAI
         sprintf(scratch, "%.*f",
            (int) Current.Field->specific.fltfld.right_of_decimal, fvalue);
         break;
      case DATATYP_DOUBLE_FLOAT:
         dvalue = *( (double *) Current.FieldDataPtr);
         // print here to help eliminate weird conversion results //11/6/90 DAI
         sprintf(scratch, "%.*lf",
            (int) Current.Field->specific.fltfld.right_of_decimal, dvalue);
   }
  
   strip_trailing_zeroes(scratch);
   string_to_field_string(scratch);
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
int float_from_field()
{
   FLOAT fvalue;                       // 11/6/90 DAI

   // scan both formats explicitly to help eliminate weird conversion results
   sscanf(Current.FieldString, "%f", &fvalue);     // 11/6/90 DAI

   switch (Current.FieldData->type)
   {
      default:
      case DATATYP_FLOAT:
         *( (float *) Current.FieldDataPtr) = fvalue;
         break;
  
      case DATATYP_DOUBLE_FLOAT:
         *( (double *) Current.FieldDataPtr) = atof(Current.FieldString);
         break;
   }
   return(FIELD_VALIDATE_SUCCESS);
}
  
  
/* -----------------------------------------------------------------------
/
/
/
/  function:
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
BOOLEAN limit_float()
{                    /* if limit error, raise an error signal! */
   BOOLEAN limited = FALSE;
  
   if (Current.FieldData->limit_index != 0)
   {
      DATA_LIMIT * limitor =
      &DataLimitRegistry[Current.FieldData->limit_index];
      if (Current.FieldData->type == DATATYP_DOUBLE_FLOAT)
      {
         double temp = *( (double *) Current.FieldDataPtr);
  
         if ((limitor->low_limit_ptr != NULL)
            && *((double *) limitor->low_limit_ptr) > temp)
         {
            temp = *((double *) limitor->low_limit_ptr);
            limited = TRUE;
         }
         if ((limitor->high_limit_ptr != NULL)
            && *((double *) limitor->high_limit_ptr) < temp)
         {
            temp = *((double *) limitor->high_limit_ptr);
            limited = TRUE;
         }
         /*
         if (limitor->min_delta_ptr != NULL)
         {
         }
         */
         *( (double *) Current.FieldDataPtr) = temp;
      }
      else
      {
         float temp = *( (float *) Current.FieldDataPtr);
  
         if ((limitor->low_limit_ptr != NULL)
            && *((float *) limitor->low_limit_ptr) > temp)
         {
            temp = *((float *) limitor->low_limit_ptr);
            limited = TRUE;
         }
         if ((limitor->high_limit_ptr != NULL)
            && *((float *) limitor->high_limit_ptr) < temp)
         {
            temp = *((float *) limitor->high_limit_ptr);
            limited = TRUE;
         }
         /*
         if (limitor->min_delta_ptr != NULL)
         {
         }
         */
         *( (float *) Current.FieldDataPtr) = temp;
      }
   }
   return (limited);
}
  
void uses_float_fields()
{
   FIELD_CLASS * FieldClassPtr;
  
   FieldClassPtr = &FieldClassArray[FLDTYP_STD_FLOAT];
  
   FieldClassPtr->TypeBit = FTB_STD_FLT;
   FieldClassPtr->field_char_action = default_char_action;
   FieldClassPtr->data_to_field = float_to_field;
   FieldClassPtr->data_from_field = float_from_field;
   FieldClassPtr->data_limit_action = limit_float;
}
  

