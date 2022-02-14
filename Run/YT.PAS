PROGRAM YT;
{---------------------------------------------------------------------------
 this program performs an intensity against time analysis of the curve set
 at curve directory index 0 at the user-specified x-pixel position. The
 resulting data is stored in a curve set of the form YT.DAT at location
 1 in the curve directory index, which must be free before running the
 program. On completion, the user can go to the curve directory and
 rename and save the yt data as required.

 Author:       Richard Burdett
               EG&G Instruments Ltd, Wokingham, Berks, RG11 2GY

 Date:         14th June 1991
 Rev:          0.9
 Updates:      22nd October 1991. XMAX redefined as X_MAX: XMAX is
               now a reserved word in OMA 2000.

               25th October 1991. Major revision including:-
               DEFAULT_PLOT now is scaled to match size of data set
               Y_T_PLOT now is scaled to match output data
               Y_T.DAT is now always saved and the program clears
               curve set 1 before it runs

               22 September 1993. (Morris Maynard) Prompt user for
               source file, stop program if it does not exist; Check
               to see if Y_T.PAS exists, and if it does not, create
               it. Also prompt for width of area; program sums the
               points in the area into the output YT curve.
---------------------------------------------------------------------------}


VAR MEMORY_NUM, PLOT_WIND, CURVE, MAX_CURVE            : INTEGER;
    X_MAX, X_WIDTH, J, SOURCE_INDEX, DEST_INDEX        : INTEGER;
    KEY                                                : BYTE;
    INTENSITY, X_INDEX, STEP_TIME, SET_MAX, TEMP       : REAL;
    REPLY, PLOT_TITLE_YT, CurveName, X_INDEX_STRING    : STRING;
    OUTPUT_NAME, CMDLINE                               : STRING;

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
  CMDLINE := GCMD_PROMPT();
  SCMD_PROMPT('Scaling for plot, press ESC to abort...');
  SET_MAX := 0;
  MAX_CURVE :=CS[SOURCE_INDEX].COUNT;
  FOR CURVE := 0 TO (MAX_CURVE-1) DO BEGIN
    TEMP := CS[SOURCE_INDEX].[CURVE].MAX;
    WRITELN('Memory ', CURVE);
    IF TEMP > SET_MAX THEN 
      SET_MAX := TEMP;
    IF KEYPRESSED() THEN BEGIN
      IF READCH() = 129 THEN CURVE := MAX_CURVE;
    END;
  END;
  SCMD_PROMPT(CMDLINE);

  S_PLOT_STYLE(0.0,                             { xmin            }
              cs[SOURCE_INDEX].[0].point_count, { xmax            }
              0.0,                              { ymin            }
              SET_MAX,                          { ymax            }
              0.0,                              { zmin            }
              cs[SOURCE_INDEX].COUNT,           { zmax            }
              0,                                { xunits          }
              0,                                { yunits          }
              20,                               { zunits (was 20) }
              'SOURCE DATA FOR YT',             { title           }
              'WAVELENGTH',                     { xlabel          }
              'INTENSITY',                      { ylabel          }
              'TIME',                           { zlabel          }
              1,                                { zside           }
              20,                               { zx%             }
              20,                               { zy%             }
              1,                                { linestyle       }
              TRUE,                             { autox           }
              TRUE,                             { autoy           }
              TRUE);                            { autoz           }
 CS[SOURCE_INDEX].DISPLAY:=1;
END;

{following sets up plot format for YT data--------------------------------}
PROCEDURE Y_T_PLOT;
BEGIN
  S_PLOT_STYLE(0.0,
               X_MAX,
               0.0,
               cs[DEST_INDEX].[0].max,
               0.0,
               1.0,
               0,
               20, { was 20 }
               0,
               'Y-T OF CURVE SET AT X',
               'TIME',
               'INTENSITY',
               '',
               0,
               20,20,
               1,
               TRUE,TRUE,TRUE);  
END;

