@echo off
set DEBUG_LIST=
:loop
if "%1"=="" goto end
nlfecho %1
del ..\obj\dos\%1.obj
set DEBUG_LIST=%DEBUG_LIST% %1
shift
goto loop
:end
type \util\cr
rem -#4 causes execution time display upon completion
make DEBUG_LIST="%DEBUG_LIST%" -#4 -F empoke.mak %1 %2 %3 %4 %5 %6 %7 %8 %9
rem if errorlevel 1 be beep
