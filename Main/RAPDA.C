/* Copyright 1992 EG & G  PARC **********************************************
 *
 *  Rapda_dr.c     The Drivers part of RAPDA.
 *
 *  The RAPDA is divided into a number of non-overlapping regions. Each
 *  region is characterized by:
 *    1. An exposure time.
 *    2. A start pixel and number of contiguous pixels.
 *    3. A group size, which is always 1 now - probably forever.
 *    4. An address of where the data points are stored.
 *
 *  All exposure times are related to a "ShortET", which is the shortest
 *  exposure time in the above regions; they are all powers of 2 times
 *  the ShortET:  Exp time = SHORT.ET * 2ü, where n ranges from 0 to 16.
 *  The scan Exposure Time is the exposure time of the region with the
 *  largest exposure time.
 *
 *  The information concerning the RAPDA scan setup is kept in an array
 *  of structures "RegionType", 9 bytes long:
 *
 *    struct RegionType
 *    {
 *      char   n;                  Exp time = SHORT.ET * 2ü
 *      int StartPixel;            First pixel in region.
 *      int Number;                Number of pixels in region.
 *      int DataOffset;            Offset to region in data points.
 *    }
 *
 *  and the parameter ShortET.  The array is arranged in decreasing "n"
 *  values, and within one "n" value in increasing "StartPixel" value.
 *  The minimum number of pixels in an region is MIN_PXLS, which is now
 *  defined as 8;  the array has a maximum of (1024 / MIN_PXLS) structures
 *  or 128 maximum structures in it.
 *
 *  The array is stored in the "method file", and is used by the drivers
 *  to set up the OMA4 board program.
 *  
 *  The data for the complete scan will be contiguous, and will be
 *  in increasing "StartPixel" number.  For example, if there are
 *  two regions, one with StartPixel 100, Number 200,
 *  and the other with StartPixel 800, Number 20,
 *  then there will be a total of 220 data points in RAM, and the 
 *  first region will have DataOffset 0, and the second region will
 *  have DataOffset 200.
 *
 *  The function "DataPtToPixel" is used to find the "Pixel" corresponding
 *  to a data point.
 *  Note that in the structure, StartPixel and DataOffset are zero
 *  based, but "DataPtToPixel" and "PixelToDataPt" are one based.
 *  So passing 1 to DataPtToPixel would result in 200.  Confusing, ? I've
 *  wondered about people who start counting at 1.
 *
 ***************************************************************************/
#if FALSE

#include "Rapda.h"
#include "oma4driv.h"
#include "driverrs.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>


static float MinET;                   /* The minimum ET needed to do the
                                       * scan. */
static float ShortET;                 /* The shortest ET. This must be set
                                       * first so error checks can be made.*/
struct RegionType ThisRegion;           /* Dummy to hold new regions. */
                    
    /* Figures out the Data offsets to the first pixel for all the
     * regions in the array.
     */
void CalcDataOffsets (void)
{
  struct RegionType *Ptr;
  int i, j;
  for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
    {
    Ptr = &RAPDA_SETUP.RapdaReg[i];
    Ptr->DataOffset = 0;
      for (j = 0; j < RAPDA_SETUP.NumRegs; j++)
        if (Ptr->StartPixel > RAPDA_SETUP.RapdaReg[j].StartPixel)
          Ptr->DataOffset += RAPDA_SETUP.RapdaReg[j].Number;
    }
}

   /* Checks that the ShortET is long enough to read the entire scan plus
    * any "overhead" such as reading source comp.  If so, it figures out
    * the number of instruction times needed for the delay and sets up
    * ShortET.  There is a quantity SHORT_ET_OVERHD which is added
    * by the driver to the last program in the list to allow time both
    * in the scan to decode which regions to scan next and also to give
    * a cushion between scans for decrementing counters, inc.mem's, etc..
    */
int SetShortET (float ExpTime )
{
  int err = NO_ERROR, i;
  unsigned long clicks, delay;
                                      /* Total number of A/D cycles. */
  delay = (unsigned long) (ExpTime / A_D_TIME);
  clicks = 0L;
  for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
                                      /* Find total number of pixels, */
                                      /* and number_regions * PROG_OVERHD. */
    clicks += (unsigned long)(RAPDA_SETUP.RapdaReg[i].Number + PROG_OVERHD);
                                      /* Add overhead per event. */
  clicks += (unsigned long)(SC_OVERHD + SHORT_ET_OVERHD);
  if (clicks > delay)                 /* See if enough time allotted. */
    {
    err = SHORT_ET_ERR;               /* If not flag error. */
    delay = clicks;                   /* and force enough time. */
    }
  MinET = (float)clicks * A_D_TIME;   /* The shortest possible ET. */
  ShortET = (float)delay * A_D_TIME;  /* ShortET must be multiple of A/D's */
  return (err);
}

