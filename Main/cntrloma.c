/***************************************************************************/
/*                                                                         */
/*   cntrloma.c  -  written September 1990 by Morris Maynard               */
/*           (Copyright(C) 1990 EG&G Princeton Applied Research)           */
/*                                                                         */
/*   Functions for low level control of OMA4 detector hardware -           */
/*   setting and getting detector settings                                 */
/*                                                                         */
/*  Copyright (c) 1990,  EG&G Instruments Inc.                             */
/*                                                                         */
/***************************************************************************/

#ifdef _WINOMA_
#include <windows.h>
#endif
#include <dos.h>
#include <stdio.h>
#include <stdlib.h> /* strtoul */
#include <stddef.h> /* offsetof */
#include <string.h>
#include <conio.h>
#include <process.h>

#ifndef __TURBOC__
#include <malloc.h>
#else
#include <mem.h>
#define TURBOC
#endif

#include <sys\types.h>
#include <sys\timeb.h>

#include "driverrs.h"
#include "cntrloma.h"
#include "oma4scan.h"
#include "monitor.h"
#include "counters.h"
#include "oma4driv.h"  /* bits_open */
#include "detsetup.h"  /* bits_open */
#include "asicodes.h"
#include "cache386.h"
#include "access4.h"

#ifdef USE_D16M
#include "d16mphys.h"
#elif defined(__WATCOMC__)
#include "wc32phys.h"
#elif defined(_WINOMA_)
#include "winphys.h"
#else
#include "himem.h"
#endif

     /* Source Comp modifiers */

static USHORT scomp_values[] = { detNOINT, detUS10, detUS100, detMS1,  
                                 detMS10, detMS100, detS1
                               };

#define TWOMEG ((ULONG)0x200000)
                         
#ifdef _PROTECTED_MODE
#define get_message_area();
#define put_message_area();
#else
#define get_message_area(); read_high_memory(CommonArea, 0L,  sizeof(SHARED_COMM_AREA));
#define put_message_area(); write_high_memory(CommonArea, 0L, sizeof(SHARED_COMM_AREA));
#endif

#ifndef __WATCOMC__
USHORT far FC_mode_list[256];
USHORT far DC_mode_list[256];
USHORT far FC_mode_count = 0, DC_mode_count = 0;
USHORT far DAC_seq_count = 0;
#else
USHORT FC_mode_list[256];
USHORT DC_mode_list[256];
USHORT FC_mode_count = 0, DC_mode_count = 0;
USHORT DAC_seq_count = 0;
#endif

static BOOLEAN HasFastShift = FALSE;

/*********************************************************************/

/* detector instructions */

typedef struct
  {
  const ULONG dNO_OP;  /* No Operation.           */
  const ULONG dNPAS;   /* Normal slow Pixel A.         must be here   */
  const ULONG dNPA;    /* Normal pixel A.              in this order  */
  const ULONG dNPAF;   /* Normal fast Pixel A.         for pixtime    */
  const ULONG dNPBS;   /* Normal slow Pixel B.         setting to work*/
  const ULONG dNPB;    /* Normal pixel B.         */
  const ULONG dNPBF;   /* Normal fast Pixel B.    */
  const ULONG dTP;     /* Trash Pixel.            */
  const ULONG dTPS;    /* Trash Pixel Slow        */
  const ULONG dAO;     /* Acquire Offset.         */
  const ULONG dDPX;    /* Dump Pixel.             */
  const ULONG dDPXS;   /* Dump Pixel Slow.        */
  const ULONG dASA;    /* Acquire Signal A.       */
  const ULONG dASB;    /* Acquire Signal B.       */
  const ULONG dAOF;    /* Acquire Offset Fast.    */
  const ULONG dASAF;   /* Acq Signal A Fast.      */
  const ULONG dASBF;   /* Acq Signal B Fast.      */
  const ULONG dSTSC;   /* Start Source Comp.      */
  const ULONG dSC1;    /* Read Source Comp        */
  const ULONG dSC2;    /* Read Source Comp        */
  const ULONG dSLS;    /* Shift Line slow.        */
  const ULONG dSLN;    /* Shift Line normal.      */
  const ULONG dSLF;    /* Shift Line fast.        */
  const ULONG FLUSH;   /* Flush pipeline.         */
  } DET_OPS;

#ifndef __WATCOMC__
static DET_OPS far sc_det_ops =
#else
static DET_OPS sc_det_ops =
#endif
  {
  0x000000L,   /* dNO_OP  */
  0x000001L,   /* dNPAS   */
  0x000002L,   /* dNPA    */
  0x000003L,   /* dNPAF   */
  0x000004L,   /* dNPBS   */
  0x000005L,   /* dNPB    */
  0x000006L,   /* dNPBF   */
  0x000007L,   /* dTP     */
  0x000000L,   /* dTPS    */
  0x000008L,   /* dAO     */
  0x000009L,   /* dDPX    */
  0x000000L,   /* dDPXS   */
  0x00000AL,   /* dASA    */
  0x00000BL,   /* dASB    */
  0x000014L,   /* dAOF    */
  0x000015L,   /* dASAF   */
  0x000016L,   /* dASBF   */
  0x000011L,   /* dSTSC   */
  0x00000CL,   /* dSC1    */
  0x000000L,   /* dSC2    */
  0x00000DL,   /* dSLS    */
  0x00000EL,   /* dSLN    */
  0x000017L,   /* dSLN    */
  0x001D00L,   /* FLUSH   */
  };

#ifndef __WATCOMC__
static DET_OPS far sm_det_ops =
#else
static DET_OPS sm_det_ops =
#endif
  {
  0x000000L,   /* dNO_OP  */
  0x000800L,   /* dNPAS   */
  0x000200L,   /* dNPA    */
  0x000100L,   /* dNPAF   */
  0x000900L,   /* dNPBS   */
  0x000400L,   /* dNPB    */
  0x000300L,   /* dNPBF   */
  0x000C00L,   /* dTP     */
  0x000600L,   /* dTPS    */
  0x000000L,   /* dAO     */
  0x000700L,   /* dDPX    */
  0x000500L,   /* dDPXS   */
  0x000000L,   /* dASA    */
  0x000000L,   /* dASB    */
  0x000000L,   /* dAOF    */
  0x000000L,   /* dASAF   */
  0x000000L,   /* dASBF   */
  0x000F00L,   /* dSTSC   */
  0x001000L,   /* dSC1    */
  0x001001L,   /* dSC2    */
  0x000000L,   /* dSLS    */
  0x000D00L,   /* dSLN    */
  0x000D00L,   /* dSLF    */
  0x001D00L,   /* FLUSH   */
  };

#ifndef __WATCOMC__
static DET_OPS far * ops = &sc_det_ops;
#else
static DET_OPS * ops = &sc_det_ops;
#endif

typedef struct
  {
  const ULONG dWFS;     /* Wait for sync  */
  const ULONG dRST;     /* Reset trigger. */
  const ULONG dSET_A;   /* Set Trigger A  */
  const ULONG dRS_A;    /* Reset Trigger A*/
  const ULONG dSET_B;   /* Set Trigger B  */
  const ULONG dRS_B;    /* Reset Trigger B*/
  const ULONG dOPEN;    /* Open shutter.           */
  const ULONG dCLOSE;   /* Close shutter.          */
  const ULONG dCUE;     /* Cue the DMA Controller. */
  } CONDITIONS;

#ifndef __WATCOMC__
static CONDITIONS far sc_cond =
#else
static CONDITIONS sc_cond =
#endif
                       {
                       0x00000FL,   /* dWFS   */  /* syncs and trigs */
                       0x000010L,   /* dRST   */
                       0x000200L,   /* dSET_A */
                       0x000100L,   /* dRS_A  */
                       0x000800L,   /* dSET_B */
                       0x000400L,   /* dRS_B  */
                       0x000040L,   /* dOPEN  */  /* shutter */
                       0x000080L,   /* dCLOSE */
                       0x800000L,   /* dCUE   */
                     };

#ifndef __WATCOMC__
static CONDITIONS far sm_cond =
#else
static CONDITIONS sm_cond =
#endif
                       {
                       0x000040L,   /* dWFS   */  /* syncs and trigs */
                       0x000080L,   /* dRST   */
                       0x000004L,   /* dSET_A */
                       0x000008L,   /* dRS_A  */
                       0x000010L,   /* dSET_B */
                       0x000020L,   /* dRS_B  */
                       0x000001L,   /* dOPEN  */  /* shutter */
                       0x000002L,   /* dCLOSE */
                       0x800000L,   /* dCUE   */
                     };

#ifndef __WATCOMC__
static CONDITIONS far * conds = &sc_cond;
#else
static CONDITIONS * conds = &sc_cond;
#endif

/* address calculation instructions */

typedef struct
  {
  const ULONG dADDA;     /* Use A Pointers.           */
  const ULONG dRPP;      /* Reset Pixel Pointer A.    */
  const ULONG dIPP;      /* Increment Pixel Pointer A.*/
  const ULONG dRTP;      /* Reset Track Pointer A.    */
  const ULONG dITP;      /* Increment Track Pointer A.*/
  const ULONG dRPP_RTP;  /* Reset A Pointers.         */
  const ULONG dRPP_ITP;  /* RS PPTRA & INC TPTRA.     */
  const ULONG dIPP_RTP;  /* INC PPTRA & RS TPTRA.     */

  const ULONG dADDB;     /* Use B Pointers.           */
  const ULONG dLPP;      /* Load Pixel Pointer B.     */
  const ULONG dDPP;      /* Decrement Pixel Pointer B.*/
  const ULONG dLTP;      /* Load Track Pointer B.     */
  const ULONG dDTP;      /* Decrement Track Pointer B.*/
  const ULONG dLPP_LTP;  /* Load B Pointers.          */
  const ULONG dLPP_DTP;  /* LD PPTRB & DEC TPTRB.     */
  const ULONG dDPP_LTP;  /* DEC PPTRB & LD TPTRB.     */
  } ADDCALCS;


#ifndef __WATCOMC__
static ADDCALCS far sc_adcalc =
#else
static ADDCALCS sc_adcalc =
#endif
                       {
                       0x000000L,  /* dADDA    */
                       0x080000L,  /* dRPP     */
                       0x100000L,  /* dIPP     */
                       0x180000L,  /* dRTP     */
                       0x200000L,  /* dITP     */
                       0x280000L,  /* dRPP_RTP */
                       0x300000L,  /* dRPP_ITP */
                       0x380000L,  /* dIPP_RTP */

                       0x400000L,  /* dADDB    */
                       0x480000L,  /* dLPP     */
                       0x500000L,  /* dDPP     */
                       0x580000L,  /* dLTP     */
                       0x600000L,  /* dDTP     */
                       0x780000L,  /* dLPP_LTP */
                       0x700000L,  /* dLPP_DTP */
                       0x680000L,  /* dDPP_LTP */
                      };

#ifndef __WATCOMC__
static ADDCALCS far * adcalc = &sc_adcalc;
#else
static ADDCALCS * adcalc = &sc_adcalc;
#endif

USHORT bits_open ;
USHORT bits_close ;

const  USHORT sc_bits_open =  0x0040;
const  USHORT sc_bits_close = 0x0080;
const  USHORT sm_bits_open =  0x0001;
const  USHORT sm_bits_close = 0x0002;

/* for analog proc board */
#define ACHANNEL    0x01L      /* Selects A high, B low.    */
#define BCHANNEL    0x00L      /* Selects B preamp          */

/* for clock/bias board */
#define A_CHANNEL   0x00L      /* Shift image toward A reg. */
#define B_CHANNEL   0x02L      /* Shift image toward B reg. */
#define D_CHANNEL   0x04L      /* Shift image toward both.  */

static BOOLEAN holdoff_release = FALSE;
static SHORT no_init_wait = 0;

/**************************************************************************/
/*                                                                        */
/*  Switch the DAC into a new DA sequence, wait for it to begin, then     */
/*  leave a message to tell it what to do after the current DA completes  */
/*  If DoLive is Zero, then the next thing to do is keepclean             */
/*  If DoLive is not Zero, then the DA sequence repeats                   */
/*                                                                        */
/**************************************************************************/

