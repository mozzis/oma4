;**********************************************************************
;
; SMTOOLS.asm
;
;  copyright (c) 1988, EG&G Princeton Applied Research                    
;
;  Various functions in assembler mostly for a small memory model.
;  Some Functions are specific to OMA4DRV.asm variables.
;       
;
; started: 9/15/88       DI
;
;**********************************************************************

             
;
;ReadToNextHex
;       Read file up to the start of the next valid hex character.
;       ENTRY   hFile contains opened file handle
;       EXIT    AX = 0  Failed
;               AX = char found
;               file pointer set to next uncommented ascii character
;
IF DEBUG
        PUBLIC  ReadToNextHex
PUBLIC  SEARCHLP
PUBLIC  RNHERROREXIT
PUBLIC  RNHEXIT        
ENDIF
ReadToNextHex PROC NEAR
        ; Read file up to next ASCII hex number character '0'..'F'
        PUSH    BP
        MOV     BP, SP
SEARCHLP:
;        MOV     SetWait, 1
        PUSH    0A0AH           ; LF
        PUSH    0D0DH           ; CR
        PUSH    3B3BH           ; ';' stop search at comment marker
        MOV     AH, 'f'         ; Push range
        MOV     AL, 'a'
        PUSH    AX
        MOV     AH, 'F'         ; Push range
        MOV     AL, 'A'
        PUSH    AX
        MOV     AH, '9'         ; Push range
        MOV     AL, '0'
        PUSH    AX
        PUSH    6               ; 6 ranges
        CALL    SetPosNext      ; Go looking
        MOV     SP, BP          ; Clear stack
;        MOV     SetWait, 0

;        MOV     CharBuf2, AL
;        PUSH    DS              ; segment
;        PUSH    OFFSET CharBuf2 ; offset
;        PUSH    1H              ; length 
;        CALL    MemDump


        AND     AL, AL          ; check to see if found
        JZ      RNHERROREXIT    ; unexpected end of file

        CMP     AL, ';'         ; check to see if hit comment first
        JZ      RNHNEWLINE
        CMP     AL, 0DH         ; check to see if hit comment first
        JZ      RNHNEWLINE
        CMP     AL, 0AH
        JNZ     RNHEXIT         ; character good

RNHNEWLINE:
        CALL    ReadToNextLine  ; Ignore rest of line and get next line
        AND     AL, AL          ; Check for read to EOF or error
        JNZ     SEARCHLP        

RNHERROREXIT:
        XOR     AX, AX
RNHEXIT:        
        POP     BP
        RET
ReadToNextHex ENDP
;
; IgnoreSpaces
;       Read past any space or TAB chars.
;       ENTRY   ES:[DI] = pointer to string
;       EXIT    ES:[DI] = pointer to next non-space, non-tab char
;

IF DEBUG
        PUBLIC  IgnoreSpaces
PUBLIC  NEXT_CHAR_TYPE
PUBLIC  TOSS_SPACES
PUBLIC  TOSS_TABS
ENDIF
IgnoreSpaces PROC NEAR

        MOV     AL, ' '
        MOV     AH, 09H         ; TAB char
NEXT_CHAR_TYPE:    
        CMP     AL, ES:[DI]
        JZ      TOSS_SPACES
        CMP     AH, ES:[DI]
        JZ      TOSS_TABS
        RET

TOSS_SPACES:    ; read up to next non-' ' character
        INC     DI
        CMP     AL, ES:[DI]
        JZ      TOSS_SPACES
        JMP     NEXT_CHAR_TYPE

TOSS_TABS:  ; read up to next non-TAB character
        INC     DI
        CMP     AH, ES:[DI]
        JZ      TOSS_TABS
        JMP     NEXT_CHAR_TYPE
IgnoreSpaces   ENDP