void PrintRegion (void )
{
  struct RegionType *iPtr;
  int i;
 printf ("Rapda:\n");
  for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
    {
    iPtr = &RAPDA_SETUP.RapdaReg[i];
    printf ("Region %i, start %i, number %i, n %i\n",
             i, iPtr->StartPixel, iPtr->Number, (int)iPtr->n);
    }
}

   /* Given a data point, retrieves the "pixel" corresponding to the
    * point.  If illegal data point, an error
    * is returned.  Data points and pixels are 1 based.
    */
int DataPtToPixel (int DataPt, float *Pixel )
{
  int i, err = OUT_OF_ARRAY_ERR;

  CalcDataOffsets();
  for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
    {
    struct RegionType *Ptr = &RAPDA_SETUP.RapdaReg[i];
    if ((DataPt > Ptr->DataOffset) &&
        (DataPt <= (Ptr->DataOffset + Ptr->Number)))
      {
      err = NO_ERROR;                 /* flag pixel ok. */
      *Pixel = (float)(Ptr->StartPixel + DataPt - Ptr->DataOffset);
      break;
      }
    }
  return (err);
}

   /* Given a pixel, returns a pointer to the region containing the pixel.
    * Returns NULL if pixel is not in an region.
    */
struct RegionType *PixelToRegion ( int Pixel)
{
  int i;
  struct RegionType *Ptr = NULL;
                                   /* Search entire Array. */
  for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
    {
    if ((Pixel >= Ptr->StartPixel) && 
        (Pixel < (Ptr->StartPixel + Ptr->Number)))
      {
      Ptr = &RAPDA_SETUP.RapdaReg[i];
      break;
      }
    }
  return (Ptr);                       /* Then return pointer. */
}

   /* Given a pixel, retrieves the data point number for that pixel.
    * It returns an error is pixel is not in an region. Pixels and data
    * points are 1 based.
    */
int PixelToDataPt (int Pixel, int *DataPt)
{
  int err = UNSCANNED_PXL_ERR;        /* See if pixel is legitimate. */
  struct RegionType *Ptr = PixelToRegion ( Pixel);
  if (Ptr)                            /* If so, */
    {
    err = NO_ERROR;                   /* Flag  pixel is ok, */
    CalcDataOffsets ();               /* Make sure data offsets are ok. */
    *DataPt = Ptr->DataOffset + 1 +   /* and return by ref value of point. */
              (Pixel - 1 - Ptr->StartPixel);
    }
  return (err);
}

   /* Given a pixel, retrieves the exposure time for the region containing
    * the pixel.  It depends on ShortET being correct.  It returns an
    * error if pixel is not in legit group.
    */
int PixelToET (int Pixel, float *ExpTime )
{
  int err = UNSCANNED_PXL_ERR;
  struct RegionType *Ptr = PixelToRegion ( Pixel);
  if (Ptr)
    {
    err = NO_ERROR;
    *ExpTime = ShortET * (float)( 1L << (int)Ptr->n);
    }
  return (err);
}

   /* Given a pixel in a group, retrieves the first pixel, last pixel,
    * and ET for the region.
    * It returns an error code if pixel is not part of legit group.
    */
int PixelToParams (int Pixel, int *First, int *Last, float *ET)
{
  int err = UNSCANNED_PXL_ERR;
  struct RegionType *Ptr = PixelToRegion ( Pixel);
  if (Ptr)
    {
    err = NO_ERROR;
    if (First)
      *First = Ptr->StartPixel + 1;
    if (Last)
      *Last = Ptr->StartPixel + Ptr->Number;
    if (ET)
      *ET = ShortET * (float)( 1L << (int)Ptr->n);
    }
  return (err);
}

   /* Check Exposure time range, and set value of "n". Return error if
    * range of ET's too great.
    */
int GetN (float RegionET, float ShortET)
{
  char n;
  for (n = 0; ((n <= MAX_N) && (RegionET > ShortET)); n++)
    RegionET /= (float)2.0;
  if (n > MAX_N)
    return (ET_ERR);
  ThisRegion.n = n;
  return (NO_ERROR);
}


int SetRegionET (float RegionET)
{
  char n;
  int err = NO_ERROR, i = 0;
  if (RegionET < MinET)
    RegionET = MinET;
  if (RegionET < ShortET)
    {
    for (i = 0; RegionET < ShortET; i++)
      ShortET /= (float)2;
    ShortET = RegionET;
    }
  n = (char)i - RAPDA_SETUP.RapdaReg [RAPDA_SETUP.NumRegs -1].n;
  for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
    RAPDA_SETUP.RapdaReg [i].n += n;
  for (n = 0; ((n <= MAX_N) && (RegionET > ShortET)); n++)
    RegionET /= (float)2.0;
  if (n > MAX_N)
    {
    n = MAX_N;
    err = ET_ERR;
    }
  ThisRegion.n = n;
  return (err);
}

#endif
