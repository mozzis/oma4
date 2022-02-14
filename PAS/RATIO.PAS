PROGRAM Ratio;
{ This program will take the ration of the first two tracks of lastlive }
{ and plot the result on the screen repetitively }
{ To use, press F2 ('GoLive') then Alt-F10 and type RUN('RATIO.PAS') }

var i, j, k, reps : integer;

begin
  buflen := 10;                                     { fix bug in output }
  writeln();                                        { clear output line }
  write('Enter number of repetitions: ');
  readln(reps);                                     { get repetition no. }
  j := g_curve_set_index('MYCURVE','', 0);
  if j = -1 then begin
    create_curve_set('MYCURVE','','Ratio curve',0); { Make new curve set }
    j := g_curve_set_index('MYCURVE','',0);         { Find it in dir }
    create_curve(512,0,0,cs[j].[0]);                { Make curve for ratio }
  end;
  d_run(3);                                         { Start DA }
  k := g_curve_set_index('lastlive','',0);          { Find live data }
  
  cs[j].[0] := cs[k].[0] / cs[k].[1];               { Do first ratio }
  cs[k].display := 0;                               { don't plot lastlive }
  cs[j].display := 1;                               { plot ratio in win 1 }
  auto_scale();                                     { autoscale to ratio }
  { replot_window(); }
  
  for i := 1 to reps do begin                       { do updating }
    cs[j] := cs[k].[0] / cs[k].[1];                 { do ratio }
    { auto_scale(); }
    replot_window();                                { replot faster than }
  end;                                              { autoscale }
  d_run(0);                                         { run an ACCUM to stop }
end.
