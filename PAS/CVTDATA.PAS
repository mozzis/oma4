{ Program converts a Dilor single curve data file to an OMA4000 data file }
{ Warning: The old Dilor data file will be erased if it is in the OMA4000 }
{ subdirectory! }

program cvtdata;

  var i, j, l : integer;
      cpoints : WORD;
      val : real;
      instr, fulname, fname, fpath : string;
      fspec : array[64] of byte;
      fext  : array[4] of byte;
      zbyte, cbyte : byte;
      infile : text;
      
{ Procedure to separate d:\path\name.ext type filename into path and name }

procedure spltpath;
  begin
      zbyte := 0;
      cbyte := '\';
      fspec := fulname;
      fname := fulname;
      fpath := fulname;
      l := strlen(fulname) - 1;
      { examine filespec from end to start to find first '\' char }
      for i := l downto 0 do begin
        if fspec[i] = cbyte then begin
          { found '\' char, terminate the fspec string there }
          fspec[i] := zbyte;
          { copy fspec up to first 0 byte into the fpath string }
          fpath := fspec;
          { copy rest of fspec (the name part) into beginning of array }
          for j := i+1 to l do
            fspec[j - (i+1)] := fspec[j];
          { terminate the name with a zero byte }
          fspec[j - (i+1)] := zbyte;
          { copy the name part into the fname string }
          fname := fspec;
          { make the search loop end here }
          i := 0;
        end;
      end;
      { if the filename and path are still equal, set the path string to nil }
      if fpath = fname then fpath := '';
    end;

    { main procedure }

  begin
     start_fresh();
     fext := '.DAT';
     { let the user enter the file name }
     writeln();
     write('Enter file name: ');
     readln(fulname);
     spltpath();
     j := g_curve_set_index(fname, '', 0);
     if j <> -1 then
       del_curve_set(cs[j]);
     create_curve_set(fname, '', 'Translated Dilor data',0);
     j := g_curve_set_index(fname, '', 0);
     assign(infile, fulname);
     for i := 0 to 2 do readln(infile,instr);
     readln(infile, cpoints);
     create_curve(cpoints,0,1,cs[j].[0]);
     for i := 4 to 15 do readln(infile,instr);
     buflen := 12;
     for i := 0 to cpoints - 1 do 
       begin
       readln(infile,instr);
       if ((i mod 10) = 0) then
         write(ftos(i), ': ', instr,' ');
       cs[j].[0].x[i] := stof(instr);
       readln(infile,instr);
       if ((i mod 10) = 0) then writeln(instr);
       cs[j].[0].y[i] := stof(instr);
       end;
     close(infile);
     fspec := fulname;
     l := strlen(fulname);
     for i := l - 4 to l do
       fspec[i] := fext[i - l + 4];
     fulname := fspec;
     writeln();
     write('Saving ', fulname);
     save_file_curves(fulname,cs[j],0,1,0,0);
   end.
