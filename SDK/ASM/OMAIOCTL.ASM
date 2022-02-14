;       IOCTL constants and variables

; category codes
OMA4CNTRL       EQU     0C0H
; Function codes
RET_ERR         EQU     80H     ; return error if unknown function flag
; Query Codes
QUERYINIFILE    EQU     020H    ; get filename of init file as put in
                                ; device initialization params
QUERYDETNUM     EQU     021H    ; get the number of detectors attached
QUERYOMATYPE    EQU     022H    ; see what kind of OMA interface is attached
                                ; as set by device initialization flag
QUERYDETTYPE    EQU     023H    ; return detector model number
QUERYMEMADDR    EQU     024H    ; Return a virtual address to an array of
                                ; virtual pointers to a detector board's
                                ; memory space
QUERYMEMSIZE    EQU     025H    ; Return the size in Kbytes of a detector
                                ; board's full memory.
QUERYPORTADDR   EQU     026H    ; Return the base port address for a 
                                ; detector board
QUERYDATASEG    EQU     027H    ; copy the device driver's data seg to a 
                                ; buffer 
QUERYPHYSADDR   EQU     028H    ; Return the physical address of a detector 
                                ; board's memory space

; Send Codes
ACTIVATEDETECTOR EQU    000H    ; Activate the detector to use the next 
                                ; available address block (2M boundaries).
                                ; Return error if no memory space left.
DEACTIVATEDETECTOR EQU  001H    ; free memory and turn off board

BLOCKFORMSG     EQU     002H    ; Block and wait for the interface board to
                                ; ask for service

STOPACQUISITION EQU     003H    ; Unblock any blocked thread for a detector
                                ; and have it return an EXPSTOP message

; 1/10/91 DAI added for OMA4000 development
;Additional codes for debugging
GET_SEG_ABSOLUTE EQU    010H    ; create a protected mode selector for a
                                ; given address
DEL_SEG_ABSOLUTE EQU    011H    ; delete the created protected mode selector

;/***************************************************************************/
;/*                        device driver messages                           */
;/***************************************************************************/

PROCEND         EQU     0       ; process ended
EXPSTOP         EQU     1       ; stop the data acquisition process
CURVESREADY     EQU     2       ; Curves are ready to be transferred off
                                ; of the interface board
TIMEDOUT        EQU     3       ; timed out of block

; from devsym.inc
;CMDGenIOCTL	EQU	16			; Generic IOCTL
;GIOCategory	EQU	BYTE PTR PktData	; Category Code
;GIOFunction	EQU	BYTE PTR PktData+1	; Function code
;GIOParaPack	EQU	DWORD PTR PktData+2	; pointer to parameter packet
;GIODataPack	EQU	DWORD PTR PktData+6	; pointer to data packet
;LENGenIOCTL	EQU	12			; Packet is 12 bytes long

;       ENTRY   ES:[DI] points to request packet.
;               [BP].BOARD_ID points to current detector number

IF DEBUG
        PUBLIC  OMAIOCTL
ENDIF
OMAIOCTL        PROC    NEAR
        ASSUME  DS:MAIN_DATA

        XOR     AH, AH
        MOV     AL, SS:[BP].BOARD_ID
        CMP     ES:[DI].GIOCategory, OMA4CNTRL
        JZ      CATEGORY_OK
UNK_CAT:
        MOV     AX, STDON OR STERR OR ERROR_INVALID_CATEGORY
        JMP     FINAL_EXIT
        
CATEGORY_OK:
        CMP     ES:[DI].GIOFunction, QUERYINIFILE
        JNZ     IOCTL1
        JMP     QueryIniFile
IOCTL1:
        CMP     ES:[DI].GIOFunction, QUERYDETNUM
        JNZ     IOCTL2
        JMP     QueryDetNum
IOCTL2:
        CMP     ES:[DI].GIOFunction, QUERYOMATYPE
        JNZ     IOCTL3
        JMP     QueryOMAType
IOCTL3:
        CMP     ES:[DI].GIOFunction, QUERYDETTYPE
        JNZ     IOCTL4
        JMP     QueryDetType
IOCTL4:
        CMP     ES:[DI].GIOFunction, QUERYMEMADDR
        JNZ     IOCTL5
        JMP     QueryMemAddr
IOCTL5:
        CMP     ES:[DI].GIOFunction, QUERYMEMSIZE
        JNZ     IOCTL6
        JMP     QueryMemSize
IOCTL6:
        CMP     ES:[DI].GIOFunction, QUERYPORTADDR
        JNZ     IOCTL7
        JMP     QueryPortAddr
IOCTL7:
        CMP     ES:[DI].GIOFunction, QUERYDATASEG
        JNZ     IOCTL8
        JMP     QueryDataSeg
IOCTL8:
        CMP     ES:[DI].GIOFunction, QUERYPHYSADDR
        JNZ     IOCTL9
        JMP     QueryPhysAddr
IOCTL9:
        CMP     ES:[DI].GIOFunction, ACTIVATEDETECTOR
        JNZ     IOCTL10
        JMP     ActivateDetector
IOCTL10:
        CMP     ES:[DI].GIOFunction, DEACTIVATEDETECTOR
        JNZ     IOCTL11
        JMP     DeActivateDetector
IOCTL11:
        CMP     ES:[DI].GIOFunction, BLOCKFORMSG
        JNZ     IOCTL12
        JMP     BlockForMsg

IOCTL12:
        CMP     ES:[DI].GIOFunction, STOPACQUISITION
        JNZ     IOCTL13
        JMP     StopAcquisition

IOCTL13:
        CMP     ES:[DI].GIOFunction, GET_SEG_ABSOLUTE
        JNZ     IOCTL14
        JMP     GetSegAbsolute

IOCTL14:
        CMP     ES:[DI].GIOFunction, DEL_SEG_ABSOLUTE
        JNZ     IOCTL15
        JMP     DelSegAbsolute

; additional functions go here
IOCTL15:

