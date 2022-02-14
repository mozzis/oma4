program compas;

var cdata   : byte;
    cbuff   : array[12] of byte;
    cinit   : string;
    cresp   : string;
    oldcl   : string;
    i, j    : integer;
    COMLEN  : integer;
    PORT    : integer;
    comsbad : BOOLEAN;
    comstat : BOOLEAN;

begin
  start_fresh();
  buflen := 6;
  PORT   := 2;
  cinit  := 'ATZ';
  cbuff  := cinit;
  i := strlen(cinit);
  cbuff[i] := 13;cbuff[i+1] := 0;
  oldcl := GCMD_PROMPT();
  SCMD_PROMPT('Press any key to abort');
  com_init(PORT, 2400, 0, 8, 1);
  j := 0;
  repeat
    comstat := com_out(PORT, cbuff[i]);
    j := j + 1;
  until (comstat AND 32768) or (j > i);
  writeln();
  if comstat AND 32768 then begin
    writeln();
    write('Timeout error transmitting init string (Done)');
  end
  else begin
    repeat 
      comstat := com_stat(PORT);
      writeln('Waiting for input ready', comstat);
    until((comstat AND 256) or keypressed());
    if keypressed() then readch();
    i := 0;
    comsbad := FALSE;
    repeat
      comstat := com_in(PORT, cbuff[i]);
      if (comstat and 32768) then comsbad := TRUE else i := i + 1;
    until( (i > 10) or comsbad or ((comstat AND 256) = 0) or keypressed());
    if comsbad then begin
      buflen := 4;
      writeln();
      write('Timeout error receiving response string (Done)');
    end
    else begin
      COMLEN := i;
      buflen := 2;
      for i := 0 to COMLEN do begin
        j := cbuff[i];
        write('<',j,'>');
        if cbuff[i] = 13 then cbuff[i] := 32;
      end;
      buflen := 12;
      cbuff[COMLEN] := 0;
      cresp := cbuff;
      if keypressed() then readch();
      write(' : ', cresp);
    end;
  end;
  buflen := 79;
  SCMD_PROMPT(oldcl);
end.
