/***************************************************************************/
/*  crventry.c                                                             */
/*
/
/  $Header:   J:/logfiles/oma4000/main/crventry.c_v   1.0   07 Jan 1992 11:53:10   cole  $
/  $Log:   J:/logfiles/oma4000/main/crventry.c_v  $
 * 
 *    Rev 1.0   07 Jan 1992 11:53:10   cole
 * Initial revision.
/
/***************************************************************************/
  
#include <string.h>

#include "crventry.h"
  
//#ifdef DEBUG
//
///**************************************************************************/
///* VOID print_curve(CURVE *curve)                                         */
///*                                                                        */
///* function:  Prints information from a single curve to the stderr stream.*/
///*                                                                        */
///* Variables:   curve - starting address of curve structure               */
///*                                                                        */
///* Returns: none                                                          */
///*                                                                        */
///**************************************************************************/
//  
//VOID print_curve(CURVE *curve)
//{
//   ULONG i;
//   PVOID Xptr, Yptr;
//   USHORT pointnum = curve->curvehdr.pointnum ;
//   USHORT print_interval;
//   USHORT DataType = (USHORT) curve->curvehdr.DataType ;
//   FLOAT XTmp, YTmp;
//  
//   fprintf( stderr, "pointnum =          %u\n", curve->curvehdr.pointnum );
//   fprintf( stderr, "curve X Units =     %d\n",
//                                        (int) curve->curvehdr.XData.XUnits );
//   fprintf( stderr, "curve DataType =     %Xx\n", DataType );
//   fprintf( stderr, "experiment number = %u\n",
//                                            curve->curvehdr.experiment_num );
//   fprintf( stderr, "time =            %lu\n", curve->curvehdr.time );
//   fprintf( stderr, "SC val =          %lu\n", curve->curvehdr.scomp );
//   fprintf( stderr, "curve PIA =    %xX %xX\n",
//                            curve->curvehdr.pia[0], curve->curvehdr.pia[1] );
//   fprintf( stderr, "Frame = %u\n", curve->curvehdr.Frame );
//   fprintf( stderr, "Track = %u\n", curve->curvehdr.Track );
//   fprintf( stderr, "curve Y min = %f Y max = %f\n",
//                                curve->curvehdr.Ymin, curve->curvehdr.Ymax );
//   fprintf( stderr, "curve X min = %f X max = %f\n",
//                                curve->curvehdr.Xmin, curve->curvehdr.Xmax );
//  
////   print_interval = (pointnum / 100) + 1; /* only print out 100 values */
////  
////   for (i=0; i < (ULONG) pointnum; i+=(ULONG)print_interval)
////   {
////      Xptr = GetCurveX( curve, (USHORT) i );
////      Yptr = GetCurveY( curve, (USHORT) i );
////      ConvertTypes( Xptr, DataType, &XTmp, FLOATTYPE );
////      ConvertTypes( Yptr, DataType, &YTmp, FLOATTYPE );
////      fprintf( stderr, "data[ %u ]    %f %f\n", (USHORT) i, XTmp, YTmp );
////   }
////  
////   i -= print_interval;
////  
////   /* make sure that the last data point is printed */
////   if (i != (ULONG) (pointnum - 1) )
////   {
////      i = pointnum - 1;
////      Xptr = GetCurveX( curve, (USHORT) i );
////      Yptr = GetCurveY( curve, (USHORT) i );
////      ConvertTypes( Xptr, DataType, &XTmp, FLOATTYPE );
////      ConvertTypes( Yptr, DataType, &YTmp, FLOATTYPE );
////      fprintf( stderr, "data[ %u ]    %f %f\n", (USHORT) i, XTmp, YTmp );
////   }
//
//}
//  
//#endif      /* end of debug code */
  
