/***************************************************************************/
/*  EGGTYPE.H                                                             */
/*                                                                         */
/*      Basic types, opaque types, and constants                           */
/*                                                                         
*  $Header: /OMA4000/Include/EGGTYPE.H 1     6/18/99 1:52a Maynard $
*  $Log: /OMA4000/Include/EGGTYPE.H $
 * 
 * 1     6/18/99 1:52a Maynard
*  
*     Rev 1.2   09 Aug 1995 11:34:38   MLM
*  Modify VCS header
*  
*     Rev 1.1 16 Jul 1994 
*  Add things necessary to compile for Windows
*
*     Rev 1.0   07 Jan 1992 11:51:46   cole
*  Initial revision.
*/
/***************************************************************************/

#ifndef EGGTYPE_INCLUDED
#define EGGTYPE_INCLUDED

#ifdef _WINDOWS
  #ifndef _WINOMA_
    #define _WINOMA_
  #endif
#endif
  
#if !defined(FALSE)
  enum { FALSE = 0, TRUE = 1 };
#endif
  
typedef unsigned short BOOLEAN;

#if defined(USE_D16M) || defined(__WATCOMC__) || defined(_WINOMA_)
#define _PROTECTED_MODE
#endif

#define NIL 0L

#ifndef _WINDOWS
  #ifdef __WATCOMC__
    #define FAR
    #define __huge
  #else
    #define FAR  _far
    #define __far _far
    #define __huge _huge
  #endif
  #define VOID void
#else
  #define FAR  _far
#endif

#ifndef __WATCOMC__  /* for "16-bit" compilers */
  
typedef char CHAR;
typedef CHAR far *PCHAR;
typedef unsigned char UCHAR;
typedef UCHAR far *PUCHAR;
typedef signed short SHORT;
typedef SHORT far *PSHORT;
typedef unsigned short USHORT;
typedef USHORT far *PUSHORT;
#ifndef _WINOMA_
typedef long LONG;
typedef LONG far *PLONG;
#endif
typedef unsigned long ULONG;
typedef ULONG far *PULONG;
  
typedef unsigned char BYTE;
  
#ifndef _WINOMA_
typedef VOID FAR *PVOID;
typedef  long FIXED;
#endif
  
typedef float FLOAT;
typedef FLOAT far *PFLOAT;
typedef double DOUBLE;
typedef DOUBLE far *PDOUBLE;
typedef unsigned int BSHORT;  
  
typedef int ERR_OMA ;  /* for error codes for error reporting */

#else    /* for the 32-bit world... */

#ifndef PLTYPES      /* unless PharLap extender header included */
typedef char CHAR;
typedef CHAR *PCHAR;
typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
typedef short int SHORT;
typedef SHORT *PSHORT;
typedef unsigned short int USHORT;
typedef USHORT *PUSHORT;
typedef int LONG;
typedef LONG *PLONG;
typedef unsigned int ULONG;
typedef ULONG *PULONG;
#endif /* PLTYPES */

typedef unsigned int BSHORT;  
typedef unsigned char BYTE;
typedef VOID FAR *PVOID;
typedef int FIXED;
typedef float FLOAT;
typedef FLOAT FAR *PFLOAT;
typedef double DOUBLE;
typedef DOUBLE FAR *PDOUBLE;
  
typedef short int ERR_OMA ;  /* for error codes for error reporting */

#endif  /* __WATCOMC__ */

enum { ERROR_NONE = 0 } ;     /* zero means no error */

/* basic typedefs for structures, OPAQUE types only !! */

#ifndef __TURBOC__
typedef struct curve_entry CURVE_ENTRY ;  /* CRVENTRY.H */
typedef CURVE_ENTRY FAR * LPCURVE_ENTRY ;

typedef struct curve_directory CURVEDIR ; /* CURVEDIR.H */
typedef CURVEDIR FAR * LPCURVEDIR ;

typedef struct method_header METHDR ;   /* OMAMETH.H */
typedef METHDR * PMETHDR;
typedef METHDR FAR * LPMETHDR;

typedef struct curve_buffer CURVEBUFFER ;  /* CURVEBUFFER.H */

typedef struct X_data XDATA ;   /* CRVHEADR.H */
typedef XDATA *PXDATA;

typedef struct curve_header CURVEHDR ;  /* CRVHEADR.H */
typedef CURVEHDR *PCURVEHDR;

typedef struct axis_data AXISDATA ;     /* PLOTBOX.H */
typedef struct curve_peak_label CURVE_PEAK_LABEL ;
typedef struct plot_peak_label PLOT_PEAK_LABEL ;
typedef struct plot_box PLOTBOX ;
typedef struct detector_info DET_INFO ;  /* DETINFO.H */
typedef struct op_block OP_BLOCK;
typedef struct stats STAT_STRUCT;
#endif  /* TURBOC */

#endif  /* EGGTYPE_INCLUDED */
