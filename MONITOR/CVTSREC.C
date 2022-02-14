#include <stdio.h>
#include <io.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>

typedef int BOOLEAN;
typedef unsigned char UCHAR;
typedef unsigned long ULONG;
typedef unsigned short USHORT;

#define TRUE  1
#define FALSE 0

#include "loader.h"
#include "loadrec.h"

/* -----------------------------------------------------------------------
/
/ cvtsrec.c
/
/ Copyright (c) 1993,  EG&G Princeton Applied Research, Inc.
/
/ ----------------------------------------------------------------------- */

struct S_header {
	char		start_mark;
	char		record_type;
	char		count[2];
	char		address[8];
};

typedef struct S_header SRECHDR;

UCHAR   *record_buffer;
int     status;
BOOLEAN verbose = FALSE;
FILE *  input_file, *output_file;

/********************************************************************/

BOOLEAN yes()
{
  char ch;

  do
    ch = (char)tolower(getche());
  while (ch != 'y' && ch != 'n');

  return(ch == 'y');
  }

/********************************************************************/
UCHAR get_byte_from_hex(char * two_hex_bytes)
{
  UCHAR binary_value;

  sscanf(two_hex_bytes, "%2x", &binary_value);
  return(binary_value);
}

/********************************************************************/
BOOLEAN find_S_record_from_file(char ** S_buffer)
{
  char * S_record_address;

  while (fgets(*S_buffer, 256, input_file) != NULL)
    {
    if (strlen(*S_buffer) &&
        ((S_record_address = strchr(*S_buffer, 'S')) != NULL))
      {
      *S_buffer = S_record_address;
      return(TRUE);
      }
    }
  return(FALSE);
}

/********************************************************************/
BOOLEAN convert_to_binrec(SRECHDR * S_rec, UCHAR * recsize, UCHAR * binrec)
{
  UCHAR  i, length, byte;
  USHORT checksum, their_checksum;
  char * text = S_rec->address;

  length = get_byte_from_hex(S_rec->count);

  checksum = (USHORT)length; /* include length byte in checksum */

  length--;                  /* don't include checksum byte in length */
  for (i=0; i<length; i++)
    {
    byte = get_byte_from_hex(text);
    *(binrec++) = byte;
    checksum += (USHORT) byte;
    text += 2;
    }
  checksum = (~checksum) & 0x00ff;             /* 1's complement */

  their_checksum = (USHORT) get_byte_from_hex(text);

  *recsize = length;
  if (their_checksum != checksum)
    fprintf(stderr, "Theirs: %x  Ours: %x\n", their_checksum, checksum);
  return(their_checksum == checksum);
}

/********************************************************************/
void swap_bytes_in_buffer(UCHAR * address, USHORT count)
{
  USHORT i;
  UCHAR swap;

  for (i=0; i < count; i+=2)
    {
    swap = address[i];
    address[i] = address[i+1];
    address[i+1] = swap;
    }
}

/********************************************************************/
ULONG translate_68k_records(void)
{
  static char S_rec_text[256];
  SRECHDR * S_rec = (SRECHDR *) S_rec_text;
  UCHAR reclen, byte_width, i;
  USHORT length;
  ULONG address, maxsize = 0L;
  BOOLEAN useable, end_of_file = FALSE;

  while ((!end_of_file) && find_S_record_from_file((char **)&S_rec))
    {
    useable = TRUE;

    if (verbose)
      printf("\nS%c: ", S_rec->record_type);

    switch (S_rec->record_type)
      {
      case '0':
        useable = FALSE;
        if (verbose)
          printf(" Start of File record");
 			break;
			case '1':
		    byte_width = 2;
      break;
      case '2':
        byte_width = 3;
 			break;
			case '3':
				byte_width = 4;
				break;
      case '7':
      case '8':
      case '9':
        useable = FALSE;
        end_of_file = TRUE;
        if (verbose)
          printf("End of File record");
      break;
      }
		if (useable)
      {
      if (convert_to_binrec(S_rec, &reclen, record_buffer))
        {
        length = reclen - byte_width;
        address = 0L;
        for (i=0; i<byte_width; i++)
          address = ((address << 8) | ((ULONG) record_buffer[i]));
        swap_bytes_in_buffer(&record_buffer[byte_width], length);
        
        if (verbose)
          printf(" Address%6lx  %2d bytes", address, length);
        
        if (maxsize < address + (ULONG)length)
          {
          maxsize = address + (ULONG)length;
          chsize(fileno(output_file), maxsize);
          }
        fseek(output_file, address, SEEK_SET);
        fwrite(&record_buffer[byte_width], length, 1, output_file);
        }
      else
        {
        status = MOTO_CHECKSUM_BAD;
        fprintf(stderr,"\n68000 S record checksum error!! Abort download?");
        if (yes())
          end_of_file = TRUE;
        }
      }
    S_rec = (SRECHDR *) S_rec_text;
    }
  return maxsize;
}

int syntax(void)
{
  fprintf(stderr, "Syntax: CVTSREC <infile> <outfile>");
  return 1;
}

BOOLEAN main(int argc, char ** argv)
{
  ULONG total_count;

  if (argc < 3)
    return(syntax());

  if ((input_file = fopen(argv[1], "r")) && 
      (output_file = fopen(argv[2], "wb")))
    {
    record_buffer = malloc(REC_BUF_SIZE);
    if (record_buffer)
      {
      total_count = translate_68k_records();

      if (verbose)
        printf("\n68000 Load Complete - %5ld Bytes", total_count);

      free(record_buffer);
      }
    else
      {
      status = OUT_OF_MEMORY;
      }
    fclose(input_file);
    fclose(output_file);
    }
  else
    {
    status = FILE_NOT_FOUND;
    fprintf(stderr, "File not found or could not be opened\n");
    }
  return(status);
}
