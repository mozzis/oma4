{=================================================}
{ Compute the LCI gain so that proper setting of  }
{ the gain knob can be made.                      }
{=================================================}

program lcigain1;

var
  ave1 : real;       { average signal for 2 accummed scans }
  ave2 : real;       { really a dummy for subtracted AVG STD DEV calc }
  deva1 : real;      { STD DEV for 2 accummed scans }
  deva2 : real;      { dummy variable for subtracted scan }
  gain : real;       { LCI gain in photoelectrons per count }
  atext : TEXT;      { silly requirement to print }
  ffd : BYTE;        { printer form feed character }
  bell : BYTE;       { PC bell, beep, beep, aaahhhh, beep, beep! }
  response : STRING; { Y or N for yes or no }
  upper_n : BYTE;    { more silly stuff contrivances }
  lower_n : BYTE;
  first_char : BYTE;


{=============================================================}
{ Take a background scan and save it as a background scan.    }
{ Bring in methods file for acquiring signal as part of calc. }
{=============================================================}

procedure setup;

begin
  WRITELN();
  LOAD_METHOD('C:\PAR\LCIGAIN1.MET');
  S_ACCUM(false);
  GO_LIVE();
  AUTO_SCALE();
  LOAD_BACKGROUND(CS[0].[0],4);
  S_ACCUM(true);
  LOAD_METHOD('C:\PAR\LCIGAIN2.MET');
end;


{=======================================================}
{ Delete all files that are not needed before beginning }
{ Otherwise, the program won't run correctly.           }
{=======================================================}

procedure del_files;

begin
  REPEAT DEL_CURVE_SET(CS[CS_COUNT()-1]) UNTIL (CS_COUNT()<=0);
end;


{==================}
{ BEEP the console }
{==================}

procedure beep;

var
  bin : TEXT;

begin
  bell:=7;
  assign(bin,'CON');
  write(bin,bell);
  close(bin);
end;


{=======================}
{ FORM FEED the PRINTER }
{=======================}

procedure ff;

begin
  ffd:=12;
  assign(bin,'PRN');
  write(bin,ffd);
  close(bin);
end;


{====================================}
{ Get curves and calculate the gain. }
{====================================}

procedure get_data;

begin
  GO_LIVE();                   { Get first curve      }
  AUTO_SCALE();                { show it off          }
  CS[2]:=CS[1];                { save in curve 2      }
  GO_LIVE();                   { Get next curve       }
  AUTO_SCALE();                { Show it off, too.    }
  CS[3]:=CS[2];                { save in curve 3      }
  CS[4]:=CS[3]-CS[1];          { bkg subtract         }
  CS[5]:=CS[3]+CS[1];          { sum for total signal }
  DEL_CURVE_SET(CS[0]);        { Take 'em out the hard way }
  DEL_CURVE_SET(CS[0]);        { crv 0,1,2, and 3          }
  DEL_CURVE_SET(CS[0]);
  DEL_CURVE_SET(CS[0]);
{ Compute AVG STD DEV of bkg subtracted crv }
  AVG_STD_DEV(CS[0].[2],1,100,300,ave1,deva1);
  DEL_CURVE_SET(CS[0]);        { Then take it out }
{ Compute AVG STD DEV of summed crv }
  AVG_STD_DEV(CS[0].[2],1,100,300,ave2,deva2);
{ Then compute gain }
  gain:=(deva1*deva1)/ave2;
end;


{======================================================}
{ Pretty print the gain numbers so that technician can }
{ refer to them as gain is adjusted.                   }
{======================================================}

procedure pretty_print;

begin
  assign(atext,'PRN');
  writeln(atext,' ');
  writeln(atext,'_/~\_/~\_/~\_/~\/~~~~~~~~~~~~~~~~~~~~\/~\_/~\_/~\_/~\_');
  writeln(atext,'~\_/~\_/~\_/~\_/ LCI GAIN MEASUREMENT \_/~\_/~\_/~\_/~');
  writeln(atext,'_/~\_/~\_/~\_/~\______________________/~\_/~\_/~\_/~\_');
  writeln(atext,' ');
  close(atext);
end;


{===========================}
{ Print out the gain number }
{===========================}

procedure print_gain;

begin
  assign(atext,'PRN');
  writeln(atext,gain);
  close(atext);
end;


{==========================================}
{ Waste time so techcician can see answers }
{==========================================}

procedure waste_time;

var
  z : integer;
  q : integer;
begin
  z:=0;
  q:=0;
  REPEAT begin
         REPEAT z:=z+1 UNTIL z > 5000;
         q:=q+1
         end
  UNTIL q > 10;
end;


{======}
{ MAIN }
{======}

BEGIN

  begin
    upper_n:='N';
    lower_n:='n';
    pretty_print();
    REPEAT begin 		
      setup();                            { get bkg crv and prepare }
      get_data();                         { get accum crvs and calc gain }
      print_gain();
      writeln();
      writeln(' GAIN= ',gain);    { put gain on screen  }
      waste_time();
      WRITELN(' ');
      writeln('Take another gain measurement? (Y)es or (N)o:');
      READLN(response);
      first_char:=response;
      del_files();
    end
    UNTIL (first_char=upper_n OR first_char=lower_n);

    ff();
    
    S_LIVE(FALSE);
    GO_LIVE();
  end;
END.
