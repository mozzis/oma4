        PAGE    ,132
        TITLE   OMA4DRV - device driver for AT bus OMA4 board
        SUBTTL  OMA4DRV - Main routine
;**********************************************************************
;
; OMA4DRV.asm
;
;  copyright (c) 1988, EG&G Princeton Applied Research                    
;
;  AT bus Board OS/2 device driver.
;
;
; started: 9/9/88       DI
;
;**********************************************************************


.286                    ; Enable 80286 instructions
;============================================================================
;
; Register conventions:
;       SS:BP is set up by the strategy routine before dispatching
;       to the appropriate handler.
;
;       Each handler must preserve SS:BP and return control to an Exit label,
;       but may trash DS:SI, ES:DI, and even SP.  See SAMPExit for details.
;
;============================================================================

.XLIST
IFDEF DEBUGDEV
  DEBUG = 1
ELSE
  DEBUG = 0
ENDIF

	INCLUDE DEVHLP.INC
	INCLUDE DEVSYM.INC
	INCLUDE ERROR.INC
	INCLUDE INFOSEG.INC
        INCLUDE PCSYS.INC


WO	EQU	WORD PTR
BY	EQU	BYTE PTR
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
;
;	for AllocPhys/FreePhys
;
Mem_Hi		EQU	0		; Allocate above 1mb
Mem_Lo		EQU	1		; Allocate below 1mb

;
;       for VerifyAccess
;
Verify_Read     EQU     0
Verify_RW       EQU     1

;
;
;
INT_SHARED      EQU     1
INT_NOT_SHARED  EQU     0


TRUE            EQU     1
FALSE           EQU     0

.LIST

;=========================================================================
;
; Local Data Area
;   LocalData structure is allocated on the stack and intialized by the
;   Strategy routine for each device driver instance.
;
LOCALDATA STRUC
    LD_PTRSAVE	DD      (?)		; Far pointer to the request header
    BOARD_ID    DB      0
LOCALDATA ENDS

MAIN_DATA SEGMENT WORD PUBLIC 'DATA'
         ASSUME CS:MAIN_CODE,DS:MAIN_DATA,ES:NOTHING,SS:NOTHING

;============================================================================
;
; Device Header -
;
;
IF DEBUG
        PUBLIC  OMA4_DEVHDR_0
        PUBLIC  OMA4_DEVHDR_1
        PUBLIC  OMA4_DEVHDR_2
        PUBLIC  OMA4_DEVHDR_3
        PUBLIC  OMA4_DEVHDR_4
        PUBLIC  OMA4_DEVHDR_5
        PUBLIC  OMA4_DEVHDR_6
        PUBLIC  OMA4_DEVHDR_7
ENDIF

OMA4_DEVHDR_0 LABEL WORD
;        DW      -1, -1                       ; * Header link words
        DD      OMA4_DEVHDR_1                       ; * Header link words


            ; * Use equates in DEVHDR.INC to define Attribute word        
        DW      DEV_30 OR DEV_CHAR_DEV OR DEVLEV_1 OR DEV_GIOCTL           
;        DW      DEV_30 OR DEV_CHAR_DEV OR DEVLEV_1 
                                                
        DW      OFFSET MAIN_CODE:Strategy0          ; * offset to strategy 
                                                    ;   routine entry point
        dw      0                                   ; * Reserved
        db      'OMA4DV0$'                          ; * Name of device
        db      8 dup (?)                           ; * More reserved
; End of the device header.
OMA4_DEVHDR_1 LABEL WORD
        DD      OMA4_DEVHDR_2                       ; * Header link words


            ; * Use equates in DEVHDR.INC to define Attribute word        
        DW      DEV_30 OR DEV_CHAR_DEV OR DEVLEV_1 OR DEV_GIOCTL           
                                                
        DW      OFFSET MAIN_CODE:Strategy1          ; * offset to strategy 
                                                    ;   routine entry point
        dw      0                                   ; * Reserved
        db      'OMA4DV1$'                          ; * Name of device
        db      8 dup (?)                           ; * More reserved
; End of the device header.
OMA4_DEVHDR_2 LABEL WORD
        DD      OMA4_DEVHDR_3                       ; * Header link words
            ; * Use equates in DEVHDR.INC to define Attribute word        
        DW      DEV_30 OR DEV_CHAR_DEV OR DEVLEV_1 OR DEV_GIOCTL           
        dw      OFFSET MAIN_CODE:Strategy2         ; * offset to strategy 
                                                    ;   routine entry point
        dw      0                                   ; * Reserved
        db      'OMA4DV2$'                          ; * Name of device
        db      8 dup (?)                           ; * More reserved
; End of the device header.
OMA4_DEVHDR_3 LABEL WORD
        DD      OMA4_DEVHDR_4                       ; * Header link words
            ; * Use equates in DEVHDR.INC to define Attribute word        
        DW      DEV_30 OR DEV_CHAR_DEV OR DEVLEV_1 OR DEV_GIOCTL           
        dw      OFFSET MAIN_CODE:Strategy3         ; * offset to strategy 
                                                    ;   routine entry point
        dw      0                                   ; * Reserved
        db      'OMA4DV3$'                          ; * Name of device
        db      8 dup (?)                           ; * More reserved
; End of the device header.
OMA4_DEVHDR_4 LABEL WORD
        DD      OMA4_DEVHDR_5                       ; * Header link words
            ; * Use equates in DEVHDR.INC to define Attribute word        
        DW      DEV_30 OR DEV_CHAR_DEV OR DEVLEV_1 OR DEV_GIOCTL           
        dw      OFFSET MAIN_CODE:Strategy4         ; * offset to strategy 
                                                    ;   routine entry point
        dw      0                                   ; * Reserved
        db      'OMA4DV4$'                          ; * Name of device
        db      8 dup (?)                           ; * More reserved
; End of the device header.
OMA4_DEVHDR_5 LABEL WORD
        DD      OMA4_DEVHDR_6                       ; * Header link words
            ; * Use equates in DEVHDR.INC to define Attribute word        
        DW      DEV_30 OR DEV_CHAR_DEV OR DEVLEV_1 OR DEV_GIOCTL           
        dw      OFFSET MAIN_CODE:Strategy5         ; * offset to strategy 
                                                    ;   routine entry point
        dw      0                                   ; * Reserved
        db      'OMA4DV5$'                          ; * Name of device
        db      8 dup (?)                           ; * More reserved
; End of the device header.
OMA4_DEVHDR_6 LABEL WORD
        DD      OMA4_DEVHDR_7                       ; * Header link words
            ; * Use equates in DEVHDR.INC to define Attribute word        
        DW      DEV_30 OR DEV_CHAR_DEV OR DEVLEV_1 OR DEV_GIOCTL           
        dw      OFFSET MAIN_CODE:Strategy6         ; * offset to strategy 
                                                    ;   routine entry point
        dw      0                                   ; * Reserved
        db      'OMA4DV6$'                          ; * Name of device
        db      8 dup (?)                           ; * More reserved
; End of the device header.
OMA4_DEVHDR_7 LABEL WORD
        DW      -1, -1          ;last header        ; * Header link words
            ; * Use equates in DEVHDR.INC to define Attribute word        
        DW      DEV_30 OR DEV_CHAR_DEV OR DEVLEV_1 OR DEV_GIOCTL           
        dw      OFFSET MAIN_CODE:Strategy7         ; * offset to strategy 
                                                    ;   routine entry point
        dw      0                                   ; * Reserved
        db      'OMA4DV7$'                          ; * Name of device
        db      8 dup (?)                           ; * More reserved
; End of the device header.
;
;============================================================================


