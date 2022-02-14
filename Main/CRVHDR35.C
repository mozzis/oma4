/*************************************************************************
*                                                                        *
*  crvhdr35.c                                                            *
*  copyright (c) 1989, EG&G Instruments Inc.                             *
*  Read old style oma35 curve headers into oma4000 style curve header.   *
*                                                                        *
*  $Header:   J:/logfiles/oma4000/main/crvhdr35.c_v   1.0   07 Jan 1992 11:53:36   cole  $
*  $Log:   J:/logfiles/oma4000/main/crvhdr35.c_v  $
* 
*    Rev 1.0   07 Jan 1992 11:53:36   cole
* Initial revision.
*
**************************************************************************/

#include "crvhdr35.h"
#include "syserror.h"  // ERROR_READ
#include "crvheadr.h"
  
#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

// old curve header format from oma35, still have to be able to read an
// oma35 type file with this type of curve header in it.
// CURVEHDR1 stuff copied from oma35typ.h

typedef struct
{
  USHORT  pointnum;       /* number of points in this curve */
  XDATA   XData;          /* X axis units and value array pointer */
  UCHAR   YUnits;         /* Y axis units for this curve, see OMA35.H */
                          /* for values */
  USHORT  DataType;       /* data type (int, float, etc. */
                          /* See OMA35.H for DATA TYPE DEFINITIONS */
  USHORT  experiment_num; /* related experiment from the DAD file */
                          /* unused in OMA35 software */
  FLOAT   time;           /* Time in seconds from start of scan */
                          /* May be alternately be used for Z axis value */
  ULONG   scomp;          
  USHORT  pia[2];         /* unused in OMA35 software */

  FLOAT   Ymin, Ymax;     /* min and max amplitude of points in curve */
  FLOAT   Xmin, Xmax;     /* min and max amplitude of points in curve */
} CURVEHDR1;

typedef CURVEHDR1 *PCURVEHDR1;
typedef CURVEHDR1 FAR *LPCURVEHDR1;

/****************************************************************************/
/* void CurvehdrOMA4v11_OMA4v12(CURVEHDR *DstCurvehdr, CURVEHDR1 *SrcCurvehdr)*/  
/*                                                                          */
/* FUNCTION: Translates the OMA2000 v11 SrcCurvehdr to a (preallocated)     */
/*           oma4000/2000 version 12 DstCurvehdr                            */
/*                                                                          */
/* REQUIRES: (LPCURVEHDR) DstCurvehdr - pointer to preallocated destination */
/*                                      curve header                        */
/*           (LPCURVEHDR1) SrcCurvehdr - pointer to the source curve header */
/*                                                                          */
/****************************************************************************/

PRIVATE void CurvehdrOMA4v11_OMA4v12(CURVEHDR * DstCurvehdr,
                                     CURVEHDR1 * SrcCurvehdr)
{
  DstCurvehdr->pointnum = SrcCurvehdr->pointnum;
  DstCurvehdr->XData = SrcCurvehdr->XData;
  DstCurvehdr->YUnits = SrcCurvehdr->YUnits;
  DstCurvehdr->DataType = SrcCurvehdr->DataType;
  DstCurvehdr->experiment_num = SrcCurvehdr->experiment_num;
  DstCurvehdr->pointnum = SrcCurvehdr->pointnum;
  DstCurvehdr->time = SrcCurvehdr->time;
  DstCurvehdr->scomp = SrcCurvehdr->scomp;
  DstCurvehdr->pia[0] = SrcCurvehdr->pia[0];
  DstCurvehdr->pia[1] = SrcCurvehdr->pia[1];
  DstCurvehdr->Frame = 1;
  DstCurvehdr->Track = 1;
  DstCurvehdr->Ymin = SrcCurvehdr->Ymin;
  DstCurvehdr->Ymax = SrcCurvehdr->Ymax;
  DstCurvehdr->Xmin = SrcCurvehdr->Xmin;
  DstCurvehdr->Xmax = SrcCurvehdr->Xmax;
  DstCurvehdr->MemData = FALSE;
  DstCurvehdr->CurveCount = 1;
}

// read an old oma35 type curve header from file, convert to oma4000 curve
// header type and put in DstCurvehdr.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA readConvertCurveHeader1(CURVEHDR *DstCurvehdr, FILE *fhSource)
{
  CURVEHDR1 curvehdr1;

  if (fread(&curvehdr1, sizeof(CURVEHDR1), 1, fhSource) != 1)
     return ERROR_READ;

  CurvehdrOMA4v11_OMA4v12(DstCurvehdr, &curvehdr1);
  return ERROR_NONE;
}

// return the size of a CURVEHDR1 type in bytes.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int curveHdr1Size(void)
{
  return sizeof(CURVEHDR1);
}
