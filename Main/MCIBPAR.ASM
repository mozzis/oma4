        page    ,132
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                               ;
; Language  Interface to Microsoft C and IBM C  ;
; Rev. C.1                                      ;
; (c) Copyright 1986, National Instruments      ;
;               All rights reserved.            ;
;                                               ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                               ;
; *** Date of last PAR edit: 9/16/93 MLM        ;
; *** Date of last edit: 10-22-86               ;
; *** Current Compiler Level = 6.0              ;
;                                               ;
; This file is linked to a compiled application ;
; program written in Microsoft C or IBM C to    ;
; produce an  executable file that accesses the ;
; GPIB-488  handler.  The purpose of this code  ;
; is to take the handler function calls and     ;
; construct the appropriate DOS calls for       ;
; accessing the handler.                        ;
;                                               ;
; Refer to the IBHANDLER and IBPCHANDLER specs  ;
; for the design specifications for this        ;
; interface.                                    ;
;                                               ;
; This interface assumes it may change all but  ;
; the si, di and segment registers, and returns ;
; values via ax and the three global variables  ;
; ibsta, iberr, and ibcnt.                      ;
;                                               ;
; This interface may be adapted to any of the   ;
; Miscrosft C or IBM C memory models.           ;
; Follow the instructions in the OUT ops below. ;
; The object files should be named:             ;
;   MCIBS.OBJ - small memory model              ;
;   MCIBM.OBJ - medium memory model             ;
; * MCIBC.OBJ - compact memory model            ;
;   MCIBL.OBJ - large or huge memory model      ;
; depending on memory model used.               ;
; * - new for version 4.0                       ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Memory Model    LPROG Value    LDATA Value
;
;   Small              0              0
;   Medium             1              0
;   Compact            0              1
;   Large/Huge         1              1

.286

include DOS16M.INC

EXTRN   _D16MemAlloc    : FAR    ; DOS16M allocate memory
EXTRN   _D16RealPtr     : FAR    ; DOS16M convert selector to real pointer

_WAIT   equ     0       ; function codes passed to handler
ONL     equ     1
RSC     equ     2
SIC     equ     3
SRE     equ     4
LOC     equ     5
RSV     equ     6
PPC     equ     7
PAD     equ     8
SAD     equ     9
IST     equ     10
DMA     equ     11
EOS     equ     12
TMO     equ     13
EOT     equ     14
GTS     equ     15
CAC     equ     16
RDF     equ     17
WRTF    equ     18
RPP     equ     19
POKE    equ     20
STOP    equ     21
CLR     equ     22
TRG     equ     23
PCT     equ     24
RSP     equ     25
BNA     equ     26
FIND    equ     27
RD      equ     28
RDA     equ     29
WRT     equ     30
WRTA    equ     31
CMD     equ     32
CMDA    equ     33
DIAG    equ     34
XTRC    equ     35
TRAP    equ     36

EDVR    equ     0       ; iberr code for DOS error in INT 21
ERRBIT  equ     8000H   ; ERR bit in ibsta


_GPIB_DATA   segment word public 'DATA';
assume ds:_GPIB_DATA

; This just defines the structure type

GPIB_ST STRUCT
  cprt     BYTE    "(c) Copyright 1986, National Instruments"
  hndlr    DWORD   ?
  hndl     WORD    ?
  stat     WORD    ?
  errw     WORD    ?
  cntw     WORD    ?
GPIB_ST ENDS

pGPIB TYPEDEF FAR PTR GPIB_ST             ; DATE pointer type

; declare pointers to the globals for prot and real mode

     public _GPIB_PTR
_GPIB_PTR pGPIB FAR ?          ; protected mode pointer to GPIB data
_GPIB_RPTR pGPIB FAR ?         ; real mode pointer to GPIB data

; This string kept for possible legal reasons

ibcprt  db "(c) Copyright 1986, National Instruments"       ;40 bytes

; define register structures for DPMI calls

reg UNION
  l DWORD ?
  s WORD ? 
  reg ENDS

reg_t TYPEDEF reg