; errors
UNK_FUNC:
        JMP     Unknown

OMAIOCTL        ENDP

; Function codes

; Query Codes
;
; QueryIniFile
;       Copy filename of init file as put in the device initialization params
;       into buffer pointed to by GIODataPack.  Buffer must be at least 132
;       chars long.
;           
;       ENTRY   ES:[DI] points to request packet.
;               GIODataPack points to 132 char buffer.
;       EXIT    Copies null terminated filename string into buffer pointed to 
;               by GIODataPack.
;
IF DEBUG
        PUBLIC QueryIniFile
ENDIF
QueryIniFile PROC NEAR
        MOV     AX, ES:[DI].GIODataPack._SEG
        MOV     BX, ES:[DI].GIODataPack._OFF
        MOV     ES, AX
        MOV     DI, BX
        MOV     CX, 132                 ; copy over whole buffer
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        JNC     QIF1
        JMP     GenFailure
QIF1:
        MOV     SI, OFFSET IniFileName
        REP MOVSB
        JMP     ExitOK

QueryIniFile ENDP

;
; QueryDetNum
;       Get the number of detectors attached
;           
;       ENTRY   ES:[DI] points to request packet.
;               GIODatPack points to a byte location for the return value.
;       EXIT    Returns the value of the number of detectors in the byte 
;               pointed to by GIODataPack.
;
IF DEBUG
        PUBLIC QueryDetNum
ENDIF
QueryDetNum PROC NEAR
        MOV     AX, ES:[DI].GIODataPack._SEG
        MOV     DI, ES:[DI].GIODataPack._OFF
        MOV     ES, AX
        MOV     CX, 1                   ; 1 byte
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        JNC     QDN1
        JMP     GenFailure
QDN1:
        MOV     AL, [DetectorNum]
        MOV     ES:[DI], AL
        JMP     ExitOK
QueryDetNum ENDP

;
; QueryOMAType
;       Get OMA interface model number
;           
;       ENTRY   ES:[DI] points to request packet.
;               Current detector number in AX
;               GIODatPack points to a byte location for the return value.
;       EXIT    Returns the ID value of the detector interface in the byte 
;               pointed to by GIODataPack.
;
IF DEBUG
        PUBLIC QueryOMAType
ENDIF
QueryOMAType PROC NEAR
        PUSH    AX              ; save detector number
        MOV     AX, ES:[DI].GIODataPack._SEG
        MOV     DI, ES:[DI].GIODataPack._OFF
        MOV     ES, AX
        MOV     CX, 1                   ; byte
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        POP     AX                      ; get detector number back
        JNC     QOT1
        JMP     GenFailure
QOT1:
        ADD     AX, OFFSET InterfaceModel  ; add detector number to get offset
        MOV     SI, AX
        MOV     AL, [SI]
        MOV     ES:[DI], AL
        JMP     ExitOK        
QueryOMAType ENDP

;
; QueryDetType
;       Get detector model number.
;           
;       ENTRY   ES:[DI] points to request packet.
;               Current detector number in AX
;               GIODatPack points to a byte location for the return value.
;       EXIT    Returns the ID value of the detector model in the byte 
;               pointed to by GIODataPack.
;
IF DEBUG
        PUBLIC QueryDetType
ENDIF
QueryDetType PROC NEAR
        PUSH    AX              ; save detector number
        MOV     AX, ES:[DI].GIODataPack._SEG
        MOV     DI, ES:[DI].GIODataPack._OFF
        MOV     ES, AX
        MOV     CX, 1                   ; byte
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        POP     AX                      ; get detector number back
        JNC     QDT1
        JMP     GenFailure
QDT1:
        ADD     AX, OFFSET DetectorModel       ; detector number was in AX
        MOV     SI, AX
        MOV     AL, [SI]
        MOV     ES:[DI], AL
        JMP     ExitOK        
QueryDetType ENDP

;
; QueryMemAddr
;       Return an array of virtual addresses for access to the given 
;       detector memory.  Virtual address are good only for the calling 
;       process since PhysToUvirt uses the LDT.
;           
;       ENTRY   ES:[DI] points to request packet.
;               Current detector number in AX
;               GIODataPack points to a far pointer location for the return 
;                  value.
;       EXIT    Returns a pointer to an array of far virtual addresses
;               as the value of the GIODataPack pointer.
;
IF DEBUG
        PUBLIC QueryMemAddr
ENDIF
QueryMemAddr PROC NEAR
        PUSH    AX              ; save detector number
        MOV     AX, ES:[DI].GIODataPack._SEG
        MOV     DI, ES:[DI].GIODataPack._OFF
        MOV     ES, AX
        MOV     CX, 4                   ; far pointer
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        POP     AX                      ; get detector number back
        JNC     QMA1
        JMP     GenFailure
QMA1:
        MOV     BX, AX                  ; detector number 
        SHL     BX, 2                   ; offset for DD
        ; return pointer to array
        MOV     AX, [VirtArrayStart][BX]._SEG
        MOV     ES:[DI]._SEG, AX
        MOV     AX, [VirtArrayStart][BX]._OFF
        MOV     ES:[DI]._OFF, AX
        JMP     ExitOK
QueryMemAddr ENDP

;
; QueryMemSize
;       Return the memory size in Kbytes for the given detector.
;           
;       ENTRY   ES:[DI] points to request packet.
;               Current detector number in AL
;               GIODataPack points to a far pointer location for the return 
;                  value.
;       EXIT    Returns the On board memory size (in KBytes) in the word 
;               pointed to by GIODataPack.
;
IF DEBUG
        PUBLIC QueryMemSize
ENDIF
QueryMemSize PROC NEAR

        PUSH    AX              ; save detector number
        MOV     AX, ES:[DI].GIODataPack._SEG
        MOV     DI, ES:[DI].GIODataPack._OFF
        MOV     ES, AX
        MOV     CX, 2                   ; word
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        POP     AX                      ; get detector number back
        JNC     QMS1
        JMP     GenFailure
