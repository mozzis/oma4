#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>  /* longjump, setjmp */
#include <float.h>   /* _fpreset */
#include <math.h>    /* _fpreset */
#include <signal.h>  /* signal */

#include "eggtype.h"  // basic typedefs
#include "crvheadr.h"  // curve header structure
#include "omameth.h"   // method header structure
#include "detsetup.h"

/* define this constant if you want to compile the main() function */
#define ALONE

/* define this here because it is internal to the detinfo.c module */
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
#endif
static char far sbuf[80];         /* buffer for formatting string output */

static int fp_error_flag = FALSE; /* flag from coprocesor exception */
static jmp_buf reentry_context;   /* means to recover from exception */

/************************************************/
/* Recover from 80x87 error exceptions          */
/************************************************/
#if _MSC_VER < 700
void floating_point_error_trap()
{
  _fpreset();
  fprintf(stderr, "Floating point error!\n");
  fp_error_flag = TRUE;

  longjmp(reentry_context, 0);
}

#else
void floating_point_error_trap(int sig)
{
  if (sig == SIGFPE)
    {
    _fpreset();
    fprintf(stderr, "Floating point error!\n");
    fp_error_flag = TRUE;
    }

  longjmp(reentry_context, 0);
}
#endif


/************************************************/
/* Print a pointer value                        */
/************************************************/
void ploc(long offval)
{
  printf("%-4.4lu  %-4.4lx  ", offval, offval);
}

/************************************************/
/* Print an integer value                       */
/************************************************/
void pfint(char far * str, long offval, int val)
{
  ploc(offval);
  strcpy(sbuf, str);
  strcat(sbuf,"      %4u  %4x\n");
  printf(sbuf, val, val);

}

/************************************************/
/* Print a floating point value                 */
/************************************************/
void pfflt(char far * str, long offval, float val)
{
  ploc(offval);
  strcpy(sbuf, str);
  strcat(sbuf,"%6g  %8lx\n");
  printf(sbuf, val, val);

}

/************************************************/
/* Print a long integer value                   */
/************************************************/
void pflng(char far * str, long offval, long val)
{
  ploc(offval);
  strcpy(sbuf, str);
  strcat(sbuf,"%8lu  %8lx\n");
  printf(sbuf, val, val);

}

/************************************************/
/* Print a string value                         */
/************************************************/
void pfstr(char far * str, long offval, char far * sval)
{
  ploc(offval);
  strcpy(sbuf, str);
  strcat(sbuf," %s\n");
  printf(sbuf, sval);

}


/************************************************/
/* Print out the data values from a float curve */
/************************************************/
void prt_f_curve(FLOAT * Ybuffer, float * Xbuffer, int points)
{
  int i;
  for (i = 0; i < points; i++)
    {
    if (fabs((double)Xbuffer[i]) > 1e-12)
      printf("Point %4d:  %5f  %f \n", i, Xbuffer[i], Ybuffer[i]);
    }
}

/*********************************************************/
/* Print out the data values from a 16-bit integer curve */
/*********************************************************/
void prt_s_curve(SHORT * Ybuffer, float * Xbuffer, int points)
{
  int i;
  for (i = 0; i < points; i++)
    {
    if (fabs((double)Xbuffer[i]) > 1e-12)
      printf("Point %4d:  %5f  %5d \n", i, Xbuffer[i], Ybuffer[i]);
    }
}

/*********************************************************/
/* Print out the data values from a 32-bit integer curve */
/*********************************************************/
void prt_l_curve(LONG * Ybuffer, float * Xbuffer, int points)
{
  int i;
  for (i = 0; i < points; i++)
    {
    if (fabs((double)Xbuffer[i]) > 1e-12)
      printf("Point %4d:  %5f  %8d \n", i, Xbuffer[i], Ybuffer[i]);
    }
}

/*********************************************************/
/* macros to simplify printout of curve header values    */
/*********************************************************/
/* integer */
#define p_crv_int(a, b)\
  pfint(a, offsetof(CURVEHDR, b) + hdrOffset, cvhdr->b)

