PROGRAM NYT;
{---------------------------------------------------------------------------
This macro, an updated and more powerful version of YT.PAS, performs an
intensity against time analysis of the given input curve set at a specified
pixel position and generates a resulting curve which has the same number
of points as the number of curves in the source curve set. The YT curve(s)
are stored in a user specified curve set; provision is made for overwriting
existing curve-sets.

When the program runs, the user is prompted for the source curve set.
If this is not already loaded in the curve directory, the user can
elect to load from disk, although he must know and specify the number
of curves in the disk curve-set. (The macro language has no way of obtaining
this information.)
The user is then prompted for the destination curve-set to hold the YT data.
If it already exists he is asked whether to overwrite.

The source data curve set is then plotted in the lower of the two plot
windows.

The user then specifies the x-pixel position for analysis (he can use the
mouse cursor to aid in identifying these), after which a y-t analysis
is performed at that pixel. The user is offered the option of saving this
YT curve and further YT analyses, each of which may or may not be saved.
Each curve is inserted before earlier curves in the defined output curve-set.
In addition the pixel at which the data was taken is appended to each curve
as the 'n+1'th intensity reading if the yt curve has n points, to allow
easy identification of the pixel at which the curve was taken.

Date:        21st August 1992 / 1st September 1992
Author:      Richard Burdett, EG&G Instruments, Wokingham, Berks.
Revision:    1.0

Date:        2nd March 1994
Revision:    2.0
             Program now saves method information before it runs and
             reloads the method from file before finishing. Hence you
             are left in the same position as you were before running
             the program.
-----------------------------------------------------------------------------}
VAR MEMORY_NUM, T_COUNT, CURVE, MAX_CURVE, SOURCE_INDEX,
    EXIT, YT_INDEX, OUT_FILE_OK, YES_FLAG, CURVE_SET_COUNT       : INTEGER;
    X_INDEX, CURVE_SET_MAX                                       : REAL;
    INPUT_NAME, OUTPUT_NAME, PROMPT_TEXT, INPUT                  : STRING;
    X_MAX, NO_MEMS                                               : WORD;
    GO_ON, UPPER_N, LOWER_N, UPPER_Y, LOWER_Y, FIRST_CHAR        : BYTE;


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
 MAX_CURVE:=CS[SOURCE_INDEX].COUNT;
 FOR CURVE:= 0 TO (MAX_CURVE-1) DO
  BEGIN
  IF CS[SOURCE_INDEX].[CURVE].MAX > CURVE_SET_MAX THEN CURVE_SET_MAX:=
     CS[SOURCE_INDEX].[CURVE].MAX;
  END;
 SELECT_WINDOW(1);
 S_PLOT_STYLE(0.0,cs[SOURCE_INDEX].[0].point_count,0.0,curve_set_max,0.0,
              CS[SOURCE_INDEX].COUNT,
              0,0,20,
              'SOURCE DATA FROM C/S',
              'PIXEL',
              'INTENSITY',
              'TIME',
              1,0,20,1,1,1,1);
 CS[SOURCE_INDEX].DISPLAY:=1;
 REPLOT_WINDOW();
END;

{following sets up default plot format when macro finishes-------------------}
PROCEDURE SOURCE_DEFAULT_PLOT;
BEGIN
 MAX_CURVE:=CS[SOURCE_INDEX].COUNT;
 FOR CURVE:= 0 TO (MAX_CURVE-1) DO
  BEGIN
  IF CS[SOURCE_INDEX].[CURVE].MAX > CURVE_SET_MAX THEN CURVE_SET_MAX:=
     CS[SOURCE_INDEX].[CURVE].MAX;
  END;
 SELECT_WINDOW(1);
 S_PLOT_STYLE(0.0,cs[SOURCE_INDEX].[0].point_count,0.0,curve_set_max,0.0,
              CS[SOURCE_INDEX].COUNT,
              0,0,20,
              'OMA 4000 DATA PLOT',
              'PIXEL',
              'INTENSITY',
              'TIME',
              1,20,20,1,1,1,1);
 CS[SOURCE_INDEX].DISPLAY:=1;
 REPLOT_WINDOW();
END;

{following sets up plot format for YT data-----------------------------------}
PROCEDURE Y_T_PLOT;
BEGIN
SELECT_WINDOW(2);
S_PLOT_STYLE(0.0,X_MAX,0.0,cs[YT_INDEX].[0].max,0.0,1.0,
            0,20,0,
            'Y-T OF CURVE SET AT X',
            'TIME',
            'INTENSITY',
            '',
            0,20,20,1,1,1,1);
CS[YT_INDEX].DISPLAY:=2;
REPLOT_WINDOW();
END;

