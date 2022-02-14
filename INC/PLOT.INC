;/* -----------------------------------------------------------------------
;/
;/   plot.h
;/
;/  Copyright (c) 1988,  EG&G Princeton Applied Research
;/
;/  Written by: TLB      Version 1.00          8 March       1988
;/  Worked on:  TLB      Version 1.01          9 June        1988
;/              TLB      Version 1.02          1 September   1988
;/              TLB      Version 1.03          9 January     1989
;/              DAI                           16 October     1989
;/                       took out things defined in OMA35.h
;/                       fixed prototype of array_hplot
;/              DAI                            6 December    1989
;/                       added units to axis structure
;/
;/  
;/  plot.h defines data structures for plotting.
;/
;/ ----------------------------------------------------------------------- */

;#define     LINE_WIDTH           PlotDevParams->min_polyline_width //75

POINT_BATCH_SIZE  	EQU	100

MAX_LABELLED_CURVES     EQU     10

NOSIDE 			EQU	0
RIGHTSIDE		EQU	1
LEFTSIDE 		EQU	2

DESCARTES STRUC
	xp			DW	?
	yp			DW	?
DESCARTES ENDS

CURVE_PEAK_LABEL STRUC
        curve_number            DD      ?       ; float
        threshold_value         DD      ?       ; float
        enabled                 DW      ?       ; boolean
CURVE_PEAK_LABEL ENDS

PLOT_PEAK_LABEL STRUC
        curve_peaks             DB MAX_LABELLED_CURVES DUP (SIZE CURVE_PEAK_LABEL DUP (?) )
        label_peaks             DW      ?       ; boolean
PLOT_PEAK_LABEL ENDS

AXISDATA STRUC
	legend			DB	30 DUP (?)
	axis_end_offset		DB	SIZE DESCARTES DUP (?)
	axis_zero		DB	SIZE DESCARTES DUP (?)
	max_value		DD	?	; float
	min_value		DD	?	; float
	original_max_value	DD	?	; float
	original_min_value	DD	?	; float
	inv_range		DD	?	; float
	ascending		DW	?	; boolean
	units			DW	?
AXISDATA ENDS

PLOTBOX STRUC

;    there used to be a CDVHANDLE device handle here but it is no longer
;    used.  Keep a spare so that method headers will still have the same
;    format/size ( maintain compatibility with old method header files ).
;    Is this really necessary ?

        plotboxSpareOne         DW      ?

        pb_title		DB	30 DUP (?)
	xscale			DD	?	; float
	yscale			DD	?	; float
	fullarea		DB	2 DUP (SIZE DESCARTES DUP (?))
	plotarea		DB	2 DUP (SIZE DESCARTES DUP (?))
	x			DB	SIZE AXISDATA DUP (?)
	y			DB	SIZE AXISDATA DUP (?)
	z			DB	SIZE AXISDATA DUP (?)
	xz_percent		DW	?
	yz_percent		DW	?
	z_position		DW	?  ; LEFTSIDE, RIGHTSIDE, or NOSIDE 
	background_color	DW	?
	box_color		DW	?
	text_color		DW	?
	grid_color		DW	?
	grid_line_type		DW	?
	plot_color		DW	?
	plot_line_type		DW	?
	style			DW	?
        plot_peak_labels        DB      SIZE PLOT_PEAK_LABEL DUP (?)
        colorKeyWidth           DW      ?
PLOTBOX ENDS


;typedef struct
;{
;   int      input_sources;
;   float *  x_array;
;   float *  y_array;
;   int      x_count;
;   int      y_count;
;   float    x_start;
;   float    x_step;
;   float    y_start;    /* y ramp not yet supported... */
;   float    y_step;     /* y ramp not yet supported... */
;} PLOTDATA;
;
;typedef struct 
;{
;   PLOTBOX     box;
;   PLOTDATA    data;
;} PLOTCONTEXT;
;
;typedef struct _SIZEF {       /* sizfx */
;    FIXED cx;
;    FIXED cy;
;} SIZEF;
;
;typedef struct _GRADIENTL {     /* gradl */
;    LONG x;
;    LONG y;
;} GRADIENTL;
;
;typedef struct 
;{
;  unsigned color; 
;  float    data_min; 
;  float    data_max; 
;} MAPEL,*PMAPEL;
;
;typedef struct 
;{
;   unsigned    num_of_colors;
;   int         default_color;
;   PMAPEL      color_index;
;} COLOR_STRUCT,*PCOLOR_STRUCT;
;
;#define FIXED_1 0x10000L
;
;#define MAXTEXT 75
;#define TICK_LENGTH_DIVISOR 6

