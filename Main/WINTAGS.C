
//    wintags.c           RAC Oct 12, 1990

//    Copyright (c) 1990,  EG&G Instruments Inc.
/*
/  $Header:   J:/logfiles/oma4000/main/wintags.c_v   0.10   30 Mar 1992 12:57:48   maynard  $
/  $Log:   J:/logfiles/oma4000/main/wintags.c_v  $
 * 
 *    Rev 0.10   30 Mar 1992 12:57:48   maynard
 * re-add include of curvdraw.h since the Find...Plot... functions were
 * re-located back in that module
 * 
 *    Rev 0.9   13 Jan 1992 15:48:36   cole
 * Change include's.
 * 
 *    Rev 0.8   28 May 1991 13:09:50   cole
 * removed all extern's from .c files
 * 
 *    Rev 0.7   10 Jan 1991 01:38:10   maynard
 * Incorporate Dwight's changes from 1.81 oma2000
 * Changes for OMA4 macro language support
 * Add temperature control, shutter control, et. al.
 * 
 *    Rev 1.1   26 Nov 1990 15:27:26   irving
 * Fixed bug in deleteTag
 * 
 *    Rev 1.0   13 Nov 1990 09:58:44   irving
 * Initial revision.
/
*/

#include <stdlib.h>
#include <malloc.h>

#include "wintags.h"
#include "oma4000.h"
#include "cursor.h"
#include "omaerror.h"
#include "curvedir.h"  // MainCurveDir
#include "curvdraw.h"  // finding plot blocks

// ***** Private Types and Data
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// specify a group of curves tagged in a window.  From startIndex to
// endIndex, every delta curves.
typedef struct { USHORT  startIndex ;
                 USHORT  endIndex ;
                 USHORT delta ;
                 SHORT  PlotIndex ;
} TagItem ;

// minimum and maximum values for a curve index
static const USHORT minCurveIndex = 0 ;
static const USHORT maxCurveIndex = 65534 ; // 64K - 2

// pointer to a contiguous array of TagItems.  This array of TagItems
// defines all of the curves in all plots that are currently tagged.
static TagItem * tagList = NULL ;
//TagItem * tagList = NULL ;

// the number of TagItems currently in the tagList.
static USHORT itemsInUse = 0 ;
//USHORT itemsInUse = 0 ;

// ***** Static Function Declarations for TagItems
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static BOOLEAN isInItem( SHORT PlotIndex, USHORT curveIndex,
TagItem * tagItem );
static void newStartIndex( TagItem * tagItem, USHORT first ) ;
static void newEndIndex( TagItem * tagItem, USHORT last ) ;
static void condenseItem( TagItem * item ) ;
static void resetLimits( TagItem * tagItem, USHORT first, USHORT last ) ;
static BOOLEAN deleteCurveIndex( USHORT listIndex, USHORT curveIndex ) ;

// *** Static Function declarations for tagList
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static USHORT indexOf( float zVal ) ;
static BOOLEAN inTagList( SHORT PlotIndex, USHORT curveIndex ) ;
static void removeTagItem( USHORT index ) ;
static BOOLEAN appendItem( TagItem newItem ) ;
static BOOLEAN addTagItem( SHORT PlotIndex, USHORT start, USHORT end,
   USHORT delta ) ;
void validateItem( USHORT listIndex ) ;

// ***** Private Function definitions for TagItems
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// determine if curveIndex is in the set of curves defined by tagItem.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static BOOLEAN isInItem( SHORT PlotIndex, USHORT curveIndex,
TagItem * tagItem )
{
   if( tagItem->PlotIndex != PlotIndex ) return FALSE ;
   if( tagItem->startIndex > curveIndex ) return FALSE ;
   if( tagItem->endIndex   < curveIndex ) return FALSE ;
   return ! ( (curveIndex - tagItem->startIndex) % tagItem->delta ) ;
}