SHORT switch_da(SHORT DAC_seq, SHORT FC_seq, SHORT OH_seq, SHORT DoLive)
{
  SHORT err = 0, i = 0;

  ULONG millisecs, killisecs;
  struct timeb tstruct;

  if (! fake_detector)
    {
    ULONG timeout;

    cacheoff();
    get_message_area();
    CommonArea->MSM_Act_RQ = SM_SWITCH_DA;
    if (FC_seq >= 0)
      CommonArea->MSM_FC_Scan_Mode = FC_mode_list[FC_seq];
    if (DAC_seq >= 0)
      CommonArea->MSM_Pgm_Inx= DAC_seq;
    if (OH_seq >= 0)
    CommonArea->MSM_FC_OH  = FC_mode_list[FC_NORM_OH + OH_seq];

    if (OH_seq == OHNOET) /* no expose time */
      CommonArea->MSM_DC_OH = DC_mode_list[DCOHNE];
    else if (OH_seq == OHLCI || OH_seq == OHLCINP)
      CommonArea->MSM_DC_OH = DC_mode_list[DCOHTP];
    else if (OH_seq >= 0)
      CommonArea->MSM_DC_OH = DC_mode_list[DCOH];

    CommonArea->MSM_Act_RQ_RPL = SMR_CLEAR;

    put_message_area();
    release_OMA_reset();

    if (no_init_wait == FALSE)
      {
      if (det_setup->same_et)     /* if "fast" keepclean, wait frame time plus */
        {                         /* keepclean overhead */
        FLOAT flt_time;
        get_FrameTime(&flt_time);
        timeout = (ULONG)(flt_time * 1000.0F + 200.0F);
        if (flt_time < 1.0F)
          timeout += 1000;        /* make minimum wait 1 second */
        }
      else
        timeout = 1200;           /* wait 1.2 sec if not "slow" keepclean */

      ftime(&tstruct);
      millisecs = tstruct.millitm + tstruct.time * 1000;

      do
        {
        ftime(&tstruct);
        killisecs = (tstruct.millitm + tstruct.time * 1000) - millisecs;
        get_message_area();
        i++;
        }
      while (CommonArea->MSM_Act_RQ_RPL == SMR_CLEAR && killisecs < timeout);

      if (CommonArea->MSM_Act_RQ_RPL == SMR_CLEAR)
        err = ERROR_DETECTOR_TIMEOUT;
      }

    CommonArea->MSM_DA_Complete = MSPC_CLEAR;
    CommonArea->MSM_Pgm_Inx = (DoLive ? (DAC_seq) : DA_CLEAN);

    put_message_area();

    cacheon();
    }
  return err;
}

void set_holdoff_release(void)
{
  holdoff_release = FALSE;
}

void clr_holdoff_release(void)
{
  holdoff_release = FALSE;
}

SHORT release_OMA_and_init(void)
{
  if (!holdoff_release && !fake_detector)
    {
    reset_and_map_data();
    return(switch_da(DA_INIT, -1, -1, 0));
    }
  else
    return 0;
}

/**************************************************************************/
/*                                                                        */
/*  Stop any data acquisition sequence in progress by putting the         */
/*  OMA board into reset, then releasing the reset, which causes          */
/*  the monitor to do its init, then go into the HALTED state.            */
/*                                                                        */
/**************************************************************************/

SHORT stop_OMA_DA_in_progress()
{
  SHORT err = NO_ERROR;

  if (!fake_detector)
    {
    return(release_OMA_and_init());
    }
  return err;
}

/***************************************************************************/
/*                                                                         */
/* Sort OMA data if split mode is being used.                              */
/*                                                                         */
/***************************************************************************/

void sort_OMA4_data(PCHAR pData, SHORT len, USHORT curvenum, ULONG data_offset, SHORT direction)
{
  SHORT track = curvenum % det_setup->tracks;

  if (det_setup->shiftmode == 0)         /* simple mode */
    {
    ULONG * pLongs = (ULONG *)pData;
    if (track >= det_setup->tracks / 2)  /* is track in 2nd half?*/
      {                                  
      SHORT i, j = len / sizeof(LONG);
      ULONG temp;
      for (i = 0;i < j / 2; i++)
        {
        temp = pLongs[j-i];
        pLongs[j-i] = pLongs[i];
        pLongs[i] = temp;

        }
      }
    }
  else /* classic mode, always sort if dual */
    {
#ifdef _PROTECTED_MODE
  #ifdef __WATCOMC__
    void __far * pm_data_ptr;     /* define 48 bit far pointer */
  #else
    void __huge * pm_data_ptr;  /* define 16:16 bit huge pointer */
  #endif
#endif
    ULONG real_address;
    ULONG offset;
    
    track = curvenum - (2 * track) + det_setup->tracks - 1;
    offset = (LONG)((LONG)len * (LONG)track);
    len = len / 2; /* look halfway through requested data */
    real_address = offset + data_offset + (ULONG)(len);

#ifdef _PROTECTED_MODE
    pm_data_ptr = GetProtectedPointer(real_address);
    if (direction)
      _fmemcpy(&(pData[len]), pm_data_ptr, len); /* read the data */
    else
      _fmemcpy(pm_data_ptr, &(pData[len]), len); /* write the data */
#else
    if (direction)
      read_high_memory(&(pData[len]), real_address, len);
    else
      write_high_memory(&(pData[len]), real_address, len);
#endif
    }
}

/***************************************************************************/
/*                                                                         */
/* Transfer data from OMA data memory into address pData                   */
/*                                                                         */
/***************************************************************************/

void trans_OMA4_data(PCHAR pData, SHORT len, USHORT curvenum, SHORT direction)
{
  ULONG offset = (LONG)((LONG)len * (LONG)curvenum);

  ULONG data_offset = get_DAC_pointer(FRAME_PTR);
  ULONG real_address;

#ifdef _PROTECTED_MODE
 #ifdef __WATCOMC__
  void __far * pm_data_ptr;     /* define 48 bit far pointer */
 #else
  void __huge * pm_data_ptr;  /* define 16:16 bit huge pointer */
 #endif
#endif

  if ((!direction) && (det_setup->outputReg == 2)) /* if Dual mode */
    sort_OMA4_data(pData, len, curvenum, data_offset, 0); /*sort bef writing*/

  /* get selector to point to data memory on OMA board */

  real_address = offset + data_offset;

#ifdef _PROTECTED_MODE
  pm_data_ptr = GetProtectedPointer(real_address);

  cacheoff();
  if (direction)
    _fmemcpy(pData, pm_data_ptr, len); /* read the data */
  else
    _fmemcpy(pm_data_ptr, pData, len); /* write the data */
  cacheon();
#else
  cacheoff();
  if (direction)
    read_high_memory(pData, real_address, len);
  else
    write_high_memory(pData, real_address, len);
  cacheon();
#endif
  if (direction && (det_setup->outputReg == 2)) /* if Dual mode */
    sort_OMA4_data(pData, len, curvenum, data_offset, direction);
}

/***************************************************************************/
/*                                                                         */
/* Get len bytes of data from OMA memory into address pData                */
/*                                                                         */
/***************************************************************************/

void get_OMA4_data(PCHAR pData, SHORT len, USHORT curvenum)
{
  trans_OMA4_data(pData, len, curvenum, 1);
}

/***************************************************************************/
/*                                                                         */
/* Put len bytes of data into OMA memory from address pData                */
/*                                                                         */
/***************************************************************************/

void put_OMA4_data(PCHAR pData, SHORT len, USHORT curvenum)
{
  trans_OMA4_data(pData, len, curvenum, 0);
}

/***************************************************************/
/*                                                             */
/* Helper function for routines which access the ASIC program  */
/*                                                             */
/***************************************************************/

USHORT setup_for_asic_patch(counter_link_record __far ** lc_tbl_addrs)
{
  map_program_memory();
  /* get address of FC block table */
#ifdef _PROTECTED_MODE
  *lc_tbl_addrs = (counter_link_record __far * )&(((CHAR __far *)monitor_addr)[monitor_addr->loopcntr_table_ptr]);
#else
  *lc_tbl_addrs = (counter_link_record __far *) (&(loop_counters[0]));
#endif
  return (monitor_addr->loopcntr_table_index);
}

/***************************************************************/
/*                                                             */
/* Set bits in a value embedded in the ASIC program            */
/*                                                             */
/***************************************************************/

void set_asic_opbits(USHORT class, USHORT value)
{
  counter_link_record __far * lc_tbl_addrs;
  USHORT lc_tbl_index;
  USHORT i, offset, processor, opvalue;

  if (!fake_detector)
    {
    lc_tbl_index = setup_for_asic_patch(&lc_tbl_addrs);

    for (i = 0;i < lc_tbl_index && i < LC_TABLE_LIMIT; i++)
      {
      if (lc_tbl_addrs[i].class == class)
        {
        offset = lc_tbl_addrs[i].offset;
        processor = lc_tbl_addrs[i].processor;

        switch(processor)
          {
          case FC_PROC:
            opvalue = read_FC_counter(offset);
            opvalue |= value;
            update_FC_counter(opvalue, offset);
          break;
          case DC_PROC:
            opvalue = read_DC_counter(offset);
            opvalue |= value;
            update_DC_counter(opvalue, offset);
          break;
          case DAC_PROC:
          break;
          }
        }
      }
    release_OMA_and_init();
    }
}

/***************************************************************/
/*                                                             */
/* Clear bits in a value embedded in the ASIC program          */
/*                                                             */
/***************************************************************/

void clr_asic_opbits(USHORT class, USHORT value)
{
  counter_link_record __far * lc_tbl_addrs;
  USHORT lc_tbl_index;
  USHORT i, offset, processor, opvalue;

  if (!fake_detector)
    {
    lc_tbl_index = setup_for_asic_patch(&lc_tbl_addrs);

    for (i = 0;i < lc_tbl_index && i < LC_TABLE_LIMIT; i++)
      {
      if (lc_tbl_addrs[i].class == class)
        {
        offset = lc_tbl_addrs[i].offset;
        processor = lc_tbl_addrs[i].processor;

        switch(processor)
          {
          case FC_PROC:
            opvalue = read_FC_counter(offset);
            opvalue &= (~value);
            update_FC_counter(opvalue, offset);
          break;
          case DC_PROC:
            opvalue = read_DC_counter(offset);
            opvalue &= (~value);
            update_DC_counter(opvalue, offset);
          break;
          case DAC_PROC:
          break;
          }
        }
      }
    release_OMA_and_init();
    }
}

/***************************************************************/
/*                                                             */
/* Replace bits in a value embedded in the ASIC program        */
/*                                                             */
/***************************************************************/

BOOLEAN rep_asic_opbits(USHORT class, USHORT value)
{
  counter_link_record __far * lc_tbl_addrs;
  USHORT lc_tbl_index, i, offset, processor;
	BOOLEAN DidSet = FALSE;

  if (!fake_detector)
    {
    lc_tbl_index = setup_for_asic_patch(&lc_tbl_addrs);

    for (i = 0;i < lc_tbl_index && i < LC_TABLE_LIMIT; i++)
      {
      if (lc_tbl_addrs[i].class == class)
        {
        offset = lc_tbl_addrs[i].offset;
        processor = lc_tbl_addrs[i].processor;
				DidSet = TRUE;
        switch(processor)
          {
          case FC_PROC:
            update_FC_counter(value, offset);
          break;
          case DC_PROC:
            update_DC_counter(value, offset);
          break;
          case DAC_PROC:
          break;
          }
        }
      }
    release_OMA_and_init();
    }
	return(DidSet);
}

/***************************************************************/
/*                                                             */
/* Retrieve a 16 bit value embedded in the ASIC program        */
/*                                                             */
/***************************************************************/

USHORT get_asic_counter(USHORT class)
{
  counter_link_record __far * lc_tbl_addrs;
  USHORT lc_tbl_index;
  USHORT i, offset, processor;
  USHORT return_val = 0;

  if (!fake_detector)
    {
    lc_tbl_index = setup_for_asic_patch(&lc_tbl_addrs);

    for (i = 0;i < lc_tbl_index && i < LC_TABLE_LIMIT; i++)
      {
      if (lc_tbl_addrs[i].class == class)
        {
        offset = lc_tbl_addrs[i].offset;
        processor = lc_tbl_addrs[i].processor;

        switch(processor)
          {
          case FC_PROC:
            return_val = read_FC_counter(offset);
          break;
          case DC_PROC:
            return_val = read_DC_counter(offset);
          break;
          case DAC_PROC:
          break;
          }
        }
      }
    release_OMA_and_init();
    }
  return(return_val);
}

