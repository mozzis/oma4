/* -----------------------------------------------------------------------
/
/  spline-3.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
*/

// FILE : spline-3.c  --  cubic spline approximation/interpolation
//                        RAC  7-Feb-90

#include <stdio.h>  // sprintf() for debugging only

#include "spline-3.h"

typedef struct {
   float x;
   float y;
} SPLINE_KNOT;

// maximum number of points used for computing coefficients, interpolating
#define MAX_KNOTS 100

// storage for the knots
static SPLINE_KNOT knot[ MAX_KNOTS ];

// the number of knots currently in use
static SHORT knot_count = 0;    

// the index of the last knot present in knot[]
#define last_index (knot_count - 1)

// TRUE iff need to recalculate coefficients before interpolating
static BOOLEAN knots_changed = FALSE;

// cubic spline coefficients
static float coeff_1[ MAX_KNOTS ];
static float coeff_2[ MAX_KNOTS ];
static float coeff_3[ MAX_KNOTS ];
static float coeff_4[ MAX_KNOTS ];

// find the index of the knot which has an x value <= a given value.
// return -1 if there are no knots or if the given value is less than
// all knot x values
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static int floor_index(float X)
{
   SHORT i;           

   if((knot_count == 0) || (X < knot[ 0 ].x)) return -1;
   for(i = 1; i < knot_count; i ++)
      if(knot[ i ].x > X)
         return i - 1;
   return knot_count - 1;
}

// linear interpolation given an x_value and two knots A and B
// A and B are index values into knot[]
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static float linear_interp(float x_value, int A, int B)
{
   float slope;
   float delta_X;

   slope = (knot[ B ].y - knot[ A ].y) / (knot[ B ].x - knot[ A ].x);
   delta_X = x_value - knot[ A ].x;
   return knot[ A ].y + slope * delta_X;
}

// point interpolation for the special case when knot_count < 3 which
// means that no spline coefficients have been calculated.
// if knot_count is zero, return 0.0
// if knot_count is one,  return the y value of the knot
// if knot_count is two,  linear interpolate based on the two knots
// else return 0.0
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static float special_interp(float x_value)
{
   switch(knot_count) {
      case 0 : return (float) 0.0;
      case 1 : return knot_y_val(0);
      case 2 : return linear_interp(x_value, 0, 1);
      default : return (float) 0.0;
   }
}

// calculate cubic spline coefficients from given x-y data points.
// x[], y[] are a set of x-y data points for the function to be approximated.
// x_points is a list of index values into x[] of the points to use for the
// approximation.  The list should start at x_points[0] and is terminated by
// any negative value.  There should not be more than MAX_KNOTS elements in
// the list.
// REFERENCE : "Introductory Computer Methods and Numerical Analysis",
//           2nd edition, by Ralph Pennington, MacMillan, 1970. page 445 ff
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void spline_calc(void)
{
   SHORT k;                         // loop index   1/10/90 DAI
   float ek, eprev, Q, a1_2, a1_m;   // temps
   float d[ MAX_KNOTS ];
   float p[ MAX_KNOTS ];
   float b[ MAX_KNOTS ];
   float z[ MAX_KNOTS ];
   float a3[ MAX_KNOTS ];
   static const float one_sixth = (float) (1.0 / 6.0);

   if(knot_count < 3) return; // not enough knots for computation

   // loop 2
   for(k = 0; k < last_index; k++) {
      d[ k ] = knot[ k + 1 ].x - knot[ k ].x;
      p[ k ] = d[ k ] * one_sixth;
   }
   // loop 3
   b[ 0 ] = (float) 0.0;
   b[ last_index ] = (float) 0.0;
   eprev = (knot[ 1 ].y - knot[ 0 ].y) / d[ 0 ];   // e[ 0 ]
   for(k = 1; k < last_index; k ++) {
      ek = (knot[ k + 1 ].y - knot[ k ].y) / d[ k ];
      b[ k ] = ek - eprev;
      eprev = ek;
   }
   a3[ 0 ] = d[ 0 ] / d[ 1 ];
   a1_2 = ((float) -1.0) - a3[ 0 ];
   Q = ((float) 2.0) * (p[ 0 ] + p[ 1 ]) - p[ 0 ] * a1_2; // Q = a2[ 1 ]
   a3[ 1 ] = (p[ 1 ] - p[ 0 ] * a3[ 0 ]) / Q;
   b[ 1 ] = b[ 1 ] / Q;
   // loop 4
   for(k = 2; k < last_index; k++) {
      // Q = a2[ 1 ]
      Q = ((float) 2.0) * (p[ k - 1 ] + p[ k ])
      - p[ k - 1 ] * a3[ k - 1 ];
      b[ k ] = b[ k ] - p[ k - 1 ] * b[ k - 1 ];
      a3[ k ] = p[ k ] / Q;
      b[ k ] = b[ k ] / Q;
   }
   Q = d[ last_index - 2 ] / d[ last_index - 1 ];
   a1_m = ((float) 1.0) + Q + a3[ last_index - 2 ];
   b[ last_index ] = b[ last_index - 2 ] - a1_m * b[ last_index - 1 ];
   z[ last_index ] = b[ last_index ] / (-Q - a1_m * a3[ knot_count - 2 ]);
   // loop 6
   for(k = last_index - 1; k > 0; k --)
      z[ k ] = b[ k ] - a3[ k ] * z[ k + 1 ];

   z[ 0 ] = -a1_2 * z[ 1 ] - a3[ 0 ] * z[ 2 ];
   // loop 7
   for(k = 0; k < last_index; k++) {
      Q = one_sixth / d[ k ];
      coeff_1[ k ] = z[ k ] * Q;
      coeff_2[ k ] = z[ k + 1 ] * Q;
      coeff_3[ k ] = knot[ k ].y / d[ k ] - z[ k ] * p[ k ];
      coeff_4[ k ] = knot[ k + 1 ].y / d[ k ] - z[ k + 1 ] * p[ k ];
   }
}
// given an x-value and a knot index, return the y-value corresponding
// to the spline from knot k to k + 1.  x_val does not necessarily have
// to belong to the region from knot k to k + 1.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static float spline_eval(float x_val, int k)
{
   float y, delta;

   if(knots_changed) {
      spline_calc();                         // compute new coefficients
      knots_changed = FALSE;
   }
   delta = knot[ k + 1 ].x - x_val;
   y = delta * (coeff_1[ k ] * delta * delta + coeff_3[ k ]);
   delta = x_val - knot[ k ].x;
   return y + delta * (coeff_2[ k ] * delta * delta + coeff_4[ k ]);
}


