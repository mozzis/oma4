@echo off
REM ***********************************************************
REM *  THIS WILL TEST THE DATA ACQUISITION CONTROLLER PORT    *
REM *  TO DATA MEMORY.                          VER 2.2      *
REM ***********************************************************
CS
BREAK ON
ECHO.
ECHO CKDAC  OMAIV Data Acquisition Controller Diagnostic    Version 2.2
ECHO.
REM -----------------------------------------------------------
REM    CHECKING FOR ARGUMENTS
REM -----------------------------------------------------------
SET UP=
IF "%1" == "" GOTO LISTARG
IF %2 == 4 GOTO CONTINUE
IF %2 == 5 GOTO CONTINUE
IF %2 == 7 GOTO CONTINUE
GOTO MEMERROR
:CONTINUE
IF "%3"== "" GOTO OK
IF "%3"== "-Q" GOTO OK
IF "%3"== "-q" GOTO OK
GOTO MEMERROR
:OK
PORTW %1 0 0
SET MEMVAL=FF
REM -----------------------------------------------------------
REM    SIZING SYSTEM MEMORY
REM -----------------------------------------------------------
IF "%4" == "EM" GOTO BLOCKE
IF "%4" == "CM" GOTO BLOCKC
IF "%4" == "AM" GOTO BLOCKA
IF "%4" == "8M" GOTO BLOCK8
IF "%4" == "6M" GOTO BLOCK6
IF "%4" == "4M" GOTO BLOCK4
IF "%4" == "2M" GOTO BLOCK2
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
REM   14MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCKE
SET UP=-U
SET MEMVAL=41
SET ADDRVAL=E00000
GOTO DOTESTS
REM -----------------------------------------------------------
REM    DO MEMORY TEST
REM -----------------------------------------------------------
:DOTESTS
IF %MEMVAL% == FF GOTO MEMERROR
PORTW %1 0 %MEMVAL%
LOADER -A%ADDRVAL% -P%1 %UP% -ED -EP >NUL
IF "%3" == "-Q" LOADER -A%ADDRVAL% -P%1 %UP% -S2 -MMONITOR.BIN -DDACMEMF.DBJ >NUL
IF "%3" == "-q" LOADER -A%ADDRVAL% -P%1 %UP% -S2 -MMONITOR.BIN -DDACMEMF.DBJ >NUL
IF "%3" == "" LOADER -A%ADDRVAL% -P%1 -S2 %UP% -MMONITOR.BIN -DDACMEM.DBJ >NUL
IF %2 == 4 SET PORTVAL=9
IF %2 == 5 SET PORTVAL=9
IF %2 == 7 SET PORTVAL=9
PORTW %1 C0 %MEMVAL% 
PORTW %1 1%PORTVAL% %MEMVAL% 
PORTW %1 3%PORTVAL% %MEMVAL% 
SET TIMEVAL=11
IF "%3" == "" SET TIMEVAL=360
ECHO          - Each test should take %TIMEVAL% seconds.  If no results are
ECHO            displayed within this time,  the result is negative.
ECHO            Press any key to return to DOS.                
ECHO.
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
REM    THERE WAS NOT ENOUGH SYSTEM MEMORY
REM -----------------------------------------------------------
:MEMERROR
ECHO          - Not enough open system memory available or
ECHO            an invalid argument was specified.
GOTO DONE
REM -----------------------------------------------------------
REM    THERE WERE NO ARGUMENTS SPECIFIED ON THE COMMAND LINE
REM -----------------------------------------------------------
:LISTARG
ECHO usage :  CKDAC  {Port address} {Interrupt level} [-Q]
ECHO.
ECHO          - Port address must be three digits, in hexadecimal (300).
ECHO          - Interrupt level must be 4,5 or 7. 
ECHO          - Q option for the quick algorithm.
GOTO END
REM -----------------------------------------------------------
REM    CLEAN UP BOARD CONFIGURATION FOR EXIT
REM -----------------------------------------------------------
:DONE
rem PORTW %1 0 0
:END
