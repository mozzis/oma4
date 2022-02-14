; Defines the 'tags' used by the driver to find the detector parameters.
; Corresponds to #defines in counters.h. Must be identical!

I_TOTAL        EQU 1   ;Image area total.
S_TOTAL        EQU 2   ;Serial area total.
OH_OUT         EQU 3   ;Over Head outer loop.
OH_IN          EQU 4   ;Over Head inter loop.
I_LEAD_INNER   EQU 5   ;Image index, starting.
I_LEAD_OUTER   EQU 6   ;Image index, ending.
I_TRAIL_INNER  EQU 7   ;Serial index, starting.
I_TRAIL_OUTER  EQU 8   ;Serial index, ending.
I_GROUP        EQU 9   ;Image length.           
S_GROUP_MIN1   EQU 10  ;Serial length.          
Y_DELTA        EQU 11  ;Image point size.
X_DELTA        EQU 12  ;Serial point size.
Y_SKIP         EQU 13  ;Image skip by value.
X_SKIP         EQU 14  ;Serial skip by value.
S_IGNORE_START EQU 15  ;Serial ignore, starting.
S_IGNORE_END   EQU 16  ;Serial ignore, stopping.
TRACKLEN       EQU 17  ;Number of words per track
I0_COUNTER     EQU 18  ;Number of frames/mem, by convention 
I1_COUNTER     EQU 19
J0_COUNTER     EQU 20  ;Number of memories, by convention 
J1_COUNTER     EQU 21  
K0_COUNTER     EQU 22  ;Number of ignored frames, by convention
K1_COUNTER     EQU 23  
H0_COUNTER     EQU 24  ;Number of prep frames, by convention 
H1_COUNTER     EQU 25  
L0_COUNTER     EQU 26  ;Auxiliary counter; may be array of counters 
L1_COUNTER     EQU 27  
T0_COUNTER     EQU 28  ;Number of tracks (spectra) for DAC 
T1_COUNTER     EQU 29  
S0_COUNTER     EQU 30  ;Auxiliary slices (points) for DAC 
S1_COUNTER     EQU 31  

TEMP_TAG       EQU 32  ;Temperature setting 
COOL_STATUS    EQU 33  ;Cooler status byte
DET_ID         EQU 34
SHUTTER_BEF    EQU 35  ;Shutter state before expose 
SHUTTER_AFT    EQU 36  ;Shutter state after expose 
FRAME_PTR      EQU 37  ;Address for data frame
BKGND_PTR      EQU 38  ;Address for background frame
SIZE_PTR       EQU 39  ;Frame size
SRCCMP_PTR     EQU 40  ;Source comp location
SCITC_SET      EQU 41  ;source comp integration time constant
SYNC_OPEN      EQU 42  ;wait for sync to open shutter
SYNC_CLOSE     EQU 43  ;wait for sync to close shutter
UT_SIMPLE      EQU 44  ;FC Uniform tracks - simple
UT_CLASSIC     EQU 45  ;FC Uniform tracks - classic
RT_ALL         EQU 46  ;FC Random tracks
SER_ACQ        EQU 47  ;Where to patch in address of serial read routine
SER_CLEAN      EQU 48  ;FC Clean shift register
FC_MODE_SC     EQU 49  ;FC Get Source Comp

RSLICE_OUT     EQU 50  ;Tag for where to put random slice code
USSLICE_OUT_MX EQU 51  ;Tag for where simple  uniform slice code is
USSLICE_OUT_DX EQU 52  ;Tag for where simple  uniform slice code is
UCSLICE_OUT_MX EQU 53  ;Tag for where classic uniform slice code is
UCSLICE_OUT_DX EQU 54  ;Tag for where classic uniform slice code is
      
FC_OH          EQU 55  ;Address of FC code for normal shutter/expose
FC_OHNS        EQU 56  ;Address of FC code for shutter forced/expose
FC_OHNOET      EQU 57  ;Address of FC code for no expose
FC_OHLCI       EQU 58  ;Address of FC code for Pulser board
FC_OHLCINP     EQU 59  ;Address of FC code for Pulser board with no pulse
DC_OH          EQU 60  ;Address of DC code for overhead with shutter
DC_OHNE        EQU 61  ;Address of DC code for overhead without EXPOSE 
DC_OHLCI       EQU 62  ;Address of DC code for overhead with Pulser board
      
DC_MODE_ADD    EQU 63  ;DC Mode for adds
DC_MODE_REP    EQU 64  ;DC Mode for replace
DC_MODE_SUB    EQU 65  ;DC Mode for subs
DC_MODE_IGN    EQU 66  ;DC Mode for replace
DC_MODE_SC     EQU 67  ;DC Get Source Comp
        
