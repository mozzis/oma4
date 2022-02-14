/***************************************************************************/
/*  crvheadr.h                                                             */
/*                                                                         */
/*  copyright (c) 1989, EG&G Instruments Inc.                              */
/*                                                                         */
/***************************************************************************/
  
#ifndef CRVHEADR_INCLUDED
#define CRVHEADR_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

// data units for XUnits field of XDATA struct, YUnits field of CURVEHDR
// struct, XUnits and YUnits and ZUnits fields in MET_PLOT struct.
enum {COUNTS      =  0, 
      ANGSTROM    =  1, 
      NM          =  2, // nanometers
      UM          =  3, // micrometers
      MM          =  4, // millimeters
      CM          =  5, // centimeters
      METER       =  6, 
      WAVENUMBER  =  7,
      RSHIFT      =  8, // raman shift
      EV          =  9, // electron volts
      JOULE       = 10, 
      ERG         = 11, 
      HZ          = 12,
      ADJ_NM      = 13, 
      WLEN_UNITNUM = 14, 
      SECOND      = 20,
      MSEC        = 21, // millisecond
      USEC        = 22, // microsecond
      NSEC        = 23, // nanoseconds
      PSEC        = 24, // picoseconds
      FSEC        = 25, // femtoseconds
      TIME_DIS_UNITNUM = 26,

      AMP         = 30, 
      VOLT        = 31, 
      OHM         = 32, 
      COULOMB     = 33, 
      MHO         = 34,
      WATT        = 35
     } ; 

struct X_data
{
  UCHAR  XUnits;          /* X axis units for this curve, see above    */
                          /* for values.                               */
  PVOID  XArray;          /* X axis value array. In a file this value  */
                          /* is meaningless, the X values will follow  */
                          /* this header.  In memory curves, more than */
                          /* one array may be used by changing this    */
                          /* and the XUnits value in the curve header. */
} ;

typedef struct X_data XDATA ;
typedef XDATA *PXDATA;
typedef XDATA FAR *LPXDATA;

/* DATA TYPE DEFINITIONS for the DataType field in struct CURVEHDR */
// uses bit flags:   0x10 to signify signed type
//                     0x20 to signify floating point type
// the lower nybble gives the number of bytes for the data type */
#define FLOATTYPE    0x34
#define UCHARTYPE    1
#define SHORTTYPE    0x12
#define USHORTTYPE   2
#define LONGTYPE     0x14
#define ULONGTYPE    4
#define DOUBLETYPE   0x38
/* no size types */
#define POINTERTYPE  0x41

/* ADD BYTES, IF NEEDED, TO MAKE CURVEHDR AN EVEN NUMBER SIZE !!! */
struct curve_header
{
  USHORT pointnum;          /* number of points in this curve */
  XDATA  XData;             /* X axis units and value array pointer */
  UCHAR  YUnits;            /* Y axis units for this curve */

  USHORT DataType;          /* data type (int, float, etc. */

  USHORT experiment_num;    /* related experiment from the DAD file */
                            /* unused in OMA35 software */
  FLOAT  time;              /* Time in seconds from start of scan */
                            /* Or may be used for Z axis value */
  ULONG  scomp;
  USHORT pia[2];            /* unused in OMA35 software */
  USHORT Frame;
  USHORT Track;
  FLOAT  Ymin, Ymax;        /* min and max amplitude of points in curve */
  FLOAT  Xmin, Xmax;         
  BOOLEAN MemData;          /* TRUE if data is in (OMA4) memory */
  USHORT CurveCount;        /* number of curves for this header */
} ;                  

#define CRVHDRLEN sizeof(CURVEHDR)

/* typedef struct curve_header CURVEHDR;
   typedef CURVEHDR     *PCURVEHDR;
   typedef CURVEHDR far *LPCURVEHDR;     */ 


#endif   // CRVHEADR_INCLUDED
