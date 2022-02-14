;**********************************************************************
;
; DBTOOLS.asm
;
;  copyright (c) 1988, EG&G Princeton Applied Research                    
;
;  Various debugging helper functions in assembler mostly for a 
;  small memory model.
;  Some Functions are specific to OMA4DRV.asm variables.
;       
;
; started: 9/15/88       DI
;
;**********************************************************************


MAIN_DATA SEGMENT WORD PUBLIC 'DATA'
DumpBuf         DB      132 DUP (0)      ; 132 character buffer
DumpLen = $ - DumpBuf

;CharStruc       DB      ?               ; ASCII character code
;                DB      ?               ; scan code
;                DB      ?               ; Character status
;                DB      ?               ; NLS Shift State
;                DW      ?               ; Shift State
;                DD      ?               ; time stamo
KbdHandle       DW      0
KbdAction       DW      ?
kbd             DB      'KBD$', 0
KbdBuf          DB      80 DUP (0)                
KbdRead         DW      0


MAIN_DATA       ENDS

;
; MemDump
;       Print the contents of memory. 
;       ENTRY:  
;               STACK
;                       SP+6 - Segment
;                       SP+4 - offset
;                       SP+2 - length
;       
;       REMARKS: Block must be contained in one segment. Takes params 
;               off of the stack before returning ala PASCAL
;
MemDump PROC    NEAR
        PUSH    BP
        MOV     BP, SP        
        ADD     BP, 04H
        PUSH    ES
        PUSH    DS
        PUSHA
        MOV     AX, SEG MAIN_DATA
        MOV     DS, AX
        ; Put the starting address in ES:DI
        MOV     CX, SS:[BP]             ; length
        MOV     DI, SS:[BP+2]._OFF      ; offset
        MOV     ES, SS:[BP+2]._SEG      ; segment 
        
        MOV     BX, OFFSET CharTable
BLOCKLP:
        ;Print the starting address for the line
        ; print the segment
        MOV     AX, ES
        SHR     AH, 4
        MOV     AL, AH
        XLAT    
        MOV     DumpBuf, AL
        PUSH    WORD PTR STDOUT
        PUSH    1                       ; print 1 char
        PUSH    DS
        PUSH    OFFSET DumpBuf
        CALL    DOSPUTMESSAGE

        MOV     AX, ES
        AND     AH, 0FH 
        MOV     AL, AH
        XLAT
        MOV     DumpBuf, AL
        PUSH    WORD PTR STDOUT
        PUSH    1                       ; print 1 char
        PUSH    DS
        PUSH    OFFSET DumpBuf
        CALL    DOSPUTMESSAGE

        MOV     AX, ES
        SHR     AL, 4
        XLAT
        MOV     DumpBuf, AL
        PUSH    WORD PTR STDOUT
        PUSH    1                       ; print 1 char
        PUSH    DS
        PUSH    OFFSET DumpBuf
        CALL    DOSPUTMESSAGE

        MOV     AX, ES
        AND     AL, 0FH
        XLAT
        MOV     DumpBuf, AL
        PUSH    WORD PTR STDOUT
        PUSH    1                       ; print 1 char
        PUSH    DS
        PUSH    OFFSET DumpBuf
        CALL    DOSPUTMESSAGE

        ; print the colon
        MOV     DumpBuf, ':'
        PUSH    WORD PTR STDOUT
        PUSH    1                       ; print 1 char
        PUSH    DS
        PUSH    OFFSET DumpBuf
        CALL    DOSPUTMESSAGE

        ; print the offset
        MOV     AX, DI
        SHR     AH, 4
        MOV     AL, AH
        XLAT    
        MOV     DumpBuf, AL
        PUSH    WORD PTR STDOUT
        PUSH    1                       ; print 1 char
        PUSH    DS
        PUSH    OFFSET DumpBuf
        CALL    DOSPUTMESSAGE

        MOV     AX, DI
        AND     AH, 0FH 
        MOV     AL, AH
        XLAT
        MOV     DumpBuf, AL
        PUSH    WORD PTR STDOUT
        PUSH    1                       ; print 1 char
        PUSH    DS
        PUSH    OFFSET DumpBuf
        CALL    DOSPUTMESSAGE

        MOV     AX, DI
        SHR     AL, 4
        XLAT
        MOV     DumpBuf, AL
        PUSH    WORD PTR STDOUT
        PUSH    1                       ; print 1 char
        PUSH    DS
        PUSH    OFFSET DumpBuf
        CALL    DOSPUTMESSAGE

        MOV     AX, DI
        AND     AL, 0FH
        XLAT
        MOV     DumpBuf, AL
        PUSH    WORD PTR STDOUT
        PUSH    1                       ; print 1 char
        PUSH    DS
        PUSH    OFFSET DumpBuf
        CALL    DOSPUTMESSAGE


        PUSH    WORD PTR STDOUT
        PUSH    2                       ; print space
        PUSH    DS
        PUSH    OFFSET SPACE
        CALL    DOSPUTMESSAGE

        XOR     DX, DX                  ; DX keeps count of the number of 
                                        ; bytes printed in the line
        PUSH    CX                      ; save the number of bytes left in the 
                                        ; block
        PUSH    DI                      ; save the index for the start of
                                        ; the line
        CMP     CX, 10H                 ; See if need to do all 16 bytes
        JL      LINELP                  ; Less than 16 bytes left
        MOV     CX, 10H                 ; Do only 16 Bytes        
