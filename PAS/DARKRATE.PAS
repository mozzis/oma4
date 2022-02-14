{-----------------------------------------------------------------------}
{                                                                       }
{       File Name : DARKRATE.PAS                                        }
{       Author    : David DiPrato                                       }
{       Date      : 02/91                                               }
{       Version   : 1.00                                                }
{                                                                       }
{       This program will calculate Dark Current rate for a             }
{       two dimensional detector. It assumes that there are             }
{       curves in a Curve Directory file. Each curve should             }
{       be aquiried with the shutter forced closed and have progressing }
{       integration times.  These times should be selected so that data }
{       values range from 5K to 230K counts. Use DA program LINEAR.DAC. }
{                                                                       }
{-----------------------------------------------------------------------}
program darkrate;

var
   Data_index : INTEGER;        {Index number of the data curve set.    }
   Data_name : STRING;          {Name of the data file.                 }
   Curve_index : INTEGER;       {Index number of the data curve set.    }
   Curve_name : STRING;         {Name of any curve set file.            }
   Num_points, Pointv : WORD;   {Used in the point dimension loops.     }
   Num_curves, Curvev : WORD;   {Used in the curve dimension loops.     }
   Num_frames, Framev : WORD;   {Used in the frame dimension loops.     }
   Num_ignore : WORD;           {Used for the number of exposures.      }
   Exp_time : SINGLE;           {This is the exposure time used.        }
   DET1_OFFSET : LONG_INTEGER;  {These are used ot index in the curve   }
   MEMORIES_OFFSET : LONG_INTEGER; {headers.                            }
   IGNORED_OFFSET : LONG_INTEGER;
   TRACKS_OFFSET : LONG_INTEGER;
   EXP_TIME_OFFSET : LONG_INTEGER;
   DataFileHandle : FILE;

{-----------------------------------------------------------------------}
{                                                                       }
{       Procedure Name: Calculate Mean value array.                     }
{                                                                       }
{       This procedure will calculate the mean  value of a data curve   }
{       set.                                                            }
{                                                                       }
{       Input  : Curve_name - Data curve set.                           }
{       Output : MEAN.DAT - Mean curve set.                             }  
{                                                                       }
{-----------------------------------------------------------------------}
procedure Cal_mean;

var
   Mean_index : INTEGER;        {Index number of the Mean curve set.    }
   Dummy1 : REAL;               {This is used as a dumy varible.        }
   Dummy2 : REAL;               {This is used as a dumy varible.        }
   Dummy3 : REAL;               {This is used as a dumy varible.        }

begin
   WRITELN();
   WRITE('Calculating Mean data ...');
   Mean_index := G_CURVE_SET_INDEX('MEAN.DAT','',0);
   IF Mean_index <> -1 THEN
     DEL_CURVE_SET(CS[Mean_index]);
   CREATE_CURVE_SET('MEAN.DAT','','',0);
   Mean_index := G_CURVE_SET_INDEX('MEAN.DAT','',0);
   Curve_index := G_CURVE_SET_INDEX(Curve_name,'',0);
   Num_points := CS[Curve_index].[0].POINT_COUNT;
   CREATE_CURVE(Num_frames,0,FALSE,CS[Mean_index].[0]); 
   CS[Mean_index].[0].XMIN := 1.0e19;
   CS[Mean_index].[0].XMAX := 1.0e-19;
   CS[Mean_index].[0].XUNITS := 2;
   Dummy3 := 0;
   WRITELN();
   WRITE('Calling Average_Std_Dev..');
   AVG_STD_DEV(CS[Curve_index].[Dummy3],
        Num_curves,0,Num_points,Dummy1,Dummy2);
   Dummy2 := Exp_time;
   CS[Mean_index].[0].Y[0] := Dummy1;
   CS[Mean_index].[0].X[0] := Dummy2;
   Dummy3 := Num_curves;
   AVG_STD_DEV(CS[Curve_index].[Dummy3],
        Num_curves,0,Num_points,Dummy1,Dummy2);
   Dummy2 := Num_ignore * Exp_time;
   CS[Mean_index].[0].Y[1] := Dummy1;
   CS[Mean_index].[0].X[1] := Dummy2;
   WRITELN();
   WRITE('Done calculating Mean ...');
end;

{-----------------------------------------------------------------------}
{                                                                       }
{       Procedure Name: Calculate Dark current rate.                    }
{                                                                       }
{       This procedure will calculate the Dark Current rate from a      }
{       curve set.                                                      }
{                                                                       }
{       Input  : Curve_name - Data curve set.                           }
{       Output : Dark_rate - The Dark Current Rate.                     }  
{                                                                       }
{-----------------------------------------------------------------------}
procedure Cal_darkrate; 

var
   Start_point : REAL;          {Hold the minimum value.                }
   End_point : REAL;            {Hold the maximum value.                }
   Pointer : WORD;              {Used to index the Linear data curve.   }
   Dark_rate : REAL;            {The finial Dark rate result.           }