;============================================================================
;
; Dispatch Table -
;       We dispatch based on the requested function code via this table.
;
;============================================================================

IF DEBUG
        PUBLIC  DISPTAB
ENDIF
        EVEN
DISPTAB LABEL WORD

        DW      OFFSET MAIN_CODE:INIT        ;0: Init
        DW      OFFSET MAIN_CODE:GenFailure  ;1: Media Check (Block Devices Only)
        DW      OFFSET MAIN_CODE:GenFailure  ;2: Build BPB (Block Devices Only)
        DW      OFFSET MAIN_CODE:GenFailure  ;3: Reserved (used to be Ioctl Input)
        DW      OFFSET MAIN_CODE:ExitOK      ;4: Input (Read)
        DW      OFFSET MAIN_CODE:GenFailure  ;5: Non-Destructive Input, no wait.
        DW      OFFSET MAIN_CODE:GenFailure  ;6: Input Status
        DW      OFFSET MAIN_CODE:GenFailure  ;7: Input Flush
        DW      OFFSET MAIN_CODE:GenFailure  ;8: Output (Write)
        DW      OFFSET MAIN_CODE:GenFailure  ;9: Output (Write) with verify
        DW      OFFSET MAIN_CODE:GenFailure  ;10: Output Status
        DW      OFFSET MAIN_CODE:GenFailure  ;11: Output Flush
        DW      OFFSET MAIN_CODE:GenFailure  ;12: Reserved (used to be Ioctl
                                             ;    Output)
        DW      OFFSET MAIN_CODE:OMAOpen     ;13: Device Open
        DW      OFFSET MAIN_CODE:OMAClose    ;14: Device Close
        DW      OFFSET MAIN_CODE:GenFailure  ;15: Query - removable media 
                                             ;    (Block Devices Only)
        DW      OFFSET MAIN_CODE:OMAIOCTL    ;16: OMA specific subcommands
        DW      OFFSET MAIN_CODE:GenFailure  ;17: Reset media (Block Devices
                                             ;    Only)
        DW      OFFSET MAIN_CODE:GenFailure  ;18: Get logical drive map 
                                             ;    (Block Devices Only)
        DW      OFFSET MAIN_CODE:GenFailure  ;19: Set logical drive map 
                                             ;    (Block Devices Only)
        DW      OFFSET MAIN_CODE:GenFailure  ;20: Deinstall
        DW      OFFSET MAIN_CODE:GenFailure  ;21: ???
        DW      OFFSET MAIN_CODE:GenFailure  ;22: Query - get # of fixed 
                                             ;    disks (Block Devices Only)
        DW      OFFSET MAIN_CODE:GenFailure  ;23: Get fixed disk/logical 
                                             ;    unit drive map.  
                                             ;    Partitionable fixed disk
                                             ;    support.
                                             ;    (Block Devices Only)
        DW      OFFSET MAIN_CODE:GenFailure  ;24: Reserved 
        DW      OFFSET MAIN_CODE:GenFailure  ;25: Reserved 
        DW      OFFSET MAIN_CODE:GenFailure  ;26: Reserved 
MAXCMD = (($ - DISPTAB)/2) - 1


CR      EQU     0Dh     
LF      EQU     0Ah
TAB     EQU     09H

;============================================================================
;
; variables -
;
        PUBLIC DEVHLP
        EVEN
DEVHLP          DD      0               ; Pointer to device driver help routine
YieldAddr       dd      0
SAVE_BX         DW      0
OpenFlag        DB      0               ; Detector Open Flags
CurPID          DW      0               ; Current PID
OpeningPID      DW      8 DUP (0)       ; PID of opening tasks
OpenNum         DW      8 DUP (0)       ; Number of valid opens still active

InterruptNum    DB      7               ; default to INT 7 preferred
DataAddr        DD      ?               ; IOCTL data address
ParaAddr        DD      ?               ; IOCTL parameter address

DetectorNum     DB      0               ; Number of installed boards
DetectorModel   DB      8 DUP (0)       ; Model number identifier for each
                                        ; detector
InterfaceModel  DB      8 DUP (0)       ; Model number identifier for each
                                        ; detector interface
SystemMemory    DW      ?               ; System applications memory in Kbytes
MemSlots        DB      8 DUP (0)       ; 2 Mbyte memory slot assignments
                                        ;   -1 = system memory
                                        ; 1..8 = board ID + 1
                                        ;    0 = open

DetectorMemAddr DD      8 DUP (0)       ; 32 bit Physical Base Memory address
                                        ; of detector boards
DetectorMemSize DW      8 DUP (0)       ; Total Board memory size (in KBytes)
DetectorAllocatedMem DW 8 DUP (0)       ; Allocated physical memory size for
                                        ; the boards in Kbytes.
SelArrayPtr     DD      8 DUP (0)       ; starting physical address of virtual
                                        ; pointer arrays for each detector
VirtAllocatedBlocks DW  8 DUP (0)       ; Number of virtual pointer entries
VirtArrayStart  DD      8 DUP (0)       ; virtual array pointers
DetectorPortAddr DW     8 DUP (0)      ; Port addresses of installed boards

FakeFlag        DB      0               ; fake interface board flag
                                        ; 0 = interface present

CharTable       DB      '0123456789ABCDEF' ; for debugging output
CharBuf         DB      132 DUP (0)      ; 132 character buffer
BufLen = $ - CharBuf
CharBuf2        DB      132 DUP (0)      ; 132 character buffer
BufLen2 = $ - CharBuf2
;DumpBuf         DB      132 DUP (0)      ; 132 character buffer
;DumpLen = $ - DumpBuf
CRLF            DB      CR, LF
SPACE           DB      '                                    '

IniFileName     DB      132 DUP (0)     ; .ini filename read in with /I
                                        ; flag at initialization time
LenFName        DW      0
hFile           DW      0               ; file handle
aFile           DW      ?               ; returned action
szFile          DW      0, 0            ; file size      
tempbuf         DB      80 DUP (0)      ; for temporary storage


; OMAHDWR variables
; Variables containing the last bytes written to the OMA board write 
; only registers.
ControlReg      DB      8 DUP (0)
ProgramEntryReg DB      8 DUP (0)
MemoryReg       DB      8 DUP (0)

DetectorBlkID   DD      8 DUP (0) ; ID used to block thread for a detector

ThreadBlocked   DB      0       ; A bit will be set when a thread is blocked 
                                ; waiting for the detector to respond.  
                                ; The bits will correspond to the 8 possible
                                ; detectors.
NoBlock         DB      0       ; A bit will be set when a thread that 
                                ; may be blocked needs to be terminated
                                ; The bits will correspond to the 8 possible
                                ; detectors.
IntPassed       DB      0       ; An interrupt occurred that would have 
                                ; unblocked a thread if one were blocked.


CLOSING         DB      0               ; temporary flag
EVEN
END_OF_DATA     EQU     $

; signon message
SignonMsg       DB      CR, LF, CR, LF, TAB
                DB      'EG&G PARC OMA4 Driver copyright (1988).'
                DB      '  Hi Mom.', CR, LF
SignonLen = $ - SignonMsg
UnknownFlagMsg  DB      '/', 0, ' Unknown Flag', CR, LF
UnknownFlagLen  = $ - UnknownFlagMsg 
SuccessExitMsg  DB      'OMA4 Driver Installed'
                DB      CR, LF, CR, LF  
SuccessMsgLen = $ - SuccessExitMsg
ErrorExitMsg    DB      'OMA4 Driver Not Installed'
                DB      CR, LF, CR, LF    
ErrorMsgLen = $ - ErrorExitMsg

