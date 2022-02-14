/* -----------------------------------------------------------------------
/
/  spline-3.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
*/

// FILE : spline-3.h  --  cubic spline approximation/interpolation
//                        RAC  7-Feb-90, 16-Feb-90
  
#ifndef SPLINE3_INCLUDED
#define SPLINE3_INCLUDED
  
#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif
  
// A spline package to support the storage/calculation of one spline based
// on x,y values given as "knots".  The points on the spline curve are
// accessed via point_interp() after setting up/changing the knots.
  
//==========================================================================
  
// Return the function value corresponding to the cubic spline curve at
// a given point on the x-axis.  Return 0.0 if there are no knots at all.
// Return the knot y value if there is only one knot.
// Return a linear interpolation of the two end knots if the x_value is not
// in the interval from first knot to last knot inclusive or if there are
// only two knots.
  
float point_interp(float x_value);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
// clear out all the knots, so as to start over
  
void delete_all_knots(void);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
// add a knot at the given (x,y) point.  If a knot already exists with
// the given x value, this merely changes the y value associated with that
// knot instead of adding a new knot.
  
void add_knot(float x_value, float y_value);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
// delete the knot with x_value.  If there is no such knot, do nothing
  
void delete_knot_at_x(float x_value);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
// delete the i'th knot.  Do nothing if there is no i'th knot.
// knots are indexed starting at zero.
  
void delete_knot_index(SHORT index);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
// return the current number of knots
  
int current_knots(void);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
// return the maximum number of knots allowed
  
int max_knots(void);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
// return the x_value of the i'th knot.  Zero if no such knot.
// knots are indexed starting at zero.
  
float knot_x_val(SHORT index);     // 1/10/90 DAI
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
// return the y_value of the i'th knot.  Zero if no such knot.
// knots are indexed starting at zero.
  
float knot_y_val(SHORT index);     // 1/10/90 DAI
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
// function type for auto_baseline function argument
  
typedef int (GET_VAL) (int index, float * X_val, float * Y_val);
typedef GET_VAL * GET_VAL_FPTR;
  
// automatic spline, pick a maximum number of spline knot candidates evenly
// distributed along the x-axis but leave 5 knots unused for later manual
// additions if needed.  Find local minimum looking both right and left from
// each candidate to define the knot.  The resultant spline is a smooth
// baseline curve passing through a set of approximately evenly distributed
// local minima of the given curve.
// Returns zero if no error, non-zero means error.
// get_data() is used to obtain values of the data points as they are
// needed. It returns zero if all is well, non-zero indicates error.
// Prototype : int get_data(int index, float * X, float * Y)
  
int auto_baseline(USHORT num_points, GET_VAL_FPTR get_data);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
// find the closest knot to a given x-value.  Return with knot_X and
// knot_Y set to the closest knot.  The integer return value is the index
// of the closest knot. Knots are indexed starting at zero.
// If there are no knots, return -1, leave knot_X and knot_Y unchanged.
  
int closest_knot(float X, float * knot_X, float * knot_Y);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
// move the spline up or down by a fixed offset.
  
void move_spline(float offset);
  
#endif      // SPLINE3_INCLUDED
