;*************************************************************************** 
;*                                                                         * 
;*   cache386.asm - control external caches on 386 machines                * 
;*           (Copyright(C) 1991 EG&G Princeton Applied Research)           * 
;*                                                                         *  
;*  $Header:  $
;*  $Log:     $
;*                                                                         * 
;*************************************************************************** 
.MODEL LARGE, C
.386                    ; Enable 80286 instructions
        .CODE

CacheOff386 PROC FAR USES eax ecx,              \
                      cache_selector : FAR PTR, \
                      cache_off      : DWORD
       mov     eax,[cache_selector]
       mov     ecx,[cache_off]
       not     ecx
       and     eax,ecx
       mov     [cache_selector],eax
       xor     ax,ax
       ret
CacheOff386 ENDP

CacheOn386 PROC FAR USES eax ecx,               \
                      cache_selector : FAR PTR, \
                      cache_on       : DWORD
       mov     eax,[cache_selector]
       mov     ecx,[cache_on]
       or      eax,ecx
       mov     [cache_selector],eax
       xor     ax,ax
       ret
CacheOn386 ENDP

END
