;************************************************************************
;
;       anytype RMFunctionCall( void *RMFunction, RmParam1, RmParam2, etc. )
;
;       Switched to real mode and calls RMFunction with properly stacked
;       parameters.  Returns values in AX, DX, math coprocessor, etc.
;       as returned by RMFunction so that they mey be used as needed,
;       just change the C SwitchMode function prototype to match
;       RMFunction's.  SwitchMode is safer being called from assembly
;       is only theoretically possible to call from C.  A C call may or
;       may not work, I don't have the time to test it and figure out
;       limitations just now.
;
;       _RMFCALL_DATA and _RMFCALL_TEXT must be transparent segments
;       defined using MAKEPM.EXE.  Any address being passed to the
;       RMFunction must be real mode compatible
;
;************************************************************************

.286

include DOS16M.INC
EXTRN   __d16info : BYTE

;       DGROUP  group   _RMFCALL_DATA   9/20/90 DAI
_RMFCALL_DATA   segment word public 'DATA'
assume ds:_RMFCALL_DATA

_PtrToProtected dd      ?
_PtrToReal      dd      ?
_LocalBP        DW      ?
_LocalSS        DW      ?
_LocalDS        DW      ?
_LocalES        DW      ?
_EnteringDS     DW      ?
_EnteringBP     DW      ?
_EnteringES     DW      ?
_EnteringAX     DW      ?
_EnteringBX     DW      ?
_EnteringCX     DW      ?
_EnteringDX     DW      ?
_ReturnAddr     DD      ?
_RMCallAddr     DD      ?
_CallReturn     DD      ?

_RMFCALL_DATA ends

public _PtrToProtected, _PtrToReal, _LocalBP, _LocalSS, _LocalDS, _LocalES
public _EnteringDS, _EnteringBP, _ReturnAddr, _RMCallAddr, _CallReturn

_RMFCALL_TEXT       segment byte    public  'CODE'

assume ds:_RMFCALL_DATA, cs:_RMFCALL_TEXT

public _RMFunctionCall
_RMFunctionCall proc    far

        PUSH    AX
        PUSH    DS
        mov     ax, SEG _RMFCALL_DATA
        mov     ds, ax
        POP     _EnteringDS
        POP     _EnteringAX
        MOV     _EnteringBX, BX
        MOV     _EnteringCX, CX
        MOV     _EnteringDX, DX
        mov     _EnteringBP, BP
        mov     _EnteringES, ES

        ; store the return address
        pop     _ReturnAddr.lo
        pop     _ReturnAddr.hi

        pop     _RMCallAddr.lo
        pop     _RMCallAddr.hi

        ; ******************************************************************
        ; NOW the top of the stack should be the real mode calling address of
        ; the function to be called, followed by that function's parameters
        ; with any data pointers referring to transparent segments

        assume es:SEG __d16info
        mov     ax, SEG __d16info
        mov     es, ax

        ; save protected mode call pointer
        mov     ax, WORD PTR es:[__d16info.D16ToPMP.hi]
        mov     _PtrToProtected.hi, ax
        mov     ax, WORD PTR es:__d16info.D16ToPMP.lo
        mov     _PtrToProtected.lo, ax
        ; save real mode call pointer
        mov     ax, WORD PTR es:[__d16info.D16ToRMP.hi]
        mov     _PtrToReal.hi, ax
        mov     ax, WORD PTR es:__d16info.D16ToRMP.lo
        mov     _PtrToReal.lo, ax

        mov     _LocalBP, BP
        mov     _LocalSS, SS
        mov     _LocalDS, DS
        mov     _LocalES, ES
          ; DOS16M function switch to real mode
        call    dword ptr DS:_PtrToReal
;        and     AX, AX                  ; 0 if failed
        mov     ax, SEG _RMFCALL_DATA
        mov     ds, ax
;        jnz     DoCall
;        jmp     done

DoCall:         ; call real mode function with stack pointing to parameters

        STI                             ; enable interrupts
        MOV     AX, _EnteringAX
        MOV     BX, _EnteringBX 
        MOV     CX, _EnteringCX 
        MOV     DX, _EnteringDX 
        mov     BP, _EnteringBP
        mov     ES, _EnteringES         ; better be transparent if function 
                                        ; expects to use this 

;        CALL    dword ptr _RMCallAddr ; this didn't work, need to restore DS
        ; fake the call using a return with an address that was pushed onto 
        ; the stack
        push    cs                      ; push return addr
        push    OFFSET DoCallRet               
        push    _RMCallAddr.hi          ; push the address to call
        push    _RMCallAddr.lo
        mov     ds, _EnteringDS         ; restore DS to its value on entry
        ret     ; return that does the fake call, tricky huh?

DoCallRet:                              ; pseudo call will return here
        push    SEG _RMFCALL_DATA
        pop     DS
        mov     _CallReturn.hi, dx
        mov     _CallReturn.lo, ax

to_prot:
          ; DOS16M function switch to protected mode
        push    _LocalES                ; push protected mode es
        push    DS                      ; push protected mode ds, transparent
        push    _LocalSS                ; push protected mode ss
        push    CS                      ; push protected mode cs, transparent
        call    dword ptr DS:_PtrToProtected
;        and     AX, AX                  ; 0 if failed
;        jnz     to_prot2

to_prot2:
        ; clean stack
        pop     BX              ; cs register
        pop     ss
        pop     ds
        pop     es
        STI                             ; enable interrupts

done:
        mov     ax, _CallReturn.lo
        mov     dx, _CallReturn.hi
        mov     BP, _EnteringBP
        push    _RMCallAddr.hi           ; replace void * parameter
        push    _RMCallAddr.lo
        push    _ReturnAddr.hi           ; replace return address
        push    _ReturnAddr.lo

        mov     DS, _EnteringDS
        ret

_RMFunctionCall endp

_RMFCALL_TEXT       ends

	end
