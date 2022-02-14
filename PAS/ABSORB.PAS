{ --------------------------------------------------------------------
/
/	program absorb;
/
/	
/	
/ -------------------------------------------------------------------- }

program absorb;

var i, j, k : integer;
    CurveTypeStr, CurveName, Descript : string;
    index, AbsorbIndex, BackgroundIndex, IntensityIndex, SourceIndex,
	 curves, points, INVALID : word;


{ --------------------------------------------------------------------
/
/	procedure 
/
/ -------------------------------------------------------------------- }


{ procedure AskCurvename;
begin
	repeat
	begin
		writeln();
		buflen := 5;
		write('Enter name of curve for ');
		write(CurveTypeStr);
		buflen := 2;
		write(': ');
		buflen := 30;
		readln(CurveName)
		buflen := 5;
	end
	until (CurveName <> ' ');
end; }

procedure AskCurvename;
begin
		writeln();
		buflen := 5;
		write('Enter name of curve for ');
		write(CurveTypeStr);
		buflen := 2;
		write(': ');
		buflen := 30;
		readln(CurveName);
		buflen := 5;
end;


{ --------------------------------------------------------------------
/
/	procedure 
/
/ -------------------------------------------------------------------- }

procedure AskExistCurvename;
var ask_count:integer;
begin
	ask_count := 0;
	repeat
	begin
		AskCurvename();		{ ask for curve name, see if it exists }
		index := G_CURVE_SET_INDEX( CurveName, '', -1 ); 
		ask_count := ask_count + 1;
	end
	until ((index <> INVALID) OR (ask_count > 3));  { don't ask more than 3 times }
end;


{ --------------------------------------------------------------------
/
/	procedure 
/
/ -------------------------------------------------------------------- }

procedure AskNewCurvename;
begin
	ask_count := 0;
   repeat
   begin
		AskCurvename();		{ ask for curve name, see if it exists already }
		index := G_CURVE_SET_INDEX( CurveName, '', -1 );
							{ if it does already exist, warn user, ask again }
		if (index <> INVALID) then  
		begin
			writeln();
			write('Curve ');
			write(CurveName);
			write(' is already present  (Press Enter)');
			readln();
		end;
		ask_count := ask_count + 1;
	end
	until ((index = INVALID) OR (ask_count > 4));	{ until new name is entered }
	index := CS_COUNT();		{ find last curveset index, create new entry }
	CREATE_CURVE_SET( CurveName, '', Descript, index )
end;


{ --------------------------------------------------------------------
/
/	procedure 
/
/ -------------------------------------------------------------------- }

procedure AskAllCurves;
begin
	CurveTypeStr := '100% Intensity'; 
	AskExistCurvename();
	IntensityIndex := index;

	if (index <> INVALID) then	{ found that one? good, ask for next one }
	begin
		CurveTypeStr := 'Background'; 
		AskExistCurvename();
		BackgroundIndex := index;

		if (index <> INVALID) then  { found that one? good, ask for next one }
		begin
			CurveTypeStr := 'Source'; 
			AskExistCurvename();
			SourceIndex := index;

			if (index <> INVALID) then { found that one? good, ask for next one }
			begin
				CurveTypeStr := 'Absorbance Result'; 
				Descript := CurveTypeStr;		{ use prompt as description }
				AskNewCurvename();
				AbsorbIndex := index;
			end
		end
	end
end;


{ --------------------------------------------------------------------
/	program absorb; (main block)
/ -------------------------------------------------------------------- }

begin
	INVALID := 65535;
							{ ask at console for the names of curves }
	AskAllCurves();
							{ if all curves were successfully found/created }
	if ((IntensityIndex <> INVALID) 
		and (BackgroundIndex <> INVALID) 
		and (SourceIndex <> INVALID) 
		and (AbsorbIndex <> INVALID))  then
	begin			{ find number of curves in raw data curve set, 
					  create that many curves in Absorbance destination set }
		curves := cs[SourceIndex].count;
		curves := curves - 1;
		for i := 0 to curves do
		begin
			points := cs[SourceIndex].[i].point_count;
			create_curve( points, 0.0, TRUE, cs[AbsorbIndex].[i] ); 
		end; 

				{ calculate absorbance }
		cs[AbsorbIndex] := 
			LOG( (cs[IntensityIndex] - cs[BackgroundIndex]) /
              (cs[SourceIndex] - cs[BackgroundIndex]) );
	end
	else
	begin
		writeln();
		buflen := 5;
		write('Unable to find or create Curve Set(s).')
	end
end.

