/* -----------------------------------------------------------------------
/
/  omameth.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/omameth.c_v   0.21   06 Jul 1992 10:34:58   maynard  $
/  $Log:   J:/logfiles/oma4000/main/omameth.c_v  $
 * 
 *    Rev 0.21   06 Jul 1992 10:34:58   maynard
 * add saveMethodtoDefaultFile - saves method to OMA4000.MET file;
 * this is now done automatically at program exit
 * 
*/

#include <string.h>   // stricmp()
#include <malloc.h>   // calloc()
#include <stdlib.h>   // atof()

#include "omameth.h"
#include "detinfo.h"
#include "filestuf.h"    // fidDataOMA4
#include "detsetup.h"    // detector_index
#include "syserror.h"    // ERROR_OPEN
#include "omaerror.h"
#include "oma4000.h"     // VersionNumber
#include "basepath.h"    // base_path()
#include "plotbox.h"     // CopyPlotToMethod

/****************************************************************************/

LPMETHDR InitialMethod;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA getInitialMethodFromFile(UCHAR interfaceType)
{
  FILE * fhnd = fopen(base_path(METHOD_FILE), "rb");
  ERR_OMA err;

  if(! fhnd)
    return error(ERROR_OPEN, base_path(METHOD_FILE));

  if (err = MethdrRead(fhnd, base_path(METHOD_FILE), &InitialMethod))
    return err;

  fclose(fhnd);

  InitialMethod->InterfaceType = interfaceType;

  // print_method(InitialMethod);

  return ERROR_NONE;
}


