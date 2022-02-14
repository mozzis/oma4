PROGRAM P_AREA;

VAR SAREA : REAL;
    INDX, NCRV, FPNT, NPNT, I : WORD;
    CNAME : STRING;

BEGIN
  BUFLEN:=10;
  SAREA := 0;
  WRITELN();
  WRITE('Enter the curve set name: ');
  READLN(CNAME);
  INDX := G_CURVE_SET_INDEX(CNAME, '', 0);
  IF INDX <> -1 THEN BEGIN
    WRITE('Enter the curve number: ');
    READLN(NCRV);
    WRITE('Enter the first point: ');
    READLN(FPNT);
    WRITE('Enter the number of points: ');
    READLN(NPNT);
    FOR i := FPNT to FPNT+NPNT DO
      SAREA := CS[INDX].[NCRV].Y[i] + SAREA;
    WRITELN();
    WRITELN('Area is: ',SAREA,' (Done)');
  END;
END.
