/* -----------------------------------------------------------------------
/
/  detinfo.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/detinfo.c_v   1.5   07 Jul 1992 17:10:12   maynard  $
/  $Log:   J:/logfiles/oma4000/main/detinfo.c_v  $
*/

#include <string.h>     // memmove
#include <malloc.h>
#include <stddef.h>     // offsetof

#include "detinfo.h"
#include "oma4scan.h"
#include "detsetup.h"   // DET_SETUP
#include "syserror.h"   // ERROR_ALLOC_MEM
#include "omaerror.h"

struct detector_info
{
   USHORT         Length;
   DET_SETUP      RunSetup;
   USHORT         XPixelGroups;
   USHORT         YPixelGroups;
   USHORT         TriggerGroups;
   USHORT         ETGroups;
   PVOID          GroupTables;
};

// typedef struct detector_info DET_INFO in eggtype.h

#define DET_INFO_BASE_LENGTH offsetof(DET_INFO, GroupTables)

// allocate space for the indicated number of DET_INFO structures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DET_INFO * allocateDetInfoSpace(int numDetectors)
{
   return (DET_INFO *) malloc(sizeof(DET_INFO) * numDetectors);
}

// i is the detector number 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA readDetInfoGroupsFromFile(DET_INFO * detInfo, int i,
                                          FILE * fhnd, const char * fName,
                                          ULONG * testLen)
{
  USHORT tableSize;

  detInfo[i].GroupTables = NULL;
  if (fread(&(detInfo[i]), DET_INFO_BASE_LENGTH, 1, fhnd) != 1)
     return error(ERROR_READ, fName);

  * testLen += DET_INFO_BASE_LENGTH;
  tableSize = sizeOfDetInfoGroups(detInfo, i);

  if (tableSize) /* MLM 2-20-91 If no tables don't be disappointed */
    {
    if(! (detInfo[i].GroupTables = malloc(tableSize)))
      return error(ERROR_ALLOC_MEM);

    if (fread(detInfo[i].GroupTables, tableSize, 1, fhnd) != 1)
      return error(ERROR_READ, fName);

    * testLen += tableSize;
    }
  return ERROR_NONE;
}

// number of bytes needed to store detNumber DET_INFO structures, but not
// including GroupTables
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int detInfoBaseLen(int detNumber)
{
   return detNumber * DET_INFO_BASE_LENGTH;
}

// return the size in bytes for the i'th detinfo groups
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int sizeOfDetInfoGroups(DET_INFO * detInfo, int i)
{
   return (sizeof(SHORT) * 2 * (detInfo[i].XPixelGroups)) +
          (sizeof(SHORT) * 2 * (detInfo[i].YPixelGroups)) +
          (sizeof(SHORT) * 2 * (detInfo[i].TriggerGroups)) +
          ((detInfo[i].ETGroups) ? ((sizeof(int) +
          (sizeof(struct RegionType) * (detInfo[i].ETGroups)))) : 0);
}

