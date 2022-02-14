/* -----------------------------------------------------------------------
/
/  helpindx.h
/
/  Copyright (c) 1989,  EG&G Princeton Applied Research
/
/  $Header: /OMA4000/Include/HELPINDX.H 1     6/18/99 1:52a Maynard $
/  $Log: /OMA4000/Include/HELPINDX.H $
 * 
 * 1     6/18/99 1:52a Maynard
 * 
 *    Rev 0.8   22 May 1991 07:50:34   cole
 * make idempotent
 * 
 *    Rev 0.7   10 Jan 1991 02:11:36   maynard
 * Incorporate Dwight's changes from 1.81 oma2000
 * Changes for OMA4 macro language support
 * Add temperature control, shutter control, et. al.
 * 
 *    Rev 1.2   17 Dec 1990 15:57:16   irving
 * Changed limits of help indices for several forms
 * 
 *    Rev 1.1   13 Nov 1990 10:23:10   irving
 * Added help for tagging curves
/
*/ /*
/ ----------------------------------------------------------------------- */

#ifndef HELPINDX_INCLUDED
#define HELPINDX_INCLUDED

#define MAIN_MENU_HBASE           100    /* omaform1.c: 100-105 */
#define FILES_MENU_HBASE          400    /* omaform1.c: 400-404 */
#define PLOT_MENU_HBASE           500    /* omaform1.c: 500-503 */
#define RUN_MENU_HBASE            600    /* omaform1.c: 600-605 */
#define MATH_MENU_HBASE           700    /* omaform1.c: 700-701 */
#define GRAPH_HBASE               800    /* graphops.c:         */
#define CALIB_HBASE               900    /* omamenu.c : 900-920 */
  
  
#define BACKGROUNDFIELDS_HBASE    1300   /* backgrnd.c: 1300-1304 */
#define X_CALIBRATIONFIELD_HBASE  1400   /* calib.c: 1400-1411 */
#define X_CALSCROLLFIELD_HBASE    1500   /* calib.c: 1500-1504 */
#define GET_CALIB_PT_FIELD_HBASE  1600   /* calib.c: 1600-1600 */
#define DOWNLOAD_FIELD_HBASE      1700   /* download.c: 1700-1703 */
#define MATHFORM_FIELD_HBASE      1800   /* mathform.c: 1800-1811 */
#define DA_FORM_FIELD_HBASE       1900   /* omaform1.c: 1900-1912 */
#define DETECTOR_1463_HBASE       2000   /* omaform1.c: 2000-2005 */
#define DETECTOR_NONE_HBASE       2100   /* omaform1.c: 2100-2100 */
#define DETECTOR_1464_HBASE       2200   /* omaform1.c: 2200-2203 */
#define PLOTSETUP_HBASE           2300   /* omaform1.c: 2300-2328 */ //@@@12/10/90 DAI
#define CHFORMAT_HBASE            2400   /* omaform1.c: 2400-2404 */
#define CONFIG_HBASE              2500   /* omaform1.c: 2500-2513 */ //@@@12/10/90 DAI
#define CONTROL_HBASE             2600   /* scrlltst.c: 2600-2606 */
#define CURVESCROLL_HBASE         2700   /* scrlltst.c: 2700-2703 */
#define FILESCROLL_HBASE          2800   /* scrlltst.c: 2800-2803 */
#define USERFORM_HBASE            2900   /* scrlltst.c: 2900-2915 */
#define GETMETHODHDR_HBASE        3000   /* scrlltst.c: 3000-3002 */
#define SCAN_HBASE                3100   /* scanset.c:            */
#define BASLNSUB_HBASE            3200   /* baslnsub.c: 3200-3212 */
#define LIVEDISK_HBASE            3300   /* omaform1.c: */
#define MACROFRM_HBASE            3400   /* omaform1.c: */
#define LIVE_HBASE                3500   /* omaform1.c: */
#define ACCUM_HBASE               3600   /* omaform1.c: */
#define KEYSTROKE_HBASE           3700   /* omaform1.c: */
#define CURSOR_GOTO_HBASE         3800   /* omaform1.c: */
#define TAGCURVE_HBASE            3900   // tagcurve.c: 3900-3904 RAC 11/12/90
#define RAPDA_HBASE               4000   // rapset.c  : 4000-4010 MLM 04/16/93
#define STATFORM_FIELD_HBASE      5000   /* statform.c: 5000-5040 */
#define YCALFORM_HBASE            5100   /* ycalib.c */
#endif  //  HELPINDX_INCLUDED