/*****************************************************************/
/*                                                               */
/* Retrieve a 16 bit asic address for a tag "class" embedded     */
/* in the ASIC program; if tag occurs several times, will        */
/* return the first occurrence.  Return 0 if tag not found. */
/*                                                               */
/*****************************************************************/

USHORT get_asic_offset (USHORT processor, USHORT class)
{
  counter_link_record __far * lc_tbl_addrs;
  USHORT lc_tbl_index;
  USHORT i, offset = 0;

  if (!fake_detector)
    {
    lc_tbl_index = setup_for_asic_patch(&lc_tbl_addrs);

    for (i = 0;i < lc_tbl_index && i < LC_TABLE_LIMIT; i++)
      if ((lc_tbl_addrs[i].class == class) &&
          (lc_tbl_addrs[i].processor == processor))
        {
        offset = lc_tbl_addrs[i].offset;
        break;
        }
    release_OMA_and_init();
    }
  return (offset);
}

ULONG get_sharedcomm_size(void)
{
  return(sizeof(SHARED_COMM_AREA));
}

/***************************************************************/
/*                                                             */
/* Replace bits in a 24 bit value embedded in the ASIC program */
/*                                                             */
/***************************************************************/

void set_asic_pointer(USHORT class, ULONG value)
{
  counter_link_record __far * lc_tbl_addrs;
  USHORT lc_tbl_index;
  USHORT i, offset, processor;

  if (!fake_detector)
    {
    lc_tbl_index = setup_for_asic_patch(&lc_tbl_addrs);

    for (i = 0;i < lc_tbl_index && i < LC_TABLE_LIMIT; i++)
      {
      if (lc_tbl_addrs[i].class == class)
        {
        offset = lc_tbl_addrs[i].offset;
        processor = lc_tbl_addrs[i].processor;

        switch(processor)
          {
          case FC_PROC:
            access_alternating_bytes(WRITE_EVEN, &value, offset, 3);
          break;
          case DC_PROC:
            access_alternating_bytes(WRITE_ODD, &value, offset, 3);
          break;
          case DAC_PROC:
          break;
          }
        }
      }
    release_OMA_and_init();
    }
}

/***************************************************************/
/*                                                             */
/* Replace bits in a 24 bit value embedded in the ASIC program */
/*                                                             */
/***************************************************************/

void set_cue_onoff(USHORT class, BOOLEAN OnOrOff)
{
  counter_link_record __far * lc_tbl_addrs;
  USHORT lc_tbl_index;
  USHORT i, offset, processor;
  UCHAR temp;

  if (!fake_detector)
    {
    lc_tbl_index = setup_for_asic_patch(&lc_tbl_addrs);

    for (i = 0;i < lc_tbl_index && i < LC_TABLE_LIMIT; i++)
      {
      if (lc_tbl_addrs[i].class == class)
        {
        offset = lc_tbl_addrs[i].offset + 2;
        processor = lc_tbl_addrs[i].processor;

        switch(processor)
          {
          case FC_PROC:
            access_alternating_bytes(READ_EVEN, &temp, offset, 1);
            if (OnOrOff)
              temp |= (UCHAR)(0x80);
            else
              temp &= ~(UCHAR)(0x80);
            access_alternating_bytes(WRITE_EVEN, &temp, offset, 1);
          break;
          case DC_PROC:
            access_alternating_bytes(READ_ODD, &temp, offset, 1);
            if (OnOrOff)
              temp |= (UCHAR)(0x80);
            else
              temp &= ~(UCHAR)(0x80);
            access_alternating_bytes(WRITE_ODD, &temp, offset, 1);
          break;
          case DAC_PROC:
          break;
          }
        }
      }
    release_OMA_and_init();
    }
}

/***************************************************************/
/* The next few functions are here to isolate detdriver from   */
/* hardware specific values and operations.  Partially does so.*/
/***************************************************************/

/* status can be 0, or 1 */
USHORT set_inex_shutter_status(SHORT status)
{
  /* internal/external shutter is bit 1 of operand */

  clr_asic_opbits(LCI_CONFIG, 0xFF);
  if (status)
    {
    set_asic_opbits(INEX_SHUTTER, 1);
    set_asic_opbits(LCI_CONFIG, 1);
    }
  else
    clr_asic_opbits(INEX_SHUTTER, 1);
  return(1);
}


USHORT set_streakmode_status(SHORT status1, SHORT status2)
{
  USHORT retval;

  if (fake_detector)
    return 1;

  get_message_area();
  switch(status1)
    {
    case DAC_STRK_NORM:
    case DAC_STRK_NORM_ET:
    case DAC_STRK_NORM_SYNC:
    case DAC_STRK_NORM_ET_SYNC:
    case DAC_STRK_PRETRIG_ET:
    case DAC_STRK_LCI:
    case DAC_STRK_LCI_CW:
      CommonArea->MSM_FC_StreakMode =
        FC_mode_list[status1];
      retval = 1;
    break;
    default:
      retval = 0;
    }
  if (retval)
    {
    switch (status2)
      {
      case DAC_STRK_OT:
      case DAC_STRK_OT_ET:
      case DAC_STRK_OT_SYNC:
      case DAC_STRK_OT_ET_SYNC:
      case DAC_STRK_PRETRIG_ET:
      case DAC_STRK_LCI:
      case DAC_STRK_LCI_CW:
        CommonArea->MSM_FC_StrkOTMode =
          FC_mode_list[status2];
        retval = 1;
      break;
      default:
        retval = 0;
      }
    }
  put_message_area();
  return retval;
}

/* status can be 0, 1, or 2 */
USHORT set_shutter_forced_status(SHORT status)
{
   USHORT clr_before, clr_after, set_before, set_after;

   switch (status)
    {
    case FORCED_OPEN:
      clr_before = clr_after = bits_close;
      set_before = set_after = bits_open;
    break;

    case FORCED_CLOSE:
      clr_before = clr_after = bits_open;
      set_before = set_after = bits_close;
    break;
    
    case NORMAL_SHUTTER:
      clr_before = set_after = bits_close;
      set_before = clr_after = bits_open;
    break;

    default:
      return(0);
    }
  clr_asic_opbits(SHUTTER_BEF, clr_before);
  clr_asic_opbits(SHUTTER_AFT, clr_after);
  set_asic_opbits(SHUTTER_BEF, set_before);
  set_asic_opbits(SHUTTER_AFT, set_after);

  return(1);
}

/* status can be 0, or 1 */
USHORT set_sync_open_status(SHORT status)
{
  if (status) set_asic_opbits(SYNC_OPEN, (SHORT)conds->dWFS );
  else        clr_asic_opbits(SYNC_OPEN, (SHORT)conds->dWFS );
  return(1);
}

/* status can be 0, or 1 */
USHORT set_sync_close_status(SHORT status)
{
  if (status) set_asic_opbits(SYNC_CLOSE, (SHORT)conds->dWFS );
  else        clr_asic_opbits(SYNC_CLOSE, (SHORT)conds->dWFS );
  return(1);
}

USHORT set_source_comp_status(SHORT status) 
{
  clr_asic_opbits(SCITC_SET, (SHORT)detSCMP);
  set_asic_opbits(SCITC_SET, (SHORT)scomp_values[status] );
  return(1);
}

/***********************************************************/
/* status can be 0, 1, or 2 */
USHORT set_pixtime_status(SHORT status)
{
  ULONG pixcode1, pixcode2;
  switch (status)
    {
    default:
    case 0:
      pixcode1 = ops->dNPA;
      pixcode2 = ops->dNPB;
    break;
    case 1:
      pixcode1 = ops->dNPAF;
      pixcode2 = ops->dNPBF;
    break;
    case 2:
      pixcode1 = ops->dNPAS;
      pixcode2 = ops->dNPBS;
    break;
    }

  clr_asic_opbits(PIXTIME_NP,(USHORT)(ops->dNPA | ops->dNPAF | ops->dNPAS) |
                             (USHORT)(ops->dNPB | ops->dNPBF | ops->dNPBS));
  set_asic_opbits(PIXTIME_NP, (USHORT)pixcode1);

  clr_asic_opbits(SEC_CUE,(USHORT)(ops->dNPA | ops->dNPAF | ops->dNPAS) |
                          (USHORT)(ops->dNPB | ops->dNPBF | ops->dNPBS));
  set_asic_opbits(SEC_CUE, (USHORT)pixcode2);
  return(1);
}

/***********************************************************/
/* status can be 0, 1, or 2 */
USHORT set_shifttime_status(SHORT status)
{
  ULONG shiftcode;
  switch (status)
    {
    default:
    case 0:
      shiftcode = ops->dSLN; /* normal shift line */
    break;
    case 1:
      shiftcode = ops->dSLF; /* fast shift line */
    break;
    case 2:
      shiftcode = ops->dSLS; /* slow shift line */
    break;
    }
  clr_asic_opbits(SHIFT_SPEED,(USHORT)(ops->dSLS | ops->dSLF | ops->dSLN));
  set_asic_opbits(SHIFT_SPEED,(USHORT)shiftcode);
  return(1);
}

/* status values: 0=A, 1=B, or 2=Dual */

USHORT set_shiftreg_status(SHORT status) 
{
  clr_asic_opbits(BIAS_AB, (SHORT)(A_CHANNEL | B_CHANNEL | D_CHANNEL));
  switch(status)
    {
    case 2:
      set_asic_opbits(BIAS_AB, (SHORT)D_CHANNEL);
      set_asic_opbits(APROC_AB, (SHORT)ACHANNEL); /* use A channel for splmode*/
      set_cue_onoff(SEC_CUE, TRUE);
      set_cue_onoff(PIXTIME_NP, TRUE);
    break;
    case 1:
      set_asic_opbits(BIAS_AB, (SHORT)B_CHANNEL);
      clr_asic_opbits(APROC_AB, (SHORT)ACHANNEL);
      if (det_setup->detector_type_index == TSM_CCD) /* detector is spmode */
        {
        set_cue_onoff(PIXTIME_NP, FALSE);
        set_cue_onoff(SEC_CUE, TRUE);
        }
    break;
    case 0:
    default:
      set_asic_opbits(BIAS_AB, (SHORT)A_CHANNEL);
      set_asic_opbits(APROC_AB, (SHORT)ACHANNEL);
      if (det_setup->detector_type_index == TSM_CCD) /* detector is spmode */
        {
        set_cue_onoff(PIXTIME_NP, TRUE);
        set_cue_onoff(SEC_CUE, FALSE);
        }  
    break;
    }
  return(1);
}

USHORT calc_antibloom_counts(USHORT percent, USHORT total,
                             USHORT * inloop,
                             USHORT * outloop)
{
  USHORT i = 1;                 /* 1 so ASIC counter acts as if zero */

  if (total == 0)
    {
    *outloop = 1;                 /* same in this case */
    *inloop = 0;                  /* and inloop doesn't really matter */
    }
  else if (percent == 100 || total < 2) /* 100 % antibloom */
    {
    *inloop = 1;                  /* shift 1 line at a time */
    *outloop = total+1;           /* do all clean shift regs */
    }
  else if (percent == 0)          /* no antibloom */
    {
    *inloop = total;              /* shift whole array at once */
    *outloop = 2;                 /* do 1 clean shift reg */
    }
  else
    {
    i = 100 - percent;            /* so i is 10, 20, 30, etc. */
    *inloop = (total * i) / 100;  /* inloop= 10/100, 20/100, etc of total */
    if (!*inloop)
      (*inloop)++;
    *outloop = total / *inloop;
    i = total - (*inloop * *outloop) + 1; /* remainder */
    (*outloop)++;                          /* so ASIC can test for =1 */
    }
  return i;
}

/*****************************************************************/
/*                                                               */
/* Return the source comp point corresponding to a given frame.  */
/* Source comps are stored in an array right before data memory. */
/* The address of the array is in MSM_SC_address in CommonArea.  */
/*                                                               */
/*****************************************************************/

