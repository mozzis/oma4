@echo off
REM ***********************************************************
REM *  THIS WILL TEST THE OMA CONTROLLER AND DETECTOR         *
REM *  CONTROLLER.                              VER 2.20      *
REM ***********************************************************
BREAK ON
CS
ECHO.
ECHO LOOPBACK   OMAIV Controller Loopback Diagnostic         Version 2.20
ECHO.
REM -----------------------------------------------------------
REM    CHECKING FOR ARGUMENTS
REM -----------------------------------------------------------
IF "%1" == "" GOTO LISTARG
IF %2 == 4 GOTO CONTINUE
IF %2 == 5 GOTO CONTINUE
IF %2 == 7 GOTO CONTINUE
GOTO MEMERROR
:CONTINUE
PORTW %1 0 0
SET MEMVAL=FF
REM -----------------------------------------------------------
REM    SIZING SYSTEM MEMORY
REM -----------------------------------------------------------
GOTO KLUDGE
MEMTEST 200000 -C2 -B -Q
IF ERRORLEVEL 1 GOTO BLOCK2
MEMTEST 400000 -C2 -B -Q
IF ERRORLEVEL 1 GOTO BLOCK4
MEMTEST 600000 -C2 -B -Q
IF ERRORLEVEL 1 GOTO BLOCK6
:KLUDGE
MEMTEST 800000 -C2 -B -Q
IF ERRORLEVEL 1 GOTO BLOCK8
MEMTEST A00000 -C2 -B -Q
IF ERRORLEVEL 1 GOTO BLOCKA
MEMTEST C00000 -C2 -B -Q
IF ERRORLEVEL 1 GOTO BLOCKC
GOTO BLOCKF
REM -----------------------------------------------------------
REM    2MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCK2
SET MEMVAL=03
SET ADDRVAL=200000
GOTO DOTESTS
REM -----------------------------------------------------------
REM    4MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCK4
SET MEMVAL=05
SET ADDRVAL=400000
GOTO DOTESTS
REM -----------------------------------------------------------
REM    6MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCK6
SET MEMVAL=09
SET ADDRVAL=600000
GOTO DOTESTS
REM -----------------------------------------------------------
REM    8MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCK8
SET MEMVAL=11
SET ADDRVAL=800000
GOTO DOTESTS
REM -----------------------------------------------------------
REM   10MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCKA
SET MEMVAL=21
SET ADDRVAL=A00000
GOTO DOTESTS
REM -----------------------------------------------------------
REM   12MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCKC
SET MEMVAL=41
SET ADDRVAL=C00000
GOTO DOTESTS
REM -----------------------------------------------------------
REM    DO LOOPBACK TESTS
REM -----------------------------------------------------------
:DOTESTS
IF %MEMVAL% == FF GOTO MEMERROR
PORTW %1 0 %MEMVAL%
LOADER -A%ADDRVAL% -P%1 -ED >NUL 
LOADER -A%ADDRVAL% -P%1 -S2 -MMONITOR.HEX -DLOOPBACK.DBJ LOOPBACK.ABJ >NUL
IF ERRORLEVEL 1 GOTO DERROR
IF %2 == 4 SET PORTVAL=9
IF %2 == 5 SET PORTVAL=9
IF %2 == 7 SET PORTVAL=9
ECHO          - Each test should take 3 seconds.  If no results are
ECHO            displayed within this time,  the result is negative.
ECHO            Press any key to return to DOS.                
ECHO.
PORTW %1 1%PORTVAL% %MEMVAL% 
PORTW %1 3%PORTVAL% %MEMVAL% 
REM -----------------------------------------------------------
REM    WAIT FOR THE USER TO TERMINATE TEST
REM -----------------------------------------------------------
:DACLOOP
ckint -P%1 -Q
if errorlevel 1 ckmsg -A%ADDRVAL% -P%1 -L35
ckkey
if not errorlevel 1 goto DACLOOP
GOTO DONE
REM -----------------------------------------------------------
REM    THERE WAS A DOWN LOAD ERROR        
REM -----------------------------------------------------------
:DERROR   
CLS
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
ECHO          - An ERROR was detected in the loading process.
GOTO DONE
REM -----------------------------------------------------------
REM    THERE WAS NOT ENOUGH SYSTEM MEMORY
REM -----------------------------------------------------------
:MEMERROR
ECHO          - Not enough open system memory available or
ECHO            invalid interrupt level.
GOTO DONE
REM -----------------------------------------------------------
REM    THERE WERE NO ARGUMENTS SPECIFIED ON THE COMMAND LINE
REM -----------------------------------------------------------
:LISTARG
ECHO    usage :  LOOPBACK  {Port address} {Interrupt level}
ECHO.
ECHO          - Port address must be three digits, in hexadecimal (300).
ECHO          - Interrupt level must be 4,5 or 7. 
GOTO END
REM -----------------------------------------------------------
REM    CLEAN UP BOARD CONFIGURATION FOR EXIT
REM -----------------------------------------------------------
:DONE
PORTW %1 0 0
:END
