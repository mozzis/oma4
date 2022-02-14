VAR
Gain_index, Data_index, i, j : INTEGER;
Num_points : INTEGER;
Data_mean  : REAL;
Data_dev   : REAL;
Data_name  : STRING;

BEGIN

  BUFLEN := 8;

  WRITELN();
  WRITE('Enter a curve set name: ');
  READLN(Data_name);
  Data_index := G_CURVE_SET_INDEX(Data_name,'',0);
  if Data_index = -1 then
    begin
      WRITELN();
      WRITE('Must load a curve set or run live first')
    end
  else
    begin
      Gain_index := G_CURVE_SET_INDEX('Gain.dat','',0);
      if Gain_index <> -1 then DEL_CURVE_SET(CS[Gain_index]);
      CREATE_CURVE_SET('Gain.dat','','',0);
      Gain_index := G_CURVE_SET_INDEX('Gain.dat','',0);
      Data_index := G_CURVE_SET_INDEX(Data_name,'',0);
      j := CS[Data_index].Count;
      Num_points := CS[Data_index].[0].Point_Count;

      for i := 0 to j-1 do
        begin
          WRITELN();
          WRITE(i);
          CREATE_CURVE(512,0,FALSE,CS[Gain_index].[i])
        end;
      for i := 0 to j-1 do
        begin
          WRITELN();
          WRITE(i);
          CS[Gain_index].[i] := (CS[Data_index].[i] + 0)
        end
  end;

END;