dpmiRegs_t STRUCT
        drEDI   reg_t <0>
        drESI   reg_t <0>
        drEBP   reg_t <0>
        drRESV  reg_t <0>
        drEBX   reg_t <0>
        drEDX   reg_t <0>
        drECX   reg_t <0>
        drEAX   reg_t <0>
        drFlags WORD 0
        drES    WORD 0
        drDS    WORD 0
        drFS    WORD 0
        drGS    WORD 0
        drIP    WORD 0
        drCS    WORD 0
        drSP    WORD 0          ; NOTE: When SS:SP is input as 0:0,
        drSS    WORD 0          ; the DPMI host attempts to provide a stack
dpmiRegs_t ENDS

tregs TYPEDEF dpmiRegs_t
pregs TYPEDEF FAR PTR dpmiRegs_t

r_regs tregs <>
p_regs pregs r_regs

     public _ibsta
     public _iberr
     public _ibcnt

_ibsta  DW  0
_iberr  DW  0
_ibcnt  DW  0

_GPIB_DATA ends                                            ;52 bytes

MCIB_TEXT       segment byte    public  'CODE'

assume ds:_GPIB_DATA, cs:MCIB_TEXT

        public _ibbna,  _ibcac,  _ibclr,  _ibcmd,  _ibcmda
        public _ibdiag, _ibdma,  _ibeos,  _ibeot,  _ibfind
        public _ibgts,  _ibist,  _ibloc,  _ibonl,  _ibpad,  _ibpct
        public _ibpoke, _ibppc,  _ibrd,   _ibrda,  _ibrdf
        public _ibrpp,  _ibrsc,  _ibrsp,  _ibrsv,  _ibsad
        public _ibsic,  _ibsre,  _ibstop, _ibtmo,  _ibtrg
        public _ibwait, _ibwrt,  _ibwrta, _ibwrtf, _ibxtrc, _ibtrap

ibprocs proc    far
args    equ     word ptr 6

_ibfind:                        ; bd = ibfind (name)
        push    bp
        mov     bp,sp
        push    di
        push    si
        push    ds              ; Save original DS.

        ; DS:DX <= address of device name string. 
        push    ss:args+2[bp]
        pop     ds
        mov     dx,ss:args[bp]
        mov     ax,3D00H        ; DOS Open
        int     21H
        jc      ibfer           ; Branch if error.

        mov     bx,ax           ; save file handle

        push    SEG _GPIB_PTR
        pop     ds   

        mov     dx,word ptr _GPIB_PTR+2  ; if globals already allocated
        or      dx,word ptr _GPIB_RPTR+2 ; don't realloc them
        jne     ibfnd1

        push    bx
        mov     ax,0100h        ; allocate space for 52 bytes of GPIB stuff
        mov     bx,4            ; use DPMI DOSAlloc for 4 paras = 64 bytes
        int     31h             ; DOS16M supplies DPMI if none present
        pop     bx
        jc      ibfer           ; Branch if error.

        mov     word ptr _GPIB_PTR+2,dx
        mov     word ptr _GPIB_RPTR+2,ax

ibfnd1: les     di, ds:_GPIB_PTR
        mov     si, offset ibcprt
        mov     cx,40
        cld
        rep     movsb
        mov     cx,12
        sub     al,al
        rep     stosb

        push    es
        pop     ds
        mov     ax,4402H        ; DOS Ioctl (handle still in bx)
        mov     dx,offset _GPIB_PTR.hndlr
        mov     cx,4            ; 10/29/92 MLM
        int     21H
        jc      ibfer
        mov     ax,3E00H        ; DOS Close
        int     21H
        jc      ibfer
        mov     ax,_GPIB_PTR.hndl       ; Return handle.
        test    ax,ax
        jns     ibfou
ibfer:                          ; If error...
        call    error
ibfou:
        pop     ds
        pop     si
        pop     di
        pop     bp
        ret

        ; The following functions do not have buffers as their second argument.

_ibstop:
        mov     al,STOP
        jmp     SHORT callfn_no_buffer

