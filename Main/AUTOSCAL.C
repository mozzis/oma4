/* -----------------------------------------------------------------------
/
/  autoscal.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/autoscal.c_v   1.3   06 Jul 1992 10:24:00   maynard  $
/  $Log:   J:/logfiles/oma4000/main/autoscal.c_v  $
/
*/
  
#include "autoscal.h"
#include "tempdata.h"    // ReadTempCurvehdr()
#include "di_util.h"     // ConvertUnits()
#include "calib.h"       // ApplyCalibrationToX()
#include "formwind.h"    // release_message_window()
#include "pltsetup.h"    // scalePlotbox
#include "live.h"        // InLiveLoop
#include "cursor.h"      // CursorStatus
#include "multi.h"       // WindowPlotAssignment
#include "omaform.h"     // COLORS_MESSAGE
#include "crventry.h"
#include "omameth.h"     // InitialMethod
#include "baslnsub.h"    // baslnsub_active()
#include "macrecor.h"    // MacRecordString
#include "doplot.h"      // create_plotbox()
#include "plotbox.h"
#include "curvedir.h"    // MainCurveDir
#include "curvbufr.h"
#include "tagcurve.h"    // ExpandedOnTagged
#include "curvdraw.h"    // plot_curves()
#include "forms.h"       // pKSRecord
#include "baslnsub.h"    // WhichAutoscaleGetFunc()
// the following is needed only for GRAPHICAL scan setup
//#include "scangraf.h"    // isScanGrafMode
  
#ifdef XSTAT
  #define PRIVATE
#else
  #define PRIVATE static
#endif
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE ERR_OMA CurveToPlotMaxMin(PLOTBOX *pPlotBox,
                                         SHORT EntryIndex,
                                         USHORT CurveIndex)
  {
  USHORT XMinIndex, XMaxIndex, i;
  SHORT prefBuf = -1;
  DOUBLE XMax, XMin, Temp;
  FLOAT YMin, YMax, YVal, XVal;
  ERR_OMA err;
  CURVEHDR TCurvehdr;
  GET_POINT_FUNC * get_point = WhichAutoscaleGetFunction();

  if(err= ReadTempCurvehdr(&MainCurveDir, EntryIndex, CurveIndex, &TCurvehdr))
    return err;
  
  if(autoscale_x)    /* Always query data */
    // use the following for GRAPHICAL scan setup
    //   if(autoscale_x && ! (* isScanGrafMode))    /* Always query data */
    {
    if (pPlotBox->x.units != COUNTS)
      {
      if (TCurvehdr.XData.XUnits == COUNTS)
        {
        XMin = (DOUBLE) 0;
        XMax = (DOUBLE) TCurvehdr.pointnum - 1;
  
        if (InitialMethod->CalibUnits[0] != COUNTS)
          {
          XMin = (DOUBLE)
          ApplyCalibrationToX(InitialMethod->CalibCoeff[0], (FLOAT) XMin);
  
          err = ConvertUnits((UCHAR) pPlotBox->x.units, &XMin,
                         InitialMethod->CalibUnits[0], XMin,
                         (double) InitialMethod->Excitation);
          if (err)
            return err;

  
          XMax =
            (DOUBLE)ApplyCalibrationToX(InitialMethod->CalibCoeff[0],(FLOAT)XMax);
  
          err = ConvertUnits((UCHAR) pPlotBox->x.units, &XMax,
                         InitialMethod->CalibUnits[0], XMax,
                         (double) InitialMethod->Excitation);
          if (err)
            return err;
          }
        }
      else
        {
        XMin = (DOUBLE) TCurvehdr.Xmin;
        XMax = TCurvehdr.Xmax;
  
        err = ConvertUnits((UCHAR) pPlotBox->x.units, &XMin, TCurvehdr.XData.XUnits,
                     XMin, (double)InitialMethod->Excitation);
  
        if (err)
          return err;

        err = ConvertUnits((UCHAR) pPlotBox->x.units, &XMax, TCurvehdr.XData.XUnits,
                     XMax, (double)InitialMethod->Excitation);
        if (err)
          return err;
        }
  
      /* check for conversion caused order switch */
      if (XMin > XMax)
        {
        Temp = XMin;
        XMin = XMax;
        XMax = Temp;
        }
      }
    else // this will now plot out as the number of points
      {
      XMin = (FLOAT) 0;
      XMax = (FLOAT) (TCurvehdr.pointnum - 1);
      }
    }
  
  // use the following for GRAPHICAL scan graf mode
  //  if(autoscale_x && * isScanGrafMode)    // force x axis to detector size
  //    {
  //    XMin = (float) 0;
  //    XMax = (float) scanGrafNumChannels();
  //    }
  
  if(autoscale_x)
    {
    if(pPlotBox->x.original_min_value > (FLOAT) XMin)
      pPlotBox->x.original_min_value = (FLOAT) XMin;
    if(pPlotBox->x.original_max_value < (FLOAT) XMax)
      pPlotBox->x.original_max_value = (FLOAT) XMax;
    }
  
  if (autoscale_y) // autoscale Y on partial curve
    {
    /* find the point indices for the data points closest to the */
    /* plot's min and max X axis values.                         */
  
    // if autoscaling x, use the new autoscaled x values for autoscaling y.
    // if not autoscaling x, use the current x values for autoscaling y.
    if(autoscale_x)
      {
      XVal = pPlotBox->x.original_min_value;
      XMax = pPlotBox->x.original_max_value;
      }
    else
      {
      XVal = pPlotBox->x.min_value;
      XMax = pPlotBox->x.max_value;
      }
  
    XMinIndex = 0;
    if (err = GetTempCurvePointIndex(&MainCurveDir, EntryIndex, CurveIndex,
                                     &XVal, &YVal, (UCHAR) pPlotBox->x.units,
                                     &XMinIndex, prefBuf))
      return err;
  
    XVal = (FLOAT)XMax;
    XMaxIndex = TCurvehdr.pointnum - 1;
    if (err = GetTempCurvePointIndex(&MainCurveDir, EntryIndex, CurveIndex,
                                     &XVal, &YVal, (UCHAR) pPlotBox->x.units,
                                     &XMaxIndex, prefBuf))
      return err;
  
    // x axis is not necessarily ascending
    if(XMinIndex > XMaxIndex)
      {
      i = XMinIndex;
      XMinIndex = XMaxIndex;
      XMaxIndex = i;
      }
  
    // same goes for Y axis
    if(pPlotBox->style != FALSE_COLOR)
      {
      YMin = pPlotBox->y.original_min_value;
      YMax = pPlotBox->y.original_max_value;
      }
    else
      {
      YMin = pPlotBox->z.original_min_value;
      YMax = pPlotBox->z.original_max_value;
      }
    
    for (i = XMinIndex; i<= XMaxIndex; i++)
      {
      if (err = (* get_point)(&MainCurveDir, EntryIndex, CurveIndex,
                              i, &XVal, &YVal, FLOATTYPE, &prefBuf))
        return err;
  
      if (YMin > YVal)
        YMin = YVal;
      if (YMax < YVal)
        YMax = YVal;
      }
   
    if(pPlotBox->style != FALSE_COLOR)
      {
      pPlotBox->y.original_min_value = YMin;
      pPlotBox->y.original_max_value = YMax;
      }
    else
      {
      pPlotBox->z.original_min_value = YMin;
      pPlotBox->z.original_max_value = YMax;
      }
    }
  return ERROR_NONE;
}
  
