PROGRAM STDEV;
VAR
AVG,DEV:REAL;
ICNT, IDX, JCNT, JDX : integer;

BEGIN
  JCNT := DG_J();
  ICNT := DG_TRACKS();
  START_FRESH();
  BUFLEN := 10;
  FOR JDX := 0 TO JCNT DO
  BEGIN
    FOR IDX := 0 TO ICNT DO
    BEGIN
      AVG_STD_DEV(CS[0].[0],1,0,512,AVG,DEV);
      WRITELN();WRITE('AVG =',AVG,'DEV=',DEV);
      READLN();
    END;
  END;
END.
