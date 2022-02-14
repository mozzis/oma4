PROGRAM tc;   { program to do long term absorbance }

  { This program sets up to do live dual track absorbance.  It then   }
  { acquires data repeatedly, and saves the absorbance at the cursor  }
  { position in a history curve.  The history curve is simultaneously }
  { displayed in a separate window. To use the program, first set the }
  { scan setup for two tracks, either random or uniform. Set up all   }
  { other parameters as desired.  The Scans parameter on the Run Setup}
  { menu is used to control the signal averaging.                     }
  { The program assumes that a periodic signal is applied to the SYNC }
  { IN connector on the detector. (WFTC can be changed to 0, though)  }
  { After all the settings are done, you should go to the plot screen }
  { and acquire some data so you can position the cursor.  Then run   }
  { this program. It will acquire a background, then set the program  }
  { into Live Dual Absorbance with background subtract mode and begin }
  { acquiring data.                                                   }
  { To stop the program, hold down a key on the keyboard. I suggest   }
  { one of the arrow keys, since all of the extra keys in the buffer  }
  { will be discarded when the program finally notices that a key is  }
  { down.                                                             }

var crvindx, oldi   : INTEGER;     { index of lastlive }
    reps, dummy1    : INTEGER;     { number of repetitions }
    histdata        : INTEGER;     { curve set index of history plot }
    oldshut         : INTEGER;
    refsz, cpoint   : WORD;        { No. of points in history curve }
    keybyte         : BYTE;        { byte from user keypress }
    oldcmd          : STRING;      { save old command line }
    ccoord, dummy2  : SINGLE;      { X coordinate of history curve }
    a0, a1, dummy3  : REAL;        { use to get point from X Cal }
    AutoScaleZ : BOOLEAN;          { Plot Style parameters... }
    AutoScaleY : BOOLEAN;          {           .              }
    AutoScaleX : BOOLEAN;          {           .              }
    LineStyle  : INTEGER;          {           .              }
    ZYPercent  : INTEGER;          {           .              }
    ZXPercent  : INTEGER;          {           .              }
    ZSide      : INTEGER;          {           .              }
    ZLabel     : STRING;           {           .              }
    YLabel     : STRING;           {           .              }
    XLabel     : STRING;           {           .              }
    PlotTitle  : STRING;           {           .              }
    ZUn        : WORD;             {           .              }
    YUn        : WORD;             {           .              }
    XUn        : WORD;             {           .              }
    MaxZ       : REAL;             {           .              }
    MinZ       : REAL;             {           .              }
    MaxY       : REAL;             {           .              }
    MinY       : REAL;             {           .              }
    MaxX       : REAL;             {           .              }
    MinX       : REAL;             {           .              }