// write out det info stuff to a file
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA detInfoWriteToFile(DET_INFO * detInfo, int i, FILE * fhnd,
                                   const char * fName)
{
   PVOID pGroup;
   
   // write out the detector run setup
   // write general detector setup info
   if (fwrite(&(detInfo[i]),
                sizeof(DET_INFO) - sizeof(PVOID), 1, fhnd) !=1)
      return error(ERROR_WRITE, fName);

   /* write group tables */
   /* write slice X0's */
   pGroup = detInfo[i].GroupTables;

   if (   fwrite(pGroup, sizeof(SHORT), detInfo[i].XPixelGroups, fhnd)
        != detInfo[i].XPixelGroups)
      return error(ERROR_WRITE, fName);
  
   /* write slice DeltaX's */
   pGroup = (PVOID) ((ULONG) pGroup
            + (ULONG) (sizeof(SHORT) * detInfo[i].XPixelGroups));

   if (   fwrite(pGroup, sizeof(SHORT), detInfo[i].XPixelGroups, fhnd)
        != detInfo[i].XPixelGroups)
      return error(ERROR_WRITE, fName);
  
   /* write track Y0's */
   pGroup = (PVOID) ((ULONG) pGroup
            + (ULONG) (sizeof(SHORT) * detInfo[i].XPixelGroups));
   if (   fwrite(pGroup, sizeof(SHORT), detInfo[i].YPixelGroups, fhnd)
        != detInfo[i].YPixelGroups)
      return error(ERROR_WRITE, fName);
  
   /* write track DeltaY's */
   pGroup = (PVOID) ((ULONG) pGroup
            + (ULONG) (sizeof(SHORT) * detInfo[i].YPixelGroups));
   if (   fwrite(pGroup, sizeof(SHORT), detInfo[i].YPixelGroups, fhnd)
        != detInfo[i].YPixelGroups)
      return error(ERROR_WRITE, fName);
  
   /* write trigger starting pixels */
   pGroup = (PVOID) ((ULONG) pGroup
            + (ULONG) (sizeof(SHORT) * detInfo[i].YPixelGroups));
   if (   fwrite(pGroup, sizeof(SHORT), detInfo[i].TriggerGroups, fhnd)
        != detInfo[i].TriggerGroups)
      return error(ERROR_WRITE, fName);
  
   /* write trigger pixel widths */
   pGroup = (PVOID) ((ULONG) pGroup
            + (ULONG) (sizeof(SHORT) * detInfo[i].TriggerGroups));
   if (   fwrite(pGroup, sizeof(SHORT), detInfo[i].TriggerGroups, fhnd)
        != detInfo[i].TriggerGroups)
      return error(ERROR_WRITE, fName);

   /* write rapda stuff */

   pGroup = (PVOID) ((ULONG) pGroup
            + (ULONG) (sizeof(SHORT) * detInfo[i].TriggerGroups));
   if (detInfo[i].ETGroups)
   if (   fwrite(pGroup,
         detInfo[i].ETGroups * sizeof(struct RegionType) + sizeof(SHORT),
          1 , fhnd)
        != 1)
      return error(ERROR_WRITE, fName);

   return ERROR_NONE;
}


// deallocate the memory for the DET_INFO area which is actually a copy
// of info elsewhere in the system
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DeAllocDetInfo(DET_INFO * detInfo, USHORT detNumber)
{
   if(detInfo)
   {
      USHORT i;
      
      for(i = 0; i < detNumber; i ++)
      {
         if(detInfo[i].GroupTables)
            free(detInfo[i].GroupTables);
      }
      free(detInfo);
   }
}

// fill detector info from the detector driver. Set * detCount to the number
// of detectors
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA DetDriverToDetInfo(DET_INFO * * theInfo, int * detCount,
                                   USHORT * totalDetLen)
{
  USHORT TableLen;
  ULONG pTmp;
  SHORT i;
  DET_INFO * detInfo;

  * detCount = 0;
  for (i = 0; i < NumDetSetups; i ++)
    {
    if (det_setups[i].det_addr != 0)
      (* detCount) ++;
    }
  detInfo = malloc(sizeof(DET_INFO) * (* detCount));
  * theInfo = detInfo;

  if(! detInfo)
    return error(ERROR_ALLOC_MEM);

  * totalDetLen = 0;
  for (i=0; i<NumDetSetups; i++)
    {
    if (det_setups[i].det_addr != 0)
      {
      int points  = det_setups[i].points  + 1;
      int tracks  = det_setups[i].tracks  + 1;
      int regions = det_setups[i].regions;
      int psize, rsize, tsize;

      if (points > MAX_POINT_COUNT + 1) points = MAX_POINT_COUNT;
      if (tracks > MAX_TRACK_COUNT + 1) tracks = MAX_TRACK_COUNT;
      if (regions > (MAX_PXL / MIN_PXLS)) regions = (MAX_PXL / MIN_PXLS);
      psize = sizeof(SHORT) * points;
      tsize = sizeof(SHORT) * tracks;
      rsize = (regions) ?
                  (sizeof(SHORT) + sizeof(struct RegionType) * regions) :
                  0;
      memmove(&(detInfo[i].RunSetup), &(det_setups[i]), sizeof(DET_SETUP));

      // this will save the same scan setup for all detectors
      // Will find a way to refresh the scan structures when switching
      // detectors later.
      detInfo[i].XPixelGroups = points;
      detInfo[i].YPixelGroups = tracks;
      detInfo[i].TriggerGroups = points;
      detInfo[i].ETGroups = regions;
                                       /* space for 0's & deltas */
      TableLen = 2 * (tsize + psize + psize) + rsize;
      if(! (detInfo[i].GroupTables = malloc(TableLen)))
        return error(ERROR_ALLOC_MEM);

      pTmp = (ULONG) detInfo[i].GroupTables;
      memmove((PVOID) pTmp, POINT_SETUP.X0, psize);
      pTmp += psize;
      memmove((PVOID) pTmp, POINT_SETUP.DeltaX, psize);
      pTmp += psize;
      memmove((PVOID) pTmp, TRACK_SETUP.Y0, tsize);
      pTmp += tsize;
      memmove((PVOID) pTmp, TRACK_SETUP.DeltaY, tsize);
      pTmp += tsize;
      memmove((PVOID) pTmp, TRIGGER_SETUP.StartPixel, psize-2);
      pTmp += psize;
      memmove((PVOID) pTmp, TRIGGER_SETUP.PixelLength, psize-2);
      pTmp += psize;
      memmove((PVOID) pTmp, &RAPDA_SETUP, rsize);


      detInfo[i].Length = DET_INFO_BASE_LENGTH + TableLen;
      * totalDetLen += detInfo[i].Length;
      }
    }
  return ERROR_NONE;
}