/* long integer */
#define p_crv_lng(a, b)\
  pflng(a, offsetof(CURVEHDR, b) + hdrOffset, (long)cvhdr->b)

/* float */
#define p_crv_flt(a, b)\
  pfflt(a, offsetof(CURVEHDR, b) + hdrOffset, cvhdr->b)

/* integer in array */
#define p_crv_pio(a, b, c)\
  pfint(a, offsetof(CURVEHDR, b) + c * sizeof(int) + hdrOffset, cvhdr->b[c])

/* hdrOffset allows multiple curve headers from multiple curves to be */
/* printed, although this program only does the first one */
void prtcrvhdr(CURVEHDR far * cvhdr, ULONG hdrOffset)
{
  printf("\n             Curve Header:\n");
  p_crv_int("pointnum............", pointnum);
  p_crv_int("XData.XUnits........", XData.XUnits);
  p_crv_lng("XData.XArray........", XData.XArray);
  p_crv_int("YUnits..............", YUnits);
  p_crv_int("DataType............", DataType);
  p_crv_int("experiment_num......", experiment_num);
  p_crv_flt("time................", time);
  p_crv_lng("scomp...............", scomp);
  p_crv_pio("pia[0]..............", pia, 0);
  p_crv_pio("pia[1]..............", pia, 1);
  p_crv_int("Frame...............", Frame);
  p_crv_int("Track...............", Track);
  p_crv_flt("Ymin................", Ymin);
  p_crv_flt("Ymax................", Ymax);
  p_crv_flt("Xmin................", Xmin);
  p_crv_flt("Xmax................", Xmax);
  p_crv_int("MemData.............", MemData);
  p_crv_int("CurveCount..........", CurveCount);
}

/*********************************************************/
/* macros to simplify printout of method header values   */
/*********************************************************/
/* string */
#define p_met_str(a, b)\
  pfstr(a, offsetof(METHDR, b), methdr -> b)

/* integer */
#define p_met_int(a, b)\
  pfint(a, offsetof(METHDR, b), methdr -> b)

/* float */
#define p_met_flt(a, b)\
  pfflt(a, offsetof(METHDR, b), methdr -> b)

/* float in array */
#define p_met_pft(a, b, c)\
  pfflt(a, offsetof(METHDR, b), methdr -> b[ c ])

/* float in two-dimensional array */
#define p_met_pfo(a, b, c, d)\
  pfflt(a, offsetof(METHDR, b) + sizeof(float) * d + c,\
    methdr -> b[ c ][ d ]);

/* integer in array */
#define p_met_pio(a, b, c)\
  pfint(a, offsetof(METHDR, b) + sizeof(int) * c,\
    methdr -> b[ c ])

