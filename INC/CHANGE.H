// FILE change.h

#ifndef CHANGE_INCLUDED
#define CHANGE_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern char ch_input_fname[];
extern char ch_output_fname[];
extern SHORT  ch_output_format;
extern USHORT  from_field;
extern USHORT  count_field;

int change_execute(void *, char *);
int ChangeVerifyCurveBlk(void);

#define     CH_FILESIZE         50
#define     XTEMPBUF_SZ         255

enum translate_types {
  TRANS_OMA1460,      /* 0  */
  TRANS_OMA1470,      /* 1  these constant values must correspond   */
  TRANS_SINGLE,       /* 2  to the offest values in the character   */
  TRANS_SINGLE_X,     /* 3  pointer array "ch_output_options"   i.e.*/
  TRANS_MULTI,        /* 4  char *ch_output_options[], which is     */
  TRANS_MULTI_X,      /* 5  located in the omaform1 source module  */
  TRANS_TCL32,
  TRANS_TIFF };

struct oma1460_type
{
   int        detector_type;
   int        detector_length;
   int        detector_temperature;
   float      exposure_time;
   float      minimum_exposure_time;
   int        synchronization_mode; /* 0=normal 1=line 2=external */
   int        line_frequency;
   int        data_mode;    /* 0=none 1=live 2=accum */
   int        DA_mode_number;
   int        number_of_scans;
   int        number_of_memories;
   int        ignore_scans;
   int        data_type;
   int        points;
   float      data_start;   /* 32 bit address of data start for header */
   int        scan_mode;    /* 0=single region 1=mixed */
   int        number_first_pixel;
   int        number_active_pixels;
   int        group_size;
   int        normalization_mode;
   int        disk_drive;
   int        file_number;
   int        pulser_used;
   int        number_of_pulses;
   int        delay_coefficient;
   int        delay_exponent;
   int        width_coefficient;
   int        width_exponent;
   int        delay_1302;
   int        delay_1303;
   int        width_1303;
   int        delay_increment_1303;
   int        clock_cycles;
   int        predelay_cycles;
   int        calibration;
   int        degree_of_xfit;
   float      a0,a1,a2,a3;
};

#define MAXNAME          12
#define MAXEQULEN        58
#define MAXTITLE         27   /* Max length string in several places */
#define NUMBER_EQUATIONS 8
#define AREA_COUNT       12
#define LABEL_SZ1470     82
#define R_BUFS           100
typedef struct queue_type
{
   struct queue_type  *next_queue;
   int                op;
   union
   {
      double      num_op;
      int         index;
      int         function_index;
      struct
      {
         int     memory_index;
         int     file_index;
         float   *data;
      } mem;
      struct
      {
         unsigned char    Nindex;
         float            Ntime;
         float            Nvalue;
      } norm;
   } operator;
} QUEUE, *PQUEUE;

typedef struct area_elem_type
{
   float          lower_lam,
                  upper_lam;   /* Lower and upper wavelengths of the area.*/
   char           calc_mode;   /* mode of correction */
   char           corr_area;   /* number of correction area for SUB_AREA */
   char           valid_area;  /* True if wavelength range ok, False if not.*/
   char           comment[MAXTITLE+1];   /* comment/title */
} AREA_ELEM, *PAREA_ELEM;
  
typedef struct equation_elem_type
{
   char           equation_text[ MAXEQULEN ];
   int            valid;
   PQUEUE         queue;
   double         ymin,ymax;
} EQUATION_ELEM, *PEQUATION_ELEM;
  
typedef struct curvetype_type
{
   float                     *spectral_data;
   struct curvetype_type     *next_curve;
   int                       YT_point;
} CURVETYPE, *PCURVETYPE;
  
typedef struct polyline_type
{
   unsigned char     xaxis;   /* type units for x, pixel, nm,.... */
   int               n;       /* degree of the polynomial, 2=linear,4=cubic */
   double            a[4];    /* a0,a1,a2,a3 */
} POLYTYPE, *PPOLYTYPE;
  
