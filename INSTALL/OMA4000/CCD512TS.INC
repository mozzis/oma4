;************************************************************************
;                                                                       *
;    This file (CCD512TS.INC) has all the equates dependent upon the    *
;    Thompson 512x512 CCD Split-mode detector.                          *
;                                                                       *
;************************************************************************

;
; $Log:   J:/logfiles/oma4000/detect/ccd512ts.inc  $
;   
;      Rev 1.1   16 Jul 1992 17:24:54   maynard
;   Clean up differences between other CCD... files
;

;------  Array dimensions -----------------------------------------------

;       _____________________________________
;      |_____________________________________|----> VIDEO OUT B
;           |                         | _2_
;           | <------ SPECTRA ------> |  |
;           |                         |  |
;           |                         |  |    I
;           |_ _ _ _ _ _ _ _ _ _ _ _ _| 512   M
;           |                         |  |    A
;           |                         |  |    G
;           |                         |  |    E
;           | <------ SPECTRA ------> | _|_
;       ____|_________________________|__2___
;      |_____________________________________|----> VIDEO OUT A
;
;      |-11-|---------- 512 ----------|--20--|
;                    (SERIAL)

NUMIMAGE         EQU    512
NUMSERIAL        EQU    512
IMAGE_LEAD  EQU    2
IMAGE_LAST  EQU    2
SERIAL_LEAD EQU    20
SERIAL_LAST EQU    11
TOTAL_SERIAL     EQU    NUMSERIAL + SERIAL_LEAD + SERIAL_LAST
TOTAL_IMAGE      EQU    NUMIMAGE + IMAGE_LEAD + IMAGE_LAST

IGROUPS        EQU 2                            ; 'Tracks', if simple mode
SGROUPS        EQU 512                          ; 'Points', if simple mode

IDELTA         EQU 1                            ;Delta, image direction.
SDELTA         EQU 1                            ;Delta, serial direction.

IMAGE0         EQU 256 + IMAGE_LEAD - 1     ; Offset to start of frame
SERIAL0        EQU SERIAL_LEAD       ; Offset to start of frame

IMAGE1         EQU TOTAL_IMAGE - IMAGE0 - (IDELTA * IGROUPS)
SERIAL1        EQU TOTAL_SERIAL - SERIAL0 - (SDELTA * SGROUPS)

;--- Detector Instructions

NO_OP    EQU     %000000000000000000000000      ;No Operation.          3
NP1F     EQU     %000000000000000100000000      ;Normal 1st half Fast.  5
NP1      EQU     %000000000000001000000000      ;Normal 1st half.       9
NP2F     EQU     %000000000000001100000000      ;Normal 2nd half Fast.  5
NP2      EQU     %000000000000010000000000      ;Normal 2nd half.       9
DPXS     EQU     %000000000000010100000000      ;Dump Pixel Slow.       5
TPS      EQU     %000000000000011000000000      ;Trash Pixel Slow.      5
DPX      EQU     %000000000000011100000000      ;Dump Pixel.            3
TP       EQU     %000000000000110000000000      ;Trash Pixel.           3
SLF      EQU     %000000000000110100000000      ;Shift Image Fast.     10
SLN      EQU     %000000000000110100000000      ;Shift Image Normal.   10
STSC     EQU     %000000000000111100000000      ;Start Source Comp.     8
SC1      EQU     %000000000001000000000000      ;Source Comp 1st half.  5
SC2      EQU     %000000000001000100000000      ;Source Comp 2nd half. 6.5
FLUSH    EQU     %000000000001110100000000      ;Flush pipe-line.       5
OPEN     EQU     %000000000000000000000001      ;Open shutter.
CLOSE    EQU     %000000000000000000000010      ;Close shutter.   (modifier
SET_A    EQU     %000000000000000000000100      ;Set Trigger A     bits)
RS_A     EQU     %000000000000000000001000      ;Reset Trigger A
SET_B    EQU     %000000000000000000010000      ;Set Trigger B
RS_B     EQU     %000000000000000000100000      ;Reset Trigger B
ENSYNC   EQU     %000000000000000001000000      ;Enable Sync input.
ARMTRG   EQU     %000000000000000010000000      ;Rearm Trigger input.
CUE      EQU     %100000000000000000000000      ;Cue the DMA Controller.

;------- General Equates ------------------------------------------------

