{ --------------------------------------------------------------------
/
/  program splice;
/
/  splice.pas is a Macro Language program to concatenate curves.
/  It will work for one or more curves in a curve set.  Each curve
/  in the source curve set will have the respective curve in the
/  destination curve set appended to it.
/
/
/ -------------------------------------------------------------------- }

program splice;

var i, j, k : integer;
    CurvePromptStr, CurveName, Descript : string;
    index, Invalid, points,
    dest_points, source_points,
    FirstSourceIndex, SecondSourceIndex, DestinationIndex,
    dest_curves, source1_curves, source2_curves: word;
    yes_flag : boolean;


{ --------------------------------------------------------------------
/
/  procedure AskIfYes
/
/  Waits for the user to type a yes or no response.  Actually is only
/  looking for a "Y"es, any other answer is assumed to be no.
/  The result is returned in the boolean variable "yes_flag".
/
/ -------------------------------------------------------------------- }

procedure AskIfYes;
VAR UpperY, LowerY, FirstChar : byte;
    answer : string;
begin
   UpperY := 'Y';
   LowerY := 'y';
   READLN(answer);
   FirstChar := answer;
   yes_flag := ((FirstChar = UpperY) or (FirstChar = LowerY));
end;


{ --------------------------------------------------------------------
/
/  function PromptForOverwrite
/
/  Asks the user permission to overwrite a curve set.  Waits for the
/  user to type a yes or no response.  Actually is only
/  looking for a "Y"es, any other answer is assumed to be no.
/  The result is returned in the boolean variable "yes_flag".
/
/ -------------------------------------------------------------------- }

procedure PromptForOverwrite;
begin
   writeln();
   write( 'Overwrite curve set "', CurveName, '"? (Y/N) ' );
   AskIfYes();
end;

{ --------------------------------------------------------------------
/
/  procedure AskCurvename
/
/  Prompt for a curve set name.  A description of what the curve set
/  is for is in "CurvePromptStr".  The curve set name is returned in
/  "CurveName".
/
/ -------------------------------------------------------------------- }

procedure AskCurvename;
begin
      WRITELN();
      buflen := 5;
      WRITE(CurvePromptStr);
      buflen := 30;
      READLN(CurveName);
      buflen := 5;
end;

{ --------------------------------------------------------------------
/
/  procedure AskExistCurvename
/
/  Prompts the user for a curve set name, then looks it up in the
/  curve directory.  If it does not exist, the user is prompted
/  again, but only three times.  The index of the curve set is
/  returned in "index" - if not found, it will be set to 65535
/  (which is what the "Invalid" psuedo-constant is used for).
/
/ -------------------------------------------------------------------- }

