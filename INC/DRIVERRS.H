/* $Header: /OMA4000/Include/DRIVERRS.H 1     6/18/99 1:52a Maynard $Log:   J:/logfiles/oma4000/inc/driverrs.h_v  $
 * 
 *    Rev 1.5   06 Jul 1992 11:49:06   maynard
 * More error value adjustments to sync with omaerrors
 * 
 *    Rev 1.4   12 Mar 1992 15:10:14   maynard
 * Adjust error values to agree with OMAERROR.H
 * 
 *    Rev 1.3   07 Jan 1992 12:31:18   maynard
 * Add Too Many Mems error
 * 
 *    Rev 1.2   01 Nov 1991 14:39:42   maynard
 * Add error number for bad da program number
 * 
 *    Rev 1.1   26 Sep 1991 07:32:50   maynard
 * Add errors for too many tracks and bad expose time
 * 
 *    Rev 1.0   28 Aug 1991 17:39:56   cole
 * Initial revision.
 * 
/* ----------------------------------------------------------------------- */

#ifndef DRIVERR_INCLUDED
#define DRIVERR_INCLUDED

#define NO_ERROR 0
#define DRIV_ERROR 1 

#define  ERROR_FAKEDETECTOR      700
#define  ERROR_NOSUCH_PARAM      701   /* No such param defined */
#define  ERROR_MONITOR_VER       702
#define  ERROR_DA_ACTIVE         704 
#define  ERROR_BAD_STRUCTURE     705   /* didn't find start or end addrs */
#define  ERROR_TOO_LONG          706   /* Template was too long */
#define  ERROR_NO_SERIAL_CODE    707   /* No code given for serial reg read*/
#define  ERROR_NO_CODE_SPACE     708   /* No address given for code output */
#define  ERROR_NO_CLEAN_CODE     709   /* No address for clean shift reg */
#define  ERROR_DETECTOR_TIMEOUT  710   /* detector timeout on stop or start */
#define  ERROR_POINT_NUMBER      711   /* CurrentPoint was > ActiveX */
#define  ERROR_TRACK_NUMBER      712   /* CurrentTrack was > ActiveY */
#define  ERROR_EXPOSE_TIME       713   /* ET out of range            */
#define  ERROR_NOSUCH_DAPROG     714   /* No address for clean shift reg */
#define  ERROR_TOO_MANY_MEMORIES 715   /* Number of memories too large */
#define  ERROR_NO_LIVE_DATA      716
#define  ERROR_SCAN_MISMATCH     717   /* ASIC file doesn't match detector */
#define  ERROR_ILLEGAL_RANGE     718   /* Can't set delay range if INC is 0 */
#define  ERROR_VALUE_INCORRECT   719   /* General parameter error */
#define  ERROR_ADJUSTED_TRACK    720   /* Track setup wouldn't fit */
#define  ERROR_ADJUSTED_POINT    721   /* Point setup wouldn't fit */
#define  ERROR_MONITOR_LOAD      722   /* couldn't load monitor file */

/* Rapda errors. */
#define  OVERLAP_ERR             723   /* region overlap. */
#define  SIZE_ERR                724   /* fewer than MIN_PXLS in region. */
#define  OUT_OF_ARRAY_ERR        725   /* region falls outside array. */
#define  UNSCANNED_PXL_ERR       726   /* Pixel not part of any region. */
#define  SHORT_ET_ERR            727   /* Short.ET too short. */
#define  ET_ERR                  728   /* ET not Short.ET * 2ü. */
#define  ET_RANGE_ERR            729   /* (BaseET / ShortET) > (2^16). */
#define  ARRAY_SIZE_ERR          730   /* Array had more
                                          than (1024 / MIN_PXLS) regions. */
#define  ERROR_ODD_TEMP          731   /* Temp was not even */
#define  ERROR_NOFASTSHIFT       732   /* Detector doesn't have fast shiftline capability */
#endif
