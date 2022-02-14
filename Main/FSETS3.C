#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>  /* longjump, setjmp */
#include <float.h>   /* _fpreset */
#include <math.h>    /* _fpreset */
#include <signal.h>  /* signal */

#include "primtype.h"
#include "crvheadr.h"
#include "crventry.h"
#include "curvbufr.h"
#include "omameth.h"
#include "detinfo.h"
#include "oma4driv.h"

#define ALONE

struct detector_info
{
   USHORT         Length;
   DET_SETUP      RunSetup;
   USHORT         XPixelGroups;
   USHORT         YPixelGroups;
   USHORT         TriggerGroups;
   USHORT         ETGroups;
   PVOID          GroupTables;
} ;

#define DET_INFO_BASE_LENGTH offsetof(DET_INFO, GroupTables )

#ifdef ALONE
CURVEHDR     near cvhdr;
METHDR       near methdr;
CURVEBUFFER  near cvbuf;
CURVE_ENTRY  near cventry;
#endif
static char far sbuf[80];
static int fp_error_flag = FALSE;
static jmp_buf  reentry_context;

void floating_point_error_trap()
{
  _fpreset();
  fprintf(stderr, "Floating point error!\n");
  fp_error_flag = TRUE;

  longjmp(reentry_context, 0);
}


void ploc(long offval)
{
  printf("%-4.4lu  %-4.4lx  ", offval, offval);
}
  
void pfint(char far * str, long offval, int val)
{
  ploc(offval);
  strcpy(sbuf, str);
  strcat(sbuf,"      %4u  %4x\n");
  printf(sbuf, val, val);

}
  
void pfflt(char far * str, long offval, float val)
{
  ploc(offval);
  strcpy(sbuf, str);
  strcat(sbuf,"%6g  %8lx\n");
  printf(sbuf, val, val);

}

void pflng(char far * str, long offval, long val)
{
  ploc(offval);
  strcpy(sbuf, str);
  strcat(sbuf,"%8lu  %8lx\n");
  printf(sbuf, val, val);

}

void pfstr(char far * str, long offval, char far * sval)
{
  ploc(offval);
  strcpy(sbuf, str);
  strcat(sbuf," %s\n");
  printf(sbuf, sval);

}


void prt_f_curve(FLOAT * Ybuffer, float * Xbuffer, int points)
{
  int i;
  for (i = 0; i < points; i++)
    {
    if (fabs((double)Xbuffer[i]) > 1e-12)
      printf("Point %4d:  %5f  %f \n", i, Xbuffer[i], Ybuffer[i]);
    }
}

void prt_s_curve(SHORT * Ybuffer, float * Xbuffer, int points)
{
  int i;
  for (i = 0; i < points; i++)
    {
    if (fabs((double)Xbuffer[i]) > 1e-12)
      printf("Point %4d:  %5f  %5d \n", i, Xbuffer[i], Ybuffer[i]);
    }
}

void prt_l_curve(LONG * Ybuffer, float * Xbuffer, int points)
{
  int i;
  for (i = 0; i < points; i++)
    {
    if (fabs((double)Xbuffer[i]) > 1e-12)
      printf("Point %4d:  %5f  %8d \n", i, Xbuffer[i], Ybuffer[i]);
    }
}