// Change a tagItem's startIndex to first, but increase as needed to ensure
// that endIndex remains a multiple of delta above startIndex.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void newStartIndex( TagItem * tagItem, USHORT first )
{
   if( first > tagItem->endIndex )
      tagItem->startIndex = first ;
   else
      tagItem->startIndex =
                 first + ( ( tagItem->endIndex - first ) % tagItem->delta ) ;
}

// Change a tagItem's endIndex to last, but decrease as needed to ensure
// that endIndex remains a multiple of delta above startIndex.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void newEndIndex( TagItem * tagItem, USHORT last )
{
   if( last < tagItem->startIndex )
      tagItem->endIndex = last ;
   else
      tagItem->endIndex =
                 last - ( ( last - tagItem->startIndex ) % tagItem->delta ) ;
}

// Change item so that neither its startIndex nor its endIndex are tagged.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void condenseItem( TagItem * item )
{
   while( (item->startIndex <= item->endIndex) && (item->endIndex != 0xFFFF))
   {
      item->startIndex += item->delta ;
      if( inTagList( item->PlotIndex, item->startIndex - item->delta ) )
         continue ;
      item->startIndex -= item->delta ;
      break ;
   }

   while( (item->endIndex >= item->startIndex) && (item->endIndex != 0xFFFF))
   {
      item->endIndex -= item->delta ;
      if( inTagList( item->PlotIndex, item->endIndex + item->delta) )
         continue;
      item->endIndex += item->delta ;
      break ;
   }
}

// Modify startIndex, endIndex of tagItem to fall within [first,last].
// Requires that first must be less than or equal to last.
// ( endIndex - startIndex ) must remain a multiple of delta.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void resetLimits( TagItem * tagItem, USHORT first, USHORT last )
{
   if( tagItem->startIndex < first )
      newStartIndex( tagItem, first ) ;

   if( tagItem->endIndex > last )
      newEndIndex( tagItem, last ) ;

   condenseItem( tagItem ) ;

   // one curve special
   if ((tagItem->endIndex < tagItem->startIndex) ||
       (tagItem->endIndex == 0xFFFF))
      tagItem->endIndex = tagItem->startIndex;

}

// Delete a single curveIndex from a tagItem.  Return TRUE iff successful.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static BOOLEAN deleteCurveIndex( USHORT listIndex, USHORT curveIndex )
{
   TagItem * tagItem = tagList + listIndex ;

   if( curveIndex == tagItem->startIndex )
   {
      tagItem->startIndex += tagItem->delta ;
      condenseItem( tagItem ) ;
   }
   else if( curveIndex == tagItem->endIndex )
   {
      tagItem->endIndex -= tagItem->delta ;
      condenseItem( tagItem ) ;
   }
   else // delete a curve from the middle of the tagItem curve set
   {
      TagItem newItem = { tagItem->PlotIndex, curveIndex + tagItem->delta,
                          tagItem->endIndex, tagItem->delta
                        } ;
      condenseItem( & newItem ) ;
      if( (newItem.startIndex <= newItem.endIndex) &&
          (newItem.endIndex != 0xFFFF) )
      {
         if( ! appendItem( newItem ) )
            return FALSE ;
         tagItem = tagList + listIndex ;     // appendItem may change tagList
         tagItem->endIndex = curveIndex - tagItem->delta ;
         condenseItem( tagItem ) ;
      }
   }
   return TRUE ;
}

// ***** Private function definitions for tagList
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Translate a float curve number to an int curve index.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static USHORT indexOf( float zVal )
{
   BOOLEAN Found;
   USHORT UTDisplayCurve, EntryIndex, FileCurve;      // 11/8/90 DAI
   FLOAT i;

   i = 0;
   Found = FindFirstPlotBlock( &MainCurveDir, &EntryIndex,
      &FileCurve, &UTDisplayCurve, ActiveWindow );

   while ( Found && (i < zVal) )
   {
      Found = FindNextPlotCurve( &MainCurveDir, &EntryIndex,
         &FileCurve, &UTDisplayCurve, ActiveWindow );
      i++;
   }

   if (! Found)
      UTDisplayCurve = 0xFFFF;            // error number

   return UTDisplayCurve;
}

