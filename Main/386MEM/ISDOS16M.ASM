.MODEL LARGE ; , C
.286                    ; Enable 80286 instructions
        .CODE


l0097:  nop
        xor     ax,ax
        lar     ax,cx
        jmp     SHORT target
target: test    ah,10
        jne     l00a6
        xor     cx,cx
l00a6:  ret

__DOS   PROC    FAR
        push    bp
        mov     bp,sp
        push    si
        push    di
        push    ds
        lds     bx,[bp+08h]
        mov     dx,[bx+0Eh]
        mov     si,[bx+06h]
        mov     di,[bx+04h]
        mov     cx,[bx+02h]
        call    l0097
        mov     es,cx
        mov     cx,[bx]
        call    l0097
        push    cx
        mov     ax,[bp+06h]
        mov     cx,[bp+10h]
        mov     bx,[bx+0Ch]
        pop     ds
        int     21
        push    bx
        push    ds
        lds     bx,[bp+08h]
        pop     word ptr [bx]
        mov     [bx+02h],es
        mov     [bx+04h],di
        mov     [bx+06h],si
        mov     [bx+0Eh],dx
        mov     [bx+10h],cx
        pop     word ptr [bx+0ch]
        mov     [bx+12h],ax
        pop     ds
        pop     di
        pop     si
        mov     ax,0000h
        cmc
        rcl     ax,1
        pop     bp
        retf
__DOS   ENDP

PUBLIC  _isD16M

_isD16M  PROC   FAR
        enter   0014h,00
        push    ds
        mov     ax,0098h
        mov     ds,ax
        mov     word ptr [bp-06h],0078h
        lea     ax,[bp-14h]
        push    ss
        push    ax
        push    0FF00h
        call    __DOS
        add     sp,06h
        cmp     byte ptr [bp-02h],01h
        sbb     ax,ax
        inc     ax
        pop     ds
        leave
        retf
        nop

_isD16M  ENDP

PUBLIC __is_pm

__is_pm  PROC  FAR
        xor     ax,ax
        push    sp
        pop     bx
        cmp     bx,sp     ; check for stack misalignment
        jne     is_finish ; if it was, 80286 is in real mode
        smsw    ax        ; else get machine status word
        and     ax,0001h  ; see if bit 0 is set
        je      is_finish ; if not, may be protected mode
        pushf
        pop     bx
        and     bh,0CFh
        push    bx
        popf
        pushf
        pop     bx
        and     bh,30h
        je      is_finish
        mov     ax,1686h
        int     2fh       ; ????
        xor     ax,1686h
is_finish: retf
__is_pm  ENDP


END
