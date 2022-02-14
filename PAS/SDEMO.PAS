program s_noise;             { calculate system noise using self-demo method }
                             { with added stuff for user friendliness }
  var ave, dev, noise : real;  { make up some variables }
    etime, gain : real;
    trax, points : word;
    bkdex, lbdex, choice : integer;
    stop : boolean;

begin
  start_fresh();             { so you can run program twice without exiting command line }
  buflen := 2;               { so printing (write, writeln) looks better }
  writeln();                 { clear the display line }
  trax := dg_tracks();       { get system number of tracks }
  points := dg_points();     { get system number of points }

  ds_shutmode(1);            { Set Shutter Forced Closed }
  ds_same_et(1);             { Set Keepclean = Normal to No Preps }
  etime := dg_et();          { Save old expose Time }
  ds_et(0);                  { Set Expose Time to 0 }
  s_accum(FALSE);            { Set Accum mode }
  writeln('Acquiring BACKGROUND data');
  go_live();                 { Run the Accum }
  lbdex := g_curve_set_index('lastlive','', 0);
  load_background(cs[lbdex], trax);
  s_accum(TRUE);                                 { Set Accum-Back mode }
  writeln('Acquiring SIGNAL data');
  go_live();
  bkdex := g_curve_set_index('backgrnd','', 0);  { see which curve set is background }
  lbdex := g_curve_set_index('lastlive','', 0);  { see which curve set is live-back }
  if bkdex <> -1 and lbdex <> -1 then begin      { if both sets exist }
    buflen := 10;             
    avg_std_dev(cs[lbdex].[0], trax, 0, points, ave, dev); { Get Std. Deviation }
    noise := dev / exp(2, 0.5);                            { Get square root of Dev. }
    write('Noise is ', noise, ' counts Continue (Y/N) ? ');{ Report the noise }
    choice := readch();
    if choice = 'Y' or choice = 'y' then begin
      ds_shutmode(0);        { Set Shutter Normal }
      ds_et(etime);          { Start with old ET }
      etime := dg_et();
      if etime = 0.0 then etime := 0.1;
      writeln();
      s_accum(FALSE);
      stop := FALSE;
      cmd_prompt('Press ESC to stop');
      repeat                 { search for good avg signal }
        ds_et(etime);
        go_live();
        auto_scale();
        lbdex := g_curve_set_index('lastlive','', 0);
        avg_std_dev(cs[lbdex].[0],trax,50,points-50,ave,dev);
        if ave > 150000 then 
          etime := etime - (etime / 10);
        if ave < 150000 then 
          etime := etime + (etime / 10);
        writeln('Need avg signal = 150,000, Curr. Avg: ', ave, ' New ET: ', etime);
        if keypressed then begin
          choice := readch();
          if choice = 129 then stop := TRUE;
        end;
      until (abs((ave - 150000)) < 10000) or (stop = TRUE);
      cmd_prompt('run(''SDEMO.PAS'')');
      if stop = FALSE then begin
        lbdex := g_curve_set_index('lastlive','', 0);
        load_background(cs[lbdex], trax);
        s_accum(TRUE);
        go_live();
        auto_scale();
        lbdex := g_curve_set_index('lastlive','', 0);
        bkdex := g_curve_set_index('backgrnd','', 0);
        avg_std_dev(cs[bkdex].[0],trax,0,points,ave,dev);
        writeln();
        write('Average signal is: ', ave, ' (Press a key)');
        choice := readch();
        avg_std_dev(cs[lbdex].[0],trax,0,points,ave,noise);
        noise := noise / exp(2, 0.5);
        writeln();
        write('Shot noise in counts: ', noise, ' (Press a key)');
        choice := readch();
        avg_std_dev(cs[bkdex].[0],trax,0,points,ave,dev);
        gain := (ave / (exp(noise, 2)));
        writeln();
        write('Gain: ', gain, ' photoelectrons/count');
      end
    end
  end
  else begin     { If curves did not exist }
    write('Acquire a background curve, then a Live-B curve.  Then run this again');
  end;
  writeln('  (Done)');
end.


