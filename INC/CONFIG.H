/*********************************************************************/
/*                                                                   */
/*      CONFIG.H - The config module for OMA4000                     */
/*      Written by Morris Maynard February 1990                      */
/*      Updates - 4/90 added read data function                      */
/*
/  $Header: /OMA4000/Include/CONFIG.H 1     6/18/99 1:52a Maynard $
/*********************************************************************/

#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

ERR_OMA read_detector_data(void);

// register config form in formTable[]
void registerConfigForm(void);

#endif  // CONFIG_INCLUDED
