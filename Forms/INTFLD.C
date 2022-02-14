/* -----------------------------------------------------------------------
/
/  intfld.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00        1-8 May         1988
/  Worked on:  TLB      Version 1.01
/
  $Header:   J:/logfiles/forms/intfld.c_v   1.1   28 May 1991 13:54:32   cole  $
  $Log:   J:/logfiles/forms/intfld.c_v  $
 * 
 *    Rev 1.1   28 May 1991 13:54:32   cole
 * remove all extern's from .c files
 * 
 *    Rev 1.0   27 Sep 1990 15:41:16   admax
 * Initial revision.
/ ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
  
#include "omaform.h"
#include "forms.h"

// -----------------------------------------------------------------------
void int_to_field()
{
   char     scratch[40];
   long     field_data;
  
   switch (Current.FieldData->type)
   {
      default:
      case DATATYP_INT:
         field_data = (long) *( (int *) Current.FieldDataPtr);
         break;
      case DATATYP_LONG_INT:
         field_data = *( (long *) Current.FieldDataPtr);
         break;
      case DATATYP_CHAR_INT:
         field_data = (long) *( (char *) Current.FieldDataPtr);
   }
   sprintf(scratch, "%ld", field_data);
   string_to_field_string(scratch);
}
  
int int_from_field()
{
   long     field_data;
  
   field_data = atol(Current.FieldString);
  
   switch (Current.FieldData->type)
   {
      default:
      case DATATYP_INT:
         *( (int *) Current.FieldDataPtr) = (int) field_data;
         break;
      case DATATYP_LONG_INT:
         *( (long *) Current.FieldDataPtr) = field_data;
         break;
      case DATATYP_CHAR_INT:
         *( (char *) Current.FieldDataPtr) = (char) field_data;
   }
   return(FIELD_VALIDATE_SUCCESS);
}

// -----------------------------------------------------------------------  
BOOLEAN limit_int()
{                    /* if limit error, raise an error signal! */
   BOOLEAN  limited = FALSE;
  
   if (Current.FieldData->limit_index != 0)
   {
      DATA_LIMIT *   limitor =
      &DataLimitRegistry[Current.FieldData->limit_index];
  
      switch (Current.FieldData->type)
      {
         default:
         case DATATYP_INT:
         {
            int   field_data = *( (int *) Current.FieldDataPtr);
  
            if ((limitor->low_limit_ptr != NULL)
               && *((int *) limitor->low_limit_ptr) > field_data)
            {
               field_data = *((int *) limitor->low_limit_ptr);
               limited = TRUE;
            }
            if ((limitor->high_limit_ptr != NULL)
               && *((int *) limitor->high_limit_ptr) < field_data)
            {
               field_data = *((int *) limitor->high_limit_ptr);
               limited = TRUE;
            }
            /*
            if (limitor->min_delta_ptr != NULL)
            {
            }
            */
            *( (int *) Current.FieldDataPtr) = field_data;
         }
            break;
  
         case DATATYP_LONG_INT:
         {
            long     field_data = *( (long *) Current.FieldDataPtr);
  
            if ((limitor->low_limit_ptr != NULL)
               && *((long *) limitor->low_limit_ptr) > field_data)
            {
               field_data = *((long *) limitor->low_limit_ptr);
               limited = TRUE;
            }
            if ((limitor->high_limit_ptr != NULL)
               && *((long *) limitor->high_limit_ptr) < field_data)
            {
               field_data = *((long *) limitor->high_limit_ptr);
               limited = TRUE;
            }
            /*
            if (limitor->min_delta_ptr != NULL)
            {
            }
            */
            *( (long *) Current.FieldDataPtr) = field_data;
         }
            break;
  
         case DATATYP_CHAR_INT:
         {
            char  field_data = *( (char *) Current.FieldDataPtr);
  
            if ((limitor->low_limit_ptr != NULL)
               && *((char *) limitor->low_limit_ptr) > field_data)
            {
               field_data = *((char *) limitor->low_limit_ptr);
               limited = TRUE;
            }
            if ((limitor->high_limit_ptr != NULL)
               && *((char *) limitor->high_limit_ptr) < field_data)
            {
               field_data = *((char *) limitor->high_limit_ptr);
               limited = TRUE;
            }
            *( (char *) Current.FieldDataPtr) = field_data;
         }
      }
   }
   return (limited);
}
  
void uses_int_fields()
{
   FIELD_CLASS *     FieldClassPtr;
  
   FieldClassPtr = &FieldClassArray[FLDTYP_INT];
  
   FieldClassPtr->TypeBit = FTB_INT;
   FieldClassPtr->field_char_action = default_char_action;
   FieldClassPtr->data_to_field = int_to_field;
   FieldClassPtr->data_from_field = int_from_field;
   FieldClassPtr->data_limit_action = limit_int;
}