ERR_OMA saveMethodtoDefaultFile(void)
{
  FILE * fhnd = fopen(base_path(METHOD_FILE), "wb");
  ERR_OMA err;

  if(! fhnd)
    {
    return error(ERROR_OPEN, base_path(METHOD_FILE));
    }

  /* copy the plotbox values to InitialMethod */
  CopyPlotToMethod();

  /* copy the scan values to the method */
  err = DetInfoToMethod(&InitialMethod);

  strcpy(InitialMethod->FileTypeID, fidMethodOMA4);

  if (err = MethdrWrite(fhnd, base_path(METHOD_FILE), InitialMethod))
    return err;

   fclose(fhnd);

  // get rid of copied detector data from the method
  DeAllocMetDetInfo(InitialMethod);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DeAllocMetDetInfo(LPMETHDR pMethdr)
{
   DeAllocDetInfo(pMethdr->DetInfo, pMethdr->DetNumber);
   pMethdr->DetInfo = NULL;
   pMethdr->DetNumber = 0;
}

// load the method header detector info from the detector driver. Previous
// detector info is deallocated.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERR_OMA DetInfoToMethod(LPMETHDR *ppMethdr)
{
   USHORT TotalDetLen;
   int   detectorCount;
   ERR_OMA err;

#ifdef COMPILE_4000
      (*ppMethdr)->InterfaceType = INTERFACE_AT_4000;
#else
      (*ppMethdr)->InterfaceType = INTERFACE_AT_2000;
#endif

   // free up previous detector info if any
   DeAllocMetDetInfo(*ppMethdr);

   err = DetDriverToDetInfo(& (* ppMethdr)->DetInfo, & detectorCount,
                             & TotalDetLen);

   (* ppMethdr)->DetNumber = (CHAR) detectorCount;
   
   if(err)
      return err;

   (*ppMethdr)->ActiveDetector = (UCHAR)detector_index;

   (*ppMethdr)->Length = HDR_BASE_LENGTH + TotalDetLen;

   return ERROR_NONE;
}

// load the detector driver from method header detector info and set the
// detector index.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MethodToDetInfo(LPMETHDR pMethdr)
{
   detector_index = pMethdr->ActiveDetector;
   
   DetInfoToDetDriver(pMethdr->DetInfo, pMethdr->DetNumber);
}

/***************************************************************************/
/* ERR_OMA FindMethdrSpace(LPMETHDR far *Methdr, USHORT length)   */
/*                                                                         */
/*  Function: Looks for far memory space to store a method header.         */
/*                                                                         */
/*  Variables: Methdr - Output. Pointer to address of the found memory     */
/*                space to put the new method header structure.            */
/*             length - Input. Length of method header in bytes.  Don't    */
/*                      include space for the group table.                 */
/*                                                                         */
/*  Returns:  ERROR_ALLOC_MEM = memory allocation error                    */
/*            ERROR_NONE = OK                                              */
/*                                                                         */
/*  Side effects: Method memory must be freed by using _ffree              */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA FindMethdrSpace(LPMETHDR far *Methdr, USHORT length)
{
   if(* Methdr = calloc(length, 1))
      return ERROR_NONE;

   return error(ERROR_ALLOC_MEM);
}
  
/***************************************************************************/
/*                                                                         */
/* ERR_OMA DeAllocMethdr(LPMETHDR pMethdr)                        */
/*                                                                         */
/*  Function:  Free up method header space.                                */
/*                                                                         */
/*  Variables: pMethdr - pointer to the method structure                   */
/*                                                                         */
/*  Returns:   ERROR_ALLOC_MEM - memory allocation error                   */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/*  Side effects: none                                                     */
/*                                                                         */
/***************************************************************************/
  
void DeAllocMethdr(LPMETHDR pMethdr)
{
   if(pMethdr)
   {
      DeAllocDetInfo(pMethdr->DetInfo , pMethdr->DetNumber);
      free (pMethdr);
   }
}

/***************************************************************************/
/*                                                                         */
/* ERR_OMA MethdrWrite(FILE *fhnd, const char * fName,              */
/*                            LPMETHDR methdr)                             */
/*                                                                         */
/*  Function:  Write the given method header to the stream fhnd in binary  */
/*             format.                                                     */
/*                                                                         */
/*  Variables: fhnd - stream pointer.                                      */
/*             fName - file name corresponding to fhnd                     */
/*             methdr - pointer to the method header                       */
/*                                                                         */
/*  Returns:   ERROR_WRITE - stream write error.                           */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA MethdrWrite(FILE *fhnd, const char * fName, LPMETHDR methdr)
{
   SHORT i;
   ERR_OMA err;
  
   methdr->SoftwareVersion =  (int)(atof(VersionString) * 100.0);
   methdr->StructureVersion = STRUCT_VERSION;
  
   methdr->Length = HDR_BASE_LENGTH + detInfoBaseLen(methdr->DetNumber);

   for (i=0; i<methdr->DetNumber; i++)
   {
      methdr->Length += sizeOfDetInfoGroups(methdr->DetInfo, i);
   }

   /* write up to Detector info pointer */
   if (fwrite(methdr, HDR_BASE_LENGTH, 1, fhnd) != 1) {
      return error(ERROR_WRITE, fName);
   }

   // write out each detectors info including it's group table
   for (i=0; i<methdr->DetNumber; i++)
   {
      // write out all the detector info stuff
      err = detInfoWriteToFile(methdr->DetInfo, i, fhnd, fName);

      if(err)
         return err;
   }
   return ERROR_NONE;
}
  
/***************************************************************************/
/*                                                                         */
/* ERR_OMA MethdrRead(FILE *fhnd, const char *fName,                */
/*                           LPMETHDR *methdr)                             */
/*                                                                         */
/*  Function:  Read in the method header from the stream. Assumes that the */
/*             method header is at the beginning of the file.              */
/*                                                                         */
/*  Variables: fhnd - pointer to the source stream.                        */
/*             methdr - Output. Pointer to the allocated space of the new  */
/*              method header.                                             */
/*                                                                         */
/*  Returns:   ERROR_ALLOC_MEM - memory allocation error, no mem allocated */
/*             ERROR_READ - stream read error, no memory allocated         */
/*             ERROR_NONE - OK                                             */
/*                                                                         */
/* side effects: Allocates memory for methdr that must be freed by _ffree  */
/*               later.  Positions file pointer after method header.       */
/*                                                                         */
/***************************************************************************/
  
ERR_OMA MethdrRead(FILE *fhnd, const char *fName, LPMETHDR far *methdr)
{
  ULONG TestLen;
  SHORT i;
  ERR_OMA err;

  rewind(fhnd); /* position file pointer to start of file */

  /* allocate space for method header */
  /* group table string following */
  if (FindMethdrSpace(methdr, sizeof(METHDR)))
    return error(ERROR_ALLOC_MEM);

  if (fread(*methdr, HDR_BASE_LENGTH, 1, fhnd) != 1)
    return error(ERROR_READ, fName);

  // check for older version
  if ((*methdr)->StructureVersion < 12)
    return error(ERROR_OLD_VERSION, fName);

  TestLen = HDR_BASE_LENGTH;

  if((*methdr)->DetNumber)
    {
    /* Allocate space for the detector info */
    (* methdr)->DetInfo = allocateDetInfoSpace((* methdr)->DetNumber);
    if(! (* methdr)->DetInfo)
      {
      DeAllocMethdr(*methdr);
      return error(ERROR_ALLOC_MEM);
      }

    for (i=0; i<(*methdr)->DetNumber; i++)
      {
      err = readDetInfoGroupsFromFile((*methdr)->DetInfo, i, fhnd,
        fName, & TestLen);
      if(err)
        {
        DeAllocMethdr(* methdr);
        * methdr = NULL;
        return err;
        }
      }
    }
  else
    (*methdr)->DetInfo = NULL;

  /* must be either a method or data file */
  if (stricmp((*methdr)->FileTypeID, fidDataOMA4) &&
    stricmp((*methdr)->FileTypeID, fidMethodOMA4))
    {
    DeAllocMethdr(*methdr);
    return error(ERROR_IMPROPER_FILETYPE, fName);
    }

  /* test for bad read */
  if (TestLen != (*methdr)->Length)
    {
    DeAllocMethdr(*methdr);
    return error(ERROR_CORRUPT_FILE, fName);
    }

  return ERROR_NONE;
}

int getMethodModelNumber(LPMETHDR methdr)
{
  return getDetInfoModelNumber(methdr->DetInfo);
}

#ifdef DEBUG

/***************************************************************************/
/*  VOID print_method(LPMETHDR methdr);                                    */
/*                                                                         */
/*  function:  Prints information from a method header to the stderr       */
/*             stream.                                                     */
/*                                                                         */
/*   Variables:   methdr - method header to be printed                     */
/*                                                                         */
/*   Returns: none                                                         */
/*                                                                         */
/* last changed:                                                           */
/*               12/16/88  DI                                              */
/*                                                                         */
/***************************************************************************/
  
VOID print_method(LPMETHDR methdr)
{
   USHORT i;
  
   fprintf(stderr, "File Identification    : %s\n", methdr->FileTypeID);
   fprintf(stderr, "Structure version      : %u\n",
      (int) methdr->StructureVersion);
   fprintf(stderr, "Length                 : %u\n", methdr->Length);
   fprintf(stderr, "User                   : %d\n", (int) methdr->User);
   fprintf(stderr, "Description            : %s\n",
      methdr->Description);
   fprintf(stderr, "Number of curves       : %u\n",
      methdr->FileCurveNum);
   fprintf(stderr, "Interface              : %u\n",
      (unsigned) methdr->InterfaceType);

   fprintf(stderr, "Spectrograph (%d)\n",
      methdr->Spectrograph);
   fprintf(stderr, "Grating (%d)\n",
      methdr->Grating);
   for (i=0; i<3; i++)
   {
      fprintf(stderr, "Spectrograph center channel[%d] : %-10.3f\n", i,
      methdr->GratingCenterChnl[i]);
   }
   fprintf(stderr,"\n");
//   for (i=0; i<16; i++)
//      fprintf(stderr, "Slit[%2d] : %-10.3f\n", i, methdr->Slit[i]);
   fprintf(stderr,"\n");
   fprintf(stderr, "Excitation (nm)        : %f\n", methdr->Excitation);
   fprintf(stderr, "X calibration units    : %d\n",
      methdr->CalibUnits[0]);
   fprintf(stderr, "Y calibration units    : %d\n",
      methdr->CalibUnits[1]);
   fprintf(stderr, "Z calibration units    : %d\n",
      methdr->CalibUnits[2]);
   for (i=0; i<4; i++)
   {
      fprintf(stderr,"X a%d : %-10.3f", i,
      methdr->CalibCoeff[0][i]);
      fprintf(stderr,"Y a%d : %-10.3f", i,
      methdr->CalibCoeff[1][i]);
      fprintf(stderr,"Z a%d : %-10.3f", i,
      methdr->CalibCoeff[2][i]);
      fprintf(stderr,"\n");
   }
  
   fprintf(stderr,"\n");
   fprintf(stderr, "DAD fname              : %s\n",
      methdr->DADName);
   fprintf(stderr, "Normalize              : %d\n",
      (int) methdr->Normalize);
   fprintf(stderr, "Active Detector        : %d\n",
      (SHORT) methdr->ActiveDetector);

   // print all the detector information for all the detectors
   printMethodDetInfo(methdr->DetInfo, methdr->DetNumber);
   
   fprintf(stderr,"Background file  : %s\n",
      methdr->BackgrndName);
   fprintf(stderr,"I0 filename      : %s\n",
      methdr->I0Name);
   fprintf(stderr,"Input filename   : %s\n",
      methdr->InputName);
   fprintf(stderr,"Output filename  : %s\n",
      methdr->OutputName);
   fprintf(stderr,"YT interval      : %g\n", methdr->YTInterval);
   fprintf(stderr,"YT Predelay      : %g\n", methdr->YTPredelay);
   fprintf(stderr,"PIA              : %xX  %xX\n",
      methdr->Pia[0], methdr->Pia[1]);

   fprintf(stderr, "Plot Window StyleIndex : %d\n",
      methdr->PlotWindowIndex);
   fprintf(stderr, "Active Plot Setup      : %d\n",
      methdr->ActivePlotSetup);
   fprintf(stderr, "AutoScaleX             : %d\n",
      methdr->AutoScaleX);
   fprintf(stderr, "AutoScaleY             : %d\n",
      methdr->AutoScaleY);
   fprintf(stderr, "AutoScaleZ             : %d\n",
      methdr->AutoScaleZ);

   for (i=0; i<8; i++)
      fprintf(stderr, "Plot Setup for window %d : %d\n", i,
         methdr->WindowPlotSetups[i]);

   for (i=0; i<8; i++)
   {
      fprintf(stderr, "Plot Setup             : %d\n", i);
      fprintf(stderr, "XUnits                 : %d\n",
         (int) methdr->PlotInfo[i].XUnits);
      fprintf(stderr, "YUnits                 : %d\n",
         (int) methdr->PlotInfo[i].YUnits);
      fprintf(stderr, "ZUnits                 : %d\n",
         (int) methdr->PlotInfo[i].ZUnits);
      fprintf(stderr,"Xlabel           : %s\n",
         methdr->PlotInfo[i].XLegend);
      fprintf(stderr,"Ylabel           : %s\n",
         methdr->PlotInfo[i].YLegend);
      fprintf(stderr,"Zlabel           : %s\n",
         methdr->PlotInfo[i].ZLegend);
      fprintf(stderr,"PlotTitle        : %s\n",
         methdr->PlotInfo[i].Title);
  
      fprintf(stderr,"\n");
      fprintf(stderr,"X minimum    : %-10.3g",
         methdr->PlotInfo[i].XMin);
      fprintf(stderr,"X maximum    : %-10.3g\n",
         methdr->PlotInfo[i].XMax);
      fprintf(stderr,"X Ascending  : %d\n",
         methdr->PlotInfo[i].XAscending);
  
      fprintf(stderr,"Y minimum    : %-10.3g",
         methdr->PlotInfo[i].YMin);
      fprintf(stderr,"Y maximum    : %-10.3g\n",
         methdr->PlotInfo[i].YMax);
      fprintf(stderr,"Y Ascending  : %d\n",
         methdr->PlotInfo[i].YAscending);
  
      fprintf(stderr,"Z minimum    : %-10.3g",
         methdr->PlotInfo[i].ZMin);
      fprintf(stderr,"Z maximum    : %-10.3g\n",
         methdr->PlotInfo[i].ZMax);
      fprintf(stderr,"Z Ascending  : %d\n",
         methdr->PlotInfo[i].ZAscending);
      fprintf(stderr,"XZ Percent   : %d\n",
         methdr->PlotInfo[i].XZPercent);
      fprintf(stderr,"YZ Percent   : %d\n",
         methdr->PlotInfo[i].YZPercent);

      fprintf(stderr,"Style             : %d\n", methdr->PlotInfo[i].Style);
      fprintf(stderr,"Plot Peak Labels  : %d\n",
         methdr->PlotInfo[i].PlotPeakLabels);
      fprintf(stderr,"\n");
   }  
  
   fprintf(stderr,"\n");
}
  
/***************************************************************************/
/*  VOID print_data(LPMETHDR methdr, LPCURVE *curve)                       */
/*                                                                         */
/*  function:  Prints information from a method header and a single curve  */
/*             associated with the method to the stderr stream.            */
/*                                                                         */
/*   Variables:   methdr - method header to be printed                     */
/*                curve - starting address of curve structure              */
/*                                                                         */
/*   Returns: none                                                         */
/*                                                                         */
/* last changed:                                                           */
/*               7/28/88   DI                                              */
/*                                                                         */
/***************************************************************************/
  
//VOID print_data(LPMETHDR methdr, LPCURVE *curve)
//{
//   USHORT i;
//  
//   print_method(methdr);
//   for (i=0; i<methdr->FileCurveNum; i++)
//      print_curve(*(curve++));
//}

#endif  // DEBUG


