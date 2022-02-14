/**************************************************************************
/
/ loadoma.c
/ Morris Maynard, June 1993
/ Upload ASIC and DAC code to OMA4 controller board
/ Based on a program by Tom Biggs, Dec. 1988
/
 **************************************************************************/

#ifdef _WINOMA_
#include <windows.h>
#include "..\sdkwin\resource.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h>

#ifndef _MSC_VER
#define TURBOC
#endif

#include "eggtype.h"
#include "omaregs.h"
#include "loadoma.h"
#include "loadrec.h"
#include "monitor.h"
#include "counters.h"

#ifdef USE_D16M
#include "d16mphys.h"
#elif defined (__WATCOMC__)
#include <i86.h>
#include "wc32phys.h"
#elif defined(_WINOMA_)
#include "winphys.h"
#else
#include "himem.h"
#endif

#include "access4.h"

#define MAX_RECORDS   100
#define REC_BUF_SIZE  512
#define PROG_MEM_SIZE 32767
#define DAC_OFFSET (OMA_DAC_address - OMA_memory_address)

BLK_NAME *BlkNameBuf = NULL;

USHORT NumDACBlks = 0;

static LoadRecordHeader LocalHeader;
static USHORT OMA_port,
              FC_IP,
              DMA_IP,
              DAC_IP,
              FC_offset,
              DMA_offset,
              DAC_offset;

static BOOLEAN verbose = FALSE;

static ULONG erase_size;

#ifndef __WATCOMC__

static UCHAR far record_buffer[REC_BUF_SIZE+2];
static LoadRecordHeader * const header = &LocalHeader;

#else

static UCHAR record_buffer[REC_BUF_SIZE+2];
static LoadRecordHeader* const header = &LocalHeader;

#endif

#ifdef _PROTECTED_MODE
void __far *DAC_pointer;
#endif

#ifdef STANDALONE

static BOOLEAN forget_flag = FALSE,
               ASIC_file_specified = FALSE,
               DAC_file_specified = FALSE,
               moto_file_specified = FALSE,
               hex_file_specified = FALSE,
               erase_flag = FALSE;

CHAR far *ver_string = "5.0",
     far dac_filename[_MAX_FNAME+_MAX_EXT +_MAX_DIR+_MAX_DRIVE],
     far moto_filename[_MAX_FNAME+_MAX_EXT +_MAX_DIR+_MAX_DRIVE],
     far asic_filename[_MAX_FNAME+_MAX_EXT +_MAX_DIR+_MAX_DRIVE];

CHAR * overrun_message =
      "\nERROR - Attempt to write %s record past end of program memory!";
CHAR * readback_error_message =
    "\nERROR - cannot read back %s code written to address %lx";

#define overrun_error(msg) printf(overrun_message, msg);
#else
#define overrun_error(msg)
#endif

/***************************************************************************/
/* Read a 16 bit value from a 16 bit offset from the board base address    */
/***************************************************************************/
static USHORT far_fetch(USHORT offset)
{
  USHORT data = 0xAAAA;

#ifdef _PROTECTED_MODE
  data = *(USHORT __far *)&(((CHAR __far *)DAC_pointer)[offset]);
#else
  read_high_memory(&data, DAC_MEM_OFFSET + (ULONG)offset, sizeof(USHORT));
#endif
  return(data);
}

/***************************************************************************/
/* Store a 16 bit value at a 16 bit offset from the board base address     */
/***************************************************************************/
static void far_store(USHORT offset, USHORT data)
{
#ifdef _PROTECTED_MODE
  *(USHORT __far *)&(((CHAR __far *)DAC_pointer)[offset]) = data;
#else
  write_high_memory(&data, DAC_MEM_OFFSET + (ULONG)offset, sizeof(USHORT));
#endif
}

/****************************************************************************/
/* Write the contents of some buffer into a location on the OMA4 board      */
/* The location is given as a 16 bit offset from the board base address     */
/****************************************************************************/
static void upload_buffer(void * buffer, USHORT offset, USHORT count)
{
#ifdef _PROTECTED_MODE
  _fmemcpy(&((UCHAR __far *)DAC_pointer)[offset], buffer, count);
#else
  write_high_memory(buffer, DAC_MEM_OFFSET + (ULONG)offset, count);
#endif
}