_ibonl:
        mov     al,ONL
        jmp     SHORT callfn_no_buffer

_ibrsc:
        mov     al,RSC
        jmp     SHORT callfn_no_buffer

_ibsic:
        mov     al,SIC
        jmp     SHORT callfn_no_buffer

_ibsre:
        mov     al,SRE
        jmp     SHORT callfn_no_buffer

_ibloc:
        mov     al,LOC
        jmp     SHORT callfn_no_buffer

_ibrsv:
        mov     al,RSV
        jmp     SHORT callfn_no_buffer

_ibppc:
        mov     al,PPC
        jmp     SHORT callfn_no_buffer

_ibpad:
        mov     al,PAD
        jmp     SHORT callfn_no_buffer

_ibsad:
        mov     al,SAD
        jmp     SHORT callfn_no_buffer

_ibist:
        mov     al,IST
        jmp     SHORT callfn_no_buffer

_ibdma:
        mov     al,DMA
        jmp     SHORT callfn_no_buffer

_ibeos:
        mov     al,EOS
        jmp     SHORT callfn_no_buffer

_ibtmo:
        mov     al,TMO
        jmp     SHORT callfn_no_buffer

_ibeot:
        mov     al,EOT
        jmp     SHORT callfn_no_buffer

_ibgts:
        mov     al,GTS
        jmp     SHORT callfn_no_buffer

_ibcac:
        mov     al,CAC
        jmp     SHORT callfn_no_buffer

_ibclr:
        mov     al,CLR
        jmp     SHORT callfn_no_buffer

_ibtrg:
        mov     al,TRG
        jmp     SHORT callfn_no_buffer

_ibpct:
        mov     al,PCT
        jmp     SHORT callfn_no_buffer

_ibpoke:
        mov     al,POKE
        jmp     SHORT callfn_no_buffer

_ibtrap:
        mov     al,TRAP
        jmp     SHORT callfn_no_buffer

_ibwait:
        mov     al,_WAIT                ; 9/19/90 DAI
;       jmp     callfn_no_buffer


; Callfn sets up the arguments for the call to handler, which expects:
;       al = function code (ah is immaterial)
;       bx = handle (offset of board or device structure) (not relevent
;               for IBFIND)
;       ds:dx = buffer pointer if function has buffer as argument, else
;       dx = 2nd argument if function has one and ds is immaterial, else
;               ds and dx are immaterial
;       cx = 3rd argument if function has one, else it is immaterial
;       es:si = address of the 3 global variables
;       di is immaterial

; To enable use with protected mode programs, the National driver (which
; was loaded in config.sys) is called via DPMI function 0301, call real
; mode function.  The registers to be used in real mode, and the address
; of the real mode function, are stored in the DPMI_regs structure defined
; above.  All buffer addresses are translated from selector:offset to 
; segment:offset form; this assumes that the buffers are located in the 
; lower 1 Meg address space.  Arguments are removed from the stack as per 
; the National interface spec and put into the appropriate places into the 
; register structure.  After the call, the values for ibsta, iberr, and
; ibcnt are copied from the real mode addresses given to the driver into
; the (possibly) protected mode globals ibsta, iberr, and ibcnt.

; Within callfn, carry clear indicates no buffer argument;
; carry set indicates that the argument after the handle is a buffer.

callfn_no_buffer:
        clc
        jmp short callfn
callfn_with_buffer:
        stc
callfn:
        push    bp
        mov     bp,sp
        push    di
        push    si
        push    ds                      ; Save original segment registers
        push    es

        push    SEG _GPIB_PTR           ; get address of pointer to globals
        pop     ds
        les     si, _GPIB_PTR           ; get pointer to globals

        cld                             ; Autoincrement for the LODS instructions.
        lea     si,ss:args[bp]          ; Address of first argument

        mov     r_regs.drEAX.s,ax       ; Save function code

        mov     bp,sp

        lods    ss:args                 ; First argument (Handle)
        mov     r_regs.drEBX.s,ax       ; Save first argument

        lods    ss:args                 ; Second argument
        mov     r_regs.drEDX.s,ax       ; Save second argument

        mov     bx,ds                   ; Save current DS

        jnc     callf0                  ; if carry set, a segment value
                                        ; follows in the next argument word,
        lods    ss:args                 ; so get buffer segment in ax
        mov     bx,ax                   ; bx= I/O buffer segment | current DS 
