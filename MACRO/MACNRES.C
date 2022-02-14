/* -----------------------------------------------------------------------
/
/  macnres.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/macro/macnres.c_v   0.34   06 Jul 1992 12:50:20   maynard  $
/  $Log:   J:/logfiles/oma4000/macro/macnres.c_v  $
 * 
 *    Rev 0.34   06 Jul 1992 12:50:20   maynard
 * Fix bug in SPlotStyle - didn't draw boxes correctly
 * 
 *    Rev 0.33   12 Mar 1992 16:01:36   maynard
 * SaveFileCurves: do SetErrorFlag if ParsePathAndName fails.  This
 * routine was using the wrong name for the output file.  In addition,
 * if the old file could not be written to, the error message also named
 * the wrong file.  Changes made so that the expected behaviors occur,
 * etc.
 * 
/
*/

#include <string.h>
#include <io.h>
#include <malloc.h>
  
#ifndef PROT
   #include <bios.h>
#endif
  
#include "macnres.h"
#include "forms.h"
#include "oma4driv.h"
#include "detsetup.h"
#include "multi.h"
#include "curvdraw.h"
#include "device.h"
#include "crventry.h"
#include "doplot.h"
#include "oma4000.h"
#include "cursor.h"
#include "calib.h"
#include "change.h"
#include "backgrnd.h"
#include "curvedir.h"
#include "di_util.h"
#include "pltsetup.h"
#include "mdatstak.h"
#include "macruntm.h" /* SetErrorFlag()*/
#include "macrores.h" /* MacBadParam()*/
#include "syserror.h" /* ERROR_OPEN*/
#include "omaerror.h"
#include "filestuf.h"
#include "curvbufr.h"
#include "plotbox.h"
#include "omameth.h"
#include "tempdata.h"
#include "cmdtbl.h"

char TempEntryName[12] = "temp";       
  
static float * flt_param_addr[2];
  
void FltPushDetVal(USHORT Command)
{
   FLOAT fTemp;
   DOUBLE dTemp;
  
   if (GetParam(Command, &fTemp))
      SetErrorFlag();
   else
   {
      dTemp = (DOUBLE) fTemp;
      PushToDataStack(&dTemp, TYPE_REAL, FALSE);
   }
}
  
void IntPushDetVal(USHORT Command)
{
   float sTemp;
   int pTemp;
  
   if(GetParam(Command, &sTemp))
      SetErrorFlag();
   else
   {
      pTemp = (int) sTemp;
      PushToDataStack(&pTemp, TYPE_INTEGER, FALSE);
   }
}
  
void IntSetSingleDetVal(USHORT Command)
{
   SHORT sTemp;
  
   if (! PopScalarFromDataStack(&sTemp, TYPE_INTEGER))
   {
      error(ERROR_BAD_PARAM_TYPE, 1);
      SetErrorFlag();
      return;                         
   }

   if (set_int_detect_param(Command, sTemp))
      SetErrorFlag();
}
  
void FltSetSingleDetVal(USHORT Command)
{
   DOUBLE dTemp;
  
   if (! PopScalarFromDataStack(&dTemp, TYPE_REAL))
   {
      error(ERROR_BAD_PARAM_TYPE, 1);
      SetErrorFlag();
      return;                         
   }
  
   if (set_flt_detect_param(Command, (float)dTemp))
      SetErrorFlag();
}
  
void PopSingleIntAndSetVal(int command)
{
   SHORT sTemp;
  
   if (! PopScalarFromDataStack(&sTemp, TYPE_INTEGER))
   {
      error(ERROR_BAD_PARAM_TYPE, 1);
      SetErrorFlag();
      return;
   }
   if (set_int_detect_param(command, sTemp))
      SetErrorFlag();
}
  
void PopSingleFltAndSetVal(int command)
{
   DOUBLE sTemp;
  
   if (! PopScalarFromDataStack(&sTemp, TYPE_REAL))
   {
      error(ERROR_BAD_PARAM_TYPE, 1);
      SetErrorFlag();
      return;
   }
   if (set_flt_detect_param(command, (float)sTemp))
      SetErrorFlag();
}

  
void DArea(void)
{
   SHORT MemNum, FirstChannel, TotalChannels, i;
   SHORT BadParam = 0;
   unsigned long * pData, sum = 0L;
   unsigned len;
   float flen;
   double Area;
  
   if (! PopScalarFromDataStack(&TotalChannels, TYPE_INTEGER))
      BadParam = 3;
  
   else if (! PopScalarFromDataStack(&FirstChannel, TYPE_INTEGER))
      BadParam = 2;
  
   else if (! PopScalarFromDataStack(&MemNum, TYPE_INTEGER))
      BadParam = 1;
  
   if (BadParam)
   {
      MacBadParam(BadParam);
      return;
   }

  GetParam(DC_BYTES, &flen);
  
  len = (unsigned)flen;

  if (!(pData = malloc(len)))
    {
    error(ERROR_ALLOC_MEM);
    SetErrorFlag();
    return;
    }

  ReadCurveFromMem(pData, len, MemNum);

  for (i = FirstChannel; i < TotalChannels; i++)
    {
    sum += pData[i];
    }

  free(pData);

  Area = (double)sum;

  PushToDataStack(&Area, TYPE_REAL, FALSE);
}
  
