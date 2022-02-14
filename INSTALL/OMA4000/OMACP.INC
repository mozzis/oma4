; $Log:   J:/logfiles/oma4000/detect/omacp.inc  $
;   
;      Rev 1.1   16 Jul 1992 17:27:20   maynard
;   Cosmetic changes
;   
;      Rev 1.1   07 Jul 1992 16:58:40   maynard
;   For version 2.41 ff.
;   
;      Rev 1.0   06 Jul 1992 12:10:12   maynard
;   Initial revision.

;************************************************************************ 
;                                                                       *
;    This file (OMACP.INC) has all the equates dependent upon the       *
;    OMA Control Processor (ASIC).                                      * 
;                                                                       * 
;************************************************************************ 

;------- Address Calculation Instructions -------------------------------

ADDA     EQU     %000000000000000000000000      ;Use A Pointers.
RPP      EQU     %000010000000000000000000      ;Reset Pixel Pointer A.
IPP      EQU     %000100000000000000000000      ;Increment Pixel Pointer A.
RTP      EQU     %000110000000000000000000      ;Reset Track Pointer A.
ITP      EQU     %001000000000000000000000      ;Increment Track Pointer A.
RPP_RTP  EQU     %001010000000000000000000      ;Reset A Pointers.
RPP_ITP  EQU     %001100000000000000000000      ;RS PPTRA & INC TPTRA.
IPP_RTP  EQU     %001110000000000000000000      ;INC PPTRA & RS TPTRA.
ADDB     EQU     %010000000000000000000000      ;Use B Pointers.
LPP      EQU     %010010000000000000000000      ;Load Pixel Pointer B.
DPP      EQU     %010100000000000000000000      ;Decrement Pixel Pointer B.
LTP      EQU     %010110000000000000000000      ;Load Track Pointer B.
DTP      EQU     %011000000000000000000000      ;Decrement Track Pointer B.
LPP_LTP  EQU     %011110000000000000000000      ;Load B Pointers.
LPP_DTP  EQU     %011100000000000000000000      ;LD PPTRB & DEC TPTRB.
DPP_LTP  EQU     %011010000000000000000000      ;DEC PPTRB & LD TPTRB.


;------- DMA Instructions -----------------------------------------------

NOTHING  EQU     %00000000                      ;Do nothing.
REPLACE  EQU     %00000111                      ;+A/D = Memory
AD_M_MEM EQU     %00000010                      ;+A/D - Memory = Memory
AD_P_MEM EQU     %00000110                      ;+A/D + Memory = Memory            
MEM_M_AD EQU     %00000100                      ;Memory - A/D = Memory
AD_M_BG  EQU     %00000011                      ;+A/D - Bkgnd = Memory
AD_P_BG  EQU     %00000001                      ;+A/D + Bkgnd = Memory
BG_M_AD  EQU     %00000101                      ;Bkgnd - A/D = Memory
LD_BGB   EQU     %00001000                      ;Load background base.
LD_FB    EQU     %00010000                      ;Load frame base.
LD_ADDR  EQU     %00011000                      ;Load both bases.

;------- ASIC register assignments. -------------------------------------

FCPROG   EQU     $30001                   ;FC Program register, 2 bytes.
FCMASK   EQU     $30005                   ;FC Output Mask register, 2 bytes.
DCPROG   EQU     $30009                   ;DC Program register, 2 bytes.
BGBASE   EQU     $3000D                   ;Background Base register, 3 bytes.
FBASE    EQU     $30013                   ;Frame Base Register, 3 bytes.


GOTOB    MACRO
         LDCNTRB $0
         JUNTILB ?1
         ENDM

