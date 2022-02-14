                PAGE    ,132
TITLE VDATA - C callable assembly language functions to do type and unit conversions
;**********************************************************************
;
; VDATA.asm
;
;  copyright (c) 1990, EG&G Instruments Inc.
;
;  C callable assembly language functions for virtual data access.  
;  Used as pre-optimized functional segments.
;
;**********************************************************************

.MODEL LARGE, C
.286                    ; Enable 80286 instructions
.287                    ; Enable 80287 instructions


        INCLUDE omatyp.inc

EXTRN   CvBuf : BYTE

EXTRN   LoadCurveBuffer : FAR

EXTRN   ConvertTypes : FAR

.DATA
; secret location for the programmer's names
    PROGRAMMERS     DB      'DWIGHT IRVING, TOM BIGGS, ROBERT COLE, '
                    DB      'JEFF LINDEMUTH, MORRIS MAYNARD, WILLIAM PETERSON'

.CODE
;***************************************************************************/
;*                                                                         */
;* ERROR_CATEGORY GetDataPoint(LPCURVEDIR CurveDir, SHORT EntryIndex,      */
;*                             USHORT CurveIndex, USHORT DataPoint,        */
;*                             PFLOAT X, PVOID Y, CHAR DataType,           */
;*                             SHORT &PrefBuf)                             */
;*                                                                         */
;*  Function: Get an X and Y data point pair.  May be read from any buffer */
;*                                                                         */
;*  Variables: CurveDir - Input. Curve directory for curve entry           */
;*             EntryIndex - Input. Entry for curve block                   */
;*             CurveIndex - Input. File curve number to change.            */
;*             DataPoint - Input. data point number 0....                  */
;*             pX - Output. location to put X value into                   */
;*             pY - Output. location to put Y value into                   */
;*             DataType - see OMA35.h, FLOATTYPE, SHORTTYPE, etc.          */
;*             PrefBuf - Input.  Preferred buffer for loading the curve    */
;*                       portion into.  May not actually be used.          */
;*                                                                         */
;*  Returns:   ERROR_READ - temp file seek error                           */
;*             ERROR_SEEK - temp file seek error                           */
;*             ERROR_NONE - OK                                             */
;*                                                                         */
;*  Side effects: CvBuf contents may be changed.                           */
;*                                                                         */
;***************************************************************************/

FPWORD TYPEDEF FAR PTR WORD

GetDataPoint PROC FAR USES bx cx dx si di ds es,            \
                  pCurveDir : DWORD, EIndex : WORD,         \
                  CIndex : WORD, DataPoint : WORD,          \
                  pX : FAR PTR, pY : FAR PTR, DType : BYTE, \
                  PrefBuf : FPWORD    ; mlm
