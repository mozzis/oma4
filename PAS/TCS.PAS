{ --------------------------------------------------------------------
/
/	program tcs;
/
/	Writes all points from all curves in the specified curveset 
/	into a file.
/ -------------------------------------------------------------------- }

program tcs;

var i,j,k : integer;
    fp : text;
    cs_name : string;
    curve_dir_index, max_curve_dir_index, curve_num, points : word;
    point, result : real;

{ --------------------------------------------------------------------
/
/	procedure progress_message;
/
/	shows a message on the output line on the screen,
/	telling the user what is going on.
/ -------------------------------------------------------------------- }

procedure progress_message;
begin
    buflen := 10;
    write('Writing');
    buflen := 5;
    write(points);
    buflen := 15;
    write(' points from curve ');
    buflen := 2;
    write(curve_num);
    buflen := 25;
    write(' to file, please wait...'); 
end;

{ --------------------------------------------------------------------
/
/	procedure done_message;
/
/	shows a message on the output line on the screen that says DONE.
/ -------------------------------------------------------------------- }

procedure done_message;
begin
    buflen := 10;
    write('Done.');
end;


{ --------------------------------------------------------------------
/
/	procedure ask_curve;
/
/	ask the user for which curve set should be written to disk.
/	
/ -------------------------------------------------------------------- }

procedure ask_curve;
begin
    buflen := 25;
    curve_dir_index := -1;
    max_curve_dir_index := cs_count();

    repeat
    begin    
        write('Enter Curve Set Index: ');
        read( curve_dir_index );
        writeln();
    end;
    until ((curve_dir_index >= 0) and (curve_dir_index < max_curve_dir_index))
end;


{ --------------------------------------------------------------------
/
/	procedure write_points;
/
/	Writes all points from a single curve into a file.
/	Uses the textfile pointer 'fp', and assumes it has
/	already been opened.
/ -------------------------------------------------------------------- }

procedure write_points;
begin
    points := cs[curve_dir_index].[curve_num].point_count;

    progress_message();

    writeln(fp);
    buflen := 8;
    write(fp, 'Curve: '); 
    buflen := 2;
    writeln(fp, curve_num);
    buflen := 30;
    writeln(fp, '     Point   Y Value   X Value');

    buflen := 10;
    points := (points-1); 
    for i:=0 to points do 
    begin
        write(fp, i);
        point := cs[curve_dir_index].[curve_num].y[i]; 
        write(fp, point); 
        point := cs[curve_dir_index].[curve_num].x[i]; 
        writeln(fp, point); 
    end; 
    writeln(); 
end;


{ --------------------------------------------------------------------
/	program tcs; (main block)
/ -------------------------------------------------------------------- }

begin
    ask_curve();   { ask user which curve from directory }

    assign(fp, 'tcsout.dat');

    cs_name := cs[curve_dir_index].name;  { get name of selected curve }
    k := cs[curve_dir_index].count;  { curve count of selected curve }

    buflen := 20;
    writeln(fp, cs_name, k);   { write header of file with name, curve count }
    
    k := (k-1); 
    for curve_num := 0 to k do 
    begin                         { write points for all curves }
        write_points();
    end;

    done_message();

    close(fp); 
end.
