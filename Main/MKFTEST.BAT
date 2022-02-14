@echo off
set DEBUG_LIST=
:loop
if "%1"=="" goto end
nlfecho %1
del ..\obj\filetest\%1.obj
set DEBUG_LIST=%DEBUG_LIST% %1
shift
goto loop
:end
type \util\cr
tm /l /log >errs
nmake -f filetest.mak DEBUG="" %1 %2 %3 %4 %5 %6 %7 %8 %9
if errorlevel 1 be beep

