PROGRAM filetest;
{ Acquire a bunch of curves and store them as ascii files }

var basename, filename  : STRING;    { strings }
    count0, count1      : BYTE;      { counters }

  begin
    s_accum(FALSE);                     { set accum mode with no background }
    basename := 'EXPD';                 { start all names with EXPD }
    for count0 := '0' to '1' do begin   { go EXPD00, 01,...10, 11,...EXPD19 }
      for count1 := '0' to '9' do begin
        filename := basename;           { copy base to filename }
        filename := filename + count0 + count1;  { append number chars }
        writeln('Name is: ', filename); { tell the user }
        go_live();                      { acquire a curve }

        { save live data (assume cs[0]), start at curve 0, save 1 curve, }
        { dest start curve= 0 (always!), format = 6 (Multicolumn with X) }

        save_file_curves(filename, cs[0], 0, 1, 0, 6);
      end;
      writeln('Done!');
    end;
  end.
