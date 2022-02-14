PROGRAM SCAN;
{---------------------------------------------------------------------------
 SPECTROGRAPH CONTROL DEMONSTRATION.

 This program demonstrates how the OMA-SPEC-4000 software can send and receive
 commands to an RS232 controlled instrument using the macro commands
 which directly access the IBM memory. Take note of the cautions contained
 in the manual regarding the use of these commands, in particular the
 S_PORTW() command.

 The program assumes that the instrument being controlled is an EG&G PAR
 spectrograph model 1235, coupled to the serial port on the PC. Edit the
 parameter PORT in the program heading to select the port (COM1 or COM2)
 required.

 The 1235 uses 9600 baud, 8 data bits, 1 stop bit and no parity as it's
 communications standard. Set these parameters up outside the OMA-SPEC
 program using the DOS MODE command. The format is MODE COM1 96 n 8 1 .

 The program will change the spectrograph grating and then drive
 the spectrograph to 4 different centre wavelengths.
 You can modify it so that at each of these wavelengths, the detector
 is addressed and a spectrum recorded. You would then need to splice
 these spectra end-to-end to give a single long curve.

 Author:       Richard Burdett
               EG&G Instruments Ltd, Wokingham, Berks, RG11 2GY

 Date:         19th December 1991
 Rev:          0.5
---------------------------------------------------------------------------}


VAR
    PART_1_CW, PART_2_CW, PART_3_CW, PART_4_CW, CW            : REAL;
    T, COM_PORT, ERR                                          : INTEGER;
    PORT_ADDRESS,SND_BYTE, RSP_BYTE                           : WORD;
    HUNS, TENS, UNITS, TENTHS                                 : INTEGER;
    NO_PARTS, GRATING                                         : INTEGER;

{following section assigns values to the variables-------------------------}
PROCEDURE SET_VAR;
BEGIN
 BUFLEN:= 10;
 COM_PORT :=    2     ; { Set to 1 for COM1 or 2 for COM2   }
 PART_1_CW:=    451.0 ; { First centre wavelength, set to 1 decimal place  }
 PART_2_CW:=    500.0 ; { Second centre wavelength, set to 1 decimal place }
 PART_3_CW:=    549.0 ; { Third centre wavelength, set to 1 decimal place  }
 PART_4_CW:=    598.0 ; { Fourth centre wavelength, set to 1 decimal place }
 GRATING:=      3     ; { spectrograph grating number}
END;

{following section defines a long delay------------------------------------}
PROCEDURE LDELAY;
BEGIN
 FOR T:=1 TO 3000 DO
 BEGIN
 END;
END;

{following section defines a standard delay -------------------------------}
PROCEDURE DELAY;
BEGIN
 FOR T:=1 to 500 DO
 BEGIN
 END;
END;

{following section defines a short delay, used to ensure that the RS232
interface can keep up with the speed at which the macro writes to the
RS232 port----------------------------------------------------------------}
PROCEDURE SDELAY;
BEGIN
 FOR T:= 1 to 100 DO
 BEGIN
 END;
END;

{following section handles error conditions-------------------------------}
PROCEDURE ERROR;
BEGIN
 WRITELN();
 BUFLEN:=42;
 WRITE('ERROR: PROGRAM TERMINATED');
 DELAY();
 ERR:=1;
END;

{following section sends the individual characters-------------------------}
PROCEDURE TRANSMIT;
BEGIN
 S_PORT(PORT_ADDRESS,SND_BYTE);   {send the character}
 SDELAY();
 RSP_BYTE:= G_PORT(PORT_ADDRESS); {clear the input buffer}
END;

{following sends the GOTO command as a string of ASCII characters --------}
PROCEDURE SEND_GOTO;
BEGIN
 SND_BYTE:=  32;  {space}
 TRANSMIT();
 SND_BYTE:=  71;  {G}
 TRANSMIT();
 SND_BYTE:=  79;  {O}
 TRANSMIT();
 SND_BYTE:=  84;  {T}
 TRANSMIT();
 SND_BYTE:=  79;  {O}
 TRANSMIT();
END;

{following sends the GRATING command as a string of ASCII characters------}
PROCEDURE SEND_GRATING;
BEGIN
 SND_BYTE:=  71;  {G}
 TRANSMIT();
 SND_BYTE:=  82;  {R}
 TRANSMIT();
 SND_BYTE:=  65;  {A}
 TRANSMIT();
 SND_BYTE:=  84;  {T}
 TRANSMIT();
 SND_BYTE:=  73;  {I}
 TRANSMIT();
 SND_BYTE:=  78;  {N}
 TRANSMIT();
 SND_BYTE:=  71;  {G}
 TRANSMIT();
