PROGRAM PROGSPEC;
{----------------------------------------------------------------------------}

{ This program allows control of an associated M1235 triple grating
spectrograph from within the OMA4000/2000 software. It may be run from the
macro command line by typing RUN('PROG1235.PAS'). It
allows the user to select grating number and centre wavelength, and
assumes that the M1235 is coupled via the IEEE interface to the interface
card in the computer, and set up as device 11.

In view of the fact that revisions 2.17 and above of OMA SPEC 4000 allow
direct control of the spectrgraph, the real purpose of this macro is to
demonstrate how IEEE instruments can be controlled through the macro
environment.

It may be used as the basis of more complex programs, including ones which
cause the x-axis to be automatically recalibrated.

Author:   Richard Burdett
          EG&G Instruments Ltd, Wokingham, Berks, RG11 2GY.

Revisions: 0.86 : 13/06/1991
           1.0  : 21/07/1992 Major revision to include only
                  one subroutine for IEEE communications, and reduced
                  program size.

-----------------------------------------------------------------------------}


{Define variables used-------------------------------------------------------}
VAR GRATING,WAVELENGTH,CR,SCMD,CMD,PCMD,REPLY,INPUT,MSSGE:      STRING;
    UPPER_X,LOWER_X,UPPER_V,LOWER_V,UPPER_G,LOWER_G,FIRST_CHAR: BYTE;
    WAVECHNG,GRATCHNG,EXIT,I,SPOLL,RSP_LEN:                     INTEGER;
    RESPONSE,ERROR,COMPLETE:                                    BOOLEAN;

{Define short delay}
PROCEDURE SHORT_DLY;
 BEGIN
  FOR I:= 1 TO 100 DO;
  BEGIN
  END;
 END;

{Define long delay}
PROCEDURE LONG_DLY;
 BEGIN
  FOR I:= 1 TO 1000 DO;
  BEGIN
  END;
 END;

{Define procedure to process error condition on IEEE bus --------------------}
PROCEDURE ERROR_ROUTINE;
 BEGIN
  WRITELN();
  BUFLEN:=65;
  WRITE('Communication error with the 1235:- program terminated');
  LONG_DLY();
  EXIT:=1;
 END;

{Define procedure to send commands to and receive response from M1235 -------}
PROCEDURE SND_REC;
 BEGIN

 {append the terminator to the command string}
 CR:=$0D;
 SCMD:=CMD+CR;

 {specify the number of bytes to be sent, and send command}
 BUFLEN:=STRLEN(SCMD);
 GPIB_SEND(11,SCMD);

 {now perform serial poll to check that command is done and the response
 ready to read or to see if a command error occurred}
 SPOLL:=GPIB_SERIAL_POLL(11);

 {derive status from bits of the serial poll status byte returned by the 1235}
 COMPLETE:= (1 AND SPOLL);
 ERROR:= (2 AND SPOLL);
 RESPONSE:=(128 and spoll);

 {if command error occured then goto error_routine}
 IF (ERROR = 2) THEN
  ERROR_ROUTINE();

 {repeat serial polls until the command is done, writing progress message
 to screen if necessary}
    WHILE (COMPLETE = 0) DO
     BEGIN
      SPOLL:=GPIB_SERIAL_POLL(11);
      COMPLETE:= (1 AND SPOLL);
      RESPONSE:= (128 AND SPOLL);
       BEGIN
        {write message to screen}
        IF WAVECHNG = 1 THEN
        BEGIN
         MSSGE:= 'CHANGING WAVELENGTH:- PLEASE WAIT';
         BUFLEN:=34;
        END;
        IF GRATCHNG = 1 THEN
        BEGIN
         MSSGE:= 'CHANGING GRATING:- PLEASE WAIT';
         BUFLEN:=31;
        END;
        IF (GRATCHNG OR WAVECHNG = 1) THEN
        BEGIN
         WRITE(MSSGE);
         LONG_DLY();
         WRITELN();
         SHORT_DLY();
        END;
       END;
     END;

   {if command gave rise to response then read it and store in variable
   REPLY}
   IF (RESPONSE=128) THEN
    BEGIN
     BUFLEN:=RSP_LEN;
     GPIB_RESPONSE(11,REPLY);
    END;
END;

{****************************************************************************}
{Start of main program ------------------------------------------------------}