;
; StrReadToSpace
;       Read to next space, TAB, or NULL char.
;       ENTRY   ES:[DI] = pointer to string
;       EXIT    ES:[DI] = pointer to next space, TAB, or NULL char
;

IF DEBUG
        PUBLIC  StrReadToSpace
PUBLIC  FOUND_SPACE
ENDIF
StrReadToSpace PROC NEAR
        CMP     BYTE PTR ES:[DI], ' '
        JZ      FOUND_SPACE
        CMP     BYTE PTR ES:[DI], 09H    ; compare to TAB
        JZ      FOUND_SPACE
        CMP     BYTE PTR ES:[DI], 0      ; compare to null
        JZ      FOUND_SPACE
        INC     DI
        JMP     StrReadToSpace

FOUND_SPACE:
        RET
StrReadToSpace ENDP


;
; SetPosNext   C TYPE CALL - conserves BP
;       Read to the next character in a given range (from one or more 
;       possible ranges) using fHandle. 
;       ENTRY   opened file handle in fHandle
;               STACK  
;                ... (more ranges)
;                SP + 5 - High end of range        
;                SP + 4 -  Low end of range 
;                SP + 2 -  number of possible matching ranges placed on stack
;       EXIT    AX = 0  Not Found
;                    matching char found;
;               CharBuf contains last block read starting from previous 
;               position.  May be useful to read in one line's worth of
;               chars.
;               afile has position of matching char relative to CharBuf
;

IF DEBUG
        PUBLIC  SetPosNext
PUBLIC  FILLBUF
PUBLIC  CHKALLCHARS
PUBLIC  TESTLOW
PUBLIC  TESTHI
PUBLIC  LOOPAGAIN
PUBLIC  ENDSRCHLP
PUBLIC  FOUND_A_CHAR
PUBLIC  BETTER_POS
PUBLIC  NEXT_CHAR
PUBLIC  SETFILEPTR
PUBLIC  NOTFOUND
ENDIF

SetPosNext PROC NEAR
        PUSH    BP
        MOV     BP, SP
        ADD     BP, 6           ; point to matchable chars
        PUSHA
        ; fill CharBuf
FILLBUF:
        PUSH    hFile
        ; pointer to input buffer
        PUSH    DS
        PUSH    OFFSET CharBuf
        ; number of bytes to read
        PUSH    BufLen
        ; pointer to number of bytes read in
        PUSH    DS
        PUSH    OFFSET aFile
        CALL    DOSREAD

;        TEST    SetWait, 0FFH
;        JZ      AD1
;        PUSH    DS              ; segment
;        PUSH    OFFSET CharBuf; offset
;        PUSH    aFile              ; length 
;        CALL    MemDump
;AD1:

        AND     AX, AX          ; check for error
        JNZ     NOTFOUND
RD1:
        MOV     AX, [aFile]
        AND     AX, AX          ; check for EOF
        JZ      NOTFOUND
RD2:
        MOV     BX, SS:[BP-2]   ; put the number of matching chars into BX
        XOR     SI, SI          ; make [BP][SI] point to matchable char
        MOV     DX, [aFile]     ; Initialize the matching position number
CHKALLCHARS:
        AND     BX, BX          ; test for any chars to match
        JZ      SETFILEPTR      ; done trying to match with this buffer
        
        MOV     CX, DX          ; Get the max number of chars to read 
                                ; whole buffer or last match position
        JCXZ    SETFILEPTR
;        MOV     WORD PTR [CharBuf2 + 2], DX
;        PUSH    DS              ; segment
;        PUSH    OFFSET CharBuf2 + 2 ; offset
;        PUSH    8H              ; length 
;        CALL    MemDump
                
        XOR     DX, DX          ; reset count to 0
        
        MOV     DI, OFFSET CharBuf

        MOV     AX, SS:[BP][SI] ; Put the matchable char range into AX

