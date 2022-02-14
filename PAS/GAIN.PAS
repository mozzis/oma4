{-----------------------------------------------------------------------}
{                                                                       }
{       File Name : GAIN.PAS                                            }
{       Author    : David DiPrato                                       }
{       Date      : 03/91                                               }
{       Version   : 1.00                                                }
{                                                                       }
{       This program will calculate the signal noise and gain           }
{       for a two dimensional detector. The results will be             }
{       displayed on the bottom line of the screen.                     }
{                                                                       }
{       Use Data Acquisition program 'GAIN.DAC'.                        }
{       This program will use the pannel controls and make a number of  }
{       curves in a Curve Directory file. Each pair of curves will be   }
{       used as the data curves and the first curve as the background.  }
{       The data curves must have large values, compared with the       }
{       background for the Gain calculation. For these values the noise }
{       value should be signal shot noise not system floor noise.       }
{                                                                       }
{-----------------------------------------------------------------------}
program gain;

var
   Num_points, Point : WORD;    {Used in the point dimension loops.     }
   Num_curves, Curve : WORD;    {Used in the curve dimension loops.     }
   Num_frames : WORD;           {Used in the frame dimension loops.     }
   Num_pairs, Pair : WORD;      {This is the number of frame pairs.     }
   Data_index : INTEGER;        {Index number of the data curve set.    }
   Difference_index : INTEGER;  {Index of the Difference curve set.     }
   Gain_index : INTEGER;        {Index number of the gain curve set.    }
   Data_name : STRING;          {Name of the data file.                 }
   DET1_OFFSET : LONG_INTEGER;  {These are used ot index in the curve   }
   MEMORIES_OFFSET : LONG_INTEGER; {headers.                            }
   TRACKS_OFFSET : LONG_INTEGER;
   DataFileHandle : FILE;

{-----------------------------------------------------------------------}
{                                                                       }
{       Procedure Name: Plot Screen.                                    }
{                                                                       }
{       This procedure will plot and auto scale the X, Y, and Z axis    }
{       of the Gain curve sets.                                         }
{                                                                       }
{       Input  : Gain curve set.                                        }  
{       Output : (Window One).                                          }
{                                                                       }
{-----------------------------------------------------------------------}
procedure Plot_screen;

var
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
   WRITELN();                                                           
   WRITE('Plotting ...');                                               
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
{--- Plot Gain ---------------------------------------------------------}
{-----------------------------------------------------------------------}
   Gain_index := G_CURVE_SET_INDEX('Gain.dat','',0);          
   CS[Gain_index].DISPLAY := 1;                                        
   S_ACTIVE_WINDOW(1);
   PLOT_TITLE := 'Gain versus Intensity';                                     
   X_LABEL := 'Intensity';                                                 
   Y_LABEL := 'Gain';                                                
   Z_LABEL := '';                                                      
   Z_SIDE := 2;                                                        
   ZX_PERCENT := 0;
   ZY_PERCENT := 0;
   X_UNITS := 2;
   LINE_STYLE := 0;
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
   PLOT_WINDOW('SCREEN',1);
   AUTO_SCALE();
end;

