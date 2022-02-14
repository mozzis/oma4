program s_noise;             { calculate system noise using self-demo method }
                             { with added stuff for user friendliness }
var ave, dev, noise : real;  { make up some variables }
    trax, points : word;
    bkdex, lbdex : integer;
    choice : string;

begin
  start_fresh();             { so you can run program twice without exiting command line }
  buflen := 2;               { so printing (write, writeln) looks better }
  writeln();                 { clear the display line }
  trax := dg_tracks();       { get system number of tracks }
  points := dg_points();     { get system number of points }
  bkdex := g_curve_set_index('backgrnd','', 0);  { see which curve set is background }
  lbdex := g_curve_set_index('lastlive','', 0);  { see which curve set is live-back }
  if bkdex <> -1 and lbdex <> -1 then begin      { if both sets exist }
    write('Curve ', bkdex, 'has background and curve ' , lbdex, 'has Live-B. True? Y/N ');
    readln(choice);          { Ask user if OK to continue }
    writeln();               { Clear display line after response received }
    buflen := 6;             
    if choice = 'Y' or choice = 'y' then begin               { If the answer was 'Yes' }
      avg_std_dev(cs[lbdex].[0], trax, 0, points, ave, dev); { Get Std. Deviation }
      noise := dev / exp(2, 0.5);                            { Get square root of Dev. }
      write('Noise is ', noise);                             { Report the noise }
    end
    else begin   { If curves were wrong }
      write('Acquire a background curve, then a Live-B curve.  Then run this again');
    end;
  end
  else begin     { If curves did not exist }
    write('Acquire a background curve, then a Live-B curve.  Then run this again');
  end;
  writeln('  (Done)');
end.