DAS_INIT       EQU 68  ;DAC Mode for ASIC sync
DAS_CLEAN      EQU 69  ;DAC Mode for KEEPCLEAN
DAS_0          EQU 70  ;DAC Mode for LIVE     
DAS_1          EQU 71  ;DAC Mode for DA Seq #1
DAS_2          EQU 72  ;DAC Mode for DA Seq #2
DAS_USER       EQU 73  ;USER DAC Modes (~ DAD)
      
SER_MODE       EQU 74  ;Monitor flag  uniform/random
IMG_MODE       EQU 75  ;Monitor flag, uniform/random

DA_DONE        EQU 76  ;Code to locate da done status in MSM

BIAS_AB        EQU 80
APROC_AB       EQU 81
PIXTIME_NP     EQU 82
INEX_SHUTTER   EQU 83  ;Internal or external drivesync for shutter

CONFIG         EQU 100 ;get Address of FC LDCFG command
TCNT           EQU 101 ;get Address of FC LDTCNT command
PCNT           EQU 102 ;get Address of FC LDPCNT command
SCNT           EQU 103 ;get Address of FC LDSCNT command

SM_SGROUPS     EQU 104 ;Number of points for split mode serial read 
SEC_CUE        EQU 105 ;Second cue of data point for spmode sc emulation

SORTQ          EQU 106 ;DAC Flag is true if data needs sorting

DUMXLD         EQU 107 ;Dead pixels in sensor serial register before data
DUMXTR         EQU 108 ;Dead pixels in sensor serial register after data
DUMYLD         EQU 109 ;Dead pixels in sensor image area before data
DUMYTR         EQU 110 ;Dead pixels in sensor image area after data
SACTIVE        EQU 111 ;Total live pixels in serial register
IACTIVE        EQU 112 ;Total live pixels in image area

PIXTIME_AS     EQU 113 ;Tag for EEV Acquire Signal instruction
SHUTID         EQU 114 ;get shutter board type code
SAME_ET        EQU 115 ;0 = do fast keepclean, 1 = normal
PIA_OUTBITS    EQU 116 ;Location of PIA output bits
PIA_INBITS     EQU 117 ;Location of PIA output bits
CLOSE_TIME     EQU 118 ;Counter to wait for shutter closed
DA_SEQ         EQU 119 ;Address DAC DA prog num variable

STREAK_MODE    EQU 120 ;Flag whether to do streaker type prescan
STREAK_ET      EQU 121 ;Special ET counter for streaker mode

STRK            EQU 122 ;FC mode for normal streak
STRK_OT         EQU 123 ;FC mode for streak on trigger
STRK_ET         EQU 124 ;FC mode for streak with ET
STRK_OT_ET      EQU 125 ;FC mode for streak on trigger with ET
STRK_SYNC       EQU 126 ;FC mode for streak with sync
STRK_OT_SYNC    EQU 127 ;FC mode for streak on trigger with sync
STRK_ET_SYNC    EQU 128 ;FC mode for streak with ET and sync
STRK_OT_ET_SYNC EQU 129 ;FC mode for streak on trigger with ET and sync
STRK_PT         EQU 130 ;FC mode for streak with pre-trigger
STRK_LCI        EQU 181 ;FC mode for streak LCI pulser
STRK_LCI_CW     EQU 182 ;FC mode for streak LCI pulser in CW


PCH_USER_0      EQU 200
PCH_USER_1      EQU 201
PCH_USER_2      EQU 202

; LCI timer board
            
LCI_TRIG_LEVEL EQU 131
LCI_DELAY_ANA  EQU 132
LCI_DELAY_LSB  EQU 133
LCI_DELAY_MSB  EQU 134
LCI_DELTA_ANA  EQU 135
LCI_DELTA_LSB  EQU 136
LCI_DELTA_MSB  EQU 137
LCI_WIDTH_ANA  EQU 138
LCI_WIDTH_LSB  EQU 139
LCI_WIDTH_MSB  EQU 140
LCI_CNT_LSB    EQU 141
LCI_CNT_MSB    EQU 142
LCI_CMD        EQU 143
LCI_CONFIG     EQU 144

I_LEAD_REM     EQU 145 ;"Remainder" part of leading tracks
I_TRAIL_REM    EQU 146 ;"Remainder" part of trailing tracks