QMS1:
        SHL     AX, 1                           ; offset for Word
        ADD     AX, OFFSET DetectorMemSize      ; detector offset was in AX
        MOV     SI, AX
        MOV     AX, [SI]
        MOV     ES:[DI], AX
        JMP     ExitOK        
QueryMemSize ENDP

;
; QueryPortAddr
;       Return the base port address for the given detector.
;           
;       ENTRY   ES:[DI] points to request packet.
;               Current detector number in AL
;               GIODataPack points to a far pointer location for the return 
;                  value.
;       EXIT    Returns the base port address in the word pointed to by 
;               GIODataPack.
;
IF DEBUG
        PUBLIC QueryPortAddr
ENDIF
QueryPortAddr PROC NEAR
        PUSH    AX              ; save detector number
        MOV     AX, ES:[DI].GIODataPack._SEG
        MOV     DI, ES:[DI].GIODataPack._OFF
        MOV     ES, AX
        MOV     CX, 2                   ; word
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        POP     AX                      ; get detector number back
        JNC     QPA1
        JMP     GenFailure
QPA1:
        SHL     AX, 1                           ; offset for Word
        ADD     AX, OFFSET DetectorPortAddr     ; detector offset was in AX
        MOV     SI, AX
        MOV     AX, [SI]
        MOV     ES:[DI], AX
        JMP     ExitOK        
QueryPortAddr ENDP



;
; QueryDataSeg
;       
;   Copy the device data segment to a buffer that is hopefully large enough.        
;       ENTRY   ES:[DI] points to request packet.
;               GIODataPack points to a pointer buffer.
;       EXIT    Returns a pointer to the data segment in the address 
;               pointed to by GIODataPack.
;
IF DEBUG
        PUBLIC QueryDetNum
ENDIF
QueryDataSeg PROC NEAR
        CLD
        MOV     AX, ES:[DI].GIODataPack._SEG
        MOV     BX, ES:[DI].GIODataPack._OFF
        MOV     ES, AX
        MOV     DI, BX
        PUSH    DI
        PUSH    ES
        MOV     CX, 4                   ; DWORD
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        JNC     QDS1
        POP     ES
        POP     DI
        JMP     GenFailure
QDS1:
        MOV     SI, OFFSET OMA4_DEVHDR_0
        DEVHELP VirtToPhys
        ; Physical address returned in AX:BX

        MOV     CX, OFFSET END_OF_DATA
        MOV     DH, ToUVirt_RW
        DEVHELP PhysToUVirt
        JNC     QDS2
        POP     ES
        POP     DI
        JMP     GenFailure

QDS2:
        MOV     AX, ES

MOVITLP:
        POP     ES
        POP     DI
        MOV     ES:[DI]._OFF, BX         ; return address of data segment
        MOV     ES:[DI]._SEG, AX  
        JMP     ExitOK
QueryDataSeg ENDP



;
; QueryPhysAddr
;       Return the base physical memory address for the given detector.
;           
;       ENTRY   ES:[DI] points to request packet.
;               Current detector number in AL
;               GIODataPack points to a far pointer location for the return 
;                  value.
;       EXIT    Returns the base port address in the word pointed to by 
;               GIODataPack.
;
IF DEBUG
        PUBLIC QueryPhysAddr
ENDIF
QueryPhysAddr PROC NEAR
        PUSH    AX              ; save detector number
        MOV     AX, ES:[DI].GIODataPack._SEG
        MOV     DI, ES:[DI].GIODataPack._OFF
        PUSH    DI
        PUSH    AX
        MOV     CX, 4                   ; Dword
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        POP     ES                      ; get data address segment 
        POP     DI                      ; get data address offset 
        POP     AX                      ; get detector number back
        JNC     QPHA1
        JMP     GenFailure
QPHA1:
        SHL     AX, 2                          ; offset for DWord
        ADD     AX, OFFSET DetectorMemAddr     ; detector offset was in AX
        MOV     SI, AX
        MOV     AX, [SI]
        MOV     ES:[DI], AX
        JMP     ExitOK        
QueryPhysAddr ENDP




;
; ActivateDetector
;       Do a secondary initialization of the detector board.  
;               1. Tell it how much memory to use.
;               2. Tell it where to place the memory.
;               3. Set up the virtual address array for the memory.
;           
;       ENTRY   ES:[DI] points to request packet.
;               Current detector number in [BP].BOARD_ID
;               GIODataPack points to a far pointer location for the virtual
;               array return pointer.
;               GIOParaPack points to a word value for the requested physical 
;               memory size as the number of 2Mbyte blocks to be allocated. 
;               0 = as much as possible on the board.
;       EXIT    
;               Returns the virtual address array pointer using the pointer 
;               in GIODataPack. The word value pointed to by GIOParaPack is 
;               set to the amount of memory (in Kbytes) actually 
;               allocated. The value pointed to by GIOParaPack is set to 0 
;               on error. The error will be due to running out of the 16Mbyte of 
;               system addressability range.
;                       
;
IF DEBUG
        PUBLIC ActivateDetector
ENDIF
ActivateDetector PROC NEAR

        PUSH    ES
        PUSH    DI
        MOV     AX, ES:[DI].GIODataPack._SEG
        MOV     DI, ES:[DI].GIODataPack._OFF
        MOV     ES, AX
        MOV     DataAddr._SEG, ES
        MOV     DataAddr._OFF, DI
        MOV     CX, 1                   ; byte
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        POP     DI
        POP     ES

        JNC     AD1
        MOV     DX, 1                   ; error location code
        JMP     GenFailure
AD1:
        MOV     AX, ES:[DI].GIOParaPack._SEG
        MOV     DI, ES:[DI].GIOParaPack._OFF
        MOV     ES, AX
        MOV     ParaAddr._SEG, AX
        MOV     ParaAddr._OFF, DI
        MOV     CX, 2                   ; word
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        JNC     AD2
        MOV     DX, 2                   ; error location code
        JMP     GenFailure
