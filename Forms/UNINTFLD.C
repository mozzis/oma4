/* -----------------------------------------------------------------------
/
/  unintfld.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00        1-8 May         1988
/  Worked on:  TLB      Version 1.01
/
  $Header:   J:/logfiles/forms/unintfld.c_v   1.1   28 May 1991 13:58:22   cole  $
  $Log:   J:/logfiles/forms/unintfld.c_v  $
 * 
 *    Rev 1.1   28 May 1991 13:58:22   cole
 * remove all extern's from .c files
 * 
 *    Rev 1.0   27 Sep 1990 15:44:44   admax
 * Initial revision.
/ ----------------------------------------------------------------------- */
  
#include <stdio.h>
#include <stdlib.h>

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
  
void uns_int_to_field()
{
   char           scratch[40];
   unsigned long  field_data;
  
   switch (Current.FieldData->type)
   {
      default:
      case DATATYP_INT:
         field_data =
         (unsigned long) *( (unsigned int *) Current.FieldDataPtr);
         break;
      case DATATYP_LONG_INT:
         field_data = *( (unsigned long *) Current.FieldDataPtr);
         break;
      case DATATYP_CHAR_INT:
         field_data =
         (unsigned long) *( (unsigned char *) Current.FieldDataPtr);
   }
   sprintf(scratch, "%lu", field_data);
   string_to_field_string(scratch);
}
  
int uns_int_from_field()
{
   char *         dummy;
   unsigned long  field_data;
  
   field_data = strtoul(Current.FieldString, &dummy, 10);
  
   switch (Current.FieldData->type)
   {
      default:
      case DATATYP_INT:
         *( (unsigned int *) Current.FieldDataPtr) =
         (unsigned int) field_data;
         break;
      case DATATYP_LONG_INT:
         *( (unsigned long *) Current.FieldDataPtr) = field_data;
         break;
      case DATATYP_CHAR_INT:
         *( (unsigned char *) Current.FieldDataPtr) =
         (unsigned char) field_data;
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
  
BOOLEAN limit_uns_int()
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
            unsigned int   field_data =
            *( (unsigned int *) Current.FieldDataPtr);
  
            if ((limitor->low_limit_ptr != NULL)
               && *((unsigned int *) limitor->low_limit_ptr) > field_data)
            {
               field_data = *((unsigned int *) limitor->low_limit_ptr);
               limited = TRUE;
            }
            if ((limitor->high_limit_ptr != NULL)
               && *((unsigned int *) limitor->high_limit_ptr) < field_data)
            {
               field_data = *((unsigned int *) limitor->high_limit_ptr);
               limited = TRUE;
            }
            /*
            if (limitor->min_delta_ptr != NULL)
            {
            }
            */
            *( (unsigned int *) Current.FieldDataPtr) = field_data;
         }
            break;
  
         case DATATYP_LONG_INT:
         {
            unsigned long     field_data =
            *( (unsigned long *) Current.FieldDataPtr);
  
            if ((limitor->low_limit_ptr != NULL)
               && *((unsigned long *) limitor->low_limit_ptr) > field_data)
            {
               field_data = *((unsigned long *) limitor->low_limit_ptr);
               limited = TRUE;
            }
            if ((limitor->high_limit_ptr != NULL)
               && *((unsigned long *) limitor->high_limit_ptr) < field_data)
            {
               field_data = *((unsigned long *) limitor->high_limit_ptr);
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
            unsigned char  field_data =
            *( (unsigned char *) Current.FieldDataPtr);
  
            if ((limitor->low_limit_ptr != NULL)
               && *((unsigned char *) limitor->low_limit_ptr) > field_data)
            {
               field_data = *((unsigned char *) limitor->low_limit_ptr);
               limited = TRUE;
            }
            if ((limitor->high_limit_ptr != NULL)
               && *((unsigned char *) limitor->high_limit_ptr) < field_data)
            {
               field_data = *((unsigned char *) limitor->high_limit_ptr);
               limited = TRUE;
            }
            *( (unsigned char *) Current.FieldDataPtr) = field_data;
         }
      }
   }
   return (limited);
}
  
void uses_uns_int_fields()
{
   FIELD_CLASS *     FieldClassPtr;
  
   FieldClassPtr = &FieldClassArray[FLDTYP_UNS_INT];
  
   FieldClassPtr->TypeBit = FTB_UNS_INT;
   FieldClassPtr->field_char_action = default_char_action;
   FieldClassPtr->data_to_field = uns_int_to_field;
   FieldClassPtr->data_from_field = uns_int_from_field;
   FieldClassPtr->data_limit_action = limit_uns_int;
}
  

