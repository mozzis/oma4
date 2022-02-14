; -----------------------------------------------------------------------
;
;  lvid.asm
;
;  Copyright (c) 1989,  EG&G Instruments Inc.
;
;
;  $Header: /OMA4000/Forms/LVID.ASM 1     6/18/99 1:53a Maynard $
;  $Log: /OMA4000/Forms/LVID.ASM $
; 
; 1     6/18/99 1:53a Maynard
;  
;     Rev 1.0   27 Sep 1990 15:41:58   admax
;  Initial revision.
;

        .MODEL LARGE,C
        .CODE

StringToVidMem PROC FAR  USES ES DS CX DX, \
	video_ram:FAR PTR, total_columns:WORD, string:FAR PTR, \
	attrib:WORD, row:WORD, col:WORD, count:WORD
	
	MOV	CX, count		; 
	JCXZ	gone
	LES	BX, video_ram		; vid segment
	MOV	BX, attrib		; attribute
	MOV	DI, col			; column
	MOV	AX, row			; row
	MOV	DX, total_columns	; 
	MUL	DX			; 
	ADD	DI,	AX
	SHL	DI,	1
	MOV	AH,	BL
	LDS	SI, string

next_byte:
	LODSB
	STOSW
	LOOP	next_byte
gone:
	RET
StringToVidMem ENDP

VidMemWrite PROC FAR  USES ES DS CX DX, \
	video_ram:FAR PTR, total_columns:WORD, save_buffer:FAR PTR, \
	row:WORD, col:WORD, cell_count:WORD
	
	MOV	CX, cell_count		; 
	JCXZ	gone
	LES	DI, video_ram		; vid segment
	MOV	DI, col			; column
	MOV	AX, row			; row
	MOV	DX, total_columns	; 
	MUL	DX			; 
	ADD	DI,	AX
	SHL	DI,	1
	LDS	SI, save_buffer

	REP MOVSW
gone:
	RET

VidMemWrite ENDP

VidMemRead PROC FAR  USES ES DS CX DX, \
	video_ram:FAR PTR, total_columns:WORD, save_buffer:FAR PTR, \
	row:WORD, col:WORD, cell_count:WORD
	
	MOV	CX, cell_count		; 
	JCXZ	gone
	LDS	SI, video_ram		; vid segment
	MOV	SI, col			; column
	MOV	AX, row			; row
	MOV	DX, total_columns	; 
	MUL	DX			; 
	ADD	SI,	AX
	SHL	SI,	1
	LES	DI, save_buffer

	REP MOVSW
gone:
	RET

VidMemRead ENDP


VidMemMove PROC FAR  USES ES DS BX CX DX, \
	video_ram:FAR PTR, total_columns:WORD, \
	src_row:WORD, src_col:WORD, \
	dst_row:WORD, dst_col:WORD, cell_count:WORD
	
	MOV	CX, cell_count		; 
	JCXZ	gone
	MOV	BX, total_columns	; 

	LDS	SI, video_ram		; vid segment
	MOV	SI, src_col		; column
	MOV	AX, src_row		; row
	MUL	BX			; 
	ADD	SI,	AX
	SHL	SI,	1

	LES	DI, video_ram		; vid segment
	MOV	DI, dst_col		; column
	MOV	AX, dst_row		; row
	MUL	BX			; 
	ADD	DI,	AX
	SHL	DI,	1

	REP MOVSW
gone:
	RET

VidMemMove ENDP

	END


