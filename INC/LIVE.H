/***************************************************************************/
/*  live.h                                                                 */
/*                *** OMA 35 Version ***                                   */
/*                                                                         */
/*  copyright (c) 1988, EG&G Instruments Inc.                              */
/*                                                                         */
/*  Live curve handling for the  OMA 35 program.                           */
/*
*  $Header: /OMA4000/Include/LIVE.H 1     6/18/99 1:52a Maynard $
*/
/***************************************************************************/

#ifndef LIVE_INCLUDED
#define LIVE_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

extern CHAR LiveFileName[];
extern CHAR AccumFileName[];
extern CHAR LiveRefName[];
extern CHAR * LastLiveEntryName;
extern CHAR * PauseStr;
extern BOOLEAN SaveLiveFrame;
extern BOOLEAN DoAccum;
extern BOOLEAN InLiveLoop;
extern SHORT LiveBlkIndex;
extern SHORT RefEntryIndex;
extern SHORT BackGroundIndex;
extern BOOLEAN BackGroundActive;
extern BOOLEAN Paused;
extern BOOLEAN DoLive;
extern USHORT LiveFilterMode;

enum FilterModes { No_Filter, 
                   Absorbance, 
                   Transmission, 
                   AbsorbDual, 
                   YIntensCorrex,
                   NumFiltModes };

ERR_OMA GetAcqCurveBuf(SHORT BlkIndex, SHORT BackIndex,
                               USHORT CurveIndex);
ERR_OMA SetupAcqCurveBlk(USHORT DataPoints, USHORT Curves);
BOOLEAN InitGoLive(void);     
void ExitGoLive(void);        
void GoLive(unsigned int);
void LiveLoop(void);
ERR_OMA sub_background(PLONG, USHORT, CURVEHDR *);
SHORT StopLive(unsigned int);
SHORT PauseLive(unsigned int);
void MacPauseLive(void);
SHORT NextFrame(unsigned int);      
SHORT FreeRun(unsigned int);
void MacFreeRun(void);
void LiveScaleParams(USHORT * BlkIndex, USHORT * CvIndex);
SHORT CaptureLive(USHORT Dummy);    
ERR_OMA SetupLiveXPlot(void);
void SwitchModeLiveKey(USHORT Dummy);
SHORT CaptureBackGround(USHORT Dummy);
ERR_OMA LiveLoopAutoScaleAdjust(PLOTBOX * pPlotBox);

void MacGoLive(void);
void MacStopLive(void);

#endif
