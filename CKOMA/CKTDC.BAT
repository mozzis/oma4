@REM ***********************************************************
@REM *  THIS WILL TEST THE DETECTOR CONTROLLER BOARD.          *
@REM *                                           VER 2.20      *
@REM ***********************************************************
@BREAK ON
:BEGIN
@CLS
@ECHO.
@ECHO CKTDC      OMAIV Thompson CCD Detector Controller
@ECHO            Diagnostic                                    Version 2.20
@ECHO.
@ECHO            This diagnostic will run several tests.  Each 
@ECHO            test will repeat util a key is depressed.  The
@ECHO            Test are as follows:
@ECHO.
@ECHO          - LOOPBACK This will check the Loopback data path.
@ECHO          - CKPIA    This will test the PIA's input and output.
@ECHO          - CKAD     This will check the normal data path. 
@ECHO          - CKTEMPL  This will verify the Temperature interrupt.
@ECHO          - CKSEQ    This will verify the operation of the sequencer.
@ECHO          - CKTRIG   This will check the Trigger Input.
@ECHO          - CKSYNC   This will verify the External Sync. Input.
@ECHO.
@REM -----------------------------------------------------------
@REM    CHECKING FOR ARGUMENTS
@REM -----------------------------------------------------------
@IF "%1" == "" GOTO LISTARG
@IF %2 == 4 GOTO CONTINUE
@IF %2 == 5 GOTO CONTINUE
@IF %2 == 7 GOTO CONTINUE
@GOTO MEMERROR
:CONTINUE
@ECHO          - Press any key to continue or the ESC key
@ECHO            to bypass the OMA Controller fitting routine.
@EPAUSE
@IF ERRORLEVEL 1 GOTO LBTEST
@PORTW %1 0 0
@SET MEMVAL=FF
@REM -----------------------------------------------------------
@REM    SIZING SYSTEM MEMORY
@REM -----------------------------------------------------------
@CLS
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO          - Fitting the OMA Controller board into your system ...            
@MEMTEST 200000 -C2 -B -Q
@IF ERRORLEVEL 1 GOTO BLOCK2
@MEMTEST 400000 -C2 -B -Q
@IF ERRORLEVEL 1 GOTO BLOCK4
@MEMTEST 600000 -C2 -B -Q
@IF ERRORLEVEL 1 GOTO BLOCK6
@MEMTEST 800000 -C2 -B -Q
@IF ERRORLEVEL 1 GOTO BLOCK8
@MEMTEST A00000 -C2 -B -Q
@IF ERRORLEVEL 1 GOTO BLOCKA
@MEMTEST C00000 -C2 -B -Q
@IF ERRORLEVEL 1 GOTO BLOCKC
@GOTO BLOCKF
@REM -----------------------------------------------------------
@REM    2MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCK2
@SET MEMVAL=83
@SET ADDRVAL=200000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM    4MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCK4
@SET MEMVAL=85
@SET ADDRVAL=400000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM    6MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCK6
@SET MEMVAL=89
@SET ADDRVAL=600000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM    8MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCK8
@SET MEMVAL=91
@SET ADDRVAL=800000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM   10MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCKA
@SET MEMVAL=A1
@SET ADDRVAL=A00000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM   12MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCKC
@SET MEMVAL=C1
@SET ADDRVAL=C00000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM    DO TESTS
@REM -----------------------------------------------------------
:DOTESTS
@IF %MEMVAL% == FF GOTO MEMERROR
@PORTW %1 0 %MEMVAL%
@LOADER -A%ADDRVAL% -P%1 -S2 -MMONITOR.HEX >NUL
@IF ERRORLEVEL 1 GOTO DERROR
@IF %2 == 4 SET PORTVAL=9
@IF %2 == 5 SET PORTVAL=9
@IF %2 == 7 SET PORTVAL=9
@REM -----------------------------------------------------------
@REM    Do Loopback test
@REM -----------------------------------------------------------
:LBTEST
@PORTW %1 0 %MEMVAL%
@LOADER -A%ADDRVAL% -P%1 -S2 -DLOOPBACK.DBJ LOOPBACK.ABJ >NUL
@IF ERRORLEVEL 1 GOTO DERROR
@CLS
@ECHO.
@ECHO LOOPBACK   OMAIV Controller Loopback Diagnostic         Version 2.20
@ECHO.
@ECHO          - Each test should take 3 seconds.  If no results are
@ECHO            displayed within this time,  the result is negative.
@ECHO.
@ECHO          - Press any key to continue or ESC to return to DOS.
@ECHO.
@PORTW %1 1%PORTVAL% %MEMVAL% 
@PORTW %1 3%PORTVAL% %MEMVAL% 
@REM -----------------------------------------------------------
@REM    WAIT FOR THE USER TO TERMINATE TEST
@REM -----------------------------------------------------------
:DACLOOP
@ckint -P%1 -Q
@if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L35
@ckkey
@if not errorlevel 1  goto DACLOOP
@if errorlevel 27 if not errorlevel 28 goto DONE
@PORTW %1 0 %MEMVAL%
@REM -----------------------------------------------------------
@REM    Do PIA test
@REM -----------------------------------------------------------
@SET ERR=.
@LOADER -A%ADDRVAL% -P%1 -S2 -DCKPIA.DBJ CKPIA.ABJ >NUL
@IF ERRORLEVEL 1 GOTO DERROR
@CLS
@ECHO.
@ECHO CKPIA      OMAIV CDD Detector Controller 
@ECHO            PIA Diagnostic                               Version 2.20
@ECHO.         
@ECHO            This test must have the PIA test connector on the PIA port.
@ECHO.         
@ECHO          - Each test should take 1 second.  If no results are
@ECHO            displayed within this time,  the result is negative.
@ECHO.
@ECHO          - Press any key to continue or ESC to return to DOS.
@ECHO.
@PORTW %1 1%PORTVAL% %MEMVAL% 
@PORTW %1 3%PORTVAL% %MEMVAL% 
@REM -----------------------------------------------------------
@REM    WAIT FOR THE USER TO TERMINATE TEST
@REM -----------------------------------------------------------
:LPBLOOP
@ckint -P%1 -Q
@if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L35
@ckkey
@if not errorlevel 1 goto LPBLOOP
@if errorlevel 27 if not errorlevel 28 goto DONE
@PORTW %1 0 %MEMVAL%
@REM -----------------------------------------------------------
@REM    Do A/D test
@REM -----------------------------------------------------------
@SET ERR=.
@LOADER -A%ADDRVAL% -P%1 -S2 -DCKAD.DBJ CKAD.ABJ >NUL
@IF ERRORLEVEL 1 GOTO DERROR
@CLS
@ECHO.
@ECHO CKAD       OMAIV CCD Detector Controller 
@ECHO            Analog to Digital Converter Diagnostic           Version 2.20
@ECHO.         
@ECHO            This test requires the use of the OMAIV Digital Test Board.
@ECHO.         
@ECHO          - Each test should take 1 second.  If no results are
@ECHO            displayed within this time,  the result is negative.
@ECHO.
@ECHO          - Press any key to continue or ESC to return to DOS.
@ECHO.
@PORTW %1 1%PORTVAL% %MEMVAL% 
@PORTW %1 3%PORTVAL% %MEMVAL% 
@REM -----------------------------------------------------------
@REM    WAIT FOR THE USER TO TERMINATE TEST
@REM -----------------------------------------------------------
:ADCLOOP
@ckint -P%1 -Q
@if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L35
@ckkey
@if not errorlevel 1 goto ADCLOOP
@if errorlevel 27 if not errorlevel 28 goto DONE
@PORTW %1 0 %MEMVAL%
@REM -----------------------------------------------------------
@REM    Do Temperature Locked test
@REM -----------------------------------------------------------
@SET ERR=.
@LOADER -A%ADDRVAL% -P%1 -S2 -DCKTEMPL.DBJ CKTEMPL.ABJ >NUL
@IF ERRORLEVEL 1 GOTO DERROR
@CLS
@ECHO.
@ECHO CKTEMPL    OMAIV CCD Detector Controller 
@ECHO            Temperature Locked Diagnostic                    Version 2.20
@ECHO.         
@ECHO            This test requires the use of the OMAIV Digital Test Board.
@ECHO.         
@ECHO          - Each test should take 1 second.  If no results are
@ECHO            displayed within this time,  the result is negative.
@ECHO.
@ECHO          - Press any key to continue or ESC to return to DOS.
@ECHO.
@PORTW %1 1%PORTVAL% %MEMVAL% 
@PORTW %1 3%PORTVAL% %MEMVAL% 
@REM -----------------------------------------------------------
@REM    WAIT FOR THE USER TO TERMINATE TEST
@REM -----------------------------------------------------------
:TELLOOP
@ckint -P%1 -Q
@if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L35
@ckkey
@if not errorlevel 1 goto TELLOOP
@if errorlevel 27 if not errorlevel 28 goto DONE
@PORTW %1 0 %MEMVAL%
@REM -----------------------------------------------------------
@REM    Do Sequencer test
@REM -----------------------------------------------------------
@SET ERR=.
@LOADER -A%ADDRVAL% -P%1 -S2 -DCKSEQ.DBJ CKSEQ.ABJ >NUL
@IF ERRORLEVEL 1 GOTO DERROR
@CLS
@ECHO.
@ECHO CKSEQ      OMAIV CCD Detector Controller 
@ECHO            Sequencer Diagnostic                             Version 2.20
@ECHO.         
@ECHO            This test requires the use of the OMAIV Digital Test Board.
@ECHO.
@ECHO          - Each test should take 1 second.  If no results are
@ECHO            displayed within this time,  the result is negative.
@ECHO.
@ECHO          - Press any key to continue or ESC to return to DOS.
@ECHO.
@PORTW %1 1%PORTVAL% %MEMVAL% 
@PORTW %1 3%PORTVAL% %MEMVAL% 
@REM -----------------------------------------------------------
@REM    WAIT FOR THE USER TO TERMINATE TEST
@REM -----------------------------------------------------------
:SEQLOOP
@ckint -P%1 -Q
@if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L35
@ckkey
@if not errorlevel 1 goto SEQLOOP
@if errorlevel 27 if not errorlevel 28 goto DONE
@PORTW %1 0 %MEMVAL%
@REM -----------------------------------------------------------
@REM    Do Trigger In test
@REM -----------------------------------------------------------
@SET ERR=.
@LOADER -A%ADDRVAL% -P%1 -S2 -DCKTRIG.DBJ CKTRIG.ABJ >NUL
@IF ERRORLEVEL 1 GOTO DERROR
@CLS
@ECHO.
@ECHO CKTRIG     OMAIV CCD Detector Controller 
@ECHO            Input Trigger Diagnostic                       Version 2.20
@ECHO.         
@ECHO            This test requires the use of the OMAIV Digital Test Board.
@ECHO.
@ECHO          - Each test should take 1 second.  If no results are
@ECHO            displayed within this time,  the result is negative.
@ECHO.
@ECHO          - Press any key to continue or ESC to return to DOS.
@ECHO.
@PORTW %1 1%PORTVAL% %MEMVAL% 
@PORTW %1 3%PORTVAL% %MEMVAL% 
@REM -----------------------------------------------------------
@REM    WAIT FOR THE USER TO TERMINATE TEST
@REM -----------------------------------------------------------
:TRILOOP
@ckint -P%1 -Q
@if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L35
@ckkey
@if not errorlevel 1 goto TRILOOP
@if errorlevel 27 if not errorlevel 28 goto DONE
@PORTW %1 0 %MEMVAL%
@REM -----------------------------------------------------------
@REM    Do Sync test
@REM -----------------------------------------------------------
@SET ERR=.
@LOADER -A%ADDRVAL% -P%1 -S2 -DCKSYNC.DBJ CKSYNC.ABJ >NUL
@IF ERRORLEVEL 1 GOTO DERROR
@CLS
@ECHO.
@ECHO CKSYNC     OMAIV CCD Detector Controller 
@ECHO            Sync. Input Diagnostic                           Version 2.20
@ECHO.         
@ECHO            This test requires the use of the OMAIV Digital Test Board.
@ECHO.         
@ECHO          - Depress the SYNC button on the Digital Test Board.  A
@ECHO            Test Passed message should be displayed.  If no display
@ECHO            or a repeating Test Passed message is observed,  the result 
@ECHO            is negative.                                        
@ECHO.
@ECHO          - Press any key to restart this test or ESC to return to DOS.
@ECHO.
@PORTW %1 1%PORTVAL% %MEMVAL% 
@PORTW %1 3%PORTVAL% %MEMVAL% 
@REM -----------------------------------------------------------
@REM    WAIT FOR THE USER TO TERMINATE TEST
@REM -----------------------------------------------------------
:SYNLOOP
@ckint -P%1 -Q
@if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L35
@ckkey
@if not errorlevel 1 goto SYNLOOP
@if errorlevel 27 if not errorlevel 28 goto DONE
@GOTO BEGIN
@REM -----------------------------------------------------------
@REM    THERE WAS NOT ENOUGH SYSTEM MEMORY
@REM -----------------------------------------------------------
:MEMERROR
@ECHO          - Not enough open system memory available or
@ECHO            invalid interrupt level.
@GOTO DONE
@REM -----------------------------------------------------------
@REM    THERE WAS A DOWN LOAD ERROR        
@REM -----------------------------------------------------------
:DERROR   
@CLS
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO.
@ECHO          - An ERROR was detected in the loading process.
@GOTO DONE
@REM -----------------------------------------------------------
@REM    THERE WERE NO ARGUMENTS SPECIFIED ON THE COMMAND LINE
@REM -----------------------------------------------------------
:LISTARG
@ECHO    usage :  CKTDC     {Port address} {Interrupt level}
@ECHO.
@ECHO          - Port address must be three digits, in hexadecimal (300).
@ECHO          - Interrupt level must be 4,5 or 7. 
@GOTO END
@REM -----------------------------------------------------------
@REM    CLEAN UP BOARD CONFIGURATION FOR EXIT
@REM -----------------------------------------------------------
:DONE
@PORTW %1 0 0
:END
