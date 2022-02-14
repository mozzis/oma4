#include <stdio.h>
#include <string.h>
#include "omatyp.h"

  CURVEHDR cvhdr;
  METHDR methdr;
  CURVE_ENTRY cventry;
  CURVEBUFFER cvbuf;
  char sbuf[80];


void ploc(char * str, int offval, int szval)
{
  strcpy(sbuf, str);
  strcat(sbuf,"%4u  %4u  %4x\n");
  printf(sbuf, szval, offval, offval);

}
  
int main(void)
{
  printf("  field name       size  dec   hex\n\n");
  printf("Curve Header:\n");
  ploc("pointnum.........", offsetof(CURVEHDR, pointnum), sizeof(cvhdr.pointnum));
  ploc("XData............", offsetof(CURVEHDR, XData), sizeof(cvhdr.XData));
  ploc("YUnits...........", offsetof(CURVEHDR, YUnits), sizeof(cvhdr.YUnits));
  ploc("DataType.........", offsetof(CURVEHDR, DataType), sizeof(cvhdr.DataType));
  ploc("experiment_num...", offsetof(CURVEHDR, experiment_num), sizeof(cvhdr.experiment_num));
  ploc("time.............", offsetof(CURVEHDR, time), sizeof(cvhdr.time));
  ploc("scomp............", offsetof(CURVEHDR, scomp), sizeof(cvhdr.scomp));
  ploc("pia[2]...........", offsetof(CURVEHDR, pia), sizeof(cvhdr.pia));
  ploc("Frame............", offsetof(CURVEHDR, Frame), sizeof(cvhdr.Frame));
  ploc("Track............", offsetof(CURVEHDR, Track), sizeof(cvhdr.Track));
  ploc("Ymin.............", offsetof(CURVEHDR, Ymin), sizeof(cvhdr.Ymin));
  ploc("Ymax.............", offsetof(CURVEHDR, Ymax), sizeof(cvhdr.Ymax));
  ploc("Xmin.............", offsetof(CURVEHDR, Xmin), sizeof(cvhdr.Xmin));
  ploc("Xmax.............", offsetof(CURVEHDR, Xmax), sizeof(cvhdr.Xmax));
  ploc("MemData..........", offsetof(CURVEHDR, MemData), sizeof(cvhdr.MemData));
  ploc("CurveCount.......", offsetof(CURVEHDR, CurveCount), sizeof(cvhdr.CurveCount));

  printf("\nMethod Header:\n");
  ploc("FileTypeID.......", offsetof(METHDR, FileTypeID), sizeof(methdr.FileTypeID));
  ploc("StructureVersion.", offsetof(METHDR, StructureVersion), sizeof(methdr.StructureVersion));
  ploc("Length...........", offsetof(METHDR, Length), sizeof(methdr.Length));
  ploc("User.............", offsetof(METHDR, User), sizeof(methdr.User));
  ploc("Description......", offsetof(METHDR, Description), sizeof(methdr.Description));
  ploc("FileCurveNum.....", offsetof(METHDR, FileCurveNum), sizeof(methdr.FileCurveNum));
  ploc("InterfaceType....", offsetof(METHDR, InterfaceType), sizeof(methdr.InterfaceType));
  ploc("ActiveDetector...", offsetof(METHDR, ActiveDetector), sizeof(methdr.ActiveDetector));
  ploc("DADName..........", offsetof(METHDR, DADName), sizeof(methdr.DADName));
  ploc("Normalize........", offsetof(METHDR, Normalize), sizeof(methdr.Normalize));
  ploc("SpectrographUnits", offsetof(METHDR, SpectrographUnits), sizeof(methdr.SpectrographUnits));
  ploc("Excitation.......", offsetof(METHDR, Excitation), sizeof(methdr.Excitation));
  ploc("Spectrograph.....", offsetof(METHDR, Spectrograph),    sizeof(methdr.Spectrograph));
  ploc("Grating..........", offsetof(METHDR, Grating),         sizeof(methdr.Grating));
  ploc("CenterChannel....", offsetof(METHDR, GratingCenterChnl), sizeof(methdr.GratingCenterChnl));
  ploc("CalibCoeff.......", offsetof(METHDR, CalibCoeff), sizeof(methdr.CalibCoeff));
  ploc("CalibUnits.......", offsetof(METHDR, CalibUnits), sizeof(methdr.CalibUnits));
  ploc("BackgrndName.....", offsetof(METHDR, BackgrndName), sizeof(methdr.BackgrndName));
  ploc("I0Name...........", offsetof(METHDR, I0Name), sizeof(methdr.I0Name));
  ploc("InputName........", offsetof(METHDR, InputName), sizeof(methdr.InputName));
  ploc("OutputName.......", offsetof(METHDR, OutputName), sizeof(methdr.OutputName));
  ploc("YTInterval.......", offsetof(METHDR, YTInterval), sizeof(methdr.YTInterval));
  ploc("YTPredelay.......", offsetof(METHDR, YTPredelay), sizeof(methdr.YTPredelay));
  ploc("Pia..............", offsetof(METHDR, Pia), sizeof(methdr.Pia));
  ploc("SoftwareVersion..", offsetof(METHDR, SoftwareVersion), sizeof(methdr.SoftwareVersion));
  ploc("PlotWindowIndex..", offsetof(METHDR, PlotWindowIndex), sizeof(methdr.PlotWindowIndex));
  ploc("ActivePlotSetup..", offsetof(METHDR, ActivePlotSetup), sizeof(methdr.ActivePlotSetup));
  ploc("AutoScaleX.......", offsetof(METHDR, AutoScaleX), sizeof(methdr.AutoScaleX));
  ploc("AutoScaleY.......", offsetof(METHDR, AutoScaleY), sizeof(methdr.AutoScaleY));
  ploc("AutoScaleZ.......", offsetof(METHDR, AutoScaleZ), sizeof(methdr.AutoScaleZ));
  ploc("WindowPlotSetups.", offsetof(METHDR, WindowPlotSetups), sizeof(methdr.WindowPlotSetups));
  ploc("PlotInfo.........", offsetof(METHDR, PlotInfo), sizeof(methdr.PlotInfo));
  ploc("Reserved.........", offsetof(METHDR, Reserved), sizeof(methdr.Reserved));
  ploc("DetNumber........", offsetof(METHDR, DetNumber), sizeof(methdr.DetNumber));
  ploc("DetInfo..........", offsetof(METHDR, DetInfo), sizeof(methdr.DetInfo));


  printf("\nCurve Entry:\n");
  ploc("name.............", offsetof(CURVE_ENTRY, name), sizeof(cventry.name));
  ploc("path.............", offsetof(CURVE_ENTRY, path), sizeof(cventry.path));
  ploc("descrip..........", offsetof(CURVE_ENTRY, descrip), sizeof(cventry.descrip));
  ploc("StartIndex.......", offsetof(CURVE_ENTRY, StartIndex), sizeof(cventry.StartIndex));
  printf("union StartOffset:\n");
  ploc("  Offset.........", offsetof(CURVE_ENTRY, StartOffset.Offset), sizeof(cventry.StartOffset.Offset));
  ploc("  CurveArray.....", offsetof(CURVE_ENTRY, StartOffset.CurveArray), sizeof(cventry.StartOffset.CurveArray));
  ploc("TmpOffset........", offsetof(CURVE_ENTRY, TmpOffset), sizeof(cventry.TmpOffset));
  ploc("count............", offsetof(CURVE_ENTRY, count), sizeof(cventry.count));
  ploc("time.............", offsetof(CURVE_ENTRY, time), sizeof(cventry.time));
  ploc("EntryType........", offsetof(CURVE_ENTRY, EntryType), sizeof(cventry.EntryType));
  ploc("Dirty............", offsetof(CURVE_ENTRY, Dirty), sizeof(cventry.Dirty));
  ploc("DisplayWindow....", offsetof(CURVE_ENTRY, DisplayWindow), sizeof(cventry.DisplayWindow));

  printf("\nCurveBuffer:\n");

  ploc("BufPtr ..........", offsetof(CURVEBUFFER, BufPtr),      sizeof(cvbuf.BufPtr));
  ploc("ActiveDir........", offsetof(CURVEBUFFER, ActiveDir),   sizeof(cvbuf.ActiveDir));
  ploc("Entry............", offsetof(CURVEBUFFER, Entry),       sizeof(cvbuf.Entry));
  ploc("CurveIndex,......", offsetof(CURVEBUFFER, CurveIndex),  sizeof(cvbuf.CurveIndex));
  ploc("BufferOffset.....", offsetof(CURVEBUFFER, BufferOffset),sizeof(cvbuf.BufferOffset));
  ploc("CurveHdr.........", offsetof(CURVEBUFFER, Curvehdr),sizeof(cvbuf.Curvehdr));
  ploc("status...........", offsetof(CURVEBUFFER, status),  sizeof(cvbuf.status));
  return(0);

}
