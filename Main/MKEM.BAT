@echo off
set DEBUG_LIST=
:loop
if "%1"=="" goto end
if "%1"=="-x" goto end
if "%1"=="-X" goto end
nlfecho %1
del ..\obj\dos\%1.obj
shift
goto loop
:end

if "%1"=="-x" goto DOS16
if "%1"=="-X" goto DOS16
rem -#4 causes execution time display upon completion
make -#4 -F empoke.mak %1 %2 %3 %4 %5 %6 %7 %8 %9
goto done
:DOS16
shift
make USE_D16M="Y" -#4 -F empoke.mak %1 %2 %3 %4 %5 %6 %7 %8 %9
:done