FakeInterfaceMsg DB      'Interface calls will be faked.'
                DB      CR, LF, CR, LF    
FakeMsgLen = $ - FakeInterfaceMsg


MemGrabOKMsg    DB      'PhysToVirt Got the memory',CR, LF
MemGrabOKMsg1Len = $ - MemGrabOKMsg
MemGrabNOKMsg   DB      '********ERROR IN PhysToVirt******',CR, LF
MemGrabNOKMsg1Len = $ - MemGrabNOKMsg
ValidFlags      DB      'INF', 0
NumValidFlags = $ - ValidFlags         ; 256 max
OpenErrorMsg    DB      'Error opening initialization file', CR, LF
OpenErrorLen = $ - OpenErrorMsg        
ReadErrorMsg    DB      'Error reading initialization file', CR, LF
ReadErrorLen = $ - ReadErrorMsg        

Debug          DB      'Debug'
Debugnum        DB      '0', CR, LF
DebugLen = $ - Debug

SetWait         DB      0

FLAGTAB LABEL WORD ; pointers to functions which perform actions depending
                   ; on the config.sys flags

        DW      OFFSET MAIN_CODE:InitFile     ; Detector parameter file
        DW      OFFSET MAIN_CODE:LogInterrupt ; Interrupt number
        DW      OFFSET MAIN_CODE:FakeDetector ; Set internal flags to show 
                                              ; that a detector board is not
                                              ; really present
MAIN_DATA       ENDS

page
        EXTRN DOSPUTMESSAGE:FAR
        EXTRN DOSOPEN:FAR
        EXTRN DOSCLOSE:FAR
        EXTRN DOSREAD:FAR
        EXTRN DOSCHGFILEPTR:FAR        
        EXTRN KBDCHARIN:FAR  


MAIN_CODE       SEGMENT WORD PUBLIC 'CODE'
         ASSUME CS:MAIN_CODE,DS:MAIN_DATA,ES:NOTHING,SS:NOTHING


;============================================================================
;
; - Strategy0...Strategy7 - Board dependent code.
;
;       Device Driver strategy entry point.
;       ENTRY   -       es:bx = Pointer to the request block
;                       ds    = Data Segment for mydevice.sys
;
;       Jumps To Strategy for board independent code

;IF      DEBUG
PUBLIC  Strategy0, Strategy1, Strategy2, Strategy3, Strategy4, Strategy5, 
PUBLIC  Strategy6, Strategy7, Strategy
;ENDIF

Strategy0 PROC    FAR
;IF      DEBUG
;                INT     3
;ENDIF
        MOV     DI, BX                  ; ES:DI -> request packet
        SUB     SP,SIZE LOCALDATA       ; reserve space for LocalData 
                                        ; (Defined in DRVRDEFS.INC file)
        MOV     BP,SP                   ; ...referenced with positive offsets
        MOV     SS:[BP].BOARD_ID,0      ; Save the Board ID for this instance
        JMP SHORT Strategy                ; Jump to Board independent code
Strategy0       ENDP

Strategy1 PROC    FAR
;IF      DEBUG
;                INT     3
;ENDIF
 ;       MOV     [Debugnum], '1'
 ;       PUSH    WORD PTR STDOUT
 ;       PUSH    DebugLen
 ;       PUSH    DS
 ;       PUSH    OFFSET Debug
 ;       CALL    DOSPUTMESSAGE

        MOV     DI, BX                  ; ES:DI -> request packet
        SUB     SP,SIZE LOCALDATA       ; reserve space for LocalData 
                                        ; (Defined in DRVRDEFS.INC file)
        MOV     BP,SP                   ; ...referenced with positive offsets
        MOV     SS:[BP].BOARD_ID,1      ; Save the Board ID for this instance
        JMP SHORT Strategy                ; Jump to Board independent code
Strategy1       ENDP

Strategy2 PROC    FAR
;IF      DEBUG
;                INT     3
;ENDIF
 ;       MOV     [Debugnum], '2'
 ;       PUSH    WORD PTR STDOUT
 ;       PUSH    DebugLen
 ;       PUSH    DS
 ;       PUSH    OFFSET Debug
 ;      CALL    DOSPUTMESSAGE

        MOV     DI, BX                  ; ES:DI -> request packet
        SUB     SP,SIZE LOCALDATA       ; reserve space for LocalData 
                                        ; (Defined in .INC file)
        MOV     BP,SP                   ; ...referenced with positive offsets
        MOV     SS:[BP].BOARD_ID,2      ; Save the Board ID for this instance
        JMP SHORT Strategy                ; Jump to Board independent code
Strategy2       ENDP

Strategy3 PROC    FAR
;IF      DEBUG
;                INT     3
;ENDIF
;        MOV     [Debugnum], '3'
;        PUSH    WORD PTR STDOUT
;        PUSH    DebugLen
;        PUSH    DS
;        PUSH    OFFSET Debug
;        CALL    DOSPUTMESSAGE

        MOV     DI, BX                  ; ES:DI -> request packet
        SUB     SP,SIZE LOCALDATA       ; reserve space for LocalData 
                                        ; (Defined in .INC file)
        MOV     BP,SP                   ; ...referenced with positive offsets
        MOV     SS:[BP].BOARD_ID,3      ; Save the Board ID for this instance
        JMP SHORT Strategy                ; Jump to Board independent code
Strategy3       ENDP

Strategy4 PROC    FAR
;IF      DEBUG
;                INT     3
;ENDIF
;        MOV     [Debugnum], '4'
;        PUSH    WORD PTR STDOUT
;        PUSH    DebugLen
;        PUSH    DS
;        PUSH    OFFSET Debug
;        CALL    DOSPUTMESSAGE
;
        MOV     DI, BX                  ; ES:DI -> request packet
        SUB     SP,SIZE LOCALDATA       ; reserve space for LocalData 
                                        ; (Defined in .INC file)
        MOV     BP,SP                   ; ...referenced with positive offsets
        MOV     SS:[BP].BOARD_ID,4      ; Save the Board ID for this instance
        JMP SHORT Strategy                ; Jump to Board independent code
Strategy4       ENDP

Strategy5 PROC    FAR
;IF      DEBUG
;                INT     3
;ENDIF
;        MOV     [Debugnum], '5'
;        PUSH    WORD PTR STDOUT
;        PUSH    DebugLen
;        PUSH    DS
;        PUSH    OFFSET Debug
;        CALL    DOSPUTMESSAGE

        MOV     DI, BX                  ; ES:DI -> request packet
        SUB     SP,SIZE LOCALDATA       ; reserve space for LocalData 
                                        ; (Defined in .INC file)
        MOV     BP,SP                   ; ...referenced with positive offsets
        MOV     SS:[BP].BOARD_ID,5      ; Save the Board ID for this instance
        JMP SHORT Strategy                ; Jump to Board independent code
Strategy5       ENDP

Strategy6 PROC    FAR
;IF      DEBUG
;                INT     3
;ENDIF
;        MOV     [Debugnum], '6'
;        PUSH    WORD PTR STDOUT
;        PUSH    DebugLen
;        PUSH    DS
;        PUSH    OFFSET Debug
;        CALL    DOSPUTMESSAGE

        MOV     DI, BX                  ; ES:DI -> request packet
        SUB     SP,SIZE LOCALDATA       ; reserve space for LocalData 
                                        ; (Defined in .INC file)
        MOV     BP,SP                   ; ...referenced with positive offsets
        MOV     SS:[BP].BOARD_ID,6      ; Save the Board ID for this instance
        JMP SHORT Strategy                ; Jump to Board independent code
Strategy6       ENDP

