program profile;

var i : integer;
    cavg, cdev : real;
    
begin
  for i := 0 to 400 do begin
    writeln('Curve: ', i);
    avg_std_dev(cs[0].[i], 1, 0, 512, cavg, cdev);
    cs[1].[0].y[i] := cavg;
    cs[2].[0].y[i] := cs[0].[i].min;
  end;
  change_curve_size(cs[1].[0], 400, true, 0);
  avg_std_dev(cs[0].[25], 1, 0, 512, cavg, cdev);
  change_curve_size(cs[2].[0], 400, true, 0);
end.
