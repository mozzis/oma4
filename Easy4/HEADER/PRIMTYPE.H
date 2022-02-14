/***************************************************************************/
/*  PRIMTYPE.H                                                             */
/*                                                                         */
/*      Basic types, opaque types, and constants
/*
/   $Header:   J:/logfiles/oma4000/inc/primtype.h_v   1.1   12 Mar 1992 15:11:52   maynard  $
/   $Log:   J:/logfiles/oma4000/inc/primtype.h_v  $     
 * 
 *    Rev 1.1   12 Mar 1992 15:11:52   maynard
 * Don't define structs which will not be used in TURBOC version of driver
 * (uses TURBOC macro to determine if driver is being compiled)
 * 
 *    Rev 1.0   07 Jan 1992 11:51:46   cole
 * Initial revision.
/
/***************************************************************************/
  
#ifndef PRIMTYPE_INCLUDED
#define PRIMTYPE_INCLUDED
  
#ifndef FORMTYPE_INCLUDED

#define NOT !
  
enum { FALSE = 0, TRUE = 1 } ;
  
typedef unsigned short BOOLEAN;
#endif

#define NIL 0L
  
#define FAR     far
#define NEAR    near
#define VOID    void
  
typedef char CHAR;
typedef CHAR far *PCHAR;
typedef unsigned char UCHAR;
typedef UCHAR far *PUCHAR;
typedef int SHORT;
typedef SHORT far *PSHORT;
typedef unsigned USHORT;
typedef USHORT far *PUSHORT;
typedef long LONG;
typedef LONG far *PLONG;
typedef unsigned long ULONG;
typedef ULONG far *PULONG;
  
typedef unsigned char BYTE;
  
typedef VOID   FAR *PVOID;
  
typedef  long FIXED;
  
typedef float FLOAT;
typedef FLOAT far *PFLOAT;
typedef double DOUBLE;
typedef DOUBLE far *PDOUBLE;
  
typedef int ERROR_CATEGORY ;  /* for error codes for error reporting */

enum { ERROR_NONE = 0 } ;     /* zero means no error */

// basic typedefs for structures, OPAQUE types only !!

#ifndef TURBOC
typedef struct curve_entry CURVE_ENTRY ;  /* CRVENTRY.H */
typedef CURVE_ENTRY far * LPCURVE_ENTRY ;

typedef struct curve_directory CURVEDIR ; /* CURVEDIR.H */
typedef CURVEDIR far * LPCURVEDIR ;

typedef struct method_header METHDR ;   /* OMAMETH.H */
typedef METHDR * PMETHDR;
typedef METHDR far * LPMETHDR;

typedef struct curve_buffer CURVEBUFFER ;  /* CURVEBUFFER.H */

typedef struct X_data XDATA ;   /* CRVHEADR.H */
typedef XDATA *PXDATA;
typedef XDATA FAR *LPXDATA;

typedef struct curve_header CURVEHDR ;  /* CRVHEADR.H */
typedef CURVEHDR *PCURVEHDR;
typedef CURVEHDR FAR *LPCURVEHDR;

typedef struct axis_data AXISDATA ;     /* PLOTBOX.H */
typedef struct curve_peak_label CURVE_PEAK_LABEL ;
typedef struct plot_peak_label PLOT_PEAK_LABEL ;
typedef struct plot_box PLOTBOX ;

typedef struct detector_info DET_INFO ;  /* DETINFO.H */
#endif

#endif         /* PRIMTYPE_INCLUDED */
