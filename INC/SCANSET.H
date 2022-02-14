/*************************************************************************/
/*                                                                       */
/*    scanset.h                                                          */
/*    written by mlm 1/25/90                                             */
/*                                                                       */
/*    menu/form for setting up scan pattern of 1464 CCD detector         */
/*    handles both uniform and non-uniform slice set ups.                */
/*                 |||||||     |||  || | |                               */
/*                           _             _                             */
/*                           _             _                             */
/*    handles both contiguous_  and random   tracks.                     */
/*                           _             _                             */
/*
/  $Header: /OMA4000/Include/SCANSET.H 1     6/18/99 1:53a Maynard $
*/
/*************************************************************************/
  
#ifndef SCANSET_INCLUDED
#define SCANSET_INCLUDED

// init FormTable[] with form addresses : ScanSetupForm, SliceForm,
//    SliceScrollForm, TrackForm, TrackScrollForm
void registerScansetForms(void);

#endif  //  SCANSET_INCLUDED
