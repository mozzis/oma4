program cr;   { 'Cosmic Ray remover' MLM 1-4-92 }
              { Replace single pixel 'events' in a data set }
              { such as would be caused by cosmic rays. To use, load and }
              { display the curve sets, press Alt-F10 and type }
              { RUN('CRR.PAS') }

var cvsrc1, cvsrc2, cvdest, i, j, tracks, pixels : integer;
    fsrc1, fsrc2, fdest                          : string;

begin
  start_fresh();
  buflen := 10;
  writeln();
  write('Enter the first source name: ');
  readln(fsrc1);
  cvsrc1 := G_Curve_Set_Index(fsrc1,'',0);   { find the curve }
  if cvsrc1 = -1 then begin
    writeln();
    write('Curve set not found: ',fsrc1,' (stopped)');
  end
  else begin 
    write('Enter the second source name: ');
    readln(fsrc2);
    cvsrc2 := G_Curve_Set_Index(fsrc2,'',0);   { find the curve }
    if cvsrc2 = -1 then begin
      writeln();
      write('Curve set not found: ',fsrc2,' (stopped)');
    end
    else begin 
      write('Enter the destination name: ');
      readln(fdest);
      cvdest := G_Curve_Set_Index(fdest,'',0);   { find the curve }
      if cvdest <> -1 then 
        DEL_CURVE_SET(cvdest);

      CREATE_CURVE_SET(fdest, '', 'Result of CR function', 0);
      cvdest := G_Curve_Set_Index(fdest,'',0);   { find the curve }
      cvsrc1 := G_Curve_Set_Index(fsrc1,'',0);   { find the curve }
      cvsrc2 := G_Curve_Set_Index(fsrc2,'',0);   { find the curve }

      tracks := CS[cvsrc1].COUNT;
      
      { if CS[cvsrc2].COUNT > tracks then 
        tracks := CS[cvsrc2].COUNT; }

      { choose the minimum Y of the two source curves for the dest curve }

      writeln(' Wait...');
      for i := 0 to tracks -1 do begin
        pixels := CS[cvsrc1].[i].POINT_COUNT;
        if CS[cvsrc2].[i].POINT_COUNT > pixels then 
          pixels := CS[cvsrc2].[i].POINT_COUNT;
        CREATE_CURVE(pixels, 0.0, TRUE, CS[cvdest].[i]);
        for j := 0 to pixels - 1 do begin
          CS[cvdest].[i].X[j] := CS[cvsrc1].[i].X[j]; { copy X coordinate }
          CS[cvdest].[i].Y[j] := CS[cvsrc1].[i].Y[j];
          if CS[cvdest].[i].Y[j] > CS[cvsrc2].[i].Y[j] then
            CS[cvdest].[i].Y[j] := CS[cvsrc2].[i].Y[j];
        end;  { points }
      end; { tracks }
      
      CS[cvsrc1].DISPLAY := 0;
      CS[cvsrc2].DISPLAY := 0;
      CS[cvdest].DISPLAY := 15;
      auto_scale();                          { show the result }
      write(' Done.');
    end; { got second curve set }
  end; { got first curve set }
end.