void DCArea(void)  
{
  SHORT MemNum, FirstChannel, TotalChannels, i;
  SHORT BadParam = 0;
  unsigned long * pData, thresh, sum = 0L;
  unsigned len;
  float flen;
  double Area;

  if (! PopScalarFromDataStack(&TotalChannels, TYPE_INTEGER))
    BadParam = 3;

  else if (! PopScalarFromDataStack(&FirstChannel, TYPE_INTEGER))
    BadParam = 2;

  else if (! PopScalarFromDataStack(&MemNum, TYPE_INTEGER))
    BadParam = 1;

  if (BadParam)
    {
    MacBadParam(BadParam);
    return;
    }

  GetParam(DC_BYTES, &flen);

  len = (unsigned)flen;

  if (!(pData = malloc(len)))
    {
    error(ERROR_ALLOC_MEM);
    SetErrorFlag();
    return;
    }

  ReadCurveFromMem((unsigned char *)pData, len, MemNum);

  thresh = (pData[FirstChannel] + pData[FirstChannel + TotalChannels]) / 2;

  for (i = FirstChannel;i<TotalChannels;i++)
    {
    if (pData[i] > thresh)
      sum += pData[i] - thresh;
    }

  free(pData);

  Area = (double)sum;

  PushToDataStack(&Area, TYPE_REAL, FALSE);
}
  
void LoadFileCurves(void)   
{
  SHORT BadParam = 0;
  SHORT InputFormat;
  USHORT DstStartCurve, Count, SrcStartCurve, Dummy;
  ERR_OMA err = ERROR_NONE;
  CURVE_REF DstCurve;
  CURVE_REF *pCurve;
  PCHAR FileName;
  CHAR Name[DOSFILESIZE + 1];
  CHAR Path[DOSPATHSIZE + 1];
  SHORT Type;

  /* input_format ignored for now */
  if (! PopScalarFromDataStack(&InputFormat, TYPE_INTEGER))
    BadParam = 6;
  else if (! PopScalarFromDataStack(&DstStartCurve, TYPE_WORD))
    BadParam = 5;

  if (! BadParam)
    {
    Type = PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO);
    DstCurve = * pCurve;
    if ((Type != TYPE_CURVE) || (DstCurve.ReferenceType != CLASS_CURVESET))
      BadParam = 4;
    else if (! PopScalarFromDataStack(&Count, TYPE_WORD))
      BadParam = 3;
    else if (! PopScalarFromDataStack(&SrcStartCurve, TYPE_WORD))
      BadParam = 2;
    else if (! PopScalarFromDataStack(&FileName, POINTER_TO | TYPE_STRING))
      BadParam = 1;
    }

  if (BadParam)
    {
    MacBadParam(BadParam);
    return;
    }

  if (ParsePathAndName(Path, Name, FileName) != 2)
    error(err = ERROR_OPEN, FileName);
  else
    err = ReadFileToTemp(Name, Path, SrcStartCurve, Count, &MainCurveDir,
                         (USHORT *) (&DstCurve.CurveSetIndex), &Dummy);
  if (err)
    SetErrorFlag();
  else
    MainCurveDir.Entries[DstCurve.CurveSetIndex].StartIndex = DstStartCurve;
}
  
void SaveFileCurves(void)
{
  SHORT BadParam = 0;
  SHORT OutputFormat;     /* 0 = oma4 binary*/
                          /* 1 = 1460 */
                          /* 2 = Single Column */
                          /* 3 = Single Column with X */
                          /* 4 = Multi Column */
                          /* 5 = Multi Column with X */
                          /* 6 = TCL (Hidris) format */
                          /* 7 = TIFF format */
  USHORT DstStartCurve, Count, SrcStartCurve;
  CURVE_REF DstCurve;
  CURVE_REF *pCurve;
  PCHAR FileName;
  CHAR Name[DOSFILESIZE + 1];
  CHAR Path[DOSPATHSIZE + 1];
  SHORT Type;

  if (! PopScalarFromDataStack(&OutputFormat, TYPE_INTEGER))
    BadParam = 6;
  else if (! PopScalarFromDataStack(&DstStartCurve, TYPE_WORD))
    BadParam = 5;
  else if (! PopScalarFromDataStack(&Count, TYPE_WORD))
    BadParam = 4;
  else if (! PopScalarFromDataStack(&SrcStartCurve, TYPE_WORD))
    BadParam = 3;
  else
    {
    Type = PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO);
    DstCurve = * pCurve;
    if ((Type != TYPE_CURVE) || (DstCurve.ReferenceType != CLASS_CURVESET))
      BadParam = 2;
    else if (! PopScalarFromDataStack(&FileName, POINTER_TO | TYPE_STRING))
      BadParam = 1;
    }

  if (BadParam)
    {
    MacBadParam(BadParam);
    return;
    }

  if (ParsePathAndName(Path, Name, FileName) != 2)
    {
    error(ERROR_OPEN, FileName);
    SetErrorFlag();
    return;
    }
  /* use translate screen buffers, (side effect: menu entries change) */
  strcpy(ch_input_fname, MainCurveDir.Entries[DstCurve.CurveSetIndex].path);
  strcat(ch_input_fname, MainCurveDir.Entries[DstCurve.CurveSetIndex].name);
  strcpy(ch_output_fname, FileName);

  if (!access(ch_output_fname, 0))  /* see if file exists */
    if (remove(ch_output_fname))     /* if so try to delete it first */
      {
      error(ERROR_ACCESS_DENIED, ch_output_fname);
      SetErrorFlag();
      return;
      }

  if (OutputFormat)
    {
    ch_output_format = OutputFormat - 1;

    count_field = Count;
    from_field = SrcStartCurve;

    if (change_execute(NULL, NULL) != FIELD_VALIDATE_SUCCESS)
      SetErrorFlag();
    }
  else /* write OMA binary */
    {
    /* copy the plotbox values to InitialMethod */
    CopyPlotToMethod();

    /* copy the scan values to the method */   
    if (DetInfoToMethod(&InitialMethod))
      return;

    if (WriteTempCurveBlkToFile(ch_output_fname,
                                DstStartCurve,
                                InitialMethod,
                                &MainCurveDir,
                                DstCurve.CurveSetIndex,
                                SrcStartCurve,
                                Count))
      SetErrorFlag();

    /* get rid of copied detector data from the method*/
    DeAllocMetDetInfo(InitialMethod);
    }
}