Strategy7 PROC    FAR
;IF      DEBUG
;                INT     3
;ENDIF
;        MOV     [Debugnum],'7'
;        PUSH    WORD PTR STDOUT
;        PUSH    DebugLen
;        PUSH    DS
;        PUSH    OFFSET Debug
;        CALL    DOSPUTMESSAGE

        MOV     DI, BX                  ; ES:DI -> request packet
        SUB     SP,SIZE LOCALDATA       ; reserve space for LocalData 
                                        ; (Defined in .INC file)
        MOV     BP,SP                   ; ...referenced with positive offsets
        MOV     SS:[BP].BOARD_ID,7      ; Save the Board ID for this instance
        JMP SHORT Strategy                ; Jump to Board independent code
Strategy7       ENDP

;============================================================================
;
; - Strategy -
;
;       Device Driver strategy entry point.
;       ENTRY   -       es:BX = Pointer to the request block
;                       ds    = Data Segment for mydevice.sys
;                       bp    = start of local data segment       
;
;       EXIT    -       result of operation set in request packet
;
Strategy PROC    FAR
        MOV     DI, BX                  ; ES:DI -> request packet
        MOV     [BP].LD_PTRSAVE._OFF,BX ; Save the virtual address  (ES:BX)
        MOV     [BP].LD_PTRSAVE._SEG,ES ; ...of the Request Packet.
        MOV     BH, 0
        MOV     BL, ES:[DI].PktCmd      ; get the request packet command

        CMP     BL,MAXCMD               ; Make sure its allowed
        JA      Unknown                 ; Jump to exit if bad command.

        ADD     BX,BX                   ; Convert to table index
                                        ; (Word sized instead of byte sized)
        JMP     DISPTAB[BX]             ;Go execute command

;
; On entry to each of the command handlers:
;
;       DS                 = Driver data
;       SS:BP              = LocalData structure
;       SS:[BP].LD_PTRSAVE = Far pointer to the request block, 
;                            which looks like:
;
;               13-BYTE request header
;                   BYTE    length of packet in bytes
;                   BYTE    subunit number of BLOCK device 
;                           (unused by char dev.)
;                   BYTE    command code
;                   WORD    status word (returned by DD)
;                   DWORD   reserved
;                   DWORD   request queue link
;               n BYTES command specific data
;

Strategy       ENDP


;============================================================================
;
; - Exits - All routines return through this path
;
; On transfer to a label within Exits:
;       ss:bp              = LocalData structure
;       ss:[bp].ld_ptrsave = Far pointer to the request block
;
;   Notice that the stack doesn't have to be balanced when you get to here.
; You can be down 6 pushes deep, and jump to here, just so long as SS:BP
; points to the LocalData structure.  SP will be set from BP.
;============================================================================
        PUBLIC  Exits,ExitOK,Unknown,GenFailure, FINAL_EXIT

Exits PROC FAR
    ASSUME DS:MAIN_DATA

ExitOK:                              ;0100h Done, no error
        MOV     AX,STDON
        JMP     SHORT  FINAL_EXIT

Unknown:                              ;8103h Done, 
                                        ;error = "unknown command"
        MOV     AX,STDON+STERR+ERROR_I24_BAD_COMMAND
        JMP     SHORT  FINAL_EXIT

GenFailure:                             ;810Ch Done, 
                                        ;error = "general failure"
        MOV     AX,STDON+STERR+ERROR_I24_GEN_FAILURE
        JMP     SHORT  FINAL_EXIT

;
; All exits come to this point before returning.
;
FINAL_EXIT:

        LDS     BX,[BP].LD_PTRSAVE      ;Load packet address back into DS:BX
        MOV     [BX].PktStatus,AX       ;Mark operation complete

        MOV     SP,BP                   ;Re-Load SP from BP (clean up stack)
        ADD     SP,SIZE LOCALDATA       ;And de-allocate local data space

        RET
Exits ENDP

page

;============================================================================
;
; OMAOpen
;       Gets the PID of the process opening the device and checks to see if 
;       the device is already open.  If it is open by the same process a  
;       counter is incremented.  If it is opened by another process the open
;       request fails.  If it is not open the PID is saved and return DONE
;       
;       Entry:  
;               SS:BP = addr of local data seg
;               SS:[BP].BOARD_ID = device number
;
;============================================================================

IF      DEBUG 
PUBLIC  OMAOpen
ENDIF

OMAOpen PROC NEAR
        XOR     CH, CH
        MOV     CL, [BP].BOARD_ID       ; Get the Board ID for this instance
        CMP     CL, DetectorNum         ; see if trying to open a nonexistant 
                                        ; board
        JB      OMAO0                   ; board present
        JMP SHORT OMAOERROR               ; If board not present
OMAO0:
        ; Get the PID of the calling process
        MOV     AL, 2
        DEVHELP  GetDOSVar       ; pointer to seg of infoseg returned in AX:BX
        MOV     ES, AX
        MOV     AX, ES:[BX]._hi
        MOV     ES, AX
        MOV     AX, ES:[LIS_CurProcID]      ; store the PID for later use
        MOV     CurPID, AX
        ; Get the I/O Port address of the Board
        MOV     BX, CX          ; CX has detector number (word offset)
        SHL     BL, 1           ; calculate the address pointer
        MOV     DX, [BX + DetectorPortAddr]; Put the port address into DX
        ; See if device already opened
        TEST    [BX + OpenNum], 0FFFFH  ; Test for previous openings
        JZ      OPEN_IT                 ; if not already open, do it

        ; already opened at least once
        MOV     BX, CX
        SHL     BX, 1                   ; check PID of previous opening 
                                        ;process
        MOV     AX, [BX + OpeningPID]
        CMP     AX, CurPID              ; Compare to current PID
        JNZ     GenFailure

OPEN_IT:  ; BX has word offset for board, CL has board number
        XOR     BH, BH
        MOV     BL, SS:[BP].BOARD_ID
        SHL     BX, 1                   ; Move to word offset position
        MOV     AX, CurPID              ; Store CurPID into Opening PID
        MOV     [BX + OpeningPID], AX
        INC     [BX + OpenNum]          ; Increment the number of valid 
                                        ; openings
        JMP     ExitOK
        
OMAOERROR:
        POP     DI
        POP     ES
        JMP     GenFailure

OMAOpen ENDP


;============================================================================
;
; OMAClose
;       Checks to see if the closing process is the one that opened
;       that particular detector board.  If it is the open count is 
;       decremented.  Errors are returned if the open count is already
;       zero or if the PID is not the same as the opening PID.
;
;       Entry:  
;               SS:BP = addr of local data seg
;               SS:[BP].BOARD_ID = device number
;
;============================================================================

IF      DEBUG
PUBLIC  OMAClose
ENDIF

OMAClose        PROC NEAR
        ; Get the PID of the calling process
        MOV     AL, 2
        DEVHELP  GetDOSVar       ; pointer to seg of infoseg returned in AX:BX
        MOV     ES, AX
        MOV     AX, ES:[BX]._hi
        MOV     ES, AX
        MOV     AX, ES:[LIS_CurProcID]      ; store the PID for later use
        MOV     CurPID, AX

        XOR     BH, BH
        MOV     BL, [BP].BOARD_ID       ; Get the Board ID for this instance
        SHL     BX, 1                   ; Get the offset for the board's
                                        ; opening PID
        SUB     AX, [OpeningPID + BX]   ; Compare to the process which opened
        JNZ     CloseFail               ; the board
        TEST    [OpenNum + BX], 0FFFFH  ; Check open count to see if it was 
        JZ      CloseFail               ; opened
        DEC     [OpenNum + BX]          ; decrement the open count
        JZ      NOT_OPEN

        JMP     ExitOK
