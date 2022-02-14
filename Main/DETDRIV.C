/* -----------------------------------------------------------------------
/
/  detdriv.c 
/  Functions used by application to set detector parameters.
/  Converts units, deals with dependent parameters, calls driver interface
/
/  Copyright (c) 1991,  EG&G Instruments Inc.
/  Author: Morris Maynard
/
/  $Header:
/  $Log:
/
/  Version 2.8  7 June  1993 - integrated loader!
/  Version 3.07              - streaker stuff
/  Version 3.08 3 March 1994 - fix bug, scan_setup_changed never set FALSE
/  Version 4.0  20 Aug 1994  - changes to start_OMA_DA
/  Version 5.0  Nov 1994     - Compiles for WATCOM and Windows DLL as well
/  Version 5.01 1 Feb 1995   - Borland support for 3.45
/  Version 5.10 1 July 1996  - Improvements to Windows DLL, fixes for WATCOM
*/

const float DriverVersion = 5.01F;

#ifdef XSTAT
#define PRIVATE
#else
#define PRIVATE static
#endif

#ifdef _WINOMA_ 
#include <windows.h>
#endif
#include <dos.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys\timeb.h>
#include <math.h>

#ifdef __TURBOC__
#include <dir.h>
#endif
  
#include "oma4driv.h"
#include "detsetup.h"
#include "cmdtbl.h"
#include "driverrs.h"
#include "counters.h"
#include "oma4scan.h"
#include "access4.h"
#include "cntrloma.h"
#include "loadoma.h"
#include "cmdtbl.h"

PRIVATE SHORT finish_scan_setup(void);
SHORT detector_index = 0;

/* structures to hold data on non-uniform point, random track setup */
/* N.B. POINT_SETUP, TRACK_SETUP, TRIGGER_SETUP are global */
/* all three allow 256 possible entries */

#if !defined(__WATCOMC__) && !defined(_WINOMA_)

PRIVATE SHORT far load_flags[MAX_DETECTS];
DET_SETUP far det_setups[MAX_DETECTS];
DET_SETUP far *det_setup = det_setups;

pointstuff far POINT_SETUP;
trackstuff far TRACK_SETUP;
trigstuff  far TRIGGER_SETUP;
rapdastuff far RAPDA_SETUP;           /* RAPDA stuff. */

char far monitor_string[] = "OMA4 Monitor version    ";
char far monitor_fname[] = "MONITOR.BIN  ";
char far dacfile_fname[] = "SCANS.DBJ    ";
char far ascfile_fname[] = "SCANS.ABJ    ";

#else

PRIVATE SHORT load_flags[MAX_DETECTS] = { FALSE };
DET_SETUP det_setups[MAX_DETECTS];
DET_SETUP *det_setup = det_setups;

pointstuff POINT_SETUP;
trackstuff TRACK_SETUP;
trigstuff  TRIGGER_SETUP;
rapdastuff RAPDA_SETUP;           /* RAPDA stuff. */

char monitor_string[] = "OMA4 Monitor version    ";
char monitor_fname[] = "MONITOR.BIN  ";
char dacfile_fname[] = "SCANS.DBJ    ";
char ascfile_fname[] = "SCANS.ABJ    ";

#endif

static FLOAT min_et = 0.00036654F;
static FLOAT max_et = 51538.0F;
static const FLOAT five_ns = 5.0e-9F;
static const FLOAT ten_ms =  10.0e-3F;
const FLOAT RequiredMonitorVersion  = 3.0F;

PRIVATE char compare_buffer[26];
PRIVATE SHORT readsys_error = NO_ERROR;
PRIVATE BOOLEAN holdoff_finish_scan = FALSE;
PRIVATE BOOLEAN scan_setup_changed  = FALSE;

#define ck_det_errs (readsys_error == NO_ERROR)

/****************************************************************/
/*                                                              */
/*  writesys section                                            */
/*                                                              */
/****************************************************************/

#ifdef _WINOMA_
SHORT FAR PASCAL _export SetParam(enum det_command cmd, FLOAT param)
#else
SHORT _pascal SetParam(enum det_command cmd, FLOAT param)
#endif
{
  SHORT err, iparam;

  if (cmd > NUM_FLT_CMDS)
     err = ERROR_NOSUCH_PARAM;
  else
    {
    if (cmd >= NUM_INT_CMDS)
       err = set_flt_detect_param(cmd, param);
    else
       {
       iparam = (SHORT)param;
       err = set_int_detect_param(cmd, iparam);
       }
    }
  return (err);
}

#ifdef _WINOMA_
SHORT FAR PASCAL _export SetIntParam(enum det_command cmd, SHORT param)
#else
SHORT _pascal SetIntParam(enum det_command cmd, SHORT param)
#endif
{
  SHORT err;

  if (cmd > NUM_INT_CMDS)
     err = ERROR_NOSUCH_PARAM;
  else
    err = set_int_detect_param(cmd, param);

  return (err);
}

#ifdef _WINOMA_
SHORT FAR PASCAL _export SetPatch(USHORT PatchTag, USHORT Value)
#else
SHORT _pascal SetPatch(USHORT PatchTag, USHORT Value)
#endif
{
  return rep_asic_opbits(PatchTag, Value);
}

#ifdef _WINOMA_
SHORT FAR PASCAL _export GetPatch(USHORT PatchTag, USHORT *Value)
#else
SHORT _pascal GetPatch(USHORT PatchTag, USHORT *Value)
#endif
{
  *Value = get_asic_counter(PatchTag);
  return(0);
}


/* stop BorlandC from complaining about unused params */
#ifndef _MSC_VER
#pragma argsused
#endif
SHORT set_nothing(SHORT prm)
{
  return(NO_ERROR);
}

#ifndef _MSC_VER
#pragma argsused
#endif
SHORT set_flt_nothing(FLOAT prm)
{
  return(NO_ERROR);
}

SHORT set_RealDetector(SHORT there)
{
  if (there = 2)
    hi_shift = 1;
  else
    fake_detector = (USHORT)(!there);
  return(NO_ERROR);
}

SHORT set_DetPort(SHORT address)
{
  det_setup->det_addr = address;
  return (NO_ERROR);
}

SHORT set_Scans(SHORT scans)
{
  det_setup->scans = scans;
  set_DAC_counter(I0_COUNTER, scans);
  return (NO_ERROR);
}
  
SHORT set_Mems(SHORT mems)
{
  if ((USHORT)mems > (USHORT)det_setup->max_memory)
    {
    return(readsys_error = ERROR_TOO_MANY_MEMORIES);
    }
  else
    {
    det_setup->memories = mems;
    set_DAC_counter(J0_COUNTER, mems);
    set_Mem(det_setup->StartMemory+1);
    return (NO_ERROR);
    }
}
  
/**********************************************************************/
/*                                                                    */
/* Set the memory location for data acquisition to start placing data */ 
/* Also set the location of the source comp array                     */ 
/*                                                                    */
/**********************************************************************/
SHORT set_Mem(SHORT mem)
{
  if ((USHORT)mem > (USHORT)det_setup->max_memory)
    {
    return(readsys_error = ERROR_TOO_MANY_MEMORIES);
    }
  else
    {

    /* find the size of one frame */
    ULONG size = get_DAC_pointer(SIZE_PTR);

    /* find the size of the control area */
    ULONG offset = get_sharedcomm_size();

    /* allow mem to be 0 or 1 based */
    if (mem <= 0) mem = 1;

    /* mem is 0 based in method */
    det_setup->StartMemory = --mem;

    /* data address must be a multiple of 4 (longword boundary) */
    offset += offset % 4L;

    /* source comps start here */
    set_DAC_pointer(SRCCMP_PTR, offset);
    
    /* calc offset to data based on mem # supplied */
    offset = offset + (ULONG)((ULONG)mem * (ULONG)size);

    /* 'real' data starts after the source comp points */
    offset += 4L * ((ULONG)(det_setup->memories+1));
    
    set_DAC_pointer(FRAME_PTR, offset);

 /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! patch !!!!!!!!!!!!!!!!!!!!!!!!! */

 /* the source comp for rapda doesn't exist, so the source comp will be
    used to point to the area where data is placed during keep clean */

    if (det_setup->detector_type_index == RAPDA)
      {
      offset += (ULONG)(det_setup->memories) *
                (ULONG)size;
      set_DAC_pointer(SRCCMP_PTR, offset);
      }

 /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! patch !!!!!!!!!!!!!!!!!!!!!!!!! */

    return (NO_ERROR);
    }
}

SHORT set_PIA_Out(SHORT bits)
{
  USHORT ubits = (USHORT)bits | 0xFF00;

  set_DAC_counter(PIA_OUTBITS, ubits);
  return(NO_ERROR);
}
  
SHORT set_Ignores(SHORT igns)
{
  det_setup->ignored_scans = igns;
  set_DAC_counter(K0_COUNTER, igns);
  return (NO_ERROR);
}
  
SHORT set_Lastscan(SHORT last)
{
  set_DAC_counter(L1_COUNTER, last);
  return (NO_ERROR);
}
  
SHORT set_Preps(SHORT frames)
{
  det_setup->prep_frames = frames;
  set_DAC_counter(H0_COUNTER, frames);
  return (NO_ERROR);
}

FLOAT shift_line_time(void)
{
  if (det_setup->detector_type_index != EEV_CCD)
    {
    switch (det_setup->shift_time_index)
      {
      case 2:          /* SLOW shift time selected */
        return 30e-6F;
      case 1:          /* FAST shift time selected */
        return 3e-6F;
      case 0:          /* NORMAL shift time selected */
      default:
        if (det_setup->detector_type_index == TSP_CCD)
          return 12e-6F;
        else
          return 10e-6F;
      }
    }
  else
    return 8e-6F;
}

/* important - any time an operation is added to the exposure loop */
/* for example, get cooler status, then total count must be adjusted */

#define div_reals(a,b)  (SHORT)ceil((double)(a / b))

SHORT set_ExposeTime(FLOAT time)
{
  USHORT inner_count, outer_count, err = NO_ERROR;
  FLOAT total_count;

  if (det_setup->streakmode) /* streaker, ET is time each track is exposed */
    {
    ULONG delay_count;
    FLOAT min_time = shift_line_time();

    min_time *= det_setup->exposed_rows;

    if (det_setup->exposure_time < min_time) /* if previous setting illegal */
      det_setup->exposure_time = min_time;
                                             /* if trying to set illegal */
    if (time < min_time)
      {
      err = readsys_error = ERROR_EXPOSE_TIME;
      time = min_time;
      }

    /* ET counter may be 0; if so, set_StreakMode selects FC routine with */
    /* no delay loop */

    delay_count = (ULONG)((time - min_time) / no_op_time);
    if (delay_count  > 65535)
      return(readsys_error = ERROR_EXPOSE_TIME);
    det_setup->exposure_time = (FLOAT)delay_count * no_op_time + min_time;
    set_StreakMode(det_setup->streakmode);
    rep_asic_opbits(STREAK_ET, (USHORT)delay_count);
    return(err);
    }

  if ((time < min_et && time !=  0.0F) || time > max_et)
    {
    return(readsys_error = ERROR_EXPOSE_TIME);
    }
  if (det_setup->pulser_index < 2) /* not a pulser */
    {
    det_setup->exposure_time = time;
    total_count = (time / no_op_time);                    /* 3 us/ NOP */
    inner_count = (USHORT)(sqrt((double)total_count));
    if (!inner_count) inner_count++;
    outer_count = inner_count++;
  
    rep_asic_opbits(OH_OUT, outer_count);
    rep_asic_opbits(OH_IN, inner_count);
    if (det_setup->detector_type_index == EEV_CCD ||
        det_setup->detector_type_index == TS_1KSQ)
        rep_asic_opbits(CLOSE_TIME, 10000);
      else
        rep_asic_opbits(CLOSE_TIME, 3500);
    }
  else if (det_setup->pulser_width && time > 0.0F) /* some kind of pulser */
    {
    if (time < det_setup->pulser_width)
      {
      time = det_setup->pulser_width;
      det_setup->pulser_trigger_count = 1;
      }
    else if (time / det_setup->pulser_width > 65535.0F)
      {
      det_setup->pulser_width = time / 65535.0F;
      det_setup->pulser_trigger_count = 0xFFFF;
      }
    else
      det_setup->pulser_trigger_count = div_reals(time, det_setup->pulser_width);
    }
  else /* pulser, but exposure time is 0 */
    {
    det_setup->pulser_width = five_ns;
    det_setup->pulser_trigger_count = 1;
    }
  if (det_setup->detector_type_index == RAPDA)
    SetUpRapdaTables ();
  return (NO_ERROR);
}

struct RegionType ThisRegion;         /* Dummy to hold new regions. */
                    
    /* Figures out the Data offsets to the first pixel for all the
     * regions in the array.
     */