DCADDR   EQU     %000000000000000000000001      ;DC Proc. loads address.
FCADDR   EQU     %000000000000000000000000      ;FC Proc. loads address.
LB       EQU     %000000000000000000000000      ;Set Pipeline for Loopback.
QNS      EQU     %000000000000000000000110      ;Set Pipeline for QNS data.
SPLTMOD  EQU     %000000000000000000000110      ;Set Pipeline for Splitmode.
XMIT13   EQU     %000000000000000001101000      ;Xmit 13 bits to detector.
XMIT14   EQU     %000000000000000001110000      ;Xmit 14 bits to detector.
REC18    EQU     %000000111111111100000000      ;Receive 18 data bits.
RECTRIG  EQU     %100000000000000000000000      ;Receive the trigger bit.
RECTEMP  EQU     %010000000000000000000000      ;Receive the Temp locked bit.
DB_PREC  EQU     %010000000000000000000000      ;Double precision.

;---- AUX Port Instruction definitions -------------------------

AN_PROC_BD      EQU    $2000                    ;Analog processing address.
ACHANNEL        EQU    %00000001                ;Selects A high, B low.
BCHANNEL        EQU    %00000000                ;Selects B preamp

AD_BD           EQU    $2100                    ;A/D Converter address.
US10            EQU    %00010000                ;10 uS time constant.
US100           EQU    %00010100                ;100 uS time constant.
MS1             EQU    %00010010                ;1 mS time constant.
MS10            EQU    %00010110                ;10 mS time constant.
MS100           EQU    %00010001                ;100 mS time constant.
S1              EQU    %00010101                ;1 Sec time constant.
NOINTEGRATION   EQU    %00001000                ;No integration used.

TE_SERVO_BD     EQU    $2200                    ;Cooler board address.
COOLER_ON       EQU    %00000001
COOLER_OFF      EQU    %00000000

CLKBIAS_BD      EQU    $2400                    ;Clock Bias address.
A_CHANNEL       EQU    %00000000                ;Shift image toward A reg.
B_CHANNEL       EQU    %00000010                ;Shift image toward B reg.
BOTH_CHANNELS   EQU    %00000100                ;Shift image toward A&B regs.
RS_XYZ          EQU    %00010000                ;RESET XY MONITOR

SHUTTER_BD      EQU    $2600                    ;Shutter Controller address.
EXT_SHUTTER     EQU    $00                      ;Use external shutter BNC.
INT_SHUTTER     EQU    $01                      ;Use internal shutter.
OPTICAL         EQU    $02                      ;Optical trigger source.
EXT_POS         EQU    $04                      ;External positive edge.
EXT_NEG         EQU    $08                      ;External negitive edge.
PULSED          EQU    $10                      ;Pulsed mode enabled.
USE_SYNC        EQU    $20                      ;Trigger OMA w/Sync.
USE_TRIG        EQU    $00                      ;Trigger OMA w/Trig.
BEEP_ON         EQU    $40                      ;Enable beeper.
ARM_PTRIG       EQU    $80                      ;Arm input triggers.

TIMER_VECTOR    EQU    $2700                    ;Timer Vector register.
TRIG_LEVEL      EQU    $00                      ;Trigger Level register.
ANALOG_DELAY    EQU    $01                      ;Analog delay register.
DELAY_LSB       EQU    $02                      ;Digital delay LSB register.
DELAY_MSB       EQU    $03                      ;Digital delay MSB register.
DELTA_LSB       EQU    $04                      ;Delta delay LSB register.
DELTA_MSB       EQU    $05                      ;Delta delay MSB register.
ANALOG_DELTA    EQU    $06                      ;Analog delta delay register
ANALOG_WIDTH    EQU    $07                      ;Analog width register.
TRIG_CNT_LSB    EQU    $08                      ;Number of trigs LSB reg.
TRIG_CNT_MSB    EQU    $09                      ;Number of trigs MSB reg.
WIDTH_LSB       EQU    $0A                      ;Width LSB register.
WIDTH_MSB       EQU    $0B                      ;Width MSB register.

TIMER_VALUE     EQU    $2800                    ;Timer Value register.

AUX_9           EQU    $2900                    ;Not defined.
TSTREG0         EQU    $2A00                    ;Test Register zero.
TSTREG1         EQU    $2B00                    ;Test Register one. 
TSTREG2         EQU    $2C00                    ;Test Register two. 

PIA_IN          EQU    $2D00                    ;PIA Input address.
PIA_OUT         EQU    $2E00                    ;PIA Output address.
DET_CNTL_BD     EQU    $2F00                    ;Detector Controller address.

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
