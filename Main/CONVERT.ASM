        PAGE    ,132
TITLE CONVERT - C callable assembly language functions to do type and unit conversions
;**********************************************************************
;
; CASMTOOL.asm
;
;  copyright (c) 1990, EG&G Instruments Inc.
;
;  C callable assembly language functions.  Used as pre-optimized
;  functional segments.
;
;**********************************************************************

.MODEL LARGE, C
.286                    ; Enable 80286 instructions
.287                    ; Enable 80287 instructions

TRUE            EQU     1
FALSE           EQU     0


; 287 exception masks
EM_INVALID      EQU     0001H      ;/*   invalid */
EM_DENORMAL     EQU     0002H      ;/*   denormal */
EM_ZERODIVIDE   EQU     0004H      ;/*   zero divide */
EM_OVERFLOW     EQU     0008H      ;/*   overflow */
EM_UNDERFLOW    EQU     0010H      ;/*   underflow */
EM_INEXACT      EQU     0020H      ;/*   inexact (precision) */

;/* Rounding Control */
RC_CHOP         EQU     0c00H      ;/*   chop */
RC_UP           EQU     0800H      ;/*   up */
RC_DOWN         EQU     0400H      ;/*   down */
RC_NEAR         EQU     0000H      ;/*   near */

RC_CLEAR        EQU     0F3FFH     ; mask off old rounding bits 
CW_F_TO_QW      EQU     RC_NEAR + EM_OVERFLOW + EM_UNDERFLOW
                        


FP_TYPE         EQU     20H
SIGNED_TYPE     EQU     10H
POINTER_TYPE     EQU     41H

FLOATTYPE       EQU     FP_TYPE + SIGNED_TYPE + 4
DOUBLETYPE      EQU     FP_TYPE + SIGNED_TYPE + 8
UCHARTYPE       EQU     1
SHORTTYPE       EQU     SIGNED_TYPE + 2
USHORTTYPE      EQU     2
LONGTYPE        EQU     SIGNED_TYPE + 4
ULONGTYPE       EQU     4

;/* data units */       from OMA35.H
COUNTS          EQU     0
ANGSTROM        EQU     1
NM              EQU     2     ; nanometers
UM              EQU     3     ; micrometers
MM              EQU     4     ; millimeters
CM              EQU     5     ; centimeters
METER           EQU     6     ;
WAVENUMBER      EQU     7     ;
RSHIFT          EQU     8     ; raman shift
EV              EQU     9     ; electron volts
JOULE           EQU     10    ;
ERG             EQU     11    ;
HZ              EQU     12    ;
ADJ_NM          EQU     13    ; Nanometers adjusted for spectrograph

SECOND          EQU     20    ;
MSEC            EQU     21    ; millisecond
USEC            EQU     22    ; microsecond
NSEC            EQU     23    ; nanoseconds
PSEC            EQU     24    ; picoseconds
FSEC            EQU     25    ; femtoseconds

        .DATA
PowerTable      DQ      1.0e-15
                DQ      1.0e-14
                DQ      1.0e-13
                DQ      1.0e-12
                DQ      1.0e-11
                DQ      1.0e-10
                DQ      1.0e-9
                DQ      1.0e-8
                DQ      1.0e-7
                DQ      1.0e-6
                DQ      1.0e-5
                DQ      1.0e-4
                DQ      1.0e-3
                DQ      1.0e-2
                DQ      1.0e-1
                DQ      1.0
                DQ      1.0e1
                DQ      1.0e2
                DQ      1.0e3
                DQ      1.0e4
                DQ      1.0e5
                DQ      1.0e6
                DQ      1.0e7
                DQ      1.0e8
                DQ      1.0e9
                DQ      1.0e10
                DQ      1.0e11
                DQ      1.0e12
                DQ      1.0e13
                DQ      1.0e14
                DQ      1.0e15

;Source : 1989 CRC 
        ; M-1 to eV conversion factor   
IMToeV          DQ      1.23984244e-6
        ; M-1 to Erg conversion factor   
IMToErg         DQ      1.9864775e-18
        ; M-1 to Joule conversion factor   
IMToJoule       DQ      1.9864775e-25
        ; M-1 to Hz conversion factor   
IMToHz          DQ      2.99792458e8
        ; eV to M-1 conversion
        ; wavenumber to eV conversion factor   
WnToeV          DQ      1.23984244e-4
        ; wavenumber to Erg conversion factor   
WnToErg         DQ      1.9864775e-16
        ; wavenumber to Joule conversion factor   
WnToJoule       DQ      1.9864775e-23
        ; wavenumber to Hz conversion factor   
WnToHz          DQ      2.99792458e10
        ; eV to wavenumber conversion
EvToWn          DQ      8.0655410e3
        ; Erg to wavenumber conversion
ErgToWn         DQ      5.0341125e15
        ; Joule to wavenumber conversion
JouleToWn       DQ      5.0341125e22
        ; Hz to wavenumber conversion
HzToWn          DQ      3.335640952e-11

;        .DATA
;TempVar         DQ      0
;                DQ      0
;
        .CODE

