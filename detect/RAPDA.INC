;************************************************************************ 
;                                                                       *
;    This file (RAPDA.INC) has all the equates dependent upon the       *
;    Reticon 1024 PDA detector and the OMA Controller board.            * 
;    See also the general equate file OMACP.INC                                                                   * 
;************************************************************************ 

NUMIMAGE         EQU    1
NUMSERIAL        EQU    1024
DEAD_IMAGE_LEAD  EQU    0
DEAD_IMAGE_LAST  EQU    0
DEAD_SERIAL_LEAD EQU    0
DEAD_SERIAL_LAST EQU    0
TOTAL_SERIAL     EQU    NUMSERIAL + DEAD_SERIAL_LEAD + DEAD_SERIAL_LAST
TOTAL_IMAGE      EQU    NUMIMAGE + DEAD_IMAGE_LEAD + DEAD_IMAGE_LAST
IGROUPS          EQU 1
SGROUPS          EQU 1023

DELAY_PAD        EQU 400                      ;To allow DAC to do its looping

;--- Detector Instructions for the Rapda Detector Controller. --
;-----for the LOAD instr.
;                 22221111111111
;                 321098765432109876543210
;                %.........0XXXXXX........ <- Instruction description.   
NO_OP    EQU     %000000000000000000000000    ;No Operation.
ANWAG    EQU     %000000000000000100000000    ;Acquire next w/auto gain.
ANWFG    EQU     %000000000000001000000000    ;Acquire next w/fixed gain.
ANSWAG   EQU     %000000000000001100000000    ;Acq'r next slow w/auto gain.
ANSWFG   EQU     %000000000000010000000000    ;Acq'r next slow w/fixed gain.
APWAG    EQU     %000000000000010100000000    ;Acq'r previous w/auto gain.
APWFG    EQU     %000000000000011000000000    ;Acq'r previous w/fixed gain.
APSWAG   EQU     %000000000000011100000000    ;Acq'r prev slow w/auto gain.
APSWFG   EQU     %000000000000100000000000    ;Acq'r prev slow w/fixed gain.
ST_GP    EQU     %000000000000100100000000    ;Start Grouping.
END_GP   EQU     %000000000000101000000000    ;Finish Grouping.
CLR_FIFO EQU     %000000000000101100000000    ;Clear FIFO.
SSUAG    EQU     %000000000000110000000000    ;Slow support up auto gain.
SSMAG    EQU     %000000000000110100000000    ;Slow support middle auto gain.
SSDAG    EQU     %000000000000111000000000    ;Slow support down auto gain.
SSEAG    EQU     %000000000000111100000000    ;Slow support end auto gain.
SSUFG    EQU     %000000000001000000000000    ;Slow support up fixed gain.
SSMFG    EQU     %000000000001000100000000    ;Slow support middle fixed gain.
SSDFG    EQU     %000000000001001000000000    ;Slow support down fixed gain.
SSEFG    EQU     %000000000001001100000000    ;Slow support end fixed gain.
SFGLO    EQU     %000000000001010000000000    ;Set fixed gain low.           
SFGMID   EQU     %000000000001010100000000    ;Set fixed gain middle.     
SFGHI    EQU     %000000000001011000000000    ;Set fixed gain high.          
STSC     EQU     %000000000001011100000000    ;Start Source Compensation.
RDSC     EQU     %000000000001100000000000    ;Read Source Compensation.
WFSI     EQU     %000000000001100100000000    ;Wait For Sync Input.
LDNS     EQU     %000000000001101000000000    ;LOAD & NEXT STROBE       
SNOP     EQU     %000000000001101100000000    ;Stretched No operation - 4æsec
RAGS     EQU     %000000000001110000000000    ;Random Access group start
GRPS     EQU     %000000000001110100000000    ;Group Support            
        ;        %..........1XXX..........    ;Random access bits.
        ;        %..............XXXXXXXXXX    ;Pixel addrs, 1K max.
RAAG     EQU     %000000000010000000000000    ;Random Access Auto Gain.
RAFG     EQU     %000000000010010000000000    ;Random Access Fixed Gain.
RASAG    EQU     %000000000010100000000000    ;Random Access Slow Auto Gain.
RASFG    EQU     %000000000010110000000000    ;Random Access Slow Fixed Gain.
        ;        %000000000010110000000000    ;RASFG Pixel 0, e.g.
        ;        %000000000010111111111111    ;RASFG Pixel 1023
TST1     EQU     %000000000011111000000000    ;Test Instruction One.
TST2     EQU     %000000000011111100000000    ;Test Instruction Two.
        ;         00000000000.....XXXXXXXX <- ;Trigger Control.