void prtmethdr(METHDR __far * methdr)
{
  printf("\n             Method Header:\n");
  printf(" Offset Dec.     Offset Hex\n");
  p_met_str("FileTypeID..........", FileTypeID);
  p_met_int("StructureVersion....", StructureVersion);
  p_met_int("Length..............", Length);
  p_met_int("User................", User);
  p_met_str("Description.........", Description);
  p_met_int("FileCurveNum........", FileCurveNum);
  p_met_int("InterfaceType.......", InterfaceType);
  p_met_int("ActiveDetector......", ActiveDetector);
  p_met_str("DADName.............", DADName);
  p_met_int("Normalize...........", Normalize);

  p_met_int("SpectrographUnits...", SpectrographUnits);
  p_met_flt("Excitation..........", Excitation);
  p_met_int("Spectrograph model..", Spectrograph);
  p_met_int("Grating number......", Grating);
  p_met_pft("GratingCenterChnl 1.", GratingCenterChnl, 0);
  p_met_pft("GratingCenterChnl 2.", GratingCenterChnl, 1);
  p_met_pft("GratingCenterChnl 3.", GratingCenterChnl, 2);
  p_met_pfo("CalibCoeff[0][0]....", CalibCoeff, 0, 0);
  p_met_pfo("CalibCoeff[0][1]....", CalibCoeff, 0, 1);
  p_met_pfo("CalibCoeff[0][2]....", CalibCoeff, 0, 2);
  p_met_pfo("CalibCoeff[0][3]....", CalibCoeff, 0, 3);
  p_met_pfo("CalibCoeff[1][0]....", CalibCoeff, 1, 0);
  p_met_pfo("CalibCoeff[1][1]....", CalibCoeff, 1, 1);
  p_met_pfo("CalibCoeff[1][2]....", CalibCoeff, 1, 2);
  p_met_pfo("CalibCoeff[1][3]....", CalibCoeff, 1, 3);
  p_met_pfo("CalibCoeff[2][0]....", CalibCoeff, 2, 0);
  p_met_pfo("CalibCoeff[2][1]....", CalibCoeff, 2, 1);
  p_met_pfo("CalibCoeff[2][2]....", CalibCoeff, 2, 2);
//p_met_pfo("CalibCoeff[2][3]....", CalibCoeff, 2, 3);
  p_met_pio("CalibUnits[0].......", CalibUnits, 0);
  p_met_pio("CalibUnits[1].......", CalibUnits, 1);
  p_met_pio("CalibUnits[2].......", CalibUnits, 2);
  p_met_str("BackgrndName........", BackgrndName);
  p_met_str("I0Name..............", I0Name);
  p_met_str("InputName...........", InputName);
  p_met_str("OutputName..........", OutputName);
  p_met_flt("YTInterval..........", YTInterval);
  p_met_flt("YTPredelay..........", YTPredelay);
  p_met_pio("Pia[0]..............", Pia, 0);
  p_met_pio("Pia[1]..............", Pia, 1);
  p_met_int("SoftwareVersion.....", SoftwareVersion);
  p_met_int("PlotWindowIndex.....", PlotWindowIndex);
  p_met_int("ActivePlotSetup.....", ActivePlotSetup);
  p_met_int("AutoScaleX..........", AutoScaleX);
  p_met_int("AutoScaleY..........", AutoScaleY);
  p_met_int("AutoScaleZ..........", AutoScaleZ);
  p_met_pio("WindowPlotSetups[0].", WindowPlotSetups, 0,);
  p_met_pio("WindowPlotSetups[1].", WindowPlotSetups, 1,);
  p_met_pio("WindowPlotSetups[2].", WindowPlotSetups, 2,);
  p_met_pio("WindowPlotSetups[3].", WindowPlotSetups, 3,);
  p_met_pio("WindowPlotSetups[4].", WindowPlotSetups, 4,);
  p_met_pio("WindowPlotSetups[5].", WindowPlotSetups, 5,);
  p_met_pio("WindowPlotSetups[6].", WindowPlotSetups, 6,);
  p_met_pio("WindowPlotSetups[7].", WindowPlotSetups, 7,);
  pflng("PlotInfo............", offsetof(METHDR, PlotInfo),
    (long)methdr->PlotInfo);
  pflng("Reserved............", offsetof(METHDR, Reserved),
    (long)methdr->Reserved);
  p_met_int("DetNumber...........", DetNumber);
  pflng("DetInfo.............", offsetof(METHDR, DetInfo),
    (long)methdr->DetInfo);
  pfint("DetInfo Length......", offsetof(METHDR, DetInfo) +
    offsetof(DET_INFO, Length), 0);
}

/*********************************************************/
/* macros to simplify printout of detector info values   */
/*********************************************************/
/* integer from Run Setup */
#define p_int_val(a, b)\
  pfint(a, offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, b),\
    methdr->DetInfo->RunSetup.b)

/* integer from Det Info */
#define p_int_inf(a, b)\
  pfint(a, offsetof(METHDR, DetInfo) + offsetof(DET_INFO, b),\
    methdr->DetInfo->b)

/* float from Run Setup */
#define p_flt_val(a, b)\
  pfflt(a, offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, b),\
    methdr->DetInfo->RunSetup.b)

