PROGRAM LYT;
{---------------------------------------------------------------------------
This macro performs an intensity against time analysis of lastlive as data
is acquired, plotting lastlive and the YT curve to screen.

The user is prompted for the lower and upper pixel positions defining an area 
which is required to be monitored over time. He is then prompted for
the name of a curve set into which this YT curve should be stored; if this
curve set already exists he is prompted as to whether it should be 
overwritten.

Finally he is prompted for the number of YT points to store.

Data acquisition then commences as dictated by the current run setup 
paramaters. Each point is acquired after an Accum run and stored to the
YT curve set, until the full set of points has been measured. In this 
version once started there is no facility for aborting a data acquisition.

Both the source data (lastlive) and the YT data are plotted in live mode.

Date:        23rd September 1993
Author:      Richard Burdett, EG&G Instruments, Wokingham, Berks.
Revision:    0.6

-----------------------------------------------------------------------------}
VAR T_COUNT, CURVE, MAX_CURVE, SOURCE_INDEX,
    YT_INDEX, OUT_FILE_OK, YES_FLAG, CURVE_SET_COUNT, AREA, I    : INTEGER;
    X_INDEX_L, X_INDEX_U, CURVE_SET_MAX, STTIME, OTIME, NTIME    : REAL;
    INPUT_NAME, OUTPUT_NAME, PROMPT_TEXT, INPUT                  : STRING;
    YT_POINTS, NO_POINTS                                         : WORD;
    GO_ON, UPPER_N, LOWER_N, UPPER_Y, LOWER_Y, FIRST_CHAR        : BYTE;

    AutoScaleZ, AutoScaleY, AutoScaleX     : BOOLEAN;
    LineStyle, ZYPercent, ZXPercent, ZSide : INTEGER;
    ZLabel, YLabel, XLabel, PlotTitle      : STRING;
    ZUn, YUn, XUn                          : WORD;
    MaxZ, MinZ, MaxY, MinY, MaxX, MinX     : REAL;


{defines delay---------------------------------------------------------------}
PROCEDURE DELAY;
BEGIN
 FOR I:= 1 to 10000 DO
  BEGIN
  END;
END;

{writes text to screen-------------------------------------------------------}
PROCEDURE WRITE_TEXT;
BEGIN
 BUFLEN:= STRLEN(PROMPT_TEXT);
 WRITELN();
 WRITE(PROMPT_TEXT);
END;

{prompt user for yes/no response to prompt_text------------------------------}
PROCEDURE ASK_IF_YES;
BEGIN
 WRITE(PROMPT_TEXT);
 BUFLEN:=10;
 WRITE('?  <Y/N> ');
 READ(INPUT);
 GO_ON:=INPUT;
 UPPER_Y:='Y';
 LOWER_Y:='y';
 IF ((GO_ON=UPPER_Y)OR(GO_ON=LOWER_Y)) THEN YES_FLAG:=1;
 UPPER_N:='N';
 LOWER_N:='n';
 IF ((GO_ON=UPPER_N)OR(GO_ON=LOWER_N)) THEN YES_FLAG:=0;
 WRITELN();
END;

{following sets up default plotting parameters for source data---------------}
PROCEDURE DEFAULT_PLOT;
BEGIN
 CS[SOURCE_INDEX].DISPLAY:=1;
 MAX_CURVE:=CS[SOURCE_INDEX].COUNT;
 FOR CURVE:= 0 TO (MAX_CURVE-1) DO
  BEGIN
  IF CS[SOURCE_INDEX].[CURVE].MAX > CURVE_SET_MAX THEN CURVE_SET_MAX:=
     CS[SOURCE_INDEX].[CURVE].MAX;
  END;
 SELECT_WINDOW(1);
 G_PLOT_STYLE(MinX, MaxX, MinY, MaxY, MinZ, MaxZ, 
              XUn, YUn, ZUn, 
              PlotTitle, XLabel, YLabel, ZLabel, 
              ZSide, ZXPercent, ZYPercent, LineStyle, 
              AutoScaleX, AutoScaleY, AutoScaleZ);

 S_PLOT_STYLE(MinX, MaxX, 
              MinY, CURVE_SET_MAX, MinZ, MaxZ, 
              0, YUn, ZUn, 
              'SOURCE DATA',
              'PIXEL',
              'INTENSITY',
              'TIME',
              0, 0, 20, LineStyle, 
              AutoScaleX, AutoScaleY, AutoScaleZ);
 REPLOT_WINDOW();
 MOVE_CURSOR(MinZ, (X_INDEX_U - X_INDEXL / 2));
END;
 
{following sets up plot format for YT data-----------------------------------}
PROCEDURE Y_T_PLOT;
BEGIN
 CS[YT_INDEX].DISPLAY:=2;
 SELECT_WINDOW(2);
 IF YT_POINTS < 20 THEN BEGIN
  XYT_MIN:=0; 
  XYT_MAX:=19;
 END