OPEN     EQU     %000000000000000000000001    ;Internal LCI open.
CLOSE    EQU     %000000000000000000000010    ;Internal LCI close.
SET_A    EQU     %000000000000000000000100    ;Set Trigger Out A.
RS_A     EQU     %000000000000000000001000    ;Reset Trigger Out A.
SET_B    EQU     %000000000000000000010000    ;Set Trigger Out B.
RS_B     EQU     %000000000000000000100000    ;Reset Trigger Out B.  
EN_SI    EQU     %000000000000000001000000    ;Enable Sync Input.
ARMTRG   EQU     %000000000000000010000000    ;Rearm Trigger Input
CUE      EQU     %100000000000000000000000    ;Cue the DMA Controller.


;------- General Equates for LDCFG ------------------------------
DCADDR   EQU     %000000000000000000000001      ;DC Proc. loads address.
FCADDR   EQU     %000000000000000000000000      ;FC Proc. loads address.
LB       EQU     %000000000000000000000000      ;Set Pipeline for Loopback.
QNS      EQU     %000000000000000000000110      ;Set Pipeline for QNS data.
RAPDA    EQU     %000000000000000000000100      ;Set Pipeline for RAPDA data.
XMIT13   EQU     %000000000000000001101000      ;Xmit 13 bits to detector.
XMIT15   EQU     %000000000000000001111000      ;Xmit 15 bits to detector.
REC16    EQU     %000000001111111100000000      ;Receive 16 data bits.
REC18    EQU     %000000111111111100000000      ;Receive 18 data bits.
RECTRIG  EQU     %100000000000000000000000      ;Receive the trigger bit.
RECTEMP  EQU     %010000000000000000000000      ;Receive the Temp locked bit.

;---- LDPCNT instr equate. -------------------------------------
DB_PREC  EQU     %010000000000000000000000      ;Double precision.
         
;---- AUX Port Instruction definitions for LOAD FC instr. ----
;  AUX Bus msg.: %0000000001..AAAADDDDDDDD,  A = Address Out, D = Data Out.
AN_PROC_BD      EQU    $4000                    ;Analog processing address.
ACHANNEL        EQU    %00000001                ;Selects A high, B low.
BCHANNEL        EQU    %00000000                ;Selects B preamp

AD_BD           EQU    $4100                    ;A/D Converter address.
CALRUN          EQU    %00000101                ;A/D in run position.
US10            EQU    %00010000                ;10 uS time constant.
US100           EQU    %00010100                ;100 uS time constant.
MS1             EQU    %00010010                ;1 mS time constant. 
MS10            EQU    %00010110                ;10 mS time constant.
MS100           EQU    %00010001                ;100 mS time constant.
S1              EQU    %00010101                ;1 Sec time constant.
NOINTEGRATION   EQU    %00001000                ;No integration used.

TE_SERVO_BD     EQU    $4200                    ;Cooler board address.
COOLER_ON       EQU    %00000001               
COOLER_OFF      EQU    %00000000

CLKBIAS_BD      EQU    $4400                    ;Clock Bias address.    
A_CHANNEL       EQU    %00000000                ;Shift image toward A reg.
B_CHANNEL       EQU    %00000010                ;Shift image toward B reg.
RS_XYZ          EQU    %00010000                ;RESET XY MONITOR

SHUTTER_BD      EQU    $4600                    ;Shutter Controller address.
INT_SHUTTER     EQU    %00000001                ;Use internal shutter.
EXT_SHUTTER     EQU    %00000000                ;Use external shutter BNC.

PIA_IN          EQU    $4D00                    ;PIA Input address.
PIA_OUT         EQU    $4E00                    ;PIA Output address.
DET_CNTL_BD     EQU    $4F00                    ;Detector Controller address.
NO_LOOPBACK     EQU    %00000100                ;Normal mode, no loopback.

;----- TE Servo temperature definitions ------------------------

TEMPSET MACRO
        WAIT
        LOAD TE_SERVO_BD | ((10-(?1))&$FE) | RPP_RTP | CUE ! TEMP_TAG
        ENDM

TEMPREAD MACRO
        WAIT
        LOAD REPLACE | LD_ADDR                       ;Store TE status
        ENDM

DEG_10_PLUS    EQU  %00000000  ; 0
DEG_5_PLUS     EQU  %00000100  ; 4
DEG_0          EQU  %00001010  ; 10
DEG_5_MINUS    EQU  %00001110  ; 14
DEG_10_MINUS   EQU  %00010100  ; 20
DEG_20_MINUS   EQU  %00011110  ; 30
DEG_30_MINUS   EQU  %00101000  ; 40
DEG_40_MINUS   EQU  %00110010  ; 50
DEG_50_MINUS   EQU  %00111100  ; 60
DEG_55_MINUS   EQU  %01000000  ; 64
DEG_60_MINUS   EQU  %01000110  ; 70
DEG_70_MINUS   EQU  %01010000 
DEG_80_MINUS   EQU  %01011010 
DEG_90_MINUS   EQU  %01100100 
DEG_100_MINUS  EQU  %01101110 
DEG_110_MINUS  EQU  %01111000 
DEG_120_MINUS  EQU  %10000010 
DEG_130_MINUS  EQU  %10001100 
DEG_140_MINUS  EQU  %10010110 
      