{ writes title to screen, waits a short time, then enters main program loop  }
BEGIN
  BUFLEN:=61;
  WRITELN();
  WRITE('MODEL 1235 SPECTROGRAPH CONTROL PROGRAM');
  LONG_DLY();


{Start of main program loop--------------------------------------------------}

{ this runs until the variable EXIT is set equal to 1                        }
WHILE (EXIT=0) DO

BEGIN
 {---------------------------------------------------------------------------}
 {generate a command to interrogate the 1235 for the current grating and
 send this over the IEEE bus}
 CMD:='?GRATING';
 RSP_LEN:=4;
 SND_REC();
 {now store the REPLY into the GRATING variable}
 GRATING:=REPLY;

 {--------------------------------------------------------------------------}
 {repeat above but for the command which asks the 1235 for the current centre
 wavelength....}
 CMD:='?NM';
 RSP_LEN:=7;
 SND_REC();
 {..and store the reply to variable WAVELENGTH}
 WAVELENGTH:=REPLY;

 {---------------------------------------------------------------------------}
 {the next section writes the main menu to the screen and prompts the
 user to enter the required option, these being to change grating, wavelength
 or to exit the program}
 WRITELN();
 BUFLEN:=9;
 WRITE('Grating:= ');
 BUFLEN:=1;
 WRITE(GRATING);
 BUFLEN:=14;
 WRITE('Mid w/length:= ');
 BUFLEN:=5;
 WRITE(WAVELENGTH);
 BUFLEN:=38;
 WRITE('nm  Change ? <waVelength,Grating,eXit> ');

 {following section reads required action from user}
 BUFLEN:=1;
 READLN(INPUT);
 FIRST_CHAR:=INPUT;

 {the variable INPUT now contains either V,G or X.
 the following section assigns ASCII characters to each of the six variables
 ; one for each of lower and upper case V,G & X. Hence when the test is
 performed, it doesn't matter whether the user entered a lower or upper case
 letter}
 LOWER_X:='x';
 UPPER_X:='X';
 LOWER_V:='v';
 UPPER_V:='V';
 LOWER_G:='g';
 UPPER_G:='G';

 {now perform the test on the FIRST_CHAR variable to see what the required
 option is}
 IF (FIRST_CHAR = LOWER_X) THEN
  EXIT:=1;
 IF (FIRST_CHAR = UPPER_X) THEN
  EXIT:=1;
 IF (FIRST_CHAR = LOWER_V) THEN
  WAVECHNG:=1;
 IF (FIRST_CHAR = UPPER_V) THEN
  WAVECHNG:=1;
 IF (FIRST_CHAR = LOWER_G) THEN
  GRATCHNG:=1;
 IF (FIRST_CHAR = UPPER_G) THEN
  GRATCHNG:=1;

 {---------------------------------------------------------------------------}
 {If the user wants to change centre wavelength, then proceed as follows}
 IF (WAVECHNG = 1) THEN
  BEGIN
   {write current centre wavelength to screen and ask for required value}
   WRITELN();
   BUFLEN:=22;
   WRITE('Current wavelength is:- ');
   BUFLEN:=5;
   WRITE(WAVELENGTH);
   BUFLEN:=3;
   WRITE('nm ');
   BUFLEN:=14;
   WRITE('Change to ? ');
   BUFLEN:=7;
   READLN(INPUT);
   {generate command to set current wavelength to required value}
   PCMD:= ' GOTO';
   CMD:=INPUT+PCMD;
   SND_REC();
   {reset WAVECHNG variable to 0}
   WAVECHNG:=0;
  END;

{----------------------------------------------------------------------------}
{if user wants to change grating, then proceed as follows}
IF (GRATCHNG = 1) THEN
 BEGIN
  {write current grating number to screen and ask for required grating}
  WRITELN();
  BUFLEN:=19;
  WRITE('Current grating is:- ');
  BUFLEN:=2;
  WRITE(GRATING);
  BUFLEN:=26;
  WRITE('Change to grating number ? ');
  BUFLEN:=1;
  READLN(INPUT);
  {generate command to set grating to required number}
  PCMD:=' GRATING';
  CMD:= INPUT+PCMD;
  SND_REC();
  {reset GRATCHNG variable to 0}
  GRATCHNG:=0;
 END;
END;

{----------------------------------------------------------------------------}
{following lines execute only when EXIT = 1, i.e at the end of the program}
WRITELN();
END.
{end of program -------------------------------------------------------------}