// load detector driver from detector info
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DetInfoToDetDriver(DET_INFO * detInfo, SHORT detNumber)
{
  /* save and restore certain params which should only be read */
  /* from the hardware or firmware, not from the method file   */

  SHORT i, det_type, DumYLead, DumYTrail, DumXLead, DumXTrail;
  ULONG pTmp;
  USHORT gCount;

  for(i = 0; i < detNumber; i++)
    {
    det_type = det_setups[i].detector_type_index;
    DumXLead = det_setups[i].DumXLead;
    DumYLead = det_setups[i].DumYLead;
    DumXTrail= det_setups[i].DumXTrail;
    DumYTrail= det_setups[i].DumYTrail;

    memmove(&(det_setups[i]), &(detInfo[i].RunSetup), sizeof(DET_SETUP));

    det_setups[i].detector_type_index = det_type;
    det_setups[i].DumXLead = DumXLead;
    det_setups[i].DumYLead = DumYLead;
    det_setups[i].DumXTrail= DumXTrail;
    det_setups[i].DumYTrail= DumYTrail;

    // this will save the same scan setup for all detectors
    // Will find a way to refresh the scan structures when switching
    // detectors later.

    pTmp = (ULONG) detInfo[i].GroupTables;

    if(gCount = (detInfo[i].XPixelGroups) * sizeof(SHORT))
      {
      memmove(POINT_SETUP.X0, (PVOID) pTmp, gCount);
      pTmp += gCount;
      memmove(POINT_SETUP.DeltaX, (PVOID) pTmp, gCount);
      pTmp += gCount;
      }

    if(gCount = (detInfo[i].YPixelGroups) * sizeof(SHORT))
      {
      memmove(TRACK_SETUP.Y0, (PVOID) pTmp, gCount);
      pTmp += gCount;
      memmove(TRACK_SETUP.DeltaY, (PVOID) pTmp, gCount);
      pTmp += gCount;
      }

    if(gCount = detInfo[i].TriggerGroups * sizeof(SHORT))
      {
      memmove(TRIGGER_SETUP.StartPixel, (PVOID) pTmp, gCount);
      pTmp += gCount;
      memmove(TRIGGER_SETUP.PixelLength, (PVOID) pTmp, gCount);
      pTmp += gCount;
      }
    if (gCount = detInfo[i].ETGroups)
      {
      memmove(&RAPDA_SETUP, (PVOID) pTmp,
        gCount * sizeof(struct RegionType) + sizeof(SHORT));
      det_setups[i].regions = RAPDA_SETUP.NumRegs;
      }
    }
}

int getDetInfoModelNumber(DET_INFO * detinfo)
{
  if (detinfo)
    return detinfo->RunSetup.detector_type_index;
  else
    return 0;
}

#ifdef DEBUG