ULONG access_source_comp(SHORT frame)
{
  SHORT active= ((SHORT)get_DAC_counter(DA_SEQ) > 2) || (!get_DAC_counter(DA_DONE));
  ULONG scmp_base = get_DAC_pointer(SRCCMP_PTR) +
           (ULONG)((ULONG)frame * sizeof(ULONG)) +
           active * sizeof(LONG);
  
  ULONG return_val;

#ifdef _PROTECTED_MODE
  ULONG __far * pm_data_ptr;

  /* get selector to point to data memory on OMA board */

  pm_data_ptr = GetProtectedPointer(scmp_base);

  if (FP_SEG(pm_data_ptr))
    return_val = *pm_data_ptr;
  else
    return_val = 0;
#else
  read_high_memory(&return_val, scmp_base, sizeof(ULONG));
#endif

  return(return_val);

}

void wait_off(void)
{
  no_init_wait = TRUE;
}

void wait_on(void)
{
  no_init_wait = FALSE;
}

USHORT wait_id(void)
{
  ULONG millisecs;
  struct timeb tstruct;
  USHORT det_id, i;

  ftime(&tstruct);
  millisecs = tstruct.millitm + tstruct.time * 1000;

  for (i = 0;i < 10;i++)
    {
    do   /* wait 100 msecs each time for keepclean or init to start */
      {
      ftime(&tstruct);
      }
    while ((((tstruct.millitm + tstruct.time * 1000) - millisecs) < 100));

    det_id = get_DAC_counter(DET_ID);
    if (!det_id) det_id = (USHORT)get_DAC_counter(DET_ID);
    if (det_id) break;
    }
  return(det_id);
}

BOOLEAN InqHasFastShift(void)
{
  return (HasFastShift);
}

SHORT convert_code_to_index(USHORT code)
{
  SHORT det_id;
  
  switch (code)
    {
    case 0xF5:             /* Th 512 X 512 with 3usec shift */
      HasFastShift = TRUE;
    case 0xFF:
      det_id = TSC_CCD;    /* Th 512 X 512 */
    break;
    case 0xFE:
      det_id = TSM_CCD;    /* Th 512 X 512 Split Mode */
    break;
    case 0xFD:
      det_id = RAPDA;      /* Reticon 1024 PDA */
    break;
    case 0xFB:
      det_id = EEV_CCD;    /* EEV 1024 X 256 */
    break;
    case 0xF4:             /* Th 1024 X 256 with 3usec shift */
     HasFastShift = TRUE;
    case 0xF7:
      det_id = TSP_CCD;    /* Th 1024 X 256 CCD */
    break;
    case 0xF6:
      HasFastShift = TRUE; /* all Th 1024 X 1024's have usec shift */
      det_id = TS_1KSQ;    /* Th 1024 X 1024 CCD */
    break;
    default:
      det_id = NO_CARD;
    break;
    }
  return(det_id);
}

static void switch_to_init(void)
{
  reset_and_map_data();
  cacheoff();
  get_message_area();
  CommonArea->MSM_Act_RQ = SM_SWITCH_DA;
  CommonArea->MSM_FC_Scan_Mode = 0;
  CommonArea->MSM_Pgm_Inx = 0;
  put_message_area();
  switch_da(DA_INIT, 0, 0, 1);
}

SHORT id_detector(void)
{
  USHORT det_id;

  HasFastShift = FALSE;
  no_init_wait = TRUE;
  switch_to_init();
  det_id = wait_id();
  det_id = convert_code_to_index(det_id);

  if (det_id == NO_CARD)
    {
    CommonArea->MSM_Pgm_Inx = 2;  /* select alternate id routine */
    put_message_area();
    det_id = wait_id();
    det_id = convert_code_to_index(det_id);
    switch_to_init();
    }

  det_setup->detector_type_index = det_id;

  if(det_setup->detector_type_index == TSM_CCD)
    {
    ops = &sm_det_ops;
    conds = &sm_cond;
    adcalc = &sc_adcalc;
    bits_open = sm_bits_open;
    bits_close = sm_bits_close;
    }
  else
    {
    ops = &sc_det_ops;
    conds = &sc_cond;
    adcalc = &sc_adcalc;
    bits_open = sc_bits_open;
    bits_close = sc_bits_close;
    }

  no_init_wait = FALSE;
  return(det_id == 0);
}

/*****************************************************************/
/*                                                               */
/* Update det_setup fields with corresponding values embedded in */
/* the ASIC programs                                             */
/*                                                               */
/*****************************************************************/

void scan_counter_table(void)
{
  counter_link_record __far * lc_tbl_addrs;
  USHORT lc_tbl_index, value, i;
#ifndef _PROTECTED_MODE
  USHORT before = 0, after = 0;
  FLOAT OH_lo = 0, OH_hi = 0;
  SHORT lead_inside = 0, lead_outside = 0;
#endif

  if (fake_detector)
    return;

  cacheoff();

  lc_tbl_index = setup_for_asic_patch(&lc_tbl_addrs);
  DAC_seq_count = monitor_addr->dac_table_index;

  for (i = 0;i < lc_tbl_index && i < LC_TABLE_LIMIT; i++)
    {
    switch(lc_tbl_addrs[i].processor)
      {
      case FC_PROC:
        value = read_FC_counter(lc_tbl_addrs[i].offset);
      break;
      case DC_PROC:
        value = read_DC_counter(lc_tbl_addrs[i].offset);
      break;
      case DAC_PROC:
      break;
      }
    if ((lc_tbl_addrs[i].class != 0) &&
      (lc_tbl_addrs[i].processor != DAC_PROC) &&
      (value != 0))
      {
      switch (lc_tbl_addrs[i].class)
        {
        case IACTIVE:         /* Image area total. */
          if (value > 0)
            det_setup->ActiveY = value;
        break;
        case SACTIVE:         /* Serial area total. */
          if (value > 0)
            det_setup->ActiveX = value;
        break;
        case DUMXLD:          /* Dead pixels at start of serial reg. */
          det_setup->DumXLead = value;
        break;
        case DUMXTR:          /* Dead pixels at end of serial reg. */
          det_setup->DumXTrail = value;
        break;
        case DUMYLD:          /* Dead pixels at start of image reg. */
          det_setup->DumYLead = value;
        break;
        case DUMYTR:          /* Dead pixels at end of image reg. */
          det_setup->DumYTrail = value;
        break;
#ifndef _PROTECTED_MODE
        case SCNT:
          if (value > 0)
            det_setup->points = value;
        break;
        case SM_SGROUPS:
          if (value > 0)
            det_setup->points = value + 1;
        break;
        case S_GROUP_MIN1:
          if (det_setup->outputReg == 2) /* if Dual mode */
            {
            value = ((value + 1) / 2) - 1;
            }
          if (value > 0)
            det_setup->points = value+1;
        break;
        case OH_OUT:          /* OverHead outer loop. */
          OH_hi = (FLOAT)value;
          break;
        case OH_IN:           /* OverHead inter loop. */
          OH_lo = (FLOAT)value;
          break;
        case I_LEAD_INNER:   /* Image index, starting. */
          lead_inside = value;
          break;
        case I_LEAD_OUTER:   /* Image index, starting. */
          lead_outside = value;
          break;
        case S_IGNORE_START:   /* Serial index, starting. */
          if (value > 0)
            POINT_SETUP.X0[0] =
              (1 + value)              /* because X0 is 1 based */
              - det_setup->DumXLead    /* X0 doesn't count this */
                                + 1;   /* because there is one less */
                                       /* trash pixel than needed -  */
                                       /* last op before NormPix is always */
                                       /* a dummy NormPix */
          break;
        case I_GROUP:         /* Image length.            */
        case T0_COUNTER:      /* Image length.            */
          if (det_setup->outputReg == 2) /* if Dual mode */
            value *= 2;
          if (value > 0)
            {
            det_setup->tracks = value;
            TRACK_SETUP.number = value;
            }
          break;
        case Y_DELTA:         /* Image point size. */
          if (value > 0)
            TRACK_SETUP.DeltaY[0] = value;
          break;
        case X_DELTA:         /* Serial point size. */
          if (value > 0)
            POINT_SETUP.DeltaX[0] = value;
          break;
#endif
        case UT_SIMPLE:       /* Uniform tracks, simple */
          FC_mode_list[IMG_SIMPLE] = lc_tbl_addrs[i].offset;
        break;
        case UT_CLASSIC:      /* Uniform tracks, classic */
          FC_mode_list[IMG_CLASSIC] = lc_tbl_addrs[i].offset;
        break;
        case RT_ALL:          /* Random tracks, simple and classic */
/*        FC_mode_list[IMG_RANDOM] = lc_tbl_addrs[i].offset;  Assume $4000 now */
        break;
        case USSLICE_OUT_MX:  /* Uniform points, simple */
          FC_mode_list[SER_UNI_MXS] = lc_tbl_addrs[i].offset;
        break;
        case USSLICE_OUT_DX:  /* Uniform points, simple */
          FC_mode_list[SER_UNI_DXS] = lc_tbl_addrs[i].offset;
        break;
        case UCSLICE_OUT_MX:  /* Uniform points, classic */
          FC_mode_list[SER_UNI_MXC] = lc_tbl_addrs[i].offset;
        break;
        case UCSLICE_OUT_DX:  /* Uniform points, classic */
          FC_mode_list[SER_UNI_DXC] = lc_tbl_addrs[i].offset;
        break;
        case RSLICE_OUT:      /* Random points, simple and classic */
          FC_mode_list[SER_RAN_ALL] = lc_tbl_addrs[i].offset;
        break;
        case SER_CLEAN:       /* Clean shift register */
          FC_mode_list[FC_CLR_SER] = lc_tbl_addrs[i].offset;
        break;
        case FC_OH:
          FC_mode_list[FC_NORM_OH] = lc_tbl_addrs[i].offset;
        break;
        case FC_OHNS:
          FC_mode_list[FC_NOSHUT_OH] = lc_tbl_addrs[i].offset;
        break;
        case FC_OHNOET:
          FC_mode_list[FC_NOET_OH] = lc_tbl_addrs[i].offset;
        break;
        case FC_OHLCI:
          FC_mode_list[FC_LCI_OH] = lc_tbl_addrs[i].offset;
        break;
        case FC_OHLCINP:
          FC_mode_list[FC_LCINP_OH] = lc_tbl_addrs[i].offset;
        break;
        case STRK:
          FC_mode_list[DAC_STRK_NORM] = lc_tbl_addrs[i].offset;
        break;
        case STRK_OT:
          FC_mode_list[DAC_STRK_OT] = lc_tbl_addrs[i].offset;
        break;
        case STRK_ET:
          FC_mode_list[DAC_STRK_NORM_ET] = lc_tbl_addrs[i].offset;
        break;
        case STRK_OT_ET:
          FC_mode_list[DAC_STRK_OT_ET] = lc_tbl_addrs[i].offset;
        break;
        case STRK_SYNC:
          FC_mode_list[DAC_STRK_NORM_SYNC] = lc_tbl_addrs[i].offset;
        break;
        case STRK_OT_SYNC:
          FC_mode_list[DAC_STRK_OT_SYNC] = lc_tbl_addrs[i].offset;
        break;
        case STRK_ET_SYNC:
          FC_mode_list[DAC_STRK_NORM_ET_SYNC] = lc_tbl_addrs[i].offset;
        break;
        case STRK_OT_ET_SYNC:
          FC_mode_list[DAC_STRK_OT_ET_SYNC] = lc_tbl_addrs[i].offset;
        break;
        case STRK_LCI:
          FC_mode_list[DAC_STRK_LCI] = lc_tbl_addrs[i].offset;
        break;
        case STRK_LCI_CW:
          FC_mode_list[DAC_STRK_LCI_CW] = lc_tbl_addrs[i].offset;
        break;
        case DC_MODE_ADD:
          DC_mode_list[DCADD] = lc_tbl_addrs[i].offset;
        break;
        case DC_MODE_REP:
          DC_mode_list[DCREP] = lc_tbl_addrs[i].offset;
        break;
        case DC_MODE_SUB:
          DC_mode_list[DCSUB] = lc_tbl_addrs[i].offset;
        break;
        case DC_MODE_IGN:
          DC_mode_list[DCIGN] = lc_tbl_addrs[i].offset;
        break;
        case DC_OH:
          DC_mode_list[DCOH] = lc_tbl_addrs[i].offset;
        break;
        case DC_OHNE:
          DC_mode_list[DCOHNE] = lc_tbl_addrs[i].offset;
        break;
        case DC_OHLCI:
          DC_mode_list[DCOHTP] = lc_tbl_addrs[i].offset;
        break;
#ifndef _PROTECTED_MODE
        case PIXTIME_NP:  /* uses 'A' pixel timing only */
          {
          ULONG pixcode = (ULONG)value &
                                  (ops->dNPA | ops->dNPAS | ops->dNPAF);
          
          if (pixcode == ops->dNPA)
            det_setup->pix_time_index = 0;
          else if (pixcode == ops->dNPAF)
            det_setup->pix_time_index = 1;
          else if (pixcode == ops->dNPAS)
            det_setup->pix_time_index = 2;
          }
        break;
        case BIAS_AB:
          value &= ((USHORT)B_CHANNEL | (USHORT)D_CHANNEL);
          det_setup->outputReg = (value >> 1);/* 0, 2 or 4 -> 0, 1, or 2 */
        break;
        case APROC_AB:
          value &= (USHORT)ACHANNEL;
          det_setup->outputReg = (value != 0);
        break;
        case TEMP_TAG:        /* all temp settings are >0, ignore lo bit */
                              /* and offset by 10 */
          det_setup->detector_temp = -(SHORT)(((value & 0x00FE) - 10));
//          det_setup->detector_temp = (SHORT)( -((value & 0x00FE) - 10) );
          det_setup->cooler_locked = 
          (det_setup->cooler_locked & 0xfd) |
          ( (value & 1 ) << 1 ) ;     /* set cooler on/off bit */
        break;
        case SHUTTER_BEF:
          before = (value & ( bits_open | bits_close));
        break;
        case SHUTTER_AFT:
          after = (value & ( bits_open | bits_close));
        break;
        case SYNC_OPEN:
          det_setup->shutter_open_sync_index = (value & (SHORT)conds->dWFS) != 0;
        break;
        case SYNC_CLOSE:
          det_setup->shutter_close_sync_index = (value & (SHORT)conds->dWFS) != 0;
        break;
        case SCITC_SET:
         {
         SHORT l;
         value &= detSCMP;
         for (l = 0;l < NUMSCOPTS; l++)
            if (scomp_values[l] == value)
               det_setup->source_comp_index = l;
         }
        break;
        case INEX_SHUTTER:
         det_setup->control_index = ((value & 1) == 0);
        break;
#endif
        default:              
          break;
        }
      }
    }
  FC_mode_list[IMG_RANDOM] = 0x4000; /* allow only 16383 bytes for  */
                                     /* other ASIC stuff - temp fix */
                                     /* since LOADER ignores ORG    */
                                     /* instruction                 */
                                   
#ifndef _PROTECTED_MODE
  if (OH_lo && OH_hi)
    det_setup->exposure_time = (OH_hi * OH_lo) * no_op_time;

  if (before != 0 && after != 0)
    {
    if ((before == bits_open) & (after == bits_open))
      det_setup->shutter_forced_mode = FORCED_OPEN;
    else if ((before == bits_close) & (after == bits_close))
      det_setup->shutter_forced_mode = FORCED_CLOSE;
    else if ((before == bits_open) & (after == bits_close))
      det_setup->shutter_forced_mode = NORMAL_SHUTTER;
    }

  if (lead_inside && lead_outside )
    {
    USHORT image_start;
    FLOAT ftemp;

    if (det_setup->shiftmode == 0)
      {
      image_start = TRACK_SETUP.Y0[0];
      image_start = (lead_inside * lead_outside);
      }
    else
      {
      image_start = POINT_SETUP.X0[0];
      image_start = (lead_inside * lead_outside);
      }
    image_start -= det_setup->DumYLead;

    if (det_setup->outputReg == 2)                /* if Dual mode */
      image_start *= 2;

    if (det_setup->shiftmode == 0)
       TRACK_SETUP.Y0[0] = image_start;
    else
       POINT_SETUP.X0[0] = image_start;

    ftemp = (FLOAT)(lead_inside * lead_outside);
    ftemp = ftemp * 100.0F;
    ftemp = ftemp / (FLOAT)lead_outside;
    det_setup->anti_bloom_percent = (SHORT) ftemp;
    }
#endif
  cacheon();
#ifndef _PROTECTED_MODE
  if (det_setup->detector_type_index == TSM_CCD)
    det_setup->points = get_asic_counter(SM_SGROUPS) + 1;
#endif
  release_OMA_and_init();

}