AD2:
        XOR     BH, BH
        MOV     BL, [BP].BOARD_ID       ; get detector number 
        
        ; find the size of the board's memory
        SHL     BX, 1                   ; get word offset
        MOV     AX, DetectorMemSize[BX] ; read memory size in Kbytes
        ; round up memory size in K to next 2Mbyte value
        TEST    AX, 7FFH                ; check for a MOD 800H (2Mbyte) value
        JZ      NUMBLK
        AND     AX, 0F800H               ; round up to next 2M boundary
        ADD     AX, 800H
NUMBLK:
        ; Put system memory slot assignments into MemSlots
        SHR     AX, 11          ; convert Kbyte to # of 2Mbyte blocks
        ; get the requested memory size        
        LES     DI, ParaAddr            ; get the requested # of 2Mbyte blocks
        MOV     DX, ES:[DI]     
        AND     DX, DX                  ; see if want to use all of board's 
        JNZ     FINDMEMSPACE            ; memory
        CMP     DX, AX  ; check if requesting more memory than board has
        JNA     FINDMEMSPACE
        MOV     AX, DX                  ; get only as much memory as board has

FINDMEMSPACE:  ; number of blocks in AX
        PUSH    AX
        MOV     CX, 8                   ; total number of slots
        MOV     SI, OFFSET MemSlots
FINDOPEN:
        POP     AX                      ; get the number of slots wanted 
        PUSH    AX                      ; and save it again
        TEST    BYTE PTR [SI], 0FFh     ; see if slot already taken
        JZ      OPENBLOCK
        INC     SI
        DEC     CL
        JNZ     FINDOPEN
        MOV     DX, 3                   ; error location code
        JMP SHORT NOMEM                   ; no open slots available
OPENBLOCK:      ; found an open block
        DEC     AX                      ; see if this block is the last one 
                                        ; needed
        JZ      GOODSIZE                ; the open space is large enough
        ; need more open space
        INC     SI
        TEST    BYTE PTR [SI], 0FFh     ; see if slot already taken
        JZ      AD2A
        MOV     DX, 4                   ; error location code
        JMP SHORT NOMEM                   ; block not big enough
AD2A:
        INC     SI
        DEC     CL
        JNZ     OPENBLOCK            ; check to see if next block is open
               ; end of addressable space
        MOV     DX, 5                   ; error location code
        
NOMEM:
        POP     AX              ; get original number of blocks
        DEC     AX              ; reduce the number of blocks by 1
        JNZ     FINDMEMSPACE    ; try again
        JMP     ADERROUT        ; no blocks free
                ;       ENTRY   ES:[DI] points to request packet.

GOODSIZE:       ; value on stack is the size of available block
        POP     AX
        ; 8-CL-AL+1 = starting block number (0..7)
        DEC     CL
        SUB     CL, 8
        NEG     CL
        SUB     CL, AL
        PUSH    AX              ; save the number of blocks
        PUSH    CX              ; save the starting block number
        MOV     DI, OFFSET MemSlots
        ADD     DI, CX          ; get the offset of the starting block table 
                                ;entry
        PUSH    DS
        POP     ES
        MOV     CX, AX          ; put block count into CX
        MOV     AL, SS:[BP].BOARD_ID    ; get the detector number back
        INC     AL              ; put in value 1..8 for detector
        REP STOSB               ; store the detector # in the open blocks
                                ; to mark them as allocated
        DEC     AL              ; get actual detector number back
        POP     DX              ; get back the starting block number
        SHL     DX, 5           ; convert to the high word of a 32 bit 
                                ; address to the start of the boards memory
        MOV     BX, AX          ; get the detector number into BX
        SHL     BX, 2           ; convert to offset into DetectorMemAddr
        MOV     DetectorMemAddr[BX]._SEG, DX ; store segment, offset stays 0
        SHR     BX, 1           ; word offset
        POP     AX              ; get the number of blocks back
        SHL     AX, 11          ; change from number of blocks to Kbytes
        MOV     DetectorAllocatedMem[BX], AX
        
;**********************************************************************        
; make the virtual address array
;**********************************************************************
        ; BX has the word offset for this detector
        MOV     VirtAllocatedBlocks[BX], 0  ; clear temporary counter
        ; Calculate the number of segments used
        MOV     BX, AX          ; Get the size in KBYTES

        ; Get the number of segments( /64) then get double word offset (*4)
        SHR     BX, 4
        PUSH    BX                      ; save segment number offset, 
                                        ; = array size



        ; Allocated array space for detector memory selectors(segment):offset
        XOR     AX, AX                  ; high word of mem size
        MOV     DH, Mem_Lo              ; allocate it below 1Mbyte space 
;        MOV     WORD PTR [tempbuf], 0         ;*******&&&&&&&
;        MOV     WORD PTR [tempbuf + 2], BX    ;*******&&&&&&&
;        MOV     WORD PTR [tempbuf + 4], AX    ;*******&&&&&&&
        DEVHELP AllocPhys
;        MOV     WORD PTR [tempbuf + 6], BX    ;*******&&&&&&&
;        MOV     WORD PTR [tempbuf + 8], AX    ;*******&&&&&&&
        JNC     AD3
;        MOV     WORD PTR [tempbuf], 0FFFFh    ;*******&&&&&&&  
        
        ; No low memory so allocate high memory
        ; BX already set up for segment number offset
        XOR     AX, AX                  ; high word of mem size
        MOV     DH, Mem_Hi              ; allocate it above 1Mbyte space 
        DEVHELP AllocPhys               ; returns adress as AX:BX
        JNC     AD3
        MOV     DX, 6                   ; error location code
        POP     BX                      ; get array size
        JMP     ADERROUT

AD3:    ; array memory is allocated, AXBX is 32 bit physical address

        ;************&&&&WORKED &&&&&&&&&&&&&&&&&&****