LEAD_PIXELS    EQU 147
SHIFT_SPEED    EQU 148 ;Select Fast, Normal, or Slow shift line
PT_TRACKS      EQU 149 ;Tracks for streak "pre-trigger" mode
STRK_TRACKS    EQU 180 ;Tracks for streak - 1/2 normal if dual mode

;*************************** RAPDA Tags ******************************

DELAY_READ_0   EQU 150            ; The first delay read program
OUTER_COUNT    EQU 151            ; the outer counter delay
INNER_COUNT    EQU 152            ; the inner counter delay
READ_OFFSET    EQU 153            ; the read part of delay read.
DELAY_RD_COUNT EQU 154            ; Number pixels to read,
DELAY_RD_START EQU 155            ; starting at this one.
DELAY_READ_1   EQU 156            ; The second delay read program

READ_0         EQU 157            ; The first read program
READ_COUNT     EQU 158            ; Number pixels to read,
READ_START     EQU 159            ; starting at this one.
READ_1         EQU 160            ; The second read program

DC_PGM_1       EQU 161            ; The first DC program
DMA_COUNT      EQU 162            ; Number pixels to read.
DC_PGM_2       EQU 163            ; The second DC program
DC_ACTION      EQU 164            ; What the DC program does.

DAS_RAPDA_TBL1 EQU 165            ; The delay_read program table
DAS_RAPDA_TBL2 EQU 166            ; the read program table.
MEMCNT         EQU 177            ; number of memories for clear mem. */

; Corresponds to SHARED_COMM_AREA struct in monitor.h - MUST BE IDENTICAL!

MSM_Act_RQ        EQU  0  ;Monitor action requested by driver
MSM_Act_RQ_RPL    EQU  2  ;Monitor reply to driver request
MSM_Sig_PC        EQU  4  ;Driver action requested by monitor
MSM_Sig_PC_RPL    EQU  6  ;Driver reply to monitor request
MSM_Pgm_Inx       EQU  8  ;Which DAC program to run
MSM_Data_Offset   EQU 10  ;Offset of frame data (assumes from 800000)
MSM_Bgnd_Offset   EQU 14  ;Offset of bkgnd data (assumes from 800000)
MSM_Data_Size     EQU 18  ;Size of memory allocated for experiment
MSM_FC_OH         EQU 22  ;Which overhead routine to use
Counter_H0        EQU 24  ;Number of prep frames, by convention
Counter_H1        EQU 26  ;Run time value of H
Counter_I0        EQU 28  ;Number of frames/mem, by convention
Counter_I1        EQU 30  ;Run time value of I
Counter_J0        EQU 32  ;Number of memories, by convention
Counter_J1        EQU 34  ;Run time value of J
Counter_K0        EQU 36  ;Number of ignored frames, by convention
Counter_K1        EQU 38  ;Run time value of K
Counter_L0        EQU 40  ;Aux. counter; may be array of counters
Counter_L1        EQU 42  ;Run time value of L
Counter_T0        EQU 44  ;Number of tracks (spectra) for DAC
Counter_T1        EQU 46  ;Run time value of T
Counter_S0        EQU 48  ;Number of slices (points) for DAC
Counter_S1        EQU 50  ;Run time value of S
MSM_FC_Scan_Mode  EQU 52  ;Which ASIC FC program to run
MSM_FC_StreakMode EQU 54  ;ASIC program to run for streak prescan
MSM_FC_StrkOTMode EQU 56  ;ASIC program to run for streak prescan
MSM_DC_Add_Mode   EQU 58  ;ASIC DC program to run to add data
MSM_DC_Rep_Mode   EQU 60  ;ASIC DC program to run to replace data
MSM_DC_Sub_Mode   EQU 62  ;ASIC DC program to run to subtract data
MSM_DC_Ign_Mode   EQU 64  ;ASIC DC program to run to ignore data
MSM_DC_OH         EQU 66  ;ASIC DC program to run during overhead
MSM_DC_StreakMode EQU 68  ;ASIC program to run for streak prescan
MSM_DA_Complete   EQU 70  ;Flag if DA complete
MSM_Serial_Mode   EQU 72  ;Flag 0 = uniform, 1 = random, others?
MSM_Image_Mode    EQU 74  ;Flag 0 = uniform, 1 = random, others?
MSM_Keepclean     EQU 76  ;Flag 0 = don't sort, 1 = do sort
MSM_Pointnum      EQU 78  ;Number of points to sort in each track
MSM_SC_Address    EQU 80  ;Address of source comp (may be array) 4 bytes
MSM_StreakMode    EQU 84  ;Flag whether to do streaker prescan
MSM_PIA_Out       EQU 86  ;Value to write to PIA
;***** All the rest are written by ASIC so must be on 4-byte boundary *****;
MSM_PIA_In        EQU 88  ;Input bits from PIA
MSM_Cooler_Status EQU 92  ;Status byte from cooler at upper address
MSM_Shutter_ID    EQU 96  ;Shutter or timer board ID
MSM_Detector_ID   EQU 100 ;Detector Type ID

