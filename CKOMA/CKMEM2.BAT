@ECHO OFF
REM ***********************************************************
REM *  THIS PROGRAM SIZES THE SYSTEM MEMORY AND TESTS :       *
REM *          - ASIC PROGRAM MEMORIES.         VER 2.20      *       
REM *          - DAC PROGRAM MEMORY.            6/24/94       *
REM *          - DATA MEMORY.                                 *
REM ***********************************************************
CS
BREAK ON
ECHO.
ECHO CKMEM  OMAIV Controller Memory Diagnostic             Version 2.20
ECHO.
REM -----------------------------------------------------------
REM    CHECKING FOR ARGUMENTS
REM -----------------------------------------------------------
IF "%1" == "" GOTO LISTARG
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
MEMTEST 200000 -C2 -B -Q
IF ERRORLEVEL 1 GOTO BLOCK2
MEMTEST 400000 -C2 -B -Q
IF ERRORLEVEL 1 GOTO BLOCK4
MEMTEST 600000 -C2 -B -Q
IF ERRORLEVEL 1 GOTO BLOCK6
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
IF %2 == 2 SET MEMVAL=03
IF %2 == 4 SET MEMVAL=07
IF %2 == 6 SET MEMVAL=0F
IF %2 == 8 SET MEMVAL=1F
GOTO DOTESTS
REM -----------------------------------------------------------
REM    4MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCK4
IF %2 == 2 SET MEMVAL=05
IF %2 == 4 SET MEMVAL=0D
IF %2 == 6 SET MEMVAL=1D
IF %2 == 8 SET MEMVAL=3D
GOTO DOTESTS
REM -----------------------------------------------------------
REM    6MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCK6
IF %2 == 2 SET MEMVAL=09
IF %2 == 4 SET MEMVAL=19
IF %2 == 6 SET MEMVAL=39
IF %2 == 8 SET MEMVAL=79
GOTO DOTESTS
REM -----------------------------------------------------------
REM    8MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCK8
IF %2 == 2 SET MEMVAL=11
IF %2 == 4 SET MEMVAL=31
IF %2 == 6 SET MEMVAL=71
IF %2 == 8 SET MEMVAL=FF
GOTO DOTESTS
REM -----------------------------------------------------------
REM   10MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCKA
IF %2 == 2 SET MEMVAL=21
IF %2 == 4 SET MEMVAL=61
IF %2 == 6 SET MEMVAL=FF
IF %2 == 8 SET MEMVAL=FF 
GOTO DOTESTS
REM -----------------------------------------------------------
REM   12MB+ OPEN FOR USE
REM -----------------------------------------------------------
:BLOCKC
IF %2 == 2 SET MEMVAL=41
IF %2 == 4 SET MEMVAL=FF
IF %2 == 6 SET MEMVAL=FF 
IF %2 == 8 SET MEMVAL=FF 
GOTO DOTESTS
REM -----------------------------------------------------------
REM    NO MEMORY OPEN FOR USE
REM -----------------------------------------------------------
SET MEMVAL=FF
GOTO DOTESTS
REM -----------------------------------------------------------
REM    CHECK MEMVAL AND DO MEMORY TESTS
REM -----------------------------------------------------------
:DOTESTS
IF %MEMVAL% == FF GOTO MEMERROR
REM -- DO ASIC PROGRAM MEMORY TEST ----------------------------
PORTW %1 0 %MEMVAL%
IF %MEMVAL% == 03 MEMTEST 200000 -C7FFF %3 
IF %MEMVAL% == 07 MEMTEST 200000 -C7FFF %3 
IF %MEMVAL% == 0F MEMTEST 200000 -C7FFF %3 
IF %MEMVAL% == 1F MEMTEST 200000 -C7FFF %3 
IF %MEMVAL% == 05 MEMTEST 400000 -C7FFF %3 
IF %MEMVAL% == 0D MEMTEST 400000 -C7FFF %3 
IF %MEMVAL% == 1D MEMTEST 400000 -C7FFF %3 
IF %MEMVAL% == 3D MEMTEST 400000 -C7FFF %3 
IF %MEMVAL% == 09 MEMTEST 600000 -C7FFF %3 
IF %MEMVAL% == 19 MEMTEST 600000 -C7FFF %3 
IF %MEMVAL% == 39 MEMTEST 600000 -C7FFF %3 
IF %MEMVAL% == 79 MEMTEST 600000 -C7FFF %3 
IF %MEMVAL% == 11 MEMTEST 800000 -C7FFF %3 
IF %MEMVAL% == 31 MEMTEST 800000 -C7FFF %3 
IF %MEMVAL% == 71 MEMTEST 800000 -C7FFF %3 
IF %MEMVAL% == 21 MEMTEST A00000 -C7FFF %3 
IF %MEMVAL% == 61 MEMTEST A00000 -C7FFF %3 
IF %MEMVAL% == 41 MEMTEST C00000 -C7FFF %3 
IF ERRORLEVEL 1 ECHO          - ASIC program memory test: FAILED
IF NOT ERRORLEVEL 1 ECHO          - ASIC program memory test: PASSED
REM -- DO DAC PROGRAM MEMORY TEST -----------------------------
IF %MEMVAL% == 03 MEMTEST 210000 -C7FFF %3 
IF %MEMVAL% == 07 MEMTEST 210000 -C7FFF %3 
IF %MEMVAL% == 0F MEMTEST 210000 -C7FFF %3 
IF %MEMVAL% == 1F MEMTEST 210000 -C7FFF %3 
IF %MEMVAL% == 05 MEMTEST 410000 -C7FFF %3 
IF %MEMVAL% == 0D MEMTEST 410000 -C7FFF %3 
IF %MEMVAL% == 1D MEMTEST 410000 -C7FFF %3 
IF %MEMVAL% == 3D MEMTEST 410000 -C7FFF %3 
IF %MEMVAL% == 09 MEMTEST 610000 -C7FFF %3 
IF %MEMVAL% == 19 MEMTEST 610000 -C7FFF %3 
IF %MEMVAL% == 39 MEMTEST 610000 -C7FFF %3 
IF %MEMVAL% == 79 MEMTEST 610000 -C7FFF %3 
IF %MEMVAL% == 11 MEMTEST 810000 -C7FFF %3 
IF %MEMVAL% == 31 MEMTEST 810000 -C7FFF %3 
IF %MEMVAL% == 71 MEMTEST 810000 -C7FFF %3 
IF %MEMVAL% == 21 MEMTEST A10000 -C7FFF %3 
IF %MEMVAL% == 61 MEMTEST A10000 -C7FFF %3 
IF %MEMVAL% == 41 MEMTEST C10000 -C7FFF %3 
IF ERRORLEVEL 1 ECHO          - DAC program memory test:  FAILED
IF NOT ERRORLEVEL 1 ECHO          - DAC program memory test:  PASSED
REM -- DO DATA MEMORY TEST ------------------------------------
PORTW %1 11 %MEMVAL%
IF %MEMVAL% == 03 MEMTEST 200000 -C200000 -B %3 
IF %MEMVAL% == 07 MEMTEST 200000 -C400000 -B %3 
IF %MEMVAL% == 0F MEMTEST 200000 -C600000 -B %3 
IF %MEMVAL% == 1F MEMTEST 200000 -C800000 -B %3 
IF %MEMVAL% == 05 MEMTEST 400000 -C200000 -B %3 
IF %MEMVAL% == 0D MEMTEST 400000 -C400000 -B %3 
IF %MEMVAL% == 1D MEMTEST 400000 -C600000 -B %3 
IF %MEMVAL% == 3D MEMTEST 400000 -C800000 -B %3 
IF %MEMVAL% == 09 MEMTEST 600000 -C200000 -B %3 
IF %MEMVAL% == 19 MEMTEST 600000 -C400000 -B %3 
IF %MEMVAL% == 39 MEMTEST 600000 -C600000 -B %3 
IF %MEMVAL% == 79 MEMTEST 600000 -C800000 -B %3 
IF %MEMVAL% == 11 MEMTEST 800000 -C200000 -B %3 
IF %MEMVAL% == 31 MEMTEST 800000 -C400000 -B %3 
IF %MEMVAL% == 71 MEMTEST 800000 -C600000 -B %3 
IF %MEMVAL% == 21 MEMTEST A00000 -C200000 -B %3 
IF %MEMVAL% == 61 MEMTEST A00000 -C400000 -B %3 
IF %MEMVAL% == 41 MEMTEST C00000 -C200000 -B %3 
IF ERRORLEVEL 1 GOTO DONE
ECHO          - ASIC data memory test:    PASSED
GOTO DOTESTS
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
ECHO usage :  CKMEM  {Port address} {Memory size} [-Q]
ECHO.
ECHO          - Port address must be three digits, in hexadecimal (300).
ECHO          - Memory size must be a single digit representing the
ECHO            amount of data memory installed, in megabytes (2).
ECHO          - Q option for the quick algorithm.
GOTO END
REM -----------------------------------------------------------
REM    CLEAN UP BOARD CONFIGURATION FOR EXIT
REM -----------------------------------------------------------
:DONE
ECHO          - ASIC data memory test:    FAILED
PORTW %1 0 0
:END