typedef struct yt_strip_type
{
   float                     *YT_data;
   struct yt_strip_type      *next_strip;
} YTSTRIP, *PYTSTRIP;
  
typedef struct oma1470_curveset
{
   int              channel_min;
   int              channel_max;
   int              number_of_memories;
   AREA_ELEM        areas[ AREA_COUNT ];
   EQUATION_ELEM    equations[ NUMBER_EQUATIONS ];
   char             datafile[ MAXNAME +1 ];
   char             crxnfile[ MAXNAME +1 ];
   char             dadfile [ MAXNAME +1 ];
   char             sparechar[MAXTITLE+2-MAXNAME-MAXNAME]; 

   PCURVETYPE       curves;
   int              detector_type;           /* 1462/1463/1216/1218 */
   int              detector_length;         /* 512/1024 */
   int              detector_temperature;
   double           exposure_time;
   double           scan_time;
   double           minimum_exposure_time;
   unsigned char    synchronization_mode; /* 0=none 1=line 2=external */
   unsigned char    line_frequency;       /* 50/60 */
   unsigned char    cooler_flag;      /* true = cooler locked, false otherwise */
   unsigned char    DA_mode_number;
   int              number_of_scans;
   int              number_of_memories_again;
   int              number_of_ignore_scans;
   unsigned char    datatype;    /* 8=int, 10=float, 12=long */
   unsigned char    scan_mode;   /* 0=single region 1=mixed */
   int              number_active_pixels;
   int              groupsize_norm_scan;
   unsigned char    data_norm_mode;
   int              max_mem;
   unsigned char    ADP;    /* A/D precision 1=14 bit */
   unsigned char    DP;     /* 0=single, 1=double */
   int              detector;      /* If config has one, otherwise -na- */
   int              spectrograph;  /* if config has one, otherwise ditto*/
   int              grating;       /* if 1235, grating number */
   int              pulser_type;   /* 0/1211/1302 */
   int              first_curve;   /* if loaded from starting mem 6,
                                    * would be 6... needed for external
                                    * data sets among others*/
   char             y_axis;   /* Type of correction applied to y values:
                               *  0: no correction; units Counts.
                               *  1: adj for ET; units Counts/Sec.
                               *  0X10: Radiometric corrections;
                               *        units uWatts/nm.cm2 .
                               *  0X11: ?????? .... 0X1F;
                               *  0X20: Spectroscopic corrections;
                               *        units % Transmission.
                               *  0X21: units Absorbance.
                               *  0X22: units Reflection.
                               *  0X30: Fingerprint functions;
                               *        label Reference.
                               *  0X31: label Sample.
                               *  0X32: label Diff.
                               */
   char   same_ET;            /* Value of same.et for ver 4.2 of firmware */
   char   shutter;            /* 1 if user has selected shutter for bg. */
   char   one_shot;           /* 1 if pulser in one-shot mode; 0 if multi. */
   int    spare_int;
   float  delay_1303;         /* Delay for 1303 in ns */
   float  width_1303;         /* Width for 1303 in ns */
   float  delay_increment_1303;        /* Delay Increment 1303 */
   float  dummy_floats[3];    /* dummies..    */
   float  predelay_time;      /* Yt PreDelay time in seconds */
   POLYTYPE         calib_data;
   int        spect_mode;
   int        equation_index[8];
   float      YT_time;
   int        YT_points;
   unsigned char valid_equation_num;  /* number of equations defined in Yt */
   int        YT_equations[NUMBER_EQUATIONS];  /* list of equations */
   PYTSTRIP   YT_strip_ptr;
   float      r_buffer [R_BUFS];  /* The R buffer array for results. */
} OMA1470_CURVESET, *POMA1470_CURVESET;
  
typedef struct oma1470_type
{
   char     file_id_string[14];
   char     label[LABEL_SZ1470];
   OMA1470_CURVESET     curveset;
} OMA1470_REC, *POMA1470_REC;

#endif  //  CHANGE_INCLUDED
