@echo off
set DEBUG_LIST=
:loop
if "%1"=="" goto end
nlfecho %1
if not exist ..\obj\dos\%1.obj echo ..\obj\dos\%1.obj not found
if exist ..\obj\dos\%1.obj del ..\obj\dos\%1.obj
set DEBUG_LIST=%DEBUG_LIST% %1
shift
goto loop
:end

rem -#4 causes execution time display upon completion
c:\tools\msc6\make MODE="DEBUG" DEBUG_LIST="%DEBUG_LIST%" -#4 -R
rem if errorlevel 1 be beep
