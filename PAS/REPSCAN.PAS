program repscan;

{ Shows how to acquire multiple scans into a curve set named by the user }
{ This program DOES NOT require that the 'lastlive' curve set already exist}

VAR i,j,scans:integer;
    n:integer;
    fname,resp:string;
    dstat:boolean;

{----------Procedure START-------------------------------------}
procedure start;
begin
  DS_DA(1);                { Ensure DA 1 }
  DS_I(1);                 { Set one scan per mem }
  DS_J(1);                 { Set one mem per RUN }
  DS_K(0);                 { Set no ignores }
end;

{----------Procedure MAIN--------------------------------------}
begin
  start();
  n := D_NUMPOINTS();                       { Get number of points in curve }
  BUFLEN := 10;
  writeln();
  write('Enter number of scans: ');         { Ask number of curves to get }
  readln(scans);
  if scans then begin
    writeln();
    write('Enter new name: ');              { Ask curve set name          }
    readln(fname);
    j := G_CURVE_SET_INDEX(fname,'',0);     { Check if curve set exists   }
																						{ if already exists, new data }
																						{ will append to old          }
    if j = -1 then begin                    { if not then create it       }
      CREATE_CURVE_SET(fname,'','Testl',0); 
      j := G_CURVE_SET_INDEX(fname,'',0);   
    end;
    if j <> -1 then begin
      CS[j].DISPLAY := 1;                   { Cause new set to plot on 1  }
      DS_MEM(1);                            { reset start mem to default  }
      for i := 0 to scans-1 do begin
        writeln('step number : ',i+1);
        D_RUN();                           { Run an ACCUM                }
        repeat
          dstat := DG_ACTIVE();             { wait till ACCUM done        }
        until (dstat = false) or KEYPRESSED();
    
    { D_READ_MEM is used here since it does not require lastlive to exist }
    { The new data are always placed in CS[j].[0] (in front of old curves)}
    
        D_READ_MEM(1, 0, n, CS[j].[0]);     { get data just acquired      }
        CS[j].[0] := CS[j].[0] * (i+1);     { Perform a trivial calc      }
        if KEYPRESSED() then i := scans;    { if user presses key abort   }
      end;
      writeln();
      write('Save ', fname, ' to disk? (Y/N) ');
      READLN(resp);
      if (RESP = 'Y') or (RESP = 'y') then begin
        SAVE_FILE_CURVES(fname, cs[j], 0, i, 0, 0);
      end;
    end;
  end;
  writeln('Done');;
end.
