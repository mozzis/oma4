/* -----------------------------------------------------------------------
/
/ 	motorola.h
/
/	Copyright (c) 1988,  EG&G Princeton Applied Research, Inc.
/
/	Written by: TLB		Version 1.00		   17 March		1989
/
/	motorola.h provides support for Motorola "S-records" (EXORMACS format)
/	for hex downloading of object code images
/
/ ----------------------------------------------------------------------- */

#pragma pack(1)

struct S_header {
	char		start_mark;
	char		record_type;
	char		count[2];
	char		address[8];
};

typedef struct S_header SRECHDR;

#pragma pack()