static void download_buffer(void * buffer, USHORT offset, USHORT count)
{
#ifdef _PROTECTED_MODE
  _fmemcpy(buffer, &((UCHAR __far *)DAC_pointer)[offset], count);
#else
  read_high_memory(buffer, DAC_MEM_OFFSET + (ULONG)offset, count);
#endif
}

/****************************************************************************/
/* This does a sanity check on the object file prior to uploading it        */
/****************************************************************************/
static BOOLEAN scan_for_object_records(FILE * object_file)
{
  BOOLEAN status = SUCCESS;

  do
    {
    fread(header, sizeof(LoadRecordHeader), 1, object_file);

    if (feof(object_file))
      status = UNEXPECTED_EOF;
    else if (ferror(object_file))
      status = FILE_READ_ERROR;
    else if (header->start_id != LOAD_RECORD_START_ID)
      status = MISSED_RECORD_HEADER;
    else if (
      (header->type == FC_BLOCK_RECORD)  ||
      (header->type == DMA_BLOCK_RECORD) ||
      (header->type == DAC_BLOCK_RECORD) ||
      (header->type == CNTR_BLOCK_RECORD)||
      (header->type == FIXUP_RECORD) )
      {
      fseek(object_file, (LONG)header->length, SEEK_CUR);
      }
    else if (header->type != EOF_RECORD)
      status = INVALID_RECORD_TYPE;
    }
  while ((status == SUCCESS) && (header->type != EOF_RECORD));

  rewind(object_file);

  return(status);
}

/***************************************************************************/
/* Do the job of reading one object record into a buffer from a file       */
/***************************************************************************/
static USHORT fill_object_buffer(FILE * object_file, USHORT * bytes_left)
{
  SHORT read_count;

  if (*bytes_left == 0)
    return(0);

  if (*bytes_left < REC_BUF_SIZE)
    read_count = *bytes_left;
  else
    read_count = REC_BUF_SIZE;

  read_count =
    fread(record_buffer, sizeof(CHAR), read_count, object_file);

  if (ferror(object_file))
    *bytes_left = read_count = 0;
  else
    *bytes_left -= read_count;
    
  return(read_count);
}

/***************************************************************************/
/* Add a code routine to one of the tables of "known" routines.  There are */
/* three such tables, one each for ASIC FC, ASIC DC, and DAC programs.     */
/* The table_ptr_offset is the 16 bit offset within monitor program space  */
/* another 16 bit offset, which is the offset of a table in monitor space  */
/* In the same way, the table_index_offset is the 16 bit address of a 16   */
/* bit number, which in turn is the count of entries already in the table  */
/* The block_ptr is a 16-bit address, either in ASIC FC, ASIC DC, or DAC   */
/* program memory, of a code routine to add to the tables of known routines*/
/***************************************************************************/

static void add_block_to_code_table(USHORT table_ptr_offset,   /* monitor addr of table addr */
                                    USHORT table_index_offset, /* monitor addr of table index */
                                    USHORT block_ptr)          /* address of code block */
{
  USHORT table;
  USHORT index;
  USHORT table_offset;

  table = far_fetch(table_ptr_offset);              /* get address of table */
  if ((table) != 0 && (table != INVALID_ADDRESS))
    {
    index = far_fetch(table_index_offset);     /* get next avail entry index */
    table_offset = (index * sizeof(USHORT));   /* convert to 16-bit offset */
    table += table_offset;                     /* add to table base addr */
    far_store(table, block_ptr);               /* store code addr there */
    table += sizeof(USHORT);                   /* mark end of list with */
    far_store(table, INVALID_ADDRESS);         /* invalid address */
    index++;                                   /* increment index */
    far_store(table_index_offset, index);      /* store new index */
    }
}

