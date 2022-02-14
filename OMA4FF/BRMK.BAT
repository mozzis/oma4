@echo off
:loop
if "%1"=="" goto end
nlfecho %1
del ..\obj\tcdriv\%1.obj
shift
goto loop
:end
make -foma4ff.mak
rem if errorlevel 1 be beep