NOT_OPEN:                       ; turn board off and free memory
        CALL    FreeDetMem              ; Clear pointers and free memory
        AND     AX, AX                  ; test for error
        JNZ     CloseFail
        CALL    HideMem                 ; turn off board memory buffer

CloseOUT:
        JMP     ExitOK

CloseFail:
        JMP     GenFailure

OMAClose        ENDP

        INCLUDE DBTOOLS.ASM     ; include some debugging functions
        INCLUDE OMAIOCTL.ASM   ; IOCTL functions
        INCLUDE OMAHDWR.ASM     ; hardware functions



SEC_END_OF_CODE     EQU     $

;============================================================================
;
; - SEC_INIT -
;
;       Device 1..7 intialization code, called at system boot time.
;
; Returns valid end of code and end of data space pointers.
;
;       Entry:  ES:DI and Local data contain the address of the 
;               command packet
;       SS:BP              = LocalData structure
;       SS:[BP].LD_PTRSAVE = Far pointer to the request block.
;       
;       Exit:   Return to the ExitOK entry point.
;
;============================================================================

IF DEBUG
        PUBLIC  SEC_INIT
ENDIF
SEC_INIT PROC NEAR
        ASSUME  DS:MAIN_DATA

        CMP     SS:[BP].BOARD_ID, 7      ; test to see if last initialization
        JZ      SEC_INIT_FINISHED

        ;
        ; RETURN END OF CODE AND END OF DATA for most of the devices
        ;
        LES     DI, [BP].LD_PTRSAVE
        MOV     DX,OFFSET END_OF_DATA
        MOV     ES:[DI].InitEdata,DX
        MOV     DX,OFFSET END_OF_CODE
        MOV     ES:[DI].InitEcode,DX
        ;
        ; SET THE DONE BIT
        ;
        JMP     ExitOK

SEC_INIT_FINISHED:
        ;
        ; CHANGE INIT JUMP ADDRESS for the last device
        ;
SEC_INITADDRCHANGE:
        MOV     BX, OFFSET DISPTAB
        MOV     DX, OFFSET MAIN_CODE:GenFailure
        MOV     [BX],DX
        ;
        ; RETURN END OF CODE AND END OF DATA
        ;
        LES     DI, [BP].LD_PTRSAVE
        MOV     DX, OFFSET END_OF_DATA
        MOV     ES:[DI].InitEdata,DX
        MOV     DX, OFFSET SEC_END_OF_CODE
        MOV     ES:[DI].InitEcode,DX
        JMP     ExitOK        

SEC_INIT        ENDP



;============================================================================
;
; - INIT -
;
;       One time intialization code, called at system boot time.
;
; The Init routine will be called to set up the DevHlp address
; and do whatever other initialization is necessary.
;
;       Entry:  ES:DI and Local data contain the address of the 
;               command packet
;       
;       Exit:   Return to the correct exit entry point based on whether
;               we were able to install or not.
;
;============================================================================

END_OF_CODE     EQU     $

IF DEBUG
        PUBLIC  INIT
ENDIF
INIT PROC NEAR
        ASSUME  DS:MAIN_DATA

        CLD                     ; set block move direction flag to increment

        ;
        ; SAVE PTR TO DEVHELP
        ;
        MOV     DX,ES:[DI].IOpData._hi
        MOV     [DEVHLP]._hi,DX
        MOV     DX,ES:[DI].IOpData._lo
        MOV     [DEVHLP]._lo,DX

        ;
        ; GET ADDRESS OF TCYield
        ;
        MOV     AL,7
        DEVHELP  GetDOSVar
        MOV     word ptr YieldAddr._hi,AX
        MOV     word ptr YieldAddr._lo,BX


        PUSH    WORD PTR STDOUT
        PUSH    SignonLen
        PUSH    DS
        PUSH    OFFSET SignonMsg
        CALL    DOSPUTMESSAGE

        CALL    ReadExpMemSize

        ;
        ; READ FLAGS FROM CONFIG.SYS "DEVICE=" CALL
        ;
        ;       The InitParms location in the request packet points to 
        ;       a string that shows what came after the "device=" in 
        ;       CONFIG.SYS.  Spaces between the = and the device filename
        ;       are stripped.  The string is 0 terminated at the newline.
        ;       
        ; FLAGS:   /Ifilename or /ifilename - spaces are ignored
        ;               Detector parameter filename
        ;          /f or /F - No detector board attached. Fake it using
        ;               info in detector parameter file as if there 
        ;               really was a detector.
        ;          '-' equivalent to '/'
        ; 
        ; Get initialization file name.  
        ;
HANDLE_FLAG:
        LES     DI, ES:[DI].InitParms
FIND_NEXT_FLAG:       
        CALL    StrReadToSpace     ; read to next ' ', TAB or NULL
        CMP     BYTE PTR ES:[DI], 0     ; see if at EOL marker
        JZ      FLAGS_DONE
        CALL    IgnoreSpaces

FLAG_ACTION:    ; ES:[DI] point to possible flag string. 
        CMP     BYTE PTR ES:[DI], '/' ; check for '/' flag indicator
        JZ      WHICH_FLAG
        CMP     BYTE PTR ES:[DI], '-' ; check for '-' flag indicator
        JZ      WHICH_FLAG
        
        JMP     INIT_EXIT_ERROR
WHICH_FLAG:                     ; check for valid flag characters
        INC     DI              ; point to next letter after flag indicator
        MOV     AL, ES:[DI]     ; get the char
        AND     AL, 5FH         ; capitalize letter

;        MOV     CharBuf2 , AL
;        PUSH    DS              ; segment
;        PUSH    OFFSET CharBuf2 ; offset
;        PUSH    8H              ; length 
;        CALL    MemDump

        PUSH    ES
        PUSH    DI

        PUSH    DS
        POP     ES
        MOV     DI, OFFSET ValidFlags   ; point to the flag string
        MOV     CX, NumValidFlags       ; get the number of valid flags for
                                        ; the search
        REPNE SCASB                     ; find the offset of the command match
        ; CL gets decremented past last matching flag

        POP     DI                      ; get flag string pointer back
        INC     DI                      ; Point past flag ID
        POP     ES
        JCXZ    UNKNOWN_FLAG            ; no match if CX = 0
        MOV     BX, NumValidFlags - 1  ; calculate the correct offset
        SUB     BL, CL
        SHL     BX, 1        
        CALL  FLAGTAB[BX]             ; do an indirect call to routine
                                      ; should return with ES:DI pointing
                                      ; past this flag's info    
        AND     AX, AX                ; check for error
        JNZ     INIT_EXIT_ERROR
        
;        INC     [Debugnum]
;        PUSH    WORD PTR STDOUT
;        PUSH    DebugLen
;        PUSH    DS
;        PUSH    OFFSET Debug
;        CALL    DOSPUTMESSAGE
            ; returns with ES:DI pointing after this flag's string portion
        JMP     FIND_NEXT_FLAG

FLAGS_DONE:
        PUSH    DS              ; segment
        PUSH    OFFSET DEVHLP ; offset
        PUSH    150H              ; length 
        CALL    MemDump

        CALL    SetInterrupt            ; set to whatever is in InterruptNum
                                        ; if possible
        AND     AX, AX                  ; check for error
        JNZ     INIT_EXIT_ERROR

        ;
        ; Try to grab some memory.
        ;
        ; AX-BX points to the start of the physical memory
;        MOV     AX, 2FH
;        MOV     BX, 0
;        MOV     CX, 0           ; 64K length
;        MOV     DH, ToES_DI    ; 
;        DEVHELP PhysToVirt     ; Virt Address returned as ES:DI
;        JC      MemGrabError

        ;
        ; DISPLAY OK MESSAGE
        ;