/* -----------------------------------------------------------------------
/
/  ERR_OMA AutoScalePlotBox(LPCURVEDIR pCurveDir, PLOTBOX *pPlotBox,
/                    SHORT DisplayWindow)
/
/  requires: pCurveDir - pointer to curvedirectory to be displayed
/            pPlotBox - pointer to active plotbox structure
/            DisplayWindow - bit indicator (0, 1, 2, 3, 4, 5, 6, etc.)
/                            showing active display window
/  returns:    error value
/
/ ----------------------------------------------------------------------- */
ERR_OMA AutoScalePlotBox(LPCURVEDIR pCurveDir, PLOTBOX *pPlotBox,
                                SHORT DisplayWindow)
  {
  ERR_OMA err = ERROR_NONE;
  USHORT ZMax, ZMin, ZTemp;
  USHORT CurveNum = 0;
  USHORT entryIndex;
  USHORT curveIndex;
  BOOLEAN falseColor = (pPlotBox->style == FALSE_COLOR);
  BOOLEAN ZSwap = FALSE;
  float savedXOriginalMin = pPlotBox->x.original_min_value;
  float savedXOriginalMax = pPlotBox->x.original_max_value;
  float savedYOriginalMin;
  float savedYOriginalMax;
//  float savedZOriginalMin = pPlotBox->z.original_min_value;
//  float savedZOriginalMax = pPlotBox->z.original_max_value;
  
  if (autoscale_x)
    {
    pPlotBox->x.original_min_value = (FLOAT) MAXFLOAT;
    pPlotBox->x.original_max_value = (FLOAT) MINFLOAT;
    }
  
  if (autoscale_y)
    {
    if(! falseColor)
      {
      savedYOriginalMin = pPlotBox->y.original_min_value;
      savedYOriginalMax = pPlotBox->y.original_max_value;
      pPlotBox->y.original_min_value = (FLOAT) MAXFLOAT;
      pPlotBox->y.original_max_value = (FLOAT) MINFLOAT;
      }
    else
      {
      savedYOriginalMin = pPlotBox->z.original_min_value;
      savedYOriginalMax = pPlotBox->z.original_max_value;
      pPlotBox->z.original_min_value = (FLOAT) MAXFLOAT;
      pPlotBox->z.original_max_value = (FLOAT) MINFLOAT;
      }
    }
  
  if(! falseColor)
    {
    ZMax = (USHORT) pPlotBox->z.original_max_value;
    ZMin = (USHORT) pPlotBox->z.original_min_value;
    }
  else
    {
    ZMax = (USHORT) pPlotBox->y.original_max_value;
    ZMin = (USHORT) pPlotBox->y.original_min_value;
    }
  
  if (autoscale_z)
    {
    if(! falseColor)
      {
      pPlotBox->z.original_min_value = (FLOAT) 0;
      pPlotBox->z.original_max_value = (FLOAT) 0;
      }
    else
      {
      pPlotBox->y.original_min_value = (FLOAT) 0;
      pPlotBox->y.original_max_value = (FLOAT) 0;
      }
    }
  else       // initialize curve scaling limits
    {
    if (pPlotBox->z_position != NOSIDE)
      {
      if(! falseColor)
        {
        ZMax = (USHORT) pPlotBox->z.max_value;
        ZMin = (USHORT) pPlotBox->z.min_value;
        }
      else
        {
        ZMax = (USHORT) pPlotBox->y.max_value;
        ZMin = (USHORT) pPlotBox->y.min_value;
        }
      }
    }
  if (ZMax < ZMin)
    {
    ZTemp = ZMin;
    ZMin = ZMax;
    ZMax = ZTemp;
    ZSwap = TRUE;
    }
  
  /* get min and max X, Y, and Z */
  if (! InLiveLoop)
    {
    if (! (autoscale_x || autoscale_y || autoscale_z))
      return ERROR_NONE;
  
    if (ExpandedOnTagged)
      {
      USHORT  UTDisplayIndex;
      BOOLEAN Found = FindFirstPlotBlock(&MainCurveDir, &entryIndex,
                                         &curveIndex, &UTDisplayIndex,
                                         DisplayWindow);
      while (Found)
        {
        // Scale X and Y if the curve is to be in the Z display range
        if((autoscale_x || autoscale_y) &&
           (autoscale_z || ((CurveNum <= ZMax) && (CurveNum >= ZMin))))
          if(err = CurveToPlotMaxMin(pPlotBox, entryIndex, curveIndex))
            return err;
  
        CurveNum++;
        if (autoscale_z)
          if(! falseColor)
            pPlotBox->z.original_max_value++;
          else
            pPlotBox->y.original_max_value++;
  
        Found = FindNextPlotCurve(&MainCurveDir, &entryIndex,
                                  &curveIndex, &UTDisplayIndex,
                                  DisplayWindow);
        }
      }
    else
      {
      WINDOW * MessageWindow;
      LPCURVE_ENTRY pEntry;
      USHORT i;
      USHORT j;
  
      put_up_message_window(BusyWorking, COLORS_MESSAGE, &MessageWindow);
      for (i=0; i<pCurveDir->BlkCount; i++)
        {
        pEntry = &(pCurveDir->Entries[i]);
        if (pEntry->DisplayWindow &
          (1 << WindowPlotAssignment[DisplayWindow]))
          {
          for (j=pEntry->StartIndex;j<pEntry->StartIndex + pEntry->count; j++)
            {
            // Scale X and Y if the curve is to be in the Z display range
            if ((autoscale_x || autoscale_y) &&
              (autoscale_z || ((CurveNum <= ZMax) && (CurveNum >= ZMin))))
              {
              err = CurveToPlotMaxMin(pPlotBox, i, j);
  
              if(err)
                {
                if(MessageWindow)
                  release_message_window(MessageWindow);
                return err;
                }
              }
  
            CurveNum++;
            if (autoscale_z)
              if(! falseColor)
                if (!ZSwap)
                  pPlotBox->z.original_max_value++;
                else
                  pPlotBox->z.original_min_value++;
              else
                if (!ZSwap)
                  pPlotBox->y.original_max_value++;
                else
                  pPlotBox->y.original_min_value++;
            }
          }
        }
      if (MessageWindow != NULL)
        release_message_window(MessageWindow);
      }
    }
  else   // in live loop
    {
    float LiveCurves;
    LiveScaleParams(&entryIndex, &curveIndex);

    LiveCurves = (float)(MainCurveDir.Entries[entryIndex].count);
  
    CurveToPlotMaxMin(pPlotBox, entryIndex, curveIndex);
  
    if (autoscale_z)
      {
      if(! falseColor)
        {
        if (!ZSwap)
          pPlotBox->z.original_max_value = LiveCurves;
        else
          pPlotBox->z.original_min_value = LiveCurves;
        }
      else
        {
        if (!ZSwap)
          pPlotBox->y.original_max_value = LiveCurves;
        else
          pPlotBox->y.original_min_value = LiveCurves;
        }
      CurveNum = 1;
      }
    }
  
  if(autoscale_x)
    {
    BOOLEAN swap = FALSE;
  
    if((pPlotBox->x.original_min_value == (float) MAXFLOAT) &&
      (pPlotBox->x.original_max_value == (float) MINFLOAT))
      {
      // there is no data being plotted, restore beginning values
      pPlotBox->x.original_min_value = savedXOriginalMin;
      pPlotBox->x.original_max_value = savedXOriginalMax;
      }
    // some data was found, maintain axis orientation
    else if((savedXOriginalMin < savedXOriginalMax) &&
      (pPlotBox->x.original_min_value
      > pPlotBox->x.original_max_value))
      {
      swap = TRUE;
      }
    else if((savedXOriginalMin > savedXOriginalMax) &&
      (pPlotBox->x.original_min_value
      < pPlotBox->x.original_max_value))
      {
      swap = TRUE;
      }
  
    if(swap)
      {
      float temp = pPlotBox->x.original_min_value;
  
      pPlotBox->x.original_min_value = pPlotBox->x.original_max_value;
      pPlotBox->x.original_max_value = temp;
      }
    initAxisToOriginal(& pPlotBox->x);
    }
  
  if(autoscale_y)
    {
    BOOLEAN swap = FALSE;
  
    if(! falseColor)
      {
      if((pPlotBox->y.original_min_value == (float) MAXFLOAT) &&
        (pPlotBox->y.original_min_value == (float) MAXFLOAT))
        {
        // no data, restore beginning values
        pPlotBox->y.original_min_value = savedYOriginalMin;
        pPlotBox->y.original_max_value = savedYOriginalMax;
        }
      // some data was found, maintain axis orientation
      else if((savedYOriginalMin < savedYOriginalMax) &&
        (pPlotBox->y.original_min_value
        > pPlotBox->y.original_max_value))
        {
        swap = TRUE;
        }
      else if((savedYOriginalMin > savedYOriginalMax) &&
        (pPlotBox->y.original_min_value
        < pPlotBox->y.original_max_value))
        {
        swap = TRUE;
        }

      if(pPlotBox->plot_peak_labels.label_peaks)
        {
        int i;
        for(i = 0;i < MAX_LABELLED_CURVES;i++)
          {
          if(pPlotBox->plot_peak_labels.curve_peaks[i].enabled)
            break;
          }
        if (i < MAX_LABELLED_CURVES)
          pPlotBox->y.original_max_value +=
            ((FLOAT)pPlotBox->y.original_max_value * 0.25F);
        }
  
      if(swap)
        {
        float temp = pPlotBox->y.original_min_value;
  
        pPlotBox->y.original_min_value = pPlotBox->y.original_max_value;
        pPlotBox->y.original_max_value = temp;
        }
      initAxisToOriginal(& pPlotBox->y);
      }
    else // falsecolor
      {
      if((pPlotBox->z.original_min_value == (float) MAXFLOAT) &&
        (pPlotBox->z.original_min_value == (float) MAXFLOAT))
        {
        // no data, restore beginning values
        pPlotBox->z.original_min_value = savedYOriginalMin;
        pPlotBox->z.original_max_value = savedYOriginalMax;
        }
      // some data was found, maintain axis orientation
      else if((savedYOriginalMin < savedYOriginalMax) &&
        (pPlotBox->z.original_min_value
        > pPlotBox->z.original_max_value))
        {
        swap = TRUE;
        }
      else if((savedYOriginalMin > savedYOriginalMax) &&
        (pPlotBox->z.original_min_value
        < pPlotBox->z.original_max_value))
        {
        swap = TRUE;
        }
  
      if(swap)
        {
        float temp = pPlotBox->z.original_min_value;
  
        pPlotBox->z.original_min_value = pPlotBox->z.original_max_value;
        pPlotBox->z.original_max_value = temp;
        }
      initAxisToOriginal(& pPlotBox->z);
      }
    }
  
  if(autoscale_z)
    {
    if(! falseColor)
      {
      if (!ZSwap)
        pPlotBox->z.original_max_value--;
      else
        pPlotBox->z.original_min_value--;
      initAxisToOriginal(&pPlotBox->z);
      }
    else
      initAxisToOriginal(&pPlotBox->y);
    }
  
  CursorStatus[WindowPlotAssignment[DisplayWindow]].TotalCurves = CurveNum;
  
  if(CurveNum || InLiveLoop)
    scalePlotbox(pPlotBox);
  
  if(InLiveLoop)
    return LiveLoopAutoScaleAdjust(pPlotBox);
  
  return ERROR_NONE;
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT ScalePlotGraph(USHORT AxisCase)
  {
  if (pKSRecord != NULL)
    {
    if (*pKSRecord)
      switch (AxisCase)
      {
      case ('X'):
        MacRecordString("AUTO_SCALEX();\n");
      break;
      case ('Y'):
        MacRecordString("AUTO_SCALEY();\n");
      break;
      case ('Z'):
        MacRecordString("AUTO_SCALEZ();\n");
      break;
      default:
        MacRecordString("AUTO_SCALE();\n");
      break;
       }
    }
  
  erase_mouse_cursor();
  RemoveGraphCursor();
  
  AutoScalePlotBox(&MainCurveDir, &(Plots[ActiveWindow]), ActiveWindow);
  
  if(baslnsub_active())
    baslnAutoscaleCursorAdjust();
  
  create_plotbox(&(Plots[ActiveWindow]));
  
  if (! InLiveLoop)
    {
    plot_curves(&MainCurveDir, &(Plots[ActiveWindow]), ActiveWindow);
    SetCursorPos(ActiveWindow,
    CursorStatus[ActiveWindow].X,
    CursorStatus[ActiveWindow].Y,
    CursorStatus[ActiveWindow].Z);
    }
  
  replace_mouse_cursor();
  return FALSE;
}

static void SaveRestoreAutoParams(BOOLEAN Save)
{
  static BOOLEAN old_x, old_y, old_z;
  
  if (Save)
    {
    old_x = autoscale_x,
    old_y = autoscale_y,
    old_z = autoscale_z;
    }
  else
    {
    autoscale_x = old_x,
    autoscale_y = old_y,
    autoscale_z = old_z;
    }
}
    
SHORT ScaleXAxis(USHORT Dummy)
{
  SaveRestoreAutoParams(TRUE);
  autoscale_y = autoscale_z = FALSE;
  autoscale_x = TRUE;
  ScalePlotGraph('X');
  SaveRestoreAutoParams(FALSE);
  return FALSE;
}  

SHORT ScaleYAxis(USHORT Dummy)
{
  SaveRestoreAutoParams(TRUE);
  autoscale_x = autoscale_z = FALSE;
  autoscale_y = TRUE;
  ScalePlotGraph('Y');
  SaveRestoreAutoParams(FALSE);
  return FALSE;
}  

SHORT ScaleZAxis(USHORT Dummy)
{
  SaveRestoreAutoParams(TRUE);
  autoscale_x = autoscale_y = FALSE;
  autoscale_z = TRUE;
  ScalePlotGraph('Z');
  SaveRestoreAutoParams(FALSE);
  return FALSE;
}  

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacAutoScale(void)
  {
  
  if(plotAreaShowing())
    {
    BOOLEAN RemGCursor = TempChangeCursorType(NULL);

    ScalePlotGraph(0);
    TempRestoreCursorType(RemGCursor, NULL);
    }
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacScaleX(void)
  {
  
  if(plotAreaShowing())
    {
    BOOLEAN RemGCursor = TempChangeCursorType(NULL);

    ScaleXAxis(0);
    TempRestoreCursorType(RemGCursor, NULL);
    }
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacScaleY(void)
  {
  
  if(plotAreaShowing())
    {
    BOOLEAN RemGCursor = TempChangeCursorType(NULL);

    ScaleYAxis(0);
    TempRestoreCursorType(RemGCursor, NULL);
    }
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MacScaleZ(void)
  {
  
  if(plotAreaShowing())
    {
    BOOLEAN RemGCursor = TempChangeCursorType(NULL);

    ScaleZAxis(0);
    TempRestoreCursorType(RemGCursor, NULL);
    }
  }
