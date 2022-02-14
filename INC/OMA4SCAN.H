/* FILE : oma4scan.h */

#ifndef OMA4SCAN_INCLUDED
#define OMA4SCAN_INCLUDED

     /* Source Comp modifiers */
#define detSCMP   0x1F      /* %00011111  All source comp bits.*/
#define detNOINT  0x08      /* %00001000  No integration.      */
#define detUS10   0x10      /* %00010000  10 uS time constant. */
#define detUS100  0x14      /* %00010100  100 uS time constant.*/
#define detMS1    0x12      /* %00010010  1 mS time constant.  */
#define detMS10   0x16      /* %00010110  10 mS time constant. */
#define detMS100  0x11      /* %00010001  100 mS time constant.*/
#define detS1     0x15      /* %00010101  1 Sec time constant. */
#define NUMSCOPTS 7

/* detector instructions */

#define DEAD_SERIAL_LEAD 20         
#define DEAD_SERIAL_TRAIL 11
#define DEAD_IMAGE_LEAD 2
#define DEAD_IMAGE_TRAIL 2
#define MAX_POINT_COUNT 1258
#define MAX_TRACK_COUNT 2258

#define no_op_time ((float)3e-6)

#endif  /*  OMA4SCAN_INCLUDED */