void InsCurve(void)    
{
  SHORT BadParam = 0;
  CURVE_REF SrcCurve, DstCurve;
  CURVE_REF *pCurve;

  if (PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO) != TYPE_CURVE)
    BadParam = 2;
  else
    DstCurve = * pCurve;

  if (! BadParam)
    {
    if (PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO) != TYPE_CURVE)
      BadParam = 1;
    else
      SrcCurve = * pCurve;
    }
  else
    {
    MacBadParam(BadParam);
    return;
    }

  if (clearAllCurveBufs() ||
      InsertMultiTempCurve(&MainCurveDir, SrcCurve.CurveSetIndex,
                           SrcCurve.CurveIndex,
                           DstCurve.CurveSetIndex,
                           DstCurve.CurveIndex, 1))
    SetErrorFlag();
}
  
void InsCurveSet(void)
{
  SHORT BadParam = 0;
  CURVE_REF SrcCurve, DstCurve;
  CURVE_REF *pCurve;
  USHORT DstStartCurve, Count;
  PCHAR DstPath, DstName;
  LPCURVE_ENTRY pSrcEntry, pDstEntry;
  ULONG NewOffset;

  if (! PopScalarFromDataStack(&DstStartCurve, TYPE_WORD))
    BadParam = 6;
  else if (! PopScalarFromDataStack(&DstPath, POINTER_TO | TYPE_STRING))
    BadParam = 5;
  else if (! PopScalarFromDataStack(&DstName, POINTER_TO | TYPE_STRING))
    BadParam = 4;
  else
    {
    if (PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO) != TYPE_CURVE)
      BadParam = 3;
    else
      DstCurve = * pCurve;
    }
  if (! BadParam)
    {
    if (! PopScalarFromDataStack(&Count, TYPE_WORD))
      BadParam = 2;
    else if (PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO) !=
      TYPE_CURVE)
      BadParam = 1;
    else
      SrcCurve = * pCurve;
    }
  else
    {
    MacBadParam(BadParam);
    return;
    }

  pSrcEntry = &(MainCurveDir.Entries[SrcCurve.CurveSetIndex]);
  if (DstCurve.CurveSetIndex < MainCurveDir.BlkCount)
    {
    pDstEntry = &(MainCurveDir.Entries[DstCurve.CurveSetIndex]);

    /* if inserting in the middle of a curve block, the DstEntry must be */
    /* split up */
    if ((DstCurve.CurveIndex > pDstEntry->StartIndex) &&
        (DstCurve.CurveIndex < (pDstEntry->StartIndex + pDstEntry->count)))
      {
      /* split the block entry already present */
      if(SplitCurveBlk(DstCurve.CurveSetIndex, DstCurve.CurveIndex,
                       & MainCurveDir))
        {
        SetErrorFlag();
        return;
        }
      /* insert before the second half of split entry */
      DstCurve.CurveSetIndex++;
      }
    NewOffset = MainCurveDir.Entries[DstCurve.CurveSetIndex].TmpOffset;
    }
  else
    NewOffset = TempFileSz;

  /* make sure of string size */
  DstName[DOSFILESIZE] = '\0';
  DstPath[DOSPATHSIZE] = '\0';
  /* add a new entry for the new curve block */
  if(InsertCurveBlkInDir(DstName, DstPath, pSrcEntry->descrip,
                         DstStartCurve, 0L, 0, &(pSrcEntry->time),
                         &MainCurveDir, DstCurve.CurveSetIndex,
                         pSrcEntry->EntryType))
    {
    SetErrorFlag();
    return;
    }

  /* the entry has been added, need to put in the offset into the temp */
  /* file */
  MainCurveDir.Entries[DstCurve.CurveSetIndex].TmpOffset = NewOffset;

  /* DstEntry is now a copy of the SrcEntry */
  /* DstEntry + 1 is now the old DstEntry */
  
  if(InsertMultiTempCurve(&MainCurveDir, SrcCurve.CurveSetIndex,
                          SrcCurve.CurveIndex, DstCurve.CurveSetIndex,
                          DstStartCurve, Count))
    SetErrorFlag();
}
  
