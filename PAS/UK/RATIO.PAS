PROGRAM Ratio;

{ This program will take the ratio of the first two tracks of lastlive }
{ and plot the result on the screen repetitively                       }

var i, j, k, reps : integer;

begin
  buflen := 10;                                     
  writeln();                                        { clear output line }
  write('Enter number of repetitions: ');
  readln(reps);                                     { get no. of reps   }
  j := g_curve_set_index('MYCURVE','', 0);
  if j = -1 then 
  begin
    create_curve_set('MYCURVE','','Ratio curve',0); { Make new curve set }
    j := g_curve_set_index('MYCURVE','',0);         { Find it in dir }
  end
  else
    begin
    del_curve_set(cs[j]);                           { Delete existing cs }
    create_curve_set('MYCURVE','','Ratio curve',0); { Make new curve set }
    j := g_curve_set_index('MYCURVE','',0);         { Find it in dir }
    end;
  d_run(3);                                         { Start DA }
  k := g_curve_set_index('lastlive','',0);          { Find live data }
  create_curve(cs[k].[0].point_count,0,0,cs[j].[0]);{ Make curve for ratio }
  cs[j].[0] := cs[k].[0] / cs[k].[1];               { Do first ratio }
  cs[k].display := 0;                               { don't plot lastlive }
  cs[j].display := 1;                               { plot ratio in win 1 }
  auto_scale();                                     { autoscale ratio plot }
  for i := 1 to (reps - 1) do 
  begin                                             { do updating }
    cs[j] := cs[k].[0] / cs[k].[1];                 { do ratio }
    auto_scale(); 
  end;                                              
  d_run(0);                                         { run an ACCUM to stop }
end.
