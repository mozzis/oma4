{ --------------------------------------------------------------------
/
/	program read_curve_set;
/
/	Reads points from ASCII file into a curve in the 
/	specified curveset.  The ASCII file is the one written
/	out by the wcs.pas program.
/ -------------------------------------------------------------------- }

program rcs;

var i, j, k, dummy : integer;
    fp : text;
    cs_name, filename : string;
    dir_index, curve_count, curve_num, points : word;
    point, xval, result : real;


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
    
    buflen := 5;
    readln(fp);           { skip curve header }
    readln(fp, points);   { get point count }
    readln(fp);           { skip blank line }
    buflen := 20;
    working_message();

    create_curve(points, 0, FALSE, cs[dir_index].[curve_num]);
    for i:= 0 to points-1 do 
    begin          { read ASCII strings from file, put into variables }
        readln(fp, dummy, point, xval);
	           { now put into curve at appropriate position (index: i) }
        cs[dir_index].[curve_num].y[i] := point; 
        cs[dir_index].[curve_num].x[i] := xval; 
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

    readln(fp);               { skip file name in header }
    readln(fp, curve_count);  { get curve count from file }
    writeln('Curve count is: ', curve_count, '(Press ENTER)');
    readln();

    { create a new curve set for data read in from file }
 
    dir_index := g_curve_set_index('junk.dat','', 0);
    if dir_index = 65535 then begin
      create_curve_set('junk.dat', '', 'Read from ASCII File', 0);
      dir_index := g_curve_set_index('junk.dat','', 0);
    end;
    curve_num := 0;

    for j := 0 to curve_count-1 do begin
      read_points();  { get points from text file }
      curve_num := curve_num + 1;
    end;

    close(fp); 

    writeln();
    write('Now, data will be displayed in plot window');

    plot_points();   { show new data in plot window }

    done_message();
end.