/* long integer from Run Setup */
#define p_lng_val(a, b)\
  pflng(a, offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, b),\
    methdr->DetInfo->RunSetup.b)

/* bit field */
#define p_bit_val(a, b, c)\
  pfint(a, offsetof(METHDR, DetInfo) + offsetof(DET_SETUP, b)+2,\
    methdr->DetInfo->RunSetup.c)

void prtdetinfo(METHDR * methdr)
{
  printf("\n             Detector Info:\n");
  printf(" Offset Dec.     Offset Hex\n");
  p_flt_val("Version.............", Version);
  p_int_val("DA_mode.............", DA_mode);
  p_int_val("det_addr............", det_addr);
  p_int_val("data_word_size......", data_word_size);
  p_int_val("scans...............", scans);
  p_int_val("memories............", memories);
  p_int_val("ignored_scans.......", ignored_scans);
  p_int_val("prep_frames.........", prep_frames);
  p_int_val("detector_temp.......", detector_temp);
  p_int_val("cooler_locked.......", cooler_locked);
  p_int_val("detector_type_index.", detector_type_index);
  p_int_val("cooler_type_index...", cooler_type_index);
  p_int_val("line_freq_index.....", line_freq_index);
  p_int_val("source_comp_index...", source_comp_index);
  p_int_val("control_index.......", control_index);
  p_int_val("extern_start_index..", external_start_index);

  p_lng_val("memory_base.........", memory_base);
  p_lng_val("memory_size.........", memory_size);

  p_int_val("shutter_open_sync...", shutter_open_sync_index);
  p_int_val("shutter_close_sync..", shutter_close_sync_index);
  p_int_val("shutter_forced_mode.", shutter_forced_mode);
  p_int_val("exposed rows/ET>0...", exposed_rows);
  p_flt_val("exposure_time.......", exposure_time);
  p_int_val("min_exposure_time...", min_exposure_time);
  p_int_val("anti_bloom_percent..", anti_bloom_percent);
  p_bit_val("pix_time_index......", anti_bloom_percent, pix_time_index);
  p_int_val("pulser_audio_index..", pulser_audio_index);
  p_int_val("pulser_index........", pulser_index);
  p_flt_val("pulser_delay........", pulser_delay);
  p_flt_val("pulser_width........", pulser_width);
  p_int_val("pulsr_trigsrc_index.", pulser_trigsrc_index);
  p_int_val("pulsr_trigger_count.", pulser_trigger_count);
  p_int_val("pulsr_intens_mode...", pulser_intensifier_mode);
  p_int_val("pulsr_trig_thrshld..", pulser_trig_threshold);
  p_flt_val("pulser_delay_inc....", pulser_delay_inc);
  p_flt_val("pulser_scan_timeout.", pulser_scan_timeout);
  p_int_val("tracks..............", tracks);
  p_int_val("points..............", points);
  p_int_val("DumXLead............", DumXLead);
  p_int_val("DumXTrail...........", DumXTrail);
  p_int_val("ActiveX.............", ActiveX);
  p_int_val("ActiveY.............", ActiveY);
  p_int_val("DumYLead............", DumYLead);
  p_int_val("DumYTrail...........", DumYTrail);
  p_int_val("Regions.............", regions);
  p_int_val("StartMemory.........", StartMemory);
  p_int_val("Current_Point.......", Current_Point);
  p_int_val("Current_Track.......", Current_Track);
  p_int_val("max_memory..........", max_memory);
  p_int_val("pointmode...........", pointmode);
  p_int_val("trackmode...........", trackmode);
  p_bit_val("shiftmode : 2.......", trackmode, shiftmode);

  p_bit_val("outputReg : 2.......", trackmode, outputReg);
  p_bit_val("same_et   : 1.......", trackmode, same_et);
  p_int_val("da_active...........", da_active);
  p_int_val("background_active...", background_active);

  p_int_inf("X Pixel Groups......", XPixelGroups);
  p_int_inf("Y Pixel Groups......", YPixelGroups);
  p_int_inf("Trigger Groups......", TriggerGroups);
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
