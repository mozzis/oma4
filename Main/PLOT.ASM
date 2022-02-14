        page    ,132    ; 9/20/90 DAI
; --------------------------------------------------------------------------
;
;       plot.asm
;
;
;       plot support for oma4000 program
;
;
;
; --------------------------------------------------------------------------

        FALSE   EQU     0
        TRUE    EQU     1

        .MODEL  LARGE,C

        .386
        .387

INCLUDE OMATYP.INC                      ; 3/7/91 DAI
INCLUDE PLOT.INC


        SIGN_BIT        EQU     08000h

        INT_MAX         EQU     32767
        INT_MIN         EQU     -32767

;line_width EQU 75      ; !!!!!!!!!!!!!!!!!!    9/19/90 DAI

        .DATA
MIN16INT                DW      INT_MIN
MAX16INT                DW      INT_MAX
;LINWIDTWICE            DW      (line_width * 2)        9/19/90 DAI
;GlobalLineWidth         DW      ?                       ; 9/20/90 DAI
;GlobalDoubleLineWidth   DW      ?

;EXTERN   GlobalDoubleLineWidth  : SWORD

        .CODE

;public GlobalLineWidth                                  ; 9/20/90 DAI
;public GlobalDoubleLineWidth                            ; 9/20/90 DAI

EXTRN   gss_position:FAR

; WARNING! THIS MACRO TRASHES >>DS<< AND >>AX<< REGISTERS!!

LIMIT_TO_INT    MACRO
  LOCAL  testhi, done
        mov     ax, @data
        mov     ds, ax
        ficom   MIN16INT
        fwait
        fstsw   ax              ; put 87 flags into CPU flag register
        sahf                    ; 
        jae     testhi
        ffree   ST
        fild    MIN16INT

testhi: 
        ficom   MAX16INT
        fwait
        fstsw   ax              ; put 87 flags into CPU flag register
        sahf                    ; 
        jbe     done
        ffree   ST
        fild    MAX16INT
done:
        ENDM

; --------------------------------------------------------------------------
;
;       GssPosX
;
;
;
; --------------------------------------------------------------------------

; returns int
GssPosX         PROC FAR USES bx ds, \
                ppbox:FAR PTR PLOTBOX, xvalue:dword, XOffset:word

        lds     bx, ppbox
 fwait
        fld     xvalue
;       fmul    [bx].x.magnitude_factor
        fld     [bx].x.min_value
        cmp     [bx].x.ascending, 0
        jz      gpx_0
        fsub
        jmp     SHORT gpx_1
gpx_0:  fsubr
gpx_1:  fmul    [bx].x.inv_range
        fimul   [bx].x.axis_end_offset.xp
        fiadd   [bx].x.axis_zero.xp
        fiadd   XOffset
        LIMIT_TO_INT
        fistp   XOffset                 ; re-use for integer result - XLen
        fwait
        mov     ax, word ptr XOffset
        ret

GssPosX         ENDP



; --------------------------------------------------------------------------
;
;       GssPosY
;
;
;
; --------------------------------------------------------------------------

; returns int
GssPosY         PROC FAR USES bx ds, \
                ppbox:FAR PTR PLOTBOX, yvalue:dword, YOffset:word

        lds     bx, ppbox
 fwait
        fld     yvalue
;       fmul    [bx].y.magnitude_factor
        fld     [bx].y.min_value
        cmp     [bx].y.ascending, 0
        jz      gpy_0
        fsub
        jmp     SHORT gpy_1
gpy_0:  fsubr
gpy_1:  fmul    [bx].y.inv_range
        fimul   [bx].y.axis_end_offset.yp
        fiadd   [bx].y.axis_zero.yp
        fiadd   YOffset
        LIMIT_TO_INT
        fistp   YOffset                 ; re-use for integer result - YLen
        fwait
        mov     ax, word ptr YOffset
        ret

GssPosY         ENDP



; returns int (gss point)
FactorPoint     PROC FAR USES ds, \
                Value:dword, Factor:dword, BasePoint:dword, \
                OffsetValue:dword, ascends:word

        fld     Value
        fld     BasePoint
        cmp     ascends, 0
        jz      fp_0
        fsub
        jmp     SHORT fp_1
fp_0:   fsubr
fp_1:   fmul    Factor
        fiadd   OffsetValue
        LIMIT_TO_INT
        fistp   ascends ; re-use for integer result - point value
        fwait
        
        mov     ax, word ptr ascends
        ret

FactorPoint     ENDP



; --------------------------------------------------------------------------
;
;       CalcOffsetForZ
;
;
;
; --------------------------------------------------------------------------
; This section gets a floating exception at 2nd fldz if c optimization 
; is on.  Also gets GenProtFault under DOS16M when the Error box resulting
; from the exception goes away;