callf0:
        lods    ss:args                 ; Last argument (e.g. count)
        mov     r_regs.drECX.s,ax       ; Save last argument

        mov     di,word ptr ss:[bp]     ; Check to see if ibfind was called...
        or      di,word ptr ss:2[bp]    ; handler entry should be non-zero
        jnz     callf1
        jmp     nodever
callf1:
        mov     ax,0006h                ; Set for DPMI get segment base
        int     31h                     ; Get base address of sel in bx

        shl     cx,12                   ; make real mode segment
        shr     dx,4                    ; make real mode segment
        add     cx,dx
        mov     r_regs.drDS,cx          ; save as real mode DS

        lea     si,_GPIB_PTR.stat       ; es:SI <== address of IBSTA

        mov     ax, word ptr _GPIB_RPTR+2
        mov     r_regs.drES,ax          ; set real mode ES to segment of ibsta
        mov     r_regs.drESI.s,si       ; set real mode SI to offset of ibsta

        mov     ax, word ptr es:2[hndlr]
        mov     r_regs.drCS,ax          ; Save handler entry point segment
        mov     ax, word ptr es:[hndlr]  
        mov     r_regs.drIP,ax          ; Save handler entry point offset 

        mov     ax,0301h                ; DPMI function to call real mode
        sub     cx,cx                   ; No args on stack
        les     di, p_regs              ; Address of real mode regs
        int     31h                     ; Exec DPMI function
           
callf3:
        les     si, _GPIB_PTR           ; get pointer to globals
        mov     ax,es:[errw]
        mov     _iberr,ax               ; update globals
        mov     ax,es:[cntw]
        mov     _ibcnt,ax
        mov     ax,es:[stat]
        mov     _ibsta, ax
        pop     es
        pop     ds
        pop     si
        pop     di
        pop     bp
        ret

        ; The following functions have buffers as their second argument.
        ; The carry flag is set so callfn can take this into account.
_ibrd:
        mov     al,RD
        jmp     callfn_with_buffer

_ibrda:
        mov     al,RDA
        jmp     callfn_with_buffer

_ibrdf:
        mov     al,RDF
        jmp     callfn_with_buffer

_ibwrtf:
        mov     al,WRTF
        jmp     callfn_with_buffer

_ibwrt:
        mov     al,WRT
        jmp     callfn_with_buffer

_ibwrta:
        mov     al,WRTA
        jmp     callfn_with_buffer

_ibcmd:
        mov     al,CMD
        jmp     callfn_with_buffer

_ibcmda:
        mov     al,CMDA
        jmp     callfn_with_buffer

_ibrpp:
        mov     al,RPP
        jmp     callfn_with_buffer

_ibrsp:
        mov     al,RSP
        jmp     callfn_with_buffer

_ibbna:
        mov     al,BNA
        jmp     callfn_with_buffer

_ibdiag:
        mov     al,DIAG
        jmp     callfn_with_buffer

_ibxtrc:
        mov     al,XTRC
        jmp     callfn_with_buffer

ibprocs endp

error   proc    near
        push    SEG _GPIB_PTR
        pop     ds

        mov     _GPIB_PTR.stat,ERRBIT   ; set ERR bit in status word
        mov     _GPIB_PTR.errw,EDVR     ; EDVR indicates INT 21H faulure.
        mov     _GPIB_PTR.cntw, ax      ; DOS error code goes into IBCNT.
        mov     ax,-1              ; -1 handle signifies error to the caller.
        ret
error   endp

nodever proc near
        mov     ax,6                    ; DOS non-existent device error
        call    error
        jmp     callf3
nodever endp

MCIB_TEXT       ends

        end