void prtcrvhdr(CURVEHDR far * cvhdr, ULONG hdrOffset)
{
  printf("\n             Curve Header:\n");
  pfint("pointnum............", offsetof(CURVEHDR, pointnum) + hdrOffset, cvhdr->pointnum);
  pfint("XData.XUnits........", offsetof(CURVEHDR, XData.XUnits) + hdrOffset, cvhdr->XData.XUnits);
  pflng("XData.XArray........", offsetof(CURVEHDR, XData.XUnits) + hdrOffset, (unsigned long)cvhdr->XData.XArray);
  pfint("YUnits..............", offsetof(CURVEHDR, YUnits) + hdrOffset, cvhdr->YUnits);
  pfint("DataType............", offsetof(CURVEHDR, DataType) + hdrOffset, cvhdr->DataType);
  pfint("experiment_num......", offsetof(CURVEHDR, experiment_num) + hdrOffset, cvhdr->experiment_num);
  pfflt("time................", offsetof(CURVEHDR, time) + hdrOffset, cvhdr->time);
  pflng("scomp...............", offsetof(CURVEHDR, scomp) + hdrOffset, cvhdr->scomp);
  pfint("pia[0]..............", offsetof(CURVEHDR, pia) + hdrOffset, cvhdr->pia[0]);
  pfint("pia[1]..............", offsetof(CURVEHDR, pia) +  sizeof(cvhdr->pia[0]) + hdrOffset, cvhdr->pia[1]);
  pfint("Frame...............", offsetof(CURVEHDR, Frame) + hdrOffset, cvhdr->Frame);
  pfint("Track...............", offsetof(CURVEHDR, Track) + hdrOffset, cvhdr->Track);
  pfflt("Ymin................", offsetof(CURVEHDR, Ymin) + hdrOffset, cvhdr->Ymin);
  pfflt("Ymax................", offsetof(CURVEHDR, Ymax) + hdrOffset, cvhdr->Ymax);
  pfflt("Xmin................", offsetof(CURVEHDR, Xmin) + hdrOffset, cvhdr->Xmin);
  pfflt("Xmax................", offsetof(CURVEHDR, Xmax) + hdrOffset, cvhdr->Xmax);
  pfint("MemData.............", offsetof(CURVEHDR, MemData) + hdrOffset, cvhdr->MemData);
  pfint("CurveCount..........", offsetof(CURVEHDR, CurveCount) + hdrOffset, cvhdr->CurveCount);
}