// print out the detector information part of a method header to stderr.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
VOID printMethodDetInfo(DET_INFO * detInfo, USHORT detNumber)
{
   USHORT i, j;
   int   iTmp;
   PUSHORT  pTmp1, pTmp2;
   ULONG    GroupLen;
  
   for (i=0; i < detNumber; i++)
   {
      fprintf(stderr, "Det Info Length     : %u\n",
         detInfo[i].Length);
      fprintf(stderr, "Monitor Version     : %f\n",
         detInfo[i].RunSetup.Version);
      fprintf(stderr,"DA mode               : %u\n",
         detInfo[i].RunSetup.DA_mode);
      fprintf(stderr,"Address               : %u\n",
         detInfo[i].RunSetup.det_addr);
      fprintf(stderr,"Data word size        : %u\n",
         detInfo[i].RunSetup.data_word_size);
      fprintf(stderr, "Number of scans        : %u\n",
         detInfo[i].RunSetup.scans);
      fprintf(stderr, "Memories               : %u\n",
         detInfo[i].RunSetup.memories);
      fprintf(stderr, "Number of ignore scans : %u\n",
         detInfo[i].RunSetup.ignored_scans);
      fprintf(stderr,"Prep frames           : %d\n",
         detInfo[i].RunSetup.prep_frames);
      fprintf(stderr, "Tracks per memory      : %u\n",
         detInfo[i].RunSetup.tracks);
      fprintf(stderr, "Temperature            : %d\n",
         detInfo[i].RunSetup.detector_temp);
      fprintf(stderr, "Cooler Locked          : %d\n",
         detInfo[i].RunSetup.cooler_locked);

      switch(detInfo[i].RunSetup.detector_type_index)
      {
         case 0:
            iTmp = 0;
            break;
         case 1:
            iTmp = 1462;
            break;
         case 2:
            iTmp = 1463;
            break;
         case 3:
            iTmp = 1464;
            break;
      }
      fprintf(stderr, "Detector type          : %d\n", iTmp);
      fprintf(stderr, "Detector Speed         : %d\n",
         detInfo[i].RunSetup.detector_speed_index);

      switch(detInfo[i].RunSetup.line_freq_index)
      {
         case 0:
            iTmp = 60;
            break;
         default:
            iTmp = 50;
            break;
      }
      fprintf(stderr, "Line Frequency         : %u\n", iTmp);

      fprintf(stderr,"Source comp. mode     : %u\n",
         detInfo[i].RunSetup.source_comp_index);
      fprintf(stderr, "Synchronization mode   : %d\n",
         (int) detInfo[i].RunSetup.control_index);
      fprintf(stderr,"External Start        : %u\n",
         detInfo[i].RunSetup.external_start_index);
//      fprintf(stderr,"Trigger On            : %u\n",
//         detInfo[i].RunSetup.prog_trig_on);
//      fprintf(stderr,"Trigger Polarity      : %u\n",
//         detInfo[i].RunSetup.prog_trig_polarity);
//      fprintf(stderr,"Trigger start_pix     : %u\n",
//         detInfo[i].RunSetup.prog_trig_start_pix);
//      fprintf(stderr,"Trigger Pixel Length  : %u\n",
//         detInfo[i].RunSetup.prog_trig_pix_len);

      // need values for memory_base and memory_size
      
      fprintf(stderr,"Shutter Open sync.    : %u\n",
         detInfo[i].RunSetup.shutter_open_sync_index);
      fprintf(stderr,"Shutter Close sync.   : %u\n",
         detInfo[i].RunSetup.shutter_close_sync_index);
      fprintf(stderr,"Shutter Forced mode   : %u\n",
         detInfo[i].RunSetup.shutter_forced_mode);
      fprintf(stderr,"Need exposure         : %u\n",
         detInfo[i].RunSetup.need_expose_index);

      fprintf(stderr, "exp time            : %f\n",
         detInfo[i].RunSetup.exposure_time);

      fprintf(stderr, "Min exp time        : %f\n",
         detInfo[i].RunSetup.min_exposure_time);
      fprintf(stderr, "Antibloom Percent   : %f\n",
         detInfo[i].RunSetup.anti_bloom_percent);
      fprintf(stderr,"Pixel time            : %d\n",
         detInfo[i].RunSetup.pix_time_index);
      fprintf(stderr,"Nop time              : %d\n",
         detInfo[i].RunSetup.nop_time_index);

      switch(detInfo[i].RunSetup.pulser_index)
      {
         case 0:
            iTmp = 0;
            break;
         case 1:
            iTmp = 1211;
            break;
         case 2:
            iTmp = 1302;
            break;
         case 3:
            iTmp = 1303;
            break;
      }
      fprintf(stderr,"Pulser type      : %u\n", iTmp);

      fprintf(stderr,"Pulser enabled        : %u\n",
         detInfo[i].RunSetup.pulser_enabled_index);
      fprintf(stderr,"Multipulse       : %u\n",
         ! detInfo[i].RunSetup.pulser_oneshot_index);
      fprintf(stderr,"Free Run         : %u\n",
         ! detInfo[i].RunSetup.pulser_freerun_index);
      fprintf(stderr,"Pulse delay      : %g\n",
         detInfo[i].RunSetup.pulser_delay);
      fprintf(stderr,"Pulse width      : %g\n",
         detInfo[i].RunSetup.pulser_width);
      fprintf(stderr,"Pulse increment  : %g\n",
         detInfo[i].RunSetup.pulser_delay_inc);
      fprintf(stderr,"Predelay clock cycles : %u\n",
         detInfo[i].RunSetup.predelay_clock_cycles);
//      fprintf(stderr, "First X pixel          : %u\n",
//         detInfo[i].RunSetup.X0);
//      fprintf(stderr, "First Y pixel          : %u\n",
//         detInfo[i].RunSetup.Y0);
      fprintf(stderr, "Active X pixels        : %u\n",
         detInfo[i].RunSetup.ActiveX);
      fprintf(stderr, "Active Y pixels        : %u\n",
         detInfo[i].RunSetup.ActiveY);
//      fprintf(stderr, "Xbin                   : %u\n",
//         detInfo[i].RunSetup.DeltaX);
//      fprintf(stderr, "Ybin                   : %u\n",
//         detInfo[i].RunSetup.DeltaY);
//      fprintf(stderr, "X Skip                 : %u\n",
//         detInfo[i].RunSetup.SkipX);
//      fprintf(stderr, "Y Skip                 : %u\n",
//         detInfo[i].RunSetup.SkipY);
      fprintf(stderr, "Current Point          : %u\n",
         detInfo[i].RunSetup.Current_Point);
      fprintf(stderr, "Current Track          : %u\n",
         detInfo[i].RunSetup.Current_Track);

      fprintf(stderr, "Max memories           : %u\n",
         detInfo[i].RunSetup.max_memory);
//      fprintf(stderr,"Points                : %d\n",
//         detInfo[i].RunSetup.number_of_points);
      fprintf(stderr,"pointmode                : %u\n",
         detInfo[i].RunSetup.pointmode);
      fprintf(stderr,"Trackmode             : %u\n",
         detInfo[i].RunSetup.trackmode);

      fprintf(stderr, "Shift mode             : %d\n",
      (int) detInfo[i].RunSetup.shiftmode);

      fprintf(stderr,"\nXPixel groups         : %d\n",
         detInfo[i].XPixelGroups);
      GroupLen = sizeof(USHORT) * detInfo[i].XPixelGroups;
      pTmp1 = detInfo[i].GroupTables;
      pTmp2 = (PUSHORT) ((ULONG)pTmp1 + GroupLen);
      for (j=0; j < detInfo[i].XPixelGroups; j++)
      {
         fprintf(stderr,"   Start X Pixel   : %d\n", *pTmp1);
         fprintf(stderr,"   Delta X Pixel   : %d\n", *pTmp2);
         pTmp1++;
         pTmp2++;
      }

      fprintf(stderr,"\nYPixel groups         : %d\n",
         detInfo[i].YPixelGroups);
      GroupLen = sizeof(USHORT) * detInfo[i].YPixelGroups;
      pTmp1 = pTmp2;
      pTmp2 = (PUSHORT) ((ULONG)pTmp1 + GroupLen);
      for (j=0; j < detInfo[i].YPixelGroups; j++)
      {
         fprintf(stderr,"   Start Y Pixel   : %d\n", *pTmp1);
         fprintf(stderr,"   Delta Y Pixel   : %d\n", *pTmp2);
         pTmp1++;
         pTmp2++;
      }

      fprintf(stderr,"\nTrigger groups        : %d\n",
         detInfo[i].TriggerGroups);
      GroupLen = sizeof(USHORT) * detInfo[i].TriggerGroups;
      pTmp1 = pTmp2;
      pTmp2 = (PUSHORT) ((ULONG)pTmp1 + GroupLen);
      for (j=0; j < detInfo[i].TriggerGroups; j++)
      {
         fprintf(stderr,"   Start Trigger Pixel   : %d\n", *pTmp1);
         fprintf(stderr,"   Delta Trigger Pixel   : %d\n", *pTmp2);
         pTmp1++;
         pTmp2++;
      }

      fprintf(stderr,"\nExposure Time groups  : %d\n",
         detInfo[i].ETGroups);
   }
}

#endif  // DEBUG
