@echo off
:begin
if "%1"=="" goto norm
if "%1"=="-d" goto debug
if "%1"=="-D" goto debug
if not exist ..\obj\wcdriv\%1.obj goto useage
del ..\obj\wcdriv\%1.obj
shift
goto begin
:norm
h:\tools\watcom\binb\wmake /h /f wcoma4ff.mak oma4ff.exe
if errorlevel 1 goto error
goto done
:debug
h:\tools\watcom\binb\wmake /h /f wcoma4ff.mak oma4ff.exe DEBUG=""
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