void prtmethdr(METHDR * methdr)
{
  printf("\n             Method Header:\n");
  pfstr("FileTypeID..........", offsetof(METHDR, FileTypeID), methdr->FileTypeID);
  pfint("StructureVersion....", offsetof(METHDR, StructureVersion), methdr->StructureVersion);
  pfint("Length..............", offsetof(METHDR, Length), methdr->Length);
  pfint("User................", offsetof(METHDR, User), methdr->User);
  pfstr("Description.........", offsetof(METHDR, Description), methdr->Description);
  pfint("FileCurveNum........", offsetof(METHDR, FileCurveNum), methdr->FileCurveNum);
  pfint("InterfaceType.......", offsetof(METHDR, InterfaceType), methdr->InterfaceType);
  pfint("ActiveDetector......", offsetof(METHDR, ActiveDetector), methdr->ActiveDetector);
  pfstr("DADName.............", offsetof(METHDR, DADName), methdr->DADName);
  pfint("Normalize...........", offsetof(METHDR, Normalize), methdr->Normalize);

  pfint("SpectrographUnits...", offsetof(METHDR, SpectrographUnits), methdr->SpectrographUnits);
  pfflt("Excitation..........", offsetof(METHDR, Excitation), methdr->Excitation);
  pfint("Spectrograph model..", offsetof(METHDR, Spectrograph), methdr->Spectrograph);
  pfint("Grating number......", offsetof(METHDR, Grating), methdr->Grating);
  pfflt("GratingCenterChnl 1.", offsetof(METHDR, GratingCenterChnl),methdr->GratingCenterChnl[0]);
  pfflt("GratingCenterChnl 2.", offsetof(METHDR, GratingCenterChnl),methdr->GratingCenterChnl[1]);
  pfflt("GratingCenterChnl 3.", offsetof(METHDR, GratingCenterChnl),methdr->GratingCenterChnl[2]);
  pfflt("CalibCoeff[0][0]....", offsetof(METHDR, CalibCoeff) + sizeof(float) * 0, methdr->CalibCoeff[0][0]);
  pfflt("CalibCoeff[0][1]....", offsetof(METHDR, CalibCoeff) + sizeof(float) * 1, methdr->CalibCoeff[0][1]);
  pfflt("CalibCoeff[0][2]....", offsetof(METHDR, CalibCoeff) + sizeof(float) * 2, methdr->CalibCoeff[0][2]);
  pfflt("CalibCoeff[0][3]....", offsetof(METHDR, CalibCoeff) + sizeof(float) * 3, methdr->CalibCoeff[0][3]);
  pfflt("CalibCoeff[1][0]....", offsetof(METHDR, CalibCoeff) + sizeof(float) * 4, methdr->CalibCoeff[1][0]);
  pfflt("CalibCoeff[1][1]....", offsetof(METHDR, CalibCoeff) + sizeof(float) * 5, methdr->CalibCoeff[1][1]);
  pfflt("CalibCoeff[1][2]....", offsetof(METHDR, CalibCoeff) + sizeof(float) * 6, methdr->CalibCoeff[1][2]);
  pfflt("CalibCoeff[1][3]....", offsetof(METHDR, CalibCoeff) + sizeof(float) * 7, methdr->CalibCoeff[1][3]);
  pfflt("CalibCoeff[2][0]....", offsetof(METHDR, CalibCoeff) + sizeof(float) * 8, methdr->CalibCoeff[2][0]);
  pfflt("CalibCoeff[2][1]....", offsetof(METHDR, CalibCoeff) + sizeof(float) * 9, methdr->CalibCoeff[2][1]);
  pfflt("CalibCoeff[2][2]....", offsetof(METHDR, CalibCoeff) + sizeof(float) * 10, methdr->CalibCoeff[2][2]);
//pfflt("CalibCoeff[2][3]....", offsetof(METHDR, CalibCoeff) + sizeof(float) * 11, methdr->CalibCoeff[2][3]);
  pfint("CalibUnits[0].......", offsetof(METHDR, CalibUnits) + sizeof(int) * 0, methdr->CalibUnits[0]);
  pfint("CalibUnits[1].......", offsetof(METHDR, CalibUnits) + sizeof(int) * 1, methdr->CalibUnits[1]);
  pfint("CalibUnits[2].......", offsetof(METHDR, CalibUnits) + sizeof(int) * 2, methdr->CalibUnits[2]);
  pfstr("BackgrndName........", offsetof(METHDR, BackgrndName), methdr->BackgrndName);
  pfstr("I0Name..............", offsetof(METHDR, I0Name), methdr->I0Name);
  pfstr("InputName...........", offsetof(METHDR, InputName), methdr->InputName);
  pfstr("OutputName..........", offsetof(METHDR, OutputName), methdr->OutputName);
  pfflt("YTInterval..........", offsetof(METHDR, YTInterval), methdr->YTInterval);
  pfflt("YTPredelay..........", offsetof(METHDR, YTPredelay), methdr->YTPredelay);
  pfint("Pia[0]..............", offsetof(METHDR, Pia) + sizeof(int) * 0, methdr->Pia[0]);
  pfint("Pia[1]..............", offsetof(METHDR, Pia) + sizeof(int) * 0, methdr->Pia[1]);
  pfint("SoftwareVersion.....", offsetof(METHDR, SoftwareVersion), methdr->SoftwareVersion);
  pfint("PlotWindowIndex.....", offsetof(METHDR, PlotWindowIndex), methdr->PlotWindowIndex);
  pfint("ActivePlotSetup.....", offsetof(METHDR, ActivePlotSetup), methdr->ActivePlotSetup);
  pfint("AutoScaleX..........", offsetof(METHDR, AutoScaleX), methdr->AutoScaleX);
  pfint("AutoScaleY..........", offsetof(METHDR, AutoScaleY), methdr->AutoScaleY);
  pfint("AutoScaleZ..........", offsetof(METHDR, AutoScaleZ), methdr->AutoScaleZ);
  pfint("WindowPlotSetups[0].", offsetof(METHDR, WindowPlotSetups) + sizeof(int) * 0, methdr->WindowPlotSetups[0]);
  pfint("WindowPlotSetups[1].", offsetof(METHDR, WindowPlotSetups) + sizeof(int) * 1, methdr->WindowPlotSetups[1]);
  pfint("WindowPlotSetups[2].", offsetof(METHDR, WindowPlotSetups) + sizeof(int) * 2, methdr->WindowPlotSetups[2]);
  pfint("WindowPlotSetups[3].", offsetof(METHDR, WindowPlotSetups) + sizeof(int) * 3, methdr->WindowPlotSetups[3]);
  pfint("WindowPlotSetups[4].", offsetof(METHDR, WindowPlotSetups) + sizeof(int) * 4, methdr->WindowPlotSetups[4]);
  pfint("WindowPlotSetups[5].", offsetof(METHDR, WindowPlotSetups) + sizeof(int) * 5, methdr->WindowPlotSetups[5]);
  pfint("WindowPlotSetups[6].", offsetof(METHDR, WindowPlotSetups) + sizeof(int) * 6, methdr->WindowPlotSetups[6]);
  pfint("WindowPlotSetups[7].", offsetof(METHDR, WindowPlotSetups) + sizeof(int) * 7, methdr->WindowPlotSetups[7]);
  pfint("PlotInfo............", offsetof(METHDR, PlotInfo), (int)methdr->PlotInfo);
  pfint("Reserved............", offsetof(METHDR, Reserved), (int)methdr->Reserved);
  pfint("DetNumber...........", offsetof(METHDR, DetNumber), methdr->DetNumber);
  pfint("DetInfo.............", offsetof(METHDR, DetInfo), (int)methdr->DetInfo);
  pfint("DetInfo Length......", offsetof(METHDR, DetInfo) + offsetof(DET_INFO, Length), 0);
}


