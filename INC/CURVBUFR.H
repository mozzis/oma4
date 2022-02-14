/***************************************************************************/
/*                                                                         */
/*  curvbufr.h                                                             */
/*                                                                         */
/*  copyright (c) 1989, EG&G Instruments Inc.                              */
/*                                                                         */
/*    Routines to buffer curve data access between normal memory and disk. */
/*    use VDISK or normal disk files as temporary storage.                 */
/*    Three buffers will be kept in normal memory to try to speed disk     */
/*    access.                                                              */
/*
/  $Header: /OMA4000/Include/CURVBUFR.H 1     6/18/99 1:52a Maynard $
/
/***************************************************************************/
  
#ifndef CURVBUFR_INCLUDED
#define CURVBUFR_INCLUDED
  
#include "crvheadr.h"  // CURVEHDR in struct curve_buffer
  
struct curve_buffer
{
   PVOID BufPtr;           /* pointer to the start of the buffer */
   CURVEDIR * ActiveDir;   /* buffered curve's CURVEDIR */
   SHORT Entry;            /* buffered curve's curve block entry index */
   USHORT CurveIndex;      /* Curve's file number */
   USHORT BufferOffset;    /* Buffer's starting point number offset into */
                           /* curve */
   CURVEHDR Curvehdr;      /* curve header for this curve */
   char     status;        /* CLEAN or DIRTY */
};

/* typedef struct curve_buffer CURVEBUFFER; */

/* 2k base size, both X and Y data, LONGINTS */
/* This is also the maximum size of a live curve! */
#define BUFFER_MAX_POINTS  4096
#define BUFLEN ((BUFFER_MAX_POINTS+1) * 2 * sizeof(LONG))

/* Most operations will assume only 3 curve buffers.  Buffer #4 (index=3) */
/* is dedicated for live data transfer. */
/* CvBuf is declared as CURVEBUFFER CvBuf[BUFNUM+1] */

#define BUFNUM 3
#define LIVE_XFER_BUF 3
extern CURVEBUFFER far CvBuf[];
  
/* values for status field of CURVEBUFFER. CVBUF_DIRTY means the buffer has */
/* been changed and must be written to disk before new data can be put into */
/* it. */
enum { CVBUF_CLEAN, CVBUF_DIRTY };

ERR_OMA InitCurveBuffer(VOID);
ERR_OMA clearLiveCurveBufs(void);
ERR_OMA clearAllCurveBufs(void);
ERR_OMA ReleaseCurveBuffer(VOID);
ERR_OMA FlushCurveBuffer(SHORT);
ERR_OMA LoadCurveBuffer(CURVEDIR *, SHORT, USHORT, PUSHORT, SHORT*);

// Fills XData part of a curvebuffer using calibration coefficients
// pointed to by XCal.
//
void LoadCalibratedX(CURVEBUFFER *pBuf, USHORT PointNum);
void GenXData(USHORT PointNum, PFLOAT XCal);
void RegenXData(PFLOAT XCal);
void GetXMinMax(FLOAT * XMin, FLOAT * XMax);

// print to stderr function for debugging only
//
VOID print_tempbuf(CURVEBUFFER FAR *);

#endif  // CURVBUFR_INCLUDED
