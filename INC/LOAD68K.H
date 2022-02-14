/* -----------------------------------------------------------------------
/
/ 	load68k.h
/
/	Copyright (c) 1990,  EG&G Princeton Applied Research, Inc.
/
/	Written by: MLM Version 1.00	22 May 1990
/
/ ----------------------------------------------------------------------- */

boolean download_68k_object_file(char * filename, int OMA_port_address,
	unsigned long * base_address, int memory_size);

void download_DAC_list_file(char * filename, int OMA_port_address,
	unsigned long * base_address, int memory_size);