void CalcDataOffsets (void)
{
  struct RegionType *Ptr;
  SHORT i, j;
  for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
    {
    Ptr = &(RAPDA_SETUP.RapdaReg[i]);
    Ptr->DataOffset = 0;
      for (j = 0; j < RAPDA_SETUP.NumRegs; j++)
        if (Ptr->StartPixel > RAPDA_SETUP.RapdaReg[j].StartPixel)
          Ptr->DataOffset += RAPDA_SETUP.RapdaReg[j].Number;
    }
}


  /* Given a region ET, and the BaseET, does the following:
   *   Limits the RegionET to <= the BaseET.
   *   Determines the largest value of n such that (BaseET / 2ü) > RegionET.
   *   Gives Error if n > 16.
   */
SHORT set_RegionET(FLOAT Time)
{
  char max_n, min_n, n;
  SHORT  i, Error = NO_ERROR;
  FLOAT temp;                         /* Only valid for RAPDA's. */
  if (det_setup->detector_type_index == RAPDA)
    {
    get_ExposeTime(&temp);
    if (Time > temp)
      Time = temp;
    for (n = 0; Time < temp; n++)
      temp /= 2.0F;
    if (n <= MAX_N)
      {
      max_n = 0, min_n = MAX_N;
      for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
        {
        max_n = (char) max (RAPDA_SETUP.RapdaReg[i].n, max_n);
        min_n = (char) min (RAPDA_SETUP.RapdaReg[i].n, min_n);
        }
      n = max_n - n;
      RAPDA_SETUP.RapdaReg[max (0, det_setup->Current_Point-1)].n = (char)n;
      n = (char) min (min_n, n);
      if (n)
        for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
          RAPDA_SETUP.RapdaReg[i].n -= n;
      }
    else
      {
      Error = readsys_error = ET_RANGE_ERR;
      }
    }
  return(Error);
}


PRIVATE void set_max_mem(ULONG data_size)
{
  ULONG max_mem = 0x7FFF;

  if (data_size)
    max_mem = ((ULONG)det_setup->memory_size / data_size) - 
              (ULONG)(det_setup->detector_type_index == RAPDA);
  if (max_mem > 0xFFFF) max_mem = 0xFFFF;
  det_setup->max_memory = (SHORT)max_mem;
  set_DAC_pointer(SIZE_PTR, data_size);
}

PRIVATE void set_size_pointer(void)
{
  ULONG data_size;

  data_size = (ULONG)det_setup->points * (ULONG)det_setup->tracks;
  data_size *= (ULONG)((det_setup->data_word_size + 1)*2); /* make index into size */
  if (!data_size) data_size = 0xFFFF;
  set_max_mem(data_size);
  set_DAC_pointer(SIZE_PTR, data_size);
  set_Mem(det_setup->StartMemory+1);
}

SHORT set_PointSize(SHORT size)
{
  det_setup->data_word_size = size;
  set_size_pointer();
  return(NO_ERROR);
}

SHORT set_DetectorType(SHORT type)
{
  det_setup->detector_type_index = type;
  return(NO_ERROR);
}
  
SHORT set_DaProg(SHORT prog)
{
  SHORT err = NO_ERROR;

  if ((prog > (SHORT)(NumDACBlks - DA_LIVE)-1) || (prog < 1))
    {
    err = readsys_error = ERROR_NOSUCH_DAPROG;
    }
  else
    det_setup->DA_mode = prog;
  return (err);
}

SHORT set_Freq(SHORT freq)
{
  det_setup->line_freq_index = (freq == 50);
  return (NO_ERROR);
}

/* calc size of dead region in serial register before data */

PRIVATE SHORT calc_lead_columns(void)
{
  SHORT zero;

  if (det_setup->shiftmode == 0)
    {
    if (det_setup->pointmode == 1)
      zero = POINT_SETUP.X0[1];
    else
      zero = POINT_SETUP.X0[0];
    }
  else  /* not simple mode */
    {
    if (det_setup->trackmode == 1)
      zero = TRACK_SETUP.Y0[1];
    else                         
      zero = TRACK_SETUP.Y0[0];
    }
  /* decrement zero because X0 is 1-based; skip dead pixels too */
  /* and allow for Normal Pixel done instead of last Trash Pixel */
  return (--zero) + (det_setup->DumXLead - 1); 
}

/* calc size of ignored region in serial register after data */

PRIVATE USHORT calc_trail_columns(void)
{
  SHORT zero, Delta, groups;
  
  if (det_setup->shiftmode == 0)
    {
    groups = det_setup->points;
    if (det_setup->pointmode == 1)         /* random points */
      {
      zero =   POINT_SETUP.X0[groups];
      Delta =  POINT_SETUP.DeltaX[groups];
      }
    else                                   /* uniform points */
      {
      zero =    POINT_SETUP.X0[0];
      Delta =   POINT_SETUP.DeltaX[0] * groups;
      }
    }
  else  /* not simple mode */
    {
    groups = det_setup->tracks;
    if (det_setup->trackmode == 1)         /* random tracks */
      {
      zero =   TRACK_SETUP.Y0[groups];
      Delta =  TRACK_SETUP.DeltaY[groups];
      }
    else                                   /* uniform tracks */
      {
      zero =   TRACK_SETUP.Y0[0];
      Delta =  TRACK_SETUP.DeltaY[0] * groups;
      }
    }
  return(det_setup->ActiveX      /* the total visible array */
         + det_setup->DumXTrail  /* plus the points you don't see */
         + 1                     /* because points are 1 based on menu */
         - zero                  /* minus the points already skipped */
         - Delta);               /* minus the points read */
}

/* Return truth that readout needs to be as fast as possible (no wait for shutter or ET) */
PRIVATE BOOLEAN IsFastestReadout(void)
{
//  return FALSE;

  return (det_setup->shutter_forced_mode &&
         det_setup->exposure_time <= det_setup->min_exposure_time);
}

/* Return truth that region of interest extends to the far edge of the array */
PRIVATE BOOLEAN IsFullyBinned(void)
{
  int index, size;

  if (det_setup->shiftmode == 0)
    {
    index = det_setup->tracks;
    if (det_setup->trackmode == 1)
      size = TRACK_SETUP.DeltaY[index] + TRACK_SETUP.Y0[index];
    else
      size = TRACK_SETUP.DeltaY[0] * index;
    }
  else
    {
    index = det_setup->points;
    if (det_setup->pointmode == 1)
      size = POINT_SETUP.DeltaX[index] + POINT_SETUP.X0[index];
    else
      size = POINT_SETUP.DeltaX[0] * index;
    }
  return (size >= det_setup->ActiveY);
}

/* calculate the number of lines between shift register and data area */
PRIVATE SHORT calc_lead_lines(void)
{                                  
  SHORT zero;                      

  if (det_setup->shiftmode == 0)
    {
    if (det_setup->trackmode == 1)
      zero = TRACK_SETUP.Y0[1];
    else
      zero = TRACK_SETUP.Y0[0];
    }
  else
    {
    if (det_setup->pointmode == 1)
      zero = POINT_SETUP.X0[1];
    else
      zero = POINT_SETUP.X0[0];
    }

  if ((!IsFastestReadout()) || !(IsFullyBinned()))
    {
    zero += det_setup->DumYLead;
    }
  zero -= 1;

  return(zero);
}

/* calculate the number of lines to skip after the data area */

PRIVATE SHORT calc_trail_lines(void)
{
  SHORT Zero, Delta, groups;        

  if (det_setup->outputReg == 2) /* if Dual mode, */
    {                            /* really no trail lines since lead lines */
    groups = 0;                  /* does both sides of the array at once */
    }                            
  else
    {
    if (det_setup->shiftmode == 0)
      {
      groups =  det_setup->tracks;
      if (det_setup->trackmode == 1)
        {
        Zero =   TRACK_SETUP.Y0[groups];
        Delta =  TRACK_SETUP.DeltaY[groups];
        }
      else
        {
        Zero =   TRACK_SETUP.Y0[0];
        Delta =  TRACK_SETUP.DeltaY[0] * groups;
        }
      }
    else
      {
      groups =  det_setup->points;
      if (det_setup->pointmode == 1)
        {
        Zero =   POINT_SETUP.X0[groups];
        Delta =  POINT_SETUP.DeltaX[groups];
        }
      else
        {
        Zero =   POINT_SETUP.X0[0];
        Delta =  POINT_SETUP.DeltaX[0] * groups;
        }
      }
    groups = det_setup->ActiveY + det_setup->DumYTrail - (Zero + Delta - 1);
    if (IsFastestReadout() && IsFullyBinned())
      {
      groups += det_setup->DumYLead;
      }
    }
  return(groups);
}

SHORT set_AntiBloom(SHORT percent)
{
  USHORT inloop, outloop, rem;
  
  det_setup->anti_bloom_percent = percent;

  rem = calc_antibloom_counts((USHORT)percent,
                              (USHORT)calc_lead_lines(),
                              &inloop, &outloop);
  rep_asic_opbits(I_LEAD_INNER, inloop);
  rep_asic_opbits(I_LEAD_OUTER, outloop);
  rep_asic_opbits(I_LEAD_REM, rem);

  rem = calc_antibloom_counts((USHORT)percent,
                              (USHORT)calc_trail_lines(),
                              &inloop, &outloop);
  rep_asic_opbits(I_TRAIL_INNER, inloop);
  rep_asic_opbits(I_TRAIL_OUTER, outloop);
  rep_asic_opbits(I_TRAIL_REM, rem);

  return (NO_ERROR);
}

PRIVATE SHORT finish_scan_setup(void)
{
  SHORT image_groups, serial_groups, err = NO_ERROR;

  if (!(holdoff_finish_scan || (det_setup->detector_type_index == NO_CARD)))
    {
    set_holdoff_release();
    if (det_setup->shiftmode == 0)
      {
      image_groups = det_setup->tracks;
      serial_groups = det_setup->points;
      }
    else
      {
      image_groups = det_setup->points;
      serial_groups = det_setup->tracks;
      if (!serial_groups) serial_groups++;
      }
    if (det_setup->outputReg != 2) /* if not Dual mode */
      {
      set_DAC_counter(T0_COUNTER, image_groups);
      rep_asic_opbits(T0_COUNTER, image_groups);
      if (det_setup->streakmode < 2)
        rep_asic_opbits(STRK_TRACKS, image_groups);
      else
        rep_asic_opbits(STRK_TRACKS, image_groups/2);
      rep_asic_opbits(SM_SGROUPS, (serial_groups - 1));
      rep_asic_opbits(S_GROUP_MIN1, serial_groups - 1);
      rep_asic_opbits(SCNT, serial_groups);
      }
    else
      {
      set_DAC_counter(T0_COUNTER, image_groups/2);
      rep_asic_opbits(T0_COUNTER, image_groups/2);
      if (det_setup->streakmode < 2)
        rep_asic_opbits(STRK_TRACKS, image_groups/2);
      else
        rep_asic_opbits(STRK_TRACKS, image_groups/4);
      rep_asic_opbits(SM_SGROUPS, (serial_groups - 1));
      rep_asic_opbits(S_GROUP_MIN1, (serial_groups * 2) - 1);
      rep_asic_opbits(SCNT, serial_groups);
      }

    if (det_setup->shiftmode == 0)
      {
      rep_asic_opbits(PCNT, (det_setup->points * 2));
      set_asic_pointer(TCNT, (ULONG)(det_setup->tracks - 1) *
                            (ULONG)(det_setup->points * 2) - 2);
      }
    else
      {
      rep_asic_opbits(PCNT, (det_setup->points * 2) - 2);
      set_asic_pointer(TCNT, (ULONG)(det_setup->tracks - 1) *
                            (ULONG)(det_setup->points * 2));
      }

    rep_asic_opbits(TRACKLEN, (det_setup->points * 2));

    rep_asic_opbits(S_IGNORE_START, calc_lead_columns());  /* */
    if (det_setup->detector_type_index == RAPDA)
      {
      rep_asic_opbits(MEMCNT, (det_setup->memories));
      clr_asic_opbits(LEAD_PIXELS, 0x3FF);
      set_asic_opbits(LEAD_PIXELS, POINT_SETUP.X0[0]);
      }
    else
      {
      rep_asic_opbits(S_IGNORE_END, calc_trail_columns());   /* */

      set_AntiBloom(det_setup->anti_bloom_percent);
      set_PointMode(det_setup->pointmode);
      set_TrackMode(det_setup->trackmode);
      }

    /* generate serial read code */

    if ((det_setup->trackmode && det_setup->shiftmode) ||
        (det_setup->pointmode && (!det_setup->shiftmode)))
      readsys_error |= rserial_setup(det_setup->pix_time_index);

    /* generate image read code */

    if ((det_setup->pointmode && det_setup->shiftmode) ||
        (det_setup->trackmode && (!det_setup->shiftmode)))
      readsys_error |= rimage_setup(det_setup -> anti_bloom_percent);

    clr_holdoff_release();
    release_OMA_and_init();
    set_size_pointer();
    scan_setup_changed = FALSE;
    }
  return (err);
}