void prtdetinfo(METHDR * methdr)
{
  printf("\n             Detector Info:\n");

  pfflt("Version.............", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, Version),             methdr->DetInfo->RunSetup.Version);  
  pfint("DA_mode.............", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, DA_mode),             methdr->DetInfo->RunSetup.DA_mode);                  
  pfint("det_addr............", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, det_addr),            methdr->DetInfo->RunSetup.det_addr);                 
  pfint("data_word_size......", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, data_word_size),      methdr->DetInfo->RunSetup.data_word_size);           
  pfint("scans...............", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, scans),               methdr->DetInfo->RunSetup.scans);                    
  pfint("memories............", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, memories),            methdr->DetInfo->RunSetup.memories);                 
  pfint("ignored_scans.......", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, ignored_scans),       methdr->DetInfo->RunSetup.ignored_scans);            
  pfint("prep_frames.........", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, prep_frames),         methdr->DetInfo->RunSetup.prep_frames);              
  pfint("detector_temp.......", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, detector_temp),       methdr->DetInfo->RunSetup.detector_temp);            
  pfint("cooler_locked.......", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, cooler_locked),       methdr->DetInfo->RunSetup.cooler_locked);            
  pfint("detector_type_index.", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, detector_type_index), methdr->DetInfo->RunSetup.detector_type_index);      
  pfint("cooler_type_index...", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, cooler_type_index),   methdr->DetInfo->RunSetup.cooler_type_index);        
  pfint("line_freq_index.....", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, line_freq_index),     methdr->DetInfo->RunSetup.line_freq_index);          
  pfint("source_comp_index...", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, source_comp_index),   methdr->DetInfo->RunSetup.source_comp_index);        
  pfint("control_index.......", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, control_index),       methdr->DetInfo->RunSetup.control_index);            
  pfint("extern_start_index..", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, external_start_index), methdr->DetInfo->RunSetup.external_start_index);     

  pflng("memory_base.........", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, memory_base), methdr->DetInfo->RunSetup.memory_base);
  pflng("memory_size.........", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, memory_size), methdr->DetInfo->RunSetup.memory_size);

  pfint("shutter_open_sync...", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, shutter_open_sync_index),   methdr->DetInfo->RunSetup.shutter_open_sync_index);  
  pfint("shutter_close_sync..", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, shutter_close_sync_index),  methdr->DetInfo->RunSetup.shutter_close_sync_index); 
  pfint("shutter_forced_mode.", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, shutter_forced_mode),       methdr->DetInfo->RunSetup.shutter_forced_mode);      
  pfint("need_expose_index...", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, need_expose_index),   methdr->DetInfo->RunSetup.need_expose_index);        
  pfflt("exposure_time.......", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, exposure_time),       methdr->DetInfo->RunSetup.exposure_time);            
  pfint("min_exposure_time...", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, min_exposure_time),   methdr->DetInfo->RunSetup.min_exposure_time);        
  pfint("anti_bloom_percent..", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, anti_bloom_percent),  methdr->DetInfo->RunSetup.anti_bloom_percent);      
  pfint("pix_time_index......", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, pix_time_index),      methdr->DetInfo->RunSetup.pix_time_index);           
  pfint("pulser_audio_index..", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, pulser_audio_index), methdr->DetInfo->RunSetup.pulser_audio_index);       
  pfint("pulser_index........", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, pulser_index),        methdr->DetInfo->RunSetup.pulser_index);             
  pfflt("pulser_delay........", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, pulser_delay),        methdr->DetInfo->RunSetup.pulser_delay);             
  pfflt("pulser_width........", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, pulser_width),        methdr->DetInfo->RunSetup.pulser_width);             
  pfint("pulsr_trigsrc_index.", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, pulser_trigsrc_index), methdr->DetInfo->RunSetup.pulser_trigsrc_index);     
  pfint("pulsr_trigger_count.", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, pulser_trigger_count), methdr->DetInfo->RunSetup.pulser_trigger_count);     
  pfint("pulsr_intens_mode...", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, pulser_intensifier_mode),   methdr->DetInfo->RunSetup.pulser_intensifier_mode);  
  pfint("pulsr_trig_thrshld..", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, pulser_trig_threshold), methdr->DetInfo->RunSetup.pulser_trig_threshold);
  pfflt("pulser_delay_inc....", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, pulser_delay_inc),    methdr->DetInfo->RunSetup.pulser_delay_inc);         
  pfflt("pulser_scan_timeout.", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, pulser_scan_timeout), methdr->DetInfo->RunSetup.pulser_scan_timeout);      
  pfint("tracks..............", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, tracks),              methdr->DetInfo->RunSetup.tracks);                   
  pfint("points..............", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, points),              methdr->DetInfo->RunSetup.points);                   
  pfint("DumXLead............", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, DumXLead),            methdr->DetInfo->RunSetup.DumXLead);                 
  pfint("DumXTrail...........", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, DumXTrail),           methdr->DetInfo->RunSetup.DumXTrail);                
  pfint("ActiveX.............", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, ActiveX),             methdr->DetInfo->RunSetup.ActiveX);                  
  pfint("ActiveY.............", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, ActiveY),             methdr->DetInfo->RunSetup.ActiveY);                  
  pfint("DumYLead............", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, DumYLead),            methdr->DetInfo->RunSetup.DumYLead);                 
  pfint("DumYTrail...........", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, DumYTrail),           methdr->DetInfo->RunSetup.DumYTrail);                
  pfint("scan_timeout_index..", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, scan_timeout_index), methdr->DetInfo->RunSetup.scan_timeout_index);
  pfint("StartMemory.........", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, StartMemory),         methdr->DetInfo->RunSetup.StartMemory);              
  pfint("Current_Point.......", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, Current_Point),       methdr->DetInfo->RunSetup.Current_Point);            
  pfint("Current_Track.......", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, Current_Track),       methdr->DetInfo->RunSetup.Current_Track);            
  pfint("max_memory..........", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, max_memory),          methdr->DetInfo->RunSetup.max_memory);               
  pfint("pointmode...........", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, pointmode),           methdr->DetInfo->RunSetup.pointmode);                
  pfint("trackmode...........", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, trackmode),           methdr->DetInfo->RunSetup.trackmode);                
  pfint("shiftmode : 2.......", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, trackmode)+2,         methdr->DetInfo->RunSetup.shiftmode);          
                                        
  pfint("outputReg : 2.......", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, trackmode)+2,           methdr->DetInfo->RunSetup.outputReg);          
  pfint("same_et   : 1.......", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, trackmode)+2,             methdr->DetInfo->RunSetup.same_et);          
  pfint("da_active...........", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, da_active),           methdr->DetInfo->RunSetup.da_active);                
  pfint("background_active...", offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, background_active),   methdr->DetInfo->RunSetup.background_active);        
  
  pfint("X Pixel Groups......", offsetof(METHDR, DetInfo) + offsetof(DET_INFO, XPixelGroups), methdr->DetInfo->XPixelGroups);
  pfint("Y Pixel Groups......", offsetof(METHDR, DetInfo) + offsetof(DET_INFO, YPixelGroups), methdr->DetInfo->YPixelGroups);
  pfint("Trigger Groups......", offsetof(METHDR, DetInfo) + offsetof(DET_INFO, TriggerGroups), methdr->DetInfo->TriggerGroups);
}