// Return the function value corresponding to the cubic spline curve at
// a given point on the x-axis.  Return 0.0 if there are no knots at all.
// Linear interpolate on the two end knots if the x_value is not in
// the interval from first knot to last knot inclusive.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float point_interp(float x_value)
{
   if(knot_count < 3)
      return special_interp(x_value);  // handle special cases

   // if out of range, use the spline formula at the end knot
   if(x_value <= knot[ 0 ].x)
      return spline_eval(x_value, 0);
   if(x_value >= knot[ knot_count - 1 ].x)
      return spline_eval(x_value, knot_count - 2);

   return spline_eval(x_value, floor_index(x_value));
}

// clear out all the knots, so as to start over
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void delete_all_knots(void) { knot_count = 0; }

// add a knot with at the given (x,y) point.  If a knot already exists with
// the given x value, this merely changes the y value associated with that
// knot instead of adding a new knot.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void add_knot(float x_value, float y_value)
{
   int j, k;

   if(knot_count >= MAX_KNOTS) return;  // no room for a new knot
   if((k = floor_index(x_value)) < 0)
      if(knot_count == 0) {  // no knots, so put in a first one
         knot[ 0 ].x = x_value;
         knot[ 0 ].y = y_value;
         knots_changed = TRUE;
         knot_count = 1;
         return;
      }
      else {  // x_value < knot[ 0 ].x   new first knot
         // move all the other knots up one place
         for(j = knot_count - 1; j >= 0; j --)
            knot[ j + 1 ] = knot[ j ];
         knot[ 0 ].x = x_value;
         knot[ 0 ].y = y_value;
         knot_count ++;
         knots_changed = TRUE;
         return;
      }
   if(x_value > knot[ knot_count - 1 ].x) { // add new last knot
      knot[ knot_count ].x = x_value;
      knot[ knot_count ++ ].y = y_value;
      knots_changed = TRUE;
      return;
   }
   // k is the floor index for an x-value between the first and last knots
   if(x_value == knot[ k ].x) {
      if(y_value == knot[ k ].y) return;        // same knot, no change
      knot[ k ].y = y_value;
      knots_changed = TRUE;
      return;
   }
   // insert a new knot between k and k + 1
   for(j = knot_count - 1; j > k; j --)
      knot[ j + 1 ] = knot[ j ];
   knot[ k + 1 ].x = x_value;
   knot[ k + 1 ].y = y_value;
   knot_count ++;
   knots_changed = TRUE;
}