/***************************************************************************/
/* The tag table is a list of addresses in the ASIC or DAC program.  The 
/* value stored at these addresses is normally an ASIC processor instruction
/* operand; by modifying this value, the host PC program can change the
/* behavior of the ASIC program without having to recompile and reload it.
/* Each tag table entry (or counter link record, since instruction operands 
/* are often loop counters) has the (16-bit) address of the tag, a value
/* which tells which processor the address is for, 
*/
static void add_record_to_tag_table(counter_link_record * counter)
{
  USHORT index;
  USHORT table_offset;

  /* get the offset of the start of the tag table */
  table_offset = far_fetch(offsetof(MONITOR_PTRS, loopcntr_table_ptr));

  /* get the current index of the next tag table entry */
  index = far_fetch(offsetof(MONITOR_PTRS, loopcntr_table_index));

  /* calc the offset to the next tag table entry */
  table_offset += index++ * sizeof(counter_link_record);

  /* store the new counter record in the next available space */
  far_store(table_offset, counter->offset);
  table_offset += sizeof(USHORT); 
  far_store(table_offset, counter->processor);
  table_offset += sizeof(USHORT); 
  far_store(table_offset, counter->class);

  /* store the new index value */
  far_store(offsetof(MONITOR_PTRS, loopcntr_table_index), index);
}

/***************************************************************************/
static void update_code_table_end_ptr(USHORT end_ptr_offset, USHORT last_addr)
{
  far_store(end_ptr_offset, last_addr);
}

/***************************************************************************/
static USHORT get_table_end_ptr(USHORT end_ptr_offset)
{
  return(far_fetch(end_ptr_offset));
}

/***************************************************************************/
static void reset_code_table(USHORT ptr_addr, USHORT index_addr,
                             USHORT end_addr)
{
  USHORT table;
  USHORT index;
  USHORT code_space_end;

  table = far_fetch(ptr_addr);
  if ((table) != 0 && (table != INVALID_ADDRESS))
    {
    index = far_fetch(index_addr);
    if (index != 0) /* if programs have been loaded for this processor */
      {
      /* reset the code table index to zero */
      far_store(index_addr, 0);

      /* find the original starting point, reset end ptr to that */
      code_space_end = far_fetch(table);

      update_code_table_end_ptr(end_addr, code_space_end);
      }
    }
}

/***************************************************************************/
static void reset_counter_table(USHORT table_index_offset)
{
  /* reset the counter table index to zero */
  far_store(table_index_offset, 0);
}

/***************************************************************************/
static SHORT download_DMA_record(FILE * object_file)
{
  USHORT read_count;
  USHORT bytes_in_record;
  SHORT status = SUCCESS;

  bytes_in_record = header->length;
  add_block_to_code_table(offsetof(MONITOR_PTRS, dma_table_ptr),
                          offsetof(MONITOR_PTRS, dma_table_index),
                          DMA_IP);

  while (read_count = fill_object_buffer(object_file, &bytes_in_record))
    {
    if ((DMA_IP + read_count) > PROG_MEM_SIZE)
      {
      if (verbose)
        overrun_error("DMA");
      status = PROG_MEM_OVERRUN;
      break;
      }
    else
      {
      access_alternating_bytes(WRITE_ODD, record_buffer, DMA_IP, read_count);
      DMA_IP += read_count;
      }
    }
  update_code_table_end_ptr(offsetof(MONITOR_PTRS, dma_code_ptr), DMA_IP);
  return status;
}

/***************************************************************************/
static SHORT download_FC_record(FILE * object_file)
{
  USHORT read_count;
  USHORT bytes_in_record;
  SHORT status = SUCCESS;

  bytes_in_record = header->length;
  add_block_to_code_table(offsetof(MONITOR_PTRS, fc_table_ptr),
                          offsetof(MONITOR_PTRS, fc_table_index),
                          FC_IP);

  while (read_count = fill_object_buffer(object_file, &bytes_in_record))
    {
    if ((FC_IP + read_count) > PROG_MEM_SIZE)
      {
      if (verbose)
        overrun_error("FC");
      status = PROG_MEM_OVERRUN;
      break;
      }
    else
      {
      access_alternating_bytes(WRITE_EVEN, record_buffer, FC_IP, read_count);
      FC_IP += read_count;
      }
    }
  if (!status)
    update_code_table_end_ptr(offsetof(MONITOR_PTRS, fc_code_ptr), FC_IP);
  return status;
}

