  page ,80

  TITLE MYBLT.ASM

; 
; Module to copy data from OMA controller board into bitmap
;

  .MODEL MEDIUM, C

  PUBLIC CopyBlock

  FAST_TEXT segment PUBLIC PARA USE16 'CODE'

  .CODE FAST_TEXT

  assume cs : FAST_TEXT
  assume ds : nothing
  assume es : nothing

;
; Copy a block
;
; register useage:   eax - data value
;                    ebx - offset (brightness) value
;                    ecx - pixel in line counter
;                    edx - line in image counter
;                    esi - byte offset into source
;                    edi - byte offset into dest
;                    ds  - segment of source
;                    es  - segment of dest

  .386P
  .387
  ALIGN 4
     
CopyBlock PROC FAR C USES ds es esi edi,
      pDest   : DWORD, ;pointer to destination bitmap
      pSource : DWORD, ;pointer to source data
      dwWidth : DWORD, ;Width of data in pixels
      dwHeight: DWORD, ;Height of data in lines
      dwScanD : DWORD, ;Size of Dest scan line in bytes
      dwScanS : DWORD, ;Size of Src scan line in bytes
      dwOffset: DWORD, ;Brightness adjustment
      dwScale : DWORD, ;Contrast adjustment

      LOCAL dwOffD : DWORD
      LOCAL fTmp   : DWORD
     
  mov edx, dwHeight       ; EDX is line counter

  xor esi, esi
  lds si, pSource         ; DS:[ESI] point to source

  xor edi, edi
  les di, pDest           ; ES:[EDI] point to dest
  mov dwOffD, edi         ; save offset
  mov ebx, dwOffset       ; save Brightness value

  mov ecx, dwWidth
  jecxz cp_nomore         ; test for silly case

cp_morelines:             ; begin copy operation
  and edx, edx            ; any more lines?
  jz cp_nomore            ; no, bail out
  mov ecx, dwWidth        ; ECX is pixel counter

cp_morepixels:
  mov   eax, ds:[esi]     ; get source byte
  sub   eax, ebx          ; subtract brightness
  mov   fTmp,eax
  fild  fTmp
  fdiv  dwScale           ; divide by scale
  fistp fTmp              ; convert back to long
  mov eax,fTmp

  cmp  ax,00ech           ; compare to max color val
  jge  cp_settomax        ; if too big clip to limit
  cmp  ax,0009h           ; compare to min color val
  jge  cp_savepixel       ; if OK, goto store pixel

  mov ax,0009h            ; clip low value to min
  jmp short cp_savepixel

cp_settomax:
  mov ax,00ech            ; clip high value to max

cp_savepixel:
  mov es:[edi], al        ; copy value to to dest bmp

cp_nextpixel:
  add esi,4               ; inc src offset
  inc edi                 ; inc dest offset
  loopnz cp_morepixels    ; continue for all pixels in line

  ; move on to the start of the next line

  mov edi, dwOffD         ; offset dest to next scan line
  add edi, dwScanD
  mov dwOffD, edi         ; save new dest offset

  dec edx                 ; dec line counter
  jmp cp_morelines

cp_nomore:
  ret

CopyBlock endp


ScaleBlock PROC FAR C USES es esi edi,
      pData   : DWORD, ;pointer to data
      dwStartX: DWORD, ;Start pixel in each line
      dwStartY: DWORD, ;Start line in data
      dwWidth : DWORD, ;Width of data block to scale
      dwHeight: DWORD, ;Height of data in lines
      pMinVal : DWORD, ;Pointer to Min Val result
      dMaxVal : DWORD, ;Pointer to Max Val result

      LOCAL dwOffD : DWORD
      LOCAL fTmp   : DWORD
     

  xor edi, edi
  les di, pData           ; ES:[EDI] point to data
  mov dwOffD, edi         ; save offset
  mov ebx, dwOffset       ; save Brightness value

  mov ecx, dwWidth        ; ECX is pixel counter
  mov edx, dwHeight       ; EDX is line counter
  jecxz sb_nomore         ; test for silly case

sb_morelines:             ; begin copy operation
  and edx, edx            ; any more lines?
  jz sb_nomore            ; no, bail out
  mov ecx, dwWidth        ; ECX is pixel counter

sb_morepixels:
  mov   eax, ds:[esi]     ; get source byte
  sub   eax, ebx          ; subtract brightness
  mov   fTmp,eax
  fild  fTmp
  fdiv  dwScale           ; divide by scale
  fistp fTmp              ; convert back to long
  mov eax,fTmp

  cmp  ax,00ech           ; compare to max color val
  jge  sb_settomax        ; if too big clip to limit
  cmp  ax,0009h           ; compare to min color val
  jge  sb_savepixel       ; if OK, goto store pixel

sb_settomin:
  mov ax,0009h
  jmp short sb_savepixel

sb_settomax:
  mov ax,00ech

sb_savepixel:
  mov es:[edi], al        ; copy to dest bmp

sb_nextpixel:
  add esi,4               ; inc src offset
  inc edi                 ; inc dest offset
  loopnz sb_morepixels    ; continue for all pixels in line

  ; move on to the start of the next line

  mov edi, dwOffD         ; offset dest to next scan line
  add edi, dwScanD
  mov dwOffD, edi         ; save new dest offset

  dec edx                 ; dec line counter
  jmp sb_morelines

sb_nomore:
  ret

ScaleBlock endp

FAST_TEXT ends
end