SHORT set_OutputReg(SHORT A_B) /*   _A_B__|___0___|____1__|___2____*/
{                              /* Shiftreg|___A___|____B__|__Dual__*/
  USHORT success;

  success = set_shiftreg_status(A_B);
  if (success)
    {
    det_setup->outputReg = A_B;
    success = set_pixtime_status(det_setup->pix_time_index);
    scan_setup_changed = TRUE;
    }
  return (success ? NO_ERROR : DRIV_ERROR);
}

SHORT set_X0(SHORT X0)
{
  SHORT pt = max (0, det_setup->Current_Point-1);

  if (X0 == 0) return (readsys_error = ERROR_VALUE_INCORRECT);

  if (det_setup->detector_type_index == RAPDA)
    RAPDA_SETUP.RapdaReg[pt].StartPixel = X0;
  else
     POINT_SETUP.X0[(det_setup->pointmode) ? pt+1 : 0] = X0;
  set_PointMode(det_setup->pointmode);
  scan_setup_changed = TRUE;
  return(0);
}

SHORT set_DeltaX(SHORT DeltaX)
{
  if (DeltaX == 0) return (readsys_error = ERROR_VALUE_INCORRECT);
  
  POINT_SETUP.DeltaX[(det_setup->pointmode) ?
                det_setup->Current_Point : 0] = DeltaX;
//  if (det_setup->detector_type_index == TS_1KSQ)
//    DeltaX -= 1;
  rep_asic_opbits(X_DELTA, DeltaX);
  set_PointMode(det_setup->pointmode);
  scan_setup_changed = TRUE;
  return(0);
}

SHORT set_RegSize(SHORT RegSize)
{
  SHORT pt = max (0, det_setup->Current_Point-1);
  if (RegSize && (RegSize < MIN_PXLS))
    {
    return(readsys_error = SIZE_ERR);
    }
  RAPDA_SETUP.RapdaReg[pt].Number = RegSize;
  scan_setup_changed = TRUE;
  return(NO_ERROR);
}

SHORT set_CurrentPoint(SHORT point)
{
  if (((point <= det_setup->points) && det_setup->pointmode) ||
     ((det_setup->detector_type_index == RAPDA) && 
                                          (point <= RAPDA_SETUP.NumRegs)))
    {
    det_setup->Current_Point = point;
    return(NO_ERROR);
    }
  return(readsys_error = ERROR_POINT_NUMBER);
}

SHORT set_DaActive(SHORT Active)
{
  set_DAC_counter(DA_DONE, !Active);
  det_setup->da_active = Active;
  return(NO_ERROR);
}

SHORT set_ActiveX(SHORT ActiveX)
{
  det_setup->ActiveX = ActiveX;
  set_Points(ActiveX);
  set_CurrentPoint(0);
  set_X0(1);
  set_DeltaX(1);
  set_PointMode(0);
  return(finish_scan_setup());
}

SHORT set_CurrentTrack(SHORT number)
{
   if ((number <= det_setup->tracks) && det_setup->trackmode)
    {
    det_setup->Current_Track = number;
    return NO_ERROR;
    }
  return(readsys_error = ERROR_TRACK_NUMBER);
}

SHORT set_Y0(SHORT Y0)
{
  if (Y0 == 0) return (readsys_error = ERROR_VALUE_INCORRECT);

  if (Y0 > det_setup->ActiveY) Y0 = det_setup->ActiveY;
  if (det_setup->trackmode)

     TRACK_SETUP.Y0[det_setup->Current_Track] = Y0;
  else
     TRACK_SETUP.Y0[0] = Y0;
   scan_setup_changed = TRUE;
   set_TrackMode(det_setup->trackmode);
   return(0);
}

SHORT set_DeltaY(SHORT DeltaY)
{
  if (DeltaY == 0) return (readsys_error = ERROR_VALUE_INCORRECT);
  
  if (det_setup->trackmode)
     TRACK_SETUP.DeltaY[det_setup->Current_Track] = DeltaY;
  else
     TRACK_SETUP.DeltaY[0] = DeltaY;
  rep_asic_opbits(Y_DELTA, DeltaY);
  scan_setup_changed = TRUE;
  set_TrackMode(det_setup->trackmode);
  return(0);
}

SHORT set_ActiveY(SHORT ActiveY)
{
  det_setup->ActiveY = ActiveY;
  set_CurrentTrack(0);
  set_Y0(1);
  set_DeltaY(1);
  set_Tracks(1);
  set_TrackMode(0);
  set_AntiBloom(det_setup->anti_bloom_percent);
  set_size_pointer();
  return(finish_scan_setup());
}

PRIVATE SHORT get_max_tracks(void)
{
  return (det_setup->shiftmode ? det_setup->ActiveX : det_setup->ActiveY);
}

PRIVATE SHORT get_max_points(void)
{
  return (det_setup->shiftmode ? det_setup->ActiveY : det_setup->ActiveX);
}

PRIVATE SHORT get_active_point(void)
{
  return(det_setup->pointmode ? det_setup->Current_Point : 0);
}

PRIVATE SHORT get_active_track(void)
{                        
  return(det_setup->trackmode ? det_setup->Current_Track : 0);
}

SHORT set_PointMode(SHORT mode)  /* mode 0 = uniform, 1 = random */
{
  SHORT i, err = NO_ERROR, max = get_max_points();

  if (det_setup->pointmode == 1 && mode == 0)
     set_CurrentPoint(0);
  det_setup->pointmode = mode;

  if (det_setup->shiftmode == 0)        /* simple mode */
    {
    USHORT serial_address = 0;        /* addr of ASIC serial reg code */
    if (mode)                           /* if random serial readout */
      serial_address = FC_mode_list[SER_RAN_ALL];    /* random points simple */
    else
      {
      if (POINT_SETUP.DeltaX[0] > 1)
        serial_address = FC_mode_list[SER_UNI_DXS];  /* uniform points simple dX > 1 */
      else
        serial_address = FC_mode_list[SER_UNI_MXS];  /* uniform points simple dX = 1*/
      }

    if (serial_address == 0)              /* if no address found */
      {
      err = readsys_error = ERROR_NO_SERIAL_CODE;
      }
    else
      rep_asic_opbits(SER_ACQ, serial_address);  /* point GOSUBs to right */
    }                                            /* serial routine */

    if (mode == 1 && err == NO_ERROR)            /* if random mode */
      {
      if (det_setup->points > max)
        {
        det_setup->points = max;
        err = readsys_error = ERROR_ADJUSTED_POINT;
        }

      if (POINT_SETUP.X0[1] == 0)
        POINT_SETUP.X0[1] = 1;

      if (POINT_SETUP.X0[1] > max)
        {
        POINT_SETUP.X0[1] = 1;
        err = readsys_error = ERROR_ADJUSTED_POINT;
        }

      if (POINT_SETUP.DeltaX[1] == 0)
        POINT_SETUP.DeltaX[1] = 1;

      for (i = 2;i < det_setup->points+1; i++)
         {
         if (POINT_SETUP.X0[i] < POINT_SETUP.X0[i-1]+POINT_SETUP.DeltaX[i-1])
          {
          if (POINT_SETUP.X0[i])
            {
            err = readsys_error = ERROR_ADJUSTED_POINT;
            }
          POINT_SETUP.X0[i] = POINT_SETUP.X0[i-1]+POINT_SETUP.DeltaX[i-1];
          }
         if (POINT_SETUP.DeltaX[i] < 1)
          {
          if(POINT_SETUP.DeltaX[i])
            {
            err = readsys_error = ERROR_ADJUSTED_POINT;
            }
          POINT_SETUP.DeltaX[i] = 1;
          }
         if ((POINT_SETUP.X0[i] + POINT_SETUP.DeltaX[i]) > det_setup->ActiveX)
          {
          POINT_SETUP.DeltaX[i] = 1;
          err = readsys_error = ERROR_ADJUSTED_POINT;
          }
         }
      }
    scan_setup_changed = TRUE;
    return (err);
}
  
SHORT set_TrackMode(SHORT mode)
{
  SHORT i, err = NO_ERROR, max = get_max_tracks();

  if (det_setup->trackmode == 1 && mode == 0)
     set_CurrentTrack(0);
  det_setup->trackmode = mode;

  if (det_setup->shiftmode)                   /* classic mode */
    {
    USHORT serial_address = 0;             /* addr of ASIC serial reg code */
    if (mode)
      serial_address = FC_mode_list[SER_RAN_ALL]; /* random tracks classic */
    else
      {
      if (TRACK_SETUP.DeltaY[0] > 1)
        serial_address = FC_mode_list[SER_UNI_DXC]; /* uniform tracks classic */
      else
        serial_address = FC_mode_list[SER_UNI_MXC]; /* uniform tracks classic */
      }

    if (serial_address == 0)            /* if no address found */
      {
      err = readsys_error = ERROR_NO_SERIAL_CODE;
      }
    else
      rep_asic_opbits(SER_ACQ, serial_address);  /* point GOSUBs to right */
                                                 /* serial routine */
    }
    
  if (mode == 1 && err == NO_ERROR)          /* if random mode */
    {                                        /* make sure things make sense */
    if (det_setup->tracks > max)
      {
      det_setup->tracks = max;
      err = readsys_error = ERROR_TRACK_NUMBER;
      }

    if ((TRACK_SETUP.Y0[1] == 0) || (TRACK_SETUP.Y0[1] > max))
      {
      if (TRACK_SETUP.Y0[1])
        {
        err = readsys_error = ERROR_ADJUSTED_TRACK;
        }
      TRACK_SETUP.Y0[1] = 1;
      }
    if (TRACK_SETUP.DeltaY[1] == 0)
      TRACK_SETUP.DeltaY[1] = 1;
    for (i = 2;i <= det_setup->tracks+1; i++)
       {
       if (TRACK_SETUP.Y0[i] < TRACK_SETUP.Y0[i-1]+TRACK_SETUP.DeltaY[i-1])
        {
        if (TRACK_SETUP.Y0[i])
          {
          err = readsys_error = ERROR_ADJUSTED_TRACK;
          }
        TRACK_SETUP.Y0[i] = TRACK_SETUP.Y0[i-1]+TRACK_SETUP.DeltaY[i-1];
        }

       if (TRACK_SETUP.Y0[i] < 1)
        TRACK_SETUP.Y0[i] = 1;

       if ((TRACK_SETUP.DeltaY[i] < 1) ||
           (TRACK_SETUP.DeltaY[i] + TRACK_SETUP.Y0[i] > det_setup->ActiveY + 1))
          {
          if (TRACK_SETUP.DeltaY[i])
            {
            err = readsys_error = ERROR_ADJUSTED_TRACK;
            }
          TRACK_SETUP.DeltaY[i] = 1;
          }
       }
    }
  scan_setup_changed = TRUE;
  return(err);
}
  
SHORT set_ShiftMode(SHORT mode)
{
   det_setup -> shiftmode = mode;
   scan_setup_changed = TRUE;
   return (0);
}
  
SHORT set_Points(SHORT points)
{
  SHORT max = get_max_points();

  if (points > max)
    {
    return(readsys_error = ERROR_POINT_NUMBER);
    }
  det_setup->points = points;
  if (det_setup->pointmode)
    POINT_SETUP.number = points;
  scan_setup_changed = TRUE;
  if (det_setup->da_active)
    stop_OMA_DA(1);
  set_size_pointer();
  return(0);
}
  
SHORT set_Regions (SHORT regions)
{
  if (regions > (MAX_PXL / MIN_PXLS))
    {
    return(readsys_error = ERROR_POINT_NUMBER);
    }
  if (det_setup->detector_type_index == RAPDA)
    {
    det_setup->regions =
    RAPDA_SETUP.NumRegs = regions;
    get_Points((SHORT *)&det_setup->points);
    }
  return (finish_scan_setup());
}

SHORT set_Tracks(SHORT tracks)
{
  SHORT err = FALSE, max = get_max_tracks();

  if (tracks > max)
    {
    return(readsys_error = ERROR_TRACK_NUMBER);
    }
  else
    {
    if (det_setup->outputReg == 2 &&  /* if Dual mode */
       !det_setup->shiftmode)         /* and simple */
      {
      SHORT temp_trax = tracks / 2;     /* make sure tracks even number */
      if ((temp_trax << 1) != tracks || !temp_trax)
        {
        return(readsys_error = ERROR_ADJUSTED_TRACK);
        }
      tracks = temp_trax * 2;
      if (!tracks) tracks = 2;
      if(!det_setup->trackmode)      /* if uniform */
        {                           
        TRACK_SETUP.Y0[0] = (det_setup->ActiveY / 2) - (temp_trax * TRACK_SETUP.DeltaY[0]) + 1;
        }
      else
        {
        SHORT i, j = det_setup->ActiveY / 2, k = tracks;

        for (i = 1; i < temp_trax; i++)
          {
          TRACK_SETUP.Y0[k-i+1] = TRACK_SETUP.Y0[i] + j;
          TRACK_SETUP.DeltaY[k-i+1] = TRACK_SETUP.DeltaY[i];
          }
        }
      }
    det_setup->tracks = tracks;
    if (det_setup->trackmode)
      TRACK_SETUP.number = tracks;
    set_size_pointer();
    set_AntiBloom(det_setup->anti_bloom_percent);
    scan_setup_changed = TRUE;
    return err;
    }
}
  