/***************************************************************************/
static BOOLEAN download_DAC_record(FILE * object_file)
{
  BOOLEAN status = SUCCESS;
  USHORT read_count,
         bytes_in_record = header->length;

  add_block_to_code_table(offsetof(MONITOR_PTRS, dac_table_ptr),
                          offsetof(MONITOR_PTRS, dac_table_index),
                          DAC_IP);

  while (read_count = fill_object_buffer(object_file, &bytes_in_record))
    {
    if ((ULONG)((ULONG)DAC_IP + (ULONG)read_count) > (ULONG)PROG_MEM_SIZE)
      {
      if (verbose)
        overrun_error("DAC");
      status = PROG_MEM_OVERRUN;
      break;
      }
    else
      {
      upload_buffer(record_buffer, DAC_IP, read_count);
      DAC_IP += read_count;
      }
    }
  if (!status)
    update_code_table_end_ptr(offsetof(MONITOR_PTRS, dac_code_ptr), DAC_IP);
  return status;
}

/***************************************************************************/
/* add a DAC block name to the list of loaded DAC blocks                   */
/***************************************************************************/
static void addDACName(CHAR * Name)
{
  /* expand buf, or if BlkNameBuf is NULL, do initial allocation */

  BLK_NAME * NewNameBuf = realloc(BlkNameBuf, (NumDACBlks+1) * (BLK_NAME_LEN+1));

  /* if (re)allocation succeeded, install name and save new pointer */
  if (NewNameBuf)
    {
    /* copy all of source name into buffer; don't depend on trailing '0' */;
    memcpy(&NewNameBuf[NumDACBlks++], Name, BLK_NAME_LEN);
    /* then turn copy into a 'C' string with trailing NULL */
    ((CHAR *)NewNameBuf)[NumDACBlks * BLK_NAME_LEN + (NumDACBlks-1)] = '\0';
    BlkNameBuf = NewNameBuf;
    }
}

/***************************************************************************/
static BOOLEAN FIXUP_absolute_references(FILE * object_file)
{
  USHORT remaining_FIXUP_record_size,
         fixup_loc,
         fixup_val;
  BOOLEAN status = SUCCESS;
  struct fixup_rec abs_reference_fix;

  remaining_FIXUP_record_size = header->length;

  while (remaining_FIXUP_record_size > 0)
    {
    if ((fread( &abs_reference_fix, sizeof(struct fixup_rec),
      1, object_file) == 1) && (!(ferror(object_file))) )
      {
      switch (abs_reference_fix.fix_type)
        {
        case FIXUP_FC:
          fixup_loc = abs_reference_fix.fix_pc + FC_offset;
          fixup_val=read_FC_counter(fixup_loc);
        break;

        case FIXUP_DMA:
          fixup_loc = abs_reference_fix.fix_pc + DMA_offset;
          fixup_val=read_DC_counter(fixup_loc);
        break;

        case FIXUP_DAC:
          {
          fixup_loc = abs_reference_fix.fix_pc + DAC_offset;
          if (abs_reference_fix.fix_len == 4) fixup_loc += 2L;
          fixup_val = far_fetch(fixup_loc);
          }
        break;
        }
      if (fixup_val == (USHORT) abs_reference_fix.fix_addr)
        {
        switch (abs_reference_fix.fix_type)
          {
          case FIXUP_FC:
            fixup_val += FC_offset;
            update_FC_counter(fixup_val, fixup_loc);
          break;

          case FIXUP_DMA:
            fixup_val += DMA_offset;
            update_DC_counter(fixup_val, fixup_loc);
          break;

          case FIXUP_DAC:
            fixup_val += DAC_offset;
            far_store(fixup_loc, fixup_val);
          break;
          }
        }
      else
        {
#ifdef STANDALONE
        CHAR * type;

        switch (abs_reference_fix.fix_type)
          {
          case FIXUP_FC:
            type = "FC";
          break;

          case FIXUP_DMA:
            type = "DMA";
          break;

          case FIXUP_DAC:
            type = "DAC";
          break;

          default:
            type = "(UNKNOWN TYPE!)";
          }
        if (verbose)
          printf("\nERROR - %s fixup at %x is %x, should be %x",
               type, abs_reference_fix.fix_pc, fixup_val,
               abs_reference_fix.fix_addr);
#endif
        status = FIXUP_MISMATCH;
        break;
        }
      remaining_FIXUP_record_size -= sizeof(struct fixup_rec);
      }
    else
      {
      /* error reading fixup sub record */
    break;
    }
  }
  return status;
}

