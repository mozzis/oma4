@echo off
if "%CTOOLS%"=="" SET CTOOLS=c:\tools\watcom\binb\
if "%CTOOLS%"=="" ECHO Environment too small!
if "%CTOOLS%"=="" ECHO Add SHELL=C:\DOS\COMMAND.COM /P /E:1024
if "%CTOOLS%"=="" ECHO to CONFIG.SYS
if "%CTOOLS%"=="" goto done
:begin
if "%1"=="" goto norm
if "%1"=="-d" goto debug
if "%1"=="-D" goto debug
if not exist ..\obj\showuif\%1.obj goto useage
del ..\obj\showuif\%1.obj
shift
goto begin
:norm
%CTOOLS%\wmake /h /f showuiff.mak showuiff.exe
if errorlevel 1 goto error
goto done
:debug
%CTOOLS%\wmake /h /f showuiff.mak showuiff.exe DEBUG=""
if errorlevel 1 goto error
goto done
:error
echo Whoops!
goto done
:useage
echo %1 not found!
echo Enter %0 -d for debugging
echo Enter %0 modulename(s) to recompile module(s)
echo Enter %0 modulename(s) -d to recompile module(s) with debugging
:done
