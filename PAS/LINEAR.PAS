{-----------------------------------------------------------------------}
{                                                                       }
{       File Name : LINEAR.PAS                                          }
{       Author    : David DiPrato                                       }
{       Date      : 02/91                                               }
{       Version   : 1.01                                                }
{                                                                       }
{       This program will calculate linearity versus intensity for a    }
{       two dimensional detector. It assumes that there are at least    }
{       five curves in a Curve Directory file. Each curve should        }
{       be aquiried with the integerating sphere and have progressing   }
{       integration times.  These times should be selected so that data }
{       values range from 5K to 230K counts. Use DA program LINEAR.DAC. }
{                                                                       }
{-----------------------------------------------------------------------}
program linear;

var
   Data_index : INTEGER;        {Index number of the data curve set.    }
   Data_name : STRING;          {Name of the data file.                 }
   Curve_index : INTEGER;       {Index number of the data curve set.    }
   Curve_name : STRING;         {Name of any curve set file.            }
   Num_points, Pointv : WORD;   {Used in the point dimension loops.     }
   Num_curves, Curvev : WORD;   {Used in the curve dimension loops.     }
   Num_frames, Framev : WORD;   {Used in the frame dimension loops.     }
   DET1_OFFSET : LONG_INTEGER;  {These are used ot index in the curve   }
   MEMORIES_OFFSET : LONG_INTEGER; {headers.                            }
   TRACKS_OFFSET : LONG_INTEGER;
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
   WRITELN();
   WRITE('Live index', Curve_index);
   Num_points := CS[Curve_index].[0].POINT_COUNT;
   CREATE_CURVE(Num_frames,0,FALSE,CS[Mean_index].[0]);                
   CS[Mean_index].[0].XMIN := 1e19;
   CS[Mean_index].[0].XMAX := 1e-19;
   CS[Mean_index].[0].XUNITS := 2;      
   for Framev := 0 to Num_frames - 1 do
   begin
      Dummy3 := Framev * Num_curves;

      AVG_STD_DEV(CS[Curve_index].[Dummy3],
        Num_curves, 0, Num_points, Dummy1, Dummy2);

      CS[Mean_index].[0].Y[Framev] := Dummy1;
      CS[Mean_index].[0].X[Framev] := Framev + 1;
  end;
end;

{-----------------------------------------------------------------------}
{                                                                       }
{       Procedure Name: Calculate Linearity array.                      }
{                                                                       }
{       This procedure will calculate the linearity from a curve set.   }
{                                                                       }
{       Input  : Curve_name - Data curve set.                           }
{       Output : LINEAR.DAT - Linearity curve set.                      }  
{                                                                       }
{-----------------------------------------------------------------------}
procedure Cal_linear; 

var
   Linear_index : INTEGER;      {Index number of the data curve set.    }
   Num_lin_pts : WORD;          {Used in the point dimension loops.     }
   Start_point : REAL;          {Hold the minimum value.                }
   End_point : REAL;            {Hold the maximum value.                }
   Pointer : WORD;              {Used to index the Linear data curve.   }
   Slope : REAL;                {The slope of the mean curve.           }
   Maximum_delta : REAL;        {This is used for finding max deviation.}

   Linearity : REAL;            {The finial linearity result.           }
   Delta : REAL;                {The difference from the ideal line.    }
   Delta_max : REAL;            {The maximum variation from the ideal.  }