;       POP     BX
;       JMP     ExitOK
        ;************&&&&&&&&&&&&&&&&&&&&&&****
        XOR     CH, CH
        MOV     CL, SS:[BP].BOARD_ID    ; get the detector number back
        MOV     SI, CX                  ; get the detector number 
        SHL     SI, 2                   ; offset for DD
        MOV     [SelArrayPtr][SI]._SEG, AX
        MOV     [SelArrayPtr][SI]._OFF, BX
        POP     CX                      ; get back array size
        ; convert array's physical address to virtual address
        ; CX has array size
        ; AX:BX has Physical address of array base
        MOV     DH, ToUVirt_RW          ; Memory access
;        MOV     WORD PTR [tempbuf], CX    ;*******&&&&&&&
;        MOV     WORD PTR [tempbuf + 2], BX    ;*******&&&&&&&
;        MOV     WORD PTR [tempbuf + 4], AX    ;*******&&&&&&&
;        MOV     WORD PTR [tempbuf + 6], DS    ;*******&&&&&&&
        DEVHELP PhysToUVirt             ; returns address as ES:BX 
;        MOV     WORD PTR [tempbuf + 8], BX    ;*******&&&&&&&
;        MOV     WORD PTR [tempbuf + 10], ES   ;*******&&&&&&&
;        MOV     WORD PTR [tempbuf + 12], SI   ;*******&&&&&&&
;        MOV     WORD PTR [tempbuf + 14], DS   ;*******&&&&&&&
        JNC     AD4
        MOV     DX, 7                   ; error location code
        JMP     ADERROUT
AD4:
        MOV     VirtArrayStart[SI]._SEG, ES ; save for possible later use
        MOV     VirtArrayStart[SI]._OFF, BX
P_TO_V:  ; ES:BX is the virtual address of the start of the pointer array
        ; CX has number of Kbytes >> 3
        SHR     CX, 2                   ; get the number of 64K segments
        AND     CX, CX
        JNZ     AD5
        MOV     DX, 8                   ; error location code
        JMP SHORT ADERROUT                ; must have at least 1 segment
AD5:
        ;************&&&&WORKED &&&&&&&&&&&&&&&&&&****
;       POP     BX
;       JMP     ExitOK
        ;************&&&&&&&&&&&&&&&&&&&&&&****
        MOV     DI, BX                  ; save array virtual offset into DI
        MOV     AX, [DetectorMemAddr][SI]._HI ; get detector's physical 
        MOV     BX, [DetectorMemAddr][SI]._LO ; address
        SHR     SI, 1                   ; word offset for this detector

;        MOV     tempbuf, 0 ;**********&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
FULL_P_TO_V:    ; get virtual addresses for this detector's memory


        PUSH    AX      ; save physical address _SEG
        PUSH    BX      ; _OFF
        PUSH    ES                      ; save array virtual segment
        PUSH    CX                      ; save the number of full segments
        PUSH    SI
        ; convert detector physical address to virtual address
        XOR     CX, CX                  ; size of segment is 64 Kbytes
        MOV     DH, ToUVirt_RW          ; Memory access

;        CMP     tempbuf, 7h             ;*******&&&&&&&&&&&&& 3,4,6 worked, 7 didn't&&&&&&&&&&&&&
;        JB      DBP1                    ;*******&&&&&&&&&&&&& d&&&&&&&&&&&&&
;        MOV     WORD PTR [tempbuf + 2], BX    ;*******&&&&&&&
;        MOV     WORD PTR [tempbuf + 4], AX    ;*******&&&&&&&
;        MOV     WORD PTR [tempbuf + 6], CX    ;*******&&&&&&&
;        MOV     WORD PTR [tempbuf + 8], DI    ;*******&&&&&&&
;
;        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
;        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
;        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
;        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
;        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
;        JMP     ExitOK                  ;*******&&&&&&&&&&&&& d&&&&&&&&&&&&&
;DBP1:                                   ;*******&&&&&&&&&&&&&&&&&&&&&&&
;        INC     tempbuf                 ;*******&&&&&&&&&&&&&&&&&&&&&&&

        DEVHELP PhysToUVirt             ; returns address as ES:BX 
        
        JNC     PTUVOK                     ; virtual address was good
        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
        MOV     DX, 9                   ; error location code
        JMP SHORT ADERROUT                ; flag left over from DEVHELP

PTUVOK:
;        CMP     tempbuf, 8h             ;*******&&&&&&&&&&&&&  &&&&&&&&&&&&&
;        JB      DBP1                    ;*******&&&&&&&&&&&&& d&&&&&&&&&&&&&
;        MOV     WORD PTR [tempbuf+4], AX;*******&&&&&&&
;        MOV     AX, ES
;        MOV     WORD PTR [tempbuf+6], AX    ;*******&&&&&&&
;        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
;        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
;        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
;        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
;        POP     AX                      ;*******&&&&&&&&&&&&&&&&&&&&&&&
;        JMP     ExitOK                  ;*******&&&&&&&&&&&&& worked&&&&&&&&&&&&&
;DBP1:                                   ;*******&&&&&&&&&&&&&&&&&&&&&&&
;        INC     tempbuf                 ;*******&&&&&&&&&&&&&&&&&&&&&&&

        POP     SI
        POP     CX                      ; get remaining segment number back

        MOV     AX, ES
        POP     ES                      ; get array segment back

        MOV     ES:[DI]._SEG, AX        ; Save virtual address of segment. 
        MOV     ES:[DI]._OFF, BX        ; Don't care if bad yet.
        POP     BX                      ; get back detector physical address
        POP     AX

        ADD     DI, 4                   ; increment array offset
        INC     DS:[VirtAllocatedBlocks][SI] ; increment block counter
        INC     AX                      ; add another 64K to physical address

        LOOP    FULL_P_TO_V             ; one less segment left
; assume memory only added in full segment increments

ADEXITOK:
        TEST    FakeFlag, 0FFH  ; see if interface really attached
        JNZ     SENTMEMADDR
        ; set up the hardware
        MOV     AX, DX
