PROGRAM scomp;

VAR outfile : TEXT;
    i, imax, j, jmax : integer;

begin
  assign(outfile, 'PRN');
  rewrite(outfile);

  imax := (cs[0].count);
  imax := imax - 1;

  jmax := dg_tracks();
  jmax := jmax - 1;

  i := 0;

  while i < imax do begin
    buflen := 2;
    writeln(outfile, 'Memory: ',i);
    for j := 0 to jmax do begin
      index := j + i;
      buflen := -4;
      write(outfile, '  Track: ', j);
      buflen := -8;
      writeln(outfile, '  source comp: ',cs[0].[index].src_comp);
    end;
    writeln(outfile);
    i := i + jmax + 1;
  end;
  close(outfile);
  start_fresh();
end.