#ifdef ALONE
int main(int argc, char * * argv)
{
  FILE * dfile;
  ULONG TestLen;
  USHORT TableSz;
  SHORT i, flag = TRUE;
  void * Ybuffer;
  float *Xbuffer;

  if (argc < 2)
    {
    fprintf(stderr, "Usage: fsets <filename>\n");
    exit(1);
    }

  if (argc > 2)
    {
    if (!stricmp(argv[2], "NP"))
      flag = FALSE;
    }

  if ((dfile = fopen(argv[1], "rb"))==NULL)
    {
    fprintf(stderr, "Can't open %s for input\n",argv[1]);
    exit(2);
    }

  if (fseek(dfile, 0L, SEEK_SET))
    {
    fprintf(stderr, "Seek error in file %s\n",argv[0]);
    exit(3);
    }

  if (fread(&(methdr.FileTypeID), HDR_BASE_LENGTH, 1, dfile) != 1)
    {
    fprintf(stderr, "Error reading methdr in file %s \n",argv[1]);
    exit(4);
    }

  if (methdr.StructureVersion < 12)
    {
    fprintf(stderr, "%s is older version\n",argv[1]);
    exit(5);
    }

  setjmp(reentry_context);
  signal(SIGFPE, floating_point_error_trap );

  TestLen = HDR_BASE_LENGTH;

  if (methdr.DetNumber)
    {
    if ((methdr.DetInfo = calloc(1, sizeof(DET_INFO) * methdr.DetNumber)) == NULL)
      {
      fprintf(stderr, "No room for detector info\n");
      exit(6);
      }

    for (i=0; i < methdr.DetNumber; i++)
      {
      methdr.DetInfo[i].GroupTables = NULL;

      if (fread(&(methdr.DetInfo[i]), DET_INFO_BASE_LENGTH, 1, dfile) != 1)
        {
        fprintf(stderr, "Can't read detector info\n");
        exit(7);
        }

      TestLen += DET_INFO_BASE_LENGTH;

      TableSz = (sizeof(SHORT) * 2 * (methdr.DetInfo[i].XPixelGroups)) +
                (sizeof(SHORT) * 2 * (methdr.DetInfo[i].YPixelGroups)) +
                (sizeof(SHORT) * 2 * (methdr.DetInfo[i].TriggerGroups)) ;

      if (TableSz) /* MLM 2-20-91 If no tables don't be disappointed */
        {
        if(! (methdr.DetInfo[i].GroupTables = calloc(1, TableSz)))
          {
          fprintf(stderr, "No room for group tables\n");
          exit(8);
          }
        if (fread(methdr.DetInfo[i].GroupTables, TableSz, 1, dfile) != 1)
          {
          fprintf(stderr, "Can't read group tables\n");
          exit(9);
          }
        TestLen += TableSz;
        }
      }
    }
  else
    methdr.DetInfo = NULL;

  if (!((Ybuffer = malloc(4100)) && (Xbuffer = malloc(4100))))
    flag = FALSE;

  if (TestLen != methdr.Length )
    {
    int contch;

    fprintf(stderr, "%s is corrupt\n", argv[1]);
    fprintf(stderr, "Press Y to continue or N to abort");
    contch = toupper((int)(getch()));
    if (contch != 'Y')
      exit(10);
    }

  if (fread(&cvhdr, sizeof(CURVEHDR), 1, dfile) != 1)
    {
    fprintf(stderr, "Error reading curvehdr in file %s \n",argv[1]);
    exit(5);
    }
  prtmethdr(&methdr);
  fprintf(stderr,"Press any key to continue");
  getch();
  fprintf(stderr,"\n");
  prtdetinfo(&methdr);
  fprintf(stderr,"Press any key to continue");
  getch();
  fprintf(stderr,"\n");
  prtcrvhdr(&cvhdr, TestLen);
  fprintf(stderr,"Press any key to continue");
  getch();
  fprintf(stderr,"\n");

  if (flag)
    {
    int dtype = cvhdr.DataType & 0x0F;
    
    if (fread(Ybuffer, dtype, cvhdr.pointnum, dfile) != cvhdr.pointnum)
      {
      fprintf(stderr, "Error reading curve Y data\n");
      exit(11);
      }
    if (fread(Xbuffer, sizeof(float), cvhdr.pointnum, dfile) != cvhdr.pointnum)
      {
      fprintf(stderr, "Error reading curve X data\n");
      exit(11);
      }

    if (cvhdr.DataType == LONGTYPE)
      prt_l_curve((LONG *)Ybuffer, Xbuffer, cvhdr.pointnum);
    else if (cvhdr.DataType == SHORTTYPE)
      prt_s_curve((SHORT *)Ybuffer, Xbuffer, cvhdr.pointnum);
    else if (cvhdr.DataType == FLOATTYPE)
      prt_f_curve((FLOAT *)Ybuffer, Xbuffer, cvhdr.pointnum);
    }

  return(0);
}
#endif