{following prompts user for extra info about the curve-set to load-----------}
PROCEDURE FILE_LOAD;
BEGIN
 BUFLEN:=48;
 WRITELN();
 WRITE('Enter number of memories [spectra] in the file   ');
 READ(NO_MEMS);
 WRITELN();
 LOAD_FILE_CURVES(INPUT_NAME,0,NO_MEMS,CS[0],0,0);
END;

{following section prompts user for source curve-set name--------------------}
PROCEDURE FETCH_SOURCE_CURVE_SET;
BEGIN
 WRITELN();
 BUFLEN:=49;
 WRITE('Enter source curve-set name for Y-T analysis     ');
 READ(INPUT_NAME);
 SOURCE_INDEX:= G_CURVE_SET_INDEX(INPUT_NAME,'',0);
 IF SOURCE_INDEX=-1 THEN
  BEGIN
  WRITELN();
  BUFLEN:=42;
  WRITE('Source curve-set not found, load from disk');
  PROMPT_TEXT:='';
  BUFLEN:=1;
  ASK_IF_YES();
  IF YES_FLAG=1 THEN
   BEGIN
    FILE_LOAD();
   END;
  IF YES_FLAG=0 THEN
    EXIT:=2;
   END;
  SOURCE_INDEX:=G_CURVE_SET_INDEX(INPUT_NAME,'',0);
END;

{following section gets pixel number to plot---------------------------------}
PROCEDURE GET_PIXEL;
BEGIN
 WRITELN();
 BUFLEN:=50;
 WRITE('Enter pixel number for intensity v. time analysis  ');
 READ(X_INDEX);
 WRITELN();
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
  END;
  END;
 IF (YT_INDEX = -1) THEN OUT_FILE_OK:=1;
 END;
END;


{main program starts here----------------------------------------------------}
BEGIN
SAVE_METHOD('method.$&$');
FETCH_SOURCE_CURVE_SET();

{following only executes if a source_curve_set has been defined--------------}
IF EXIT=0 THEN
BEGIN
 GET_DEST_FILE_NAME();
 IF (YT_INDEX = -1) THEN
  BEGIN
  YT_INDEX:=(G_CURVE_SET_INDEX(INPUT_NAME,'',-1)+1);
  END;
 ENTER_GRAPH();
 S_WINDOW_STYLE(2);
 {now turn off display of all other curves-----------------------------------}
 CURVE_SET_COUNT:= (CS_COUNT() - 1);
 FOR T_COUNT:=0 TO CURVE_SET_COUNT DO
 BEGIN
 CS[T_COUNT].DISPLAY:=0;
 END;
 DEFAULT_PLOT();
 {now create destination curve set Y_T.DAT-----------------------------------}
 CREATE_CURVE_SET(OUTPUT_NAME,'','Intensity against time',YT_INDEX);
 {now find out number of curves in source curve-set--------------------------}
 X_MAX:=CS[SOURCE_INDEX].COUNT;
END;

{following repeats YT analysis until exit=1----------------------------------}
WHILE (EXIT=0) DO
BEGIN
 GET_PIXEL();
 WRITELN();
 {create curve to store data YT data-----------------------------------------}
 CREATE_CURVE(X_MAX,0.0,0,CS[YT_INDEX].[0]);
 {perform y-t analysis-------------------------------------------------------}
 FOR MEMORY_NUM:= 0 TO (X_MAX-1) DO
  BEGIN
  CS[YT_INDEX].[0].Y[MEMORY_NUM]:=CS[SOURCE_INDEX].[MEMORY_NUM].Y[X_INDEX];
  END;
 Y_T_PLOT();
 PROMPT_TEXT:='Save ';
 BUFLEN:= 5;
 ASK_IF_YES();
 IF (YES_FLAG=0) THEN DEL_CURVE(CS[YT_INDEX].[0]);
 {if YT is to be saved, put X_INDEX onto end of YT curve to tie it to data---}
 IF (YES_FLAG=1) THEN
  BEGIN
  CHANGE_CURVE_SIZE(CS[YT_INDEX].[0],(X_MAX + 1),TRUE, X_INDEX);
  END;
 PROMPT_TEXT:='Perform another analysis ';
 BUFLEN:=25;
 ASK_IF_YES();
 EXIT:=(YES_FLAG -1);
 END;

{following section executes only at end of program---------------------------}
IF EXIT=-1 THEN
 BEGIN
 S_WINDOW_STYLE(1);
 SOURCE_DEFAULT_PLOT();
 END;
 WRITELN();
 LEAVE_GRAPH();
 BUFLEN:=46;
 WRITE ('End of Program');
LOAD_METHOD('method.$&$');      
AUTO_SCALE();
END.
{end of program--------------------------------------------------------------}