; mlm                  PrefBuf : WORD
;{
;   USHORT i;
;   ERROR_CATEGORY err;
;   USHORT FirstPoint;
;   BOOL Found = FALSE;
;   USHORT DataSz;
;   PVOID  TmpPtr;
;


        LOCAL   FirstPoint : WORD;


;   /* see if header is already in a buffer */
;   for (i=0; (i<BUFNUM) && (! Found); i++)
;      {
;      if (CvBuf[i].ActiveDir)
;         DataSz = CvBuf[i].Curvehdr.DataType & 0x0F;
;
;      if ( (CvBuf[i].ActiveDir == CurveDir) &&
;           (CvBuf[i].Entry == EntryIndex) &&
;           (CvBuf[i].CurveIndex == CurveIndex) &&
;           (DataPoint >= CvBuf[i].BufferOffset) &&
;           (DataPoint < (CvBuf[i].BufferOffset + ((BUFLEN / 2) / DataSz))) )
;
;         {  /* data point is already in buffer */
;         Found = TRUE;
;         PrefBuf = i;
;         FirstPoint = CvBuf[i].BufferOffset;
;         }
;      }

        CLD
        PUSH    SS
        POP     DS
        MOV     AX, seg CvBuf
        MOV     ES, AX
        XOR     BX, BX
        MOV     CX, BUFNUM        ; look through the curve buffers to see 
                                  ; if curve is already loaded
        PUSH    BUFNUM + 1        ; not found flag
BUF_SEARCH:                       ; get dirptr in DX:AX
        MOV     AX, WORD PTR ES:CvBuf[BX].ActiveDir
        MOV     DX, WORD PTR ES:CvBuf[BX].ActiveDir[2]

        ; if dirptr != 0 buffer is in use
        TEST    AX, AX            
        JZ      END_BUF_SEARCH    ; if not in use loop to test next buf

        ; see if dirptr is same as request
        CMP     AX, WORD PTR SS:pCurveDir 
        JNZ     END_BUF_SEARCH              ; if not same loop, test next buf
        CMP     DX, WORD PTR SS:pCurveDir[2]
        JNZ     END_BUF_SEARCH

        ; same directory
        ; check entry number
        MOV     AX, WORD PTR ES:CvBuf[BX].Entry
        CMP     AX, EIndex
        JNZ     END_BUF_SEARCH

        ; same Entry
        ; check curve number
        MOV     AX, WORD PTR ES:CvBuf[BX].CurveIndex
        CMP     AX, CIndex
        JNZ     END_BUF_SEARCH

        ; same curve
        ; check for data point
        MOV     AX, WORD PTR ES:CvBuf[BX].BufferOffset
        CMP     AX, SS:DataPoint        ; is data point number equal to the
                                        ; first data point in the buffer?
        JZ      FOUND_BUF               ; yes, match found, end search
        JA      END_BUF_SEARCH          ; no, is greater,loop to next buffer
        PUSH    BX                      ; requested 1st point less than 
        PUSH    CX                      ; buffer 1st point
        MOV     CL, BYTE PTR ES:CvBuf[BX].cb_Curvehdr.DataType ;
        MOV     BX, BUF_X_OFFSET
        AND     CL, 0FH                 ; get the data size
        SHR     CL,1                    ; shift right once for 1 byte data
        AND     CL, CL
        JZ      ENDPOINT_CHK
POINTS_IN_BUF:                          ; calculate the number of points of 
        SHR     BX, 1                   ; this size (2,4,or 8) that will fit
        SHR     CL, 1                   ; in the buffer
        JNZ     POINTS_IN_BUF

ENDPOINT_CHK:
        ADD     AX, BX                  ; check to see if data point is 
        CMP     AX, SS:DataPoint        ; less than the last point that can 
        POP     CX                      ; held in the buffer
        POP     BX
        JBE     END_BUF_SEARCH

FOUND_BUF:
        POP     AX                      ; pop off the not found flag
        MOV     AX, BUFNUM
        SUB     AX, CX
        PUSH    AX                      ; push on buffer it was found in
; mlm        MOV     WORD PTR SS:PrefBuf[2], AX
        PUSH    ES                      ; mlm
        LES     DI, PrefBuf             ; mlm
        MOV     ES:[DI], AX             ; mlm
        POP     ES                      ; mlm
        MOV     CX, 1                   ; don't think this is used
        JMP SHORT AFTER_BUF_SEARCH

END_BUF_SEARCH:
        ADD     BX, SIZE CURVEBUFFER
        LOOP    BUF_SEARCH

AFTER_BUF_SEARCH:
        POP     AX
        CMP     AX, BUFNUM
        JB      ALREADY_LOADED

;   if (! Found)
;      {
;      /* not already in buffer so read it in */
;      FirstPoint = DataPoint;
;      if (err = LoadCurveBuffer(CurveDir, EntryIndex, CurveIndex,
;                                &FirstPoint, PrefBuf))
;         return err;
;      }
        MOV     AX, SS:DataPoint
        MOV     SS:FirstPoint, AX

; mlm     PUSH    WORD PTR PrefBuf
        PUSH    WORD PTR SS:PrefBuf[2]   ; mlm
        PUSH    WORD PTR SS:PrefBuf      ; mlm
        LEA     AX, WORD PTR FirstPoint
        PUSH    SS
        PUSH    ax
        PUSH    WORD PTR SS:CIndex
        PUSH    WORD PTR SS:EIndex
        PUSH    WORD PTR SS:pCurveDir[2]
        PUSH    WORD PTR SS:pCurveDir
        CALL    FAR PTR LoadCurveBuffer
; mlm     ADD     SP,14
        ADD     SP,16                     ; mlm

        XOR     BX, BX
; mlm        MOV     CX, WORD PTR SS:PrefBuf
        PUSH    ES                        ; mlm
        LES     DI, PrefBuf               ; mlm
        MOV     CX, ES:[DI]               ; mlm
        POP     ES                        ; mlm
        INC     CL
GET_CVBUF_OFFSET:
        ADD     BX, SIZE CURVEBUFFER
        LOOP    GET_CVBUF_OFFSET

        SUB     BX, SIZE CURVEBUFFER
        CMP     AX,0                    ; check for error
        JZ      ALREADY_LOADED
        JMP     GDP_RETURN_BAD

ALREADY_LOADED:
;   DataSz = CvBuf[PrefBuf].Curvehdr.DataType & 0x0F;
;   TmpPtr = (PVOID)( (ULONG) CvBuf[PrefBuf].BufPtr +
;                      ((ULONG) (DataPoint - FirstPoint) * (ULONG) DataSz) );
; X is always a 4 byte float.  Do the normal Y stuff but save
; some values for use in finding the X value.  Y is found first, then x

        MOV     DL, BYTE PTR ES:CvBuf[BX].cb_Curvehdr.DataType
        MOV     CL, DL                 ; save full buffer data type in case 
                                       ; of different types
        MOV     CH, BYTE PTR SS:DType  ; save full needed data type in case 
                                       ; of different types
ASSUME  DS : NOTHING

        ; Swap DS and ES
        PUSH    ES
        MOV     AX, DS
        MOV     ES, AX
        POP     DS

        MOV     AX, WORD PTR DS:CvBuf[BX].BufPtr

        ; save for finding X later
        PUSH    AX

        MOV     SI, ES:DataPoint
        SUB     SI, WORD PTR DS:CvBuf[BX].BufferOffset

        ; save for finding X later 
        PUSH    SI  ; ( DataPoint - FirstPoint )     
        
        AND     DL, 0FH                 ; get the data size
        MOV     DH, DL                  ; save it for later
        SHR     DL, 1                   ; data sizes are 1, 2, 4, and 8

        ; calculate the data offset from the start of the buffer
FIND_Y_PT:                     
        JZ      Y_PT_FOUND
        SHL     SI, 1
        SHR     DL, 1                   ; data sizes are 1, 2, 4, and 8
        AND     DL, DL                  ; test to see if done
        JMP     FIND_Y_PT
        
Y_PT_FOUND:
        ; have Y point's offset fron the start of the data buffer in SI
        ADD     SI, AX          ; get offset from start of segment
        MOV     AX, WORD PTR DS:CvBuf[BX].BufPtr[2] ; get the segment

        ; save segment for finding X later
        PUSH    AX 
        
        MOV     DS, AX
        LES     DI, SS:pY

        CMP     CL, CH
        JNZ     NOT_SAME_TYPE
;
;   if (DataType == (CHAR) CvBuf[PrefBuf].Curvehdr.DataType )
;      {
;      memmove( pY, TmpPtr, DataSz );

        ; same types
        MOV     CL, DH
        XOR     CH, CH
        REP MOVSB       ; copy the Y data

;       jump to the code to get X
        JMP SHORT GET_X_VAL
;      }
;   else
;      {

NOT_SAME_TYPE:

;      ConvertTypes( TmpPtr, CvBuf[PrefBuf].Curvehdr.DataType,
;                    pY, DataType );

        MOV     AL, CH          ; needed data type
        XOR     AH, AH
        XOR     CH, CH

        PUSH    AX              ; Needed DataType
        PUSH    ES
        PUSH    DI              ; Push pY
        PUSH    CX              ; Buffer Data Type
        PUSH    DS
        PUSH    SI              ; Push buffer's Y data location
        CALL    FAR PTR ConvertTypes
        ADD     SP, 12

GET_X_VAL:

        ; x is always a 4 byte float, copy its value
        POP     DS         ; the segment -- WORD PTR DS : CvBuf[BX].BufPtr[2]
        POP     SI         ; DataPoint - FirstPoint
                           ; always 4 bytes, so multiply by 4 to get offset
        SHL     SI, 2      ; shift left 2 same as multiply by 4
        POP     AX  ;  WORD PTR DS : CvBuf[BX].BufPtr
        ADD     SI, AX     ; offset from start of segment
                           ; add in BUFLEN / 2 to get to x data
        ADD     SI, BUF_X_OFFSET ;
        LES     DI, SS:pX
        MOV     CX,4
        REP     MOVSB       ; copy 4 bytes

        JMP     SHORT GDP_RETURN_GOOD

;      TmpPtr = (PVOID)( (ULONG) TmpPtr + (BUFLEN / 2) );
;
;   return ERROR_NONE;
;}

GDP_RETURN_BAD:
        RET                             ; error number is in AX

GDP_RETURN_GOOD:
        XOR     AX, AX                  ; clear AX for entry return
        RET

GetDataPoint ENDP

END
