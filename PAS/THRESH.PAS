Program lthresh;

{ Illustrate trapping of escape key to exit program }
{ Program scans current dataset (assumed to be live) for threshold }
{ If threshold entered is negative, looks for value below abs(thresh) }
{ If threshold entered is positive, looks for value above abs(thresh) }

var thresh : integer;
    trax   : integer;
    poinc  : integer;
    val    : integer;
    key    : integer;
    done   : integer;
    i,j    : word;

begin
  start_fresh(); { run multiple times without exiting command line }
  done := 0;
  i := 0;
  j := 0;
  buflen := 8;
  writeln();
  write('Enter threshold: ');
  readln(thresh);
  trax := dg_tracks() - 1;
  poinc := dg_points() - 1;
  writeln('Working...');

  { Can't use for... loop, no way to break out of it }

  repeat begin
    val := cs[0].[i].y[j];
    if (((thresh < 0) and (val < (-thresh))) or ((thresh >= 0) and (val > thresh)))
      then begin
      writeln('Track ', i, ' Pixel ', j, ' = ', val);
      move_cursor(i, j);
      tag_curve()
    end;
    if keypressed() then begin
      key := readch;
      if key = 129 then done := 1
    end;
    j := j + 1;
    if j > poinc then begin
      i := i + 1;
      j := 0
    end;
  end;
  until ((done > 0) or (i > trax)); { loop till ESC pressed or all data done }
end.