void PlotWindow(void)
{
  SHORT WindowNum;
  CHAR *DeviceName;
  int ErrorIndex = 0;

  if (! PopScalarFromDataStack(&WindowNum, TYPE_INTEGER))
    ErrorIndex = 2;
  if ((PopFromDataStack(&DeviceName,
    POINTER_TO | TYPE_STRING) & ~POINTER_TO)!= TYPE_STRING)
    ErrorIndex = 1;

  if (ErrorIndex)
    {
    error(ERROR_BAD_PARAM_TYPE, ErrorIndex);
    SetErrorFlag();
    return;
    }

  WindowNum--;  /* make it 0 based*/

  if ((WindowNum >= fullscreen_count[window_style]) || (WindowNum < 0))
    {
    error(ERROR_BAD_PARAM_RANGE, 2);
    SetErrorFlag();
    return;
    }

  if((plotWindowToDevice(WindowNum, DeviceName)) != ERROR_NONE)
    SetErrorFlag();
}
 
/* axis and plot titles have 29 char max */
void SPlotStyle(void)
{
  BOOLEAN AutoScaleX, AutoScaleY, AutoScaleZ;
  SHORT LineStyle, ZYPercent, ZXPercent, ZSide;
  CHAR *ZLabel, *YLabel, *XLabel, *PlotTitle;
  USHORT ZUnits, YUnits, XUnits;
  DOUBLE ZMax, ZMin, YMax, YMin, XMax, XMin;
  PLOTBOX *pPlot;
  SHORT ErrorIndex = 0;

  pPlot = &Plots[ActiveWindow];

  if (! PopScalarFromDataStack(&AutoScaleZ, TYPE_BOOLEAN))
    ErrorIndex = 20;
  if (! PopScalarFromDataStack(&AutoScaleY, TYPE_BOOLEAN))
    ErrorIndex = 19;
  if (! PopScalarFromDataStack(&AutoScaleX, TYPE_BOOLEAN))
    ErrorIndex = 18;
  else if (! PopScalarFromDataStack(&LineStyle, TYPE_INTEGER))
    ErrorIndex = 17;
  else if (! PopScalarFromDataStack(&ZYPercent, TYPE_INTEGER))
    ErrorIndex = 16;
  else if (! PopScalarFromDataStack(&ZXPercent, TYPE_INTEGER))
    ErrorIndex = 15;
  else if (! PopScalarFromDataStack(&ZSide, TYPE_INTEGER))
    ErrorIndex = 14;
  else if (! PopScalarFromDataStack(&ZLabel, POINTER_TO | TYPE_STRING))
    ErrorIndex = 13;
  else if (! PopScalarFromDataStack(&YLabel, POINTER_TO | TYPE_STRING))
    ErrorIndex = 12;
  else if (! PopScalarFromDataStack(&XLabel, POINTER_TO | TYPE_STRING))
    ErrorIndex = 11;
  else if (! PopScalarFromDataStack(&PlotTitle, POINTER_TO | TYPE_STRING))
    ErrorIndex = 10;
  else if (! PopScalarFromDataStack(&ZUnits, TYPE_WORD))
    ErrorIndex = 9;
  else if (! PopScalarFromDataStack(&YUnits, TYPE_WORD))
    ErrorIndex = 8;
  else if (! PopScalarFromDataStack(&XUnits, TYPE_WORD))
    ErrorIndex = 7;
  else if (! PopScalarFromDataStack(&ZMax, TYPE_REAL))
    ErrorIndex = 6;
  else if (! PopScalarFromDataStack(&ZMin, TYPE_REAL))
    ErrorIndex = 5;
  else if (! PopScalarFromDataStack(&YMax, TYPE_REAL))
    ErrorIndex = 4;
  else if (! PopScalarFromDataStack(&YMin, TYPE_REAL))
    ErrorIndex = 3;
  else if (! PopScalarFromDataStack(&XMax, TYPE_REAL))
    ErrorIndex = 2;
  else if (! PopScalarFromDataStack(&XMin, TYPE_REAL))
    ErrorIndex = 1;

  if (ErrorIndex)
    {
    MacBadParam(ErrorIndex);
    return;
    }

  autoscale_x = AutoScaleX;
  autoscale_y = AutoScaleY;
  autoscale_z = AutoScaleZ;
  if (LineStyle > 2)
    pPlot->plot_line_type = LineStyle - 2;
  else
    pPlot->style = LineStyle;

  pPlot->yz_percent = ZYPercent;
  pPlot->xz_percent = ZXPercent;
  pPlot->z_position = ZSide;

  strncpy(pPlot->z.legend, ZLabel, LEGEND_SIZE);
  pPlot->z.legend[ LEGEND_SIZE - 1 ] = '\0';

  strncpy(pPlot->y.legend, YLabel, LEGEND_SIZE);
  pPlot->y.legend[ LEGEND_SIZE - 1 ] = '\0';

  strncpy(pPlot->x.legend, XLabel, LEGEND_SIZE);
  pPlot->x.legend[ LEGEND_SIZE - 1 ] = '\0';

  strncpy(pPlot->title, PlotTitle, TITLE_SIZE);
  pPlot->title[ TITLE_SIZE - 1 ] = '\0';

  pPlot->z.units = ZUnits;
  pPlot->y.units = YUnits;
  pPlot->x.units = XUnits;

  pPlot->z.original_max_value = (FLOAT) ZMax;
  pPlot->z.original_min_value = (FLOAT) ZMin;
  pPlot->y.original_max_value = (FLOAT) YMax;
  pPlot->y.original_min_value = (FLOAT) YMin;
  pPlot->x.original_max_value = (FLOAT) XMax;
  pPlot->x.original_min_value = (FLOAT) XMin;

  initAxisToOriginal(& pPlot->x);
  initAxisToOriginal(& pPlot->y);
  initAxisToOriginal(& pPlot->z);

  ResizePlotForWindow(ActiveWindow);
  create_plotbox(pPlot);
}