/************************** LCI pulser stuff *******************************/

SHORT set_PulserType(SHORT type)
{
  det_setup->pulser_index = type;
  return (NO_ERROR);
}

void calc_pulse_patches(FLOAT value, USHORT * a, USHORT * b, USHORT * c)
{
  ULONG temp;

  temp = (ULONG)((value + 5.0e-10F) * 1.0e9F); /* convert to ns */

  *a = (USHORT)(temp % 200L);        /* get remainder, analog part */

  temp /= 200L;                      /* discard analog part, get digital */

  *b = (USHORT)temp & 0xFF;          /* digital lsb */

  *c = ((USHORT)temp / 0xFF) & 0xFF; /* digital msb */
  
}

SHORT set_PulserDelay(FLOAT delay)
{
  USHORT anabits, lsbits, msbits;

  det_setup->pulser_delay = delay;

  calc_pulse_patches(delay, &anabits, &lsbits, &msbits);

  clr_asic_opbits(LCI_DELAY_ANA, 0xFF);
  set_asic_opbits(LCI_DELAY_ANA, anabits);

  clr_asic_opbits(LCI_DELAY_LSB, 0xFF);
  set_asic_opbits(LCI_DELAY_LSB, lsbits);

  clr_asic_opbits(LCI_DELAY_MSB, 0xFF);
  set_asic_opbits(LCI_DELAY_MSB, msbits);

  return (NO_ERROR);
}
  
SHORT set_PulserWidth(FLOAT width)
{
  USHORT anabits, lsbits, msbits;

  if (width < five_ns || width > ten_ms)
    {
    return(readsys_error = ERROR_VALUE_INCORRECT);
    }

  det_setup->pulser_width = width;

  width -= five_ns;

  calc_pulse_patches(width, &anabits, &lsbits, &msbits);

  clr_asic_opbits(LCI_WIDTH_ANA, 0xFF);
  set_asic_opbits(LCI_WIDTH_ANA, anabits);

  clr_asic_opbits(LCI_WIDTH_LSB, 0xFF);
  set_asic_opbits(LCI_WIDTH_LSB, lsbits);

  clr_asic_opbits(LCI_WIDTH_MSB, 0xFF);
  set_asic_opbits(LCI_WIDTH_MSB, msbits);

  return (NO_ERROR);
}

SHORT set_PulserDelayInc(FLOAT delinc)
{
  USHORT anabits, lsbits, msbits;

  det_setup->pulser_delay_inc = delinc;

  calc_pulse_patches(delinc, &anabits, &lsbits, &msbits);

  clr_asic_opbits(LCI_DELTA_ANA, 0xFF);
  set_asic_opbits(LCI_DELTA_ANA, anabits);

  clr_asic_opbits(LCI_DELTA_LSB, 0xFF);
  set_asic_opbits(LCI_DELTA_LSB, lsbits);

  clr_asic_opbits(LCI_DELTA_MSB, 0xFF);
  set_asic_opbits(LCI_DELTA_MSB, msbits);

  return (NO_ERROR);
}

SHORT set_PulserDelayRange(FLOAT range)
{
  if (det_setup->pulser_delay_inc)
    {
    return set_Mems((SHORT)ceil((range / det_setup->pulser_delay_inc)));
    }
  else
    {
    return(readsys_error = ERROR_ILLEGAL_RANGE);
    }
}

#define EXT_SHUTTER  0x00     /* Use external shutter BNC. */ 
#define INT_SHUTTER  0x01     /* Use internal shutter.     */ 
#define OPTICAL      0x02     /* Optical trigger source.   */ 
#define EXT_POS      0x04     /* External positive edge.   */ 
#define EXT_NEG      0x08     /* External negitive edge.   */ 

#define PULSED       0x10     /* Pulsed mode enabled.      */ 

#define USE_SYNC     0x20     /* Trigger OMA w/Sync.       */ 
#define USE_TRIG     0x00     /* Trigger OMA w/Trig.       */ 
#define BEEP_ON      0x40     /* Enable beeper.            */ 
#define ARM_PTRIG    0x80     /* Arm input triggers.       */ 

void set_PulserConfig(void)
{
  USHORT cfg_byte;

  cfg_byte =
    (1 << det_setup->pulser_trigsrc_index) >> 1; /* dis, opt, +ext or -ext */

  cfg_byte |= ((det_setup->pulser_intensifier_mode == 1) * PULSED);
  cfg_byte |= USE_TRIG; /* always use JIFNOT to run LCI */

  clr_asic_opbits(LCI_CONFIG, 0xFF);
  set_asic_opbits(LCI_CONFIG, cfg_byte);

  cfg_byte |= ((det_setup->pulser_audio_index) * BEEP_ON);
  cfg_byte |= ARM_PTRIG;

  clr_asic_opbits(LCI_CMD, 0xFF);
  set_asic_opbits(LCI_CMD, cfg_byte);
}

SHORT set_PulserTrigSrc(SHORT source)
{
  if (source > 4 || source < 0)
    {
    return(readsys_error = ERROR_VALUE_INCORRECT);
    }
  else
    det_setup->pulser_trigsrc_index = source;
  set_PulserConfig();

  if ((source == POS_SLOPE) || (source == NEG_SLOPE))
    {
    set_PulserTrigThresh(det_setup->pulser_trig_threshold);
    }

  return(NO_ERROR);
}

SHORT set_PulserAudio(SHORT onoff)
{
  det_setup->pulser_audio_index = (onoff != 0);
  set_PulserConfig();
  return(NO_ERROR);
}

/* set the pulser to gated (mode == 1) or CW (mode = 0) */
SHORT set_IntensifierMode(SHORT mode)
{
  if (mode > 2 || mode < 0)
    {
    return(readsys_error = ERROR_VALUE_INCORRECT);
    }
  else
  det_setup->pulser_intensifier_mode = mode;
  set_PulserConfig();
  return(NO_ERROR);

}

SHORT set_PulserScanTimeout(FLOAT timeout)
{
  det_setup->pulser_scan_timeout = timeout;
  return(NO_ERROR);
}

SHORT set_PulserTrigCount(SHORT Count)
{
  if (Count < 1)
    {
    return(readsys_error = ERROR_VALUE_INCORRECT);
    }

  det_setup->pulser_trigger_count = Count--;

  clr_asic_opbits(LCI_CNT_MSB, 0xFF);
  set_asic_opbits(LCI_CNT_MSB, Count / 0xFF);

  clr_asic_opbits(LCI_CNT_LSB, 0xFF);
  set_asic_opbits(LCI_CNT_LSB, Count & 0xFF);

  set_ExposeTime((Count+1) * det_setup->pulser_width); 

  return (NO_ERROR);
}
  
#ifndef __WATCOMC__
FLOAT const far vstep = (FLOAT)(13.888F / 0xFF);
#else
FLOAT const vstep = (FLOAT)(13.888F / 0xFF);
#endif

static FLOAT calc_volts(USHORT index)
{
  FLOAT voltval;

  if (index == 0x80)
    voltval =  0.0F;
  else
    {
    voltval = ((FLOAT)(index * vstep) - 6.944F) + 0.005F;
    }
  return(voltval);
 }

SHORT set_TrigThreshVolts(FLOAT voltval)
{
  USHORT index;

  if (voltval > 5.0F || voltval < -5.0F)
    {
    return(readsys_error = ERROR_VALUE_INCORRECT);
    }

  if (fabs(voltval) < 0.05F)
    index = 0x80;
  else
    {
    index = (USHORT)(((voltval + 6.944F) / vstep) + 0.5F);
    }

  det_setup->pulser_trig_threshold = index & 0xFF;

  clr_asic_opbits(LCI_TRIG_LEVEL, 0xFF);
  set_asic_opbits(LCI_TRIG_LEVEL, index & 0xFF);

  return(NO_ERROR);
}

/* given a threshold value from 0 to 0xFF, calc the */
/* equivalent voltage and set the LCI trigger thresh*/
/* (the LCI threshold is actually set by sending it */
/* a value from 0 to 255, but I convert to volts and*/
/* back to keep everything in terms of volts.)      */

SHORT set_PulserTrigThresh(SHORT thresh)
{
  FLOAT fthresh;

  if ((USHORT)thresh > 0xFF)
    {
    return(readsys_error = ERROR_VALUE_INCORRECT);
    }

  fthresh = calc_volts((USHORT)thresh);

  return(set_TrigThreshVolts(fthresh));
}

SHORT set_ShutterMode(SHORT mode)
{            
  SHORT success;
  det_setup->shutter_forced_mode = mode;
  success = set_shutter_forced_status(mode);
  scan_setup_changed = TRUE;
  return success ? NO_ERROR : DRIV_ERROR;
}
  
SHORT set_ShutterOpenSync(SHORT syncindex)
{
  det_setup->shutter_open_sync_index = syncindex;
  set_StreakMode(det_setup->streakmode);
  if (set_sync_open_status(syncindex))
    return (NO_ERROR);
  else
    return (DRIV_ERROR);
}

SHORT set_ShutterCloseSync(SHORT syncindex)
{
  det_setup->shutter_close_sync_index = syncindex;
  if (set_sync_close_status(syncindex))
    return (NO_ERROR);
  else
    return (DRIV_ERROR);
}
  
SHORT set_SourceCompMode(SHORT mode)
{
  det_setup->source_comp_index = mode;
  if (set_source_comp_status(mode))
    return (NO_ERROR);
  else
    return (DRIV_ERROR);
}

SHORT set_SameET(SHORT mode)
{
  det_setup->same_et = 0;
  if (mode) det_setup->same_et |= 1;
  set_DAC_counter(SAME_ET, mode);
  return(NO_ERROR);
}

/* Clear data acquisition memory from curve Start for Count curves */

SHORT ClearMems(SHORT Start, SHORT Count)
{
  ULONG * pmem;
  SHORT i, bytes;
  
  get_Bytes(&bytes);
  pmem = calloc(bytes, 1);

  if (pmem)
    {
    for (i = Start; i < Start + Count; i++)
      {
  #ifndef __WATCOMC__
      put_OMA4_data((char far *)pmem, bytes, i);
  #else
      put_OMA4_data((char *)pmem, bytes, i);
  #endif
      }
    free(pmem);
    return(NO_ERROR);
    }
  else
    {
    return(readsys_error = ERROR_NO_CODE_SPACE);
    }
}

/* Clear one designated curve */

SHORT set_ClearMem(SHORT mem)
{
  return ClearMems(mem, 1);
}

/* Clear the curve areas which are used in acquisition */

SHORT ClearRunMems(void)
{
  SHORT StartMem, Tracks, Mems;
      
  get_Mem(&StartMem);             /* MEM is 1 based */
  StartMem -= 1;
  get_Mems(&Mems);
  get_Tracks(&Tracks);

  Mems *= Tracks;

  return ClearMems(StartMem, Mems);
}

/* Clear all the curve areas, used or not */

SHORT ClearAllMems(void)
{
  SHORT Mems, Tracks;
  
  get_MaxMem(&Mems);
  get_Tracks(&Tracks);

  Mems *= Tracks;

  return ClearMems(0, Mems);
}

SHORT set_ControlMode(SHORT mode)
{
   SHORT i;
   det_setup->control_index = mode;
   i = set_inex_shutter_status((mode== 0));
   return (i ? NO_ERROR : DRIV_ERROR);
}

SHORT set_MainTrigger(SHORT onoff)
{
  det_setup->trigson = onoff;
  return(NO_ERROR);
}
  
SHORT set_Temp(SHORT newtemp)
{
  SHORT temp;
  SHORT err = NO_ERROR;

  temp = (newtemp / 2) * 2;
  if (temp != newtemp)
    {
    err = readsys_error = ERROR_ODD_TEMP;
    }
  det_setup->detector_temp = temp;

  temp = (SHORT)get_asic_counter(TEMP_TAG); /* get the on/off bit */
  temp = (temp & 0xff01);

  temp = temp | ((10 - newtemp) &  0x00FE);
  temp |= (det_setup -> cooler_locked >> 1) & 1;

  rep_asic_opbits(TEMP_TAG, temp);
  return(err);
}
  
SHORT set_CoolOnOff(SHORT coolerOn)
{
  SHORT temp;

  if(coolerOn)
     det_setup -> cooler_locked |= 2;     /* set bit 1 */
  else
     det_setup -> cooler_locked &= ~ 2;   /* clear bit 1 */

  temp = (SHORT) get_asic_counter(TEMP_TAG);

  temp &= ~ 1;                                     /* clear the on/off bit */
                                                    /* in det_setup */
  temp |= (det_setup -> cooler_locked >> 1) & 1;
  rep_asic_opbits(TEMP_TAG, temp);
  return NO_ERROR;
}