;***************************************************************************
;*                                                                         *
;*                                                                         *
;*  C declaration:                                                         *
;*   BOOL ConvertUnits( UCHAR DstUnits, PDOUBLE pDstValue,                 *
;*                      UCHAR SrcUnits, double SrcValue, double RelVal )   *
;*                                                                         *
;*   function:   Converts values from one unit dimension to another.       *
;*                                                                         *
;*   requires:   (See oma4.h for allowable unit values)                    *
;*               DstUnits - units to convert to.                           *
;*               pDstValue - Pointer to converted value.                   *
;*               SrcUnits - units to convert from.                         *
;*               pDstValue - value to convert.                             *
;*               RelVal - Relative value for conversion, such as laser     *
;*                        line for Raman Shift. In nm for raman shift.     *
;*                                                                         *
;*   returns:    FALSE - good conervion                                    *
;*               TRUE - conversion not possible, will set *pDstValue to    *
;*                      SrcValue.                                          *
;*                                                                         *
;*   side effects:                                                         *
;*                 4/24/90  DAI                                            *
;*                                                                         *
;***************************************************************************

ConvertUnits    PROC FAR USES bx cx dx si di ds es, \
                     DstUnits:BYTE, pDstVal:FAR PTR QWORD, \
                     SrcUnits:BYTE, SrcVal:QWORD, RelVal:QWORD

        ; get the source and destination addresses where they can be used
        LES     DI, pDstVal
        LEA     SI, SrcVal
        MOV     DH, DstUnits
        CMP     DH, ADJ_NM
        JNZ     NS_ADJ
        MOV     DH, NM
NS_ADJ:
        MOV     DL, SrcUnits
        CMP     DL, ADJ_NM
        JNZ     ND_ADJ
        MOV     DL, NM
ND_ADJ:
;   if (DstUnits == SrcUnits)
;      return FALSE;
        CMP     DH, DL
        JNZ     CONVERT1
        JMP     CU_RETURN_GOOD

CONVERT1:
;   /* can't convert unitless values */       //**&&&*** 1/8/90 DAI
;   if ((DstUnits == COUNTS) || (SrcUnits == COUNTS))
;      return FALSE;
        CMP     DH, COUNTS
        JNZ     CONVERT2
        JMP     CU_RETURN_GOOD
CONVERT2:
        CMP     DL, COUNTS
        JNZ     CONVERT3
        JMP     CU_RETURN_GOOD

CONVERT3:
;   if ( ((DstUnits == RSHIFT) || (SrcUnits == RSHIFT)) && (RelVal == 0.0) )
;      return TRUE;  /* can't do a divide by 0 for conversion */
        CMP     DH, RSHIFT
        JZ      TEST_RELVAL

        CMP     DL, RSHIFT
        JNZ     CONVERT5

TEST_RELVAL:
        FLD     QWORD PTR RelVal        ; in nanometers
        FLDZ
        FCOMP
        FSTSW   AX
        SAHF
        JNZ     CONVERT4
        JMP     CU_RETURN_BAD

CONVERT4:
        FDIVR   QWORD PTR PowerTable[22 * 8]
                                        ; convert RelVal to wavenumbers
                                        ; will be at stack pos 1 after load
                                        ; source value

CONVERT5:
        FLD     QWORD PTR SrcVal
        DEC     DL
        CMP     DL, FSEC-1              ; check for bad unit type
        JNA     CONVERT6
        JMP     CU_RETURN_BAD

