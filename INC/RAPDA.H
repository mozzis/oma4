/* Copyright 1992 EG & G  PARC **********************************************
 *
 *  Rapda.h
 *
 ***************************************************************************/
#if FALSE
   /* Holds the info for the RAPDA scan setup in an array. */ 
struct RegionType
{
  char   n;                           /* Exp time = SHORT.ET * 2ü */
  int StartPixel;                     /* First pixel in region. */
  int Number;                         /* Number of pixels in region. */
  int GroupSize;                      /* Number of pixels per datapoint. */
  int DataOffset;                     /* Offset to region in data points. */
};

#define MIN_PXLS        8
#define MIN_PXL         1
#define MAX_PXL         1023
#define MAX_N           16
#define A_D_TIME        (float)3.2E-6
#define PROG_OVERHD     2     /* Extra instructions need for each program. */
#define SC_OVERHD       16    /* Instructions needed for source comp. */
#define SHORT_ET_OVERHD 32    /* NOP's added at end of scan for looping... */

  /* Functions that return an error code return one of the following:
   *  0 - no error
   *  1 - Array had more than (1024 / MIN_PXLS) regions.
   *  2 - region overlap
   *  3 - fewer than MIN_PXLS in region
   *  4 - region falls outside array.
   *  6 - Pixel not part of any region.
   *  7 - Short.ET too short
   *  8 - ET not Short.ET * 2ü
   *  9 - (Scan.ET / Short.ET)  > 65536  (2^16)
   */

#define NO_ERROR          0
#define ARRAY_SIZE_ERR    1
#define OVERLAP_ERR       2
#define SIZE_ERR          3
#define OUT_OF_ARRAY_ERR  4
#define UNSCANNED_PXL_ERR 6
#define SHORT_ET_ERR      7
#define ET_ERR            8
#define ET_RANGE_ERR      9

/* ---------------  The functions ------------------- */

int GetN (float RegionET, float ShortET);
   /* Check Exposure time range, and set value of "n". Return error if
    * range of ET's too great.
    */

int AddArea (void );
   /* Adds "ThisArea" to list, if ok. List is kept in increasing pixel
    * number. Return one of the following error codes.
    *  0 - no error
    *  1 - Array had more than (1024 / MIN_PXLS) regions.
    *  2 - region overlap
    *  3 - fewer than MIN_PXLS in region
    *  4 - region falls outside array.
    *  6 - Pixel not part of any region.
    *  7 - Short.ET too short
    *  8 - ET not Short.ET * 2ü
    *  9 - (Scan.ET / Short.ET)  > 65536  (2^16)
    */

void  FreeAreas (void );
   /* Cleans out entire list and zeroes the exposure times. */

int   InsertArea (float ExpTime, int First, int Number, int GPSize); 
   /* float ExpTime
    * int First
    * int Number
    * int GPSize
    * Given the parameters that describe an area, this checks the parameters
    * and if all are ok, it adds the area to the list. If not, it returns
    * an error code.
    */

int   RemoveArea (int Pixel);
   /* int Pixel
    * Given a pixel, removes the area containing that pixel, if any.
    * It returns an error code if pixel is not part of legit group.
    */

int   DataPtToPixel (int DataPt, float *Pixel );
   /* int DataPt
    * float *Pixel
    * Given a data point, retrieves the "pixel" corresponding to the
    * point.  Grouping creates superpixels, and in this case, the center
    * of the group is retrieved.  For example, is pixels 98 and 99 were
    * grouped this would retrieve 98.5.  If illegal data point, an error
    * is returned.  Data points and pixels are 1 based.
    */

int   PixelToDataPt (int Pixel, int *DataPt);
   /* int Pixel
    * int *DataPt
   /* Given a pixel, retrieves the data point number for that pixel.
    * It returns an error is pixel is not in an area. Pixels and data
    * points are 1 based.
    */

int   PixelToET (int Pixel, float *ExpTime); 
   /* int Pixel
    * float *ExpTime
   /* Given a pixel, retrieves the exposure time for the area containing
    * the pixel.  It depends on ShortET being correct.  It returns an
    * error if pixel is no in legit group.
    */

int   PixelToGpSize (int Pixel, int *GpSize); 
   /* int Pixel
    * int *GpSize
    * Given a pixel, retrieves the group size for the group containing the
    * area.  It returns an error code if pixel is not part of legit group.
    */

int SetShortET (float ExpTime );
   /* float ExpTime
    * Checks that the ShortET is long enough to read the entire scan plus
    * any "overhead" such as reading source comp.  If so, it figures out
    * the number of instruction times needed for the delay and sets up
    * ShortET and ShortDelay.
    */

int PixelToParams (int Pixel, int *First, int *Last, int *Size, float *ET);
   /* int Pixel
    * int *First
    * int *Last
    * int *Size
    * float *ET
    * Given a pixel in a group, retrieves the first pixel, last pixel,
    * group size and ET for the area.
    * It returns an error code if pixel is not part of legit group.
    */

void  CalcDataOffsets(void);
struct  ArrayAreaType *PixelToArea(int  Pixel);

extern struct RegionType ThisRegion; /* Dummy to hold new regions. */

void  CalcDataOffsets(void);
void  zCalcDataOffsets(void);
int  InsertRegion(float  ExpTime,int  First,int  Number);
int  SetShortET(float  ExpTime);
int  AddRegion(void);
int  zAddRegion(void);
int  CheckRegionOverlap(void);
void  PrintRegion(void);
void  FreeRegions(void);
int  DataPtToPixel(int  DataPt,float  *Pixel);
struct  RegionType *PixelToRegion(int  Pixel);
int  PixelToDataPt(int  Pixel,int  *DataPt);
int  PixelToET(int  Pixel,float  *ExpTime);
int  PixelToParams(int  Pixel,int  *First,int  *Last,float  *ET);
int  zRemoveRegion(int  Num);
int  RemoveRegion(int  Num);
int  GetN(float  RegionET,float  ShortET);
int  SetRegionET(float  RegionET);
#endif

