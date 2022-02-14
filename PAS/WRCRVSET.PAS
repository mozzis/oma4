{ --------------------------------------------------------------------
/
/ program write_curve_set;
/
/ Writes all points from all curves in the specified curveset 
/ into a file.  There must be at least one entry in the curve
/ directory.
/ -------------------------------------------------------------------- }

program wcs;

var i,j,k : integer;
    fp : text;
    CurveTypeStr, CurveName, FileName : string;
    curve_dir_index, curve_count, curve_num, points, 
   INVALID, index : word;
    point, result : real;

{ --------------------------------------------------------------------
/
/ procedure progress_message;
/
/ shows a message on the output line on the screen,
/ telling the user what is going on.
/ -------------------------------------------------------------------- }

procedure progress_message;
begin
    buflen := 5;
    write('Writing ');
    write(points);
    write('points from curve ');
    write(curve_num);
    write('to file, please wait...'); 
    buflen := 20;
end;

{ --------------------------------------------------------------------
/
/ procedure done_message;
/
/ shows a message on the output line on the screen that says DONE.
/ -------------------------------------------------------------------- }

procedure done_message;
begin
    write('Done.');
end;


{ --------------------------------------------------------------------
/
/ procedure AskFileName;
/
/ 
/ 
/ -------------------------------------------------------------------- }

procedure AskFileName;
begin
    writeln();
    write('Enter Filename: ');
    readln(FileName);
end;

{ --------------------------------------------------------------------
/
/ procedure AskCurvename;
/
/ 
/ 
/ -------------------------------------------------------------------- }

procedure AskCurvename;
begin
    writeln();
    write('Enter name of curve for ');
    write(CurveTypeStr);
    write(': ');
    readln(CurveName);
end;

{ --------------------------------------------------------------------
/
/ procedure AskExistCurvename
/
/ -------------------------------------------------------------------- }

procedure AskExistCurvename;
var ask_count:integer;
begin
  ask_count := 0;
  repeat
  begin
    AskCurvename();   { ask for curve name, see if it exists }
    index := G_CURVE_SET_INDEX( CurveName, '', -1 ); 
    ask_count := ask_count + 1;
  end
  until ((index <> INVALID) OR (ask_count > 3));  { don't ask more than 3 times }
end;

{ --------------------------------------------------------------------
/
/ procedure write_points;
/
/ Writes all points from a single curve into a file.
/ Uses the textfile pointer 'fp', and assumes it has
/ already been opened.
/ -------------------------------------------------------------------- }

procedure write_points;
begin
    points := cs[curve_dir_index].[curve_num].point_count; 

    progress_message();

    write(fp, 'Curve: '); 
    writeln(fp, curve_num);
    write(fp, points); 
    writeln(fp, ' Points'); 
    writeln(fp, 'Point             Y Value          X Value');

    points := (points-1); 
      
    for i:=0 to points do 
    begin
        write(fp, i);
        write(fp, cs[curve_dir_index].[curve_num].y[i]); 
        writeln(fp, cs[curve_dir_index].[curve_num].x[i]); 
    end; 
   writeln();
end;

{ --------------------------------------------------------------------
/ program wcs; (main block)
/ -------------------------------------------------------------------- }

begin
  start_fresh();
  buflen := 20;
  INVALID := 65535;

  CurveTypeStr := 'writing to file'; 
  AskExistCurvename();   { ask user which entry from directory to write out }
  curve_dir_index := index;

  if (curve_dir_index <> INVALID) then
  begin
    AskFileName();        { ask user for output file name }
    assign(fp, FileName); { open output file }
    rewrite(fp);

  { find curve count of selected curve set }
    curve_count := cs[curve_dir_index].count;

  { write header of file with name, curve count }
    writeln(fp, CurveName);
    writeln(fp, curve_count, ' Curves');
  { curve indicies are zero-based }
    curve_count := (curve_count-1);
  { write points for all curves }
    for curve_num := 0 to curve_count do 
    begin                         
      write_points();
    end; 

    done_message();

    close(fp); 
  end
end.
