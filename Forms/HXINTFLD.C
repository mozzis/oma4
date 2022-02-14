/* -----------------------------------------------------------------------
/
/  hxintfld.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00        1-8 May         1988
/  Worked on:  TLB      Version 1.01
/
  $Header:   J:/logfiles/forms/hxintfld.c_v   1.2   28 Aug 1991 18:04:56   cole  $
  $Log:   J:/logfiles/forms/hxintfld.c_v  $
 * 
 *    Rev 1.2   28 Aug 1991 18:04:56   cole
 * delete include for omaform.h
 * 
 *    Rev 1.1   28 May 1991 13:54:08   cole
 * remove all extern's from .c files
 * 
 *    Rev 1.0   27 Sep 1990 15:41:02   admax
 * Initial revision.
/
/  (unintfld.c must also be included with program to use this file.)
/
/ ----------------------------------------------------------------------- */
  
#include <stdio.h>
#include <stdlib.h>

#include "forms.h"
  
/* -----------------------------------------------------------------------
/  requires:   (void)
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
void hex_int_to_field(void)
{
  char scratch[40];
  ULONG  field_data;
 
  switch (Current.FieldData->type)
    {
    default:
    case DATATYP_INT:
      field_data = (ULONG)*((USHORT *)Current.FieldDataPtr);
      break;
    case DATATYP_LONG_INT:
      field_data = *((ULONG *)Current.FieldDataPtr);
      break;
    case DATATYP_CHAR_INT:
      field_data = (ULONG)*((UCHAR *)Current.FieldDataPtr);
    }
  sprintf(scratch, "%lX", field_data);
  string_to_field_string(scratch);
}
  
int hex_int_from_field(void)
{
  char *         dummy;
  ULONG  field_data;
 
  field_data = strtoul(Current.FieldString, &dummy, 16);
 
  switch (Current.FieldData->type)
    {
    default:
    case DATATYP_INT:
      *((USHORT *) Current.FieldDataPtr) = (USHORT) field_data;
      break;
    case DATATYP_LONG_INT:
      *((ULONG *) Current.FieldDataPtr) = field_data;
      break;
    case DATATYP_CHAR_INT:
      *((UCHAR *) Current.FieldDataPtr) = (UCHAR) field_data;
    }
  return(FIELD_VALIDATE_SUCCESS);
}
  
void uses_hex_int_fields(void)
{
   FIELD_CLASS *     FieldClassPtr;
  
   FieldClassPtr = &FieldClassArray[FLDTYP_HEX_INT];
  
   FieldClassPtr->TypeBit = FTB_HEX_INT;
   FieldClassPtr->field_char_action = default_char_action;
   FieldClassPtr->data_to_field = hex_int_to_field;
   FieldClassPtr->data_from_field = hex_int_from_field;
   FieldClassPtr->data_limit_action = limit_uns_int;
}
