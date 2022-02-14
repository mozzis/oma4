PROGRAM SLICE;
{---------------------------------------------------------------------------
 this program performs an intensity against position analysis of the curve
 set at curve directory index 0 along a chord defined between two x,z
 coordinates which the user is prompted to enter as the program runs. The
 resulting data is stored in a curve set of the form SLICE.DAT at location
 1 in the curve directory index, which must be free before running the
 program. On completion, the user can go to the curve directory and
 rename and save the yt data as required.

 Author:       Richard Burdett
               EG&G Instruments Ltd, Wokingham, Berks, RG11 2GY

 Date:         7th April 1992
 Rev:          1.0
 Updates:      22nd July 1992
               Slight modifications and cosmetic improvements.


---------------------------------------------------------------------------}


VAR MEMORY_NUM, T, SLICE_CV_LEN, SLICE_CV_POINT                : INTEGER;
    CORRECTION, X_1_COORD, X_2_COORD, Z_1_COORD, Z_2_COORD     : INTEGER; 
    X_INDEX, CURVE_SET_MAX, CHORD_SLOPE                        : REAL;
    X_MAX                                                      : WORD;
    

{following section defines a delay-----------------------------------------}
PROCEDURE DELAY;
  BEGIN
   FOR T:=1 TO 1500 DO
    BEGIN
    END;
  END;

{following sets up default plotting parameters for source data-------------}
PROCEDURE DEFAULT_PLOT;
  BEGIN
   S_PLOT_STYLE(0.0,cs[0].[0].point_count,0.0,curve_set_max,0.0,cs[0].COUNT,
              0,0,20,
              'SOURCE DATA FROM C/S 1',
              'X-AXIS',
              'INTENSITY',
              'Z-AXIS',
              1,20,20,1,1,1,1);
  END;

{following sets up plot format for slice data-----------------------------}
PROCEDURE SLICE_PLOT;
  BEGIN
   S_PLOT_STYLE(0.0,X_MAX,0.0,cs[1].[0].max,0.0,1.0,
                0,20,0,
                'SLICE OF CURVE SET 0',
                'POSITION ALONG CHORD',
                'INTENSITY',
                '',
                0,20,20,1,1,1,1);
  END;

{main program starts here--------------------------------------------------}
  BEGIN
   ENTER_GRAPH();
   BUFLEN:= 60;
   WRITELN();
   WRITE('UTILITY TO PERFORM SLICE ANALYSIS OF THE DATA IN CURVE SET 0');
   DELAY();
   BUFLEN:= 57;
   WRITELN();
   WRITE('THE RESULTING CURVE IS STORED TO CURVE SET 1 AS SLICE.DAT');
   DELAY();

{now find out number of curves in curve-set 1-----------------------------}
   X_MAX:=CS[0].COUNT;

{prompt the user for the required coordinates defining chord--------------}
   BUFLEN:= 53;
   WRITELN();
   WRITE('PLEASE ENTER THE FIRST COORDINATE  "x" VALUE... ');
   READ(X_1_COORD);
   WRITE(' & "z" VALUE... ');
   READ(Z_1_COORD);
   WRITELN();
   WRITE('PLEASE ENTER THE SECOND COORDINATE "x" VALUE... ');
   READ(X_2_COORD);
   WRITE(' & "z" VALUE... ');
   READ(Z_2_COORD);
   WRITELN();

   CHORD_SLOPE:= ((X_2_COORD - X_1_COORD)/(Z_2_COORD -Z_1_COORD));
   SLICE_CV_LEN:= ABS((Z_2_COORD - Z_1_COORD))+1;
   CORRECTION:=1;
   IF (Z_2_COORD-Z_1_COORD) < 0 THEN CORRECTION:= -1;

{check to see if curve_set 1 already exists,and delete if it does---------}
   IF (CS_COUNT() > 1) THEN DEL_CURVE_SET(CS[1]);

{now create destination curve set Y_T.DAT and fill with 1 curve of zeros--}
   CREATE_CURVE_SET('SLICE.DAT','.\','INTENSITY ALONG CHORD',1);
   CREATE_CURVE(SLICE_CV_LEN,0.0,0,CS[1].[0]);

{perform slice analysis---------------------------------------------------}
   FOR SLICE_CV_POINT:= 0 TO (SLICE_CV_LEN - 1) DO
   BEGIN
    X_INDEX:= ROUND(X_1_COORD + CORRECTION*(SLICE_CV_POINT*CHORD_SLOPE));
    MEMORY_NUM:=Z_1_COORD + (CORRECTION*SLICE_CV_POINT);
    WRITELN();
    WRITE('WORKING....');
    CS[1].[0].Y[SLICE_CV_POINT]:= CS[0].[MEMORY_NUM].Y[X_INDEX];
   END;

WRITELN();

{stop displaying curve set 0 in plot window 1-----------------------------}
  CS[0].DISPLAY:=0;

{now turn on display of curve set 1 in plot window 1, and plot data-------}
  CS[1].DISPLAY:=1;
  SLICE_PLOT();
  AUTO_SCALE();

{prompt user for any key to clear the screen and go on -------------------}
  BUFLEN:=23;
  WRITELN();
  WRITE('PRESS <ENTER> TO GO ON');
  READLN();

{replot data in curve set 0-----------------------------------------------}
  CS[1].DISPLAY:=0;
  CS[0].DISPLAY:=1;
  DEFAULT_PLOT();
  AUTO_SCALE();
  BUFLEN:=46;
  WRITELN();
  WRITE ('End of Program');

END.

{end of program-----------------------------------------------------------}