SHORT set_PixTime(SHORT pixtime_index)
{
   SHORT i;
   det_setup->pix_time_index = pixtime_index;
   i = set_pixtime_status(pixtime_index);
   return (i ? NO_ERROR : DRIV_ERROR);
}
  
SHORT set_ShiftTime(SHORT shifttime_index)
{
   SHORT i;
   if (shifttime_index == 1 && ! InqHasFastShift())
     {
     return(readsys_error = ERROR_NOFASTSHIFT);
     }
   det_setup->shift_time_index = shifttime_index;
   i = set_shifttime_status(shifttime_index);
   return (readsys_error = i ? NO_ERROR : DRIV_ERROR);
}
  
SHORT set_StreakMode(SHORT Mode)
{
  SHORT no_trig_fc_mode = 0, with_trig_fc_mode = 0;

  if (Mode > 2 || Mode < 0)
    return(readsys_error = ERROR_VALUE_INCORRECT);

  det_setup->streakmode = Mode;
  set_DAC_counter(STREAK_MODE, Mode);

  if (Mode)
    {
    if (det_setup->pulser_index > 1) /* set up streak mode for pulser */
      {
      SHORT intensifier_mode;

      get_IntensifierMode(&intensifier_mode);

      if (intensifier_mode)     /* GATED mode */
        no_trig_fc_mode = DAC_STRK_LCI;
      else
        no_trig_fc_mode = DAC_STRK_LCI_CW;
      }
    else if (det_setup->exposure_time >
             shift_line_time() * det_setup->exposed_rows) /* Need ET loop? */
      {
      if (det_setup->min_exposure_time)               /* Pretrigger mode? */
        {
        no_trig_fc_mode = with_trig_fc_mode =         /* yes, set PT ET */
          DAC_STRK_PRETRIG_ET;
        }
      else if (det_setup->shutter_open_sync_index)    /* Yes, need WFS? */
        {
        no_trig_fc_mode = DAC_STRK_NORM_ET_SYNC;      /* Yes, set addresses */
        with_trig_fc_mode = DAC_STRK_OT_ET_SYNC;      /* with and w/o trig  */
        }
      else                                            /* Need ET loop */
        {                                             /* & don't need sync */
        no_trig_fc_mode = DAC_STRK_NORM_ET;           /* set addresses for */
        with_trig_fc_mode = DAC_STRK_OT_ET;           /* with and w/o trig */
        }
      }
    else                                              /* Don't need ET loop */
      {
      if (det_setup->min_exposure_time)               /* Pretrigger mode? */
        {
        no_trig_fc_mode = with_trig_fc_mode =         /* yes, set PT no ET */
          DAC_STRK_PRETRIG_NORM;
        }
      else if (det_setup->shutter_open_sync_index)    /* Need WaitForSync? */
        {
        no_trig_fc_mode = DAC_STRK_NORM_SYNC;         /* Yes, set addresses */
        with_trig_fc_mode = DAC_STRK_OT_SYNC;         /* with and w/o trig  */
        }
      else                                            /* Don't need ET loop */
        {                                             /* & don't need sync  */
        no_trig_fc_mode = DAC_STRK_NORM;              /* set addresses for  */
        with_trig_fc_mode = DAC_STRK_OT;              /* with and w/o trig  */
        }
      }
    set_streakmode_status(no_trig_fc_mode, with_trig_fc_mode);
    }
  scan_setup_changed = TRUE;
  return (NO_ERROR);
}

SHORT set_ExposedRows(SHORT Rows)
{
  det_setup->exposed_rows = Rows;
  return (NO_ERROR);
}

SHORT set_Prescan(SHORT Tracks)
{
  det_setup->min_exposure_time = Tracks; /* use min et for now */
  rep_asic_opbits(PT_TRACKS, Tracks);
  scan_setup_changed = TRUE;
  return (NO_ERROR);
}

SHORT set_ProgTrigger(SHORT location)
{
  TRIGGER_SETUP.StartPixel[0] = location;
  TRIGGER_SETUP.PixelLength[0] = 1;
  return (NO_ERROR);
}

/* Write complete detector info structure to detector */

SHORT WriteSys(void)
{
  holdoff_finish_scan = TRUE;
  wait_off();
  readsys_error |= set_Freq(   60 - (det_setup->line_freq_index * 10));
  readsys_error |= set_Tracks(  det_setup->tracks);
  readsys_error |= set_Points(  det_setup->points);
  readsys_error |= set_ShiftMode(det_setup->shiftmode);
  readsys_error |= set_Temp(    det_setup->detector_temp);
  readsys_error |= set_Preps(   det_setup->prep_frames);
  readsys_error |= set_Scans(   det_setup->scans);
  readsys_error |= set_Mems(    det_setup->memories);
  readsys_error |= set_Ignores( det_setup->ignored_scans);
  readsys_error |= set_PixTime(det_setup->pix_time_index);
  readsys_error |= set_ShiftTime(det_setup->shift_time_index);
  readsys_error |= set_SourceCompMode(det_setup->source_comp_index);
  readsys_error |= set_OutputReg(det_setup->outputReg);
  readsys_error |= set_DeltaX(  POINT_SETUP.DeltaX[get_active_point()]);
  readsys_error |= set_DeltaY(  TRACK_SETUP.DeltaY[get_active_track()]);
  if (det_setup->detector_type_index != RAPDA)
    readsys_error |= set_X0(    POINT_SETUP.X0[get_active_point()]);
  readsys_error |= set_Y0(      TRACK_SETUP.Y0[get_active_track()]);
  readsys_error |= set_SameET(det_setup->same_et);
  readsys_error |= set_StreakMode(det_setup->streakmode);

  get_PulserType(&det_setup->pulser_index);

  if (det_setup->pulser_index > 1)
    {
    set_PulserConfig();
    readsys_error |= set_PulserDelay(det_setup->pulser_delay);
    readsys_error |= set_PulserDelayInc(det_setup->pulser_delay_inc);
    readsys_error |= set_PulserTrigCount(det_setup->pulser_trigger_count);
    readsys_error |= set_PulserScanTimeout(det_setup->pulser_scan_timeout);
    readsys_error |= set_PulserTrigThresh(det_setup->pulser_trig_threshold);
    readsys_error |= set_PulserWidth(det_setup->pulser_width);
    }
  else
    {
    readsys_error |= set_ExposeTime(det_setup->exposure_time); 
    readsys_error |= set_ControlMode(det_setup->control_index);    
    readsys_error |= set_ShutterMode(det_setup->shutter_forced_mode);
    readsys_error |= set_ShutterOpenSync(det_setup->shutter_open_sync_index);
    readsys_error |= set_ShutterCloseSync(det_setup->shutter_close_sync_index);
    }

  holdoff_finish_scan = FALSE;
  wait_on();
  readsys_error |= finish_scan_setup();
  readsys_error |= set_DaProg(det_setup->DA_mode);

  return(readsys_error);
}

PRIVATE void get_ASIC_vectors(SHORT *FC_vec, SHORT *DAC_vec, SHORT *OH_vec)
{
  if (det_setup->shiftmode)   /* if classic mode */
  {
  if (det_setup->pointmode)   /* if random points (image) */
    *FC_vec = IMG_RANDOM;     /* select random image routine */
  else                        /* else uniform points (image) */
    *FC_vec = IMG_CLASSIC;    /* select uniform image routine */
  }
  else                        /* simple mode */
  {
  if (det_setup->trackmode)   /* if random tracks (image) */
    *FC_vec = IMG_RANDOM;     /* select random image routine */
  else                        /* else uniform tracks (image) */
    *FC_vec = IMG_SIMPLE;     /* select uniform image routine */
  }

  *DAC_vec = det_setup->DA_mode; /* no special live mode */
  *DAC_vec += (DA_LIVE);         /* skips init, keep clean & live seqs */

  if (det_setup->pulser_index > 1)
    {
    if (det_setup->pulser_trigsrc_index != 0 && /* gated and not disabled */
        det_setup->pulser_intensifier_mode == 1)
      *OH_vec = OHLCI;
    else
      *OH_vec = OHLCINP;
    }
  else if (det_setup->exposure_time == 0.0F) /* if no expose time */
    *OH_vec = OHNOET;                        /* scan has no et or shutter */
  else if (det_setup->shutter_forced_mode)   /* if no shutter needed */
    *OH_vec = OHNOSHUT;                      /* scan has et, no shutter */
  else
    *OH_vec = OHNORM;                        /* et and shutter */
}

/**************************************************************************/
/*                                                                        */
/*  Start an OMA data acquisition sequence                                */
/*                                                                        */
/**************************************************************************/

SHORT start_OMA_DA(SHORT DoLive)
{
  SHORT FC_vec, DAC_vec, OH_vec;

  get_ASIC_vectors(&FC_vec, &DAC_vec, &OH_vec);

  if (det_setup->detector_type_index == NO_CARD)
    {
    if (InitDetector() || fake_detector)
      {
      return(readsys_error = ERROR_FAKEDETECTOR);
      }
    WriteSys();
    get_ASIC_vectors(&FC_vec, &DAC_vec, &OH_vec);
    }
  if (scan_setup_changed)  /* Now DoLive = 0 or 2 same, 1 or 3 same */
    {
    struct timeb tstruct;
    ULONG millisecs;

    WriteSys();                /* so write new parameters  */
    ftime(&tstruct);           /* then wait 40 ms so shutter will fire 1st time */
    millisecs = tstruct.millitm + tstruct.time * 1000;
    do
      ftime(&tstruct);
    while (((tstruct.millitm + tstruct.time * 1000) - millisecs) < 40);
    }

  if (DoLive >= 2)
    DoLive -= 2;                               /* convert for access4 */

  if (switch_da(DAC_vec, FC_vec, OH_vec, DoLive) == NO_ERROR)
    {
    det_setup->da_active = TRUE;
    return(FALSE);
    }
  else
    {
    return(readsys_error = ERROR_DETECTOR_TIMEOUT);
    }
}

/**************************************************************************/
/*                                                                        */
/* Special case of switch_da - switch into keepclean, but store vectors   */
/* for normal acquisition.                                                */
/*                                                                        */
/**************************************************************************/
SHORT InitStartDetector(void)
{
  SHORT FC_vec, DAC_vec, OH_vec;

  get_ASIC_vectors(&FC_vec, &DAC_vec, &OH_vec);
  return switch_da(DA_CLEAN, FC_vec, OH_vec, 1);
}

/**************************************************************************/
/*                                                                        */
/*  Stop an OMA data acquisition sequence                                 */
/*                                                                        */
/**************************************************************************/
SHORT stop_OMA_DA(SHORT dontforceit)
{
  det_setup->da_active = FALSE;
  if (!fake_detector)
    {
    if (dontforceit)
      return(switch_da(DA_CLEAN, 0, 0, 0));
    else
      return(stop_OMA_DA_in_progress());
    }
  else
    return(NO_ERROR);
}

#ifdef _WINOMA_
SHORT FAR PASCAL _export WriteCurveToMem(void * pData, SHORT len, USHORT curvenum)
#else
SHORT _pascal WriteCurveToMem(void * pData, SHORT len, USHORT curvenum)
#endif
{
   put_OMA4_data((char *)pData, len, curvenum);
   return(NO_ERROR);
}

/****************************************************************/
/*                                                              */
/*  readsys section                                             */
/*                                                              */
/****************************************************************/

#ifdef _WINOMA_
SHORT FAR PASCAL _export GetParam(enum det_command cmd, FLOAT * address)
#else
SHORT _pascal GetParam(enum det_command cmd, FLOAT *address)
#endif
{
  if (cmd > NUM_FLT_CMDS)
    return ERROR_NOSUCH_PARAM;
  else
    {
    if (cmd >= NUM_INT_CMDS)
      return get_flt_detect_param(cmd, address);
    else
      {
      SHORT err, iparam;
      err = get_int_detect_param(cmd, &iparam);
      *address = (float)iparam;
      return(err);
      }
    }
}

#ifdef _WINOMA_
SHORT FAR PASCAL _export GetIntParam(enum det_command cmd, SHORT far * address)
#else
SHORT _pascal GetIntParam(enum det_command cmd, SHORT * address)
#endif
{
  if (cmd > NUM_INT_CMDS)
     return ERROR_NOSUCH_PARAM;
  else
    return get_int_detect_param(cmd, address);
}


SHORT get_HiMem(SHORT * retval)
{
  *retval = get_DAC_counter(J1_COUNTER);
  return(NO_ERROR);
}

SHORT get_Ident(SHORT * retval)
{
  *retval = 1560;
  return(NO_ERROR);
}