;        TEST    SetWait, 0FFH
;        JZ      AD2
;        MOV     WORD PTR [CharBuf2], AX
;        PUSH    DS              ; segment
;        PUSH    OFFSET CharBuf2 ; offset
;        PUSH    2H              ; length 
;        CALL    MemDump
;AD2:

TESTLOW:
;        PUSH    AX
;        MOV     AL, [DI]
;        MOV     [Debugnum], AL
;        PUSH    WORD PTR[Debugnum-2]
;        MOV     [Debugnum-2], 's'
;        MOV     Debugnum-1, 'p'
;        PUSH    WORD PTR STDOUT
;        PUSH    DebugLen
;        PUSH    DS
;        PUSH    OFFSET Debug
;        CALL    DOSPUTMESSAGE  
;        POP     WORD PTR[Debugnum-2]
;        CALL    KeyWait
;        POP     AX

        CMP     [DI], AL        ; see if char is less than the low limit
        JB      LOOPAGAIN
TESTHI:
        CMP     [DI], AH        ; see if char is greater than the high limit
        JNA     ENDSRCHLP
LOOPAGAIN:                      ; char in buffer not in this range
        INC     DI
        INC     DX              ; Inc buffer char count
        LOOP    TESTLOW

ENDSRCHLP:
;        TEST    SetWait, 0FFH
;        JZ      AD3
;        MOV     WORD PTR [CharBuf2], DX
;        PUSH    DS              ; segment
;        PUSH    OFFSET CharBuf2 ; offset
;        PUSH    2H              ; length 
;        CALL    MemDump
;        CALL    KeyWait
;AD3:

        ADD     SI, 2           ; point to next char to look for
        DEC     BX              ; One char down
        JMP     CHKALLCHARS

SETFILEPTR: ; set the file pointer to the matching char
        MOV     AX, [aFile]       ; Get the matching position
        SUB     AX, DX          ; put offset from present position into AX
        JNZ     SFP1            ; match found
        JMP     FILLBUF         ; refill the buffer and try again
        
SFP1:   ; position file pointer at the matching char. Offset in AX
;        PUSH    DX
;        MOV     DX, [DI]                ; matching char
;        MOV     WORD PTR [CharBuf2 + 2], DX
;        PUSH    DS              ; segment
;        PUSH    OFFSET CharBuf2 + 2 ; offset
;        PUSH    1H              ; length 
;        CALL    MemDump
;        POP     DX

;        MOV     [CharBuf2 + 2], AL
;        PUSH    DS              ; segment
;        PUSH    OFFSET CharBuf2 + 2 ; offset
;        PUSH    1H              ; length 
;        CALL    MemDump

        PUSH    hFile
        ; distance to move (DWORD)
        PUSH    -1
        NEG     AX              ; offset from present position
        PUSH    AX        
        ADD     aFile, AX       ; Put the char offset from CharBuf into aFile
        ; relative to present position 
        PUSH    1
        ; pointer to szFile
        PUSH    DS
        PUSH    OFFSET szFile
        CALL    DOSCHGFILEPTR
        AND     AX, AX          ; test for error
        JNZ     NOTFOUND        ; error

        MOV     BX, DX       ; DX has finishing count for all char 
                             ; search loop           
        MOV     AL, [DI][BX]
        MOV     CharBuf2, AL ; save best matching char


        POPA
        POP     BP
        ; AH is still 0
        MOV     AL, CharBuf2           ; load matching char

;        TEST    SetWait, 0FFH
;        JZ      AD4
;        PUSH    DS              ; segment
;        PUSH    OFFSET CharBuf2 ; offset
;        PUSH    1H              ; length 
;        CALL    MemDump
;AD4:

        RET

NOTFOUND:
        POPA
        POP     BP
        XOR     AX, AX      
        RET             
        
SetPosNext ENDP