// Return TRUE iff the indicated curve is in tagList
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// removed static    10/29/90 DAI
BOOLEAN inTagList( SHORT PlotIndex, USHORT curveIndex )
{
   TagItem * tagItem ;
   USHORT i ;

   for( i = 0 ; i < itemsInUse ; i ++ )
   {
      tagItem = tagList + i ;
      if( isInItem( PlotIndex, curveIndex, tagItem ) ) return TRUE ;
   }
   return FALSE ;
}

// Remove the index'th tagItem from the tagList. Don't leave any holes.
// The list should be compacted.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void removeTagItem( USHORT index )
{
   itemsInUse -- ;  // one less item

   if( index != itemsInUse )
      // copy the last item into the deleted item location.
      *( tagList + index ) = *( tagList + itemsInUse ) ;

   tagList = (TagItem *) realloc( tagList, itemsInUse * sizeof(TagItem) ) ;
}

// Append a new item to the end of the tagList.  Return TRUE iff the append
// was successful.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static BOOLEAN appendItem( TagItem newItem )
{
   TagItem * temp =
         (TagItem *) realloc( tagList, (itemsInUse + 1) * sizeof(TagItem) ) ;

   if( temp ) // realloc was successful
   {
      tagList = temp ;
      *( tagList + itemsInUse ++ ) = newItem ;
   }
   return ( temp != NULL ) ;  // TRUE iff item was added
}

// Add a tagItem to tagList.  Return TRUE iff successful, else return FALSE.
// delta must NOT be zero, else an infinite loop might result.
// startIndex MUST be less than or equal to endIndex.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static BOOLEAN addTagItem( SHORT PlotIndex, USHORT startIndex,
      USHORT endIndex, USHORT delta )
{
   TagItem newItem ;

   newItem.PlotIndex  = PlotIndex;
   newItem.startIndex = startIndex ;
   newItem.endIndex   = endIndex ;
   newItem.delta      = delta ;

   // modify startIndex, endIndex until they are not tagged curves
   condenseItem( & newItem ) ;

   // success if nothing to add
   if( (newItem.startIndex > newItem.endIndex) ||
       (newItem.endIndex == 0xFFFF))
      return TRUE ;

   return appendItem( newItem ) ;
}

// Delete the listIndex item from the taglist if it has no curves in it.
// Condense the item first.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void validateItem( USHORT listIndex )
{
   TagItem * tagItem = tagList + listIndex ;

//   condenseItem( tagItem ) ;
   if( (tagItem->startIndex > tagItem->endIndex) ||
       (tagItem->endIndex == 0xFFFF))
      removeTagItem( listIndex ) ;
}

// ***** Public function definitions for wintags
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Add the display curve number corresponding to the curve at zVal to the
// set of display curve numbers for the plot.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void addTag( SHORT PlotIndex, USHORT UTCurveIndex )
{
   // Add a tagItem for a single curve to tagList
   if( addTagItem( PlotIndex, UTCurveIndex, UTCurveIndex, 1 ) ) return ;

   // tried to add an item and failed, no more room
   error( ERROR_WINTAGS_NO_MEM ) ;
}

// Delete the display curve number corresponding to the curve at zVal
// from the set of display curve numbers for the plot.  If the display curve
// number is not in the set, do nothing.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void deleteTag( SHORT PlotIndex, USHORT UTCurveIndex )
{
   int i ;

   for( i = itemsInUse - 1 ; i >= 0 ; i -- )
   {
      if( ! isInItem( PlotIndex, UTCurveIndex, tagList + i ) ) continue ;
      if( ! deleteCurveIndex( i, UTCurveIndex) )
         error( ERROR_WINTAGS_NO_MEM ) ;
      validateItem( i ) ;                             // 11/21/90 DAI
   }
}

