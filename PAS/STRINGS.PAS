PROGRAM Strings;

{ This program demonstrates some of the techniques for manipulating }
{ strings with the OMA4000 version 2.60 and later                   }

VAR

  ASTRING : string;               { hold the test string }
  ANARRAY : array[81] of byte;    { must be large enough to hold a string }
  NEWCHAR : array[10] of byte;
  ACHAR   : byte;                 { for moving characters around }
  ACOUNT  : integer;              { indexer }
  AREAL   : real;

begin
  start_fresh();
  BUFLEN := 1;
  NEWCHAR := 'Best Thing';        { Set up for later replace }
  ASTRING := 'Test String';       { Initialize test string }
  ANARRAY := ASTRING;             { Move characters into the array };

  writeln();                      { display the array contents }
  write('Array contains: ');
  for ACOUNT := 0 to strlen(ASTRING) do
    write(ANARRAY[ACOUNT]);
  
  ACHAR := 'B';                   { To replace characters in the array, }
  ANARRAY[0] := ACHAR;            { must use var of type BYTE, else     }
  ACHAR := 'T';                   { character after target byte becomes 0 }
                          
  for ACOUNT := 5 to 9 do         { replace part of string 1 with string 2 }
    ANARRAY[ACOUNT] := NEWCHAR[ACOUNT];
  ANARRAY[10] := 0;

  write(' (change to) ');         { show array contents }
  for ACOUNT := 0 to strlen(ASTRING) do
    write(ANARRAY[ACOUNT]);
  readch();                       { wait for user keypress }

    { the following only works in 2.64 or later }

  ASTRING := ANARRAY;             { copy array back to string }
  writeln();
  write(ASTRING);           { show new string contents }
  readch();                       { wait for user keypress }

  BUFLEN := 10;

  AREAL := 5432.1;                { convert real to ASCII }
  ASTRING := ftos(AREAL);
  writeln();
  write('Real: ', AREAL, ' String: ', ASTRING);  
  readch();                       { wait for user keypress }

  ASTRING := '1234.5';            { convert ASCII to real }
  AREAL := stof(ASTRING);
  writeln();
  write('String: ', ASTRING, ' Real: ', AREAL, ' (Done).');  
end.
