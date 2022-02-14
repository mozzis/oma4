/* -----------------------------------------------------------------------
/
/  rmintfld.c
/
/  Copyright (c) 1989,  EG&G Instruments Inc.
/
/  Written by: TLB      Version 1.00        1-8 May         1988
/  Worked on:  TLB      Version 1.01
/
  $Header:   J:/logfiles/forms/rmintfld.c_v   1.1   28 May 1991 13:56:36   cole  $
  $Log:   J:/logfiles/forms/rmintfld.c_v  $
 * 
 *    Rev 1.1   28 May 1991 13:56:36   cole
 * remove all extern's from .c files
 * 
 *    Rev 1.0   27 Sep 1990 15:42:40   admax
 * Initial revision.
/ ----------------------------------------------------------------------- */
  
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "omaform.h"
#include "forms.h"
  
#define        ROMAN_NUMERAL_SET_SIZE     7
  
struct {
  
   char     subtractor_index;
   char     numeral;
   long     value;
  
} RomanNumeralSet[ROMAN_NUMERAL_SET_SIZE] = {
  
   { 0, 'I',    1 },
   { 0, 'V',    5 },
   { 0, 'X',   10 },
   { 2, 'L',   50 },
   { 2, 'C',  100 },
   { 4, 'D',  500 },
   { 4, 'M', 1000 }
  
};
  
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
  
void rom_int_to_field()
{
   char     scratch[80];
   int      digit_count = 0;
   int      numeral_index = (ROMAN_NUMERAL_SET_SIZE - 1);
   int      subtractor_numeral_index;
   long     binary_value;
   long     threshold;
  
  
   if (Current.FieldData->type == DATATYP_LONG_INT)
      binary_value = *( (long *) Current.FieldDataPtr);
   else
      binary_value = (long) *( (int *) Current.FieldDataPtr);
  
   while ((numeral_index >= 0) && (binary_value > 0))
   {
      while (binary_value >= RomanNumeralSet[numeral_index].value)
      {
         scratch[digit_count++] = RomanNumeralSet[numeral_index].numeral;
         binary_value -= RomanNumeralSet[numeral_index].value;
      }
      if (numeral_index > 0)
      {
         subtractor_numeral_index =
         RomanNumeralSet[numeral_index].subtractor_index;
  
         threshold = ( RomanNumeralSet[numeral_index].value
         - (RomanNumeralSet[subtractor_numeral_index].value * 2) );
  
         if (binary_value >= threshold)
         {
            binary_value -= RomanNumeralSet[numeral_index].value;
            while (binary_value < 0)
            {
               scratch[digit_count++] =
               RomanNumeralSet[subtractor_numeral_index].numeral;
               binary_value +=
               RomanNumeralSet[subtractor_numeral_index].value;
            }
            scratch[digit_count++] = RomanNumeralSet[numeral_index].numeral;
         }
      }
      numeral_index--;
   }
   scratch[digit_count] = '\0';        // 7/17/90 DAI
  
   string_to_field_string(scratch);
}
  
int rom_int_from_field()
{
   char     digit;
   int      numeral_index = ROMAN_NUMERAL_SET_SIZE; /* init for first interation */
   int      last_numeral_index;
   int      i;
   int      j;
   int      total_digits = strlen(Current.FieldString);
   long     accumulator = 0;
   long     binary_value = 0;
  
   for (i=0; i<total_digits; i++)
   {
      last_numeral_index = numeral_index;
      numeral_index = -1;
      digit = (char) toupper( Current.FieldString[i] );
      for (j=0; j<ROMAN_NUMERAL_SET_SIZE; j++)
      {
         if (digit == RomanNumeralSet[j].numeral)
         {
            numeral_index = j;
            break;
         }
      }
      if (numeral_index == -1)
         return(i);
  
      if (last_numeral_index == numeral_index)
      {
         accumulator += RomanNumeralSet[numeral_index].value;
      }
      else if (last_numeral_index < numeral_index)
      {
         binary_value +=
         (RomanNumeralSet[numeral_index].value - accumulator);
         accumulator = 0;
      }
      else
      {
         binary_value += accumulator;
         accumulator = RomanNumeralSet[numeral_index].value;
      }
   }
   binary_value += accumulator;
  
  
   if (Current.FieldData->type == DATATYP_LONG_INT)
      *( (long *) Current.FieldDataPtr) = binary_value;
   else
      *( (int *) Current.FieldDataPtr) = (int) binary_value;
  
   return(FIELD_VALIDATE_SUCCESS);
}
  
  
static unsigned char    control_keys[] = {
   KEY_BACKSPACE,
   KEY_DELETE,
   KEY_DELETE_FAR,
   KEY_LEFT,
   KEY_RIGHT,
   KEY_HOME,
   KEY_END,
   '\0',                   // 7/17/90 DAI
};
  
void uses_rom_int_fields()
{
   FIELD_CLASS *     FieldClassPtr;
  
   FieldClassPtr = &FieldClassArray[FLDTYP_ROM_INT];
  
   FieldClassPtr->TypeBit = FTB_ROM_INT;
   FieldClassPtr->field_char_action = default_char_action;
   FieldClassPtr->data_to_field = rom_int_to_field;
   FieldClassPtr->data_from_field = rom_int_from_field;
   FieldClassPtr->data_limit_action = limit_int;
  
   legalize_chars_for_field(FTB_ROM_INT, "MDCLXVI-mdclxvi");
   legalize_chars_for_field(FTB_ROM_INT, control_keys);
}
  

