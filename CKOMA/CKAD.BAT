@REM ***********************************************************
@REM *  THIS WILL TEST THE ANALOG TO DIGITAL PORT.             *
@rem *                                          version 2.2   *
@REM ***********************************************************
@BREAK ON
@CLS
@ECHO.
@ECHO CKAD       OMAIV CCD Detector Controller 
@ECHO            Analog to Digital Converter Diagnostic           Version 2.2
@ECHO.         
@ECHO            This test requires the use of the OMAIV Digital Test Board.
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
@PORTW %1 0 0
@SET MEMVAL=FF
@REM -----------------------------------------------------------
@REM    SIZING SYSTEM MEMORY
@REM -----------------------------------------------------------
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
@SET MEMVAL=03
@SET ADDRVAL=200000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM    4MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCK4
@SET MEMVAL=05
@SET ADDRVAL=400000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM    6MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCK6
@SET MEMVAL=09
@SET ADDRVAL=600000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM    8MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCK8
@SET MEMVAL=11
@SET ADDRVAL=800000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM   10MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCKA
@SET MEMVAL=21
@SET ADDRVAL=A00000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM   12MB+ OPEN FOR USE
@REM -----------------------------------------------------------
:BLOCKC
@SET MEMVAL=41
@SET ADDRVAL=C00000
@GOTO DOTESTS
@REM -----------------------------------------------------------
@REM    DO LOOPBACK TESTS
@REM -----------------------------------------------------------
:DOTESTS
@IF %MEMVAL% == FF GOTO MEMERROR
@PORTW %1 0 %MEMVAL%
@LOADER -A%ADDRVAL% -P%1 -S2 -MMONITOR.HEX -DCKAD.DBJ CKAD.ABJ >NUL
@IF ERRORLEVEL 1 GOTO DERROR
@IF %2 == 4 SET PORTVAL=9
@IF %2 == 5 SET PORTVAL=9
@IF %2 == 7 SET PORTVAL=9
@ECHO          - Each test should take 1 second.  If no results are
@ECHO            displayed within this time,  the result is negative.
@ECHO            Press any key to return to DOS.                
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
@if errorlevel 1  goto DACNEXT
@goto DACLOOP
:DACNEXT
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
@REM    THERE WAS NOT ENOUGH SYSTEM MEMORY
@REM -----------------------------------------------------------
:MEMERROR
@ECHO          - Not enough open system memory available or
@ECHO            invalid interrupt level.
@GOTO DONE
@REM -----------------------------------------------------------
@REM    THERE WERE NO ARGUMENTS SPECIFIED ON THE COMMAND LINE
@REM -----------------------------------------------------------
:LISTARG
@ECHO    usage :  CKAD   {Port address} {Interrupt level}
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