/***************************************************************************/
static void mark_COUNTER_references(FILE * object_file)
{
  USHORT remaining_COUNTER_record_size;
  counter_record counter;
  counter_link_record counterlink;

  remaining_COUNTER_record_size = header->length;

  while (remaining_COUNTER_record_size > 0)
    {
    if ((fread(&counter, sizeof(struct counter_rec),
      1, object_file) == 1) && (!(ferror(object_file))))
      {
      counterlink.processor = counter.counter_type;
      counterlink.class = counter.counter_category;
      switch (counter.counter_type)
        {
        case COUNTER_FC:
          counterlink.offset = (counter.counter_pc + FC_offset);
        break;
        case COUNTER_DMA:
          counterlink.offset = (counter.counter_pc + DMA_offset);
        break;
        case COUNTER_DAC:
          counterlink.offset = (counter.counter_pc + DAC_offset);
        break;
        }

      add_record_to_tag_table(&counterlink);

      remaining_COUNTER_record_size -= sizeof(counter_record);
      }
    else
      /* error reading counter sub record */
      break;
  }
}

/***************************************************************************/
/* Search for and open the file with the given name */
/* Look in .INI file, then in current directory and then in environment path */
static FILE * FindAndOpenFile(char *filename)
{
  FILE * infile;
  char path[192];
#ifdef _WINDOWS
  static char IniName[]="OMA4CFG.INI";
  BOOLEAN fContinue=FALSE, IniOK=FALSE;

  IniOK = 
    GetPrivateProfileString("OBJECTS", "PATH", ".\\", path, sizeof(path), 
                            IniName);
  if (IniOK)
    {
    if (path[strlen(path)] != '\\')
      strcat(path, "\\");
    strcat(path, filename);
    }
  do
    {
    if (!IniOK)
      _searchenv(filename,"OMAOBJ",path); /* look in curr. dir, or chk OMAOBJ */
    infile = fopen(path, "rb");
    if (!infile) // should have a file at this point
      {
      char Str[10];
      GetPrivateProfileString("USER", "VERIFY", "TRUE", Str, sizeof(Str), 
                              IniName);
      _strupr(Str);
      if (strstr(Str, "TRUE") || strstr(Str, "YES") || strstr(Str, "1"))
        {
        static char msg[64];
        sprintf(msg, "Could not find file: %s", filename);
        fContinue = MessageBox(NULL, msg, "OMA4WIN DLL", 
                               MB_ICONEXCLAMATION | MB_RETRYCANCEL) != 
                               IDCANCEL;
        }
      }
    }
  while (fContinue);
  return(infile);
#else
  _searchenv(filename,"OMAOBJ",path);    /* not in curr. dir, chk OMAOBJ */
  infile = fopen(path, "rb");
  return(infile);
#endif
}

