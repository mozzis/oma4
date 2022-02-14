program lnscan;

VAR j,k  :integer;
    m,n  :integer;
    savg, sdev: real;
    dstat:boolean;

{----------Procedure MAIN--------------------------------------}
begin
  start_fresh();
  k := 0;
  n := DG_POINTS();                       { Get number of points in curve }
  m := 0;                                 { Use D_RUN(0) the first time }
  BUFLEN := 10;
  writeln();
  j := G_CURVE_SET_INDEX('lastlive','',0); { Check if curve set exists   }
  if j = -1 then
    write('Set up two tracks and start Live first ')
  else 
  repeat begin
    D_RUN(m);                             { Run an ACCUM                }
    m := 2;                               { Do quicker start after 1st  }
    repeat                                { scan. }
      dstat := DG_ACTIVE();               { wait till ACCUM done        }
    until (dstat = false) or KEYPRESSED();
    avg_std_dev(cs[0].[0], 2, 0, n, savg, sdev);
    writeln(k, ': Energy: ', savg);
    k := k + 1;
  end until KEYPRESSED();
  writeln('(Done)');;
end.
