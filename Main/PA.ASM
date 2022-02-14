;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   pa.asm -- contains D16PhysAddress ()
;
;
;   $Revision: 1 $
;   $Date: 6/18/99 1:52a $
;   $Logfile: /OMA4000/Main/PA.ASM $
;   $Log: /OMA4000/Main/PA.ASM $
; 
; 1     6/18/99 1:52a Maynard
;   
;      Rev 1.1   08 Feb 1991 13:23:52   jab
;   Changed _d16info reference to _d16_selectors reference for easier portability
;   among compilers...
;
;      Rev 1.0   21 Sep 1990 15:39:56   JAB
;   Initial revision.
;
;
;   Copyright (c) 1990 Rational Systems, Incorporated
;   All Rights Reserved
;
;
codesize  TEXTEQU <LARGE>

        .286p
        .model  LARGE, C
        include run.mac

        extrn   __d16_selectors:word

d16sels struc
svers   dw      0
sgdts   dw      0
sidts   dw      0
skcode  dw      0
skdata  dw      0
spsps   dw      0
senvs   dw      0
smem0s  dw      0
d16sels ends

DOS16LIB_TEXT segment word public 'code'
        assume  cs:DOS16LIB_TEXT, ds:DGROUP

        public  D16PhysAddress          ; long = D16PhysAddress(long linear)

D16PhysAddress proc linear:dword
        local   savebp:word

        mov     cx, [__d16_selectors].skdata
        mov     es, cx
        mov     cx, es:[2Eh]            ; cl=.swmode, ch=.cpumod
        cmp     ch, 3
        jl      l_eq_p                  ; 286, linear == physical
        or      cl, cl                  ; see if DPMI, error if yes
        jnz     check_paging
err:    mov     ax, -1
        mov     dx, ax
done:   ret

l_eq_p: 
        mov     ax, word ptr [linear]
        mov     dx, word ptr [linear+2]
        jmp     done


DOS16LIB_TEXT ends
        .386p
DOS16LIB_TEXT segment use16
        assume  cs:DOS16LIB_TEXT
check_paging:
        mov     ecx, cr0
        or      ecx, ecx
        jns     l_eq_p                  ; no paging, return linear
        push    ds                      ; must exit through pgret label
	mov	ah, 0FFh
	mov	dh, 00Eh		; get kernel info call
	int	21h
	mov	ds, dx			; dx:ax -> xbrk
	mov	bx, ax
	movzx	ecx, ds:[bx].pgtab_rm	; get real mode para for page dir
	shl	ecx, 4			; convert to linear
        push    gdts
        pop     ds
        mov     bx, mem0s
        push    word ptr ds:[bx].gdtreserv      ; save descriptor
        or      byte ptr ds:[bx].gdtreserv, 8Fh  ; force infinite limit
        mov     ds, bx                  ; ds is now flat segment
        movzx   ebx, word ptr [linear+2]
        shr     bx, 22-16               ; highest 10 bits (pd index)
        mov     eax, ds:[ecx][ebx*4]    ; eax is pd entry
        test    al, 1                   ; presence test
        jz      short pgerr
        mov     ebx, [linear]
        shr     ebx, 12
        and     ebx, 03FFh              ; pte index
        push	ebx
        pop	ebx
        mov     eax, ds:[ecx+4096][ebx*4]    ; eax is pde
        push	eax
        pop	eax
        test    al, 1                   ; presence test
        jz      short pgerr
        and     ax, 0F000h              ; keep page base
        movzx   ebx, word ptr [linear]
        and     bx, 0FFFh               ; offset withing page
        add     eax, ebx                ; physical address of linear
        mov     edx, eax
        shr     edx, 16                 ; dx:ax = physical
pgret:  push    gdts
        pop     ds
        pop     ds:[mem0s].gdtreserv
        pop     ds
        jmp     done

pgerr:  mov     ax, -1
        cwd
        jmp     pgret

D16PhysAddress endp
DOS16LIB_TEXT ends

        end

