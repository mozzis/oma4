/* fabsdata.h */

#ifndef PRIMTYPE_INCLUDED
  #include "primtype.h"
#endif

/* establish communication with detector */

int ConnectDetector(USHORT port, ULONG address, ULONG size);

/* set the detector settings for acquisition, and allocate memory for    */
/* data and post processing. Data1 and Data2 point to pointers; the      */
/* pointers are not to be initialized or changed by the calling routine  */
/* on return they will point to regions of memory each large enough to   */
/* hold one track.  The same goes for Result, except it will hold floats */

int SetupDetector(USHORT *Points, ULONG **Data1,
                    ULONG **Data2, float **Result);


/* read parameters from a text file */

int DoCustomSetup(char * filename);

/* begin acquiring data in live mode */

int StartData(void);

/* stop the detector acquisition mode */

int StopData(void);

/* read data from two tracks, and do absorbance calculation */

int get_Data(ULONG *Track1, ULONG *Track2, short Count, float * Result);

