PROGRAM LYT;
{---------------------------------------------------------------------------
This macro performs an intensity against time analysis of lastlive as data
is acquired. In order to execute as fast as possible, the plot of lastlive 
and the YT curve to screen does not occur until the user presses the F3 key.
With the proper Scan Setup and Run Setup, this program will collect 20
samples per second on a 33-MHz 486. This is the limit of the PC BIOS clock.

The user is prompted for the lower and upper pixel positions defining an area 
which is required to be monitored over time. He is then prompted for
the name of a curve set into which this YT curve should be stored; if this
curve set already exists he is prompted as to whether it should be 
overwritten.

Data acquisition then commences as dictated by the current run setup 
paramaters. Each point is acquired after an Accum run and stored to the
YT curve set, until the full set of points has been measured.

Both the source data (lastlive) and the YT data are plotted in live mode.

Date:        27 July 1994
Author:      Richard Burdett, EG&G Instruments, Wokingham, Berks.
             Morris Maynard, EG&G Instruments, Ewing, NJ.
Revision:    1.0

----------------------------------------------------------------------------}

  EXPLICIT    { all variables MUST be defined before use }

VAR OldL, NewL, SrcIndex, YTIndex, YTPoints, YTCount : INTEGER;
    X_Lo, X_Hi, I, J                                 : WORD;
    YTArea                                           : SINGLE;
    NewTime, OldTime, StartTime, XAxisMin            : REAL;
    YTSetName, InString                              : STRING;
    OutFileOK, YesFlag, NeedScale                    : BOOLEAN;
    CharNo, CharYes, InChar                          : BYTE;

    { Plot Parameters }

    AutoScaleZ, AutoScaleY, AutoScaleX     : BOOLEAN;
    LineStyle, ZYPercent, ZXPercent, ZSide : INTEGER;
    ZLabel, YLabel, XLabel, PlotTitle      : STRING;
    ZUn, YUn, XUn                          : WORD;
    MaxZ, MinZ, MaxY, MinY, MaxX, MinX     : REAL;

{prompt user for yes/no response to prompt_text------------------------------}
PROCEDURE AskIfYes;
  BEGIN
    CharNo := 'n';  { these must be lower case! }
    CharYes := 'y';
    BUFLEN := 10;
    WRITE('?  <Y/N> ');
    REPEAT
      InChar := READCH() OR 32; { Bit 5 changes Uppercase to lower case }
      IF (InChar = CharYes) THEN YesFlag := 1
      ELSE IF (InChar = CharNo) THEN YesFlag := 0;
    UNTIL (InChar = CharYes) OR (InChar = CharNo);
    WRITELN(InChar);
    BUFLEN := 79;
  END;

{following plots the source data---------------------------------------------}
PROCEDURE DefaultPlot;
  BEGIN
    CS[SrcIndex].DISPLAY := 1; { display in window 1 }
    SELECT_WINDOW(1);

{get current plot setup, maintain all user settings possible-----------------}
    G_PLOT_STYLE(MinX, MaxX, MinY, MaxY, MinZ, MaxZ,
                 XUn, YUn, ZUn,
                 PlotTitle, XLabel, YLabel, ZLabel,
                 ZSide, ZXPercent, ZYPercent, LineStyle,
                 AutoScaleX, AutoScaleY, AutoScaleZ);

    S_PLOT_STYLE(MinX, MaxX, MinY, MaxY, MinZ, MaxZ,
                 0, YUn, ZUn,
                 'SOURCE DATA',
                 'PIXEL',
                 'INTENSITY',
                 'TIME',
                 0, 0, 20, LineStyle,
                 AutoScaleX, AutoScaleY, AutoScaleZ);

    SCALE_Y();
  END;

{following plots the YT data-------------------------------------------------}
PROCEDURE YTPlot;
  BEGIN
{This line keeps window 20 seconds wide and slides to right------------------}
    IF (NewTime < 20) THEN XAxisMin := 0 ELSE XAxisMin := (NewTime - 20);
    CS[YTIndex].DISPLAY := 2;  { display YT plot in window 2 }
    SELECT_WINDOW(2);
    G_PLOT_STYLE(MinX, MaxX,
                 MinY, MaxY,
                 MinZ, MaxZ,
                 XUn, YUn, ZUn,
                 PlotTitle, XLabel, YLabel, ZLabel,
                 ZSide, ZXPercent, ZYPercent, LineStyle,
                 AutoScaleX, AutoScaleY, AutoScaleZ);

{'20' for XUn sets the X axis to display in units of seconds-----------------}
    S_PLOT_STYLE(MinX, MaxX, MinY, MaxY, MinZ, MaxZ,
                 20, YUn, ZUn,
                 'Y-T OF CURVE SET AT X',
                 'TIME',
                 'INTENSITY',
                 ZLabel,
                 0, 20, 20, LineStyle,
                 AutoScaleX, AutoScaleY, AutoScaleZ);
    AUTO_SCALE();
    MOVE_CURSOR(MinZ, NewTime);
  END;

