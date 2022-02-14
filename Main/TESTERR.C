#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <conio.h>
#include <malloc.h>

#define allocate(ptr, type, count) \
((ptr = (type *) malloc(sizeof(type) * count)) != NULL)

#define MAX_CHOICES             10
#define STRING_SET_GRANULES     32
#define TRUE 1
#define FALSE 0

// -----------------------------------------------------------------------
char ** start_new_string_set(int * elements)
{
   char ** new_string_set;
   int i ;

   if (allocate(new_string_set, char *, STRING_SET_GRANULES))
   {
      *elements = STRING_SET_GRANULES;
      for( i = 0 ; i < * elements ; i ++ )
         new_string_set[ i ] = NULL ;
   }
   else
      *elements = 0;

   return(new_string_set);
}

static char ** extend_string_set( char ** current_set, int * elements )
{
   char ** extended_string_set;

   if (allocate(extended_string_set, char *, (*elements + STRING_SET_GRANULES)))
   {
      int i ;

      memcpy(extended_string_set, current_set, (*elements * sizeof(char *)) );
      free(current_set);

      for( i = * elements ; i < * elements + STRING_SET_GRANULES ; i ++ )
         extended_string_set[ i ] = NULL ;

      *elements += STRING_SET_GRANULES;
      return(extended_string_set);
   }
   else
      return(current_set);
}

char ** add_string_to_string_set(char ** current_set, char * string,
                                 int * index, int * elements)
{
   int      string_length = (strlen(string) + 1);
   char **  working_set = current_set;
   char *   new_string;

   if( (* index) >= ( (* elements) - 1 ) ) // leave a last NULL pointer
   {
      int size_before = *elements;

      working_set = extend_string_set(current_set, elements);
      if (*elements == size_before)
         return(current_set);
   }
   if ((new_string = (char *) malloc(sizeof(char) * string_length)) != NULL)
   {
      strcpy(new_string, string);
      working_set[ (*index)++ ] = new_string;
   }
   return(working_set);
}

int get_file_message(const char * file_spec, int message_index,
                     va_list insert_args )
{
   FILE *   fileptr;
   int  message_found = FALSE;
   int  first_line_of_message = FALSE;
   char **  message_string_array;
   int  string_array_elements;
   int  string_count = 0;
   int  string_length;
  
   message_string_array = start_new_string_set(&string_array_elements);
  
   if( ( fileptr = fopen( file_spec, "r" ) ) != NULL)
   {
      char   file_line[128];
      char * marker;

      while ( fgets(file_line, 128, fileptr) != NULL)
      {
         if ((marker = strstr(file_line, "@@@")) != NULL)
         {
            if (message_found)
               break;
            else
            {
               if (atoi(&marker[3]) == message_index)
                  message_found = TRUE;
            }
         }
         else if (message_found)
         {
            string_length = strlen(file_line);
            if (file_line[ (string_length - 1) ] == '\n')
               file_line[ (string_length - 1) ] = '\0'; /* zap \n at end */
  
            if (strchr(file_line, '%') != NULL)
            {
               char format_string[128];
  
               vsprintf(format_string, file_line, insert_args);
  
               message_string_array =
               add_string_to_string_set(message_string_array,
               format_string, &string_count, &string_array_elements);
            }
            else
            {
               message_string_array =
               add_string_to_string_set(message_string_array,
               file_line, &string_count, &string_array_elements);
            }
         }
      }
      fclose(fileptr);
   }
  
   if (message_found)
    {
    int i;

    for (i = 0; i < string_count; i++)
      printf("%s\n",message_string_array[i]);
    }
   else
    {
    printf("Error #%d\n", message_index);
    }
   return message_found ;
}

void error(int error_number, ... )
{
  int message_result ;
  va_list insert_args;

  va_start(insert_args, error_number);

  message_result = get_file_message("OMA4000.ERS", error_number, insert_args);

  printf("Press any key to continue\n");

  while( ! kbhit() ) ;     // wait for a key press
  getch() ;                // read character, no echo

}



