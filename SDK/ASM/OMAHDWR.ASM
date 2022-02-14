;************************************************************************** 
;
; OMAHDWR.ASM
;       Hardware specific (AT, PS/2) code
;
;**************************************************************************

CONTROL_REG     EQU     0
PROGENTRY_REG   EQU     1
MEMORY_REG      EQU     2
STATUS_REG      EQU     3
PROGDESC_REG    EQU     4
SCANDESC_REGL   EQU     5
SCANDESC_REGH   EQU     6

; Control and status register bits
        ; control
DATA_ACQ_ATTN   EQU     1       ; disables data acquisition attn interrupt
PARITY_DISABLE  EQU     2       ; disables parity interrupt
OMA_RESET       EQU     4       ; resets board and swaps program memory in
INT_ENABLE      EQU     8       ; Enable host interrupt request line
        ;status
COOLER_NOLOCK   EQU     010H    ; cooler is not locked interrupted on 
                                ; transition
PARITY_ERR      EQU     020H    ; interrupted by parity error
ATTN_HOST       EQU     040H    ; track done interrupt?
BUSCNTRL        EQU     080H    ; PC doesn't control DAC's bus when bit set

; Memory Map Control Register bits
MEMORY_ENABLE   EQU     020H    ; Allow host access to memory
INTNULL         EQU     000H    ; No Interrupts
INTLVL4         EQU     040H    ; Interrupt line 4
INTLVL5         EQU     080H    ; Interrupt line 5
INTLVL7         EQU     0C0H    ; Interrupt line 7


;
; OMAIntHand
;       OMA interrupt handler
;       ENTRY: Stack size ~2K, CS and DS set up
;       EXIT: Carry cleared if interrupt is claimed
;             Carry is set if not claimed.
;

IF DEBUG
        PUBLIC  OMAIntHand
ENDIF
OMAIntHand PROC FAR
        PUSHA
        TEST    FakeFlag, 1     ; see if hardware really attached
        JNZ     UNCLAIMED_EXIT  

        MOV     CL, [DetectorNum]                
        XOR     BH, BH
INTFLAGLP:      ; find out which board caused interrupt
        DEC     CL              ; get detector ID
        MOV     BL, CL
        SHL     BX, 1           ; get word offset
        MOV     DX, [DetectorPortAddr][BX]
        ADD     DX, STATUS_REG  ; address the status register with DX
        IN      AL, DX          ; get status
; ****Figure out how to see cooler transition here
        AND     AL, COOLER_NOLOCK + PARITY_ERR + ATTN_HOST ;**??????????
        JNZ     HANDLER
        LOOP    INTFLAGLP

        ; CL has board number BL has board word offset
HANDLER:
CLAIMED_EXIT:   
        ; test to see if have a thread to unblock
        MOV     AL, 1
        SHL     AL, CL
        OR      IntPassed, AL           ; set flag that interrupt occurred
        TEST    ThreadBlocked, AL       ; see if thread blocked
        JZ      CE1
        ; unblock the thread
        SHL     BX, 1           ; get DWORD offset
        MOV     AX, DetectorBlkID[BX]._HI
        MOV     BX, DetectorBlkID[BX]._LO
        DEVHELP ProcRun
        
CE1:
        MOV     AL, [InterruptNum]
        DEVHELP EOI
        POPA
        CLC             ; Clear Carry Flag
        RETF

UNCLAIMED_EXIT:        
        POPA
        STC             ; set carry flag
        RETF

OMAIntHand ENDP



;
; HideMem
;       Turn off the memory buffer so that the Host will not be able to 
;       access the board's memory. Returns without effect if FakeFlag is 
;       non-zero.
;       ENTRY: SS:[BP].BOARD_ID = board number 0..7
;       EXIT: 
;

IF DEBUG
        PUBLIC  HideMem
ENDIF
HideMem PROC NEAR
        TEST    FakeFlag, 0FFH          ; see if board really attached
        JNZ     HMEXIT
        XOR     BH, BH
        MOV     BL, SS:[BP].BOARD_ID    ; get the board number
        MOV     AL, MemoryReg[BX]       ; get the last byte written to this 
        AND     AL, NOT MEMORY_ENABLE   ; register and mask the memory enable 
                                        ; bit
        MOV     MemoryReg[BX], AL       ; save the new control value
        SHL     BX, 1                   ; get word offset into 
                                        ; DetectorPortAddr
        MOV     DX, DetectorPortAddr[BX]        
        ADD     DX, MEMORY_REG
        OUT     DX, AL
HMEXIT:
        RET
HideMem ENDP


;
; SetBrdInt
;       Set all boards' interrupt lines to the value in InterruptNum. 
;       Returns without effect if FakeFlag is non-zero.
;       ENTRY: InterruptNum is set to the interrupt line number
;       EXIT: 
;

IF DEBUG
        PUBLIC  SetBrdInt
ENDIF
SetBrdInt PROC NEAR
        TEST    FakeFlag, 0FFH          ; see if board really attached
        JNZ     SBIEXIT
        MOV     AL, [InterruptNum]
        CMP     AL, 0           ; check if want to turn off interrupts
        JNZ     SBI0
        MOV     AL, INTNULL
        JMP SHORT SBI3
SBI0:
        CMP     AL, 4           ; check for interrupt level 4
        JNZ     SBI1      
        MOV     AL, INTLVL4
        JMP SHORT SBI3
SBI1:
        CMP     AL, 5           ; check for interrupt level 5
        JNZ     SBI2      
        MOV     AL, INTLVL5
        JMP SHORT SBI3
SBI2:
        CMP     AL, 7           ; check for interrupt level 7
        JNZ     SBIEXIT         ; Ignore error      
        MOV     AL, INTLVL7

SBI3:
        PUSH    AX
        XOR     CH, CH
        MOV     CL, DetectorNum         ; get the number of boards attached
        
SBIBRDLP:
        POP     AX
        PUSH    AX
        MOV     BX, CX
        OR      AL, MemoryReg[BX]       ; get the last byte written to this 
                                        ; register and add the proper 
                                        ; interrupt bits
        MOV     MemoryReg[BX], AL       ; save the new control value
        SHL     BX, 1                   ; get word offset into 
                                        ; DetectorPortAddr
        MOV     DX, DetectorPortAddr[BX]        
        ADD     DX, MEMORY_REG
        OUT     DX, AL
        LOOP    SBIBRDLP
        POP     AX
SBIEXIT:
        RET
SetBrdInt ENDP

