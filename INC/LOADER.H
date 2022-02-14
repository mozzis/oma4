/* -----------------------------------------------------------------------
/
/ 	loader.h
/
/	Copyright (c) 1988,  EG&G Princeton Applied Research, Inc.
/
/	Written by: TLB Version 1.00	17 March 1989
/	Worked on:	TLB Version 1.01	
/					    TLB Version 1.02	
/
/ ----------------------------------------------------------------------- */

#define	FALSE			0
#define	TRUE			1
#define	NOT				!
#define	ODD				1

typedef int boolean;

#define	MAX_RECORDS		100
#define	REC_BUF_SIZE		512
#define	PROG_MEM_SIZE		32767		/* s.b. 65536 */
#define	SIZEOF_ASIC_PROGRAM_SPACE	65536
#define	SIZEOF_68K_PROGRAM_SPACE	65536

#define	INVALID_ADDRESS		0xFFFF

#define	DMA_INITIAL_ADDRESS		0
#define	FC_INITIAL_ADDRESS		0

#define	OFFSET_TO_CODE_PTR_TABLE	0x700

#define	ERASE_PROGRAM_MEM		1
#define	ERASE_DATA_MEM		2

#define	SUCCESS		0
#define	UNEXPECTED_EOF		1
#define	INVALID_RECORD_TYPE		2
#define	MISSED_RECORD_HEADER		3
#define	FILE_READ_ERROR		4
#define	FILE_NOT_FOUND		5
#define	OUT_OF_MEMORY		6
#define	FIXUP_MISMATCH		7
#define	READBACK_ERROR		8
#define	PROG_MEM_OVERRUN		9
#define	MOTO_CHECKSUM_BAD		10
#define DAC_CODE_NOT_LOADED       11

#define	WRITE_ALL_BYTES	  0
#define	READ_ALL_BYTES		1
#define	WRITE_ODD_BYTES           2
#define	READ_ODD_BYTES		3
#define	WRITE_EVEN_BYTES	  4
#define	READ_EVEN_BYTES	  5

struct parblk {
	
	int	function;
	int	status;
	unsigned long ASIC_program_memory;
	void far * copy_buffer;
	unsigned int byte_count;
};

struct recpos {
	int	type;
	unsigned int load_address;
	unsigned int size;
	long offset;
};

boolean compare(unsigned char *, unsigned char *, unsigned int);
boolean allocate_buffers(void);
void deallocate_buffers(void);
void close_object_file(void);