SHORT get_Version(FLOAT * retval)
{
  *retval = DriverVersion;
  return(NO_ERROR);
}

SHORT get_DetPort(SHORT * retval)
{
  *retval = det_setup->det_addr;
  return (NO_ERROR);
}

SHORT get_Error(SHORT * err)
{
  *err = readsys_error;
  return(readsys_error = NO_ERROR);
}

/***********************************************/
/*                                             */
/* this one works a little differently         */
/* caller passes curve # in *frame             */
/* This precludes use by GetParam!             */
/* routine calcs frame # and returns in *frame */
/* frame # returned is 0-based!                */
/*                                             */
/***********************************************/

SHORT get_Frame(SHORT * frame)
{
  *frame = *frame / det_setup->tracks;
  return(NO_ERROR);
}

SHORT get_Freq(SHORT * retval)
{
  *retval = det_setup->line_freq_index ? 50 : 60;
  return(NO_ERROR);
}

SHORT get_MinET(FLOAT * retval)
{
  if (det_setup->detector_type_index == RAPDA)
    {
    FLOAT ExpTime = det_setup->exposure_time;
    det_setup->exposure_time = 0.0F;
    SetShortDelay (retval);
    det_setup->exposure_time = ExpTime;
    }
  else
    *retval = (det_setup->pulser_index < 2) ? 0.010F : 5.0e-9F;
  return(NO_ERROR);
}

SHORT get_DaActive(SHORT * retval)
{
  *retval = (get_DAC_counter(DA_DONE) == 0);
  det_setup->da_active = *retval;
  return(NO_ERROR);
}

SHORT get_Bytes(SHORT * retval) /* return bytes per curve */
{
  *retval = (det_setup->data_word_size + 1) * 2;
  *retval *= det_setup->points;
  return(NO_ERROR);
}

SHORT get_PointSize(SHORT * retval)
{
  *retval = det_setup->data_word_size;
  return(NO_ERROR);
}

SHORT get_DetectorType(SHORT * retval)
{
  *retval = det_setup->detector_type_index;
  return(NO_ERROR);
}
  
SHORT get_DaProg(SHORT * prog)
{
  *prog = det_setup->DA_mode;
  return(NO_ERROR);
}

SHORT get_DaMaxProg(SHORT * prog)
{
  *prog = ((SHORT)NumDACBlks-DA_LIVE)-1;
  return(NO_ERROR);
}

PRIVATE SHORT get_DaName(CHAR * Name, SHORT prog)
{
  if (prog <= (SHORT)((NumDACBlks - DA_LIVE) - 1))
    {
    if((!fake_detector) && (BlkNameBuf != NULL) && (Name != NULL))
      strcpy(Name, (char *)(&BlkNameBuf[prog+DA_LIVE]));
    else if (Name != NULL)
      *Name = '\0';
    return(NO_ERROR);
    }
  return(DRIV_ERROR);
}

PRIVATE SHORT set_DaName(CHAR * Name, SHORT prog)
{
  if (prog <= (SHORT)((NumDACBlks - DA_LIVE) - 1))
    {
    if((!fake_detector) && (BlkNameBuf != NULL) && (Name != NULL))
      strcpy((char *)(&BlkNameBuf[prog+DA_LIVE]), Name);
    else if (Name != NULL)
      *Name = '\0';
    return(NO_ERROR);
    }
  return(DRIV_ERROR);
}

#ifdef _WINOMA
SHORT PASCAL _export GetOMAString(enum OMA_strings Cmd, SHORT Param, char far * lpszRStr)
#else
SHORT _pascal GetOMAString(enum OMA_strings Cmd, SHORT Param, char * lpszRStr)
#endif
{
  switch (Cmd)
    {
    case DS_DANAME:
      return get_DaName(lpszRStr, Param);
    default:
      return(readsys_error = DRIV_ERROR);
    }
}

#ifdef _WINOMA
SHORT PASCAL _export SetOMAString(enum OMA_strings Cmd, SHORT Param, char far * lpszRStr)
#else
SHORT _pascal SetOMAString(enum OMA_strings Cmd, SHORT Param, char * lpszRStr)
#endif
{
  switch (Cmd)
    {
    case DS_DANAME:
      return get_DaName(lpszRStr, Param);
    default:
      return(readsys_error = DRIV_ERROR);
    }
}

SHORT get_nothing(SHORT * retval)
{
  *retval = 0;
  return(NO_ERROR);
}

SHORT get_source_comp_point(LONG * SourceComp, SHORT frame)
{
  *SourceComp = access_source_comp(frame);
  return(NO_ERROR);
}

SHORT get_SourceComp(FLOAT * retval)
{
  LONG scmp_val;

  if (det_setup->detector_type_index == RAPDA)
    scmp_val = 0L;
  else
    get_source_comp_point(&scmp_val, 0);
  *retval = (FLOAT)scmp_val;
  return(NO_ERROR);
}

SHORT get_SameET(SHORT * retval)
{
  *retval = det_setup->same_et;
  return(NO_ERROR);
}

SHORT get_OMA_memory_size(ULONG * memsize)
{
  *memsize = det_setup->memory_size;
  return(NO_ERROR);
}

SHORT get_DeltaX(SHORT * retval)
{
  *retval = POINT_SETUP.DeltaX[(det_setup->pointmode) ?
                               det_setup->Current_Point : 0];
  return(NO_ERROR);
}

SHORT get_RegSize(SHORT * retval)
{
  *retval = RAPDA_SETUP.RapdaReg[max (0, det_setup->Current_Point-1)].Number;
  return(NO_ERROR);
}

SHORT get_DeltaY(SHORT * retval)
{
  if (det_setup->trackmode)
     *retval = TRACK_SETUP.DeltaY[det_setup->Current_Track];
  else
     *retval = TRACK_SETUP.DeltaY[0];
  return(NO_ERROR);
}

SHORT get_Temp(SHORT * retval)
{
  *retval = det_setup->detector_temp;
  return(NO_ERROR);
}

SHORT get_ExposeTime(FLOAT * retval)
{
  if ((det_setup->pulser_index < 2) ||
      (det_setup->detector_type_index == RAPDA))
    *retval = det_setup->exposure_time;
  else
    {
    ULONG temp = (ULONG)(det_setup->pulser_width * 1e10F);
    USHORT temp2 = (USHORT)(temp % 10);

    if (temp2 > 4) temp += (10 - temp2); else temp -= temp2;

    *retval = (FLOAT)(temp / 1e10F) * det_setup->pulser_trigger_count;
    }
  return(NO_ERROR);
}

SHORT get_RegionET(FLOAT * retval)
{
  char n = 0;
  FLOAT temp;
  SHORT i, pt = max (0, det_setup->Current_Point-1);
  get_ExposeTime(&temp);
  if (det_setup->detector_type_index == RAPDA)
    {
    for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
      n = (char) max (RAPDA_SETUP.RapdaReg[i].n, n);
    }
  if (det_setup->detector_type_index == RAPDA)
    temp /= (FLOAT)(1 << (n - RAPDA_SETUP.RapdaReg[pt].n));
  *retval = temp;
  return(NO_ERROR);
}

const FLOAT pixtime_matrix[3][3][3] ={  /* OK, a struct would be better */
                                {
                             /*  no cooler cryo cooler peltier cooler */
 /* Thomson 512^2  normal */     { 18e-6F,    18e-6F,    18e-6F,},
 /* & 256X1024     fast   */     { 10e-6F,    10e-6F,    10e-6F,},
                /* slow   */     { 32e-6F,    32e-6F,    32e-6F,},
                                },
                                {
                /* normal */     { 20e-6F,    20e-6F,    20e-6F,},
   /*eev           fast   */     { 10e-6F,    10e-6F,    10e-6F,},
                /* slow   */     { 28e-6F,    30e-6F,    30e-6F,},
                                },
                                {
                /* normal */     { 18e-6F,    18e-6F,    18e-6F,},
   /*splitmode     fast   */     { 10e-6F,    10e-6F,    10e-6F,},
                /* slow   */     { 32e-6F,    32e-6F,    32e-6F,},
                                }
                               };

/* indices for addressing pixel time matrix (sorry, no Lisp!) */

#define dt_regular 0 
#define dt_eev     1
#define dt_splmode 2

FLOAT calc_rpix_time(void) /* return time to read one pixel */
{
  SHORT cooltype = det_setup->cooler_type_index;
  SHORT pixtime = det_setup->pix_time_index;
  FLOAT rpix_time = 0.0F;

  switch (det_setup->detector_type_index)
    {
    case TSM_CCD:
      rpix_time = pixtime_matrix[dt_splmode][pixtime][cooltype];
      if (det_setup->outputReg == 2) /* if Dual mode */
        rpix_time /= 2.0F;
    break;
    case EEV_CCD:
      if (POINT_SETUP.DeltaX[0] > 1)
        rpix_time = 25.5e-6F; /* timing adjusted for EEV binning */
      else                    /* since it lacks an output node */
        rpix_time = pixtime_matrix[dt_eev][pixtime][cooltype];
    break;
    case RAPDA:
        rpix_time = A_D_TIME;
    break;
    default:
      rpix_time = pixtime_matrix[dt_regular][pixtime][cooltype];
    break;
    }
  return rpix_time;
}

PRIVATE FLOAT tp_time(void) /* return time to trash a pixel */
{
  switch(det_setup->detector_type_index)
    {
    case EEV_CCD:      /* eev */
      return(4e-6F);   
    case TSP_CCD:      /* splitmode */
      return(6e-6F);
    case TSC_CCD:      /* "regular" types */
    case TSM_CCD:
    case RAPDA: 
    case TS_1KSQ:
    default:
      return(3e-6F);
    }
}

PRIVATE FLOAT calc_tpix_time(void)  /* return time to trash pix in one trak*/
{
  SHORT lgroups, tgroups;

  if (det_setup->shiftmode == 0) /* if simple mode */
    {
    lgroups = calc_lead_columns();
    tgroups = calc_trail_columns();
    }
  /* should also account for points in serial groups, i.e. DeltaX */
  /* but action of binning requires similar time to trashing      */
  else
    {
    tgroups = det_setup->ActiveX - det_setup->tracks;
    lgroups = 0;
    }

  /* Timing calculation changed 2/28/94 to account for fix to 512^2    */
  /* detectors - leading Trash Pixel instructions replaced with        */
  /* Fast Normal pixel instructions.  Assuming 10us/fast normal pixel. */

  if (det_setup->detector_type_index == TSC_CCD)
    return (FLOAT)((FLOAT)tgroups * tp_time() + (FLOAT)lgroups * 10e-6F);
  else
    return (FLOAT)((FLOAT)(tgroups + lgroups) * tp_time());
}

PRIVATE ULONG calc_antibloom(SHORT percent, USHORT lines)
{
  ULONG llines;
  USHORT inloop, outloop;

  llines = calc_antibloom_counts(percent, lines, &inloop, &outloop);
  llines += outloop;
  llines -= 2;
  return(llines);
}

/* macros to make get_FrameTime more readable */

#define TOTAL_LINES (det_setup->ActiveY +\
                     det_setup->DumYLead +\
                     det_setup->DumYTrail)

#define TOTAL_PIXELS (det_setup->ActiveX +\
                      det_setup->DumXLead +\
                      det_setup->DumXTrail)

SHORT get_FrameTime(FLOAT * retval)
{
  SHORT i;
  USHORT temp;
  LONG skiplines;
  BOOLEAN dual = (det_setup->outputReg == 2); /* TRUE if Dual mode */
  SHORT percent = det_setup->anti_bloom_percent;

  get_ExposeTime(retval);

  if (det_setup->detector_type_index == RAPDA)
    return(NO_ERROR);

  /* calc time to shift all lines in array - twice as fast if Dual mode */
  *retval += TOTAL_LINES * (shift_line_time() / (FLOAT)(dual + 1));

  /* add time to digitize all points read */
  *retval +=
     (FLOAT)det_setup->tracks *
     (FLOAT)det_setup->points *
     calc_rpix_time();
        
  /* calc the number of lines not read */
  skiplines = calc_antibloom(percent, (USHORT)calc_lead_lines());
  skiplines += calc_antibloom(percent,(USHORT)calc_trail_lines());

  if (det_setup->shiftmode == 0)   /* if simple mode */
    {
    /* add time to skip pixels not read in each track which is read */
    *retval += calc_tpix_time() * det_setup->tracks / (FLOAT)(dual+1);

    /* Random tracks, skiplines includes skipped lines between each track */
    if (det_setup->trackmode == 1)
      {
      for (i = 1;i < det_setup->tracks+1;i++)
        {
        temp = (TRACK_SETUP.Y0[i+1] - TRACK_SETUP.Y0[i]);
        skiplines += calc_antibloom(percent, (temp - TRACK_SETUP.DeltaY[i]));
        }
      }
    }
  else /* classic mode */
    {
    /* add time to skip pixels not read in each slice which is read */
    *retval += (calc_tpix_time() * det_setup->points) / (FLOAT)(dual+1);

    /* Random points, skiplines includes skipped lines between each slice */
    if (det_setup->pointmode == 1)
      {
      for (i = 1;i < det_setup->points + 1; i++)
        {
        temp = (POINT_SETUP.X0[i+1] - POINT_SETUP.X0[i]);
        skiplines += calc_antibloom(percent, (temp - POINT_SETUP.DeltaX[i]));
        }
      }
    }
   if (dual) /* if Dual mode , lines skipped two at a time */
    skiplines /= 2;

  /* add time to trash pixels in unread (skipped) lines */
  *retval += skiplines * tp_time() * (LONG)(TOTAL_PIXELS);

  *retval += 600e-6F;  /* .6ms for mucking about with PIA & SRC COMP */

  if ((!det_setup->shutter_forced_mode) && (det_setup->pulser_index < 2) &&
       (det_setup->exposure_time > 1e-4F))
    {
    if (det_setup->detector_type_index == EEV_CCD || /* bigger chips have */
        det_setup->detector_type_index == TS_1KSQ)   /* slower shutters */
        *retval += 35e-3F;  /* shutter travel time */
      else
        *retval += 15e-3F;  /* shutter travel time */
    }
  else
    *retval += 120e-6F; /* default 120 usec for no-shut, no-et */

  return(NO_ERROR);
}

