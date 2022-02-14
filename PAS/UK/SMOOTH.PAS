program sm;
{---------------------------------------------------------------------------
This program perform a sliding n-point smooth on the curve set at curve
set index 0. To change the number of points used, edit the points_average
constant, which must be an odd integer value greater than or equal to 3.
(e.g. 3, 5, 7, 9 etc.) The points at the ends of the curve set are not
averaged.

Author:        Richard Burdett
               EG&G Instruments Ltd, Wokingham, Berks, RG11 2GY

Date:          30th October 1991
Rev:           1.0
--------------------------------------------------------------------------}

var curve_id, ipoint_id, points_average, point_id:integer;
    no_of_curves, offset_curve_id, delay_count, point_offset:integer;
    average,sum:real;
    curve_length:word;

{following action defines a delay ----------------------------------------}
procedure delay;
begin
for delay_count := 1 to 1000 do
begin
end;
end;

{main program starts here. Set points_average constant as required--------}
begin
points_average:= 5;

buflen:=32;
writeln();
write('UTILITY TO SMOOTH CURVE SET 0 USING ');
buflen:= 2;
write(points_average);
buflen:= 21;
write('  POINT SLIDING AVERAGE');
delay();
writeln();

no_of_curves:=cs[0].count;

{for the whole curve set--------------------------------------------------}
for curve_id:= 0 to (no_of_curves-1) do
begin
buflen:=15;
writeln();
write('smoothing curve',curve_id);
offset_curve_id:=(curve_id+1);
curve_length:= cs[0].[curve_id].point_count;
point_offset:= (points_average-1)/2;
create_curve(curve_length,0.0,FALSE,cs[0].[offset_curve_id]);

{for each curve, perform smooth------------------------------------------}
{for each smoothed point-------------------------------------------------}
for ipoint_id:=point_offset to (curve_length - point_offset) do
begin
sum:=0;
for point_id:= (ipoint_id - point_offset) to (ipoint_id + point_offset) do
sum:=sum + cs[0].[curve_id].y[point_id];
average:= sum/points_average;
cs[0].[offset_curve_id].y[ipoint_id]:= average;
end;

{for points at start of curve, copy directly without smoothing------------}
for point_id:=0 to (point_offset-1) do
begin
cs[0].[offset_curve_id].y[point_id]:= cs[0].[curve_id].y[point_id];
end;
{for points at end of curve, copy directly without smoothing--------------}
for point_id:=(curve_length-point_offset) to curve_length do
begin
cs[0].[offset_curve_id].y[point_id]:= cs[0].[curve_id].y[point_id];
end;
{now remove the unsmoothed source curve-----------------------------------}
begin
del_curve(cs[0].[curve_id]);
end;
end;

{write end message to screen----------------------------------------------}
writeln();
write('Program Finished');
end.
{end of program-----------------------------------------------------------}
