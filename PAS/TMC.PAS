{ --------------------------------------------------------------------
/
/	program tmc;
/
/	Writes all points from the specified memory curve
/	into a file.
/ -------------------------------------------------------------------- }

program tmc;

var i,j,k : integer;
    fp : text;
    curve_num, points : word;
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
/	procedure write_points;
/
/	Writes all points from a single curve into a file.
/	Uses the textfile pointer 'fp', and assumes it has
/	already been opened.
/ -------------------------------------------------------------------- }

procedure write_points;
begin		{ find out how many points in curve }
    points := mc[curve_num].point_count;

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
        writeln();
        write( 'point ', i);
        write(fp, i);
        point := mc[curve_num].y[i]; 
        write(fp, point); 
        point := mc[curve_num].x[i]; 
        writeln(fp, point); 
    end; 
    writeln(); 
end;




{ --------------------------------------------------------------------
/	program tmc; (main block)
/ -------------------------------------------------------------------- }

begin
    assign(fp, 'tmcout.dat');
    
    write_points();

    done_message();

    close(fp); 
end.

