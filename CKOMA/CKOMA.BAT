@ECHO OFF
REM ***********************************************************
REM *  THIS WILL TEST THE OMA CONTROLLER BOARD.               *
REM *                                           VER 2.20      *
REM ***********************************************************
BREAK ON
:BEGIN
CS
ECHO.
ECHO CKOMA      OMAIV OMA Controller Diagnostic            Version 2.30
ECHO.
ECHO            This diagnostic will run several tests.  Most 
ECHO            tests will repeat until a key is depressed.  The
ECHO            tests are as follows:
ECHO.
ECHO          - CKMEM    This will verify access to memory.
ECHO          - CKDAC    This will check the Data Acquisition Controller.
ECHO          - LOOPBACK This will check the Loopback data path.
ECHO.
REM -----------------------------------------------------------
REM    CHECKING FOR ARGUMENTS
REM -----------------------------------------------------------
IF "%1" == "" GOTO LISTARG
IF %2 == 4 GOTO CONTINUE
IF %2 == 5 GOTO CONTINUE
IF %2 == 7 GOTO CONTINUE
IF "%4"== "" GOTO CONTINUE
IF "%4"== "-Q" GOTO CONTINUE
IF "%4"== "-q" GOTO CONTINUE
GOTO MEMERROR
:CONTINUE
ECHO          - Press any key to continue or ESC to return to DOS.
ECHO.
EPAUSE
IF ERRORLEVEL 1 GOTO DONE
PORTW %1 0 0
SET MEMVAL=FF
REM -----------------------------------------------------------
REM    SIZING SYSTEM MEMORY
REM -----------------------------------------------------------
CS
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO          - Sizing your systems memory ...
SET UP=
IF "%5" == "EM" GOTO BLOCKE
IF "%5" == "CM" GOTO BLOCKC
IF "%5" == "AM" GOTO BLOCKA
IF "%5" == "8M" GOTO BLOCK8
IF "%5" == "6M" GOTO BLOCK6
IF "%5" == "4M" GOTO BLOCK4
IF "%5" == "2M" GOTO BLOCK2
OMA4LOC > NUL
if errorlevel 6 if not errorlevel 7 GOTO BLOCKC
if errorlevel 5 if not errorlevel 6 GOTO BLOCKA
if errorlevel 4 if not errorlevel 5 GOTO BLOCK8
if errorlevel 3 if not errorlevel 4 GOTO BLOCK6
if errorlevel 2 if not errorlevel 3 GOTO BLOCK4
if errorlevel 1 if not errorlevel 2 GOTO BLOCK2
GOTO BLOCKF
REM -----------------------------------------------------------
REM    2MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCK2
IF %3 == .5 SET MEMVAL=03
IF %3 == 2 SET MEMVAL=03
IF %3 == 4 SET MEMVAL=07
IF %3 == 6 SET MEMVAL=0F
IF %3 == 8 SET MEMVAL=1F
SET ADDRVAL=200000
GOTO DOTESTS
REM -----------------------------------------------------------
REM    4MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCK4
IF %3 == .5 SET MEMVAL=05
IF %3 == 2 SET MEMVAL=05
IF %3 == 4 SET MEMVAL=0D
IF %3 == 6 SET MEMVAL=1D
IF %3 == 8 SET MEMVAL=3D
SET ADDRVAL=400000
GOTO DOTESTS
REM -----------------------------------------------------------
REM    6MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCK6
IF %3 == .5 SET MEMVAL=09
IF %3 == 2 SET MEMVAL=09
IF %3 == 4 SET MEMVAL=19
IF %3 == 6 SET MEMVAL=39
IF %3 == 8 SET MEMVAL=79
SET ADDRVAL=600000
GOTO DOTESTS
REM -----------------------------------------------------------
REM    8MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCK8
IF %3 == .5 SET MEMVAL=11
IF %3 == 2 SET MEMVAL=11
IF %3 == 4 SET MEMVAL=31
IF %3 == 6 SET MEMVAL=71
IF %3 == 8 SET MEMVAL=F1
SET ADDRVAL=800000
GOTO DOTESTS
REM -----------------------------------------------------------
REM   10MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCKA
IF %3 == .5 SET MEMVAL=21
IF %3 == 2 SET MEMVAL=21
IF %3 == 4 SET MEMVAL=61
IF %3 == 6 SET MEMVAL=E0
IF %3 == 8 SET MEMVAL=FF 
SET ADDRVAL=A00000
GOTO DOTESTS
REM -----------------------------------------------------------
REM   12MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCKC
IF %3 == .5 SET MEMVAL=41
IF %3 == 2 SET MEMVAL=41
IF %3 == 4 SET MEMVAL=C1
IF %3 == 6 SET MEMVAL=FF 
IF %3 == 8 SET MEMVAL=FF 
SET ADDRVAL=C00000
GOTO DOTESTS
REM -----------------------------------------------------------
REM   14MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCKE
SET UP=-U
IF %3 == .5 SET MEMVAL=41
IF %3 == 2 SET MEMVAL=41
IF %3 == 4 SET MEMVAL=C1
IF %3 == 6 SET MEMVAL=FF 
IF %3 == 8 SET MEMVAL=FF 
SET ADDRVAL=E00000
GOTO DOTESTS
REM -----------------------------------------------------------
REM    NO MEMORY OPEN FOR USE
REM -----------------------------------------------------------
:BLOCKF
SET MEMVAL=FF
GOTO DOTESTS
REM -----------------------------------------------------------
REM    DO TESTS
REM -----------------------------------------------------------
:DOTESTS
ECHO %MEMVAL% > MEMVAL.TXT
ECHO  %ADDRVAL% > ADDRVAL.TXT
IF %MEMVAL% == FF GOTO MEMERROR
REM -----------------------------------------------------------
REM    Do Ckeck Memory test
REM -----------------------------------------------------------
:CKMEM
CS
ECHO.
ECHO CKMEM  OMAIV Controller Memory Diagnostic             Version 2.2
ECHO.
ECHO.
REM -- DO ASIC PROGRAM MEMORY TEST ----------------------------
PORTW %1 0 %MEMVAL%
IF %MEMVAL% == 03 MEMTEST 200000 -C7FFF %4 
IF %MEMVAL% == 07 MEMTEST 200000 -C7FFF %4 
IF %MEMVAL% == 0F MEMTEST 200000 -C7FFF %4 
IF %MEMVAL% == 1F MEMTEST 200000 -C7FFF %4 
IF %MEMVAL% == 05 MEMTEST 400000 -C7FFF %4 
IF %MEMVAL% == 0D MEMTEST 400000 -C7FFF %4 
IF %MEMVAL% == 1D MEMTEST 400000 -C7FFF %4 
IF %MEMVAL% == 3D MEMTEST 400000 -C7FFF %4 
IF %MEMVAL% == 09 MEMTEST 600000 -C7FFF %4 
IF %MEMVAL% == 19 MEMTEST 600000 -C7FFF %4 
IF %MEMVAL% == 39 MEMTEST 600000 -C7FFF %4 
IF %MEMVAL% == 79 MEMTEST 600000 -C7FFF %4 
IF %MEMVAL% == 11 MEMTEST 800000 -C7FFF %4 
IF %MEMVAL% == 31 MEMTEST 800000 -C7FFF %4 
IF %MEMVAL% == 71 MEMTEST 800000 -C7FFF %4 
IF %MEMVAL% == 21 MEMTEST A00000 -C7FFF %4 
IF %MEMVAL% == 61 MEMTEST A00000 -C7FFF %4 
IF %MEMVAL% == 41 MEMTEST C00000 -C7FFF %4 
IF ERRORLEVEL 1 ECHO          - ASIC program memory test: FAILED
IF NOT ERRORLEVEL 1 ECHO          - ASIC program memory test: PASSED
REM -- DO DAC PROGRAM MEMORY TEST -----------------------------
IF %MEMVAL% == 03 MEMTEST 210000 -C7FFF %4 
IF %MEMVAL% == 07 MEMTEST 210000 -C7FFF %4 
IF %MEMVAL% == 0F MEMTEST 210000 -C7FFF %4 
IF %MEMVAL% == 1F MEMTEST 210000 -C7FFF %4 
IF %MEMVAL% == 05 MEMTEST 410000 -C7FFF %4 
IF %MEMVAL% == 0D MEMTEST 410000 -C7FFF %4 
IF %MEMVAL% == 1D MEMTEST 410000 -C7FFF %4 
IF %MEMVAL% == 3D MEMTEST 410000 -C7FFF %4 
IF %MEMVAL% == 09 MEMTEST 610000 -C7FFF %4 
IF %MEMVAL% == 19 MEMTEST 610000 -C7FFF %4 
IF %MEMVAL% == 39 MEMTEST 610000 -C7FFF %4 
IF %MEMVAL% == 79 MEMTEST 610000 -C7FFF %4 
IF %MEMVAL% == 11 MEMTEST 810000 -C7FFF %4 
IF %MEMVAL% == 31 MEMTEST 810000 -C7FFF %4 
IF %MEMVAL% == 71 MEMTEST 810000 -C7FFF %4 
IF %MEMVAL% == 21 MEMTEST A10000 -C7FFF %4 
IF %MEMVAL% == 61 MEMTEST A10000 -C7FFF %4 
IF %MEMVAL% == 41 MEMTEST C10000 -C7FFF %4 
IF ERRORLEVEL 1 ECHO          - DAC program memory test:  FAILED
IF NOT ERRORLEVEL 1 ECHO          - DAC program memory test:  PASSED
REM -- DO DATA MEMORY TEST ------------------------------------
PORTW %1 11 %MEMVAL%
IF %3 == .5 GOTO HMEGTEST
IF %3 == 2 GOTO 2MEGTEST
IF %3 == 4 GOTO 4MEGTEST
IF %3 == 6 GOTO 6MEGTEST
IF %3 == 8 GOTO 8MEGTEST
GOTO MEMERROR
:HMEGTEST
IF %MEMVAL% == 03 MEMTEST 200000 -C080000 -B %4 
IF %MEMVAL% == 05 MEMTEST 400000 -C080000 -B %4 
IF %MEMVAL% == 09 MEMTEST 600000 -C080000 -B %4 
IF %MEMVAL% == 11 MEMTEST 800000 -C080000 -B %4 
IF %MEMVAL% == 21 MEMTEST A00000 -C080000 -B %4 
IF %MEMVAL% == 41 MEMTEST C00000 -C080000 -B %4 
GOTO TSTRESULT
:2MEGTEST
IF %MEMVAL% == 03 MEMTEST 200000 -C200000 -B %4 
IF %MEMVAL% == 05 MEMTEST 400000 -C200000 -B %4 
IF %MEMVAL% == 09 MEMTEST 600000 -C200000 -B %4 
IF %MEMVAL% == 11 MEMTEST 800000 -C200000 -B %4 
IF %MEMVAL% == 21 MEMTEST A00000 -C200000 -B %4 
IF %MEMVAL% == 41 MEMTEST C00000 -C200000 -B %4 
GOTO TSTRESULT
:4MEGTEST
IF %MEMVAL% == 07 MEMTEST 200000 -C400000 -B %4 
IF %MEMVAL% == 0D MEMTEST 400000 -C400000 -B %4 
IF %MEMVAL% == 19 MEMTEST 600000 -C400000 -B %4 
IF %MEMVAL% == 31 MEMTEST 800000 -C400000 -B %4 
IF %MEMVAL% == 61 MEMTEST A00000 -C400000 -B %4 
GOTO TSTRESULT
:6MEGTEST
IF %MEMVAL% == 0F MEMTEST 200000 -C600000 -B %4 
IF %MEMVAL% == 1D MEMTEST 400000 -C600000 -B %4 
IF %MEMVAL% == 39 MEMTEST 600000 -C600000 -B %4 
IF %MEMVAL% == 71 MEMTEST 800000 -C600000 -B %4 
GOTO TSTRESULT
:8MEGTEST
IF %MEMVAL% == 1F MEMTEST 200000 -C800000 -B %4 
IF %MEMVAL% == 3D MEMTEST 400000 -C800000 -B %4 
IF %MEMVAL% == 79 MEMTEST 600000 -C800000 -B %4 
:TSTRESULT
IF ERRORLEVEL 1 ECHO          - ASIC data memory test:    FAILED
IF NOT ERRORLEVEL 1 ECHO          - ASIC data memory test:    PASSED
ECHO.
ECHO          - Press any key to continue or ESC to return to DOS.
ECHO.
EPAUSE
IF ERRORLEVEL 1 GOTO DONE
PORTW %1 0 %MEMVAL%
REM -----------------------------------------------------------
REM    Loading DAC Test
REM -----------------------------------------------------------
CS
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO.
ECHO                - Loading the DAC Controller test...                               
IF %2 == 4 SET PORTVAL=9
IF %2 == 5 SET PORTVAL=9
IF %2 == 7 SET PORTVAL=9
LOADER -V -A%ADDRVAL% -P%1 -EP %UP% 
IF "%4" == "-Q" LOADER -V -A%ADDRVAL% -P%1 %UP% -S2 -Mmonitor.bin -DCKDACF.DBJ 
IF ERRORLEVEL 1 GOTO DERROR
IF "%4" == "-q" LOADER -V -A%ADDRVAL% -P%1 -S2 %UP% -Mmonitor.bin -DCKDACF.DBJ 
IF ERRORLEVEL 1 GOTO DERROR
IF "%4" == "" LOADER -V -A%ADDRVAL% -P%1 %UP% -S2 -Mmonitor.bin -DCKDAC.DBJ    
IF ERRORLEVEL 1 GOTO DERROR
ECHO          - Press any key to continue or ESC to return to DOS.
:dbgloop
ckkey
if not errorlevel 1 goto dbgloop
if errorlevel 27 if not errorlevel 28 goto DONE
REM -----------------------------------------------------------
REM    Do Check Data Acquisition test
REM -----------------------------------------------------------
CS
ECHO.
ECHO CKDAC  OMAIV Data Acquisition Controller Diagnostic    Version 2.20
ECHO.
SET TIMEVAL=4
IF "%4" == "" SET TIMEVAL=120
ECHO          - Each test should take %TIMEVAL% seconds.  If no results are
ECHO            displayed within this time,  the result is negative.
ECHO.
ECHO          - Press any key to continue or ESC to return to DOS.
ECHO.
PORTW %1 1%PORTVAL% %MEMVAL% 
PORTW %1 3%PORTVAL% %MEMVAL% 
REM -----------------------------------------------------------
REM    WAIT FOR THE USER TO TERMINATE TEST
REM -----------------------------------------------------------
:DACLOOP
ckint -P%1 -Q
if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L35 "         - DAC Memory Test: PASSED"
ckkey
if not errorlevel 1 goto DACLOOP
if errorlevel 27 if not errorlevel 28 goto DONE
PORTW %1 0 %MEMVAL%
REM -----------------------------------------------------------
REM    Do Loopback test
REM -----------------------------------------------------------
LOADER -V -A%ADDRVAL% -P%1 -S2 %UP% -DLOOPBACK.DBJ LOOPBACK.ABJ 
IF ERRORLEVEL 1 GOTO DERROR
CS
ECHO.
ECHO LOOPBACK   OMAIV Controller Loopback Diagnostic         Version 2.20
ECHO.
ECHO          - Each test should take 3 seconds.  If no results are
ECHO            displayed within this time,  the result is negative.
ECHO.
ECHO          - Press any key to continue or ESC to return to DOS.
ECHO.
PORTW %1 1%PORTVAL% %MEMVAL% 
PORTW %1 3%PORTVAL% %MEMVAL% 
REM -----------------------------------------------------------
REM    WAIT FOR THE USER TO TERMINATE TEST
REM -----------------------------------------------------------
:LPBLOOP
ckint -P%1 -Q
if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L31 "         - Loopback Test: PASSED"
ckkey
if not errorlevel 1 goto LPBLOOP
if errorlevel 27 if not errorlevel 28 goto DONE
PORTW %1 0 %MEMVAL%
GOTO BEGIN
REM -----------------------------------------------------------
REM    THERE WAS NOT ENOUGH SYSTEM MEMORY
REM -----------------------------------------------------------
:MEMERROR
ECHO          - Not enough open system memory available or
ECHO            invalid argument was specified.
GOTO DONE
REM -----------------------------------------------------------
REM    THERE WAS A DOWN LOAD ERROR        
REM -----------------------------------------------------------
:DERROR   
@rem CS
@rem ECHO.
@rem ECHO.
@rem ECHO.
@rem ECHO.
@rem ECHO.
@rem ECHO.
@rem ECHO.
@rem ECHO.
@rem ECHO.
@rem ECHO.
@rem ECHO.
@ECHO.
ECHO          - An ERROR was detected in the loading process.
echo . >>errs
TYPE ERRS
GOTO DONE
REM -----------------------------------------------------------
REM    THERE WERE NO ARGUMENTS SPECIFIED ON THE COMMAND LINE
REM -----------------------------------------------------------
:LISTARG
ECHO    usage : CKOMA {Port address} {Interrupt level} {Memory size} [-Q]
ECHO.
ECHO          - Port address must be three digits, in hexadecimal (300).
ECHO          - Interrupt level must be 4,5 or 7. 
ECHO          - Memory size must be a single digit representing the
ECHO            amount of data memory installed, in megabytes (2).
ECHO            Use (.5) for the 512Kb option.
ECHO          - Q option for the quick algorithm.
GOTO END
REM -----------------------------------------------------------
REM    CLEAN UP BOARD CONFIGURATION FOR EXIT
REM -----------------------------------------------------------
:DONE
ECHO This test used a memory address of %ADDRVAL%
ECHO And a memory size of %3 Megabytes
rem PORTW %1 0 0
IF EXIST ERRS DEL ERRS >NUL
:END