SHORT get_Preps(SHORT * retval)
{
  *retval = det_setup->prep_frames;
  return(NO_ERROR);
}

BOOLEAN IsActive(void)
{
  return (!get_DAC_counter(DA_DONE) || ((SHORT)get_DAC_counter(DA_SEQ) > 2));
}

SHORT get_Scans(SHORT * retval)
{
  if (IsActive())
    *retval = get_DAC_counter(I1_COUNTER);
  else
    *retval = det_setup->scans;
  return(NO_ERROR);
}

SHORT get_Lastscan(SHORT * retval)
{
  *retval = get_DAC_counter(L1_COUNTER);
  return(NO_ERROR);
}

SHORT get_Mem(SHORT * retval)
{
  if (IsActive())
    *retval = get_DAC_counter(J1_COUNTER);
  else
    *retval = det_setup->StartMemory + 1;
  return(NO_ERROR);
}

SHORT get_Mems(SHORT * retval)
{
  *retval = det_setup->memories;
  return(NO_ERROR);
}

SHORT get_PIA_In(SHORT * retval)
{
  *retval = (get_DAC_counter(PIA_INBITS) & 0x00FF);
  return(NO_ERROR);
}

SHORT get_PIA_Out(SHORT * retval)
{
  *retval = (get_DAC_counter(PIA_OUTBITS) & 0x00FF);
  return(NO_ERROR);
}

SHORT get_Ignores(SHORT * retval)
{
  if (IsActive())
    *retval = get_DAC_counter(K1_COUNTER);
  else
    *retval = det_setup->ignored_scans;
  return(NO_ERROR);
}

SHORT get_PulserDelay(FLOAT * retval)
{
  *retval = det_setup->pulser_delay;
  return(NO_ERROR);
}

SHORT get_PulserType(SHORT * retval)
{
  USHORT shut_id;

  if (fake_detector)
    *retval = det_setup->pulser_index;
  else
    {
    shut_id = get_DAC_counter(SHUTID);

    if ((shut_id & 0xF) == 0x0E)
      shut_id = 2 + ((shut_id >> 6) & 0x03); /* id intensifier type */
    else if (shut_id == 0xFF) /* this happens if *no* board present */
      shut_id = 0;

    det_setup->pulser_index = *retval = shut_id;
    }
  return(NO_ERROR);
}

SHORT get_PulserWidth(FLOAT * retval)
{
  *retval = det_setup->pulser_width;
  return(NO_ERROR);
}

SHORT get_PulserTrigSrc(SHORT * retval)
{
  *retval = det_setup->pulser_trigsrc_index;
  return(NO_ERROR);
}

SHORT get_IntensifierMode(SHORT * retval) /* 0 = CW, 1 = GATE */
{
  USHORT shut_id;

  if (!fake_detector)
    shut_id = get_DAC_counter(SHUTID);  /* actually gets it from detector */
  else
    shut_id = det_setup->pulser_intensifier_mode << 4;

  *retval = det_setup->pulser_intensifier_mode = (shut_id >> 4) & 0x01;

  return(NO_ERROR);
}

SHORT get_PulserDelayInc(FLOAT * retval)
{
  *retval = det_setup->pulser_delay_inc;
  return(NO_ERROR);
}

SHORT get_PulserDelayRange(FLOAT * retval)
{
  *retval = det_setup->pulser_delay_inc * det_setup->memories * det_setup->scans;
  return(NO_ERROR);
}

SHORT get_PulserTrigCount(SHORT * retval)
{
  *retval = det_setup->pulser_trigger_count;
  return(NO_ERROR);
}

SHORT get_PulserScanTimeout(FLOAT * retval)
{
  *retval = det_setup->pulser_scan_timeout;
  return(NO_ERROR);
}

SHORT get_PulserAudio(SHORT * retval)
{
  *retval = det_setup->pulser_audio_index;
  return(NO_ERROR);
}

SHORT get_PulserTrigThresh(SHORT * retval)
{
  *retval = det_setup->pulser_trig_threshold;
  return(NO_ERROR);
}

SHORT get_TrigThreshVolts(FLOAT * retval)
{
  *retval = calc_volts((USHORT)det_setup->pulser_trig_threshold);
  return(NO_ERROR);
}

SHORT get_SourceCompMode(SHORT * retval)
{
  *retval = det_setup->source_comp_index;
  return(NO_ERROR);
}

SHORT get_ShiftMode(SHORT * retval)
{
  *retval = det_setup->shiftmode;
  return(NO_ERROR);
}

SHORT get_ShutterMode(SHORT * retval)
{
  *retval = det_setup->shutter_forced_mode;
  return(NO_ERROR);
}

SHORT get_PointMode(SHORT * retval)
{
  *retval = det_setup->pointmode;
  return(NO_ERROR);
}

SHORT get_Point(SHORT * retval)
{
  *retval = det_setup->Current_Point;
  return(NO_ERROR);
}

SHORT get_Points(SHORT * retval)
{
  if (det_setup->detector_type_index == RAPDA)
    {
    SHORT i, j;
    for (i = j = 0; i < RAPDA_SETUP.NumRegs; i++)
     j += RAPDA_SETUP.RapdaReg[i].Number;
    det_setup->points = j;
    }
  *retval = det_setup->points;
  return(NO_ERROR);
}

SHORT get_Regions(SHORT * retval)
{
  *retval = (SHORT)RAPDA_SETUP.NumRegs;
  return(NO_ERROR);
}

SHORT get_StartMode(SHORT * retval)
{
  *retval = det_setup->external_start_index;
  return(NO_ERROR);
}

SHORT get_ControlMode(SHORT * retval)
{
  *retval = det_setup->control_index;
  return(NO_ERROR);
}

SHORT get_MainTrigger(SHORT * retval)

{
  *retval = (det_setup->trigson !=0);
  return(NO_ERROR);
}

SHORT get_MaxET(FLOAT * retval)
{
  *retval = max_et;
  return(NO_ERROR);
}

SHORT get_MaxMem(SHORT * retval)
{
  *retval = det_setup->max_memory;
  return(NO_ERROR);
}

SHORT get_CurrentTrack(SHORT * retval)
{
  if (IsActive())
    *retval = get_DAC_counter(T1_COUNTER);
  else
    *retval = det_setup->Current_Track;
  return(NO_ERROR);
}

SHORT get_CurrentPoint(SHORT * retval)
{
  *retval = det_setup->Current_Point;
  return(NO_ERROR);
}

SHORT get_Tracks(SHORT * retval)
{
  if (det_setup->detector_type_index == RAPDA)
    det_setup->tracks = 1;
  *retval = det_setup->tracks;
  return(NO_ERROR);
}

SHORT get_TrackMode(SHORT * retval)
{
  *retval = det_setup->trackmode;
  return(NO_ERROR);
}

SHORT get_ShutterCloseSync(SHORT * retval)
{
  *retval = det_setup->shutter_close_sync_index;
  return(NO_ERROR);
}

SHORT get_ShutterOpenSync(SHORT * retval)
{
  *retval = det_setup->shutter_open_sync_index;
  return(NO_ERROR);
}

SHORT get_X0(SHORT * retval)
{
  SHORT pt = max (0, det_setup->Current_Point-1);

  if (det_setup->detector_type_index == RAPDA)
    *retval = RAPDA_SETUP.RapdaReg[pt].StartPixel;
  else
     *retval =  POINT_SETUP.X0[(det_setup->pointmode) ? pt+1 : 0];
  return(NO_ERROR);
}

SHORT get_Y0(SHORT * retval)
{
  if (det_setup->trackmode)
     *retval = TRACK_SETUP.Y0[det_setup->Current_Track];
  else
     *retval = TRACK_SETUP.Y0[0];
  return(NO_ERROR);
}

SHORT get_CoolOnOff(SHORT * retval)
{
   *retval = det_setup -> cooler_locked; /* bit 1 is cooler on/off */
   *retval &= 2;
   *retval >>= 1;
   return(NO_ERROR);
}

SHORT get_CoolLocked(SHORT * retval)
{
  USHORT cool_stat;
  
  if (fake_detector)
    *retval = FAKE;
  else
    {
    cool_stat = get_DAC_counter(COOL_STATUS);

    if(cool_stat & 0x02)
      det_setup -> cooler_locked |= 1; /* set LSB */
    else
      det_setup -> cooler_locked &= ~ 1; /* clear LSB */

    *retval = det_setup -> cooler_locked & 1; /* bit 0 is cooler locked */
    }

  return(NO_ERROR);
}

SHORT get_CoolStatus(SHORT * retval)
{
  USHORT cool_stat;
  USHORT cooler_type_ID;
  enum CoolerErrorStatus ErrorStat;

  if(fake_detector)
    ErrorStat = COOL_OK;
  else
    {
    cool_stat = get_DAC_counter(COOL_STATUS);
    
    if (cool_stat == 0xFF) cool_stat = 0;
  
    cooler_type_ID = (cool_stat >> 5) & 3;  /*  bits 5 and 6 */

    if(cool_stat & 0x02)
      det_setup -> cooler_locked |= 1; /* set LSB */
    else
      det_setup -> cooler_locked &= ~ 1; /* clear LSB */
  
    /* In case of status UNKNOWN, Call John Kaznicki at (609) 530-1000 x8224 */
  
    switch(cooler_type_ID)
      {
      case 0 :
        ErrorStat = UNKNOWN_COOLER;
      break;
      case 1 : /*  cryo type cooler   */
        if((cool_stat & 0x10) == 0)
          ErrorStat = DEWAR_EMPTY;
        else if((cool_stat & 0x8) == 0)
          ErrorStat = TOO_HIGH;
        else
          ErrorStat = NO_ERROR;
      break;
      case 2 : /*  TE type cooler */
        if((cool_stat & 0x4) == 0)
          ErrorStat = HEAT_EXCHANGE;
        else if ((cool_stat & 0x1) == 0)
          ErrorStat = DIFF_EXCEEDED;
        else
          ErrorStat = NO_ERROR;
      break;
      case 3 :                         
        ErrorStat = UNKNOWN_COOLER;
      break;
      }
    }
  *retval = ErrorStat;
  return(NO_ERROR);
}

SHORT get_CoolerType(SHORT * retval)
{
  USHORT cooler_type_ID;

  cooler_type_ID = get_DAC_counter(COOL_STATUS);

  if (cooler_type_ID == 0xFF) 
    cooler_type_ID = COOLER_UNKNOWN;
  
  cooler_type_ID = (cooler_type_ID >> 5) & 3;        /* get bits 5 and 6 */

  if (cooler_type_ID == 5) 
    cooler_type_ID = COOLER_CRYO;       /* type 1= cryo */
  else if (cooler_type_ID == 6) 
    cooler_type_ID = COOLER_TE;        /* type 2= peltier */

  det_setup->cooler_type_index = *retval = cooler_type_ID;

  return(NO_ERROR);
}

SHORT get_StreakTime(FLOAT *retval)
{
  if (det_setup->streakmode)
    {
    FLOAT shift_time = shift_line_time();

    *retval = det_setup->ActiveY * shift_time;
    if(det_setup->exposure_time > shift_time)
      *retval += det_setup->tracks * (det_setup->exposure_time -
                                      shift_time);
    }
  else
    *retval = 0.0F;
  return(NO_ERROR);
}

SHORT get_PrescanTime(FLOAT *retval)
{
  if (det_setup->streakmode)
    {
    FLOAT shift_time = shift_line_time();

    *retval = TRACK_SETUP.DeltaY[0] * shift_time;
    if(det_setup->exposure_time > shift_time)
      *retval += det_setup->exposure_time - shift_time;
    *retval *= (FLOAT)det_setup->min_exposure_time; /* use min et for now */
    }
  else
    *retval = 0.0F;
  return(NO_ERROR);
}