// delete the i'th knot.  Do nothing if i < 0 or i > number of knots
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void delete_knot_index(SHORT index)  
{
   if((index < 0) || (index >= knot_count)) return;
   for(; index < knot_count - 1; index ++)
      knot[ index ] = knot[ index + 1 ];
   knot_count --;
   knots_changed = TRUE;
}

// delete the knot with x_value.  If there is no such knot, do nothing
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void delete_knot_at_x(float x_value)
{
   int k;

   if((k = floor_index(x_value)) < 0) return;
   if(x_value == knot_x_val(k))
      delete_knot_index(k);
}

// return the current number of knots
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int current_knots(void) { return knot_count; }

// return the maximum number of knots allowed
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int max_knots(void) { return MAX_KNOTS; }

// return the x_value of the i'th knot.  Zero if no such knot.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float knot_x_val(SHORT index)  
{
   if((index < 0) || (index >= knot_count)) return (float) 0.0;
   return knot[ index ].x;
}

// return the y_value of the i'th knot.  Zero if no such knot.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float knot_y_val(SHORT index)  
{
   if((index < 0) || (index > knot_count)) return (float) 0.0;
   return knot[ index ].y;
}

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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int auto_baseline(USHORT num_points, GET_VAL_FPTR get_data)
{
   int err;
   USHORT i;        
   int k;
   int excursion;      
   float interval;
   int half_interval;  
   USHORT left_min;                    // index of the left min point
   float leftmin_xval, leftmin_yval;   // left min point on the curve
   USHORT right_min;                   // index of the right min point
   float rightmin_xval, rightmin_yval; // right min point on the curve
   float X;
   float Y;
   // leave some spare knots available for later manual additions
   int knot_count = max_knots() / 4;

   delete_all_knots();  // "erase" previous spline and start over

   interval = (float) (num_points - 1) / (float) (knot_count - 1);
   if(interval < (float) 2.0) interval = (float) 2.0;
   half_interval = ((int) interval) / 2;
   for(k = 0; k < knot_count; k ++)
   {
      i = (USHORT) (k * interval);
      if(i >= num_points - 1) i = num_points - 2;
      // always try to use next to last point, assume the last point is a
      // calibration point and don't use it.
      if(k == knot_count - 1)
         i = num_points - 2;
      // pick a local minimum but don't move farther than half an interval
      // otherwise a smooth curve might have only one knot at the min
      left_min = right_min = i;
      if(err = (* get_data) (i, & X, & Y)) return err;
      leftmin_yval = rightmin_yval = Y;
      leftmin_xval = rightmin_xval = X;
      // move left as long as there are smaller or equal y values
      excursion = i - half_interval;
      if(excursion < 0) excursion = 0;
      while((left_min > (USHORT) excursion))   
      {
         if(err = (* get_data) (left_min - 1, & X, & Y)) return err;
         if(Y > leftmin_yval) break;
         left_min --;
         leftmin_yval = Y;
         leftmin_xval = X;
      }
      // move right as long as there are smaller or equal y values
      excursion = i + half_interval;
      // assume the last point is a calibration point, don't use it
      if((USHORT) excursion > (num_points - 2))   
         excursion = num_points - 2;
      while(right_min < (USHORT) excursion)      
      {
         if(err = (* get_data) (right_min + 1, & X, & Y)) return err;
         if(Y > rightmin_yval) break;
         right_min ++;
         rightmin_yval = Y;
         rightmin_xval = X;
      }
      if(leftmin_yval > rightmin_yval)
         add_knot(rightmin_xval, rightmin_yval);
      else
         add_knot(leftmin_xval, leftmin_yval);
   }
   return 0;
}
// find the closest knot to a given x-value.  Return with knot_X and
// knot_Y set to the closest knot.  The integer return value is the index
// of the closest knot. If there are no knots, return -1, leave knot_X
// and knot_Y unchanged.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int closest_knot(float X, float * knot_X, float * knot_Y)
{
   USHORT k;              
   int closest_index;

   if((k = floor_index(X)) == -1)   
   {
      if(knot_count == 0) return -1;
      closest_index = 0;
   }
   else
      // k is the floor_index for X
      if(k >= (USHORT) (knot_count - 1))      
         closest_index = k;
      else
         if((X - knot_x_val(k)) <= (knot_x_val(k + 1) - X))
            closest_index = k;
         else
            closest_index = k + 1;

   * knot_X = knot_x_val(closest_index);
   * knot_Y = knot_y_val(closest_index);
   return closest_index;
}

// move the spline up or down by a fixed offset.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void move_spline(float offset)
{
   int k;

   for(k = 0; k < current_knots(); k ++)
      add_knot(knot_x_val(k), knot_y_val(k) + offset);
}