CONVERT6:
;   switch (SrcUnits)
;      {
;      /* convert distances to meters for final conversion */
;      case ANGSTROM:
;         *pDstValue *=1.0e-1;

        XOR     CX, CX                  ; power table index
        MOV     BL, DL
        XOR     BH, BH
        ADD     BL, BL
        JMP     WORD PTR CS:SrcUnitJmpTable[BX]

SRC_ANGSTROM:
        MOV     CL, -10                  ; source power
        JMP SHORT SRC_M
;      case NM:
;         *pDstValue *=1.0e-3;
SRC_NM:
        MOV     CL, -9                   ; source power
        JMP SHORT SRC_M
;      case UM:
;         *pDstValue *=1.0e-3;
SRC_UM:
        MOV     CL, -6                   ; source power
        JMP SHORT SRC_M
;      case MM:
;         *pDstValue *=1.0e-1;
SRC_MM:
        MOV     CL, -3                   ; source power
        JMP SHORT SRC_M
;      case CM:
;         *pDstValue *=1.0e-2;
SRC_CM:
        MOV     CL, -2                   ; source power
;      case METER:
SRC_M:
;         switch (DstUnits)
;            {
        DEC     DH
        CMP     DH, HZ-1              ; check for bad unit type
        JNA     CONVERT7
        JMP     CU_RETURN_BAD
CONVERT7:
        MOV     BL, DH
        XOR     BH, BH
        ADD     BL, BL
        JMP     WORD PTR CS:Dst1UnitJmpTable[BX]

;            case ANGSTROM:
;               *pDstValue *=1.0e10;
;               break;
DST1_ANGSTROM:
        ADD     CL, 10
        JMP SHORT DST1_M
;
;            case NM:
;               *pDstValue *=1.0e9;
;               break;
DST1_NM:
        ADD     CL, 9
        JMP SHORT DST1_M
;
;            case UM:
;               *pDstValue *=1.0e6;
;               break;
DST1_UM:
        ADD     CL, 6
        JMP SHORT DST1_M
;
;            case MM:
;               *pDstValue *=1.0e3;
;               break;
DST1_MM:
        ADD     CL, 3
        JMP SHORT DST1_M
;
;            case CM:
;               *pDstValue *=1.0e2;
;               break;
DST1_CM:
        ADD     CL, 2
;
;            case METER:
;               break;
DST1_M:
        MOV     BX, CX
        ADD     BL, 15          ; make 0 based
        SHL     BX, 3           ; QWORD Offset into table
        FMUL    QWORD PTR PowerTable[BX]
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD
;
;
;            case WAVENUMBER:
;               *pDstValue = 1.0e-2 / *pDstValue;
;               break;
DST1_WN:
        MOV     AL, CL          ; convert SrcVal to meters
        MOV     BL, 13          ; Meters To CM power factor (-2)
                                ; and make 0 based (+15)
        SUB     BL, AL
        XOR     BH, BH
        SHL     BX, 3           ; QWORD Offset into table
        FDIVR   QWORD PTR PowerTable[BX]   ; Convert To WaveNumbers
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD
;
;
;            case RSHIFT:  /* raman shift */
;               *pDstValue = 1.0e-2 / *pDstValue;
;               /* convert reference value to wavenumbers */
;               RelVal = 1.0e2 / RelVal;
;               *pDstValue = RelVal - *pDstValue;
;               break;
DST1_RS:
        ; Rel val is already in wavenumbers in stack position 1
        MOV     AL, CL          ; convert SrcVal to meters
        MOV     BL, 13          ; Meters To CM power factor (-2)
                                ; and make 0 based (+15)
        SUB     BL, AL
        XOR     BH, BH
        SHL     BX, 3           ; QWORD Offset into table
        FDIVR   QWORD PTR PowerTable[BX]   ; Convert To WaveNumbers
        FSUBP   ST(1), ST
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD
;
;            case EV:
;               /* assume speed of light */
;               /* convert first to Hz */
;               *pDstValue = LIGHTSPEED / *pDstValue;
;
;               /* convert from Hz to eV */
;               *pDstValue *= PLANCK * 6.24180e11;
;               break;
DST1_EV:
        MOV     AL, CL          ; convert SrcVal to meters
        MOV     BL, 15          ; make Meters To power factor (0)
                                ; and make 0 based (+15)
        SUB     BL, AL          ; power for source to meters
        XOR     BH, BH
        SHL     BX, 3           ; QWORD Offset into table
        FDIVR   QWORD PTR PowerTable[BX]   ; Convert To inverse meters
        FMUL    QWORD PTR IMToeV
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD
;
;            case ERG:
;               /* assume speed of light */
;               /* convert first to Hz */
;               *pDstValue = LIGHTSPEED / *pDstValue;
;
;               /* convert from Hz to ergs*/
;               *pDstValue *= PLANCK * 1.0e7;
;
;               break;
DST1_ERG:
        MOV     AL, CL          ; convert SrcVal to meters
        MOV     BL, 15          ; make Meters To power factor (0)
                                ; and make 0 based (+15)
        SUB     BL, AL
        XOR     BH, BH
        SHL     BX, 3           ; QWORD Offset into table
        FDIVR   QWORD PTR PowerTable[BX]   ; Convert To WaveNumbers
        FMUL    QWORD PTR IMToErg
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD
;
;            case JOULE:
;               /* assume speed of light */
;               /* convert first to Hz */
;               *pDstValue = LIGHTSPEED / *pDstValue;
;
;               /* convert from Hz to Joules*/
;               *pDstValue *= PLANCK;
;               break;
DST1_J:
        MOV     AL, CL          ; convert SrcVal to meters
        MOV     BL, 15          ; make Meters To power factor (0)
                                ; and make 0 based (+15)
        SUB     BL, AL
        XOR     BH, BH
        SHL     BX, 3           ; QWORD Offset into table
        FDIVR   QWORD PTR PowerTable[BX]   ; Convert To WaveNumbers
        FMUL    QWORD PTR IMToJoule
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD
;
;            case HZ:
;               /* assume speed of light */
;               *pDstValue = LIGHTSPEED / *pDstValue;
;               break;
DST1_HZ:
        MOV     AL, CL          ; convert SrcVal to meters
        MOV     BL, 15          ; make Meters To power factor (0)
                                ; and make 0 based (+15)
        SUB     BL, AL
        XOR     BH, BH
        SHL     BX, 3           ; QWORD Offset into table
        FDIVR   QWORD PTR PowerTable[BX]   ; Convert To WaveNumbers
        FMUL    QWORD PTR IMToHz
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD
;
;            default:
;               return TRUE;  /* no conversion */
;            }
;         break;
;
;      case RSHIFT:
;         /* convert first to wavenumber */
;         /* assume that RelVal is in nanometers */
;         *pDstValue = (1.0e2 / RelVal) - *pDstValue;
SRC_RSHIFT:
        FSUBRP  ST(1), ST
        FSTP    QWORD PTR SrcVal
;
;      case WAVENUMBER:  /* convert to meters */
;         *pDstValue = 1.0e-2 / *pDstValue;
;         /* call function again using meters as source units */
;         return ConvertUnits( DstUnits, pDstValue, METER, *pDstValue,
;                                                               RelVal );
SRC_WN:
        DEC     DH
        CMP     DH, HZ-1              ; check for bad unit type
        JNA     CONVERT8
        JMP     CU_RETURN_BAD

CONVERT8:
        MOV     BL, DH
        XOR     BH, BH
        ADD     BL, BL
        JMP     WORD PTR CS:Dst2UnitJmpTable[BX]

DST2_ANGSTROM:
        FDIVR   QWORD PTR PowerTable[23 * 8]
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD

DST2_NM:
        FDIVR   QWORD PTR PowerTable[22 * 8]
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD

DST2_UM:
        FDIVR   QWORD PTR PowerTable[19 * 8]
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD

DST2_MM:
        FDIVR   QWORD PTR PowerTable[16 * 8]
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD

DST2_CM:
        FLD1           ; just invert
        FDIVRP  ST(1), ST
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD

DST2_M:
        FDIVR   QWORD PTR PowerTable[13 * 8]
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD

DST2_WN:
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD

DST2_RS:
        ; Rel val is already in wavenumbers in stack position 1
;         CM = RS + st(1);
        FSUBP   ST(1), ST
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD

DST2_EV:
        FMUL    QWORD PTR WnToeV
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD

DST2_ERG:
        FMUL    QWORD PTR WnToErg
        FSTP    QWORD PTR SrcVal
        JMP     CU_RETURN_GOOD

DST2_J:
        FMUL    QWORD PTR WnToJoule
        FSTP    QWORD PTR SrcVal
        JMP SHORT CU_RETURN_GOOD

DST2_HZ:
        FMUL    QWORD PTR WnToHz
        FSTP    QWORD PTR SrcVal
        JMP SHORT CU_RETURN_GOOD

;
;      /* convert from energy to wavelength first */
;      case EV:
;         *pDstValue *= 1.60210e-12;
SRC_EV:
        FMUL    QWORD PTR EvToWn
        JMP     SRC_WN
;      case ERG:
;         *pDstValue *= 1.0e-7;
SRC_ERG:
        FMUL    QWORD PTR ErgToWn
        JMP     SRC_WN
;      case JOULE:
;         /* convert from Joules to Hz */
;         *pDstValue /= PLANCK;
SRC_J:
        FMUL    QWORD PTR JouleToWn
        JMP     SRC_WN
;      case HZ:
;         /* convert from Hz to meters, assume speed of light */
;         *pDstValue = LIGHTSPEED / *pDstValue;
;
;         /* call function again using meters as source units */
;         return ConvertUnits( DstUnits, pDstValue, METER, *pDstValue,
;                                                               RelVal );
SRC_HZ:
        FMUL    QWORD PTR HzToWn
        JMP     SRC_WN
;
;      /* convert times to seconds first */
;      case FSEC:
;         *pDstValue *=1.0e-3;
SRC_FSEC:
        MOV     CL, -15                  ; source power
        JMP SHORT SRC_SEC
;      case PSEC:
;         *pDstValue *=1.0e-3;
SRC_PSEC:
        MOV     CL, -12                  ; source power
        JMP SHORT SRC_SEC
;      case NSEC:
;         *pDstValue *=1.0e-3;
SRC_NSEC:
        MOV     CL, -9                   ; source power
        JMP SHORT SRC_SEC
;      case USEC:
;         *pDstValue *=1.0e-3;
SRC_USEC:
        MOV     CL, -6                   ; source power
        JMP SHORT SRC_SEC
;      case MSEC:
;         *pDstValue *=1.0e-3;
SRC_MSEC:
        MOV     CL, -3                   ; source power
        JMP SHORT SRC_SEC
;      case SECOND:
SRC_SEC:
;         switch (DstUnits)
;            {
        SUB     DH, SECOND
        JS      CU_RETURN_BAD
        CMP     DH, FSEC-SECOND         ; check for bad unit type
        JA      CU_RETURN_BAD
        MOV     BL, DH
        XOR     BH, BH
        ADD     BL, BL
        JMP     WORD PTR CS:Dst3UnitJmpTable[BX]
;
;            case MSEC:
;               *pDstValue *=1.0e3;
;               break;
DST3_MSEC:
        ADD     CL, 3
        JMP SHORT DST3_SEC
;
;            case USEC:
;               *pDstValue *=1.0e6;
;               break;
DST3_USEC:
        ADD     CL, 6
        JMP SHORT DST3_SEC
;
;            case NSEC:
;               *pDstValue *=1.0e9;
;               break;
DST3_NSEC:
        ADD     CL, 9
        JMP SHORT DST3_SEC
;
;            case PSEC:
;               *pDstValue *=1.0e12;
;               break;
DST3_PSEC:
        ADD     CL, 12
        JMP SHORT DST3_SEC
;
;            case FSEC:
;               *pDstValue *=1.0e15;
;               break;
DST3_FSEC:
        ADD     CL, 15
        JMP SHORT DST3_SEC

;            case SECOND:
;               break;
DST3_SEC:
        MOV     BX, CX
        ADD     BL, 15          ; make 0 based
        SHL     BX, 3           ; QWORD Offset into table
        FMUL    QWORD PTR PowerTable[BX]
        FSTP    QWORD PTR SrcVal
        JMP SHORT CU_RETURN_GOOD
;
;            default:
;               return TRUE;  /* no conversion */
;
;            }
;         break;
;
;
;;      default:
;;         return TRUE;
;      }
CU_RETURN_BAD:
        MOV     AX,TRUE
        JMP SHORT LEAVE_CU

