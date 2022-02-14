;/***************************************************************************/ 
;/*  OMATYP.H                                                               */ 
;/*                                                                         */ 
;/*      Type definitions for the OMA35 program.                            */ 
;/*                                                                         */ 
;/*  last changed:                                                          */  
;/*  Adapted from OMA4TYPE.H                                                */ 
;/*          9/29/89     DI  Changed XArray from PFLOAT to PVOID            */
;/*          6/1/89      DI                                                 */ 
;/*                                                                         */ 
;/***************************************************************************/ 


DOSFILESIZE             EQU  12      ; handy.h
DOSPATHSIZE             EQU  64      ; handy.h
FNAME_LENGTH            EQU  DOSFILESIZE + DOSPATHSIZE + 1 
DESCRIPTION_LENGTH      EQU  81 
FTIDLEN                 EQU  40 
 

MET_PLOT STRUC
        PTitle          DB   30 DUP (?)
        XLegend         DB   30 DUP (?)    
        PXMax           DD   ?
        PXMin           DD   ?
        XAscending      DW   ?
        PXUnits         DW   ?
        YLegend         DB   30 DUP (?)
        PYMax           DD   ?
        PYMin           DD   ?
        YAscending      DW   ?
        PYUnits         DW   ?
        ZLegend         DB   30 DUP (?)
        PZMax           DD   ?
        PZMin           DD   ?
        ZAscending      DW   ?
        PZUnits         DW   ?
        XZPercent       DW   ?
        YZPercent       DW   ?
        ZPosition       DW   ?
        Style           DW   ?
        PlotPeakLabels  DW   ?       
MET_PLOT ENDS


;#define HDR_BASE_LENGTH  FIELDOFFSET( METHDR, DetInfo ) 

METHDR STRUC                                    ; major revision 1/31/91 DAI
       FileType          DB FTIDLEN DUP (?)
       StructureVersion  DB ?
       mh_length         DW ?
       User              DB ?
       Description       DB DESCRIPTION_LENGTH DUP (?)
       FileCurvenum      DW ?
       InterfaceType     DB ?
       ActiveDetector    DB ?
       DADName           DB FNAME_LENGTH DUP (?)
       Normalize         DB ?
       SpectrographUnits DB ?
       Excitation        DD ?
       SpectrographSet   DD 4 DUP (?)
       SpectrographInc   DD 4 DUP (?)
       Slit              DD 16 DUP (?)
       CalibCoeff        DD 3 DUP (4 DUP (?))
       CalibUnits        DB 3 DUP (?)
       BackgroundName    DB FNAME_LENGTH DUP (?)
       I0Name            DB FNAME_LENGTH DUP (?)
       InputName         DB FNAME_LENGTH DUP (?)
       OutputName        DB FNAME_LENGTH DUP (?)
       YTInterval        DD ?
       YTPredelay        DD ?
       mh_pia            DW 2 DUP (?)
       SoftwareVersion   DW ?
       PlotWindowIndex   DW ?
       AutoScaleX        DW ?
       AutoScaleY        DW ?
       AutoScaleZ        DW ?
       WindowPlotSetups  DW 8 DUP (?)
       PlotInfo          DB 8 DUP (SIZE MET_PLOT DUP (?))
       reserved          DB 100 DUP (?)
       DetNumber         DB ?
       DetInfo           DD ?       ; pointer
METHDR ENDS
 
;   /* Define the maximum number of segments a curve's data may take up. 
;   The value is 64K (max number of points) * maximum data type size */ 

       MAXDATASEGS EQU   8

XDATA STRUC
       XUnits            DB ?
       XArray            DD ?       ; pointer 
XDATA ENDS

 
;/* ADD BYTES, IF NEEDED, TO MAKE A CURVEHDR AN EVEN NUMBER IN SIZE !!! */ 
CURVEHDR STRUC
        pointnum       DW ?
        ch_XData       DB SIZE XDATA DUP (?)
        YUnits         DB ?
        DataType       DW ?
        experiment_num DW ?
        ch_time        DD ?         ; float
        scomp          DD ?         ; u long
        ch_pia         DW 2 DUP (?)
        Frame          DW ?
        Track          DW ?         ; 1/31/91 DAI
        Ymin           DD ?         ; float
        Ymax           DD ?         ; float
        Xmin           DD ?         ; float
        Xmax           DD ?         ; float
        MemData        DW ?         ; 3/7/91 DAI
        ch_CurveCount  DW ?         ; 3/7/91 DAI
CURVEHDR ENDS


;CURVE STRUC
;        cv_curvehdr    DB  SIZE CURVEHDR DUP (?)
;        DataStart      DD  ?       ; float
;CURVE ENDS
 
 
CURVE_ENTRY STRUC
        ce_name        DB  (DOSFILESIZE+1) DUP (?)
        path           DB  (DOSPATHSIZE+1) DUP (?)
        desc           DB  (DESCRIPTION_LENGTH) DUP (?)   ; array
        StartIndex     DW  ?
        StartOffset    DD  ?       ; ulong
        TmpOffset      DD  ?       ; ulong
        count          DW  ?
;       ce_time        DB  SIZE TM DUP (?) ; ??????????????
        ce_time        DW  9 DUP (?) ; time.h
        EntryType      DB  ?
        Dirty          DB  ?
        DisplayWindow  DW  ?
CURVE_ENTRY ENDS 


CURVEDIR STRUC
        Entries        DD  ?       ; pointer
        BlkCount       DW  ?
        CurveCount     DW  ?
CURVEDIR ENDS 

 
CURVEBUFFER STRUC
        BufPtr         DD  ?       ; pointer
        ActiveDir      DD  ?       ; pointer
        Entry          DW  ?
        CurveIndex     DW  ?
        BufferOffset   DW  ?
        cb_Curvehdr    DB  SIZE CURVEHDR DUP (?)
        status         DB  ?
CURVEBUFFER ENDS

BUFNUM                 EQU 3               
BUFPOINTS              EQU 4096            
BUFLEN                 EQU (BUFPOINTS + 1) * 2 * 4
BUF_X_OFFSET           EQU BUFLEN / 2


;typedef struct 
;   { 
;   USHORT Index;  /* index of entry */ 
;   USHORT Start;  /* starting curve, may be different from starting curve */ 
;                  /* of entry */ 
;   USHORT Count;  /* curve count, may be different from curve count */ 
;                  /* of entry */ 
;   }INDEXEDENTRY; 
;typedef INDEXEDENTRY far *LPINDEXEDENTRY; 
; 
; 
;/* information for live acquisition data placement */ 
;typedef struct 
;   { 
;   LPCURVEDIR CurveDir; /* curve directory for live data */ 
;   SHORT GroupSz;       /* number of curves to transfer at a time */ 
;   SHORT TotalCurves;   /* total number of curves held in buffer */ 
;   BOOL replace;        /* replace previous curves in buffer */ 
;   } ACQPARMS; 
;typedef ACQPARMS FAR *LPACQPARMS;
; 
;typedef struct 
;   { 
;   PVOID BufPtr;           /* pointer to the start of the buffer */ 
;   LPCURVEDIR ActiveDir;   /* buffered curve's CURVEDIR */ 
;   SHORT Entry;            /* buffered curve's curve block entry index */ 
;   USHORT CurveIndex;      /* Curve's file number */ 
;   USHORT BufferOffset;    /* Buffer's starting point number offset into */ 
;                           /* curve */ 
;   CURVEHDR Curvehdr;      /* curve header for this curve */ 
;   }CURVEBUFFER; 
;