begin
   WRITELN();
   WRITE('Calculating Linearity data ...');
   Curve_index := G_CURVE_SET_INDEX('LINEAR.DAT','',0);
   IF Curve_index <> -1 THEN
     DEL_CURVE_SET(CS[Curve_index]);
   CREATE_CURVE_SET('LINEAR.DAT','','',0);
   Curve_index := G_CURVE_SET_INDEX(Curve_name,'',0);
   Linear_index := G_CURVE_SET_INDEX('LINEAR.DAT','',0);
   Num_points := CS[Curve_index].[0].POINT_COUNT;                      
   Num_lin_pts := Num_points - 2;
   CREATE_CURVE(Num_lin_pts,0,FALSE,CS[Linear_index].[0]);             
   CS[Linear_index].[0].XMIN := 1e19;
   CS[Linear_index].[0].XMAX := 1e-19;
   CS[Linear_index].[0].XUNITS := 2;
   Start_point := CS[Curve_index].[0].Y[0];
   for Pointer := 2 to Num_points - 1 do
   begin
      End_point := CS[Curve_index].[0].Y[Pointer];
      Slope := ( End_point - Start_point) / Pointer;
      Maximum_delta := 0;
      for Pointv := 1 to Pointer do
      begin
         if ABS(CS[Curve_index].[0].Y[Pointv] - 
               (Slope * Pointv + Start_point)) > ABS(Maximum_delta) then
            Maximum_delta := CS[Curve_index].[0].Y[Pointv] -
                                (Slope * Pointv + Start_point);
      end;
      Linearity := Maximum_delta / End_point;
      CS[Linear_index].[0].X[Pointer - 2] := End_point;
      CS[Linear_index].[0].Y[Pointer - 2] := Linearity;

   end
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
   X_LABEL : STRING;            {X axis lable.                          }   
   Y_LABEL : STRING;            {Y axis lable.                          }   
   Z_LABEL : STRING;            {Z axis lable.                          }   
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
   S_WINDOW_STYLE(2);
   Linear_index := G_CURVE_SET_INDEX('LINEAR.DAT','',0);          
   CS[Linear_index].DISPLAY := 1;                                        
   Curve_index := G_CURVE_SET_INDEX('MEAN.DAT','',0);                   
   CS[Curve_index].DISPLAY := 2;                                   
   Data_index := G_CURVE_SET_INDEX(Data_name,'',0);                   
   CS[Data_index].DISPLAY := 0;                                   
{-----------------------------------------------------------------------}
{--- Plot Linear data --------------------------------------------------}
{-----------------------------------------------------------------------}
   S_ACTIVE_WINDOW(1);
   PLOT_TITLE := 'Linearity versus Intensity';                                     
   X_LABEL := 'Intensity (counts)';                                                 
   Y_LABEL := 'Linearity';                                                
   Z_LABEL := '';                                                      
   Z_SIDE := 2;                                                        
   ZX_PERCENT := 0;                                                    
   ZY_PERCENT := 0;                                                    
   LINE_STYLE := 0;                                                    
   X_UNITS := 2;                                                       
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
{-----------------------------------------------------------------------}
{--- Plot Average data  ------------------------------------------------}
{-----------------------------------------------------------------------}
   S_ACTIVE_WINDOW(2);
   PLOT_TITLE := 'Average Intensity versus Time';                                     
   X_LABEL := 'Time (ET units)';                                                 
   Y_LABEL := 'Intensity (counts)';                                                
   Z_LABEL := 'Tracks';                                                      
   Z_SIDE := 2;                                                        
   ZX_PERCENT := 0;                                                    
   ZY_PERCENT := 0;                                                    
   X_UNITS := 2;                                                       
   LINE_STYLE := 0;                                                    
   X_UNITS := 2;                                                       
   AUTO_SCALE_X := TRUE;                                               
   AUTO_SCALE_Y := TRUE;                                               
   AUTO_SCALE_Z := TRUE;                                               
   S_PLOT_STYLE(X_MIN, X_MAX, 
                Y_MIN, Y_MAX, 
                Z_MIN, Z_MAX,                    
                X_UNITS, Y_UNITS, Z_UNITS,                             
                PLOT_TITLE, 
                X_LABEL, Y_LABEL, Z_LABEL,                 
                Z_SIDE, 
                ZX_PERCENT, ZY_PERCENT,                        
                LINE_STYLE,                                            
               AUTO_SCALE_X, AUTO_SCALE_Y, AUTO_SCALE_Z);              
   ENTER_GRAPH();
   PLOT_WINDOW('SCREEN',2);                                            
   AUTO_SCALE();

end;

{-----------------------------------------------------------------------}
{                                                                       }
{       Procedure Name: Main                                            }
{                                                                       }
{-----------------------------------------------------------------------}

begin
   BUFLEN := 13;
   WRITELN();
   WRITE('Enter the Data file name. ');
   READLN(Data_name);
   BUFLEN := 4;
   WRITELN();
   WRITE('Working, please wait ...');
{-----------------------------------------------------------------------}
{-- Read curve parameters-----------------------------------------------}
{-----------------------------------------------------------------------}
   Num_frames := DG_J();
   Num_curves := DG_Tracks();

   WRITELN();
   WRITE(Num_frames, Num_curves);

   if Num_frames >= 3 then
   begin
{-----------------------------------------------------------------------}
{-- Do calculations ----------------------------------------------------}
{-----------------------------------------------------------------------}
     Curve_name := Data_name;
     Cal_mean();
     Curve_name := 'MEAN.DAT';
     Cal_linear();
     Plot_screen();
     BUFLEN := 7;
     WRITELN();
     WRITE('The End Point linearity is ',Linearity);
   end
   else
{-----------------------------------------------------------------------}
{-- ERROR wrong curve set. ---------------------------------------------}
{-----------------------------------------------------------------------}
   begin
     WRITELN();
     WRITE('ERROR: Data set must have at least 3 frames, Use DA LINEAR.DAC');
   end;
end.
