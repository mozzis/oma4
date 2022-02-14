PROGRAM PIATEST;

VAR I, J, K, OUTVAL : INTEGER;

BEGIN
  start_fresh();
  WRITELN();
  WRITE('Enter Delay: ');
  READLN(K);
  WRITELN();
  while (KEYPRESSED() = 0) do begin
    for I := 0 TO 7 DO BEGIN
      OUTVAL := EXP(2,I);
      WRITELN(OUTVAL);
      DS_OUTPIA(OUTVAL);
    for J := 0 to K DO BEGIN END;
    end;
  end;
  readch();
END.