;        PUSH    WORD PTR STDOUT
;        PUSH    MemGrabOKMsg1Len
;        PUSH    DS
;        PUSH    OFFSET MemGrabOKMsg
;        CALL    DOSPUTMESSAGE
;        JMP     INITADDRCHANGE
;MemGrabError:
;        ;
;        ; DISPLAY ERROR MESSAGE
;        ;
;        PUSH    WORD PTR STDOUT
;        PUSH    MemGrabNOKMsg1Len
;        PUSH    DS
;        PUSH    OFFSET MemGrabNOKMsg
;        CALL    DOSPUTMESSAGE
;        
        ;
        ; CHANGE INIT JUMP ADDRESS
        ;
INITADDRCHANGE:
        MOV     BX, OFFSET DISPTAB
        MOV     DX, OFFSET MAIN_CODE:SEC_INIT ; redirect other device 
        MOV     [BX],DX             ; initializations to a dummy routine


INIT_EXIT_OK:
        PUSH    WORD PTR STDOUT
        PUSH    SuccessMsgLen
        PUSH    DS
        PUSH    OFFSET SuccessExitMsg
        CALL    DOSPUTMESSAGE

        TEST    FakeFlag, 0FFH         
        JZ      EOC_EOD

        PUSH    WORD PTR STDOUT
        PUSH    FakeMsgLen
        PUSH    DS
        PUSH    OFFSET FakeInterfaceMsg
        CALL    DOSPUTMESSAGE

EOC_EOD:
        ;
        ; RETURN END OF CODE AND END OF DATA
        ;
        LES     DI, [BP].LD_PTRSAVE
        MOV     DX,OFFSET END_OF_DATA
        MOV     ES:[DI].InitEdata,DX
        MOV     DX,OFFSET END_OF_CODE
        MOV     ES:[DI].InitEcode,DX
        ;
        ; SET THE DONE BIT
        ;
        JMP     ExitOK

UNKNOWN_FLAG:
        ; AL still has flag
        MOV     [UnknownFlagMsg + 1], AL
        PUSH    WORD PTR STDOUT
        PUSH    UnknownFlagLen
        PUSH    DS
        PUSH    OFFSET UnknownFlagMsg
        CALL    DOSPUTMESSAGE
INIT_EXIT_ERROR:
        PUSH    WORD PTR STDOUT
        PUSH    ErrorMsgLen
        PUSH    DS
        PUSH    OFFSET ErrorExitMsg
        CALL    DOSPUTMESSAGE
        ;
        ; CHANGE INIT JUMP ADDRESS
        ;
        MOV     BX, OFFSET DISPTAB
        MOV     DX, OFFSET MAIN_CODE:ExitOK
        MOV     [BX],DX
        ;
        ; RETURN END OF CODE AND END OF DATA
        ;
        LES     DI, [BP].LD_PTRSAVE
        MOV     DX, 0
        MOV     ES:[DI].InitEdata,DX
        MOV     DX, 0
        MOV     ES:[DI].InitEcode,DX
        JMP     GenFailure        

INIT        ENDP

;
; InitFile
;       Read in detector parameters from a file.
;       ENTRY -  ES:[DI] points just after flag identifier
;       EXIT -   ES:[DI] pointers just after filename
;                AX = 0 ; No errors
;                     1 ; Error
;
IF DEBUG
        PUBLIC  InitFile
ENDIF
InitFile PROC NEAR
        CALL    IgnoreSpaces   ; read past any intervening spaces or tabs
        CMP     BYTE PTR ES:[DI], 0     ; check for string terminator
        JNZ     BUF_XFER        ; file name present
        JMP     IFERROREXIT      ; No filename        
BUF_XFER:       ; transfer filename to a buffer
        PUSH    DS              ; set up pointers for string transfer
        MOV     AX, ES
        MOV     DS, AX
        POP     ES
        MOV     SI, DI        
        MOV     DI, OFFSET IniFileName
        XOR     CX, CX                  ; CX will keep count of length
XFERLP:
        INC     CX
        MOVSB                   ; ES:[DI] <- DS:[SI]
        CMP     BYTE PTR DS:[SI], ' '    ; If char value greater than ' '
        JA      XFERLP          ; not done yet.
        MOV     LenFName, CX    ; CX is length of filename + 1, the +1 will
                                ; be used to put in the null terminator
        PUSH    DS              ; get pointers back in proper place
        MOV     AX, ES
        MOV     DS, AX
        POP     ES
        XCHG    DI, SI

        PUSH    ES              ; save pointer past file name
        PUSH    DI

        MOV     BYTE PTR [SI], 0 ; put null terminator onto buffer string
OPEN_INIT_FILE:
        ; filename pointer
        PUSH    DS
        PUSH    OFFSET IniFileName
        ; file handle pointer
        PUSH    DS
        PUSH    OFFSET hFile
        ; action pointer
        PUSH    DS
        PUSH    OFFSET aFile
        ; file size pointer
        PUSH    DS
        PUSH    OFFSET szFile
        ; file attribute
        PUSH    0
        ; open flags - If file exists open it, if file doesn't exist fail.
        PUSH    1
        ; open in read-only, share mode
        PUSH    020H
        ; DWORD reserved   
        PUSH    0
        PUSH    0
        CALL    DOSOPEN         ; error returned in AX
        OR      AX, AX
        JNZ     OPENERROR

        ; Read in detector parameters for as many detectors as listed
        ; in the parameter file.  Fails when the next contiguous detector
        ; is not found or an error occurs when trying to read it's parameters
        ; stores the number of good detector parameter blocks it has found
        ; in DetectorNum.  Error exit if 0 good detector parameter blocks
        ; were found.
        XOR     AX, AX          ; find detector 0 params
DETNUMLP:
        PUSH    AX        
        CALL    ReadParams
        AND     AX, AX          ; check for errors
        POP     AX
        JNZ     DETDONE         ; refers to AND one instruction back
        INC     AL              ; read next detector
        JMP SHORT DETNUMLP
DETDONE:    
        MOV     [DetectorNum], AL  ; store number of good parameter blocks
        AND     AL, AL          ; test for any good parameter blocks          
        JNZ     IFEXITOK        ; something good was read in

        ; Detector 0 parameter block in error

READ_ERROR:
        POP     DI              ; flag string pointers
        POP     ES

        PUSH    WORD PTR STDOUT
        PUSH    ReadErrorLen
        PUSH    DS
        PUSH    OFFSET ReadErrorMsg
        CALL    DOSPUTMESSAGE
        ; close file if error in reads
        ; file handle
        PUSH    hFile
        CALL    DOSCLOSE
        JMP SHORT IFERROREXIT
OPENERROR:
        POP     DI                      ; flag string pointers
        POP     ES      

        PUSH    WORD PTR STDOUT
        PUSH    OpenErrorLen
        PUSH    DS
        PUSH    OFFSET OpenErrorMsg
        CALL    DOSPUTMESSAGE

IFERROREXIT:
        MOV     AX, 1
        RET
IFEXITOK:
        ; close file
        ; file handle 
        PUSH    hFile
        CALL    DOSCLOSE
        POP     DI              ; flag string pointers
        POP     ES
        XOR     AX, AX
        RET

InitFile ENDP

;
; ReadParams
;       Read the parameters for the detector.
;       ENTRY - Opened file handle in hFile
;               Detector number in AX   0..7
;            PARAMETER ORDER IN INITIALIZATION FILE
;               1. Interface type ( ID number(see list (when made)), 255 max )
;               2. Starting memory address
;               3. Available memory
;               4. Starting port address
;               5. Detector type ( ID number(see list (when made)), 255 max )
;               
;       RETURN - AX = 0 No error
;                   = 1 Error    
;          
 