/***************************************************************/
/*                                                             */
/* Initialize det_setup fields which have corresponding values */
/* in the Monitor Shared Memory Construct.  It's possible for  */
/* this area to get messed up by programs which access extended*/
/* memory (including THIS one!) so a lot of checking must be   */
/* done on the values stored here.  On the other hand, if the  */
/* values left are good, it's convenient to be able to come    */
/* back to the same setup you left as much as possible.        */
/*                                                             */
/***************************************************************/

void update_from_shared_memory(void)
{
  ULONG size;
  
  if (fake_detector)
    return;

#ifndef _PROTECTED_MODE
  /* get value of TCNTR, =#tracks */
  {
  SHORT i =
    get_DAC_counter(H0_COUNTER);                /* if a legal value stored */
  if (i >= 0) det_setup->prep_frames = i;       /* in shared memory, use */
  i = get_DAC_counter(K0_COUNTER);              /* it, else the method val */
  if (i >= 0) det_setup->ignored_scans = i;     /* is used */
  i = get_DAC_counter(I0_COUNTER);
  if (i > 0) det_setup->scans  = i;
  i = get_DAC_counter(J0_COUNTER);
  if (i > 0 ) det_setup->memories = i;
  i = get_DAC_counter(T0_COUNTER);
  if (det_setup->outputReg == 2) i *= 2;        /* if Dual mode */
  if (i > 0)
    det_setup->tracks = TRACK_SETUP.number = i;
  i = get_DAC_counter(SAME_ET);
  det_setup->same_et = 0;
  if (i) det_setup->same_et |= 1;
  }
#endif
  size = ((ULONG)det_setup->tracks *
          (ULONG)det_setup->ActiveX * 4L); /* bytes/point */
  det_setup->data_word_size = 1;           /* indicate "double precision" */
  set_DAC_pointer(SIZE_PTR, size);
  if (size)
   det_setup->max_memory = (SHORT)(det_setup->memory_size / size) -
                           (SHORT)(det_setup->detector_type_index == RAPDA);
  
  size = sizeof(SHARED_COMM_AREA) - (sizeof(SHARED_COMM_AREA) % 4) + 4;
  size += 4L * ((ULONG)det_setup->memories + 1L);
  set_DAC_pointer(FRAME_PTR, size);

  det_setup->da_active = FALSE; /* may be bad assumption */
  
  cacheoff(); /* no get_DAC_counter provision for following stuff */

#ifndef _PROTECTED_MODE
  read_high_memory(CommonArea, 0L, sizeof(SHARED_COMM_AREA));

  /* point and track modes (uniform or random) are stored as 1 or 2 */
  /* in monitor so 0 or number > 3 means mode value is unitialized */
  /* mode value is stored as 0 or 1 in det_setup */

  if ((SHORT)CommonArea->MSM_Serial_Mode > 0
      && (SHORT)CommonArea->MSM_Serial_Mode < 3) 
    det_setup->pointmode = CommonArea->MSM_Serial_Mode - 1;
  else
#endif
    CommonArea->MSM_Serial_Mode = det_setup->pointmode + 1;

#ifndef _PROTECTED_MODE
  if ((SHORT)CommonArea->MSM_Image_Mode > 0
      && (SHORT)CommonArea->MSM_Image_Mode < 3)
    det_setup->trackmode = (SHORT)CommonArea->MSM_Image_Mode - 1;
  else
#endif
    CommonArea->MSM_Image_Mode = (USHORT)det_setup->trackmode + 1;

  /* more sanity checks */
  
  if (det_setup->points > MAX_POINT_COUNT)
    det_setup->points = MAX_POINT_COUNT;

  if (det_setup->tracks > MAX_TRACK_COUNT)
    det_setup->tracks = MAX_TRACK_COUNT;

  CommonArea->MSM_DC_Add_Mode = DC_mode_list[DCADD];
  CommonArea->MSM_DC_Rep_Mode = DC_mode_list[DCREP];
  CommonArea->MSM_DC_Sub_Mode = DC_mode_list[DCSUB];
  CommonArea->MSM_DC_Ign_Mode = DC_mode_list[DCIGN];
  CommonArea->MSM_DC_OH       = DC_mode_list[DCOH];

#ifndef _PROTECTED_MODE
  write_high_memory(CommonArea, 0L, sizeof(SHARED_COMM_AREA));
#endif

  cacheon();
}


/*********************************************************************/
/*                                                                   */
/* Get monitor version string from OMA4 board as a C string          */
/*                                                                   */
/*********************************************************************/
void get_monitor_string(CHAR * compare_buffer, SHORT len)
{
  SHORT i;
  
  if (fake_detector)
    return;

  wait_off();
  map_program_memory();
  cacheoff();

#ifndef _PROTECTED_MODE
  read_high_memory(monitor_addr, DAC_MEM_OFFSET, sizeof(MONITOR_PTRS));
#endif

  /* WATCOM didn't handle i++ in first assignment correctly */

  for (i = 0;i < len; i++) /* swap bytes from OMA4 board to Intel order */
    {
    compare_buffer[i+1] = monitor_addr->version_string[i];
    i++;
    compare_buffer[i-1] = monitor_addr->version_string[i];
    }
  cacheon();
  release_OMA_and_init();
  wait_on();
}

/*********************************************************************/
/*                                                                   */
/* Erase shared Comm area (forcing reload of vals from method)       */
/*                                                                   */
/*********************************************************************/
void erase_comm_area(void)
{
  if (fake_detector)
    return;

  release_OMA_and_init();
  _fmemset(CommonArea, 0, sizeof(SHARED_COMM_AREA));

#ifndef _PROTECTED_MODE
  write_high_memory(CommonArea, 0L, sizeof(SHARED_COMM_AREA));
#endif
}

/***************************************************************************
/
/ Random point & track stuff
/
***************************************************************************/

void emit_byte(USHORT *pcode, UCHAR bval)
{
#ifdef _PROTECTED_MODE
  USHORT __far * fcptr = (USHORT __far *)&(((CHAR *)CommonArea)[*pcode * 2]);
  USHORT contents = * fcptr;
  contents = (contents & 0xFF00) | (USHORT)bval;
  *fcptr = contents;
#else
  ULONG fcptr = *pcode * 2;
  USHORT contents;
  read_high_memory((void __far *)(&contents), fcptr, sizeof(USHORT));
  contents = (contents & 0xFF00) | (USHORT)bval;
  write_high_memory((void __far *)(&contents), fcptr, sizeof(USHORT));
#endif

  *pcode += 1;
}

void emit_word(USHORT *pcode, USHORT wval)
{
  update_FC_counter(wval, *pcode);
  *pcode += 2;
}

void emit_long(USHORT *pcode, ULONG lval) /* long here means 3 bytes */
{
   USHORT loword = (USHORT)lval & 0xFFFF;
   UCHAR hibyte = (UCHAR)((ULONG)((lval & 0xFFFFFF) >> 16));
   emit_word(pcode, loword);
   emit_byte(pcode, hibyte);
}