;
; HexStrToUNum   
;       Convert an ASCII hex string to an unsigned number.
;       Only reads as many characters as it needs to to fill the number
;       of bytes parameter.
;       ENTRY   
;               STACK 
;                SP + 11 - number of chars in ALPHANUMERIC string
;                SP + 10 - number of bytes to fill in Intel format              
;                SP + 8 -  destination segment
;                SP + 6 -  Destination offset
;                SP + 4 -  string segment 
;                SP + 2 -  string offset
;       EXIT    AX = 0 OK
;               AX = 1 error
;               0's in destination on error        
;

IF DEBUG
        PUBLIC  HexStrToUNum
PUBLIC  CNVRTCHAR
PUBLIC  FILLBYTES
PUBLIC  FILL_HIGH_NYBBLE
PUBLIC  ZEROFILL
PUBLIC  HSERROR
PUBLIC  HSDONE
ENDIF

HexStrToUNum PROC NEAR
        PUSH    BP
        MOV     BP, SP
        ADD     BP, 2           ; past where BP itself was pushed
        PUSH    DS
        ; put the string pointer into ES:[DI]
        MOV     DI, SS:[BP + 2]
        MOV     AX, SS:[BP + 4]
        MOV     ES, AX
        ; put the destination string into DS:[SI]
        MOV     SI, SS:[BP + 6]
        MOV     AX, SS:[BP + 8]
        MOV     DS, AX
        
        XOR     CH, CH
        MOV     DL, SS:[BP + 10] ; get max number of bytes
        MOV     CL, SS:[BP + 11] ; get number of chars in string
        ADD     DI, CX          ; set string offset after lowest digit
        DEC     DI              ; bring back to lowest digit
        PUSH    CX              ; save number of chars
        ; find the number of bytes the chars will fill
        INC     CL              ; round up a single nybble           
        SHR     CL, 1           ; 2 nybbles per byte
        SUB     DL, CL          ; leftover bytes
        POP     CX              ; get back number of chars
        PUSH    DX              ; save leftover bytes
CNVRTCHAR:
        XOR     AH, AH          ; Clear AH
FILLBYTES: ; converted char digits will be placed into destination 
        ; first nybble       
        MOV     AL, ES:[DI]     
        Call    HexCharToNum
        AND     BL, BL          ; check for error
        JNZ     HSERROR
        MOV     DL, AL
        DEC     CL              ; one less char
        JZ      FILL_HIGH_NYBBLE
        DEC     DI
        ; second nybble
        MOV     AL, ES:[DI]     
        Call    HexCharToNum
        AND     BL, BL          ; check for error
        JNZ     HSERROR
        SHL     AL, 4
        ADD     AL, DL
FILL_HIGH_NYBBLE:
        MOV     [SI], AL        ; store byte
        INC     SI              ; point to next byte
        DEC     DI              ; point to next higher digit
        AND     CL, CL          ; test to see if C is already 0 from jump to 
        JZ      FILLSPACE       ; FILL_HIGH_NYBBLE
        LOOP    FILLBYTES       ; decrements char count in CL
        
FILLSPACE:
        ;put in 0's if needed
        POP     CX              ; get number of leftover bytes
ZEROFILL:
        JCXZ    HSDONE
        MOV     BYTE PTR [SI], 0
        INC     SI    
        DEC     CL
        JMP     ZEROFILL

HSERROR:        ; fill destination with zeros
        POP     AX              ; Clear leftover bytes from stack
        MOV     AX, 1            ; flag error
        MOV     SI, SS:[BP + 6]  ; start of destination
        MOV     CL, SS:[BP + 10] ; get max number of bytes
        JMP     ZEROFILL

HSDONE:
        XOR     AX, AX          ; clear error flag
        POP     DS
        POP     BP
        RET

HexStrToUNum ENDP

;
; HexCharToNum   
;       Convert an ASCII hex character to a number.
;       ENTRY   AL = ASCII DIGIT
;       EXIT    AL = hex value
;               BX = 0 OK
;                    1 ERROR        
;
IF DEBUG
        PUBLIC  HexCharToNum