procedure GetPlot;                 { Get the current window's }
  begin                            { plot parameters          }
  G_PLOT_STYLE(MinX,
               MaxX, 
               MinY, 
               MaxY, 
               MinZ, 
               MaxZ, 
               XUn, 
               YUn, 
               ZUn, 
               PlotTitle, 
               XLabel, 
               YLabel, 
               ZLabel, 
               ZSide, 
               ZXPercent, 
               ZYPercent, 
               LineStyle, 
               AutoScaleX, 
               AutoScaleY, 
               AutoScaleZ);
  end;


procedure SetPlot;                 { Set new plot parameters  }
  begin                            { for the current window   }
  S_PLOT_STYLE(MinX,
               MaxX, 
               MinY, 
               MaxY, 
               MinZ, 
               MaxZ, 
               XUn, 
               YUn, 
               ZUn, 
               PlotTitle, 
               XLabel, 
               YLabel, 
               ZLabel, 
               ZSide, 
               ZXPercent, 
               ZYPercent, 
               LineStyle, 
               AutoScaleX, 
               AutoScaleY, 
               AutoScaleZ);
  end;


begin
  START_FRESH();
  BUFLEN  := 7;
  keybyte := 0;
  reps    := 0;
  refsz   := 10;

  { the program stores a history of the absorbance at the cursor }
  { position in a separate curve set. If this curve set exists,  }
  { it is deleted and created anew.                              }

  histdata := G_CURVE_SET_INDEX('histdata.dat', '', 0);
  if histdata <> -1 then
    DEL_CURVE_SET(CS[histdata]);
  CREATE_CURVE_SET('histdata.dat', '', 'History data set', 0);
  histdata :=  G_CURVE_SET_INDEX('histdata.dat', '', 0);
  CREATE_CURVE(refsz, 0, FALSE, CS[histdata].[0]);

  CS[histdata].display := 2;                 { history displays in win 2 }

  oldcmd := GCMD_PROMPT();                   { save old command line }
  S_WINDOW_STYLE(2);                         { set two horiz plot windows }
  ENTER_GRAPH();                             { enter graph mode }
  S_ACTIVE_WINDOW(1);                        { start in window 1 }
  WRITELN();                                 { clear output line }
  S_ACCUM(FALSE);                            { backgrnd subtract off }
  SET_LIVEFILTER(0, '');                     { no live filter yet }
  oldshut := DG_SHUTMODE();                  { save old shutter state }
  DS_SHUTMODE(1);                            { shutter closed }
  SCMD_PROMPT('Acquiring background: ');
  WRITELN('Shutter closed');

  { signal averaging is used to increase signal to noise. The number }
  { of samples averaged is set by the user on the run setup screen   }
  { before the program runs.                                         }

  oldi := DG_I();                            { Save user scans }
  DS_I(1);                                   { 1 scan for background }
  DS_H(3);                                   { always 3 prep frames }
  DS_WFTO(1);                                { Wait for Sync ON }
  WAIT(500);
  GO_LIVE();                                 { acquire the background }
  repeat until DG_ACTIVE() = 0;              { wait till done }
  WAIT(500);
  SCMD_PROMPT('');
  WRITELN();
  SCALE_Y();                                 { display }
  S_ACTIVE_WINDOW(2);                        { goto to window 2 }
  GetPlot();                                 { get plot settings }
  MaxX := refsz-1;                           { set X axis length }
  XUn  := 0;
  SetPlot();                                 { set new settings }
  S_ACTIVE_WINDOW(1);                        { back to window 1 }
  crvindx := 
    G_CURVE_SET_INDEX('lastlive', '', 0);    { find live data }
  WRITELN('Loading background');
  LOAD_BACKGROUND(cs[crvindx].[0], 2);       { copy to background }
  S_ACCUM(TRUE);                             { backgrnd subtract on }
  SET_LIVEFILTER(3, 'lastlive');             { live dual absorbance }
  SCMD_PROMPT('Acquiring data; press a key to stop');
  DS_SHUTMODE(oldshut);                      { restore shutter state}
  if oldshut = 0 then
    WRITE('Shutter Normal - ')
  else if oldshut = 1 then 
    WRITE('Shutter Closed - ')
  else
    WRITE('Shutter Open - ');
  WRITELN('Acquiring first scan...');
  DS_I(oldi);                                { set scans back to old }
  GO_LIVE();
  WAIT(500);                                 { get one scan to scale window }
  repeat until (DG_ACTIVE() = 0);
  SCALE_Y();                                 { display }

  { The program reports the absorbance at the cursor position. The system }
  { calibration coefficients are used to derive the pixel number of the   }
  { cursor position. Use cpoint to hold the calib units temporarily.      }

  cpoint := G_CALIB(a0, a1, dummy3, dummy3, dummy3);  { get X calib setup }

  { read the current curve cursor position (ignore other information) }
  { and convert to pixels if in wavelength                            }

  READ_MOUSE(dummy1, dummy1, ccoord, dummy2, dummy2);
  if (cpoint) then                {X units (from above call) not in pixels }
    cpoint := ROUND((ccoord - a0) / a1)
  else
    cpoint := ccoord;
  
  crvindx := 
    G_CURVE_SET_INDEX('lastlive', '', 0);     { find live data again }
  histdata := 
    G_CURVE_SET_INDEX('histdata.dat', '', 0); { and find the history data }

  { loop to acquire and display data }
  { quit if keystroke detected. note keys received during acquisition }
  { get thrown away if they are not used (e.g. F3)                    }
  { so to stop program must see key pressed after GO_LIVE is done.    }  

  repeat begin
    GO_LIVE();
    repeat begin
      if KEYPRESSED() then begin
        SCMD_PROMPT('Aborting...');
        keybyte := READCH();
        if keybyte = 0 then keybyte := READCH();
      end;
    end until ((DG_ACTIVE() = 0) or (keybyte <> 0));
    WRITE('Absorbance at ', ccoord);
    BUFLEN := -3;
    WRITE(' (CV', crvindx, ' PT', cpoint, ')');
    BUFLEN := 7;
    WRITE(' is: ', CS[crvindx].[0].Y[cpoint]);
    WRITELN(' run ', reps+1);

  { Save history data }

    CS[histdata].[0].y[reps] := CS[crvindx].[0].Y[cpoint];
    reps := reps + 1;

  { The history curve is "grown" in increments of 10 points at a time }

    if reps >= refsz then begin
      S_ACTIVE_WINDOW(2);              { Go to window 2 }
      SCALE_Y();                       { autoscale to real data }
      refsz := reps + 10;
      CHANGE_CURVE_SIZE(CS[histdata].[0], refsz, TRUE, 0);
      GetPlot();
      MaxX := refsz-1;                 { grow plot window also }
      SetPlot();
      REPLOT_WINDOW();
      S_ACTIVE_WINDOW(1);              { back to window 1 }
    end else begin                     { if not growing curve, just plot }
      S_ACTIVE_WINDOW(2);              { new data in window 2 }
      REPLOT_WINDOW();
      S_ACTIVE_WINDOW(1);
    end;
  end until keybyte <> 0;              { big loop end - when key detected }
  SCMD_PROMPT(oldcmd);
end.