/* axis and plot titles have 29 char max */
void GPlotStyle(void)
{
  BOOLEAN *AutoScaleX, *AutoScaleY, *AutoScaleZ;
  SHORT *LineStyle, *ZYPercent, *ZXPercent, *ZSide;
  CHAR *ZLabel, *YLabel, *XLabel, *PlotTitle;
  USHORT *ZUnits, *YUnits, *XUnits;
  DOUBLE *ZMax, *ZMin, *YMax, *YMin, *XMax, *XMin;
  PLOTBOX *pPlot;

  pPlot = &Plots[ActiveWindow];

  #define dsPTR(a) (a | POINTER_TO)
  #define isPTR (DataStackPeek(0) & POINTER_TO)

  #define getPTR(Var, Type)\
    (isPTR && PopScalarFromDataStack(&Var, dsPTR(Type)))

  if (!getPTR(AutoScaleZ, TYPE_BOOLEAN))
    MacBadParam(20);
  else if (!getPTR(AutoScaleY, TYPE_BOOLEAN))
    MacBadParam(19);
  else if (!getPTR(AutoScaleX, TYPE_BOOLEAN))
    MacBadParam(18);
  else if (!getPTR(LineStyle, TYPE_INTEGER))
    MacBadParam(17);
  else if (!getPTR(ZYPercent, TYPE_INTEGER))
    MacBadParam(16);
  else if (!getPTR(ZXPercent, TYPE_INTEGER))
    MacBadParam(15);
  else if (!getPTR(ZSide, TYPE_INTEGER))
    MacBadParam(14);
  else if (!getPTR(ZLabel, (POINTER_TO | TYPE_STRING)))
    MacBadParam(13);
  else if (!getPTR(YLabel, (POINTER_TO | TYPE_STRING)))
    MacBadParam(12);
  else if (!getPTR(XLabel, (POINTER_TO | TYPE_STRING)))
    MacBadParam(11);
  else if (!getPTR(PlotTitle, (POINTER_TO | TYPE_STRING)))
    MacBadParam(10);
  else if (!getPTR(ZUnits, TYPE_WORD))
    MacBadParam(9);
  else if (!getPTR(YUnits, TYPE_WORD))
    MacBadParam(8);
  else if (!getPTR(XUnits, TYPE_WORD))
    MacBadParam(7);
  else if (!getPTR(ZMax, TYPE_REAL))
    MacBadParam(6);
  else if (!getPTR(ZMin, TYPE_REAL))
    MacBadParam(5);
  else if (!getPTR(YMax, TYPE_REAL))
    MacBadParam(4);
  else if (!getPTR(YMin, TYPE_REAL))
    MacBadParam(3);
  else if (!getPTR(XMax, TYPE_REAL))
    MacBadParam(2);
  else if (!getPTR(XMin, TYPE_REAL))
    MacBadParam(1);
  else
    {
    *AutoScaleX = autoscale_x;
    *AutoScaleY = autoscale_y;
    *AutoScaleZ = autoscale_z;

    *LineStyle = pPlot->plot_line_type;
    *ZYPercent = pPlot->yz_percent;
    *ZXPercent = pPlot->xz_percent;
    *ZSide = pPlot->z_position;

    strncpy(ZLabel, pPlot->z.legend, LEGEND_SIZE);
    strncpy(YLabel, pPlot->y.legend, LEGEND_SIZE);
    strncpy(XLabel, pPlot->x.legend, LEGEND_SIZE);
    strncpy(PlotTitle, pPlot->title, TITLE_SIZE);

    *ZUnits = pPlot->z.units;
    *YUnits = pPlot->y.units;
    *XUnits = pPlot->x.units;

    *ZMax = pPlot->z.max_value;
    *ZMin = pPlot->z.min_value;
    *YMax = pPlot->y.max_value;
    *YMin = pPlot->y.min_value;
    *XMax = pPlot->x.max_value;
    *XMin = pPlot->x.min_value;
    }
}