/***************************************************************************/
BOOLEAN download_object_file(CHAR * filename)
{
  BOOLEAN status = 0;
  FILE * object_file;

  object_file = FindAndOpenFile(filename);

  if(object_file)
    {
    memset(record_buffer, 0, REC_BUF_SIZE);

    if (record_buffer)
      {
      if ((status = scan_for_object_records(object_file)) == SUCCESS)
        {
#ifdef _PROTECTED_MODE
        DAC_pointer = GetProtectedPointer(DAC_OFFSET);
#endif
        FC_offset=get_table_end_ptr(offsetof(MONITOR_PTRS, fc_code_ptr));
        FC_IP = FC_offset;
        DMA_offset=get_table_end_ptr(offsetof(MONITOR_PTRS, dma_code_ptr));
        DMA_IP = DMA_offset;
        DAC_offset=get_table_end_ptr(offsetof(MONITOR_PTRS, dac_code_ptr));
        DAC_IP = DAC_offset;
        map_program_memory();
        do
          {
          fread(header, sizeof(LoadRecordHeader), 1, object_file);

          switch (header->type)
            {
            case FC_BLOCK_RECORD:
              download_FC_record(object_file);
            break;

            case DMA_BLOCK_RECORD:
              download_DMA_record(object_file);
            break;

            case DAC_BLOCK_RECORD:
              addDACName(header->name);
              download_DAC_record(object_file);
            break;

            case CNTR_BLOCK_RECORD:
              mark_COUNTER_references(object_file);
            break;

            case FIXUP_RECORD:
              FIXUP_absolute_references(object_file);
            break;

            case EOF_RECORD:
              fseek(object_file, (long)header->length, SEEK_CUR);
            break;

            default:
              status = INVALID_RECORD_TYPE;
            break;
            }
          }
        while ((status == SUCCESS) && (header->type != EOF_RECORD) &&
               !feof(object_file));
        }
      }
    else
      {
      status = OUT_OF_MEMORY;
      }
    fclose(object_file);
    }
  else
    {
    status = FILE_NOT_FOUND;
    }
#ifdef _PROTECTED_MODE
  DAC_pointer = 0;
#endif
  return(status);
}


/***************************************************************************/
BOOLEAN download_monitor_file(CHAR * filename)
{
  FILE * infile;
  USHORT i = 0;

  infile = FindAndOpenFile(filename);

  if (!infile) /* if still not there */
    {
#ifdef STANDALONE
    perror("\nError opening monitor file ");
#endif
    return(FILE_NOT_FOUND);
    }
  else
    {
    map_program_memory();
#ifdef _PROTECTED_MODE
    DAC_pointer = GetProtectedPointer(DAC_OFFSET);
#endif
    do
      {
      fread(record_buffer, REC_BUF_SIZE, 1, infile);
      upload_buffer(record_buffer, i * REC_BUF_SIZE, REC_BUF_SIZE);
      i++;
      }
    while(!feof(infile));
#ifdef _PROTECTED_MODE
    DAC_pointer = 0;
#endif
    }
  fclose(infile);
  return(SUCCESS);
}

/***************************************************************************/
BOOLEAN write_monitor_file(CHAR * filename)
{
  FILE * outfile;
  USHORT i = 0;

  outfile = fopen(filename, "wb");

  if (!outfile) /* if still not there */
    {
#ifdef STANDALONE
    perror("\nError opening monitor dump file ");
#endif
    return(FILE_NOT_FOUND);
    }
  else
    {
    map_program_memory();
#ifdef _PROTECTED_MODE
    DAC_pointer = GetProtectedPointer(DAC_OFFSET);
#endif
    do
      {
      download_buffer(record_buffer, i * REC_BUF_SIZE, REC_BUF_SIZE);
      fwrite(record_buffer, REC_BUF_SIZE, 1, outfile);
      i++;
      }
    while(i < PROG_MEM_SIZE / REC_BUF_SIZE);
#ifdef _PROTECTED_MODE
    DAC_pointer = 0;
#endif
    }
  fclose(outfile);
  return(SUCCESS);
}


/***************************************************************************/
void forget_previous_programs(void)
{
#ifdef _PROTECTED_MODE
  DAC_pointer = GetProtectedPointer(DAC_OFFSET);
#endif
  map_program_memory();
  reset_code_table(offsetof(MONITOR_PTRS, fc_table_ptr),
                   offsetof(MONITOR_PTRS, fc_table_index),
                   offsetof(MONITOR_PTRS, fc_code_ptr));

  reset_code_table(offsetof(MONITOR_PTRS, dma_table_ptr),
                   offsetof(MONITOR_PTRS, dma_table_index),
                   offsetof(MONITOR_PTRS, dma_code_ptr));

  reset_code_table(offsetof(MONITOR_PTRS, dac_table_ptr),
                   offsetof(MONITOR_PTRS, dac_table_index),
                   offsetof(MONITOR_PTRS, dac_code_ptr));

  reset_counter_table(offsetof(MONITOR_PTRS, loopcntr_table_index));
  NumDACBlks = 0;
  if (BlkNameBuf)
    {
    free(BlkNameBuf);
    BlkNameBuf = 0;
    }
#ifdef _PROTECTED_MODE
    DAC_pointer = 0;
#endif
}

