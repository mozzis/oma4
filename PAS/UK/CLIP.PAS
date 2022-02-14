program clip;
{---------------------------------------------------------------------------
This program reduces each of the curves in curve set 0 in length to that
specified by the difference between the two variables upper_pixel_number
and lower_pixel_number. The main application of it is to allow the user to
shrink data sets collected using a 1024 element intensified diode array
detector to the central 700 or so active pixels, thereby significantly
reducing the data set size and speeding up subsequent calculations.


Author:        Richard Burdett
               EG&G Instruments Ltd, Wokingham, Berks, RG11 2GY

Date:          29th October 1991
Rev:           1.0
--------------------------------------------------------------------------}

var upper_pixel_number, lower_pixel_number, curve_id, ipoint_id:integer;
    opoint_id, no_of_curves, offset_curve_id, delay_count:integer;
    pixel_range:word;

{following action defines a delay ----------------------------------------}
procedure delay;
begin
for delay_count := 1 to 1000 do
begin
end;
end;

{main program starts here. Set upper_pixel_number and lower_pixel_number
as required---------------------------------------------------------------}
begin

upper_pixel_number:= 800;
lower_pixel_number:= 100;

buflen:=65;
writeln();
write('UTILITY TO CLIP THE END (UNINTENSIFIED) REGIONS FROM DATA SETS');
delay();
writeln();

pixel_range:= (upper_pixel_number - lower_pixel_number);
no_of_curves:=cs[0].count;

{for the whole curve set--------------------------------------------------}
for curve_id:= 0 to (no_of_curves-1) do
begin
buflen:=15;
writeln();
write('Clipping curve',curve_id);
offset_curve_id:=curve_id+1;
create_curve(pixel_range,0.0,FALSE,cs[0].[offset_curve_id]);

{for each curve, copy across the selected x and y values------------------}
for ipoint_id:=lower_pixel_number to upper_pixel_number do
begin
opoint_id:=(ipoint_id - lower_pixel_number);
cs[0].[offset_curve_id].X[opoint_id] := cs[0].[curve_id].X[ipoint_id];
cs[0].[offset_curve_id].Y[opoint_id] := cs[0].[curve_id].Y[ipoint_id];
end;
{now remove the long source curve-----------------------------------------}
del_curve(cs[0].[curve_id]);
end;

{write end message to screen----------------------------------------------}
writeln();
write('Program Finished');
end.
{end of program-----------------------------------------------------------}
