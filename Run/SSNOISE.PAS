program s_noise;             { calculate system noise using self-demo method }
                             { ASSUMES background is cs[0], lastlive is cs[1] }
var ave, dev, noise : real;  { make up some variables }

begin
  buflen := 6;             
  avg_std_dev(cs[1].[0], 2, 0, 512, ave, dev); { Get Std. Deviation }
  noise := dev / exp(2, 0.5);                  { Get square root of Dev. }
  write('Noise is ', noise);                   { Report the noise }
end.


