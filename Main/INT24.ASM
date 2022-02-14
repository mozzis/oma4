        PAGE    ,132
; ---------------------------------------------------------------------------
;
;       INT24.ASM
;
;       Written by:     TLB     Version 1.00       26 May       1988
;       Worked on:      TLB     Version 1.01    
;
;       INT24.ASM allows a program written in a high-level language
;       to trap int 24h calls, which occur when DOS has an error it 
;       doesn't know what to do with.  This prevents the ugly
;       "Abort, Retry, Fail?" message from trashing your beautiful
;       screen layout.  In addition, the user is never given the
;       option of crashing the program by pressing "A" (abort),
;       which is probably the first thing the user would do when
;       presented with such a message.
;
;       To intercept these traps, your program should call 
;       prepare_int24_trap() in the main() routine before doing
;       anything else.  Then, following any I/O routine which might
;       get in trouble, it should check the (unsigned int) variable 
;       DOS_int24_info to see if it has been set to some non-zero
;       value.  This variable's high byte contains what DOS returned in 
;       the AH register and the low byte what was in the low byte of
;       the DI register.  DOS_int24_info must be reset to zero by the 
;       program after the error has been processed, so that the next 
;       function which checks it doesn't get a leftover error signal.
;
;       AH values:
;               bit 7 - 0 = disk error, 1 = other I/O error
;               bits 1,2 - disk location where error occurred
;                       00 - DOS area
;                       01 - File Allocation Table (FAT)
;                       10 - directory
;                       11 - file area
;               bit 0 - 0 = read error, 1 = write error
;
;       DI values (DOS error code):
;                0 - write-protected
;                1 - unknown unit
;                2 - drive not ready
;                3 - unknown command
;                4 - data error (bad CRC)
;                5 - bad request structure length
;                6 - seek error
;                7 - unknown media type
;                8 - sector not found
;                9 - printer out of paper
;               10 - write fault
;               11 - read fault
;               12 - general failure (including unformatted diskette)
; ---------------------------------------------------------------------------
.MODEL LARGE

        PUBLIC  _prepare_int24_trap
        PUBLIC  _DOS_int24_info
        PUBLIC  _dev_err_info

IGNORE          EQU     0       ; 
RETRY           EQU     1       ; dangerous, could lead to endless loop
ABORT           EQU     2       ; the ABSOLUTE WORST way to handle the problem
FAIL            EQU     3       ; 


DOS_SERVICE_INT EQU     21h
DOS_CRITICAL_INT EQU    24h
SET_VECTOR      EQU     25h


.DATA

_DOS_int24_info DW      0
_dev_err_info   DW      0

.CODE
; ---------------------------------------------------------------------------
;       int24_handler
;
;       is not called by the application program; it is executed
;       when int 24 is invoked by DOS.  This routine does nothing
;       more than save the useful information in a variable that
;       the application program can see, then does a return from
;       interrupt, which brings it back to DOS.  DOS is told to
;       return a failure code for the function that caused the 
;       problem.
; ---------------------------------------------------------------------------

        ASSUME  DS: DGROUP

int24_handler PROC FAR
        sti                                     ; allow other interrupts

        mov   si, seg _DOS_int24_info           ; get segment of variable
        push  ds                                ; save current data segment
        mov   ds, si                            ; use this variable's ds

        mov   word ptr ds:_DOS_int24_info, di   ; store di data into var

        mov   byte ptr ds:_DOS_int24_info+1,ah  ; store ah data into var
        mov   word ptr ds:_dev_err_info,ax      ; store ax data into var
        pop   ds                                ; restore data segment
        mov   al, FAIL                          ; tell DOS to fail function

        iret                                    ; return interrupt (to DOS)

int24_handler ENDP


; ---------------------------------------------------------------------------
;       prepare_int24_trap
;
;       is called by the main routine in the applications program to
;       hook the int 24 service routine to the interrupt vector.
;
;       requires:       (void)
;       returns:        (void)
;       side effects:   modifies the int 24 vector address to point
;                       to a temporary service routine.  DOS will
;                       reset to the default service routine after
;                       the program terminates.
; ---------------------------------------------------------------------------

_prepare_int24_trap PROC FAR
        push    ds

        mov     ah, SET_VECTOR          ; call DOS to point critical int
        mov     al, DOS_CRITICAL_INT    ; vector to our routine
        mov     dx, seg int24_handler
        mov     ds, dx
        mov     dx, offset int24_handler
        int     DOS_SERVICE_INT

        PUSH    SI
        mov     si, seg _DOS_int24_info ; get segment of variable
        mov     ds, si                  ; use this variable's ds
        POP     SI

        mov     _DOS_int24_info, 0      ; clear error variable

        pop     ds
        ret
_prepare_int24_trap ENDP

        END