static void skip_ser_block(USHORT *pcode_addr, USHORT skipchn)
{
  USHORT looptop;
  if (skipchn > 2)
    {
    emit_byte(pcode_addr, fcLDCNTRA); /* set up loop top */
    emit_word(pcode_addr, skipchn);     /* loop counter for skips */
    looptop = *pcode_addr;            /* present address to loop to */
    }
  if (skipchn > 1)
    {
    emit_byte(pcode_addr, fcWAIT);    /* wait for detector to be ready */
    emit_byte(pcode_addr, fcLOAD);    /* load a trash pixel opcode */
    emit_long(pcode_addr, ops->dTP | conds->dRS_B);
    }

  if (skipchn > 2)
    {
    emit_byte(pcode_addr, fcJUNTILA); /* and do loop till done */
    emit_word(pcode_addr, looptop);
    }

  if (skipchn > 0)
    {
    if (det_setup->detector_type_index == TSM_CCD)
      {
      emit_byte(pcode_addr, fcWAIT);    /* wait for detector to be ready */
      emit_byte(pcode_addr, fcLOAD);    /* put in transition after trashes */
      emit_long(pcode_addr, ops->dNPAF);

      emit_byte(pcode_addr, fcWAIT);    /* wait for detector to be ready */
      emit_byte(pcode_addr, fcLOAD);    /* put in transition after trashes */
      emit_long(pcode_addr, ops->dNPAF);
      }
    else
      {
      emit_byte(pcode_addr, fcWAIT);    /* wait for detector to be ready */
      emit_byte(pcode_addr, fcLOAD);    /* put in dummy read after trashes */
      emit_long(pcode_addr, ops->dNPA);
      }
    }
}

static void read_ser_block(USHORT *pcode_addr, USHORT deltaX, ULONG pixact)
{
  USHORT looptop;

  if (deltaX > 1)
    {
    emit_byte(pcode_addr, fcLDCNTRA); /* set up loop top */
    emit_word(pcode_addr, deltaX-1);  /* loop counter for deltaX */
    looptop = *pcode_addr;            /* present address to loop to */


    if (det_setup->detector_type_index == EEV_CCD ||
        det_setup->detector_type_index == TS_1KSQ)
      {
      emit_byte(pcode_addr, fcWAIT);    /* for detectors with no summing */
      emit_long(pcode_addr, ops->dAO);  /* node, do acquire offset */
      }

    emit_byte(pcode_addr, fcWAIT);    /* wait for detector to be ready */
    emit_byte(pcode_addr, fcLOAD);    /* load a dump pixel opcode */
    emit_long(pcode_addr, ops->dDPX);

    emit_byte(pcode_addr, fcJUNTILA); /* and do loop till done */
    emit_word(pcode_addr, looptop);
    }
  emit_byte(pcode_addr, fcWAIT);      /* wait for detector to be ready */
  emit_byte(pcode_addr, fcLOAD);      /* load the read pixel opcode */

  if ((det_setup->detector_type_index == EEV_CCD ||
        det_setup->detector_type_index == TS_1KSQ) &&
        (deltaX > 1))
    emit_long(pcode_addr, ops->dASA);
  else
    emit_long(pcode_addr, pixact);
}

static void read_dual_ser_block(USHORT *pcode_addr, USHORT deltaX,
                                ULONG pixact1, ULONG pixact2)
{
  USHORT looptop;

  if (deltaX > 1)
    {
    /* the dual register output requires a delay between normal pixels */
    /* and fast operations such as dump pixels.  the delay must be equal*/
    /* to two 'normal' (i.e. 5usec) operations. if deltax is big enough,*/
    /* slow dump pixels can be used for the delay.  otherwise, a flush */
    /* instruction is inserted to provide one of the 5 usec instructions */

    if (deltaX < 3)                   /* if dX too small */
      {
      emit_byte(pcode_addr, fcWAIT);    /* wait for detector to be ready */
      emit_byte(pcode_addr, fcLOAD);    /* load a flush opcode */
      emit_long(pcode_addr, ops->FLUSH);
      }

    emit_byte(pcode_addr, fcWAIT);    /* wait for detector to be ready */
    emit_byte(pcode_addr, fcLOAD);    /* load a dump pixel opcode */
    emit_long(pcode_addr, ops->dDPXS);
    
    if (deltaX > 2)
      {
      emit_byte(pcode_addr, fcWAIT);    /* wait for detector to be ready */
      emit_byte(pcode_addr, fcLOAD);    /* load a dump pixel opcode */
      emit_long(pcode_addr, ops->dDPXS);
      
      if (deltaX > 3)
        {
        emit_byte(pcode_addr, fcLDCNTRA); /* set up loop top */
        emit_word(pcode_addr, deltaX-1);  /* loop counter for deltaX */

        looptop = *pcode_addr;            /* present address to loop to */
    
        emit_byte(pcode_addr, fcWAIT);    /* wait for detector to be ready */
        emit_byte(pcode_addr, fcLOAD);    /* load a dump pixel opcode */
        emit_long(pcode_addr, ops->dDPX);

        emit_byte(pcode_addr, fcJUNTILA); /* and do loop till done */
        emit_word(pcode_addr, looptop);
        }
      }
    }
  emit_byte(pcode_addr, fcWAIT);      /* wait for detector to be ready */
  emit_byte(pcode_addr, fcLOAD);      /* load the read pixel opcode */
  emit_long(pcode_addr, pixact1);

  emit_byte(pcode_addr, fcWAIT);      /* wait for detector to be ready */
  emit_byte(pcode_addr, fcLOAD);      /* load the read pixel opcode */
  emit_long(pcode_addr, pixact2);
}


/* random setup for serial readout (points or tracks) */

SHORT rserial_setup(SHORT speed)
{
  USHORT i, code_offset;            /* offset is current address in ASIC */
  SHORT err = NO_ERROR;
  ULONG pixcode = ops->dNPA, pixcode2 = ops->dNPB;
  
  if (speed == 1)
    {
    pixcode = ops->dNPAF;
    pixcode2 = ops->dNPBF;
    }
  else if (speed == 2)
    pixcode = ops->dNPAS;    /* no slow pixels for dual channel */


  /* Look for start address */

  if ((code_offset = FC_mode_list[SER_RAN_ALL]) == 0) /* get address for code*/
    {
    err = ERROR_NO_CODE_SPACE;              /* no address found at init */
    }
  else
    {
    map_program_memory(); /* turn on program memory */

    if (det_setup->shiftmode == 0) /* simple mode */
      {
      /* do initial skip of dead pixels and others up to X0 */

      skip_ser_block(&code_offset, det_setup->DumXLead + POINT_SETUP.X0[1]);

      /* do initial read of first group and reset the pixel pointer */
      /* also set the B trigger out, will be cleared by track routine */
      /* subsequent groups will increment the pixel pointer */

      if (det_setup->detector_type_index == TSM_CCD) /* dual channel detect */
        {
        if (det_setup->outputReg == 2) /* Dual channel output */
          {
          read_dual_ser_block(&code_offset, POINT_SETUP.DeltaX[1],
                      pixcode  | conds->dCUE | adcalc->dRPP | conds->dSET_B,
                      pixcode2 | conds->dCUE | adcalc->dLPP | conds->dSET_B);
          }
        else  if (det_setup->outputReg == 1) /* Single channel output from B*/
          {
          read_dual_ser_block(&code_offset, POINT_SETUP.DeltaX[1],
                      pixcode  | adcalc->dRPP | conds->dSET_B,
                      pixcode2 | conds->dCUE | adcalc->dLPP | conds->dSET_B);
          }
        else                                 /* Single channel output from A*/
          {
          read_dual_ser_block(&code_offset, POINT_SETUP.DeltaX[1],
                      pixcode  | conds->dCUE |adcalc->dRPP | conds->dSET_B,
                      pixcode2 | adcalc->dLPP | conds->dSET_B);
          }
        }
      else                           /* single channel detector */
        {
        read_ser_block(&code_offset, POINT_SETUP.DeltaX[1],
                    pixcode | conds->dCUE | adcalc->dRPP | conds->dSET_B);
        }

      for (i = 2;i < (USHORT)det_setup->points+1; i++)
        {
        USHORT skipchn;

        skipchn = POINT_SETUP.X0[i] - POINT_SETUP.X0[i-1] -
                  POINT_SETUP.DeltaX[i-1];
        skip_ser_block(&code_offset, skipchn);

        if (det_setup->detector_type_index == TSM_CCD) /* 2 channel detector*/
          {
          if (det_setup->outputReg == 2) /* Dual channel output */
            {
            read_dual_ser_block(&code_offset, POINT_SETUP.DeltaX[i],
                        pixcode  | conds->dCUE | adcalc->dIPP | conds->dSET_B,
                        pixcode2 | conds->dCUE | adcalc->dDPP | conds->dSET_B);
            }
          else  if (det_setup->outputReg == 1) /* Single channel output from B*/
            {
            read_dual_ser_block(&code_offset, POINT_SETUP.DeltaX[i],
                        pixcode  | adcalc->dIPP | conds->dSET_B,
                        pixcode2 | conds->dCUE | adcalc->dDPP | conds->dSET_B);
            }
          else                                 /* Single channel output from A*/
            {
            read_dual_ser_block(&code_offset, POINT_SETUP.DeltaX[i],
                        pixcode  | conds->dCUE |adcalc->dIPP | conds->dSET_B,
                        pixcode2 | adcalc->dDPP | conds->dSET_B);
            }
          }
        else                           /* single channel detector */
          {
          read_ser_block(&code_offset, POINT_SETUP.DeltaX[i],
                      pixcode | conds->dCUE | adcalc->dIPP | conds->dSET_B);
          }

        }

      if (det_setup->detector_type_index == TSM_CCD) /* 2 channel detector */
        {
        emit_byte(&code_offset, fcWAIT);      /* wait for detector to be ready */
        emit_byte(&code_offset, fcLOAD);      /* load the read pixel opcode */
        emit_long(&code_offset, ops->dTPS); /* ITP done in image shift routine */


        emit_byte(&code_offset, fcWAIT);      /* wait for detector to be ready */
        emit_byte(&code_offset, fcLOAD);      /* load the read pixel opcode */
        emit_long(&code_offset, ops->dTPS | adcalc->dDTP);
        }
      
      /* do final skip of dead pixels and others to the end of serial reg. */
      skip_ser_block(&code_offset, det_setup->DumXTrail +
                                (det_setup->ActiveX -
                                (POINT_SETUP.X0[i])));

      emit_byte(&code_offset, fcRTS);
      }
    else /* classic mode */
      {
      /* do initial skip of dead pixels and others up to X0 */

      skip_ser_block(&code_offset, det_setup->DumXLead + TRACK_SETUP.Y0[1]);

      if (det_setup->detector_type_index == TSM_CCD) /* 2 channel detector */
        {
        if (det_setup->outputReg == 2) /* Dual channel output */
          {
          read_dual_ser_block(&code_offset, TRACK_SETUP.DeltaY[1],
                      pixcode  | conds->dCUE | adcalc->dRTP | conds->dSET_B,
                      pixcode2 | conds->dCUE | adcalc->dLTP | conds->dSET_B);
          }
        else  if (det_setup->outputReg == 1) /* Single channel output from B*/
          {
          read_dual_ser_block(&code_offset, TRACK_SETUP.DeltaY[1],
                      pixcode  | adcalc->dRTP | conds->dSET_B,
                      pixcode2 | conds->dCUE | adcalc->dLTP | conds->dSET_B);
          }
        else                                 /* Single channel output from A*/
          {
          read_dual_ser_block(&code_offset, TRACK_SETUP.DeltaY[1],
                      pixcode  | conds->dCUE |adcalc->dRTP | conds->dSET_B,
                      pixcode2 | adcalc->dLTP | conds->dSET_B);
          }
        }
      else                           /* single channel detector */
        {
        read_ser_block(&code_offset, TRACK_SETUP.DeltaY[1],
                    pixcode | conds->dCUE | adcalc->dRTP | conds->dSET_B);
        }
      for (i = 2;i < (USHORT)det_setup->tracks+1; i++)
        {
        USHORT skipchn;

        skipchn = TRACK_SETUP.Y0[i] - TRACK_SETUP.Y0[i-1] -
                  TRACK_SETUP.DeltaY[i-1];
        skip_ser_block(&code_offset, skipchn);

        if (det_setup->detector_type_index == TSM_CCD) /* 2 channel detector*/
          {
          if (det_setup->outputReg == 2) /* Dual channel output */
            {
            read_dual_ser_block(&code_offset, TRACK_SETUP.DeltaY[i],
                        pixcode  | conds->dCUE | adcalc->dITP | conds->dSET_B,
                        pixcode2 | conds->dCUE | adcalc->dDTP | conds->dSET_B);
            }
          else  if (det_setup->outputReg == 1) /* Single channel output from B*/
            {
            read_dual_ser_block(&code_offset, TRACK_SETUP.DeltaY[i],
                        pixcode  | adcalc->dITP | conds->dSET_B,
                        pixcode2 | conds->dCUE | adcalc->dDTP | conds->dSET_B);
            }
          else                                 /* Single channel output from A*/
            {
            read_dual_ser_block(&code_offset, TRACK_SETUP.DeltaY[i],
                        pixcode  | conds->dCUE |adcalc->dITP | conds->dSET_B,
                        pixcode2 | adcalc->dDTP | conds->dSET_B);
            }
          }
        else                           /* single channel detector */
          {
          read_ser_block(&code_offset, TRACK_SETUP.DeltaY[i],
                      pixcode | conds->dCUE | adcalc->dITP | conds->dSET_B);
          }

        }

      if (det_setup->detector_type_index == TSM_CCD)
        {
        emit_byte(&code_offset, fcWAIT);      /* wait for detector to be ready */
        emit_byte(&code_offset, fcLOAD);      /* load the read pixel opcode */
        emit_long(&code_offset, ops->dTPS ); /* IPP done in image shift routine */

        emit_byte(&code_offset, fcWAIT);      /* wait for detector to be ready */
        emit_byte(&code_offset, fcLOAD);      /* load the read pixel opcode */
        emit_long(&code_offset, ops->dTPS | adcalc->dDPP);
        }
      
      /* do final skip of dead pixels and others to the end of serial reg. */
      skip_ser_block(&code_offset, det_setup->DumXTrail +
                                (det_setup->ActiveX -
                                (TRACK_SETUP.Y0[i])));

      emit_byte(&code_offset, fcRTS);
      }
    release_OMA_and_init();
    }
  return(err);
}

