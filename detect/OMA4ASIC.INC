;**************************************************************************
;
;	OMA4ASIC.INC
;
;	This include file contains equates which define the 
;	function code modifiers for the OMAIV ASIC processors,
;	and codes generic to all detector interfaces connected
;	to it.
;
;	Written by:	TLB	Version 1.00	     2	January	  1990
;	Worked on:	TLB	Version 1.01	   
;
;**************************************************************************


SET_TRIG_A		EQU	%0000001000000000	; SET TRIGGER A
RESET_TRIG_A		EQU	%0000000100000000	; RESET TRIGGER A
SET_TRIG_B		EQU	%0000010000000000	; SET TRIGGER B
RESET_TRIG_B		EQU	%0000100000000000	; RESET TRIGGER B

CUE_DMA			EQU	%1000000000000000	;



AtoD12			EQU	$000FFF
AtoD14			EQU	$003FFF
AtoD16			EQU	$00FFFF
AtoD18			EQU	$03FFFF
AtoD20			EQU	$0FFFFF
AtoD22			EQU	$3FFFFF

CLBIT			EQU	$400000
OTBIT			EQU	$800000		; check these - might be opposite

	; FC or DC drives load of address base
FC_LOADS		EQU	0
DC_LOADS		EQU	1

	; Pixel times included in link compensation pipeline
PIXTIM_1_LOOPB		EQU	0
PIXTIM_2		EQU	2
PIXTIM_3_PROTO		EQU	4
PIXTIM_4_QNS		EQU	6

	; Transmission length control
TLEN_13_NORMAL		EQU	(13 * 8)	; shift left by 3 bits
TLEN_18_LOOPB		EQU	(18 * 8)


AUX_PORT_0		EQU	$0000
AUX_PORT_1		EQU	$0100
AUX_PORT_2		EQU	$0200
AUX_PORT_3		EQU	$0300
AUX_PORT_4		EQU	$0400
AUX_PORT_5		EQU	$0500
AUX_PORT_6		EQU	$0600
AUX_PORT_7		EQU	$0700
AUX_PORT_8		EQU	$0800
AUX_PORT_9		EQU	$0900
AUX_PORT_10	EQU	$0A00
AUX_PORT_11	EQU	$0B00
AUX_PORT_12	EQU	$0C00
AUX_PORT_13	EQU	$0D00
AUX_PORT_14	EQU	$0E00
AUX_PORT_15	EQU	$0F00

DEG_10_PLUS	EQU	0
DEG_5_PLUS		EQU	%00000100 
DEG_0			EQU	%00001010 
DEG_5_MINUS	EQU	%00001110 
DEG_10_MINUS	EQU	%00010100 
DEG_20_MINUS	EQU	%00011110 
DEG_30_MINUS	EQU	%00101000 
DEG_40_MINUS	EQU	%00110010 
DEG_50_MINUS	EQU	%00111100 
DEG_55_MINUS	EQU	%01000000 
DEG_60_MINUS	EQU	%01000110 
COOLER_ON      EQU %00000001
COOLER_OFF     EQU %00000000

; for LOAD2 instruction
AUX_PORT_INSTR		EQU	%10000000
DETECTOR_INSTR		EQU	%00000000
;
;------- Address Calculation Instructions -------------
;
RPP			EQU	%0001000000000000	;Reset Pixel Pointer.
IPP			EQU	%0010000000000000	;Increment Pixel Ptr.
RTP			EQU	%0011000000000000	;Reset Track Pointer.
ITP			EQU	%0100000000000000	;Increment Track Ptr.
RPP_RTP			EQU	%0101000000000000	;Reset Pointers.
RPP_ITP			EQU	%0110000000000000	;RS PPTR & INC TPTR.
IPP_RTP			EQU	%0111000000000000	;INC PPTR & RS TPTR.
;
;------- DMA Instructions -----------------------------
;
NOTHING			EQU	%00000000	;Do nothing.
REPLACE			EQU	%00000111	; +A/D = Memory
SUB			EQU	%00000010	; +A/D - Memory = Memory
ADD			EQU	%00000110	; +A/D + Memory = Memory	    
SUBD			EQU	%00000100	; Memory-A/D = Memory
SUBBG			EQU	%00000011	; +A/D - Bkgnd = Memory
ADDBG			EQU	%00000001	; +A/D + Bkgnd = Memory
SUBDBG			EQU	%00000101	; Bkgnd - A/D = Memory
;
LD_BGB			EQU	%00001000	;Load background base.
LD_FB			EQU	%00010000	;Load frame base.
LD_ADDR		EQU	%00011000	;Load both bases.
DB_PREC		EQU	%00100000	;Double precision.