IF DEBUG
        PUBLIC  ReadParams
ENDIF
ReadParams PROC NEAR
        ASSUME  DS:MAIN_DATA
        PUSH    BP
        MOV     BX, OFFSET CharTable    ; change the detector number in AL
        XLAT                            ; to an ASCII number
        PUSH    AX              ; save ASCII detector number
        ; position file pointer at the start of the file
        PUSH    hFile
        ; distance to move
        PUSH    0
        PUSH    0
        ; relative to start
        PUSH    0
        ; pointer to szFile
        PUSH    DS
        PUSH    OFFSET szFile
        CALL    DOSCHGFILEPTR
        AND     AX, AX          ; test for error
        JZ      BLOCKFIND
        JMP     RPERROREXIT

        ; look for the correct detector parameter block
BLOCKFIND: 
        ; Find the start of the next detector parameter block
        MOV     BP, SP
        PUSH    0A0AH           ; LF
        PUSH    0D0DH           ; CR
        MOV     AH, ';'         ; If comment comes out first, ignore line
        MOV     AL, ';'
        PUSH    AX
        MOV     AH, '@'         ; Assume '@' used only to denote start of 
        MOV     AL, '@'         ; detector info block
        PUSH    AX
        PUSH    4               ; Number of ranges
        CALL    SetPosNext
        MOV     SP, BP          ; Clear stack
        
        ; see which char was found
        CMP     AL, ';'
        JZ      PBNEWLINE
        CMP     AL, 0AH
        JZ      PBNEWLINE
        CMP     AL, 0DH
        JNZ     FOUND_PBSTART

PBNEWLINE:
        CALL    ReadToNextLine
        AND     AL, AL          ; Check for read error or EOF
        JNZ     BFLP            ; no error
        JMP     RPERROREXIT
BFLP:
        JMP     BLOCKFIND
        

FOUND_PBSTART:
        ; found the start of a parameter block, next read char is '@' 
        ; followed by a detector number
        PUSH    hFile
        ; pointer to input buffer
        PUSH    DS
        PUSH    OFFSET CharBuf
        ; number of bytes to read
        PUSH    2
        ; pointer to number of bytes read
        PUSH    DS
        PUSH    OFFSET aFile
        CALL    DOSREAD
        AND     AX, AX          ; check for error
        JZ      CHECK_EOF       ; no error
        JMP SHORT RPERROREXIT
CHECK_EOF:
        TEST    aFile, 0FFFFH
        JNZ     CHECK_DET       ; not at EOF
        JMP SHORT RPERROREXIT      ; Reached end of file

CHECK_DET:
        POP     AX              ; Get detector number back
        PUSH    AX              ; and save it again

;        MOV     CharBuf, AL
;        PUSH    DS              ; segment
;        PUSH    OFFSET CharBuf ; offset
;        PUSH    2H              ; length 
;        CALL    MemDump
        

        CMP     AL, [CharBuf+1]     ; Compare to detector number read in
        JZ      RIGHTDETECTOR
        JMP     BLOCKFIND       ; look again if not right one

RIGHTDETECTOR:
FillParms:      ; Read in the parameters and put in proper variables
        POP     AX              ; read back ASCII rep of detector number
        SUB     AX, '0'         ; convert back to actual number
        PUSH    AX              ; save it again
        
READINTERFACEID:
        MOV     BP, SP          ; save stack position
        ; read in Unsigned Hex
        PUSH    1
        ; Char - 1 byte
        PUSH    1
        ; Address of Interface for this detector
        PUSH    DS
        ADD     AX, OFFSET InterfaceModel
        PUSH    AX
        CALL    GetNextParam
        MOV     SP, BP          ; clean up stack
        AND     AL, AL          ; check for error                      
        JNZ     RPERROREXIT     ; bad number

        POP     AX              ; get back detector number
        PUSH    AX              ; save it again
;READMEMADDR:
;        MOV     [Debugnum], '1'
;        PUSH    WORD PTR STDOUT
;        PUSH    DebugLen
;        PUSH    DS
;        PUSH    OFFSET Debug
;        CALL    DOSPUTMESSAGE
;        MOV     BP, SP          ; save stack position
;        ; read in Unsigned hex
;        PUSH    1
;        ; Long - 4 bytes
;        PUSH    4
;        ; Address of DetectorMemAddr for this board
;        PUSH    DS
;        SHL     AX, 2           ; get addr offset
;        ADD     AX, OFFSET DetectorMemAddr
;        PUSH    AX
;        CALL    GetNextParam
;        MOV     SP, BP          ; clean up stack
;        AND     AL, AL          ; check for error                      
;        JNZ     RPERROREXIT     ; bad number

;        POP     AX              ; get back detector number
;        PUSH    AX              ; save it again
READMEMSZ:
;        MOV     [Debugnum], '2'
;        PUSH    WORD PTR STDOUT
;        PUSH    DebugLen
;        PUSH    DS
;        PUSH    OFFSET Debug
;        CALL    DOSPUTMESSAGE

        MOV     BP, SP          ; save stack position
        ; read in Unsigned hex
        PUSH    1
        ; Word - 2 bytes
        PUSH    2
        ; Address of DetectorMemSize for this board
        PUSH    DS
        SHL     AX, 1           ; get addr offset
        ADD     AX, OFFSET DetectorMemSize
        PUSH    AX
        CALL    GetNextParam
        MOV     SP, BP          ; clean up stack
        AND     AL, AL          ; check for error                      
        JNZ     RPERROREXIT     ; bad number

        POP     AX              ; get back detector number
        PUSH    AX              ; save it again
READPORTADDR:
;        MOV     [Debugnum], '3'
;        PUSH    WORD PTR STDOUT
;        PUSH    DebugLen
;        PUSH    DS
;        PUSH    OFFSET Debug
;        CALL    DOSPUTMESSAGE
        MOV     BP, SP          ; save stack position
        ; read in Unsigned hex
        PUSH    1
        ; WORD - 2 bytes
        PUSH    2
        ; Address of DetectorPortAddr for this board
        PUSH    DS
        SHL     AX, 1           ; get addr offset
        ADD     AX, OFFSET DetectorPortAddr
        PUSH    AX
        CALL    GetNextParam
        MOV     SP, BP          ; clean up stack
        AND     AL, AL          ; check for error                      
        JNZ     RPERROREXIT     ; bad number

        POP     AX              ; get back detector number
        PUSH    AX              ; save it again
READDETECTORID:
        MOV     BP, SP          ; save stack position
        ; read in Unsigned Hex
        PUSH    1
        ; Char - 1 byte
        PUSH    1
        ; Address of Interface for this detector
        PUSH    DS
        ADD     AX, OFFSET DetectorModel
        PUSH    AX
        CALL    GetNextParam
        MOV     SP, BP          ; clean up stack
        AND     AL, AL          ; check for error                      
        JNZ     RPERROREXIT     ; bad number

        POP     AX              ; get back detector number
        PUSH    AX              ; save it again

;************************************************************************
;*                                                                      * 
;*    ADD IN NEW PARAMETERS HERE                                        *
;*                                                                      * 
;************************************************************************
        JMP SHORT RPEXITOK

RPERROREXIT:
;        PUSH    DS              ; segment
;        PUSH    OFFSET DetectorMemAddr ; offset
;        PUSH    4H              ; length 
;        CALL    MemDump