/* syntax: Units := G_CALIB(VAR A0, A1, A2, A3, ExWl); */
/******************************************************************/
void MacGetCal(void)
{
  DOUBLE *A0, *A1, *A2, *A3, *ExWl;
  USHORT Units;

  if (PopFromDataStack(&ExWl, TYPE_REAL | POINTER_TO) != TYPE_REAL)
    MacBadParam(5);

  else if (PopFromDataStack(&A3, TYPE_REAL | POINTER_TO) != TYPE_REAL)
    MacBadParam(4);

  else if (PopFromDataStack(&A2, TYPE_REAL | POINTER_TO) != TYPE_REAL)
    MacBadParam(3);

  else if (PopFromDataStack(&A1, TYPE_REAL | POINTER_TO) != TYPE_REAL)
    MacBadParam(2);

  else if (PopFromDataStack(&A0, TYPE_REAL | POINTER_TO) != TYPE_REAL)
    MacBadParam(1);

  else
    {
    *A0 = InitialMethod->CalibCoeff[0][0];
    *A1 = InitialMethod->CalibCoeff[0][1];
    *A2 = InitialMethod->CalibCoeff[0][2];
    *A3 = InitialMethod->CalibCoeff[0][3];
    *ExWl = InitialMethod->Excitation;
    Units = (USHORT)InitialMethod->CalibUnits[0];
  
    PushToDataStack(&Units, TYPE_INTEGER, FALSE);
    }
}
  
/* syntax: Units := G_CALIB(VAR A0, A1, A2, A3, ExWl); */
/******************************************************************/
void MacSetCal(void)
{
  DOUBLE A0, A1, A2, A3, ExWl;
  LONG Units;

  if (!PopScalarFromDataStack(&ExWl, TYPE_REAL))
    MacBadParam(6);

  else if (!PopScalarFromDataStack(&A3, TYPE_REAL))
    MacBadParam(5);

  else if (!PopScalarFromDataStack(&A2, TYPE_REAL))
    MacBadParam(4);

  else if (!PopScalarFromDataStack(&A1, TYPE_REAL))
    MacBadParam(3);

  else if (!PopScalarFromDataStack(&A0, TYPE_REAL))
    MacBadParam(2);

  else if (!PopScalarFromDataStack(&Units, TYPE_INTEGER))
    MacBadParam(1);

  else
    {
    InitialMethod->CalibCoeff[0][0] = (float)A0;
    InitialMethod->CalibCoeff[0][1] = (float)A1;
    InitialMethod->CalibCoeff[0][2] = (float)A2;
    InitialMethod->CalibCoeff[0][3] = (float)A3;
    InitialMethod->Excitation  = (float)ExWl; 
    InitialMethod->CalibUnits[0] = (UCHAR)Units;
    }
}
  
/**********************************************************************/
/*                                                                    */
/* Copy Count curves from Curveset to special background curveset     */
/* If Count is -1, copy all of the curves from Curveset               */
/* If curveset is a single curve, set Count to 1                      */
/*                                                                    */
/**********************************************************************/

void LoadBackground(void)   
{
  CHAR BlockName[FNAME_LENGTH];
  USHORT Count = 0;
  CURVE_REF Curve;
  CURVE_REF *pCurve;
  LPCURVE_ENTRY pEntry;
  int ErrorIndex = 0;

  if (!PopScalarFromDataStack(&Count, TYPE_WORD))
    ErrorIndex = 2;
  else if ((PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO) == TYPE_CURVE))
    {
    Curve = * pCurve;
    pEntry = &(MainCurveDir.Entries[Curve.CurveSetIndex]);
    if (Curve.ReferenceType == CLASS_CURVESET)
      {
      Curve.CurveIndex = pEntry->StartIndex;
      if (Count < 1)
        Count = pEntry->count;
      }
    else if (Curve.ReferenceType == CLASS_CURVE)
      Count = 1;
    }
  if (Count < 1)
    ErrorIndex = 1;

  if (ErrorIndex)
    {
    MacBadParam(ErrorIndex);
    return;
    }

  strcpy(BlockName, pEntry->path);
  strcat(BlockName, pEntry->name);

  if(CopyToBackGround(BlockName, Curve.CurveIndex, Count))
    SetErrorFlag();
}
  
/**********************************************************************/
/*                                                                    */
/*   calc_curves_to_dump - 07May1990 Morris Maynard                   */
/*                      find out how many spectra must be received    */
/*                      for a 'post-Accum' transfer.                  */
/*                                                                    */
/**********************************************************************/
  
static void calc_curves_to_dump(USHORT * curves)
{
  *curves = det_setup->memories * det_setup->tracks;
}
  
static BOOLEAN checkout_curve_entry(LPCURVE_ENTRY pEntry)
{
   SHORT i;
  
   /* test for unique name, path, and curve number*/
  
   switch(CheckCurveBlkOverLap(pEntry->path, pEntry->name,
                                pEntry->StartIndex + pEntry->count,
                                pEntry->StartIndex + pEntry->count, & i))
   {
      case RANGEOK:
      case SPLITRANGE:
      case OVERLAPCAT:
         SetErrorFlag();
         return(TRUE);
      case NOOVERLAPCAT:
      case DISJOINT:
      case BADNAME:
         break;               /* OK a unique name and range was given*/
   }
   return FALSE;
}
  
