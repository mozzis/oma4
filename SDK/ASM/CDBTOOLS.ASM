        PAGE    ,132
;**********************************************************************
;
; CDBTOOLS.asm
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

        INCLUDE SAMPDEFS.INC  

        EXTRN DOSPUTMESSAGE:FAR
.286
.MODEL MEDIUM,C
        .DATA
;MAIN_DATA SEGMENT WORD PUBLIC 'DATA'
DumpBuf         DB      132 DUP (0)      ; 132 character buffer
DumpLen = $ - DumpBuf

CharTable       DB      '0123456789ABCDEF' ; for debugging output

CR      EQU     0Dh     
LF      EQU     0Ah
TAB     EQU     09H
CRLF            DB      CR, LF
SPACE           DB      '                                    '

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

        .LALL
        .CODE
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
MemDump PROC    NEAR USES ES DI, dumpaddr:DWORD, dumplength:WORD
        ; Put the starting address in ES:DI
        MOV     CX, dumplength             ; length
        LES     DI, dumpaddr
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
        RET     

MemDump ENDP

END
;************************************************************************
;*                              END                                     *
;************************************************************************