/***************************************************************************/
/*  int  DirEntryNameCompare(CURVE_ENTRY *elem1, CURVE_ENTRY *elem2)       */
/*                                                                         */
/*  function:  Compare 2 directory entries.  Used with qsort() to sort an  */
/*             array of directory entries into ascending order based on    */
/*             name and starting curve number.                             */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to entries to be compared     */
/*                                                                         */
/*   Returns:     <0 - if elem1 < elem2                                    */
/*                =0 - if elem1 = elem2                                    */
/*                >0 - if elem1 > elem2                                    */
/*                                                                         */
/***************************************************************************/
  
int DirEntryNameCompare(const CURVE_ENTRY *elem1, const CURVE_ENTRY *elem2)
{
   SHORT RelVal;
  
   /* compare names first */
   RelVal = strcmp(elem1->name, elem2->name);
   if (RelVal != 0)
      return RelVal;
  
   /* names are same so compare starting curves */
  
   return (elem1->StartIndex - elem2->StartIndex);
}
  
/***************************************************************************/
/*        int  InverseDirEntryNameCompare(CURVE_ENTRY *elem1,              */
/*                                        CURVE_ENTRY *elem2)              */
/*                                                                         */
/*  function:  Compare 2 directory entries.  Used with qsort() to sort an  */
/*             array of directory entries into descending order based on   */
/*             name and starting curve number.                             */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to entries to be compared     */
/*                                                                         */
/*   Returns:     <0 - if elem2 < elem1                                    */
/*                =0 - if elem2 = elem1                                    */
/*                >0 - if elem2 > elem1                                    */
/*                                                                         */
/***************************************************************************/
  
int InverseDirEntryNameCompare(const CURVE_ENTRY *elem1, const CURVE_ENTRY *elem2)
{
   return - DirEntryNameCompare(elem1, elem2);
}
  
/***************************************************************************/
/*  int  DirEntryTimeCompare(CURVE_ENTRY *elem1, CURVE_ENTRY *elem2)       */
/*                                                                         */
/*  function:  Compare 2 directory entries.  Used with qsort() to sort an  */
/*             array of directory entries into ascending order based on    */
/*             time of last change.                                        */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to entries to be compared     */
/*                                                                         */
/*   Returns:     <0 - if elem1 < elem2                                    */
/*                =0 - if elem1 = elem2                                    */
/*                >0 - if elem1 > elem2                                    */
/*                                                                         */
/***************************************************************************/
  
int DirEntryTimeCompare(const CURVE_ENTRY *elem1, const CURVE_ENTRY *elem2)
{
   SHORT Difference;
  
   /* compare years first */
   Difference = elem1->time.tm_year - elem2->time.tm_year;
  
   if (Difference != 0)
      return Difference;
  
   /* compare days of the year */
   Difference = elem1->time.tm_yday - elem2->time.tm_yday;
  
   if (Difference != 0)
      return Difference;
  
   /* compare hours */
   Difference = elem1->time.tm_hour - elem2->time.tm_hour;
  
   if (Difference != 0)
      return Difference;
  
   /* compare minutes */
   Difference = elem1->time.tm_min - elem2->time.tm_min;
  
   if (Difference != 0)
      return Difference;
  
   /* compare seconds */
   Difference = elem1->time.tm_sec - elem2->time.tm_sec;
  
   return Difference;
}
  
/***************************************************************************/
/*  int InverseDirEntryTimeCompare(CURVE_ENTRY *elem1, CURVE_ENTRY *elem2) */
/*                                                                         */
/*  function:  Compare 2 directory entries.  Used with qsort() to sort an  */
/*             array of directory entries into descending order based on   */
/*             time of last change.                                        */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to entries to be compared     */
/*                                                                         */
/*   Returns:     <0 - if elem2 < elem1                                    */
/*                =0 - if elem2 = elem1                                    */
/*                >0 - if elem2 > elem1                                    */
/*                                                                         */
/***************************************************************************/
  
int InverseDirEntryTimeCompare(const CURVE_ENTRY *elem1, const CURVE_ENTRY *elem2)
{
  return - DirEntryTimeCompare(elem1, elem2);
}
