/* -----------------------------------------------------------------------
/
/  livedisk.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/livedisk.c_v   0.24   06 Jul 1992 10:30:10   maynard  $
/  $Log:   J:/logfiles/oma4000/main/livedisk.c_v  $
*/

#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <string.h>
#include <limits.h>

#include "livedisk.h"
#include "forms.h"
#include "barmenu.h"
#include "filestuf.h"
#include "tempdata.h"
#include "curvedir.h"
#include "live.h"
#include "oma4driv.h"
#include "detsetup.h"
#include "curvdraw.h"
#include "access4.h"
#include "counters.h" /* J1_COUNTER */
#include "cntrloma.h"
#include "oma4000.h"  // InitialMethod
#include "cursor.h"   // ActiveWindow
#include "formwind.h"
#include "pltsetup.h"
#include "syserror.h"  // ERROR_OPEN
#include "omaerror.h"
#include "omaform.h"   // COLORS_MESSAGE
#include "omameth.h"   // InitialMethod
#include "curvbufr.h"
#include "plotbox.h"    // copyPlotToMethod()

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

#define FAST_MODE TRUE
#define SLOW_MODE FALSE

CHAR LiveDiskFileName[FNAME_LENGTH];       // initialized in oma4000.c

int LiveDiskCount = 1;
int LiveDiskMode = SLOW_MODE;

long start_of_file;
USHORT curves_to_dump;
FILE *hLiveDiskFile;
int  live_handle;
CURVEBUFFER * pLiveBuf;

char * FixingDiskFile[] = { "Writing X Axis data to file.", NULL };

/****************************************************************************/

PRIVATE void abort_fast_loop(WINDOW *MessageWindow, USHORT CurveNum)
{
  SetParam(DC_STOP, (float)0);
  if (MessageWindow != NULL)
    release_message_window(MessageWindow);

  /* back off by one good curve to make sure */
  if (CurveNum > 1) CurveNum--;

  InitialMethod->FileCurveNum = CurveNum;
}

/****************************************************************************/
PRIVATE ERR_OMA LiveDiskLoop(void)
{
  USHORT i, j, len, DataSz, TotalCurves, L1Cntr;
  USHORT PointNum, OffsetInc, tracks, HdrSize = sizeof(CURVEHDR);
  LONG offset;
  FLOAT param;
  WINDOW *MessageWindow;
  char * buffer;

  stop_OMA_DA_in_progress();
  put_up_message_window(BusyWorkingEsc, COLORS_MESSAGE, &MessageWindow);

  GetParam(DC_TRACKS, &param);
  tracks = (SHORT)param;
  TotalCurves = (USHORT) LiveDiskCount * tracks;
  GetParam(DC_POINTS, &param);
  PointNum = (SHORT)param;

  GetParam(DC_ADPREC, &param);
  DataSz = ((SHORT)param + 1) * 2;

  OffsetInc = (PointNum * (DataSz + sizeof(float))) + sizeof(CURVEHDR);

  len = PointNum * DataSz;

  /* initialize the pointer to the live data buffer */
  buffer = pLiveBuf->BufPtr;

  offset = start_of_file;

  fflush(hLiveDiskFile);

  if (fseek(hLiveDiskFile,offset, SEEK_SET)) /* seek for first buffer */
    {
    abort_fast_loop(MessageWindow, 0);
    return error(ERROR_SEEK, LiveDiskFileName);
    }

  SetParam(DC_L, 0.0F);
  L1Cntr = 0;

  SetParam(DC_RUN, 3.0F);                      /* start the acquisition */

  for (i = 0; i < TotalCurves; i+=tracks)
    {
    while ((get_DAC_counter(L1_COUNTER)) == L1Cntr)
      {
      if (kbhit() && (getch() == ESCAPE))
        {
        abort_fast_loop(MessageWindow, i+j);
        return ERROR_NONE;
        }
      }
    L1Cntr = get_DAC_counter(L1_COUNTER);
    for (j = 0; j < tracks; j++)
      {
      if (fseek(hLiveDiskFile, offset, SEEK_SET)) /* seek for curveheader */
        {
        abort_fast_loop(MessageWindow, i+j);
        return error(ERROR_SEEK, LiveDiskFileName);
        }

      /* write header to disk */
      if(write(live_handle, &(pLiveBuf->Curvehdr), HdrSize) != (int)HdrSize)
        {
        abort_fast_loop(MessageWindow, i+j);
        return error(ERROR_SEEK, LiveDiskFileName);
        }

      get_OMA4_data(buffer, len, j);            /* get the data */

      if(write(live_handle, buffer, len) != (int)len) /* write data to disk */
        {
        abort_fast_loop(MessageWindow, i+j);
        return error(ERROR_WRITE, LiveDiskFileName);
        }
      offset += OffsetInc;
      }
    }                                                /* for loop ends */
  pLiveBuf->status = CVBUF_CLEAN;

  if (MessageWindow != NULL)
    release_message_window(MessageWindow);

  return ERROR_NONE;
}

// clean up for GoLiveDisk() to use if there is an error
//---------------------------------------------------------------------
PRIVATE int shutdownGoLiveDisk(ERR_OMA err, WINDOW ** messageWin)
{
  if (err != ERROR_OPEN)      // don't close if not open
    fclose(hLiveDiskFile);

  if (messageWin)
    * messageWin = release_message_window(* messageWin);

  error(err, LiveDiskFileName);

  return FIELD_VALIDATE_WARNING;
  }

