;
; <<==++ Copyright (c) Microsoft Corporation  1986 ++==>>
;

IFDEF DEBUGDEV
  DEBUG = 1
ELSE
  DEBUG = 0
ENDIF

;   Turn off listing for standard include files
.xlist
	INCLUDE DEVHLP.INC
	INCLUDE DEVSYM.INC
	INCLUDE ERROR.INC
	INCLUDE INFOSEG.INC
.list

;
;       for DevHlp_GetDosVar call
;
DOSVAR_CURPID   = 2  ;LocINFOseg (1st word is CurPid)

WO	EQU	WORD PTR
BY	EQU	BYTE PTR

ALLOC_MEM_LOW   EQU 1
ALLOC_MEM_HIGH  EQU 0
STDOUT		EQU 1

MemBufSz	EQU 256

LONG_PTR STRUC
	_OFF	DW	(?)	;Offset of far pointer
	_SEG	DW	(?)	;Segment of far pointer
LONG_PTR ENDS

PHYSADDR STRUC
	_LO	DW	(?)	;Low word of a 32-bit pointer
	_HI	DW	(?)	;High word of a 32-bit pointer
PHYSADDR ENDS

;
; Call a Device Helper service. Assumes that DS is DD's data seg
;
DEVHELP MACRO	func
	mov	dl,DevHlp_&func
	call	[DEVHLP]
ENDM

;
;	for DevHlp_GetDOSVar call
;
DOSVar_SysInfo	EQU	1
DOSVar_LocInfo	EQU	2	      ;LocINFOseg (1st word is CurPid)
DOSVar_ReBoot	EQU	5
DOSVar_YieldFlag    EQU     7
DOSVar_TCYieldFlag  EQU     8
;
;	for SemHandle call
;
NOT_IN_USE	EQU	0
IN_USE		EQU	1
;
;	for PhysToVirt call
ToDS_SI 	EQU	0		; Put result in DS:SI
ToES_DI 	EQU	1		; Put result in ES:DI
;
;	for PhyToUvirt call
;
ToUVirt_RE	EQU	0		; Make segment read/execute
ToUVirt_RW	EQU	1		; Make segment read/write
ToUVirt_Fre	EQU	2		; Free virtual address
Read_Exec       EQU     0               ; also use with VerifyAccess
Read_Write      EQU     1               ; also use with VerifyAccess

;
;	result placement for PhyToVirt call
;
ToES_DI         EQU	1               
ToDS_SI         EQU     0
;
;	for AllocPhys/FreePhys
;
Mem_Hi		EQU	0		; Allocate above 1mb
Mem_Lo		EQU	1		; Allocate below 1mb

;
;       for Lock
;
Lock_Short      EQU     0
Lock_Long       EQU     1
Lock_Wait       EQU     0
Lock_NoWait     EQU     1

;
; For VerifyAccess and
;
Verify_Read             EQU     0       ; use with VerifyAccess
Verify_RW               EQU     1       ; use with VerifyAccess


;
;	for SetIRQ
;
INT_NOT_SHARED  EQU     0
INT_SHARED      EQU     1

;=========================================================================
;
; Local Data Area
;   LocalData structure is allocated on the stack and intialized by the
;   Strategy routine for each device driver instance.
;
LOCALDATA STRUC
    LD_PTRSAVE	DD      (?)     ; Far pointer to the request header
    BOARD_ID    DB      (?)     ; OMA Board ID
LOCALDATA ENDS