;&&&&        CALL    SendMemAddr     ; AX has starting block number
SENTMEMADDR:        
        TEST    FakeFlag, 0FFH  ; see if interface really attached
        JNZ     SENTMEMSIZE
        ; set up the hardware
;&&&&        CALL    SendMemSize     ; AX has number of usable 2Mbyte blocks

SENTMEMSIZE: ; SI at word pointer offset for this detector
        MOV     AX, DetectorAllocatedMem[SI]    ;

        LES     DI, ParaAddr    ; send back the number of Kbytes usable
        MOV     ES:[DI], AX
        SHL     SI, 1           ; double word offset
        MOV     AX, VirtArrayStart[SI]._SEG
        MOV     BX, VirtArrayStart[SI]._OFF

        LES     DI, DataAddr    ; Set address array pointer
        MOV     WORD PTR ES:[DI]._SEG, AX
        MOV     WORD PTR ES:[DI]._OFF, BX
        JMP     ExitOK

ADERROUT:       ; clear out any assignments for this board 
        PUSH    DX
        CALL    FreeDetMem
        POP     DX
        LES     DI, ParaAddr    ; No usable memory
        MOV     WORD PTR ES:[DI], DX    ; put in error location code
        LES     DI, DataAddr    ; clear address array pointer
        MOV     WORD PTR ES:[DI]._SEG, 0
        MOV     WORD PTR ES:[DI]._OFF, 0
        JMP     GenFailure
ActivateDetector ENDP

;
;DeActivateDetector
;       ENTRY   SS:[BP]._BOARD_ID points to detector ID.
;       EXIT    Changes values in SelArrayPtr, VirtAllocatedBlocks,
;               VirtArrayStart, MemSlots, DetectorMemAddr, 
;               DetectorAllocatedMem
;

IF DEBUG
        PUBLIC DeActivateDetector
ENDIF
DeActivateDetector PROC NEAR
        PUSH    ES
        PUSH    DI
        MOV     AX, ES:[DI].GIOParaPack._SEG
        MOV     BX, ES:[DI].GIOParaPack._OFF
        MOV     ES, AX
        MOV     DI, BX
        MOV     ParaAddr._SEG, AX
        MOV     ParaAddr._OFF, BX
        MOV     CX, 2                   ; word
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        POP     DI
        POP     ES
        JNC     DAD1
        MOV     DX, 2                   ; error location code
        JMP     GenFailure

DAD1:
        CALL    FreeDetMem              ; Clear pointers and free memory
        AND     AX, AX                  ; test for error
        JNZ      DADERROUT

        TEST    FakeFlag, 0FFH  ; see if interface really attached
        JNZ     DADGETOUT
;&&&&   CALL    HideMem                 ; turn off board memory buffer
DADGETOUT:
        JMP     ExitOK

DADERROUT:
        LES     DI, ParaAddr    ; No usable memory
        MOV     WORD PTR ES:[DI], AX    ; put in error location code
        JMP     GenFailure
DeActivateDetector ENDP

;
; FreeDetMem
;       Free up any virtual memory pointers or physical memory associated
;       with activating the board in SS:[BP].BOARD_ID  
;           
;       ENTRY   
;               Current detector number in SS:[BP].BOARD_ID
;       EXIT    Changes values in SelArrayPtr, VirtAllocatedBlocks,
;               VirtArrayStart, MemSlots, DetectorMemAddr, 
;               DetectorAllocatedMem
;                       
;
IF DEBUG
        PUBLIC FreeDetMem
ENDIF
FreeDetMem PROC NEAR
        ; Clear PhysToUVirt
        XOR     BH, BH
        MOV     BL, SS:[BP].BOARD_ID
        MOV     SI, BX
        SHL     SI, 1                   ; word offset
        MOV     CX, VirtAllocatedBlocks[SI] ; Get the number of segments 
                                            ; successfully allocated

        JCXZ    FREE_VIRT_ARRAYPTR
        SHL     SI, 1                   ; double word offset
        LES     DI, VirtArrayStart[SI]  ; get the virtual address of the 
                                        ; array's start
FREEUVIRTLP:
        MOV     AX, ES:[DI]._SEG        ; get the detector mem block virtual
                                        ; selector
        AND     AX, AX                  ; see if have a valid segment
        JZ      FREE_VIRT_ARRAYPTR

        ; free selector in AX
        MOV     DH, ToUVirt_Fre         ; free memory
        PUSH    ES
        DEVHELP PhysToUVirt
        POP     ES
        JNC     FDM1
        MOV     AX, 1                   ; set error flag
        JMP     FDMERROUT 

FDM1:   
        ADD     DI, 4                   ; go to next virtual address
        LOOP    FREEUVIRTLP             ; one less segment left
FREE_VIRT_ARRAYPTR:
        MOV     AX, [VirtArrayStart][SI]._SEG
        AND     AX, AX                  ; see if have a valid segment
        JZ      FREE_PHYS_ARRAYPTR
        ; free selector in AX
        MOV     DH, ToUVirt_Fre         ; free memory
        DEVHELP PhysToUVirt             ; returns address as ES:BX 
        JNC     FREE_PHYS_ARRAYPTR
        MOV     AX, 2                   ; set error flag
        JMP SHORT FDMERROUT

FREE_PHYS_ARRAYPTR:     ; free physical memory allocated for pointer array
        MOV     AX, [SelArrayPtr][SI]._SEG
        AND     AX, AX                  ; see if have a valid segment
        JZ      CLRMEMSLOTS
        MOV     BX, [SelArrayPtr][SI]._OFF
        DEVHELP FreePhys
        JNC     CLRMEMSLOTS
        MOV     AX, 3                   ; set error flag
        JMP SHORT FDMERROUT 

CLRMEMSLOTS:
        MOV     CX, 8
        MOV     DI, OFFSET MemSlots - 1
