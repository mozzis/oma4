/* -----------------------------------------------------------------------
/
/  points.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/POINTS.H 1     6/18/99 1:53a Maynard $
/
 *    Rev 1.0   09 Sep 1991 14:30:18   cole
 * Initial revision.
 * 
/
*/
  
#ifndef POINTS_INCLUDED
#define POINTS_INCLUDED
  
#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

// type of function used to retrieve a data point from a curve set.  
// baseline subtract uses special ones, which are needed to make
// autoscale function correctly
typedef ERR_OMA GET_POINT_FUNC(LPCURVEDIR,  /* directory */
                                      SHORT,       /* Entry Index */
                                      USHORT,      /* Curve Index */
                                      USHORT,      /* Point Index */
                                      PFLOAT,      /* X Value */
                                      PVOID,       /* Y Value */
                                      CHAR,        /* data type */
                                      SHORT*);     /* pref Buf */

/* return pointer to function to get a point for plotting */
GET_POINT_FUNC * WhichGetFunction(void);

/* set the system get_data function to some special purpose routine */
BOOLEAN SetGetFunction(GET_POINT_FUNC * GetFunc);

// X value always a float
GET_POINT_FUNC GetDataPoint;

// X value always a float
GET_POINT_FUNC SetDataPoint;

//extern GET_POINT_FUNC * get_point;
  
ERR_OMA GetTempCurvePointIndex(LPCURVEDIR, USHORT, USHORT, PFLOAT,
                                      PFLOAT, UCHAR, PUSHORT, SHORT);

#endif   // POINTS_INCLUDED