ERR_OMA CreateReadMemCurve(CURVE_REF *DstCurve, CURVEHDR *Curvehdr,
                                  SHORT Points, SHORT tracks)
{
  ERR_OMA err;
  time_t Time;
 
  /* Put in an new curve with the given defaults */
  Curvehdr->pointnum = Points;
  Curvehdr->XData.XUnits = InitialMethod->CalibUnits[0]; 
  Curvehdr->YUnits = COUNTS;
  Curvehdr->DataType = LONGTYPE;
  Curvehdr->experiment_num = 0;
  
  Curvehdr->Xmin = ApplyCalibrationToX(InitialMethod->CalibCoeff[0], 0.0F);
  Curvehdr->Xmax = ApplyCalibrationToX(InitialMethod->CalibCoeff[0], (FLOAT)Points);

  time(&Time);
  Curvehdr->time = (FLOAT) Time;
 
  err = AddCurveSpaceToTempBlk(&MainCurveDir, DstCurve->CurveSetIndex,
                               DstCurve->CurveIndex, tracks, Curvehdr);
 
  if (! err)
    err = clearAllCurveBufs();  /* flush all temporary buffers*/

  return err;
}
  
/*****************************************************************/
/*  DReadMem - d_read_mem(mem, point, #points, curve);           */
/*                                                               */
/*  If CCD and multiple tracks set up, all tracks in frame       */
/*  are received (or till curve set is full).                    */
/*                                                               */
/*  Curves are inserted in front of current curves in set.       */
/*                                                               */
/*****************************************************************/
#pragma optimize("",off)
void DReadMem(void)            
{
  USHORT StartPoint, Mem, tracks, len, i;
  CURVEHDR Curvehdr;
  CURVE_REF DstCurve, *pCurve;
  LPCURVE_ENTRY pEntry;
  SHORT BadParam = 0, Points, curve_num,
        Type, real_detector, prefBuf = 0;

  Type = PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO);

  DstCurve = * pCurve;

  if ((Type != TYPE_CURVE) || (DstCurve.ReferenceType != CLASS_CURVE))
    BadParam = 4;
  else if (! PopScalarFromDataStack(&Points, TYPE_INTEGER))
    BadParam = 3;
  else if (! PopScalarFromDataStack(&StartPoint, TYPE_INTEGER))
    BadParam = 2;
  else if (! PopScalarFromDataStack(&Mem, TYPE_INTEGER))
    BadParam = 1;
  else BadParam = 0;

  get_RealDetector(&real_detector);
  if (!real_detector)
    {
    SetErrorFlag();
    error(ERROR_FAKEDETECTOR);
    return;
    }

  if (BadParam)
    {
    MacBadParam(BadParam);
    return;
    }

  pEntry = &(MainCurveDir.Entries[DstCurve.CurveSetIndex]);
  len = Points * (sizeof (FLOAT));

  if (checkout_curve_entry(pEntry))
    return;

  calc_curves_to_dump(&tracks);

  if(CreateReadMemCurve(&DstCurve, &Curvehdr, Points, tracks))
    return;

  for (i = 0; i < tracks; i++)
    {
    if (LoadCurveBuffer(&MainCurveDir, DstCurve.CurveSetIndex,
                        i, &StartPoint, &prefBuf))
      {
      SetErrorFlag();
      return;
      }

    curve_num = Mem * tracks + i;

    /* receive the data */
    ReadCurveFromMem(CvBuf[prefBuf].BufPtr, len, curve_num);

    /* force FlushCurveBuffer() to write to disk */
    CvBuf[prefBuf].status = CVBUF_DIRTY;  
                                    
    if(FlushCurveBuffer(prefBuf))
      {
      SetErrorFlag();
      return;
      }
    }
}
#pragma optimize("", on)
  
/*****************************************************************/
/*                                                               */
/*  DLoadMem - d_load_mem(mem, point, #points, curve);           */
/*                                                               */
/*  Loads data into detector memory                              */
/*                                                               */
/*  If CCD and multiple tracks set up, all tracks in frame       */
/*  are sent (till end of valid data in curve set.)              */
/*                                                               */
/*****************************************************************/
void DLoadMem(void)
{
  CURVE_REF DstCurve, *pCurve;
  USHORT StartPoint, Mem, tracks, curve_num, i, data_type;
  SHORT len, Points, j, BadParam = 0, prefBuf = 0, Type;
  CURVEBUFFER *pBuf;

  union
    {
    CHAR * bData;
    SHORT * wData;
    FLOAT * fData;
    LONG * lData;
    } pData;

  Type = PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO);

  DstCurve = * pCurve;

  if ((Type != TYPE_CURVE) || (DstCurve.ReferenceType != CLASS_CURVE))
    BadParam = 4;
  else if (! PopScalarFromDataStack(&Points, TYPE_INTEGER))
    BadParam = 3;
  else if (! PopScalarFromDataStack(&StartPoint, TYPE_INTEGER))
    BadParam = 2;
  else if (! PopScalarFromDataStack(&Mem, TYPE_INTEGER))
    BadParam = 1;
  else BadParam = 0;

  {
  SHORT real_detector;
  get_RealDetector(&real_detector);
  if (!real_detector)
    {
    SetErrorFlag();
    error(ERROR_FAKEDETECTOR);
    return;
    }

  if (BadParam)
    {
    MacBadParam(BadParam);
    return;
    }
  }

  len = Points * ((det_setup->data_word_size+1)*2);

  calc_curves_to_dump(&tracks);

  for (i = 0; i < tracks;i ++)
    {
    if (LoadCurveBuffer(&MainCurveDir, DstCurve.CurveSetIndex,
                        i, &StartPoint, &prefBuf))
      {
      SetErrorFlag();
      break;
      }
    pBuf = &CvBuf[prefBuf];
    data_type = pBuf->Curvehdr.DataType;

    pData.bData = pBuf->BufPtr;

    if (data_type == FLOATTYPE)    /* OMA doesn't know from FLOATS */
      {
      for (j = 0;j < Points; j++)
        {
        pData.lData[j] = (LONG)(pData.fData[j]);    /* so do FIX op */
        }
      }

    /* if PC data is 4-byte but OMA wants 2-byte convert to SHORT */
    
    if ((data_type == FLOATTYPE || data_type == LONGTYPE) &&    
         det_setup->data_word_size == 0)                     
      {
      for (j = 0;j < Points;j++)                   /* if was float, is now */
        {                                          /* long anyway */
        pData.wData[j] = (SHORT)(pData.lData[j]);
        }
      }

    /* if PC data is 2-byte but OMA wants 4-byte, convert to long */
    if ((data_type == SHORTTYPE) && det_setup->data_word_size == 1)        
      {                                             
      for (j = Points - 1;j != 0; j--)
        {
        pData.lData[j] = (LONG)(pData.wData[j]);
        }
      }

    curve_num = Mem * tracks + i;                  /* Mem is 0 based */

    WriteCurveToMem(pBuf->BufPtr, len, curve_num); /* write the data */
    }
  pBuf->ActiveDir = NULL;
}