CLRMEMSLOTLP:
        INC     DI
        MOV     AL, SS:[BP].BOARD_ID
        INC     AL              ; value placed in MemSlot is 1 + detector
        CMP     [DI], AL
        LOOPNE  CLRMEMSLOTLP     ; loop until find a value for this detector
                                ; or CX runs out
        JCXZ    CLRSTATICS      ; ran out of MemSlots
        MOV     BYTE PTR [DI], 0 ; clear the slot for reuse
        JMP SHORT CLRMEMSLOTLP

CLRSTATICS:
        XOR     BH, BH
        MOV     BL, SS:[BP].BOARD_ID
        SHL     BX, 2                   ; DWORD OFFSET
        MOV     [SelArrayPtr][BX]._SEG, 0
        MOV     [SelArrayPtr][BX]._OFF, 0
        MOV     [VirtArrayStart][BX]._SEG, 0
        MOV     [VirtArrayStart][BX]._OFF, 0
        MOV     [DetectorMemAddr][BX]._SEG, 0 ; clear segment, offset stays 0
        SHR     BX, 1                   ; Word offset
        MOV     [VirtAllocatedBlocks][BX], 0
        MOV     DetectorAllocatedMem[BX], 0 ; clear actual allocated memory
        XOR     AX, AX          ; set for no error
        RET 

FDMERROUT:
        XOR     BH, BH
        MOV     BL, SS:[BP].BOARD_ID
        SHL     BX, 2                   ; DWORD OFFSET
        MOV     [SelArrayPtr][BX]._SEG, 0
        MOV     [SelArrayPtr][BX]._OFF, 0
        MOV     [VirtArrayStart][BX]._SEG, 0
        MOV     [VirtArrayStart][BX]._OFF, 0
        MOV     [DetectorMemAddr][BX]._SEG, 0 ; clear segment, offset stays 0
        SHR     BX, 1                   ; Word offset
        MOV     [VirtAllocatedBlocks][BX], 0
        MOV     DetectorAllocatedMem[BX], 0 ; clear actual allocated memory
        RET
FreeDetMem ENDP


;
; BlockForMsg
;       Block and wait for the interface board to ask for service.  Return
;       a value to the blocking thread to indicate the type of service 
;       needed.  Returns Message number in GIOParaPack and any extra info 
;       in GIODataPack.
;           
;       ENTRY   Long Value at GIOParaPack contains timeout limit in 
;               milliseconds.
;               Current detector number in [BP].BOARD_ID
;
;       EXIT    GIOParaPack contains message number and GIODataPack has
;               additional information if needed.        
;                       
;
IF DEBUG
        PUBLIC BlockForMsg
ENDIF
BlockForMsg PROC NEAR
        PUSH    ES
        PUSH    DI
        MOV     AX, ES:[DI].GIODataPack._SEG
        MOV     DI, ES:[DI].GIODataPack._OFF
        MOV     ES, AX
        MOV     CX, 1                   ; byte
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        POP     DI
        POP     ES
        JNC     BFM1
        MOV     DX, 1                   ; error location code
        JMP     GenFailure
BFM1:
        MOV     AX, ES:[DI].GIOParaPack._SEG
        MOV     DI, ES:[DI].GIOParaPack._OFF
        MOV     ES, AX
        MOV     CX, 2                   ; word
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        JNC     BFM2
        MOV     DX, 2                   ; error location code
        JMP     GenFailure
BFM2:

;        JMP     ExitOK  ;*******&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

        MOV     CL, [BP].BOARD_ID       ; get detector number 


        ; flag that the thread will be blocked 
        MOV     AL, 1        
        SHL     AL, CL

        ; Test to see if an interrupt occurred while the thread was 
        ; unblocked and the interface board needs to say something.
        CLI                     ; hold off interrupts for a while
        TEST    NoBlock, AL     ; see if someone wants to terminate 
                                ; acquisition
        JZ      TestInt         ; Don't terminate
        JMP     ExpStop         ; Terminate
TestInt:
        TEST    IntPassed, AL
        JNZ     ReadMessage

        OR      ThreadBlocked, AL

        ; otherwise, block and wait for interrupt 
BlockThread:

;        STI             ;*******&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
;        JMP     ExitOK  ;*******&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

        ; use address of GIOParaPack for Block ID
        XOR     BH, BH
        MOV     BL, CL            ; CL still has detector number
        SHL     BL, 2             ; get double word offset for detector #

        LES     DI, [BP].LD_PTRSAVE
        MOV     AX, ES:[DI].GIOParaPack._SEG
        MOV     SI, ES:[DI].GIOParaPack._OFF

        MOV     DetectorBlkID[BX]._LO, SI
        MOV     DetectorBlkID[BX]._HI, AX

        MOV     ES, AX           ; get the timeout limit stored in ParaPack
        MOV     CX, ES:[SI]._LO         ; timeout limit in milliseconds
        MOV     DI, ES:[SI]._HI
        MOV     DH, FALSE               ; block is interruptible

        DEVHELP ProcBlock
       
        CLI

;        STI             ;*******&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
;        JMP     ExitOK  ;*******&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

        ; check to see if valid wakeup
        JNC     ValidUnblock
        ; unusual wakeup
        ; check to see if timed out
        JNZ     BlockThread     ; reblock if not timeout
        JMP SHORT TimedOut

ValidUnblock:
        MOV     CL, [BP].BOARD_ID       ; get detector number 
        MOV     AL, 1        
        SHL     AL, CL
        TEST    NoBlock, AL             ; test to see if need to 
        JNZ     ExpStop                 ; terminate acquisition
        TEST    IntPassed, AL
        JNZ     ReadMessage
        ; must be random unblock
        JMP SHORT BlockThread     ; reblock if not an interrupt


