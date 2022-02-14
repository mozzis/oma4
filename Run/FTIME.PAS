Program FTIME;

  var start, stop, diff : real;
      reps : integer;

  begin
    START_FRESH();
    BUFLEN := 8;
    S_ACCUM(FALSE);
    WRITELN('Running...');
    D_RUN(0);
    start := NOW();
    repeat until DG_ACTIVE() = 0;
    stop := NOW();
    reps := DG_I() * DG_J();
    diff := ((DG_FTIME() * reps) - (stop - start)) / reps;
    writeln('Run Time: ',stop - start, ' Diff: ',diff);
  end.