{main program starts here----------------------------------------------------}
BEGIN
  OUTPUT_NAME := 'Y_T.DAT';
  ENTER_GRAPH();
  BUFLEN:= 60;
  WRITELN();
  WRITE('Utility to perform Y-T analysis of the curve set data ');
  DELAY();
  BUFLEN:= 57;
  WRITELN();
  WRITE('The resulting curve is stored as Y_T.DAT ');
  DELAY();

  WRITELN();
  buflen := 5;
  WRITE('Enter the name of the curve to analyze: ');
  buflen := 30;
  READLN(CurveName);
  SOURCE_INDEX := G_CURVE_SET_INDEX(CurveName, '', 0);
  IF SOURCE_INDEX <> -1 THEN BEGIN

    {now find out number of curves in curve-set 1----------------------------}
    X_MAX := CS[SOURCE_INDEX].COUNT;
    
    {check to see if curve_set 2 already exists,and delete if it does--------}
    DEST_INDEX := G_CURVE_SET_INDEX(OUTPUT_NAME, '', 0);
    
    IF DEST_INDEX <> -1 THEN BEGIN  
      DEL_CURVE_SET(CS[DEST_INDEX]);
    END;                            

  {now create destination curve set Y_T.DAT and fill with 1 curve of zeros---}
    CREATE_CURVE_SET(OUTPUT_NAME, '', '', 0);
    DEST_INDEX := G_CURVE_SET_INDEX(OUTPUT_NAME, '', 0);
    SOURCE_INDEX := G_CURVE_SET_INDEX(CurveName, '', 0);
  
    IF DEST_INDEX <> -1 THEN BEGIN
      CREATE_CURVE(X_MAX, 0, FALSE, CS[DEST_INDEX].[0]);
  
    {prompt the user for the required x-pixel position-----------------------}
   
      BUFLEN:= 53;
      WRITELN();
      WRITE('Please enter the center X-pixel position for Y-T analysis: ');
      READ(X_INDEX);
      WRITELN();
      WRITE('Please enter the area width for Y-T analysis: ');
      READ(X_WIDTH);
      X_WIDTH := X_WIDTH / 2;
      WRITELN();

    {perform y-t analysis---------------------------------------------------}
      CMDLINE := GCMD_PROMPT();
      SCMD_PROMPT('Converting Data, Press ESC to abort');
      FOR MEMORY_NUM:= 0 TO (X_MAX-1) DO BEGIN
        WRITELN('Memory ',MEMORY_NUM);
        TEMP := 0;
        FOR J := (X_INDEX - X_WIDTH) TO (X_INDEX + X_WIDTH) DO BEGIN
          TEMP := TEMP + CS[SOURCE_INDEX].[MEMORY_NUM].Y[J];
        END;
        CS[DEST_INDEX].[0].Y[MEMORY_NUM] := TEMP / X_WIDTH;
        IF KEYPRESSED() THEN BEGIN
          IF READCH() = 129 THEN MEMORY_NUM := X_MAX;
        END;
      END;
      SCMD_PROMPT(CMDLINE);

    {stop displaying curve set 0 in plot window 1----------------------------}
      CS[SOURCE_INDEX].DISPLAY := 0;

    {now turn on display of curve set 1 in plot window 1, and plot data------}
      CS[DEST_INDEX].DISPLAY := 1;
      Y_T_PLOT();
      REPLOT_WINDOW();
      {AUTO_SCALE();}

    {prompt user for any key to clear the screen and go on -------------------}
      BUFLEN:=23;
      WRITELN();
      WRITELN('PRESS <ENTER> TO GO ON');
      READLN();

    {replot data in curve set 0-----------------------------------------------}
      CS[DEST_INDEX].DISPLAY := 0;
      DEFAULT_PLOT();
      WRITELN('Replotting, press ESC to abort...');
      {AUTO_SCALE();}
      REPLOT_WINDOW();
    END;             { if dest curve created }
  END;               { if source curve found }
  BUFLEN := 46;
  WRITELN();
  WRITE ('End of Program');
END.

{end of program-----------------------------------------------------------}