ReadMessage:
        ; reset thread blocked flag 
        XOR     ThreadBlocked, AL
        ; reset interrupted flag
        XOR     IntPassed, AL

        LES     DI, [BP].LD_PTRSAVE
        MOV     AX, ES:[DI].GIOParaPack._SEG
        MOV     DI, ES:[DI].GIOParaPack._OFF
        MOV     ES, AX

        MOV     ES:[DI]._LO, CURVESREADY
        MOV     ES:[DI]._HI, 0
        STI
        JMP     ExitOK

TimedOut:
        ; reset thread blocked flag 
        XOR     ThreadBlocked, AL
        LES     DI, [BP].LD_PTRSAVE
        MOV     AX, ES:[DI].GIOParaPack._SEG
        MOV     DI, ES:[DI].GIOParaPack._OFF
        MOV     ES, AX

SendTimeOut:
        MOV     ES:[DI]._LO, TIMEDOUT
        MOV     ES:[DI]._HI, 0
        STI
        JMP     ExitOK

ExpStop:
        ; Terminate Acquisition
        XOR     NoBlock, AL

        LES     DI, [BP].LD_PTRSAVE
        MOV     AX, ES:[DI].GIOParaPack._SEG
        MOV     DI, ES:[DI].GIOParaPack._OFF
        MOV     ES, AX
        MOV     ES:[DI]._LO, EXPSTOP
        MOV     ES:[DI]._HI, 0
        STI
        JMP     ExitOK

BlockForMsg ENDP



;
; StopAcquisition
;       Unblock this detector's thread, if it is blocked and have BlockForMsg
;       return EXPSTOP the next time it gets called.
;           
;       ENTRY   Current detector number in [BP].BOARD_ID
;
;       EXIT    VOID
;                       
;
IF DEBUG
        PUBLIC StopAcquisition
ENDIF
StopAcquisition PROC NEAR
        MOV     CL, [BP].BOARD_ID       ; Get the Board ID for this instance
        MOV     AL, 1
        SHL     AL, CL
        OR      NoBlock, AL             ; set No block flag
        ; test to see if have a thread to unblock
        TEST    ThreadBlocked, AL       ; see if thread blocked
        JZ      Unblocked
        ; unblock the thread
        XOR     BH, BH
        MOV     BL, CL
        SHL     BX, 2                   ; get DWORD offset
        MOV     AX, DetectorBlkID[BX]._HI
        MOV     BX, DetectorBlkID[BX]._LO
        DEVHELP ProcRun
Unblocked:
        
        JMP     ExitOK

StopAcquisition ENDP

;
; GetSegAbsolute
;       Get a protected mode selector for an area of physical memory
;           
;       ENTRY   ES:[DI] points to request packet.
;               GIOParaPack points to a word value for the 
;               requested physical memory size in bytes. 
;               GIODataPack points to a long word location for the return 
;               virtual selector-offset value.
;
;       EXIT    Returns the selector as a word value to the location 
;               pointed to by the GIODataPack pointer.
;                       
;
IF DEBUG
        PUBLIC GetSegAbsolute
ENDIF
GetSegAbsolute PROC NEAR
        PUSH    ES
        PUSH    DI
        MOV     AX, ES:[DI].GIODataPack._SEG
        MOV     DI, ES:[DI].GIODataPack._OFF
        MOV     ES, AX
        MOV     CX, 4                   ; far pointer
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        JNC     GSA1
        POP     DI
        POP     ES
        JMP     GenFailure
GSA1:
        MOV     AX, ES
        MOV     WORD PTR [tempbuf]._SEG, AX     ;store the return data address
        MOV     WORD PTR [tempbuf]._OFF, DI 
        POP     DI
        POP     ES

        ; segment size pointer
        MOV     AX, ES:[DI].GIOParaPack._SEG
        MOV     DI, ES:[DI].GIOParaPack._OFF
        MOV     ES, AX
        MOV     CX, 4                   ; far pointer
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        JNC     GSA2
        JMP     GenFailure
GSA2:
        MOV     CX, ES:[DI]             ; get the segment size
        MOV     AX, tempbuf._SEG        ; get the data address
        MOV     DI, tempbuf._OFF
        MOV     ES, AX
        PUSH    ES                      ; save data address to return virtual 
        PUSH    DI                      ; address in
        MOV     AX, ES:[DI]._SEG        ; get the physical address high word
        MOV     BX, ES:[DI]._OFF        ; and offset
        MOV     DH, ToUVirt_RW          ; Read/Write Memory access
        DEVHELP PhysToUVirt             ; returns virtual address as ES:BX 
        POP     DI
        POP     AX
        PUSH    ES
        MOV     ES, AX
        POP     AX
        MOV     ES:[DI]._SEG, AX          ; store virtual address using data
        MOV     ES:[DI]._OFF, BX          ; pointer
        JNC     GSA_OK                  ; error check
        MOV     DX, 1                   ; error location code
        JMP     GenFailure

GSA_OK:
        JMP     ExitOK
GetSegAbsolute ENDP




;
; DelSegAbsolute
;       Free up a virtual memory pointer associated with physical memory 
;           
;       ENTRY   ES:[DI] points to request packet.
;               GIODataPack points to a word location for the selector value.
;
;       EXIT    
;                       
;
IF DEBUG
        PUBLIC DelSegAbsolute
ENDIF
DelSegAbsolute PROC NEAR
        MOV     AX, ES:[DI].GIODataPack._SEG
        MOV     DI, ES:[DI].GIODataPack._OFF
        MOV     ES, AX
        MOV     CX, 2                   ; word
        MOV     DH, Verify_RW           ; make sure can use memory first
        DEVHELP VerifyAccess
        JNC     DSA1
        JMP     GenFailure
DSA1:
        MOV     AX, ES:[DI]             ; get the 16 bit selector
        ; free selector in AX
        MOV     DH, ToUVirt_Fre         ; free memory
        DEVHELP PhysToUVirt             ; returns address as ES:BX 
        JNC     DSA_OK
        MOV     DX, 1                   ; set error flag
        JMP     GenFailure
DSA_OK:
        JMP     ExitOK
DelSegAbsolute ENDP



