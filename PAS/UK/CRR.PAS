program crr;   { 'Cosmic Ray remover' MLM 1-4-92 }
               { Replace single pixel 'events' in a data set }
               { such as would be caused by cosmic rays. To use, load and }
               { display the curve, press Alt-F10 and type RUN('CRR.PAS') }

var cvindex, i, j, tracks, points, dummy : integer;
    fname                                : string;
    threshold, average, deviation        : real;
    lval, rval, pval                     : real;

begin
  start_fresh();
  buflen := 10;
  writeln();
  write('Enter the curve set name: ');
  readln(fname);
  cvindex := G_Curve_Set_Index(fname,'',0);   { find the curve }
  if cvindex = -1 then begin
    writeln();
    write('Curve set not found: ',fname,' (stopped)');
  end
  else begin 
    writeln();
    write('Enter the threshold value ');
    readln(threshold);
    tracks := cs[cvindex].COUNT;
    auto_scale();
    writeln();
    write('Working...');
    for j := 0 to (tracks - 1) do begin
      points := cs[cvindex].[j].POINT_COUNT;
      AVG_STD_DEV(cs[cvindex].[j],1,0,points,average,deviation);
      deviation := deviation * 5;           { adjust if > 5 sigmas }
      for i := 1 to (points - 1) do begin
        if cs[cvindex].[j].y[i] > threshold then begin
          lval := cs[cvindex].[j].Y[i-1];
          rval := cs[cvindex].[j].Y[i+1];
          pval := cs[cvindex].[j].Y[i];

          if ((pval - lval) > deviation) or ((pval - rval) > deviation)
          then begin
            writeln();
            write('Replacing pixel at ',i);
            cs[cvindex].[j].Y[i] :=
            ((cs[cvindex].[j].Y[i-1] + cs[cvindex].[j].Y[i+1]) / 2);
          end;
        end;
      end;
    end;
    auto_scale();                          { show the result }
    write(' Done.');
  end;
end.