begin
   WRITELN();
   WRITE('Calculating Dark Current Rate ...');
   Curve_index := G_CURVE_SET_INDEX(Curve_name,'',0);
   Num_points := CS[Curve_index].[0].POINT_COUNT;                      
   Start_point := CS[Curve_index].[0].Y[0];
   End_point := CS[Curve_index].[0].Y[Num_points - 1];
   Dark_rate := (End_point - Start_point) / ((Num_ignore - 1) * Exp_time);
end;

{-----------------------------------------------------------------------}
{                                                                       }
{       Procedure Name: Plot Screen.                                    }
{                                                                       }
{       This procedure will plot and auto scale the X, Y, and Z axis    }
{       of the Mean and Linear curve sets.                              }
{                                                                       }
{       Input  : None.                                                  }  
{       Output : (Window One and two).                                  }
{                                                                       }
{-----------------------------------------------------------------------}
procedure Plot_screen;

var
                                {-------  Ploting Parameters  ----------}
   X_MIN, X_MAX : REAL;         {Extremes on the X axis.                } 
   Y_MIN, Y_MAX : REAL;         {Extremes on the Y axis.                }
   Z_MIN, Z_MAX : REAL;         {Extremes on the Z axis.                } 
   X_UNITS : INTEGER;           {Units on the X axis.                   }
   Y_UNITS : INTEGER;           {Units on the Y axis.                   }
   Z_UNITS : INTEGER;           {Units on the Z axis.                   }
   PLOT_TITLE : STRING;         {Plot's title.                          }
   X_LABLE : STRING;            {X axis lable.                          }   
   Y_LABLE : STRING;            {Y axis lable.                          }   
   Z_LABLE : STRING;            {Z axis lable.                          }   
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
   Curve_index := G_CURVE_SET_INDEX('MEAN.DAT','',0);                   
   CS[Curve_index].DISPLAY := 1;                                   
   Data_index := G_CURVE_SET_INDEX(Data_name,'',0);                   
   CS[Data_index].DISPLAY := 0;                                   
{-----------------------------------------------------------------------}
{--- Plot Average data  ------------------------------------------------}
{-----------------------------------------------------------------------}
   S_ACTIVE_WINDOW(1);
   PLOT_TITLE := 'Average Intensity versus Time';                                     
   X_LABLE := 'Time (seconds)';                                                 
   Y_LABLE := 'Intensity (counts)';                                                
   Z_LABLE := 'Tracks';                                                      
   Z_SIDE := 2;                                                        
   ZX_PERCENT := 0;                                                    
   ZY_PERCENT := 0;                                                    
   X_UNITS := 2;                                                       
   LINE_STYLE := 0;                                                    
   X_UNITS := 2;                                                       
   AUTO_SCALE_X := TRUE;                                               
   AUTO_SCALE_Y := TRUE;                                               
   AUTO_SCALE_Z := TRUE;                                               
   S_PLOT_STYLE(X_MIN, X_MAX, Y_MIN, Y_MAX, Z_MIN, Z_MAX,                    
                X_UNITS, Y_UNITS, Z_UNITS,                             
                PLOT_TITLE, X_LABLE, Y_LABLE, Z_LABLE,                 
                Z_SIDE, ZX_PERCENT, ZY_PERCENT,                        
                LINE_STYLE,                                            
               AUTO_SCALE_X, AUTO_SCALE_Y, AUTO_SCALE_Z);
   ENTER_GRAPH();
   PLOT_WINDOW('SCREEN',1);
   AUTO_SCALE();

end;

{-----------------------------------------------------------------------}
{                                                                       }
{       Procedure Name: Main                                            }
{                                                                       }
{-----------------------------------------------------------------------}

begin
   BUFLEN := 13;
   Data_name := 'lastlive';
   WRITE('Working, please wait ...');
{-----------------------------------------------------------------------}
{-- Get data parameters ------------------------------------------------}
{-----------------------------------------------------------------------}
   Num_frames := DG_J();
   Num_curves := DG_Tracks();
   Exp_time   := DG_ET();
   Num_ignore := DG_K();
   if Num_frames = 2 then
   begin
{-----------------------------------------------------------------------}
{-- Do calculations ----------------------------------------------------}
{-----------------------------------------------------------------------}
{     SAVE_METHOD('temp.mtd');                                          }
     Curve_name := Data_name;
     Cal_mean();
     Curve_name := 'MEAN.DAT';
     Plot_screen();
     Cal_darkrate();
     Plot_screen();
     BUFLEN := 8;
     WRITELN();
     WRITE('The Dark Current rate is ',Dark_rate,' (counts/sec).');
{     LOAD_METHOD('temp.mtd');                                          }
   end
   else
{-----------------------------------------------------------------------}
{-- ERROR wrong curve set. ---------------------------------------------}
{-----------------------------------------------------------------------}
   begin
     WRITELN();
     WRITE('ERROR: Data set must 2 frames, Use Method file DARKRATE.MET');
   end;
end.
