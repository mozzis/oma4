// -------------------------------------------------------------------------
//   wintags.h

//  Copyright (c) 1988,  EG&G Princeton Applied Research

//  Written by: RAC       Version 1.00  September 28, 1990

// Provide curve tagging by display curve number on a per window basis.

// Curves are tagged/untagged using the Z value of the curve in the window.
// However, the tagging is actually being done by display curve number
// within the window.  The first curve in the window is display curve number
// zero, the next is display curve number one, etc.

// The user can tag curves in a window for display in a different color,
// or for display in an "isolated" mode with only the tagged curves plotted
// in a full screen window.  The wintags module provides the functionality
// needed to maintain a set of display curve numbers for each window and to
// indicate whether or not a specific curve number is in the set.
// -------------------------------------------------------------------------
  
#ifndef WINTAGS_INCLUDED
#define WINTAGS_INCLUDED
 
#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

// Add the display curve number corresponding to the curve at zVal to the
// set of display curve numbers for window.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void addTag(SHORT window, USHORT UTCurveIndex);

// Delete the display curve number corresponding to the curve at UTCurveIndex
// from the set of display curve numbers for window.  If the display curve
// number is not in the set, do nothing.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void deleteTag(SHORT window, USHORT UTCurveIndex);

// Make the set of display curve numbers for window the empty set.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void deleteAllTags(SHORT window);

// Add a group of display curve numbers to the set of display curve numbers
// for window.  The group to be added starts with the display curve number
// corresponding to the curve at startZVAl.  It also includes all of the
// display curve numbers which are an integer multiple of increment above
// the starting display curve number of the group.
// If increment is zero, only the single curve at startZVal is tagged.
// If increment is less than zero, tag curves from startZVal down to zero.
//   Examples --
//            addTagGroup(3, 0.0, 1); // tag all curves in window 3.
//            addTagGroup(5, 1.0, 2); // tag all odd curves in window 5.
//            addTagGroup(2, 9.0, -2); // tag 1,3,5,7,9 in window 2.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void addTagGroup(SHORT window, float startZVal, USHORT increment);

// Return TRUE iff curve display number corresponding to the curve at zVal
// is in the set of display curve numbers for window.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN is_tagged(SHORT window, USHORT UTCurveIndex);

// Any entries in the set of display curve numbers for window which
// corresponding to curves outside the range specified by ZvalStart and
// ZvalEnd (inclusive) are to be deleted.  ZvalStart and ZvalEnd should be
// the first and last curves actually being displayed in the window.  They
// can be in either order : 
//       ZvalStart <--> first curve and ZvalEnd <--> last curve.
//                                  OR
//       ZvalStart <--> last curve  and ZvalEnd <--> first curve. 
// The intent is to delete display curve numbers which are no longer valid
// because fewer curves are being displayed in a window than previously.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setTagLimits(SHORT window, float ZvalStart, float ZvalEnd);

// Return TRUE iff there is at least on tagged curve in the window
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN taggedInWindow(SHORT window);

#endif   // WINTAGS_INCLUDED