STARTADDR EQU   $800000  ;Start of OMA4 DRAM (Shared Comm Area)
DATAADDR EQU $0000716
BGNDADDR EQU $000071A

PGMINDX     EQU STARTADDR + MSM_Pgm_Inx
H0ADDR      EQU STARTADDR + Counter_H0        ;'P', or preps
H1ADDR      EQU STARTADDR + Counter_H1        ;'P', or preps
I0ADDR      EQU STARTADDR + Counter_I0        ;'I', or frames
I1ADDR      EQU STARTADDR + Counter_I1        ;'I', or frames
J0ADDR      EQU STARTADDR + Counter_J0        ;'J', or mems
J1ADDR      EQU STARTADDR + Counter_J1        ;'J', or mems
K0ADDR      EQU STARTADDR + Counter_K0        ;'K', or ignores
K1ADDR      EQU STARTADDR + Counter_K1        ;'K', or ignores
L0ADDR      EQU STARTADDR + Counter_L0        ;'L', or util ctr
L1ADDR      EQU STARTADDR + Counter_L1        ;'L', or util ctr
T0ADDR      EQU STARTADDR + Counter_T0        ;'T', or tracks
T1ADDR      EQU STARTADDR + Counter_T1        ;'T', or tracks
S0ADDR      EQU STARTADDR + Counter_S0        ;'S', or slices
S1ADDR      EQU STARTADDR + Counter_S1        ;'S', or slices
FRAMEOFFS   EQU STARTADDR + MSM_Data_Offset   ;Where driver stores data addr
FRAMESIZE   EQU STARTADDR + MSM_Data_Size     ;Where driver stores data size
BKGNDOFFS   EQU STARTADDR + MSM_Bgnd_Offset   ;Where driver stores bkgd addr
COOLDATA    EQU STARTADDR + MSM_Cooler_Status ;Where ASIC stores cooler stat
DETECTID    EQU STARTADDR + MSM_Detector_ID   ;Where ASIC stores detector ID
SHUTID      EQU STARTADDR + MSM_Shutter_ID    ;Where ASIC stores shutter ID
STREAKFLG   EQU STARTADDR + MSM_StreakMode    ;DAC Flag for streaker mode
STREAK_FC   EQU STARTADDR + MSM_FC_StreakMode ;FC routine for streaker mode
STREAK_OTFC EQU STARTADDR + MSM_FC_StrkOTMode ;FC routine for streaker mode
PIA_INVAL   EQU STARTADDR + MSM_PIA_In        ;Where ASIC stores PIA input  
PIA_OPVAL   EQU STARTADDR + MSM_PIA_Out       ;Where DAC gets PIA output    
FC_SCANMODE EQU STARTADDR + MSM_FC_Scan_Mode  ;Where FC scan mode is stored
FC_OHMODE   EQU STARTADDR + MSM_FC_OH         ;Where FC overhead routine is
DC_ADDMODE  EQU STARTADDR + MSM_DC_Add_Mode   ;Where DC scan mode is stored
DC_REPMODE  EQU STARTADDR + MSM_DC_Rep_Mode   ;Where DC scan mode is stored
DC_SUBMODE  EQU STARTADDR + MSM_DC_Sub_Mode   ;Where DC scan mode is stored
DC_IGNMODE  EQU STARTADDR + MSM_DC_Ign_Mode   ;Where DC scan mode is stored
DC_OHMODE   EQU STARTADDR + MSM_DC_OH         ;Where FC overhead routine is
DA_COMPLETE EQU STARTADDR + MSM_DA_Complete   ;Where DA Comp. flag is stored
DA_SERFLAG  EQU STARTADDR + MSM_Serial_Mode   ;Where Smode flag is stored
DA_IMGFLAG  EQU STARTADDR + MSM_Image_Mode    ;Where Imode flag is stored
DA_KEEPCLN  EQU STARTADDR + MSM_Keepclean
SC_ADDRESS  EQU STARTADDR + MSM_SC_Address    ;Where SrcCmp address is stored
PCNTR       EQU STARTADDR + MSM_Pointnum      ;Where number of points is
