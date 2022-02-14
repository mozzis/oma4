@echo off
set DEBUG_LIST=
:loop
if "%1"=="" goto end
nlfecho %1
del ..\obj\dos\%1.obj
shift
goto loop
:end

rem -#4 causes execution time display upon completion
c:\tools\msc6\make MODE="RELEASE" -S -R
rem if errorlevel 1 be beep