PUBLIC  ALPHA
PUBLIC  EXITDIGIT
PUBLIC  DIGITERROR
ENDIF

HexCharToNum PROC NEAR
        CMP     AL, '9' ; see if in numeric or alpha range
        JA      ALPHA
        CMP     AL, '0' ; check lower numeric limit
        JB      DIGITERROR
        SUB     AL, '0' 
        JMP SHORT EXITDIGIT

ALPHA:
        AND     AL, 5FH         ; Convert to upper case     
        CMP     AL, 'A' ; check lower numeric limit
        JB      DIGITERROR
        CMP     AL, 'F'
        JA      DIGITERROR
        SUB     AL, 'A' - 10 
EXITDIGIT:        
        XOR     BX, BX
        RET

DIGITERROR:
        MOV     BX, 1           ; flag error
        RET
HexCharToNum    ENDP

;
; ReadUnsigned   
;       Read an unsigned value starting at the current position 
;       in the file hFile.  
;       ENTRY   hFile has opened file handle
;               STACK 
;                SP + 6 -  Number of bytes        
;                SP + 4 -  destination segment
;                SP + 2 -  Destination offset
;       EXIT    AX = 0 OK
;               AX = 1 error
;               0's in destination on error        
;

IF DEBUG
        PUBLIC  ReadUnsigned
PUBLIC  STRTONUM
PUBLIC  RULERROREXIT
PUBLIC  RULEXIT
ENDIF

ReadUnsigned PROC NEAR
        ; read to next delimiter, puts number string into CharBuf
        ; push parameters for call to SetPosNext 
        PUSH    BP
        MOV     BP, SP          ; entry stack pointer into BP

        PUSH    0A0AH           ; LF
        PUSH    0D0DH           ; CR
        PUSH    0909H           ; TAB
        PUSH    3B3BH           ; ';'
        PUSH    2020H           ; SPACE
        PUSH    5               ; 5 chars to look for
        CALL    SetPosNext ; Go looking
        MOV     SP, BP          ; Clear stack

;        PUSH    DS              ; segment
;        PUSH    OFFSET CharBuf; offset
;        PUSH    aFile              ; length 
;        CALL    MemDump

        ; afile has the number of chars possibly representing a number
        MOV     AX, aFile
        MOV     BX, SS:[BP + 8] ; see if too many chars
        SHL     BL, 1           ; Up to twice as many chars as maximum bytes
        CMP     AX, BX
        JA      RULERROREXIT    ; too many chars before a delimiter
STRTONUM:
        SHR     BL, 1           ; get number of bytes back
        MOV     BH, AL          ; number of chars in string
        PUSH    BX
        ; Destination Segment
        MOV     BX, SS:[BP + 6]
        PUSH    BX
        ; Destination Offset
        MOV     BX, SS:[BP + 4]
        PUSH    BX
        ; String Segment
        PUSH    DS
        ; String Offset
        PUSH    OFFSET CharBuf
        CALL    HexStrToUNum    ; Error code in AX
        MOV     SP, BP          ; Clear stack
        JMP SHORT RULEXIT         

RULERROREXIT:
        MOV     AX, 1
RULEXIT:
        POP     BP
        RET
ReadUnsigned       ENDP


;
; ReadToNextLine
;       Read to the first char of the next line of hFile.  
;       ENTRY   hFile has opened file handle
;       EXIT    AX = 0 EOF or error
;               AX = first char
;

IF DEBUG
        PUBLIC  ReadToNextLine
ENDIF

ReadToNextLine PROC NEAR
        PUSH    BP
        PUSHA
        MOV     BP, SP          ; entry stack pointer into BP