LINELP:
        AND     CX, CX                  ; test for 0 length
        JNZ     LNLP1
        JMP     EXITMD
LNLP1:
        ; print the high nybble
        MOV     AL, ES:[DI]             ; Put the Byte into AL
        SHR     AL, 4                   ; Shift high nibble down and 0 fill
        XLAT                            ; Translate the byte in AL to the 
                                        ; entry in the [BX] table
        MOV     DumpBuf, AL
        PUSH    WORD PTR STDOUT
        PUSH    1                       ; print 1 char
        PUSH    DS
        PUSH    OFFSET DumpBuf
        CALL    DOSPUTMESSAGE
        ; print the low nybble        
        MOV     AL, ES:[DI]             ; Put the Byte into AL
        AND     AL, 0FH                 ; Mask high nibble 
        XLAT                            ; Translate the byte in AL to the 
                                        ; entry in the [BX] table
        MOV     DumpBuf, AL
        PUSH    WORD PTR STDOUT
        PUSH    1                       ; print 1 char
        PUSH    DS
        PUSH    OFFSET DumpBuf
        CALL    DOSPUTMESSAGE

        ; print a space
        PUSH    WORD PTR STDOUT
        PUSH    1
        PUSH    DS
        PUSH    OFFSET SPACE
        CALL    DOSPUTMESSAGE
        INC     DI                      ; Increment address
        INC     DX                      ; Increment the char count in DX
        LOOP    LINELP                  ; Loop and DEC CX until 0
        
        ; print a space
        PUSH    WORD PTR STDOUT
        PUSH    1
        PUSH    DS
        PUSH    OFFSET SPACE
        CALL    DOSPUTMESSAGE

        MOV     CX, DX                  ; Get the number of bytes printed
        POP     DI
ASCIILP:
        MOV     AL, ES:[DI]
RangeCheck1:                    ; check to see if value in normal char range     
        CMP     AL, 20H
        JB      DotFill         ; Bad if char value is less than 20H
RangeCheck2:      
        CMP     AL, 7EH         ; Bad if char value is greater than 7EH
        JNA     PrintCharRep    ; 

DotFill:
        MOV     AL, '.'        
PrintCharRep:
        MOV     DumpBuf, AL
        PUSH    WORD PTR STDOUT
        PUSH    1                       ; print 1 char
        PUSH    DS
        PUSH    OFFSET DumpBuf
        CALL    DOSPUTMESSAGE
        INC     DI
        LOOP    ASCIILP
        
        PUSH    WORD PTR STDOUT
        PUSH    2                       ; print a carriage return / lf
        PUSH    DS
        PUSH    OFFSET CRLF
        CALL    DOSPUTMESSAGE
        
        POP     CX                      ; get the number of bytes left back
        SUB     CX, DX                  ; subtract the number done in this 
                                        ; line
        JZ      EXITMD                  ; Do another line if not done
        JMP     BLOCKLP

EXITMD:
        POPA
        POP     DS
        POP     ES
        POP     BP
        RET     6                       ; Pop params off of stack

MemDump ENDP



;
; KeyWait
;       Wait for a key to be hit. 
;
KeyWait PROC    NEAR

        PUSHA
        TEST    SetWait, 0FFH
        JZ      KEYGETOUT

; 1. call DosOpen( "KBD$", @handle, @action, 0L, 0, 1, 12h, 0L)
        PUSH    DS
        PUSH    OFFSET kbd              ; device name
        PUSH    DS
        PUSH    OFFSET KbdHandle           ; returned handle
        PUSH    DS
        PUSH    OFFSET KbdAction           ; returned new/existed flag
        XOR     AX, AX
        PUSH    AX                      ; initial file length = DWORD 0
        PUSH    AX
        PUSH    AX                      ; file attribute = WORD 0
        PUSH    1                       ; fail if file non-existant
        PUSH    12h                     ; not sharable, r/w access
        PUSH    AX                      ; reserved DWORD 0
        PUSH    AX
        CALL    DOSOPEN
        OR      AX, AX
        JNZ     KEYGETOUT
        ; get current keyboard state
        PUSH    KbdHandle
        ; pointer to input buffer
        PUSH    DS
        PUSH    OFFSET KbdBuf
        ; number of bytes to read
        PUSH    1
        ; pointer to number of bytes read
        PUSH    DS
        PUSH    OFFSET KbdRead
        CALL    DOSREAD
        AND     AX, AX          ; check for error
        JZ      KeyClose
        PUSH    WORD PTR KbdBuf
; 2. Wait for char in
CharWaitLoop:

        PUSH    KbdHandle
        ; pointer to input buffer
        PUSH    DS
        PUSH    OFFSET KbdBuf
        ; number of bytes to read
        PUSH    80
        ; pointer to number of bytes read
        PUSH    DS
        PUSH    OFFSET KbdRead
        CALL    DOSREAD
        AND     AX, AX          ; check for error
        JZ      PREKEYCLOSE

        POP     AX                      ; get old char
        PUSH    AX                      ; save it again
        SUB     AL, [KbdBuf]
        JZ      CharWaitLoop           ; wait for change
        
PREKEYCLOSE:
        POP     AX
; 3. Close the KBD$ handle: call DosClose(handle)
KeyClose:
        PUSH    KbdHandle
        CALL    DOSCLOSE
KEYGETOUT:
        POPA
        RET
KeyWait         ENDP

;************************************************************************
;*                              END                                     *
;************************************************************************

