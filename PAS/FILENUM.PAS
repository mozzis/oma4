PROGRAM filetest;

var basename, filename       : STRING;
    count0, count1 : BYTE;

  begin
    basename := 'EXPD';
    for count0 := '0' to '9' do begin
      for count1 := '0' to '9' do begin
        filename := basename;
        filename := filename + count0 + count1;
        writeln('Name is: ', filename);
      end;
      writeln();
    end;
  end.