static void skip_img_block(USHORT *pcode_addr, USHORT skipimg, USHORT clser,
                           SHORT percent)
{
  USHORT looptop;
  USHORT inloop, outloop, remain;
  ULONG shiftop;
  
  switch(det_setup->shift_time_index)
    {
    case 0:
      shiftop = ops->dSLN;
    break;
    case 1:
      shiftop = ops->dSLF;
    break;
    case 2:
      shiftop = ops->dSLS;
    break;
    }

  if (skipimg > 1)
    {
    remain = calc_antibloom_counts(percent, skipimg, &inloop, &outloop);

    if (inloop < 1 || outloop < 2)
      {
      
      emit_byte(pcode_addr, fcLDCNTRA); /* set up loop top */
      emit_word(pcode_addr, skipimg);   /* loop counter for skips */
      looptop = *pcode_addr;            /* present address to loop to */
      emit_byte(pcode_addr, fcWAIT);    /* wait for detector to be ready */
      emit_byte(pcode_addr, fcLOAD);    /* load a shift line opcode */
      emit_long(pcode_addr, shiftop);
      emit_byte(pcode_addr, fcJUNTILA); /* and do loop till done */
      emit_word(pcode_addr, looptop);
      emit_byte(pcode_addr, fcGOSUB);   /* go to subroutine to clear sreg */
      emit_word(pcode_addr, clser);     /* write the gosub address */
      }
    else
      {
      SHORT inlooptop;

      emit_byte(pcode_addr, fcLDCNTRA); /* set up loop counter */
      emit_word(pcode_addr, outloop-1); /* loop counter for skips */
      looptop = *pcode_addr;            /* present address to loop to */
      emit_byte(pcode_addr, fcLDCNTRB); /* set up loop top */
      emit_word(pcode_addr, inloop);    /* loop counter for skips */
      inlooptop = *pcode_addr;

      emit_byte(pcode_addr, fcWAIT);    /* wait for detector to be ready */
      emit_byte(pcode_addr, fcLOAD);    /* load a shift line opcode */
      emit_long(pcode_addr, shiftop);
      emit_byte(pcode_addr, fcJUNTILB); /* and do loop till done */
      emit_word(pcode_addr, inlooptop);
      emit_byte(pcode_addr, fcGOSUB);   /* go to subroutine to clear sreg */
      emit_word(pcode_addr, clser);     /* write the gosub address */
      emit_byte(pcode_addr, fcJUNTILA); /* and do loop till done */
      emit_word(pcode_addr, looptop);

      if (remain > 1)
        {
        emit_byte(pcode_addr, fcLDCNTRA); /* set up loop counter */
        emit_word(pcode_addr, remain-1);  /* loop counter for skips */
        looptop = *pcode_addr;            /* present address to loop to */
        emit_byte(pcode_addr, fcWAIT);    /* wait for detector to be ready */
        emit_byte(pcode_addr, fcLOAD);    /* load a shift line opcode */
        emit_long(pcode_addr, shiftop);
        emit_byte(pcode_addr, fcGOSUB);   /* go to subroutine to clear sreg */
        emit_word(pcode_addr, clser);     /* write the gosub address */
        emit_byte(pcode_addr, fcJUNTILA); /* and do loop till done */
        emit_word(pcode_addr, looptop);
        }
      }
    }
  else
    {
    if (skipimg > 0)
      {
      emit_byte(pcode_addr, fcWAIT);    /* wait for detector to be ready */
      emit_byte(pcode_addr, fcLOAD);    /* load a shift line opcode */
      emit_long(pcode_addr, shiftop);
      emit_byte(pcode_addr, fcGOSUB);   /* go to subroutine to clear sreg */
      emit_word(pcode_addr, clser);     /* write the gosub address */
      }
    }
}

static void read_img_block(USHORT *pcode_addr, USHORT deltaY, USHORT rdser)
{
  USHORT looptop;
  ULONG shiftop;
  
  switch(det_setup->shift_time_index)
    {
    case 0:
      shiftop = ops->dSLN;
    break;
    case 1:
      shiftop = ops->dSLF;
    break;
    case 2:
      shiftop = ops->dSLS;
    break;
    }

  if (deltaY > 1)
    {
    emit_byte(pcode_addr, fcLDCNTRA); /* set up loop top */
    emit_word(pcode_addr, deltaY);    /* loop counter for deltaY */
    looptop = *pcode_addr;            /* present address to loop to */
    }

  emit_byte(pcode_addr, fcWAIT);      /* wait for detector to be ready */
  emit_byte(pcode_addr, fcLOAD);      /* load a shift line opcode */
  emit_long(pcode_addr, shiftop);
    
  if (deltaY > 1)
    {
    emit_byte(pcode_addr, fcJUNTILA); /* and do loop till done */
    emit_word(pcode_addr, looptop);
    }

  emit_byte(pcode_addr, fcGOSUB);      /* go to subroutine to read serial reg */
  emit_word(pcode_addr, rdser);        /* write the gosub address */

  emit_byte(pcode_addr, fcWAIT);       /* wait for detector to be ready */
  emit_byte(pcode_addr, fcLOAD);       /* cause the track pointer to increment */

  if (det_setup->shiftmode == 0)
    emit_long(pcode_addr, ops->dNO_OP | adcalc->dITP | conds->dCUE);
  else
    emit_long(pcode_addr, ops->dNO_OP | adcalc->dIPP | conds->dCUE);
}

/* random setup for image readout (points or tracks) */
SHORT rimage_setup(SHORT antibloom_percent)
{
  USHORT i, lines, code_offset;   /* offset is current address in ASIC */
  SHORT err = NO_ERROR;
  USHORT rdserial_address = 0;    /* loc. of code to read serial register */
  USHORT clserial_address = 0;    /* loc. of code to clean serial register */

  if (det_setup->shiftmode == 0)  /* simple orientation */
    lines = det_setup->tracks;
  else                            /* classic orientation */
    lines = det_setup->points;

  if (det_setup->outputReg == 2)  /* if Dual register output */
    lines = lines / 2;            /* tracks are two for one! */

  lines++;

  /* Look for addresses */

  code_offset  = FC_mode_list[IMG_RANDOM]; /* get address for code output */
  if (det_setup->shiftmode == 0)           /* simple shiftreg? */
    {
    if (det_setup->pointmode == 0)         /* uniform point mode? */
      {
      if (POINT_SETUP.DeltaX[0] > 1)
        rdserial_address = FC_mode_list[SER_UNI_DXS]; /* uniform serial, simple*/
      else
        rdserial_address = FC_mode_list[SER_UNI_MXS]; /* uniform serial, simple*/
      }
    else                                  /* no, random mode */
      rdserial_address = FC_mode_list[SER_RAN_ALL]; /* use random serial address */
    }
  else                                    /* not simple, then classic mode */
    {
    if (det_setup->trackmode == 0)        /* uniform track mode? */
      {
      if (TRACK_SETUP.DeltaY[0] > 1)
        rdserial_address = FC_mode_list[SER_UNI_DXC]; /* uniform serial, classic */
      else
        rdserial_address = FC_mode_list[SER_UNI_MXC]; /* uniform serial, classic */
      }
    else                                  /* else random mode */
      rdserial_address = FC_mode_list[SER_RAN_ALL]; /* use random serial address */
    }

  clserial_address = FC_mode_list[FC_CLR_SER];

  if (code_offset == 0)               /* no address found at init */
    err = ERROR_NO_CODE_SPACE;
  else if (rdserial_address == 0)     /* no address found at init */
    err = ERROR_NO_SERIAL_CODE;
  else if (clserial_address == 0)     /* no address found at init */
    err = ERROR_NO_CLEAN_CODE;
  else                                /* All OK so far */
    {
    map_program_memory(); /* turn on program memory */

    /* do initial read of first group */
    if (det_setup->shiftmode == 0)  /* simple mode */
      {
      read_img_block(&code_offset, TRACK_SETUP.DeltaY[1], rdserial_address);
      for (i = 2;i < (USHORT)lines; i++)
        {
        USHORT skipimg;
        skipimg = (TRACK_SETUP.Y0[i] - TRACK_SETUP.Y0[i-1] -
                  TRACK_SETUP.DeltaY[i-1]);
        skip_img_block(&code_offset,skipimg,clserial_address,antibloom_percent);
        read_img_block(&code_offset, TRACK_SETUP.DeltaY[i], rdserial_address);
        }
      emit_byte(&code_offset, fcEND);
      }
    else /* classic mode */
      {
      read_img_block(&code_offset, POINT_SETUP.DeltaX[1], rdserial_address);
      for (i = 2;i < (USHORT)lines; i++)
        {
        USHORT skipimg;

        skipimg = (POINT_SETUP.X0[i] - POINT_SETUP.X0[i-1] -
                  POINT_SETUP.DeltaX[i-1]);
        skip_img_block(&code_offset,skipimg,clserial_address,antibloom_percent);
        read_img_block(&code_offset, POINT_SETUP.DeltaX[i], rdserial_address);
        }
      emit_byte(&code_offset, fcEND);
      }
    release_OMA_and_init();
    }
  return(err);
}

/***************************************************************************
*
* The RAPDA stuff; how you take the structure rapdastuff and transfer
* the parameters to the ASIC and the DAC
*
*
***************************************************************************/
/* The driver routines to handle the RAPDA. */

static FLOAT ShortET;
static LONG  ShortDelay ;
FLOAT  short_et;                  /* The shortest expose time */