{following section gets lower and upper pixel numbers defining area to plot--}
PROCEDURE GetUserArea;
  BEGIN
    WRITELN();
    WRITE('Enter lower pixel number for intensity vs. time analysis: ');
    READLN(X_Lo);
    WRITE('Enter upper pixel number for intensity vs. time analysis: ');
    READLN(X_Hi);
  END;

{following section gets destination curve-set name --------------------------}
PROCEDURE GetDestFileName;
  BEGIN
    OutFileOK := FALSE;
    WHILE (OutFileOK = FALSE) DO BEGIN  
      WRITELN();
      BUFLEN := 56;
      WRITE('Enter required destination curve-set name for YT data   ');
      READLN(YTSetName);
      WRITELN();
      YTIndex := G_CURVE_SET_INDEX(YTSetName, '', -1);
      IF (YTIndex <> -1) THEN BEGIN
        BUFLEN := 46;
        WRITE('Destination curve-set already exists! Overwrite');
        AskIfYes();
        IF (YesFlag = 1) THEN BEGIN
          OutFileOK := TRUE;
          DEL_CURVE_SET(CS[YTIndex]);
          YTIndex := CS_COUNT();
          CREATE_CURVE_SET(YTSetName, '', 'YT data', YTIndex);
          YTIndex := G_CURVE_SET_INDEX(YTSetName, '', -1);
        END;
      END;
      IF (YTIndex = -1) THEN BEGIN
        OutFileOK := TRUE;
        YTIndex := CS_COUNT();
        CREATE_CURVE_SET(YTSetName, '', 'YT data', YTIndex);
        YTIndex := G_CURVE_SET_INDEX(YTSetName, '', -1);
      END;
    END;
  END;

{main program starts here----------------------------------------------------}
BEGIN
  START_FRESH();
  WRITELN('                           LIVE Y-T DISPLAY MACRO');
  WAIT(1000);
  YTPoints := 500;
  GetUserArea();
  GetDestFileName();
  ENTER_GRAPH();
  S_WINDOW_STYLE(2); { 2 Windows, split horizontally }

{now turn off display of all other curves-----------------------------------}
{Save Curve Set Count; assignment creates temp set--------------------------}
  IF CS_COUNT THEN BEGIN
    FOR I := 0 TO CS_COUNT - 1 DO
      CS[I].DISPLAY := 0;
  END;
{create curve to store data YT data-----------------------------------------}
  CREATE_CURVE(YTPoints, 0.0, 0, CS[YTIndex].[0]);

{'20' here set the units of the X data to seconds----------------------------}
  CS[YTIndex].[0].XUNITS := 20;

  YTCount := 0;
  WRITELN('Acquisition in progress - press <F3> to stop ');
  BUFLEN := 6;
{Begin acquisition here------------------------------------------------------}
  D_RUN(1);
  StartTime := TRUNC(NOW());
  SrcIndex := G_CURVE_SET_INDEX('lastlive', '', 0);
  YTIndex := G_CURVE_SET_INDEX(YTSetName, '', -1);
  OldL := 0;
  DS_L(0);
  WHILE (InChar <> 156) DO BEGIN

{Wait for new scan done------------------------------------------------------}
    NewL := DG_L();
    WHILE (DG_L() = OldL) DO BEGIN
      IF KEYPRESSED() THEN InChar := READCH();
    END;
    OldL := DG_L();

{Bump curve size every 500 samples-------------------------------------------}
    IF (YTCount >= YTPoints) THEN BEGIN
      YTPoints := YTCount + 500;
      CHANGE_CURVE_SIZE(cs[YTIndex].[0], YTPoints, true, 0.0);
    END;

{Store Area of source as YT Y value, Time of sample as YT X value------------}
    NewTime := NOW() - StartTime;
    IF NewTime > OldTime THEN BEGIN
      OldTime := NewTime;
      YTArea := D_AREA(0, X_Lo, X_Hi);
      CS[YTIndex].[0].Y[YTCount] := YTArea;
      CS[YTIndex].[0].X[YTCount] := NewTime;
      CS[YTIndex].[0].XMAX := NewTime;
      WRITELN('Time: ', NewTime, '   Sample: ', YTCount);
      YTCount := YTCount + 1;
      IF KEYPRESSED() THEN InChar := READCH();
    END;
  END;

{End acquisition here--------------------------------------------------------}
  D_STOP(0);
{Fix curve size to discard points not acquired at stop-----------------------}
  CHANGE_CURVE_SIZE(cs[YTIndex].[0], YTCount, true, 0.0);
  DefaultPlot();
  YTPlot();
  WRITELN('Acquistion completed - End of program ');
END.
