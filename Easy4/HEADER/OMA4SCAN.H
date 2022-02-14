/* FILE : oma4scan.h */

#ifndef OMA4SCAN_INCLUDED
#define OMA4SCAN_INCLUDED

typedef struct
  { 
  struct 
    {
    unsigned int Total_Image;       /* Total pixels in image direction */
    unsigned int Total_Serial;      /* Total pixels in serial direction */
    unsigned int Image_Delta;       /* Pixels/point in image direction */
    unsigned int Serial_Delta;      /* Pixels/point in serial direction */
    unsigned int Image0;            /* Pixels to skip at start in image direction */
    unsigned int Serial0;           /* Pixels to skip at start in serial direction */
    unsigned int Image1;            /* Pixels to skip at end in image direction */
    unsigned int Serial1;           /* Pixels to skip at end in serial direction */
    unsigned int Image_Groups;      /* Points or tracks in image direction */
    unsigned int Serial_Groups;     /* Points or tracks in serial direction */
    unsigned int Image_Skip;        /* pixels between in image direction */
    unsigned int Serial_Skip;       /* pixels between in serial direction */
    unsigned int Dead_Serial_Lead;  /* Dead pixels at start in image direction */
    unsigned int Dead_Image_Lead;   /* Dead pixels at start in serial direction */
    unsigned int Dead_Serial_Trail; /* Dead pixels at end in image direction */
    unsigned int Dead_Image_Trail;  /* Dead pixels at end in serial direction */
    } real_setup;

  struct
    {
    unsigned int Total_Image;       /* Total pixels in image direction */
    unsigned int Total_Serial;      /* Total pixels in serial direction */
    unsigned int Tracks_Delta;      /* Height of a track */
    unsigned int Points_Delta;      /* Width of a point */
    unsigned int Track0;            /* Start pixel of a track */
    unsigned int Point0;            /* Start pixel of a point */
    unsigned int Track_Groups;      /* Number of tracks in a frame */
    unsigned int Point_Groups;      /* Number of points in a track */
    unsigned int Track_Skip;        /* pixels between each track */
    unsigned int Point_Skip;        /* pixels between each point */
    } presented_setup;              
  } SCAN_SETUP;
                                    

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