END;

{following sends the terminator OK <CR><LF> as required by 1235 ---------}
PROCEDURE SEND_TERM;
BEGIN
 SND_BYTE:=  32;  {space}
 TRANSMIT();
 SND_BYTE:=  79;  {O}
 TRANSMIT();
 SND_BYTE:=  75;  {K}
 TRANSMIT();
 SND_BYTE:=  13;  {carriage return}
 TRANSMIT();
 SND_BYTE:=  10;  {line feed}
 TRANSMIT();
 SND_BYTE:=  13;  {carriage return}
 TRANSMIT();
END;

{following converts real centre wavelength value to a series of ASCII
characters and sends them, followed by GOTO OK <CR><LF> ----------------}
PROCEDURE SEND_CW;
BEGIN
 HUNS:=   TRUNC(CW/100);
 TENS:=   TRUNC((CW-(HUNS * 100))/10);
 UNITS:=  TRUNC(CW-(HUNS * 100)-(TENS *10));
 TENTHS:= 10*(CW-(HUNS*100)-(TENS*10)-UNITS);
 SND_BYTE:= 48 + HUNS;    {convert hundreds digit to ASCII}
 TRANSMIT();
 SND_BYTE:= 48 + TENS;    {convert tens     digit to ASCII}
 TRANSMIT();
 SND_BYTE:= 48 + UNITS;   {convert units    digit to ASCII}
 TRANSMIT();
 SND_BYTE:= 46;           {send a decimal point .         }
 TRANSMIT();
 SND_BYTE:= 48+  TENTHS;  {convert tenths   digit to ASCII}
 TRANSMIT();
 SEND_GOTO();
 SEND_TERM();
END;

{following converts GRATING integer into ASCII and sends it, followed by
GRATING OK <CR><LF>--------------------------------------------------------}
PROCEDURE SET_GRATING;
BEGIN
 IF GRATING = 1 THEN GRATING:=49 ELSE
 IF GRATING = 2 THEN GRATING:=50 ELSE
 IF GRATING = 3 THEN GRATING:=51 ELSE
 ERROR();
 SND_BYTE:= GRATING;
 TRANSMIT();
 SEND_GRATING();
 SEND_TERM();
END;

{main program starts here-------------------------------------------------}
BEGIN
SET_VAR();

{following section defines which port to send data to --------------------}
IF      (COM_PORT = 1) THEN
 PORT_ADDRESS:= 1016
ELSE IF (COM_PORT = 2) THEN
 PORT_ADDRESS:=  760
ELSE ERROR();


IF ERR= 0 THEN
BEGIN

BUFLEN:= 57;
WRITELN();
WRITE('SPECTROGRAPH CONTROL DEMONSTRATION');
LDELAY();
WRITELN();
BUFLEN:=18;
WRITE('Changing to grating');
BUFLEN:=2;
WRITE(GRATING);
BUFLEN:=29;
WRITE(';press <ENTER> when this finishes');
SET_GRATING();
READLN();

BUFLEN:= 72;
WRITELN();
WRITE('The spectrograph will be driven to 4 different centre wavelengths');
DELAY();

CW:= PART_1_CW;
SEND_CW();
WRITELN();
BUFLEN:=28;
WRITE('Moving to centre wavelength =');
BUFLEN:=4;
WRITE(CW,'nm');
LDELAY();

CW:= PART_2_CW;
SEND_CW();
WRITELN();
BUFLEN:=28;
WRITE('Moving to centre wavelength =');
BUFLEN:=4;
WRITE(CW,'nm');
LDELAY();

CW:= PART_3_CW;
SEND_CW();
WRITELN();
BUFLEN:=28;
WRITE('Moving to centre wavelength =');
BUFLEN:=4;
WRITE(CW,'nm');
LDELAY();

CW:=PART_4_CW;
SEND_CW();
WRITELN();
BUFLEN:=28;
WRITE('Moving to centre wavelength =');
BUFLEN:=4;
WRITE(CW,'nm');
LDELAY();
END;

DELAY();
WRITELN();
BUFLEN:=47;
WRITE('End of Program');
END.

{end of program-----------------------------------------------------------}