procedure AskExistCurvename;
var ask_count:integer;
begin
   ask_count := 0;
   REPEAT
   begin
      AskCurvename();      { ask for curve name, see if it exists }
      index := G_CURVE_SET_INDEX( CurveName, '', -1 );
      ask_count := ask_count + 1;
   end                         { don't ask more than 3 times }
   UNTIL ((index <> Invalid) OR (ask_count > 3));
end;

{ --------------------------------------------------------------------
/
/  procedure AskNewCurvename
/
/  Prompts the user for a curve set name, then looks it up in the
/  curve directory.  If it exists, the user is prompted about
/  overwriting it.  The index of the curve set is
/  returned in "index" - if not found, it will be set to the number of 
/  curve sets iin the directory, placing new curves at the end of the list
/  (which is what the "Invalid" psuedo-constant is used for).
/  If it is found and overwrite permission is not given index
/  is returned as Invalid.
/
/ -------------------------------------------------------------------- }

procedure AskNewCurvename;
begin
   AskCurvename();      { ask for curve name, see if it exists }
   index := G_CURVE_SET_INDEX( CurveName, '', -1 );
   IF ( (index <> FirstSourceIndex) and 
               (index <> SecondSourceIndex) ) THEN
   begin
      IF (index <> Invalid) THEN
      begin
         PromptForOverwrite();
         IF (yes_flag = FALSE) THEN
            index := Invalid
         ELSE
         BEGIN
               DEL_CURVE_SET( cs[index] )  { Delete the existing curve set }
         END
      end
      ELSE
         index := CS_COUNT();
   end
   ELSE
   BEGIN
      writeln();
      write( 'Writing over the source curves is not allowed!',
             ' Press ENTER to continue.');
      readln();
      index := Invalid;
   END

end;

{ --------------------------------------------------------------------
/
/  procedure AskForCurves
/
/  Ask for the names of the Destination and Source Curve Sets.
/
/ -------------------------------------------------------------------- }

procedure AskForCurves;
begin
   CurvePromptStr := 'First curve set name: ';
   AskExistCurvename();
   FirstSourceIndex := index;

   IF (index <> Invalid) THEN { found that one? good, ask for next one }
   begin
      CurvePromptStr := 'Second curve set name: ';
      AskExistCurvename();
      SecondSourceIndex := index;

      IF (index <> Invalid) THEN { found that one? good, ask for next one }
      begin
         CurvePromptStr := 'Resultant curve set name: ';
         AskNewCurvename();
         DestinationIndex := index;
      end
   end
end;


{ --------------------------------------------------------------------
/
/  program splice; (main block)
/
/  Asks user for curve set names.  If ok, checks to see that they have
/  the same number of curves.  If not, the user is asked if it is ok
/  to proceed.  If yes:  An outer loop iterates for each curve in the
/  curve set.  First, it enlarges the destination curve to fit the
/  added points.  Then, the inner loop appends the Y values from the
/  source curve on to the end of the destination curve.
/
/ -------------------------------------------------------------------- }

begin
   Invalid := 65535;
   buflen := 5;
                     { ask at console for the names of curves }
   AskForCurves();

                     { if source curves were successfully found }
   IF ( (DestinationIndex <> Invalid) and (FirstSourceIndex <> Invalid)
       and (SecondSourceIndex <> Invalid) ) THEN
   begin
      source1_curves := CS[FirstSourceIndex].COUNT;

      { copy the destination curve from the first source curve set }
      INS_CURVE_SET( CS[FirstSourceIndex].[0],
                     source1_curves,
                     cs[DestinationIndex].[0],
                     CurveName, '',
                     0 );

      { find number of curves in first curve set }
      source2_curves := CS[SecondSourceIndex].COUNT;
      IF (source1_curves <> source2_curves) THEN
      begin
         WRITELN();
         WRITE('Curve Sets do not have same number of curves - Continue? ');
         AskIfYes();       { user will answer Y if ok to proceed }
         IF (source1_curves > source2_curves) THEN
         begin
            source1_curves := source2_curves;
         end
      end
      else
      begin
         yes_flag := TRUE  { proceed, curve sets have same number of curves }
      end;
      IF yes_flag THEN
      begin
         source1_curves := source1_curves - 1;
         FOR i := 0 TO source1_curves DO
         begin
                  { adjust destination curve size, to hold added curve }
            dest_points := CS[DestinationIndex].[i].POINT_COUNT;
            source_points := CS[SecondSourceIndex].[i].POINT_COUNT;
            points := (dest_points + source_points);
            CHANGE_CURVE_SIZE( CS[DestinationIndex].[i], points, TRUE, 0.0);

            source_points := source_points - 1;
            dest_points := dest_points - 1;

            WRITELN();
            WRITE('Splicing Curve: ', i);
                     { now append source Y values to end of dest curve }
            FOR j := 0 TO source_points DO
            begin
               CS[DestinationIndex].[i].Y[ dest_points + j ] :=
                  CS[SecondSourceIndex].[i].Y[j];
            end;
         end;
      end;
      WRITELN();
      WRITE('Done.');
   end
   else
   begin
      WRITELN();
      WRITE('Unable to find Curve Set(s).')
   end

end.