SHORT SetShortDelay (FLOAT * ShortExpTime)
{
  SHORT err, i,
      NumPixels = 0,
      FirstPixel = MAX_PXL;
  ULONG clicks, delay;
  FLOAT ExpTime = det_setup->exposure_time;
  ExpTime /= (FLOAT)(1L << (LONG)RAPDA_SETUP.RapdaReg[0].n);
                                      /* Total number of A/D cycles. */
  delay = (ULONG) (ExpTime / A_D_TIME);
                                      /* Find total number of pixels, 
                                       * and number_regions * PROG_OVERHD. */
  for (i = 0, clicks = 0L; i < RAPDA_SETUP.NumRegs; i++)
    {
    NumPixels += RAPDA_SETUP.RapdaReg[i].Number;
    FirstPixel = min (FirstPixel, RAPDA_SETUP.RapdaReg[i].StartPixel);
    clicks += (ULONG)(RAPDA_SETUP.RapdaReg[i].Number + PROG_OVERHD);
    }                                 /* Add overhead per scan. */
  clicks += (ULONG)(SC_OVERHD + SHORT_ET_OVERHD);
                                      /* See if enough time allotted. */
  err = (clicks >= delay) ? NO_ERROR : SHORT_ET_ERR;
  delay = max (delay, clicks);
  ShortET = (FLOAT)delay * A_D_TIME;  /* ShortET must be multiple of A/D's */
  *ShortExpTime = ShortET;
  ShortDelay = delay - clicks;        /* ... includes overhead per event. */
  det_setup->exposure_time = ShortET *
                          (FLOAT)(1L << (LONG)RAPDA_SETUP.RapdaReg[0].n);
  det_setup->points = NumPixels;
  return (err);
}


   /* This assumes the linked list has been converted to the array which
    * is kept in decreasing "n" value, and for any one n value, in increasing
    * pixel number.  Given a group level n, it finds how many active cycles
    * will be in that level, subtracts from the total in ShortET, and
    * figures how many delay periods will be needed in a 2-deep nested loop
    * of 16 bit counters.  Note that the asic JUNTIL instruction decrements
    * before test, so it is impossible to loop zero times; therefore in the
    * table setup routines, if InCnt and OutCnt are zero, the delay part
    * is not executed at all.  Sample number of NO_OPs for InCnt, OutCnt:
    *       NO_OPs     OutCnt    InCnt
    *         0          0        0   impossible - 2^32 loops...
    *         1          1        1
    *         2          1        2
    *       65535        1      65535
    *       65536        1        0
    *       65537        2        1
    *  etc...
    */

void DelayToCntrVals (struct RegionType *Region, SHORT Index,
                      USHORT *InCnt, USHORT *OutCnt)
{
  ULONG TotalCount;
  SHORT i = 0,
      clicks = 0;                     /* Number of active A/D cycles. */
                                      /* Get to this level of "n" first. */
  while ((SHORT)Region[Index].n < (SHORT)Region[i].n)
    i++;
  while (i < RAPDA_SETUP.NumRegs)    /* Add the rest of the regions. */
    {                                 /* Add number of active cycles. */
    clicks += Region[i].Number + PROG_OVERHD;
    i++;
    }
  
  TotalCount = (ULONG)(ShortET/A_D_TIME) + 1L -
               (ULONG)(clicks + SC_OVERHD);
  *OutCnt = (USHORT) ((TotalCount - 1) / 65536L + 1L);
  *InCnt = (USHORT) (TotalCount % 65536L);
}

 /* The RAPDA tags - see scans.asc and scans.dac

  * DELAY_READ_0   EQU 150            ; The first delay read program
  * OUTER_COUNT    EQU 151            ; the outer counter delay
  * INNER_COUNT    EQU 152            ; the inner counter delay
  * READ_OFFSET    EQU 153            ; the read part of delay read.
  * DELAY_READ_1   EQU 156            ; The second delay read program
  * 
  * READ_0         EQU 157            ; The first read program
  * READ_COUNT     EQU 158            ; Number pixels to read,
  * READ_START     EQU 159            ; starting at this one.
  * READ_1         EQU 160            ; The second read program
  * 
  * DC_PGM_1       EQU 161            ; The first DC program
  * DMA_COUNT      EQU 162            ; Number pixels to read.
  * DC_PGM_2       EQU 163            ; The second DC program
  * DC_ACTION      EQU 164            ; What the DC program does.
  * 
  * DAS_RAPDA_TBL1 EQU 165            ; The delay_read program table
  * DAS_RAPDA_TBL2 EQU 166            ; the read program table.
  * 
  */

  /* Assumes setup_for_asic_patch() has been done so ASIC and DAC program
   * memory are in the host's memory map.
   */
  /* Swaps bytes before storing to comply with 68000 adressing scheme. */

SHORT StoreDAC_W (USHORT DAC_Address, USHORT Value)
{
  ULONG RealAddress = DAC_MEM_OFFSET + (ULONG)DAC_Address;
#ifdef _PROTECTED_MODE
  void __far * pm_data_ptr;

  pm_data_ptr = GetProtectedPointer(RealAddress);
  _fmemcpy(pm_data_ptr, &Value, sizeof(USHORT));
  return 0;
#else
  return (write_high_memory(&Value, RealAddress, sizeof(USHORT)));
#endif
}

SHORT StoreDAC_L (USHORT DAC_Address, ULONG Value)
{
  SHORT err;
  USHORT MotValueH = (USHORT)(Value & 0xFFFFL);
  USHORT MotValueL = (USHORT)(Value >> 16);
  err = StoreDAC_W (DAC_Address, MotValueL);
  if (!err)
    err = StoreDAC_W (DAC_Address + 2, MotValueH);
  return (err);
}

SHORT SetUpRapdaTables (void )
{
  SHORT i, Table1Index,
        Table2Index;

  USHORT AddrDelayRead0,    /* FC address of first delay read sequence */
           LengthDelayRead,   /* Length of the delay read sequence. */

           OffsetReadPart,    /* Offset to read portion of delay-read. */
           OffsetFirstPixel,  /* Offsets to first pixel and number pixels */
           OffsetNumPixels,   /* tags from start of read sequence. */

           OffsetOuterCnt,    /* Offsets to the delay outer and inner */
           OffsetInnerCnt,    /* counters from start of DC program. */

           AddrRead0,         /*   "  of the first read only sequence. */
           LengthRead,        /* Length of the read only sequence. */

           AddrDMA_Pgm0,      /* DC address of the DMA program. */
           DMA_PgmLength,     /* Length of the DMA program. */
           DMA_CountOffset,   /* Offset to PxlCnt tag from start of DC pgm.*/

           TableOneItem,      /* DAC address of item of interest, table 1. */
           TableTwoItem;      /* DAC address of item of interest, table 2. */

  struct RegionType *Region;
  counter_link_record __far * lc_tbl_addrs;

  SetShortDelay (&ShortET);
  Region = RAPDA_SETUP.RapdaReg;      /* Init Region pointer. */
  i = RAPDA_SETUP.NumRegs;           /* Init indices. */
  Table1Index = 0;
  Table2Index = 1024 / MIN_PXLS;
                                     /* Get FC addresses of tags. */
  AddrDelayRead0 = (USHORT)get_asic_offset (FC_PROC, DELAY_READ_0);
  LengthDelayRead = (USHORT)get_asic_offset (FC_PROC, DELAY_READ_1) -
                                            AddrDelayRead0;
  OffsetOuterCnt = (USHORT)get_asic_offset (FC_PROC, OUTER_COUNT) -
                                            AddrDelayRead0;
  OffsetInnerCnt = (USHORT)get_asic_offset (FC_PROC, INNER_COUNT) -
                                            AddrDelayRead0;
  OffsetReadPart = (USHORT)get_asic_offset (FC_PROC, READ_OFFSET);
  OffsetFirstPixel = (USHORT)get_asic_offset (FC_PROC, READ_START) -
                                            OffsetReadPart;
  OffsetNumPixels = (USHORT)get_asic_offset (FC_PROC, READ_COUNT) -
                                            OffsetReadPart;
  OffsetReadPart -= AddrDelayRead0;
  AddrRead0 = (USHORT)get_asic_offset (FC_PROC, READ_0);
  LengthRead = (USHORT)get_asic_offset (FC_PROC, READ_1) - AddrRead0;

                                      /* Get DC addresses of tags. */
  AddrDMA_Pgm0 = (USHORT)get_asic_offset (DC_PROC, DC_PGM_1);
  DMA_PgmLength = (USHORT)get_asic_offset (DC_PROC, DC_PGM_2) - AddrDMA_Pgm0;
  DMA_CountOffset = (USHORT)get_asic_offset (DC_PROC, DMA_COUNT) - AddrDMA_Pgm0;

                                      /* Addr of DAC table 1 index. */
  TableOneItem =  (USHORT)get_asic_offset (DAC_PROC, DAS_RAPDA_TBL1);
                                      /* Addr of DAC table 2 index. */
  TableTwoItem =  (USHORT)get_asic_offset (DAC_PROC, DAS_RAPDA_TBL2) +
                       (USHORT)(Table2Index * sizeof(struct Table2Entry));

                                      /* Get ready to read/store into ASIC.*/
  setup_for_asic_patch(&lc_tbl_addrs);
  StoreDAC_W (TableTwoItem, 0);       /* Ending condition for DAC loop. */
                                      /* Store number of events. */
  StoreDAC_L (TableOneItem, 1L << (LONG)Region[0].n);
  TableOneItem += sizeof(LONG);       /* inc to point to first real entry. */
  while ( i )
    {
    SHORT DeltaN;                       /* Change in "n" if any. */
    USHORT InCnt, OutCnt;     /* Counter values for RegionET. */
    USHORT FCAddress, DCAddress, opvalue;

    i--, Table2Index--;               /* DAC table 2 address of entry. */
    TableTwoItem -= (USHORT)sizeof (struct Table2Entry);
/*     Move Region values to Table 2 */

    FCAddress = AddrRead0 + (USHORT)Table2Index * LengthRead;
    DCAddress = AddrDMA_Pgm0 + (USHORT)Table2Index * DMA_PgmLength;
    StoreDAC_W (TableTwoItem +
                (USHORT)offsetof(struct Table2Entry, FCAddr), FCAddress);
    StoreDAC_W (TableTwoItem +
                (USHORT)offsetof(struct Table2Entry, DCAddr), DCAddress);
    StoreDAC_L (TableTwoItem +
                (USHORT)offsetof(struct Table2Entry, FBAddr),
                (ULONG)(Region[i].DataOffset * sizeof(LONG) ));

/* Now patch the ASIC for StartPixel, NumPixels. */

    opvalue = read_FC_counter(FCAddress + OffsetFirstPixel);
    opvalue &= 0xFC00;
    opvalue |= Region[i].StartPixel;
    update_FC_counter(opvalue, FCAddress + OffsetFirstPixel);
    update_FC_counter(Region[i].Number, FCAddress + OffsetNumPixels);

    update_DC_counter(Region[i].Number, DCAddress + DMA_CountOffset);
                                      /* Check if time is changing. */
    DeltaN = i ? (SHORT)(Region [i-1].n - Region[i].n) : 1;
    if ( DeltaN )                     /* Set up the delay counters. */
      DelayToCntrVals (Region, i, &InCnt, &OutCnt);
                                      /* Now set up table 1. */
    while ( DeltaN )
      {                               /* Move Region to Table 1 */
      FCAddress = AddrDelayRead0 + (USHORT)(Table1Index * LengthDelayRead);
      StoreDAC_W (TableOneItem +
                (USHORT)offsetof(struct Table1Entry, FCAddr),
                                      /* If no delays, go to read portion.*/
                                       (InCnt || OutCnt) ? FCAddress :
                                              FCAddress + OffsetReadPart);
      StoreDAC_W (TableOneItem +
                (USHORT)offsetof(struct Table1Entry, DCAddr), DCAddress);
      StoreDAC_L (TableOneItem +
                (USHORT)offsetof(struct Table1Entry, FBAddr),
                (ULONG)(Region[i].DataOffset * sizeof(LONG) ));
                                     /* Store table 2 address of next event*/
      StoreDAC_W (TableOneItem +
                (USHORT)offsetof(struct Table1Entry, Table2EntryAddr),
                (USHORT)((RAPDA_SETUP.NumRegs > 1) ?
                         TableTwoItem + sizeof (struct Table2Entry) : 0));

          /* Now patch the ASIC for Delay counters, StartPixel, NumPixels. */
      if (InCnt || OutCnt)
        {
        update_FC_counter (InCnt,  FCAddress + OffsetInnerCnt);
        update_FC_counter (OutCnt, FCAddress + OffsetOuterCnt);
        }

      FCAddress += OffsetReadPart;
      opvalue = read_FC_counter(FCAddress + OffsetFirstPixel);
      opvalue &= 0xFC00;
      opvalue |= Region[i].StartPixel;
      update_FC_counter(opvalue, FCAddress + OffsetFirstPixel);
      update_FC_counter(Region[i].Number, FCAddress + OffsetNumPixels);
      TableOneItem += (USHORT)sizeof(struct Table1Entry);
      Table1Index++;
      DeltaN--;
      }
    }
  release_OMA_and_init();
  return (1);
}