{-----------------------------------------------------------------------}
{                                                                       }
{       Procedure Name: Calculate.                                      }
{                                                                       }
{       This procedure will calculate the gain for the Point            }
{       dimension of the array.                                         }
{                                                                       }
{       Input  : Num_points, Num_curves, Num_frames, Num_pairs.         }
{       Output : Gain curve set.                                        }
{                                                                       }
{ 5 sigmas captures > 95% of shot noise, due to Poisson shape of noise  }
{ distribution curve.                                                   }
{                                              |                        }
{  Npp                                      |1s|1s|                     }
{ -----       = SIGMAshot                   |..|..                      }
{   5(sigmas)                           ----..-|-..---- 2 sig width     }
{                                          ..  |  ..                    }
{ SIGshot = sqrt(#SigCts * Gain)        --..---|-- ..--- 4              }
{                                       -..----|----..- 5               }
{ Gain = Npp^2/(5(sigmas)^2 * AvgSig)   ..     |     ..                 }
{                                                                       }
{-----------------------------------------------------------------------}
procedure calculate;

var
   Offset      : REAL;
   Dif_value   : REAL;    {Holds the difference of both curves.   }
   Dummy       : REAL;    {This is used as a dumy varible.        }
   BG_mean     : REAL;    {This is the mean value of the curve set}
   Data_mean   : REAL;    {This is the mean value of the curve set}
   Total_mean  : REAL;    {This is the mean value of the curve set}
   Total_noise : REAL;    {This is the noise of all data.         }
   Avg_gain    : REAL;    {This is the average gain value.        }

begin
   CREATE_CURVE_SET('Difference.dat','','',0);
   CREATE_CURVE_SET('Gain.dat','','',0);
   Gain_index       := G_CURVE_SET_INDEX('Gain.dat','',0);
   Difference_index := G_CURVE_SET_INDEX('Difference.dat','',0);
   Data_index       := G_CURVE_SET_INDEX(Data_name,'',0);
   CREATE_CURVE(Num_pairs,0,FALSE,CS[Gain_index].[0]);
   CS[Gain_index].[0].XUNITS := 2;
{-----------------------------------------------------------------------}
{-- Calculate gain. ----------------------------------------------------}
{-----------------------------------------------------------------------}
   WRITELN();
   WRITE('Calculating system gain ...');
   AVG_STD_DEV(CS[Data_index].[0],Num_curves,0,Num_points,BG_mean,Dummy);
   Avg_gain := 0;

   for Curve := 0 to (Num_curves-1) DO
     begin
       WRITELN();
       WRITE('Pair',Pair, ', Curve',Curve);
       CREATE_CURVE(Num_points,0,FALSE,CS[Difference_index].[Curve])
     end;

   For Pair := 0 to (Num_pairs-1) Do
     begin
       for Curve := 0 to (Num_curves-1) Do
         begin
           WRITELN();
           WRITE('Pair',Pair, ', Curve',Curve);
           CS[Difference_index].[Curve] :=
             CS[Data_index].[Pair * 2 * Num_curves + Num_curves + Curve] -
             CS[Data_index].[Pair * 2 * Num_curves + 2 * Num_curves + Curve]
         end;
       AVG_STD_DEV(CS[Difference_index].[0],
         Num_curves,0,Num_points,Dummy,Total_noise);
       AVG_STD_DEV(CS[Data_index].[Pair * 2 * Num_curves + Num_curves],
         Num_curves,0,Num_points,Data_mean,Dummy);
       Total_mean := Data_mean - BG_mean;
       CS[Gain_index].[0].Y[Pair] :=
         (Total_mean * 2) / (Total_noise * Total_noise);
       CS[Gain_index].[0].X[Pair] := Total_mean;
       Avg_gain := Avg_gain + CS[Gain_index].[0].Y[Pair]
     end;
   Avg_gain := Avg_gain / Num_pairs
end;

{-----------------------------------------------------------------------}
{                                                                       }
{       Procedure Name: Main                                            }
{                                                                       }
{-----------------------------------------------------------------------}
begin
   BUFLEN := 13;
   WRITELN();
   WRITE('Enter the Data file name : ');
   READLN(Data_name);
   BUFLEN := 4;
   WRITELN();
   WRITE('Working, please wait ...');

   Difference_index := G_CURVE_SET_INDEX('Difference.dat','',0);
   if Difference_index <> -1 then begin
     DEL_CURVE_SET(CS[Difference_index])
   end;

   Gain_index := G_CURVE_SET_INDEX('Gain.DAT','',0);
   If Gain_index <> -1 then begin
     DEL_CURVE_SET(CS[Gain_index])
   end;


{-----------------------------------------------------------------------}
{-- Reading curve header -----------------------------------------------}
{-----------------------------------------------------------------------}
  Num_frames := DG_J();
  Num_curves := DG_TRACKS();
   if Num_frames >= 3 then
   begin
{-----------------------------------------------------------------------}
{-- Do calculations ----------------------------------------------------}
{-----------------------------------------------------------------------}
     Data_index := G_CURVE_SET_INDEX(Data_name,'',0);
     Num_points := CS[Data_index].[0].POINT_COUNT;
     Num_pairs := (Num_frames - 1) / 2;
     calculate();
     Plot_screen();
     WRITELN();
     WRITE('System average Gain is ',Avg_gain,' (e-/cnt)');
   end
   else
{-----------------------------------------------------------------------}
{-- ERROR wrong curve set. ---------------------------------------------}
{-----------------------------------------------------------------------}
   begin
     WRITELN();
     WRITE('ERROR: Incorrect Curve Set, Use DA program GAIN.DAC')
   end;
end.
