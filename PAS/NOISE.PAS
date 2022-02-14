{-----------------------------------------------------------------------}
{                                                                       }
{       File Name : NOISE.PAS                                           }
{       Author    : David DiPrato                                       }
{       Date      : 03/91                                               }
{       Version   : 1.00                                                }
{                                                                       }
{       This program will calculate the total system floor noise for a  }
{       two dimensional detector. The results will be  displayed on the }
{       bottom line of the screen.                                      }
{                                                                       }
{       Use Data Acquisition program 'NOISE.DAC'.                       }
{       This program will use the front panel controls and make three   }
{       frames in a Curve Directory file. The last two frames will be   }
{       used as the data frames and the first frame as the background.  }
{       The data frame must be close to the background for System       }
{       Floor noise, ie shutter forced closed.                          }
{                                                                       }
{-----------------------------------------------------------------------}
program noise;

var
  Num_points, Point : WORD;       {Used in the point dimension loops.   }
  Num_curves, Curve : WORD;       {Used in the curve dimension loops.   }
  Num_frames : WORD;              {Used in the frame dimension loops.   }
 { Gain : REAL;                    {System gain in electrons per count.  }
  Data_index : INTEGER;           {Index number of the data curve set.  }
  Difference_index : INTEGER;     {Index of the Difference curve set.   }
  Data_name : STRING;             {Name of the data file.               }
  DET1_OFFSET : LONG_INTEGER;     {These are used ot index in the curve }
  MEMORIES_OFFSET : LONG_INTEGER; {headers.                             }
  TRACKS_OFFSET : LONG_INTEGER;

{-----------------------------------------------------------------------}
{                                                                       }
{       Procedure Name: Calculate noise.                                }
{                                                                       }
{       This procedure will calculate the noise for the Point           }
{       dimension of the array.                                         }
{                                                                       }
{       Input  : Num_points, Num_curves.                                }
{       Output : Total_noise, Total_mean                                }
{                                                                       }
{-----------------------------------------------------------------------}
procedure calculate;

var
  Offset, Dif_value : REAL;    {Holds the difference of both curves.   }
  Dummy : REAL;                {This is used as a dummy varible.       }
  BG_mean : REAL;              {This is the mean value of the curve set}
  Data_mean : REAL;            {This is the mean value of the curve set}
  Total_mean : REAL;           {This is the mean value of the curve set}
  Total_noise : REAL;          {This is the noise of all data.         }

begin
  CREATE_CURVE_SET('Difference.dat','','',Data_index);
  Difference_index := G_CURVE_SET_INDEX('Difference.dat','',0);
  Data_index := G_CURVE_SET_INDEX(Data_name,'',0);
{-----------------------------------------------------------------------}
{-- Build two dimensional data set and find mean, min and max values. --}
{-----------------------------------------------------------------------}
  WRITELN();
  WRITE('Building Difference curve set...');
  Total_mean := 0;
  for Curve := 0 to Num_curves - 1 do
  begin
    WRITELN();
    WRITE('Curve', Curve, ' <',Num_curves);
    CREATE_CURVE(Num_points,0,FALSE,CS[Difference_index].[Curve]);
    CS[Difference_index].[Curve] := CS[Data_index].[Curve + Num_curves] -
       CS[Data_index].[Curve + (2 * Num_curves)];
  end;
{-----------------------------------------------------------------------}
{-- Calculate noise. ---------------------------------------------------}
{-----------------------------------------------------------------------}
  WRITELN();
  WRITE('Calculating standard deviation ...');
  AVG_STD_DEV(CS[Difference_index].[0],Num_curves,0,Num_points,
             Dummy,Total_noise);
  Total_noise := Total_noise / 1.414;
end;

{-----------------------------------------------------------------------}
{                                                                       }
{       Procedure Name: Ask If Yes.                                     }
{                                                                       }
{       Waits for the user to type a yes or no response.  Actually is   }
{       only looking for a "Y"es, any other answer is assumed to be no. }
{       The result is returned in the boolean variable "yes_flag".      }
{                                                                       }
{       Input  : None.                                                  }
{       Output : Yes_flag.                                              }
{                                                                       }
{-----------------------------------------------------------------------}
procedure AskIfYes;

var
  UpperY, LowerY : BYTE;       {These are temp variables.               }
  FirstChar : BYTE;            {These are temp variables.               }
  answer : STRING;             {These are temp variables.               }
  Yes_flag : BOOLEAN;          {This is the return value.               }

begin
  UpperY := 'Y';
  LowerY := 'y';
  READLN(answer);
  FirstChar := answer;
  yes_flag := ((FirstChar = UpperY) or (FirstChar = LowerY));
end;

{-----------------------------------------------------------------------}
{                                                                       }
{       Procedure Name: Plot Screen.                                    }
{                                                                       }
{       This procedure will plot and auto scale the X, Y, and Z axis    }
{       of the Difference curve set.                                    }
{                                                                       }
{       Input  : None.                                                  }
{       Output : (Window One and two).                                  }
{                                                                       }
{-----------------------------------------------------------------------}
procedure Plot_screen;

var
                               {-------  Plotting Parameters  ---------}
  X_MIN, X_MAX : REAL;         {Extremes on the X axis.                }
  Y_MIN, Y_MAX : REAL;         {Extremes on the Y axis.                }
  Z_MIN, Z_MAX : REAL;         {Extremes on the Z axis.                }
  X_UNITS : INTEGER;           {Units on the X axis.                   }
  Y_UNITS : INTEGER;           {Units on the Y axis.                   }
  Z_UNITS : INTEGER;           {Units on the Z axis.                   }
  PLOT_TITLE : STRING;         {Plot's title.                          }
  X_LABEL : STRING;            {X axis label.                          }
  Y_LABEL : STRING;            {Y axis label.                          }
  Z_LABEL : STRING;            {Z axis label.                          }
  Z_SIDE : INTEGER;            {Three different points of view.        }
  ZX_PERCENT : INTEGER;        {X angle of Z axis.                     }
  ZY_PERCENT : INTEGER;        {Y angle of Z axis.                     }
  LINE_STYLE : INTEGER;        {Type of plot.                          }
  AUTO_SCALE_X : BOOLEAN;      {Auto-scale switch for the X axis.      }
  AUTO_SCALE_Y : BOOLEAN;      {Auto-scale switch for the Y axis.      }
  AUTO_SCALE_Z : BOOLEAN;      {Auto-scale switch for the Z axis.      }

begin
  X_MIN := 0;
  X_MAX := 0;
  Y_MIN := 0;
  Y_MAX := 0;
  Z_MIN := 0;
  Z_MAX := 0;
  Y_UNITS := 0;
  Z_UNITS := 0;
  S_WINDOW_STYLE(1);
  Data_index := G_CURVE_SET_INDEX(Data_name,'',0);
  CS[Data_index].DISPLAY := 0;
{-----------------------------------------------------------------------}
{--- Plot Noise data  --------------------------------------------------}
{-----------------------------------------------------------------------}
  Difference_index := G_CURVE_SET_INDEX('Difference.dat','',0);
  CS[Difference_index].DISPLAY := 1;
  S_ACTIVE_WINDOW(1);
  PLOT_TITLE := 'Noise Data';
  X_LABEL := 'Pixels';
  Y_LABEL := 'Intensity';
  Z_LABEL := 'Tracks';
  Z_SIDE := 1;
  ZX_PERCENT := 20;
  ZY_PERCENT := 20;
  X_UNITS := 0;
  LINE_STYLE := 1;
  AUTO_SCALE_X := TRUE;
  AUTO_SCALE_Y := TRUE;
  AUTO_SCALE_Z := TRUE;
  S_PLOT_STYLE(X_MIN, X_MAX, Y_MIN, Y_MAX, Z_MIN, Z_MAX,
              X_UNITS, Y_UNITS, Z_UNITS,
              PLOT_TITLE, X_LABEL, Y_LABEL, Z_LABEL,
              Z_SIDE, ZX_PERCENT, ZY_PERCENT,
              LINE_STYLE,
             AUTO_SCALE_X, AUTO_SCALE_Y, AUTO_SCALE_Z);
  ENTER_GRAPH();
  AUTO_SCALE();
end;

{-----------------------------------------------------------------------}
{                                                                       }
{       Procedure Name: Main                                            }
{                                                                       }
{-----------------------------------------------------------------------}
begin
  BUFLEN := 13;
  Difference_index := G_CURVE_SET_INDEX('DIFFERENCE.DAT','',0);
  if Difference_index <> -1 then DEL_CURVE_SET(CS[DIFFERENCE_INDEX]);
  Data_name := 'lastlive';
  WRITELN();
  WRITE('Do you want to plot the data when done? (y/n)? : ');
  AskIfYes();
  BUFLEN := 4;
  WRITELN();
  WRITE('Working, please wait ...');
{-----------------------------------------------------------------------}
{-- Reading curve header -----------------------------------------------}
{-----------------------------------------------------------------------}
  Num_frames := DG_J();
  Num_curves := DG_TRACKS();
  if Num_frames = 3 then
  begin
{-----------------------------------------------------------------------}
{-- Do calculations ----------------------------------------------------}
{-----------------------------------------------------------------------}
    Data_index := G_CURVE_SET_INDEX(Data_name,'',0);
    Num_points := CS[Data_index].[0].POINT_COUNT;
    calculate();
    WRITELN();
    WRITE('The System Floor Noise is ',Total_noise,' (counts).');
    if yes_flag then Plot_screen();
  end
  else
{-----------------------------------------------------------------------}
{-- ERROR wrong curve set. ---------------------------------------------}
{-----------------------------------------------------------------------}
  begin
    WRITELN();
    WRITE('ERROR: Incorrect Curve Set(',Num_frames,'), Use DA program NG.DAC');
  end;
end.