// Make the set of display curve numbers for window the empty set.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void deleteAllTags( SHORT PlotIndex )
{
   int i ;

   for( i = itemsInUse - 1 ; i >= 0 ; i -- )
   {
      if( (tagList + i)->PlotIndex == PlotIndex)
         removeTagItem( i ) ;
   }
}

// Add a group of display curve numbers to the set of display curve numbers
// for a plot.  The group to be added starts with the display curve number
// corresponding to the curve at startZVAl.  It also includes all of the
// display curve numbers which are an integer multiple of increment above
// the starting display curve number of the group.
// If increment is zero, only the single curve at startZVal is tagged.
// If increment is less than zero, tag curves from startZVal down to zero.
//   Examples --
//            addTagGroup( 3, 0.0, 1 ) ; // tag all curves in plot 3.
//            addTagGroup( 5, 1.0, 2 ) ; // tag all odd curves in plot 5.
//            addTagGroup( 2, 9.0, -2 ) ; // tag 1,3,5,7,9 in window 2.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void addTagGroup( SHORT PlotIndex, float startZVal, USHORT increment )
{
   BOOLEAN result ;
   USHORT UTCurveIndex = indexOf( startZVal ) ;

   if( increment == 0 )
      result = addTagItem( PlotIndex, UTCurveIndex, UTCurveIndex, 1 ) ;
   else if( increment > 0 )
   {
      // endIndex an even multiple of increment larger than startIndex
      USHORT endIndex
           = maxCurveIndex - ( ( maxCurveIndex - UTCurveIndex ) % increment ) ;

      result = addTagItem( PlotIndex, UTCurveIndex, endIndex, increment ) ;
   }
   else // increment < 0
   {
      // startIndex an even multiple of -increment smaller than endIndex
      USHORT startIndex
        = minCurveIndex + ( ( UTCurveIndex - minCurveIndex ) % (-increment) ) ;

      result = addTagItem( PlotIndex, startIndex, UTCurveIndex, - increment ) ;
   }

   if( ! result )                             // append failed
      error( ERROR_WINTAGS_NO_MEM ) ;
}

// Return TRUE iff curve display number corresponding to the curve at zVal
// is in the set of display curve numbers for a plot.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN is_tagged( SHORT PlotIndex, USHORT UTCurveIndex ) // 11/5/90 DAI
{
   return inTagList( PlotIndex, UTCurveIndex ) ;  // 11/5/90 DAI
}

// Any entries in the set of display curve numbers for the plot which
// corresponding to curves outside the range specified by ZvalStart and
// ZvalEnd (inclusive) are to be deleted.  ZvalStart and ZvalEnd should be
// the first and last curves actually being displayed in the plot.  They
// can be in either order :
//       ZvalStart <--> first curve and ZvalEnd <--> last curve.
//                                  OR
//       ZvalStart <--> last curve  and ZvalEnd <--> first curve.
// The intent is to delete display curve numbers which are no longer valid
// because fewer curves are being displayed in a plot than previously.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setTagLimits( SHORT PlotIndex, float ZvalStart, float ZvalEnd )
{
   USHORT firstIndex = indexOf( ZvalStart ) ;
   USHORT lastIndex  = indexOf( ZvalEnd ) ;
   int i ;

   if( firstIndex > lastIndex )
   {
      i = firstIndex ;
      firstIndex = lastIndex ;
      lastIndex  = i ;
   }

   for( i = itemsInUse - 1 ; i >= 0 ; i -- )
   {
      TagItem * tagItem = tagList + i ;

      resetLimits( tagItem, firstIndex, lastIndex ) ;
      validateItem( i ) ;
   }
}

// Return TRUE iff there is at least one tagged curve in the plot
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN taggedInWindow( SHORT PlotIndex )
{
   int i ;

   for( i = itemsInUse - 1 ; i >= 0 ; i -- )
      if( (tagList + i)->PlotIndex == PlotIndex ) return TRUE ;

   return FALSE ;
}