CU_RETURN_GOOD:
;   return FALSE;
        XOR     AX,AX

LEAVE_CU:
;}
        MOVSW               ; copy the 8 bytes from SrcVal to the destination
        MOVSW
        MOVSW
        MOVSW

        FWAIT                   ; wait for last multiply to be done
        RET

SrcUnitJmpTable:
        DW      SRC_ANGSTROM
        DW      SRC_NM
        DW      SRC_UM
        DW      SRC_MM
        DW      SRC_CM
        DW      SRC_M
        DW      SRC_WN
        DW      SRC_RSHIFT
        DW      SRC_EV
        DW      SRC_J
        DW      SRC_ERG
        DW      SRC_HZ
        DW      CU_RETURN_BAD
        DW      CU_RETURN_BAD
        DW      CU_RETURN_BAD
        DW      CU_RETURN_BAD
        DW      CU_RETURN_BAD
        DW      CU_RETURN_BAD
        DW      CU_RETURN_BAD
        DW      SRC_SEC
        DW      SRC_MSEC
        DW      SRC_USEC
        DW      SRC_NSEC
        DW      SRC_PSEC
        DW      SRC_FSEC

Dst1UnitJmpTable:
        DW      DST1_ANGSTROM
        DW      DST1_NM
        DW      DST1_UM
        DW      DST1_MM
        DW      DST1_CM
        DW      DST1_M
        DW      DST1_WN
        DW      DST1_RS
        DW      DST1_EV
        DW      DST1_J
        DW      DST1_ERG
        DW      DST1_HZ

