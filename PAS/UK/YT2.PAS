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
---------------------------------------------------------------------------}


VAR MEMORY_NUM, PLOT_WIND, curve, max_curve           : INTEGER;
    INTENSITY, X_INDEX, STEP_TIME, curve_set_max,TEMP : REAL;
    REPLY, PLOT_TITLE_YT, CurveName, X_INDEX_STRING   : STRING;
    X_MAX, X_WIDTH, J, SOURCE_INDEX, DEST_INDEX       : INTEGER;

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
 MAX_CURVE:=CS[SOURCE_INDEX].COUNT;
 FOR CURVE:= 0 TO (MAX_CURVE-1) DO
  BEGIN
  IF CS[SOURCE_INDEX].[CURVE].MAX > CURVE_SET_MAX THEN CURVE_SET_MAX:=
     CS[SOURCE_INDEX].[CURVE].MAX;
  END;

  S_PLOT_STYLE(0.0,                             { xmin            }
              cs[SOURCE_INDEX].[0].point_count, { xmax            }
              0.0,                              { ymin            }
              curve_set_max,                    { ymax            }
              0.0,                              { zmin            }
              cs[SOURCE_INDEX].COUNT,           { zmax            }
              0,                                { xunits          }
              0,                                { yunits          }
              20,                               { zunits (was 20) }
              'SOURCE DATA FROM C/S 1',         { title           }
              'WAVELENGTH',                     { xlabel          }
              'INTENSITY',                      { ylabel          }
              'TIME',                           { zlabel          }
              1,                                { zside           }
              20,                               { zx%             }
              20,                               { zy%             }
              1,                                { linestyle       }
              FALSE,                            { autox           }
              FALSE,                            { autoy           }
              FALSE);                           { autoz           }
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
               'Y-T OF CURVE SET 0 AT X',
               'TIME',
               'INTENSITY',
               '',
               0,
               20,20,
               1,
               FALSE,FALSE,FALSE);  
END;

{main program starts here--------------------------------------------------}
BEGIN
  START_FRESH();
  ENTER_GRAPH();
  BUFLEN:= 60;
  WRITELN();
  WRITE('UTILITY TO PERFORM Y-T ANALYSIS OF THE CURVE SET DATA');
  DELAY();
  BUFLEN:= 57;
  WRITELN();
  WRITE('THE RESULTING CURVE IS STORED AS Y_T.DAT');
  DELAY();

  WRITELN();
  buflen := 5;
  WRITE('Enter the name of the curve to analyze: ');
  buflen := 30;
  READLN(CurveName);
  writeln('checking source curve...');
  SOURCE_INDEX := G_CURVE_SET_INDEX(CurveName, '', -1);
  IF SOURCE_INDEX <> -1 THEN BEGIN
  {check to see if curve_set 2 already exists,and delete if it does---------}
    DEST_INDEX := G_CURVE_SET_INDEX('Y_T.DAT', '', -1);
    IF DEST_INDEX <> -1 THEN BEGIN
      DEL_CURVE_SET(CS[DEST_INDEX]);
    END;

  {now find out number of curves in curve-set 1-----------------------------}
    X_MAX:=CS[SOURCE_INDEX].COUNT;

  {prompt the user for the required x-pixel position------------------------}
    BUFLEN:= 53;
    WRITELN();
    WRITE('PLEASE ENTER THE X-PIXEL POSITION FOR Y-T ANALYSIS ');
    READ(X_INDEX);
    WRITELN();
    WRITE('PLEASE ENTER THE AREA WIDTH FOR Y-T ANALYSIS ');
    READ(X_WIDTH);
    X_WIDTH := X_WIDTH / 2;
    WRITELN();

  {now create destination curve set Y_T.DAT and fill with 1 curve of zeros--}
    CREATE_CURVE_SET('Y_T.DAT','','INTENSITY AGAINST TIME',1);
    DEST_INDEX := G_CURVE_SET_INDEX('Y_T.DAT', '', -1);
    writeln('Dest index is: ', DEST_INDEX);
    readln();
    CREATE_CURVE(X_MAX,0.0,0,CS[DEST_INDEX].[0]);
    writeln('Made output curve');
    readln();

  {perform y-t analysis-----------------------------------------------------}
    FOR MEMORY_NUM:= 0 TO (X_MAX-1) DO BEGIN
      WRITELN('Memory ',MEMORY_NUM);
      READLN();
      TEMP := 0;
      FOR J := (X_INDEX - X_WIDTH) TO (X_INDEX + X_WIDTH) DO BEGIN
        TEMP := TEMP + CS[SOURCE_INDEX].[MEMORY_NUM].Y[J];
        END;
      CS[DEST_INDEX].[0].Y[MEMORY_NUM]:= TEMP / X_WIDTH;
    END;

  {stop displaying curve set 0 in plot window 1-----------------------------}
    CS[SOURCE_INDEX].DISPLAY:=0;

  {now turn on display of curve set 1 in plot window 1, and plot data-------}
    CS[DEST_INDEX].DISPLAY:= 1;
    Y_T_PLOT();
    REPLOT_WINDOW();
    {AUTO_SCALE();}

  {prompt user for any key to clear the screen and go on -------------------}
    BUFLEN:=23;
    WRITELN();
    WRITE('PRESS <ENTER> TO GO ON');
    READLN();

  {replot data in curve set 0-----------------------------------------------}
    CS[DEST_INDEX].DISPLAY:=0;
    DEFAULT_PLOT();
    {AUTO_SCALE();}
    REPLOT_WINDOW();
  END;               { if source curve found }
  BUFLEN:=46;
  WRITELN();
  WRITE ('End of Program');
END.

{end of program-----------------------------------------------------------}