/***************************************************************************/
void erase_memory(ULONG base_address, enum erase_cmds erase_cmd)
{
  USHORT i, buf_count;
  ULONG  offset = base_address - OMA_memory_address;
#ifdef USE_D16M
  void * far_buffer;
  #define SZRBUF 0x8000
#elif defined(__WATCOMC__) || defined(_WINOMA_)
  void __far * far_buffer;
  #define SZRBUF 0x8000
#else
  #define SZRBUF REC_BUF_SIZE
#endif

  memset(record_buffer, 0, REC_BUF_SIZE);

  if (record_buffer)
    {
    if (erase_cmd == ERASE_PROGRAM_MEM)
      map_program_memory();
    else
      {
      if (erase_size == 0L)
        erase_size = 0x200000L; /* two megabytes (in hex) */
      reset_and_map_data();
      }
    if (erase_size == 0L)
      erase_size = (PROG_MEM_SIZE * 4L);

    buf_count = ((USHORT) (erase_size / SZRBUF));

    memset(record_buffer, 0, SZRBUF);

    for (i=0; i<buf_count; i++)
      {
#ifdef USE_D16M
      far_buffer = GetProtectedPointer(offset);
      memset(far_buffer, 0, SZRBUF);
#elif defined(__WATCOMC__) || defined(_WINOMA_)
      far_buffer = GetProtectedPointer(offset);
      _fmemset(far_buffer, 0, SZRBUF);
#else
      write_high_memory(record_buffer, offset, SZRBUF);
#endif
      offset += (ULONG)SZRBUF;
      }
    }
}

#ifdef STANDALONE

void cacheon()
{

}

void cacheoff()
{

}

/***************************************************************************/
static CHAR * status_text(SHORT status)
{
  switch (status)
    {
    case SUCCESS:
      return("Download Successful");
    break;
    case UNEXPECTED_EOF:
      return("Unexpected End of File");
    break;
    case INVALID_RECORD_TYPE:
      return("Record Type Invalid");
    break;
    case MISSED_RECORD_HEADER:
      return("Invalid Record Length / Missing Record Header");
    break;
    case FILE_READ_ERROR:
      return("Read Error");
    break;
    case FILE_NOT_FOUND:
      return("Load File Not Found");
    break;
    case OUT_OF_MEMORY:
      return("No Memory Available");
    break;
    case FIXUP_MISMATCH:
      return("Problems with relocation fixup");
    break;
    case READBACK_ERROR:
      return("Cannot read back image written to memory");
    break;
    case PROG_MEM_OVERRUN:
      return("Program memory overflow");
    break;
    case MOTO_CHECKSUM_BAD:
      return("Bad Checksum in Motorola 68000 S record");
    break;
   }
}

/***************************************************************************/
static void get_parameters_from_environment(void)
{
  CHAR * env_param;
  CHAR * dummy;

  if ( (env_param = getenv("OMAADDR")) != NULL)
    OMA_phys_address = strtoul(env_param, &dummy, 16);

  if ( (env_param = getenv("OMAPORT")) != NULL)
    OMA_port_address = (SHORT) strtoul(env_param, &dummy, 16);
}