/**************************************************************************/  
/*                                                                        */  
/* Make a new curve in an existing curve set; type is always 8 byte float */
/*                                                                        */  
/**************************************************************************/  

/* YFill is an unreferenced formal parameter*/

ERR_OMA create_new_curve(BOOLEAN XCal, SHORT Curves, SHORT Points,
                                SHORT SetIndex, SHORT CrvIndex, DOUBLE YFill)
{
  LPCURVE_ENTRY pEntry;
  ERR_OMA err = ERROR_NONE;
  SHORT i;
  static CURVEHDR Curvehdr;
  time_t Time;
  FLOAT PixelCalCoeff[4];
  PFLOAT pCal;

  PixelCalCoeff[0] = (FLOAT)0;
  PixelCalCoeff[1] = (FLOAT)1;
  PixelCalCoeff[2] = (FLOAT)0;
  PixelCalCoeff[3] = (FLOAT)0;

  if (SetIndex < 0 || SetIndex > (SHORT)MainCurveDir.BlkCount)
    return(error(ERROR_BAD_CURVE_BLOCK, SetIndex));
  
  pEntry = &(MainCurveDir.Entries[SetIndex]);

  switch (CheckCurveBlkOverLap(pEntry->path, pEntry->name,
    pEntry->StartIndex + pEntry->count,
    pEntry->StartIndex + pEntry->count, &i))
    {
    case RANGEOK:
    case SPLITRANGE:
    case OVERLAPCAT:
     return ERROR_NONE;
    case NOOVERLAPCAT:
    case DISJOINT:
    case BADNAME:
      break;               /* OK a unique name and range was given*/
    }

  /* Put in a new curve with the given defaults */
  Curvehdr.pointnum = Points;
  if (XCal)
    Curvehdr.XData.XUnits = InitialMethod->CalibUnits[0];
  else
    Curvehdr.XData.XUnits = COUNTS;
  Curvehdr.YUnits = InitialMethod->CalibUnits[1];
  Curvehdr.DataType = FLOATTYPE;
  Curvehdr.experiment_num = 0;
  time(&Time);
  Curvehdr.time = (FLOAT) Time;
  Curvehdr.CurveCount = 1;
  Curvehdr.MemData = FALSE;
  Curvehdr.Ymin = (FLOAT) 0;
  Curvehdr.Ymax = (FLOAT) 0;

  err = AddCurveSpaceToTempBlk(&MainCurveDir, SetIndex,
                               CrvIndex, Curves, &Curvehdr);
  
  if (! err)
    {
    if (XCal)
      pCal = InitialMethod->CalibCoeff[0];
    else
      pCal = PixelCalCoeff;

    err = ApplyCalibrationToCurve(&MainCurveDir, SetIndex, CrvIndex, pCal,
                                  Curvehdr.XData.XUnits);
    }
  return err;
}

void CreateCurve(void)
{
  CURVE_REF DstCurve;
  CURVE_REF *pCurve;
  SHORT BadParam = 0;
  SHORT Type;
  SHORT Points;
  BOOLEAN XCal;
  DOUBLE YFillVal;

  Type = PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO);
  DstCurve = * pCurve;
  if ((Type != TYPE_CURVE) || (DstCurve.ReferenceType != CLASS_CURVE))
    BadParam = 4;
  else if (! PopScalarFromDataStack(&XCal, TYPE_BOOLEAN))
    BadParam = 3;
  else if (! PopScalarFromDataStack(&YFillVal, TYPE_REAL))
    BadParam = 2;
  else if (! PopScalarFromDataStack(&Points, TYPE_WORD))
    BadParam = 1;

  if (BadParam)
    MacBadParam(BadParam);
  else if (Points == 0)
    {
    error(ERROR_BAD_PARAM_RANGE, 1);
    SetErrorFlag();
    }
  else if(create_new_curve(XCal, 1, Points, DstCurve.CurveSetIndex,
                           DstCurve.CurveIndex, YFillVal))
    SetErrorFlag();
}