;        PUSH    DS              ; segment
;        PUSH    OFFSET DetectorMemSize ; offset
;        PUSH    4H              ; length 
;        CALL    MemDump


        POP     AX
        POP     BP
        MOV     AX, 1
        RET
RPEXITOK:
;        MOV     [Debugnum], '5'
;        PUSH    WORD PTR STDOUT
;        PUSH    DebugLen
;        PUSH    DS
;        PUSH    OFFSET Debug
;        CALL    DOSPUTMESSAGE
;        PUSH    DS              ; segment
;        PUSH    OFFSET DetectorMemAddr ; offset
;        PUSH    4H              ; length 
;        CALL    MemDump
;
;        PUSH    DS              ; segment
;        PUSH    OFFSET DetectorMemSize ; offset
;        PUSH    4H              ; length 
;        CALL    MemDump

        POP     AX
        POP     BP
        XOR     AX, AX
        RET
ReadParams ENDP

;
;GetNextParam
;       Read file up to the start of the next valid hex character.
;       ENTRY   hFile contains opened file handle
;               SP + 8 - parameter type
;                        0 = ALL alphanumeric and punctuation > SPACE and
;                            not ';' or after it on a line   
;                        1 = Unsigned hex number                        
;               SP + 6 - Number of bytes
;               SP + 4 - Destination Segment       
;               SP + 2 - Destination offset
;               
;       EXIT    AX = 1  ERROR
;               AX = 0  OK
;               file pointer set to next uncommented ascii character
;
IF DEBUG
        PUBLIC GetNextParam
ENDIF
GetNextParam PROC NEAR
        PUSH    BP
        MOV     BP, SP

        CALL    ReadToNextLine
        AND     AL, AL          ; check to see if found
        JZ      GNPERROR     ; unexpected end of file

        TEST    BYTE PTR SS:[BP+10], 1   ; See if looking for Hex number
        JZ      ANYCHAR
        ; Read up to next ASCII hex number character '0'..'F'
        ; Assume its the first usable char on the line
        CALL    ReadToNextHex
        AND     AL, AL          ; check to see if found
        JZ      GNPERROR     ; unexpected end of file

        ; Found an acceptable starting char
        POP     BX              ; get back detector number
        PUSH    BX              ; save it for later
        MOV     BP, SP
        ; set up call
        MOV     AX, SS:[BP + 8] 
        PUSH    AX               ; number of bytes to fill
        MOV     AX, SS:[BP + 6] 
        PUSH    AX              ; addr segment
        MOV     AX, SS:[BP + 4] 
        PUSH    AX              ; addr offset
        CALL    ReadUnsigned
        MOV     SP, BP          ; clean up stack
        AND     AL, AL          ; check for error                      
        JNZ     GNPERROR        ; bad number
        JMP SHORT GNPEXIT
ANYCHAR:
        ;*************************************************
        ;*      NOT IMPLEMENTED YET                      *
        ;*************************************************

GNPERROR:
        MOV     AX, 1
        POP     BP
        RET
GNPEXIT:
        XOR     AX, AX
        POP     BP
        RET

GetNextParam    ENDP


;
; LogInterrupt
;       Read in the preffered interrupt number (in HEX). If the preferred 
;       interrupt is unavailable other possibilities will be tried before
;       giving up with an error.
;       ENTRY -  ES:[DI] points just after flag identifier
;       EXIT -   ES:[DI] pointers just the interrupt number
;                AX = 0 ; No errors
;                     1 ; Error
IF DEBUG
        PUBLIC  LogInterrupt
ENDIF
LogInterrupt PROC NEAR
        CALL    IgnoreSpaces   ; read past any intervening spaces or tabs
        CMP     BYTE PTR ES:[DI], 0     ; check for string terminator
        JNZ     GETINTNUMBER        ; interrupt number (IN HEX) present
        JMP SHORT LIERROREXIT      ; No interrupt number

GETINTNUMBER:
        XOR     AH, AH
        MOV     AL, BYTE PTR ES:[DI]
        CMP     AL, '9'         ; see if 0..9 or A..F
        JG      GOTLETTER       ; if A..F
        CMP     AL, '0'         ; see if bad character
        JB      LIERROREXIT

        ; got 0..9
        SUB     AL, '0'
        JMP SHORT SET_INTERRUPT

GOTLETTER:
        CMP     AL, 'A'         ; see if bad character
        JB      LIERROREXIT
        AND     AL, 0DFH        ; capitalize the letter
        CMP     AL, 'F'         ; see if bad character
        JA      LIERROREXIT
        SUB     AL, 'A' - 10    ; change to value
        MOV     [InterruptNum], AL      ; save preferred level
        ; SetInterrupt will be called later by initialization routine
LIEXITOK:
        CALL    StrReadToSpace     ; read to next ' ', TAB or NULL
        XOR     AX, AX
        RET

LIERROREXIT:
        MOV     AX, 1
        RET
LogInterrupt    ENDP

;
; SetInterrupt
;       Set up the interrupt request handler. If the preferred 
;       interrupt is unavailable other possibilities will be tried before
;       giving up with an error.
;       ENTRY - InterruptNum is set to the preferred interrupt number
;       EXIT -   InterruptNum is set to the interrupt obtained, if any.
;                AX = 0 ; No errors
;                     1 ; Error
IF DEBUG
        PUBLIC  SetInterrupt
ENDIF
SetInterrupt PROC NEAR
        XOR     BH, BH
        MOV     BL, [InterruptNum]
        TEST    FakeFlag, 0FFH          ; no real hardware attached
        JNZ     SIEXITOK
        MOV     DH, INT_NOT_SHARED      ; try to get lone posession first
SET_INTERRUPT:
        MOV     AX, OFFSET OMAIntHand        
        DEVHELP SetIRQ                  ; try to get the interrupt
        JNC     SIEXITOK                ; got it
        ; didn't work so try other interrupts
        MOV     BX, 7
        MOV     AX, OFFSET OMAIntHand        
        DEVHELP SetIRQ                  ; try to get the interrupt
        JNC     SIEXITOK                ; got it
        ; didn't work so try 5
        MOV     BX, 5
        MOV     AX, OFFSET OMAIntHand        
        DEVHELP SetIRQ                  ; try to get the interrupt
        JNC     SIEXITOK                ; got it
        ; didn't work so try 4
        MOV     BX, 4
        MOV     AX, OFFSET OMAIntHand        
        DEVHELP SetIRQ                  ; try to get the interrupt
        JNC     SIEXITOK                ; got it
        ; didn't work so try sharing
        ; first see if already tried sharing
        CMP     DH, INT_SHARED
        JZ      SIERROREXIT             ; it already was tried
        MOV     DH, INT_SHARED
        MOV     BL, [InterruptNum]      ; get back preferred value
        JMP SHORT SET_INTERRUPT

SIEXITOK:
        CALL    SetBrdInt               ; tell the hardware which interrupt        
        MOV     [InterruptNum], BL
        XOR     AX, AX
        RET

SIERROREXIT:
        MOV     AX, 1
        RET
SetInterrupt    ENDP


;
; FakeDetector
;       Set the FakeFlag which will stop software from sending IN and
;       OUT to hardware which is not really there.
;       ENTRY -  ES:[DI] points just after flag identifier
;       EXIT -   ES:[DI] pointers just after flag identifier
;                AX = 0 ; No errors

IF DEBUG
        PUBLIC  FakeDetector
ENDIF
FakeDetector PROC NEAR
        MOV     [FakeFlag], -1
        XOR     AX, AX
        RET
FakeDetector    ENDP

        INCLUDE SMTOOLS.ASM    ; which includes some helpful functions


MAIN_CODE       ENDS
                END
;************************************************************************
;*                              END                                     *
;************************************************************************

