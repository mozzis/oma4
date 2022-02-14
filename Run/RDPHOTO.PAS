{ --------------------------------------------------------------------
/
/	program read_curve_set;
/
/	Reads points from ASCII file into a curve in the 
/	specified curveset.  The ASCII file is the one written
/	out by the wcs.pas program.
/ -------------------------------------------------------------------- }

program rcs;

var i : integer;
    fp : text;
    test_string, filename  : string;
    L_byte, E_byte, N_byte : byte;
    test_array : array[80] of byte;
    dir_index, curve_num, points : word;
    point : real;


{ --------------------------------------------------------------------
/
/	procedure done_message;
/
/	shows a message on the output line on the screen that says DONE.
/ -------------------------------------------------------------------- }

procedure done_message;
begin
    writeln();
    write('Done.');
end;

{ --------------------------------------------------------------------
/
/	procedure working_message;
/
/	shows a message on the output line on the screen that 
/	indicates work is in progress.
/ -------------------------------------------------------------------- }

procedure working_message;
begin
    buflen := 5;
    writeln();
    write('Reading ', points, 'points from curve ', curve_num+1);
end;

{ --------------------------------------------------------------------
/
/	procedure read_points;
/
/	reads points from ASCII text file into a curve in the 
/	specified curveset.
/ -------------------------------------------------------------------- }

procedure read_points;
begin
               { find out how many points are in curve }
    L_byte := 'L';
    E_byte := 'E';
    N_byte := 'N';
    buflen := 5;
    done := FALSE;
    repeat
      readln(fp, test_string, points);
      test_array := test_string;
    until test_array[0] = L_byte and
          test_array[1] = E_byte and
          test_array[2] = N_byte;

    buflen := 20;
    create_curve(points, 0, FALSE, cs[dir_index].[curve_num]);
    working_message();
    for i:= 0 to points-1 do 
    begin          { read ASCII strings from file, put into variables }
        readln(fp, point);
	           { now put into curve at appropriate position (index: i) }
        cs[dir_index].[curve_num].y[i] := point; 
        cs[dir_index].[curve_num].x[i] := i; 
    end; 
end;

{ --------------------------------------------------------------------
/
/	procedure plot_points;
/
/	Opens a new plot window and draws our data into it.
/ -------------------------------------------------------------------- }

procedure plot_points;
begin
     { set new curveset to display in window one - NOTE: bit 1, not number 1 }
    cs[dir_index].display := 1;  { bit value }

     { now draw window one }    
    plot_window('SCREEN', 1);
end;

{ --------------------------------------------------------------------
/	program rcs; (main block)
/ -------------------------------------------------------------------- }

begin
    start_fresh();
    buflen := 20;
    { open ASCII text file with curve data }
    writeln();
    write('Enter ascii data file name to read: ');
    readln(filename);

    assign(fp, filename);
    reset(fp);   { rewind to beginning of file }

    { create a new curve set for data read in from file }
 
    dir_index := g_curve_set_index(filename, '', 0);
    if dir_index = 65535 then begin
      create_curve_set(filename, '', 'Read from ASCII File', 0);
      dir_index := g_curve_set_index(filename, '', 0);
    end;
    curve_num := 0;

    read_points();  { get points from text file }

    close(fp); 

    writeln();
    write('Now, data will be displayed in plot window');

    plot_points();   { show new data in plot window }

    done_message();
end.
