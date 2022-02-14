PROGRAM filetest;

var basename, filename       : STRING;
    count0, count1 : BYTE;

  begin
    basename := 'EXPD';
    for count0 := '0' to '1' do begin
      for count1 := '0' to '9' do begin
        filename := basename;
        filename := filename + count0 + count1;
        writeln('Name is: ', filename);
        d_run(0);
        repeat until not dg_Active();
        save_file_curves(filename, cs[0], 0, 1, 0, 6);
      end;
      writeln();
    end;
  end.