ELSE BEGIN
 { XYT_MIN:= YT_POINTS - 20; }
 XYT_MAX:= YT_POINTS;
 END;
 G_PLOT_STYLE(MinX, MaxX, 
              MinY, MaxY,
              MinZ, MaxZ, 
              XUn, YUn, ZUn, 
              PlotTitle, XLabel, YLabel, ZLabel, 
              ZSide, ZXPercent, ZYPercent, LineStyle, 
              AutoScaleX, AutoScaleY, AutoScaleZ);

 S_PLOT_STYLE(XYT_MIN, 
              CS[YT_INDEX].[0].XMAX, 
              CS[YT_INDEX].[0].MIN, 
              CS[YT_INDEX].[0].MAX, 
              MinZ, MaxZ, 
              14, YUn, ZUn, 
             'Y-T OF CURVE SET AT X',
             'TIME',
             'INTENSITY',
              ZLabel, 
              0, 20, 20, LineStyle, 
              AutoScaleX, AutoScaleY, AutoScaleZ);
 REPLOT_WINDOW();
 MOVE_CURSOR(MinZ, NTIME);
END;

{following section gets lower and upper pixel numbers defining area to plot--}
PROCEDURE GET_AREA;
BEGIN
 PROMPT_TEXT:='Enter lower pixel number for intensity v. time analysis ';
 WRITE_TEXT();
 READ(X_INDEX_L);
 PROMPT_TEXT:='Enter upper pixel number for intensity v. time analysis ';
 WRITE_TEXT();
 READ(X_INDEX_U);
END;

{following section gets destination curve-set name --------------------------}
PROCEDURE GET_DEST_FILE_NAME;
BEGIN
 WHILE (OUT_FILE_OK=0) DO
 BEGIN
 WRITELN();
 BUFLEN:= 56;
 WRITE('Enter required destination curve-set name for YT data   ');
 READ(OUTPUT_NAME);
 WRITELN();
 YT_INDEX:=G_CURVE_SET_INDEX(OUTPUT_NAME,'',-1);
 IF (YT_INDEX <> -1) THEN
  BEGIN
  BUFLEN:=46;
  WRITE('Destination curve-set already exists! Overwrite');
  PROMPT_TEXT:= '';
  BUFLEN:=1;
  ASK_IF_YES();
  IF (YES_FLAG=1) THEN
  BEGIN
  OUT_FILE_OK:=1;
  DEL_CURVE_SET(CS[YT_INDEX]);
  YT_INDEX:=CS_COUNT();
  CREATE_CURVE_SET(OUTPUT_NAME,'','YT data',YT_INDEX);
  YT_INDEX:=G_CURVE_SET_INDEX(OUTPUT_NAME,'',-1);     
  END;
  END;
 IF (YT_INDEX = -1) THEN 
 BEGIN
 OUT_FILE_OK:=1;
 YT_INDEX:=CS_COUNT();
 CREATE_CURVE_SET(OUTPUT_NAME,'','YT data',YT_INDEX);
 YT_INDEX:=G_CURVE_SET_INDEX(OUTPUT_NAME,'',-1);     
 END;
 END;
END;
 
{main program starts here----------------------------------------------------}
BEGIN
 PROMPT_TEXT:= '                           LIVE Y-T DISPLAY MACRO';
 WRITE_TEXT();
 DELAY();
 GET_AREA();
 GET_DEST_FILE_NAME();
 ENTER_GRAPH();
 S_WINDOW_STYLE(2);
 {now turn off display of all other curves-----------------------------------}
 CURVE_SET_COUNT:= (CS_COUNT() - 1);
 FOR T_COUNT:=0 TO CURVE_SET_COUNT DO
 BEGIN
 CS[T_COUNT].DISPLAY:=0;
 END;
 {create curve to store data YT data-----------------------------------------}
 CREATE_CURVE(100,0.0,0,CS[YT_INDEX].[0]);
 CS[YT_INDEX].[0].XUNITS := 14;
 YT_COUNT:=0;
 PROMPT_TEXT:= 'Acquisition in progress - press <F3> to stop ';
 WRITE_TEXT();
 STIME := NOW();
 OTIME := 0;
 WHILE (INPUT <> 156) DO BEGIN
  D_RUN(0);
  WHILE (DG_ACTIVE() = TRUE) DO
  BEGIN
   IF KEYPRESSED THEN INPUT:= READCH();   
  END;
  SOURCE_INDEX:= G_CURVE_SET_INDEX('lastlive','',0);                 
  YT_POINTS:= YT_COUNT + 1;     
  YT_INDEX:=G_CURVE_SET_INDEX(OUTPUT_NAME,'',-1);      
  IF (YT_POINTS >1) THEN 
   CHANGE_CURVE_SIZE(cs[yt_index].[0],YT_POINTS,true,0.0);
  CS[YT_INDEX].[0].Y[YT_COUNT] := D_AREA(0,X_INDEX_L,X_INDEX_U);   
  NTIME := NOW() - STIME;
  IF NTIME <= OTIME THEN NTIME := OTIME + 0.05; { Max 20 samples / sec }
  CS[YT_INDEX].[0].X[YT_COUNT] := NTIME;
  CS[YT_INDEX].[0].XMAX := NTIME;
  OTIME := NTIME;
  { DEFAULT_PLOT(); }
  { Y_T_PLOT();     }
  WRITELN('Sample: ', YT_COUNT, 'Time: ', NTIME);
  YT_COUNT:= YT_COUNT + 1;
  IF KEYPRESSED THEN INPUT:= READCH();  
 END;
  DEFAULT_PLOT();
  Y_T_PLOT();
 PROMPT_TEXT:= 'Acquistion completed - End of program ';
 WRITE_TEXT();
END.