static void get_curves_to_dump(void)
{
  curves_to_dump = det_setup->memories * det_setup->tracks;
}

int GoLiveDisk(void)
{
  static USHORT i;
  USHORT PointNum;
  long offset, OffsetInc;
  float * XPtr, param;
  SHORT DataSz;
  ERR_OMA err = ERROR_NONE;
  LONG NewSize;
  SHORT hTemp;
  WINDOW * MessageWindow = NULL;

  if (fake_detector)
    return FIELD_VALIDATE_SUCCESS;

  GetParam(DC_POINTS, &param);
  PointNum = (SHORT)param;
  GetParam(DC_ADPREC, &param);
  DataSz = ((SHORT)param + 1) * 2;
  get_curves_to_dump();

  if ((hLiveDiskFile = fopen(LiveDiskFileName, "rb")) != 0)
    {
    fclose(hLiveDiskFile);
    if (yes_no_choice_window(DataFileOverwritePrompt, 0,
      COLORS_MESSAGE) != YES)      // exit on escape or NO
      return FIELD_VALIDATE_SUCCESS;
    }

  if(SetupAcqCurveBlk(PointNum, curves_to_dump))
    return FIELD_VALIDATE_WARNING;

  hLiveDiskFile = fopen(LiveDiskFileName,"w+b");
  if (hLiveDiskFile == 0)
    {
    error(ERROR_OPEN, LiveDiskFileName);
    return FIELD_VALIDATE_WARNING;
    }

  live_handle = fileno(hLiveDiskFile);

  offset = 0;

  InitialMethod->FileCurveNum = LiveDiskCount * curves_to_dump;
  strcpy(InitialMethod->FileTypeID, fidDataOMA4);

  SetFormFromPlotBox(ActiveWindow);               /* copy plotbox vals */
  CopyPlotToMethod();                             /* to InitialMethod */
  err = DetInfoToMethod(&InitialMethod);          /* fill method from */
                                                  /* current detector */
  if (!err)
    err = MethdrWrite(hLiveDiskFile, LiveDiskFileName, InitialMethod);

  if (err)
    {
    fclose(hLiveDiskFile);
    return FIELD_VALIDATE_WARNING;
    }

  start_of_file = InitialMethod->Length;

//  FlushCurveBuffer(0);
  pLiveBuf = &(CvBuf[LIVE_XFER_BUF]);

  ReadTempCurvehdr(&MainCurveDir, LiveBlkIndex, 0, &pLiveBuf->Curvehdr);

  if (DataSz == 2)
    pLiveBuf->Curvehdr.DataType = SHORTTYPE;
  else
    pLiveBuf->Curvehdr.DataType = LONGTYPE;

  pLiveBuf->Curvehdr.MemData = FALSE;

  LiveDiskLoop();

  stop_OMA_DA_in_progress();
  put_up_message_window(FixingDiskFile, COLORS_MESSAGE, &MessageWindow);

  /* check for write error */
  if (InitialMethod->FileCurveNum !=
    ((USHORT) LiveDiskCount * (USHORT) curves_to_dump))
    {
    rewind(hLiveDiskFile);      /* clear error */
    NewSize = (LONG) InitialMethod->Length;
    NewSize +=   (LONG) InitialMethod->FileCurveNum
      * (LONG) (sizeof(CURVEHDR) + (2 * PointNum * DataSz));
    hTemp = fileno(hLiveDiskFile);

    NewSize = (LONG) chsize(hTemp, NewSize);

    SetFileCurveNum(hLiveDiskFile, LiveDiskFileName,
      InitialMethod->FileCurveNum);
    }

  LoadCalibratedX(pLiveBuf, PointNum);

  offset    = start_of_file;
  OffsetInc = (LONG)((LONG)PointNum * (LONG)DataSz);

  XPtr = (float *)((ULONG) pLiveBuf->BufPtr + (ULONG) (BUFLEN / 2));

  for (i = 0; i < InitialMethod->FileCurveNum; i++)
    {
    if (fseek(hLiveDiskFile, offset, SEEK_SET))
      return shutdownGoLiveDisk(ERROR_SEEK, & MessageWindow);

    if (fwrite(&(pLiveBuf->Curvehdr), sizeof(CURVEHDR), 1, hLiveDiskFile) != 1)
      return shutdownGoLiveDisk(ERROR_WRITE, & MessageWindow);

    // write out the X data

    offset += sizeof(CURVEHDR) + OffsetInc;

    if (fseek(hLiveDiskFile, offset, SEEK_SET))
      return shutdownGoLiveDisk(ERROR_SEEK, & MessageWindow);

    if (fwrite(XPtr, sizeof(float), PointNum, hLiveDiskFile) != PointNum)
      return shutdownGoLiveDisk(ERROR_WRITE, & MessageWindow);

    offset += PointNum * sizeof(float);

    }  /* for all the curves */

  fclose(hLiveDiskFile);
  if (MessageWindow != NULL)
    release_message_window(MessageWindow);

  return FIELD_VALIDATE_SUCCESS;
}
