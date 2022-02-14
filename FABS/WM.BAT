@echo off
SET CDIR=%TOOLS%\watcom
:begin
if "%1"=="" goto norm
if "%1"=="-d" goto debug
if "%1"=="-D" goto debug
if not exist ..\obj\wcdriv\%1.obj goto useage
del ..\obj\wcdriv\%1.obj
shift
goto begin
:norm
%CDIR%\binb\wmake /h /f fabswc.mak fabs.exe
if errorlevel 1 goto error
goto done
:debug
%CDIR%\binb\wmake /h /f fabswc.mak fabs.exe DEBUG=""
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
