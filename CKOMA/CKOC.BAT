@REM ***********************************************************
@REM *  THIS WILL TEST THE OMA CONTROLLER BOARD.               *
@REM *                                           VER 2.2      *
@REM ***********************************************************
@BREAK ON
:BEGIN
@CLS
@ECHO.
@ECHO CKOC       OMAIV OMA Controller Diagnostic            Version 2.2
@ECHO.
@ECHO            This diagnostic will run several tests.  Most 
@ECHO            tests will repeat until a key is depressed.  The
@ECHO            tests are as follows:
@ECHO.
@ECHO          - CKMEM    This will verify access to memory.
@ECHO          - CKDAC    This will check the operation of the 68000uP.
@ECHO          - LOOPBACK This will check the Loopback data path.
@ECHO          - CKAD     This will check the normal data path. 
@ECHO          - CKTEMPL  This will verify the Temperature interrupt.
@ECHO          - CKTRIG   This will check the Trigger Input.
@ECHO.
@REM -----------------------------------------------------------
@REM    CHECKING FOR ARGUMENTS
@REM -----------------------------------------------------------
@IF "%1" == "" GOTO LISTARG
@IF %2 == 4 GOTO CONTINUE
@IF %2 == 5 GOTO CONTINUE
@IF %2 == 7 GOTO CONTINUE
@IF "%4"== "" GOTO CONTINUE
@IF "%4"== "-Q" GOTO CONTINUE
@IF "%4"== "-q" GOTO CONTINUE
@GOTO MEMERROR
:CONTINUE
@ECHO          - Press any key to continue or ESC to return to DOS.
@ECHO.
@EPAUSE
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
@ECHO          - Sizing your systems memory ...                                   
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
@IF %3 == .5 SET MEMVAL=83
@IF %3 == 2 SET MEMVAL=83
@IF %3 == 4 SET MEMVAL=87
@IF %3 == 6 SET MEMVAL=8F
@IF %3 == 8 SET MEMVAL=9F
@SET ADDRVAL=200000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM    4MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCK4
@IF %3 == .5 SET MEMVAL=85
@IF %3 == 2 SET MEMVAL=85
@IF %3 == 4 SET MEMVAL=8D
@IF %3 == 6 SET MEMVAL=9D
@IF %3 == 8 SET MEMVAL=BD
@SET ADDRVAL=400000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM    6MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCK6
@IF %3 == .5 SET MEMVAL=89
@IF %3 == 2 SET MEMVAL=89
@IF %3 == 4 SET MEMVAL=99
@IF %3 == 6 SET MEMVAL=B9
@IF %3 == 8 SET MEMVAL=F9
@SET ADDRVAL=600000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM    8MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCK8
@IF %3 == .5 SET MEMVAL=91
@IF %3 == 2 SET MEMVAL=91
@IF %3 == 4 SET MEMVAL=B1
@IF %3 == 6 SET MEMVAL=F1
@IF %3 == 8 SET MEMVAL=FF
@SET ADDRVAL=800000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM   10MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCKA
@IF %3 == .5 SET MEMVAL=A1
@IF %3 == 2 SET MEMVAL=A1
@IF %3 == 4 SET MEMVAL=E1
@IF %3 == 6 SET MEMVAL=FF
@IF %3 == 8 SET MEMVAL=FF 
@SET ADDRVAL=A00000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM   12MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCKC
@IF %3 == .5 SET MEMVAL=C1
@IF %3 == 2 SET MEMVAL=C1
@IF %3 == 4 SET MEMVAL=FF
@IF %3 == 6 SET MEMVAL=FF 
@IF %3 == 8 SET MEMVAL=FF 
@SET ADDRVAL=C00000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM    NO MEMORY OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCKF
@SET MEMVAL=FF
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM    DO TESTS
@REM -----------------------------------------------------------
:DOTESTS
@IF %MEMVAL% == FF GOTO MEMERROR
@REM -----------------------------------------------------------
@REM    Do Ckeck Memory test
@REM -----------------------------------------------------------
:CKMEM
@CLS
@ECHO.
@ECHO CKMEM  OMAIV Controller Memory Diagnostic             Version 2.20
@ECHO.
@ECHO.
@REM -- DO ASIC PROGRAM MEMORY TEST ----------------------------
@PORTW %1 0 %MEMVAL%
@IF %MEMVAL% == 83 MEMTEST 200000 -C7FFF %4 
@IF %MEMVAL% == 87 MEMTEST 200000 -C7FFF %4 
@IF %MEMVAL% == 8F MEMTEST 200000 -C7FFF %4 
@IF %MEMVAL% == 9F MEMTEST 200000 -C7FFF %4 
@IF %MEMVAL% == 85 MEMTEST 400000 -C7FFF %4 
@IF %MEMVAL% == 8D MEMTEST 400000 -C7FFF %4 
@IF %MEMVAL% == 9D MEMTEST 400000 -C7FFF %4 
@IF %MEMVAL% == BD MEMTEST 400000 -C7FFF %4 
@IF %MEMVAL% == 89 MEMTEST 600000 -C7FFF %4 
@IF %MEMVAL% == 99 MEMTEST 600000 -C7FFF %4 
@IF %MEMVAL% == B9 MEMTEST 600000 -C7FFF %4 
@IF %MEMVAL% == F9 MEMTEST 600000 -C7FFF %4 
@IF %MEMVAL% == 91 MEMTEST 800000 -C7FFF %4 
@IF %MEMVAL% == B1 MEMTEST 800000 -C7FFF %4 
@IF %MEMVAL% == F1 MEMTEST 800000 -C7FFF %4 
@IF %MEMVAL% == A1 MEMTEST A00000 -C7FFF %4 
@IF %MEMVAL% == E1 MEMTEST A00000 -C7FFF %4 
@IF %MEMVAL% == C1 MEMTEST C00000 -C7FFF %4 
@IF ERRORLEVEL 1 ECHO          - ASIC program memory test: FAILED
@IF NOT ERRORLEVEL 1 ECHO          - ASIC program memory test: PASSED
@REM -- DO DAC PROGRAM MEMORY TEST -----------------------------
@IF %MEMVAL% == 83 MEMTEST 210000 -C7FFF %4 
@IF %MEMVAL% == 87 MEMTEST 210000 -C7FFF %4 
@IF %MEMVAL% == 8F MEMTEST 210000 -C7FFF %4 
@IF %MEMVAL% == 9F MEMTEST 210000 -C7FFF %4 
@IF %MEMVAL% == 85 MEMTEST 410000 -C7FFF %4 
@IF %MEMVAL% == 8D MEMTEST 410000 -C7FFF %4 
@IF %MEMVAL% == 9D MEMTEST 410000 -C7FFF %4 
@IF %MEMVAL% == BD MEMTEST 410000 -C7FFF %4 
@IF %MEMVAL% == 89 MEMTEST 610000 -C7FFF %4 
@IF %MEMVAL% == 99 MEMTEST 610000 -C7FFF %4 
@IF %MEMVAL% == B9 MEMTEST 610000 -C7FFF %4 
@IF %MEMVAL% == F9 MEMTEST 610000 -C7FFF %4 
@IF %MEMVAL% == 91 MEMTEST 810000 -C7FFF %4 
@IF %MEMVAL% == B1 MEMTEST 810000 -C7FFF %4 
@IF %MEMVAL% == F1 MEMTEST 810000 -C7FFF %4 
@IF %MEMVAL% == A1 MEMTEST A10000 -C7FFF %4 
@IF %MEMVAL% == E1 MEMTEST A10000 -C7FFF %4 
@IF %MEMVAL% == C1 MEMTEST C10000 -C7FFF %4 
@IF ERRORLEVEL 1 ECHO          - DAC program memory test:  FAILED
@IF NOT ERRORLEVEL 1 ECHO          - DAC program memory test:  PASSED
@REM -- DO DATA MEMORY TEST ------------------------------------
@PORTW %1 11 %MEMVAL%
@IF %3 == .5 GOTO HMEGTEST
@IF %3 == 2 GOTO 2MEGTEST
@IF %3 == 4 GOTO 4MEGTEST
@IF %3 == 6 GOTO 6MEGTEST
@IF %3 == 8 GOTO 8MEGTEST
@GOTO MEMERROR
:HMEGTEST
@IF %MEMVAL% == 83 MEMTEST 200000 -C080000 -B %4 
@IF %MEMVAL% == 85 MEMTEST 400000 -C080000 -B %4 
@IF %MEMVAL% == 89 MEMTEST 600000 -C080000 -B %4 
@IF %MEMVAL% == 91 MEMTEST 800000 -C080000 -B %4 
@IF %MEMVAL% == A1 MEMTEST A00000 -C080000 -B %4 
@IF %MEMVAL% == C1 MEMTEST C00000 -C080000 -B %4 
@GOTO TSTRESULT
:2MEGTEST
@IF %MEMVAL% == 83 MEMTEST 200000 -C200000 -B %4 
@IF %MEMVAL% == 85 MEMTEST 400000 -C200000 -B %4 
@IF %MEMVAL% == 89 MEMTEST 600000 -C200000 -B %4 
@IF %MEMVAL% == 91 MEMTEST 800000 -C200000 -B %4 
@IF %MEMVAL% == A1 MEMTEST A00000 -C200000 -B %4 
@IF %MEMVAL% == C1 MEMTEST C00000 -C200000 -B %4 
@GOTO TSTRESULT
:4MEGTEST
@IF %MEMVAL% == 87 MEMTEST 200000 -C400000 -B %4 
@IF %MEMVAL% == 8D MEMTEST 400000 -C400000 -B %4 
@IF %MEMVAL% == 99 MEMTEST 600000 -C400000 -B %4 
@IF %MEMVAL% == B1 MEMTEST 800000 -C400000 -B %4 
@IF %MEMVAL% == E1 MEMTEST A00000 -C400000 -B %4 
@GOTO TSTRESULT
:6MEGTEST
@IF %MEMVAL% == 8F MEMTEST 200000 -C600000 -B %4 
@IF %MEMVAL% == 9D MEMTEST 400000 -C600000 -B %4 
@IF %MEMVAL% == B9 MEMTEST 600000 -C600000 -B %4 
@IF %MEMVAL% == F1 MEMTEST 800000 -C600000 -B %4 
@GOTO TSTRESULT
:8MEGTEST
@IF %MEMVAL% == 9F MEMTEST 200000 -C800000 -B %4 
@IF %MEMVAL% == BD MEMTEST 400000 -C800000 -B %4 
@IF %MEMVAL% == F9 MEMTEST 600000 -C800000 -B %4 
:TSTRESULT
@IF ERRORLEVEL 1 ECHO          - ASIC data memory test:    FAILED
@IF NOT ERRORLEVEL 1 ECHO          - ASIC data memory test:    PASSED
@ECHO.
@ECHO          - Press any key to continue or ESC to return to DOS.
@ECHO.
@EPAUSE
@IF ERRORLEVEL 1 GOTO DONE
@PORTW %1 0 %MEMVAL%
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
@ECHO          - Loading the DAC test onto the OMA4 controller board ...          
@IF %2 == 4 SET PORTVAL=9
@IF %2 == 5 SET PORTVAL=9
@IF %2 == 7 SET PORTVAL=9
@LOADER -A%ADDRVAL% -P%1 -EP >NUL
@IF "%4" == "-Q" LOADER -A%ADDRVAL% -P%1 -S2 -MMONITOR.HEX -DCKDACF.DBJ >NUL    
@IF "%4" == "-q" LOADER -A%ADDRVAL% -P%1 -S2 -MMONITOR.HEX -DCKDACF.DBJ >NUL
@IF "%4" == "" LOADER -A%ADDRVAL% -P%1 -S2 -MMONITOR.HEX -DCKDAC.DBJ >NUL
@IF ERRORLEVEL 1 GOTO DERROR
@REM -----------------------------------------------------------
@REM    Do Check Data Acquisition test
@REM -----------------------------------------------------------
@CLS
@ECHO.
@ECHO CKDAC  OMAIV Data Acquisition Controller Diagnostic    Version 2.20
@ECHO.
@SET TIMEVAL=4
@IF "%4" == "" SET TIMEVAL=120
@ECHO          - Each test should take %TIMEVAL% seconds.  If no results are
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
@if not errorlevel 1 goto DACLOOP
@if errorlevel 27 if not errorlevel 28 goto DONE
@PORTW %1 0 %MEMVAL%
@REM -----------------------------------------------------------
@REM    Do Loopback test
@REM -----------------------------------------------------------
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
:LPBKLOOP
@ckint -P%1 -Q
@if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L32
@ckkey
@if not errorlevel 1  goto LPBKLOOP
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
@ECHO            Analog to Digital Converter Diagnostic           Version 2.2
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
:CKADLOOP
@ckint -P%1 -Q
@if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L32
@ckkey
@if not errorlevel 1  goto CKADLOOP
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
@ECHO            Temperature Locked Diagnostic                    Version 2.2
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
:CTMPLOOP
@ckint -P%1 -Q
@if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L32
@ckkey
@if not errorlevel 1  goto CTMPLOOP
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
@ECHO            Input Trigger Diagnostic                       Version 2.2
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
:LPBLOOP
@ckint -P%1 -Q
@if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L35
@ckkey
@if not errorlevel 1 goto LPBLOOP
@if errorlevel 27 if not errorlevel 28 goto DONE
@PORTW %1 0 %MEMVAL%
@GOTO BEGIN
@REM -----------------------------------------------------------
@REM    THERE WAS NOT ENOUGH SYSTEM MEMORY
@REM -----------------------------------------------------------
:MEMERROR
@ECHO          - Not enough open system memory available or
@ECHO            invalid argument was specified.
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
@ECHO    usage :  CKOC {Port address} {Interrupt level} {Memory size} [-Q]
@ECHO.
@ECHO          - Port address must be three digits, in hexadecimal (300).
@ECHO          - Interrupt level must be 4,5 or 7. 
@ECHO          - Memory size must be a single digit representing the
@ECHO            amount of data memory installed, in megabytes (2).
@ECHO            Use (.5) for the 512Kb option.
@ECHO          - Q option for the quick algorithm.
@GOTO END
@REM -----------------------------------------------------------
@REM    CLEAN UP BOARD CONFIGURATION FOR EXIT
@REM -----------------------------------------------------------
:DONE
@PORTW %1 0 0
@OMAMSG -i%2 -U >NUL
:END