SHORT get_PixTime(SHORT * retval)
{
  *retval = det_setup->pix_time_index;
  return(NO_ERROR);
}

SHORT get_ShiftTime(SHORT * retval)
{
  *retval = det_setup->shift_time_index;
  return(NO_ERROR);
}

SHORT get_StreakMode(SHORT * retval)
{
  *retval = det_setup->streakmode;
  return(NO_ERROR);
}

SHORT get_ExposedRows(SHORT * Rows)
{
  *Rows = det_setup->exposed_rows;
  return (NO_ERROR);
}

SHORT get_Prescan(SHORT * retval)
{
  *retval = det_setup->min_exposure_time; /* use min et for now */
  return (NO_ERROR);
}

SHORT get_AntiBloom(SHORT * retval)
{
   *retval = det_setup -> anti_bloom_percent;
   return(NO_ERROR);
}

SHORT get_OutputReg(SHORT * retval)
{
   *retval = det_setup -> outputReg;
   return(NO_ERROR);
}

SHORT get_ActiveX(SHORT * retval)
{
  if (det_setup->detector_type_index == RAPDA)
    *retval =
    det_setup -> ActiveX = (SHORT)MAX_PXL;
  else
    *retval = det_setup -> ActiveX;
  return(NO_ERROR);
}

SHORT get_ActiveY(SHORT * retval)
{
  if (det_setup->detector_type_index == RAPDA)
    det_setup -> ActiveY = 1;
  *retval = det_setup -> ActiveY;
  return(NO_ERROR);
}

SHORT get_RealDetector(SHORT * retval)
{
  *retval = (fake_detector == FALSE);
  return(NO_ERROR);
}

SHORT CheckRegionOverlap(void) /* helper for sort_regions */
{
  char n = MAX_N;
  struct RegionType *iPtr, *jPtr;
  SHORT i, j, local_error = NO_ERROR;
                              /* Remove zeroed regions. */
  for (i = 0; i < RAPDA_SETUP.NumRegs;)
    {
    iPtr = &RAPDA_SETUP.RapdaReg[i];  /* Pointer to region in question. */
                                      /* Check if "good" region. */
    if (iPtr->StartPixel && iPtr->Number)
      {
      i++;                            /* Ready for next region. */
      if (n > iPtr->n)                /* Find min value of n. */
        n = iPtr->n;
      }
    else                              /* Empty region, remove it. */
      {
      RAPDA_SETUP.NumRegs--;          /* If so, delete region. */
      memmove(iPtr, iPtr + 1,
            (RAPDA_SETUP.NumRegs - i)*sizeof(struct RegionType));
      memset ((void*)&RAPDA_SETUP.RapdaReg[RAPDA_SETUP.NumRegs],0,
                sizeof(struct RegionType));
      }
    }
  if (n)                              /* Must be at least 1 BaseET region. */
    for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
      RAPDA_SETUP.RapdaReg[i].n -= n;

  for (i = 0; i < RAPDA_SETUP.NumRegs - 1;)
    {
    iPtr = &RAPDA_SETUP.RapdaReg[i++];  /* Pointer to region in question. */
    for (j = i; j < RAPDA_SETUP.NumRegs; j++)
      {
      jPtr = &RAPDA_SETUP.RapdaReg[j];
      if ((iPtr->n < jPtr->n) ||      /* Order the regions. */
          ((iPtr->n == jPtr->n) && (iPtr->StartPixel > jPtr->StartPixel)))
        {
        memcpy ((void*)&ThisRegion,(void*)iPtr, sizeof (struct RegionType));  
        memcpy ((void*)iPtr, (void*)jPtr, sizeof (struct RegionType));  
        memcpy ((void*)jPtr,(void*)&ThisRegion, sizeof (struct RegionType));
        }                             /* Check the error overlap. */
      if (((iPtr->StartPixel < jPtr->StartPixel) &&
           ((iPtr->StartPixel + iPtr->Number) > jPtr->StartPixel)) ||
           ((iPtr->StartPixel >= jPtr->StartPixel) &&
            (iPtr->StartPixel < (jPtr->StartPixel + jPtr->Number))))
        {  
        local_error = readsys_error = OVERLAP_ERR;
        break;
        }
      }
    }
  if (!local_error)              /* If ok, figure the data offset. */
    {
    SHORT TotalPoints = 0;
    for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
      {
      iPtr = &RAPDA_SETUP.RapdaReg[i];  /* Pointer to region in question. */
      TotalPoints += iPtr->Number;
      iPtr->DataOffset = 0;             /* Start with clean slate. */
      for (j = 0; j < RAPDA_SETUP.NumRegs; j++)
        if (iPtr->StartPixel > RAPDA_SETUP.RapdaReg[j].StartPixel)
          iPtr->DataOffset += RAPDA_SETUP.RapdaReg[j].Number;
      set_Points(TotalPoints);
      }
    set_ExposeTime(det_setup->exposure_time);
    }
  return (local_error);
}

SHORT sort_regions(SHORT * retval)
{
  SHORT err = CheckRegionOverlap();

  *retval = RAPDA_SETUP.NumRegs;
  return (err);
}

   /* Given a data point, returns the "pixel" corresponding to the
    * point.  If illegal data point, readsys_error is set;
    * Data points and pixels are 0 based.
    */
FLOAT RapdaDataPtToPixel(SHORT DataPt)
{
  SHORT i;
  
  if (RAPDA_SETUP.NumRegs)
    for (i = 0; i < RAPDA_SETUP.NumRegs; i++)
      {
      struct RegionType *Ptr = &RAPDA_SETUP.RapdaReg[i];
      if ((DataPt >= Ptr->DataOffset) &&
          (DataPt < (Ptr->DataOffset + Ptr->Number)))
        {
        return ((FLOAT)(Ptr->StartPixel + DataPt - Ptr->DataOffset));
        }
      }
  readsys_error = OUT_OF_ARRAY_ERR;
  return ((FLOAT)DataPt);
}

#ifdef _WINOMA_
SHORT FAR PASCAL _export ReadCurveFromMem(void * pData, SHORT len, USHORT curvenum)
#else
SHORT _pascal ReadCurveFromMem(void * pData, SHORT len, USHORT curvenum)
#endif
{
  get_OMA4_data((char *)pData, len, curvenum);
  return(NO_ERROR);
}

#ifdef _PROTECTED_MODE
 #ifdef _WINOMA_
void __huge * PASCAL GetDataPointer(void)
{
  return get_data_address();
}

 #else
void __far * _pascal GetDataPointer(void)
{
  return (get_data_address());
}
 #endif
#endif

/*********************************************************************/
/* Try to load monitor, ASIC, and DAC code onto OMA4 board from disk.*/
/* If the load is successful, then the shared memory area is erased. */
/*********************************************************************/
BOOLEAN install_monitor(void)            /* return non zero if error */
{
  SHORT len, status = FALSE;
  len = strlen(monitor_string)+1;

  if (!load_flags[detector_index])
    {
    load_flags[detector_index] = TRUE;

    status = download_monitor_file(monitor_fname);
    if (status)
      return status;

    status = download_object_file(ascfile_fname);
    if (status)
      return status;

    status = download_object_file(dacfile_fname);
    if (status)
      return status;

    erase_comm_area();
    }

  get_monitor_string(compare_buffer, len);
  if (memicmp(monitor_string, compare_buffer, 15))
    return TRUE;                /* monitor incompatible */
  else
    det_setup->Version = (FLOAT)atof(&(compare_buffer[21]));

  return FALSE;
}

PRIVATE SHORT load_detector_code(USHORT id_code)
{
  char * basename;
  SHORT err = 0;

  switch (id_code)
    {
    case TSC_CCD:
      basename = "mmscans";
    break;
    case TSM_CCD:
      basename = "smscans";
    break;
    case EEV_CCD:
      basename = "eevscans";
    break;
    case RAPDA:
      basename = "pdascans";
    break;
    case TSP_CCD:
      basename = "specscan";
    break;
    case TS_1KSQ:
      basename = "1ksqscan";
    break;
    default:
      err = TRUE;
    break;
    }
  if (!err)
    {
    sprintf(dacfile_fname, "%s.dbj", basename);
    sprintf(ascfile_fname, "%s.abj", basename);

    forget_previous_programs();
    if ((download_object_file(ascfile_fname))  ||
        (download_object_file(dacfile_fname)))
      err = TRUE;  /* couldn't load new scans files */
    }
  return (err);
}

/*************************************************************************/
/* Connect to a detector by checking for the controller board, detector  */
/* id, loading code onto the board, and intializing local control data   */
/*************************************************************************/
SHORT init_detector(USHORT port_address) /* return non-0 if error */
{
  ULONG data_size;
  SHORT error = 0;

  det_setup->Current_Point = 0;
  det_setup->Current_Track = 0;
  det_setup->det_addr = port_address;

  if (access_init_detector(port_address, det_setup->memory_size))
    readsys_error = error = ERROR_FAKEDETECTOR;
  else
    {
    if(install_monitor()) /* try for monitor load */
      readsys_error = error = ERROR_MONITOR_LOAD;
    else if (det_setup->Version < RequiredMonitorVersion)
      readsys_error = error = ERROR_MONITOR_VER;
    else
      {
      USHORT dac_id, asic_id;
#ifndef _PROTECTED_MODE
      init_local_data();
#endif
      id_detector();
      dac_id = det_setup->detector_type_index;
      asic_id = get_asic_counter(DET_ID);
      asic_id = convert_code_to_index(asic_id);
      if (dac_id != asic_id)
        {
        if (load_detector_code(dac_id))
          readsys_error = error = ERROR_SCAN_MISMATCH;
        else
          if (access_init_detector(port_address, det_setup->memory_size))
            readsys_error = error = ERROR_FAKEDETECTOR;
        }
      if (!error)
        {
        wait_off();
        if (dac_id != TSM_CCD)
          if (det_setup->outputReg == 2)
            det_setup->outputReg = 1;
        scan_counter_table();            /* get ASIC code settings*/
        update_from_shared_memory();     /* get shared driver info*/
        wait_on();
        data_size = (ULONG)det_setup->points * (ULONG)det_setup->tracks * 4L;
        set_max_mem(data_size);
        if (det_setup->pulser_index > 1) /* set ET limits for pulser */
          {
          min_et = five_ns;          /* min pulse width */
          max_et =
            (FLOAT)(0.01F * 65535.0F); /* 10ms * max pulse count */
          }
        else                    /* set ET limits without pulser */
          {
          min_et = 0.00036654F; /* 3us NOP plus some overhead */
          max_et = 51538.0F;    /* set by loop counter limits */
          }
        if (POINT_SETUP.X0[0] <= 0)
          POINT_SETUP.X0[0] = 1;
        if (POINT_SETUP.DeltaX[0] <= 0)
          POINT_SETUP.DeltaX[0] = 1;
        if (TRACK_SETUP.Y0[0] <= 0)
          TRACK_SETUP.Y0[0] = 1;
        if (TRACK_SETUP.DeltaY[0] <= 0)
          TRACK_SETUP.DeltaY[0] = 1;
        if (det_setup->points <= 0)
          det_setup->points = det_setup->ActiveX;
        if (det_setup->tracks <= 0)
          {
          det_setup->tracks = 1;
          det_setup->exposure_time = 0.100F;
          }
        if (det_setup->DA_mode <= 0)
          det_setup->DA_mode = 1;
        if (det_setup->scans <= 0)
          det_setup->scans = 1;
        if (det_setup->memories <= 0)
          det_setup->memories = 1;

        WriteSys();
        InitStartDetector();
        }
      }
    }
  return(error);
}

#ifdef _WINOMA_
SHORT FAR PASCAL _export InitDetector(void)
#else
SHORT FAR _pascal InitDetector(void)
#endif
{
  return init_detector(det_setup->det_addr);
}

#ifdef _WINOMA_
SHORT PASCAL _export setup_detector_interface(USHORT port_addr, ULONG memaddr, ULONG memsize)
#else
SHORT _pascal setup_detector_interface(USHORT port_addr, ULONG memaddr, ULONG memsize)
#endif
{
  if (memaddr) det_setup->memory_base  = memaddr;
  if (memsize) det_setup->memory_size  = memsize;

  detector_index = 0;
  access_startup_detector(det_setup->memory_base, det_setup->memory_size);
  return(init_detector(port_addr));
}

#ifdef _WINOMA_
void FAR _pascal _export ShutdownDetector(void)
{
  access_shutdown_detector();
  /* stop_OMA_DA_in_progress(); */
  /* set_OMA_reset(); */
}
#else
void _pascal shutdown_detector_interface(void)
{
  /* access_shutdown_detector(); */
  /* stop_OMA_DA_in_progress(); */
  /* set_OMA_reset(); */
}
#endif