Dst2UnitJmpTable:
        DW      DST2_ANGSTROM
        DW      DST2_NM
        DW      DST2_UM
        DW      DST2_MM
        DW      DST2_CM
        DW      DST2_M
        DW      DST2_WN
        DW      DST2_RS
        DW      DST2_EV
        DW      DST2_J
        DW      DST2_ERG
        DW      DST2_HZ

Dst3UnitJmpTable:
        DW      DST3_SEC
        DW      DST3_MSEC
        DW      DST3_USEC
        DW      DST3_NSEC
        DW      DST3_PSEC
        DW      DST3_FSEC
ConvertUnits ENDP

;/***************************************************************************/
;/*  VOID ConvertTypes( void * Source, int SourceType,                      */
;/*                     void *Destination, int DestinationType )            */
;/*                                                                         */
;/*  function:  Take a representation of a value and convert to             */
;/*             another data type representation. If the source or          */
;/*             destination types are not defined, the defined type will be */
;/*             used as the undefined type.  If both are undefined, a 4     */
;/*             byte copy will happen.                                      */
;/*                                                                         */
;/*  variables:                                                             */
;/*             Source - Pointer to a value to convert into another type    */
;/*             SourceType - Type of the source value                       */
;/*             Destination - Output. Pointer to output variable            */
;/*             DestinationType - Input. Data Type as defined using type    */
;/*                               #define's in OMA35.H                      */
;/*                                                                         */
;/* last changed:                                                           */
;/*               2/13/90   DI                                              */
;/*                                                                         */
;/***************************************************************************/
;
;VOID ConvertTypes( void * Source, int SourceType,
;                   void *Destination, int DestinationType )
;{
;   ULONG ulTemp;
;   LONG lTemp;
;   DOUBLE dTemp, RoundNum;
;   SHORT TempType;
;
ConvertTypes    PROC FAR USES bx cx dx si di ds es, \
                     Src: FAR PTR, SrcType:WORD,    \
                     Dst: FAR PTR, DstType:WORD


        LOCAL   TempVar14 : WORD
        LOCAL   TempVar12 : WORD
        LOCAL   TempVar10 : WORD
        LOCAL   TempVar8  : WORD
        LOCAL   TempVar6  : WORD
        LOCAL   TempVar4  : WORD
        LOCAL   TempVar2  : WORD
        LOCAL   TempVar   : WORD


        ; get the source and destination addresses where they can be used
        MOV     DX, SS:DstType     ;Dst Type in DL
        MOV     AX, SS:SrcType
        MOV     DH, AL

        LES     DI, SS:Dst
        LDS     SI, SS:Src

;   /* convert string pointer to a ULONG */
;   if (SourceType == POINTERTYPE)
;      SourceType = ULONGTYPE;
;   if (DestinationType == POINTERTYPE)
;      DestinationType = ULONGTYPE;
;
;   if (SourceType == DestinationType)
;      {
;      memcpy( Destination, Source, (SourceType & 0x0F) );
;      return;
;      }
CT1:
        CMP     DL, DH
        JNZ     CT2
        CLD
        MOV     CL, DL
        AND     CX, 0FH
        REP     MOVSB
        JMP     CT_RETURN_GOOD

;   if ( (DestinationType != FLOATTYPE) &&
;        (DestinationType != UCHARTYPE) &&
;        (DestinationType != SHORTTYPE) &&
;        (DestinationType != USHORTTYPE) &&
;        (DestinationType != LONGTYPE) &&
;        (DestinationType != ULONGTYPE) &&
;        (DestinationType != DOUBLETYPE) )
;      TempType = FALSE;
;   else
;      TempType = TRUE;
;
;
;   switch (SourceType)
;      {
CT2:
        XOR     CX, CX          ; clear registers to hold temporary variables
        XOR     BX, BX
        TEST    DH, FP_TYPE
        JNZ     SRC_FP_TYPE
        TEST    DH, SIGNED_TYPE
        JZ      CT2AA
        JMP     SRC_SIGNED_TYPE
CT2AA:
        CMP     DH, POINTER_TYPE
        JZ      CT2A
        JMP     SRC_UNSIGNED_TYPE
CT2A:
        MOV     DH, ULONGTYPE
        JMP     SRC_UNSIGNED_TYPE
;      case FLOATTYPE:
;         if (! TempType)
;            {
;            *(PFLOAT)Destination = *(PFLOAT)Source;
;            return;
;            }
;         dTemp = *(PFLOAT)Source;
;         TempType = DOUBLETYPE;
;         break;
;      case DOUBLETYPE:
;         if (! TempType)
;            {
;            *(PDOUBLE)Destination = *(PDOUBLE)Source;
;            return;
;            }
;         dTemp = *(PDOUBLE)Source;
;         TempType = DOUBLETYPE;
;         break;

SRC_FP_TYPE:
        CMP     DH, DOUBLETYPE
        JZ      SRC_LONG_FLOAT
        CMP     DH, FLOATTYPE
        JZ      SRC_SHORT_FLOAT
        JMP     CT_UNRECOGNIZED

SRC_SHORT_FLOAT:
        FLD     DWORD PTR [SI]
        JMP SHORT SRC_FP1

SRC_LONG_FLOAT:
        FLD     QWORD PTR [SI]
SRC_FP1:
        CMP     DL, DOUBLETYPE
        JZ      DST1_LONG_FLOAT
        CMP     DL, FLOATTYPE
        JNZ     FLOAT_TO_INT

        ; 4 byte float
        FSTP    DWORD PTR ES:[DI]
        JMP     CT_RETURN_GOOD

DST1_LONG_FLOAT:
        ; 8 byte float
        FSTP    QWORD PTR ES:[DI]
        JMP     CT_RETURN_GOOD
FLOAT_TO_INT:
        CMP     DL, SHORTTYPE
        JZ      FLOAT_TO_LONG
        CMP     DL, USHORTTYPE
        JZ      FLOAT_TO_LONG
        CMP     DL,LONGTYPE
        JZ      FLOAT_TO_LONG
        CMP     DL, ULONGTYPE
        JZ      FLOAT_TO_LONG
        CMP     DL, UCHARTYPE
        JZ      FLOAT_TO_LONG
        JMP     CT_UNRECOGNIZED
        
FLOAT_TO_LONG:
        LEA     BX, TempVar
        FSTCW   WORD PTR SS:[BX]
        FWAIT
        MOV     AX, WORD PTR SS:[BX]
        AND     AX, RC_CLEAR
        OR      AX, CW_F_TO_QW          ; mask out underflow and overflow
        MOV     WORD PTR SS:[BX + 2], AX
        FLDCW   WORD PTR SS:[BX + 2]
        FISTP   QWORD PTR SS:[BX + 8]
        FWAIT
        MOV     AX, WORD PTR SS:[BX + 8]
        FLDCW   WORD PTR SS:[BX]
        TEST    DL, 1
        JNZ     FLOAT_TO_1BYTE
        MOV     WORD PTR ES:[DI], AX
        TEST    DL, 2
        JNZ     FLOAT_TO_2BYTE
        MOV     AX, WORD PTR SS:[BX + 10]
        MOV     WORD PTR ES:[DI+2], AX
        JMP     CT_RETURN_GOOD

FLOAT_TO_1BYTE:
        MOV     BYTE PTR ES:[DI], AL
        JMP     CT_RETURN_GOOD


FLOAT_TO_2BYTE:
        MOV     WORD PTR ES:[DI], AX
        JMP     CT_RETURN_GOOD

;      case SHORTTYPE:
;         if (! TempType)
;            {
;            *(PSHORT)Destination = *(PSHORT)Source;
;            return;
;            }
;         lTemp = (LONG) *(PSHORT)Source;
;         TempType = LONGTYPE;
;         break;
;
;      case LONGTYPE:
;         if (! TempType)
;            {
;            *(PLONG)Destination = *(PLONG)Source;
;            return;
;            }
;         lTemp = *(PLONG)Source;
;         TempType = LONGTYPE;
;         break;
;
SRC_SIGNED_TYPE:
        TEST    DL, FP_TYPE     ; test for floating point destination
        JNZ     SIGNED_TO_FLOAT
        ; put the low word in BX
        MOV     BX, WORD PTR DS:[SI]
        CMP     DH, LONGTYPE
        JNZ     SRC_SIGNED_SHORT
        ; put the high word in CX
        MOV     CX, WORD PTR DS:[SI+2]
        JMP SHORT SRC_S1


SRC_SIGNED_SHORT:
        CMP     DH, SHORTTYPE
        JZ      NEG_TEST
        JMP     CT_UNRECOGNIZED
NEG_TEST:
        TEST    BX, 8000H       ; test for neg
        JZ      SRC_S1
        MOV     CX, 0FFFFH      ; extend sign

SRC_S1:
        CMP     DL, LONGTYPE
        JZ      SIGNED_TO_LONG
        CMP     DL, ULONGTYPE
        JZ      SIGNED_TO_LONG
        ; destination is short or bad
        CMP     DL, SHORTTYPE
        JZ      SIGNED_TO_SHORT
        CMP     DL, USHORTTYPE
        JZ      SIGNED_TO_SHORT
        CMP     DL, UCHARTYPE
        JZ      SIGNED_TO_CHAR
        JMP     CT_UNRECOGNIZED

SIGNED_TO_SHORT:
        ; dst is a short
        MOV     ES:[DI], BX
        JMP     CT_RETURN_GOOD

SIGNED_TO_CHAR:
        ; dst is a byte
        MOV     ES:[DI], BL
        JMP     CT_RETURN_GOOD

SIGNED_TO_LONG:
        ; low word is already in BX high is in CX
        MOV     ES:[DI], BX
        MOV     ES:[DI+2], CX
        JMP     CT_RETURN_GOOD

SIGNED_TO_FLOAT:
        CMP     DH, LONGTYPE
        JZ      LONG_TO_FLOAT

        ; source is short or bad
        CMP     DH, SHORTTYPE
        JZ      SHORT_TO_FLOAT
        JMP     CT_UNRECOGNIZED

SHORT_TO_FLOAT:
        FILD    WORD PTR DS:[SI]
        JMP SHORT SRC_S2

LONG_TO_FLOAT:
        FILD    DWORD PTR DS:[SI]

SRC_S2:
        CMP     DL, FLOATTYPE
        JZ      INTEGER_TO_SHORT_FLOAT
        CMP     DL, DOUBLETYPE
        JZ      INTEGER_TO_LONG_FLOAT
        JMP     CT_UNRECOGNIZED

INTEGER_TO_SHORT_FLOAT:
        FSTP    DWORD PTR ES:[DI]
        JMP     CT_RETURN_GOOD

INTEGER_TO_LONG_FLOAT:
        FSTP    QWORD PTR ES:[DI]
        JMP     CT_RETURN_GOOD

;      case UCHARTYPE:
;         if (! TempType)
;            {
;            *(PUCHAR)Destination = *(PUCHAR)Source;
;            return;
;            }
;         ulTemp = (ULONG) *(PUCHAR)Source;
;         TempType = ULONGTYPE;
;         break;
;
;      case USHORTTYPE:
;         if (! TempType)
;            {
;            *(PUSHORT)Destination = *(PUSHORT)Source;
;            return;
;            }
;         ulTemp = (ULONG) *(PUSHORT)Source;
;         TempType = ULONGTYPE;
;         break;
;
;      case ULONGTYPE:
;         if (! TempType)
;            {
;            *(PULONG)Destination = *(PULONG)Source;
;            return;
;            }
;         ulTemp = *(PULONG)Source;
;         TempType = ULONGTYPE;
;         break;
SRC_UNSIGNED_TYPE:
        ; put the low word in BX
        XOR     BH, BH
        MOV     BL, BYTE PTR DS:[SI]
        CMP     DH, UCHARTYPE
        JNZ     SRC_2A
        MOV     DH, USHORTTYPE
        JMP SHORT SRC_2B

SRC_2A:
        MOV     BH, BYTE PTR DS:[SI + 1] ; fill in rest of first word for
                                         ; USHORT and ULONG

SRC_2B:
        CMP     DH, ULONGTYPE
        JNZ     SRC_S3
        ; put the high word in CX
        MOV     CX, WORD PTR DS:[SI+2]

SRC_S3:
        TEST    DL, FP_TYPE     ; test for floating point destination
        JNZ     UNSIGNED_TO_FLOAT
        CMP     DH, ULONGTYPE
        JZ      SRC_S4
        CMP     DH, USHORTTYPE
        JZ      SRC_S4
        JMP SHORT CT_UNRECOGNIZED

SRC_S4:
        CMP     DL, LONGTYPE
        JZ      UNSIGNED_TO_LONG
        CMP     DL, ULONGTYPE
        JZ      UNSIGNED_TO_LONG
        ; destination is short or bad
        CMP     DL, SHORTTYPE
        JZ      UNSIGNED_TO_SHORT
        CMP     DL, USHORTTYPE
        JZ      UNSIGNED_TO_SHORT
        CMP     DL, UCHARTYPE
        JZ      UNSIGNED_TO_CHAR
        JMP SHORT CT_UNRECOGNIZED

UNSIGNED_TO_SHORT:
        ; dst is a short
        MOV     ES:[DI], BX
        JMP SHORT CT_RETURN_GOOD

UNSIGNED_TO_CHAR:
        ; dst is a byte
        MOV     ES:[DI], BL
        JMP SHORT CT_RETURN_GOOD

UNSIGNED_TO_LONG:
        ; low word is already in BX high is in CX
        MOV     ES:[DI], BX
        MOV     ES:[DI+2], CX
        JMP SHORT CT_RETURN_GOOD

UNSIGNED_TO_FLOAT:
        ; need to preserve the lack of sign by loading as the next larger
        ; integer type
        XOR     AX, AX
        ; put the low word in the temp word buffer
        MOV     WORD PTR SS:TempVar, BX
        LEA     BX, TempVar
        CMP     DH, ULONGTYPE
        JZ      ULONG_TO_FLOAT

        ; source is unsigned short or bad
        CMP     DH, USHORTTYPE
        JZ      USHORT_TO_FLOAT
        JMP SHORT CT_UNRECOGNIZED

USHORT_TO_FLOAT:
	MOV	WORD PTR SS:[BX + 2], AX
        LEA     BX, TempVar
	FILD	DWORD PTR SS:[BX]
        JMP     SHORT SRC_S5

ULONG_TO_FLOAT:
        MOV     WORD PTR SS:[BX + 2], CX
        MOV     WORD PTR SS:[BX + 4], AX
        MOV     WORD PTR SS:[BX + 6], AX
        FILD    QWORD PTR SS:[BX]

SRC_S5:
        JMP     SRC_S2

;      default:  // must be a don't care, use the destination type as the type
;         // if both are don't cares, default to long integer copy and exit
;         if (! TempType)
;            {
;            *(PLONG)Destination = *(PLONG)Source;
;            return;
;            }
;         TempType = 0;
;         break;
;      }
;
;   if (dTemp < 0.0)
;      RoundNum = -0.5;
;   else
;      RoundNum = 0.5;
;
;   switch (DestinationType)
;      {
;      case UCHARTYPE:
;         switch (TempType)
;            {
;            case ULONGTYPE:
;               *(PUCHAR)Destination = (UCHAR) ulTemp;
;               break;
;            case LONGTYPE:
;               *(PUCHAR)Destination = (UCHAR) lTemp;
;               break;
;            case DOUBLETYPE:
;               *(PUCHAR)Destination = (UCHAR) (dTemp + RoundNum); /* round off */
;               break;
;            case 0:
;               *(PUCHAR)Destination = *(PUCHAR)Source;
;               break;
;            }
;         break;
;
;      case SHORTTYPE:
;         switch (TempType)
;            {
;            case ULONGTYPE:
;               *(PSHORT)Destination = (SHORT) ulTemp;
;               break;
;            case LONGTYPE:
;               *(PSHORT)Destination = (SHORT) lTemp;
;               break;
;            case DOUBLETYPE:
;               *(PSHORT)Destination = (SHORT) (dTemp + RoundNum); /* round off */
;               break;
;            case 0:
;               *(PSHORT)Destination = *(PSHORT)Source;
;               break;
;            }
;         break;
;
;      case USHORTTYPE:
;         switch (TempType)
;            {
;            case ULONGTYPE:
;               *(PUSHORT)Destination = (USHORT) ulTemp;
;               break;
;            case LONGTYPE:
;               *(PUSHORT)Destination = (USHORT) lTemp;
;               break;
;            case DOUBLETYPE:
;               *(PUSHORT)Destination = (USHORT) (dTemp + RoundNum); /* round off */
;               break;
;            case 0:
;               *(PUSHORT)Destination = *(PUSHORT)Source;
;               break;
;            }
;         break;
;
;      case LONGTYPE:
;         switch (TempType)
;            {
;            case ULONGTYPE:
;               *(PLONG)Destination = (LONG) ulTemp;
;               break;
;            case LONGTYPE:
;               *(PLONG)Destination = lTemp;
;               break;
;            case DOUBLETYPE:
;               *(PLONG)Destination = (LONG) (dTemp + RoundNum); /* round off */
;               break;
;            case 0:
;               *(PLONG)Destination = *(PLONG)Source;
;               break;
;            }
;         break;
;
;      case ULONGTYPE:
;         switch (TempType)
;            {
;            case ULONGTYPE:
;               *(PULONG)Destination = ulTemp;
;               break;
;            case LONGTYPE:
;               *(PULONG)Destination = (ULONG) lTemp;
;               break;
;            case DOUBLETYPE:
;               *(PULONG)Destination = (ULONG) (dTemp + RoundNum); /* round off */
;               break;
;            case 0:
;               *(PULONG)Destination = *(PULONG)Source;
;               break;
;            }
;         break;
;
;      case FLOATTYPE:
;         switch (TempType)
;            {
;            case ULONGTYPE:
;               *(PFLOAT)Destination = (FLOAT) ulTemp;
;               break;
;            case LONGTYPE:
;               *(PFLOAT)Destination = (FLOAT) lTemp;
;               break;
;            case DOUBLETYPE:
;               *(PFLOAT)Destination = (FLOAT) dTemp;
;               break;
;            case 0:
;               *(PFLOAT)Destination = *(PFLOAT)Source;
;               break;
;            }
;         break;
;
;      case DOUBLETYPE:
;         switch (TempType)
;            {
;            case ULONGTYPE:
;               *(PDOUBLE)Destination = (DOUBLE) ulTemp;
;               break;
;            case LONGTYPE:
;               *(PDOUBLE)Destination = (DOUBLE) lTemp;
;               break;
;            case DOUBLETYPE:
;               *(PDOUBLE)Destination = dTemp;
;               break;
;            case 0:
;               *(PDOUBLE)Destination = *(PDOUBLE)Source;
;               break;
;            }
;         break;
;
;      default:
;         switch (SourceType)
;            {
;            case ULONGTYPE:
;               *(PULONG)Destination = *(PULONG)Source;
;               break;
;            case LONGTYPE:
;               *(PLONG)Destination = *(PLONG)Source;
;               break;
;            case DOUBLETYPE:
;               *(PULONG)Destination = *(PULONG)Source;
;               break;
;            }
;         return;
;      }
;}
;

CT_UNRECOGNIZED:
        MOVSW           ; default to direct 4 byte copy
        MOVSW

CT_RETURN_BAD:
        FWAIT
        RET
CT_RETURN_GOOD:
        FWAIT
        RET
ConvertTypes ENDP

END





