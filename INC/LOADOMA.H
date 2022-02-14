/* -----------------------------------------------------------------------
/ loadoma.h
/ Morris Maynard
/ Copyright (c) 1993,  EG&G Princeton Applied Research, Inc.
/ ----------------------------------------------------------------------- */
#ifndef LOADOMA_INCLUDED
#define LOADOMA_INCLUDED
enum erase_cmds { ERASE_PROGRAM_MEM = 1, ERASE_DATA_MEM };

enum { SUCCESS = 0, 
       UNEXPECTED_EOF, 
       INVALID_RECORD_TYPE, 
       MISSED_RECORD_HEADER, 
       FILE_READ_ERROR, 
       FILE_NOT_FOUND, 
       OUT_OF_MEMORY, 
       FIXUP_MISMATCH, 
       READBACK_ERROR, 
       PROG_MEM_OVERRUN, 
       MOTO_CHECKSUM_BAD, 
       DAC_CODE_NOT_LOADED };

typedef struct blk_nam {
  char name[9];
} BLK_NAME;

extern USHORT NumDACBlks;

extern BLK_NAME *BlkNameBuf;

BOOLEAN download_object_file(char * filename);
BOOLEAN download_monitor_file(char * filename);
void erase_memory(ULONG base_address, enum erase_cmds erase_cmd);
void forget_previous_programs(void);

#endif