;CalcOffsetForZ  PROC FAR USES ax bx ds, \
;                ppbox:FAR PTR PLOTBOX, ZValue:dword, \
;                pXOffset:FAR PTR dword, pYOffset:FAR PTR dword
;
;        lds     bx, ppbox
;        fwait
;        fld     [bx].z.min_value
;        fld     ZValue
;        fcom
;        fwait
;        fstsw   ax              ; put 87 flags into CPU flag register
;        sahf                    ; 
;        jne     cofz_0          
;        ffree   ST
;        ffree   ST(1)
;        fldz
;        fwait
;        fldz
;        jmp     SHORT cofz_3
;
;cofz_0: cmp     [bx].z.ascending, 0
;        jz      cofz_1
;        fsubr
;        jmp     SHORT cofz_2
;
;cofz_1: fsub
;
;cofz_2: fwait 
;        fmul    [bx].z.inv_range
;        fwait
;        fld     ST              ; DUP top stack item
;        fimul   [bx].z.axis_end_offset.yp
;        fwait
;        fxch
;        fimul   [bx].z.axis_end_offset.xp
;
;cofz_3: fwait
;        LIMIT_TO_INT
;        lds     bx, pXOffset
;        fistp   dword ptr ds:[bx]
;        fwait
;        LIMIT_TO_INT
;        lds     bx, pYOffset
;        fistp   dword ptr ds:[bx]
;        fwait
;        ret
;
;CalcOffsetForZ  ENDP
;

; --------------------------------------------------------------------------
;
;       CalcClipRect
;
;
;
; --------------------------------------------------------------------------
;
;_CalcClipRect  PROC FAR USES bx cx dx si di ds es, \
;               ppbox:FAR PTR PLOTBOX, ZValue:dword, \
;               ClipRect:FAR PTR DESCARTES
;
;       lds     bx, ppbox
;       mov     di, ss
;       mov     es, di
; fwait
;       fld     [bx].y.restore_magnitude        ; push on FPP stack
;                                               ; to multiply with x min/max
;       fld     ZValue
;       sub     sp, 4
;       mov     di, sp
;       fstp    dword ptr es:[di]
;       fwait
;       fld     ST                              ; [bx].y.restore_magnitude
;       fmul    [bx].y.min_value
;       sub     sp, 4
;       mov     di, sp
;       fstp    dword ptr es:[di]
;       fwait
;       fmul    [bx].x.min_value
;       sub     sp, 4
;       mov     di, sp
;       fstp    dword ptr es:[di]
;       fwait
;       push    ds
;       push    bx              ; plotbox
;;      fwait
;       call    gss_position
;       add     sp, 16
;       push    ax
;       push    dx
;
;       lds     bx, ppbox
;       mov     di, ss
;       mov     es, di
;       fld     ZValue
;       sub     sp, 4
;       mov     di, sp
;       fstp    dword ptr es:[di]
;       fwait
;       fld     [bx].y.restore_magnitude        ; push on FPP stack
;                                               ; to multiply with y min/max
;       fld     ST
;       fmul    [bx].y.max_value
;       sub     sp, 4
;       mov     di, sp
;       fstp    dword ptr es:[di]
;       fwait
;       fmul    [bx].x.max_value                ; [bx].y.restore_magnitude
;       sub     sp, 4
;       mov     di, sp
;       fstp    dword ptr es:[di]
;       fwait
;       push    ds
;       push    bx              ; plotbox
;;      fwait
;       call    gss_position
;       add     sp, 16
;
;       pop     si
;       pop     cx
;       lds     bx, ppbox
;
;       cmp     cx, [bx].plotarea.xp
;       jge     ccr_0
;       mov     cx, [bx].plotarea.xp
;ccr_0:
;       cmp     cx, [bx+4].plotarea.xp
;       jle     ccr_1
;       mov     cx, [bx+4].plotarea.xp
;ccr_1:
;       cmp     ax, [bx].plotarea.xp
;       jge     ccr_2
;       mov     ax, [bx].plotarea.xp
;ccr_2:
;       cmp     ax, [bx+4].plotarea.xp
;       jle     ccr_3
;       mov     ax, [bx+4].plotarea.xp
;ccr_3:
;       cmp     si, [bx].plotarea.yp
;       jge     ccr_4
;       mov     si, [bx].plotarea.yp
;ccr_4:
;       cmp     si, [bx+4].plotarea.yp
;       jle     ccr_5
;       mov     si, [bx+4].plotarea.yp
;ccr_5:
;       cmp     dx, [bx].plotarea.yp
;       jge     ccr_6
;       mov     dx, [bx].plotarea.yp
;ccr_6:
;       cmp     dx, [bx+4].plotarea.yp
;       jle     ccr_7
;       mov     dx, [bx+4].plotarea.yp
;ccr_7:
;       xor     di, di                  ; FALSE
;       cmp     ax, cx
;       jne     ccr_8
;       mov     di, TRUE
;ccr_8: cmp     dx, si
;       jne     ccr_9
;       mov     di, TRUE
;ccr_9:
;        push    ax                      ; 9/19/90 DAI
;       mov     ax, @data       ;SEG GlobalLineWidth   ; @data 9/19/90 DAI
;        mov     ds, ax
;        pop     ax
;
;;      mov     bx, GlobalLineWidth     ; line_width  9/19/90 DAI
;;      shl     bx, 1
;        mov     bx, ds:GlobalDoubleLineWidth
;
;       sub     ax, bx
;        sub    dx, bx
;
;       lds     bx, ClipRect
;       mov     [bx].xp, cx
;       mov     [bx].yp, si
;       mov     [bx+4].xp, ax
;       mov     [bx+4].yp, dx
;
;       mov     ax, di
;       ret
;
;_CalcClipRect  ENDP




        END


