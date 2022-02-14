PROGRAM LiveMulti;

{ This program will acquire a number of curve sets and save them to disk }
{ Each curve set is saved with a filename starting with EXPD and ending  }
{ with an integer between '00' and '19' for a total of twenty curves     }

var basename, filename       : STRING;
    count0, count1 : BYTE;
    liveindex, traknum, pntnum : INTEGER;

  begin
    basename := 'EXPD';
    liveindex :=
      G_CURVE_SET_INDEX('lastlive','',0);    { See if lastlive exists }
    if i = -1 then begin                     { if not, }
      S_LIVE(0);                             { set to live mode, }
      GO_LIVE();                             { start acquisition, }
      for j := 0 to 1000 do;                 { wait briefly, and stop. }
      STOP_LIVE();                           { This creates laslive. }
      liveindex :=
        G_CURVE_SET_INDEX('lastlive','',0);  { Now get its index }
    end;
    traknum := DG_TRACKS();
    pntnum := DG_POINTS();
    for count0 := '0' to '1' do begin
      for count1 := '0' to '9' do begin
        filename := basename;
        filename := filename + count0 + count1;
        writeln('Name is: ', filename);
        d_run(0);
        repeat until not dg_Active();
        save_file_curves(filename, cs[i], 0, traknum, 0, 6);
      end;
      writeln();
    end;
  end.