;        MOV     [Debugnum], 'L'
;        PUSH    WORD PTR STDOUT
;        PUSH    DebugLen
;        PUSH    DS
;        PUSH    OFFSET Debug
;        CALL    DOSPUTMESSAGE
        
        ; Read to end of current line
        ; push parameters for call to SetPosNext 
        PUSH    0A0AH           ; LF
        PUSH    0D0DH           ; CR
        PUSH    2               ; 2 chars to look for
        CALL    SetPosNext      ; Go looking
        MOV     SP, BP          ; Clear stack
        AND     AL, AL          ; test for EOF
        JZ      RNLNOTFOUND
NEXTLINE:
        PUSH    hFile
        ; pointer to input buffer
        PUSH    DS
        PUSH    OFFSET CharBuf
        ; number of bytes to read
        PUSH    1
        ; pointer to number of bytes read
        PUSH    DS
        PUSH    OFFSET aFile
        CALL    DOSREAD
        AND     AX, AX          ; check for error
        JNZ     RNLNOTFOUND
        TEST    aFile, 0FFFFH   ; check for EOF
        JZ      RNLNOTFOUND
        MOV     AL, CharBuf     ; See what was next
        CMP     AL, 0AH         ; See if another CR or LF
        JZ      NEXTLINE      
        CMP     AL, 0DH
        JZ      NEXTLINE
        ; not another CR or LF
        ; Step file pointer back 1
        ; position file pointer at the matching char
        PUSH    hFile
        ; distance to move (DWORD)
        PUSH    -1
        PUSH    -1
        ; relative to present position 
        PUSH    1
        ; pointer to szFile
        PUSH    DS
        PUSH    OFFSET szFile
        CALL    DOSCHGFILEPTR
        AND     AX, AX          ; test for error
        JZ      RNLFOUND     ; error
      
RNLNOTFOUND:
        POPA
        POP     BP
        XOR     AX, AX
        RET
RNLFOUND:
        POPA
        POP     BP
        XOR     AH, AH
        MOV     AL, CharBuf
        RET
ReadToNextLine  ENDP


;
; ReadExpMemSize
;       Read the system configuration RAM to find the expanded memory size 
;       EXIT    Expanded Memory size in Kbytes put into SystemMemory.
;               System memory slots (2Mbyte/slot) are denoted in MemSlots.
;

IF DEBUG
        PUBLIC  ReadExpMemSize
ENDIF

ReadExpMemSize PROC NEAR
        PUSH    ES
        PUSH    DI
        PUSH    BX
        PUSH    CX
        XOR     AH, AH
        MOV     AL, EXPANDED_MEM_2_LOW  ; low byte of expanded memory value
        OUT     ADDRESS_PORT_8042, AL
        IN      AL, DATA_PORT_8042      
        PUSH    AX

        MOV     AL, EXPANDED_MEM_2_HIGH  ; high byte of expanded memory value
        OUT     ADDRESS_PORT_8042, AL
        IN      AL, DATA_PORT_8042      
                
        POP     BX                      ; get low byte back
        MOV     AH, AL
        MOV     AL, BL
        ADD     AX, 400H                ; assume low memory full (1Mbyte)
        MOV     [SystemMemory], AX      ; store total memory size in K's

        ; round up memory size in K to next 2Mbyte value
        TEST    AX, 7FFH                ; check for a MOD 800H (2Mbyte) value
        JZ      ENDREMS
        AND     AX, 0F800H               ; round up to next 2M boundary
        ADD     AX, 800H

ENDREMS:
        ; Put system memory slot assignments into MemSlots
        SHR     AX, 11          ; convert Kbyte to # of 2Mbyte blocks
        MOV     CX, AX          ; block count in CX
        MOV     DI, OFFSET MemSlots
        PUSH    DS
        POP     ES
        MOV     AL, -1          ; system memory flag
        REP STOSB               ; fill slots with system memory flag
        POP     CX
        POP     BX
        POP     DI
        POP     ES
        RET
ReadExpMemSize ENDP
;************************************************************************
;*                              END                                     *
;************************************************************************