/***************************************************************************/
void process_argument(CHAR * arg)
{
  CHAR * dummy;

  if ((arg[0] == '-') || (arg[0] == '/'))
    {
    switch ( tolower(arg[1]) )
      {
      case 'a':                  /* -a Address of Controller Memory */
        OMA_phys_address = strtoul(&arg[2], &dummy, 16);
      break;

      case 'd':                  /* -d DAC list file */
        DAC_file_specified = TRUE;
        strcpy(dac_filename, &arg[2]);
      break;

      case 'e':                  /* -e Erase Flag - erase memory */
        if (tolower(arg[2]) == 'p')
          erase_flag = ERASE_PROGRAM_MEM;
        else
          erase_flag = ERASE_DATA_MEM;

        erase_size = (long) strtoul(&arg[3], &dummy, 16);
      break;

      case 'f':                  /* -f Forget Flag - loses all previously */
        forget_flag = TRUE;      /*            loaded programs */
      break;

      case 'h':                  /* -f Forget Flag - loses all previously */
        hex_file_specified = TRUE;      /*            loaded programs */
        strcpy(moto_filename, &arg[2]);
      break;

      case 'm':                  /* -m Motorola file (for 68000) */
        moto_file_specified = TRUE;
        strcpy(moto_filename, &arg[2]);
      break;
           
      case 'p':                  /* -p Port (I/O) Address of Controller */
        OMA_port_address = (SHORT) strtoul(&arg[2], &dummy, 16);
      break;

      case 's':                  /* -s Memory Size in 2 Mb increments */
        OMA_memory_size = strtoul(&arg[2], &dummy, 16);
      break;

      case 'u':                  /* -u Use ISA upper 2Mb memory block */
        hi_shift = TRUE;
      break;

      case 'v':                  /* -s Memory Size in 2 Mb increments */
        verbose = TRUE;
      break;

      default:                   /* -? what was that? */
        if (verbose)
          printf("\nArgument error: >>%s<<", arg);
      }
    }
  else        /* argument w/o leading '-' or '/' assumed to be filename */
    {
    ASIC_file_specified = TRUE;
    strcpy(asic_filename, arg);
    }
  }


/***************************************************************************/
void validate_parameters(void)
{
  if (OMA_phys_address == 0L)
    {
    if (verbose)
      printf("\nMemory Address 0 Illegal!\n");
    exit(1);
    }

  if (OMA_port_address <= 0x100 || OMA_port_address > 0x7FF)
    {
    if (verbose)
      printf("\n-p  Port Address %x Illegal!\n", OMA_port_address);
    exit(1);
    }
}

/***************************************************************************/
SHORT main(SHORT argc, CHAR ** argv)
{
  SHORT dac_status = SUCCESS,
      asic_status = SUCCESS,
      moto_status = SUCCESS;

  if (argc > 1)
    {
    SHORT i;
    get_parameters_from_environment();

    for (i=1; i < argc; i++)
      process_argument(argv[i]);

    validate_parameters();
    access_startup_detector(OMA_phys_address, OMA_memory_size);
    
    if (!access_init_detector(OMA_port, OMA_memory_size))
      {
      if (erase_flag)
        {
        erase_memory(OMA_memory_address, erase_flag);
        if (verbose)
          printf("\nMemory Erased!\n");
        }

      if (moto_file_specified)
        moto_status = download_monitor_file(moto_filename);

      if (forget_flag)
        forget_previous_programs();

      if (DAC_file_specified) /* DAC files no longer unique */
        dac_status = download_object_file(dac_filename);

      if (ASIC_file_specified)
        asic_status = download_object_file(asic_filename);

      if (asic_status == SUCCESS &&
          dac_status  == SUCCESS &&
          moto_status == SUCCESS)
        if (verbose)
          printf("\nDownload Successful\n");
      else
        {
        if (verbose)
          {
          printf("\nASIC Download: %s", status_text(asic_status));
          printf("\nDAC  Download: %s\n", status_text(dac_status));
          printf("\nMONITOR Download: %s\n", status_text(moto_status));
          }
        }
      }
    else
      {
      if (verbose)
        printf("\nOMA board not found\n");
      }
    }
  else
    {
    printf("\nLOADOMA   Version %s", ver_string);
    printf("\nDownloads ASIC & 68000 Object Files to OMA program memory");
    printf("\nusage:");
    printf("\nLOADER [ <filespec> -a -p -s -m -d -s -n -g -e -f -v ]");
    printf("\n-a<HexAddr>        Memory Address");
    printf("\n-p<HexAddr>        Port (I/O) address");
    printf("\n-s<num>            Memory Size in <num> 2 Megabyte chunks");
    printf("\n-d<filespec>       load DAC List file");
    printf("\n-ep<HexSize>       Erase program memory");
    printf("\n-ed<HexSize>       Erase data memory");
    printf("\n-f                 Forget previously loaded programs");
    printf("\n-v                 Print error and info messages");
    printf("\n-u                 Use ISA Upper 2MB Memory block");
    }
  return(asic_status + dac_status);
  }
#endif
