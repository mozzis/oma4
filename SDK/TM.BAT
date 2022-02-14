@echo off
if "%TOOLS%"=="" SET TOOLS=C:\TOOLS
: set location of object files
set OBJ=..\obj\tcdriv
: set location of TurboC
set CDIR=%TOOLS%\bc\bin
set tmpbcc=%bccpp%
set bccpp=
:begin
if "%1"=="" goto norm
if "%1"=="-d" goto debug
if "%1"=="-D" goto debug
if not exist %OBJ%\%1.obj goto useage
del %OBJ%\%1.obj
shift
goto begin
:norm
%CDIR%\make -r -a -ftcdriv.mak  
if errorlevel 1 goto error
%CDIR%\make -r -a -ftcoma4si.mak
if errorlevel 1 goto error
%CDIR%\make -r -a -fttstdriv.mak
if errorlevel 1 goto error
goto done
:debug
%CDIR%\make -r -a -ftcdriv.mak   -DDEBUG
if errorlevel 1 goto error
%CDIR%\make -r -a -ftcoma4si.mak -DDEBUG
if errorlevel 1 goto error
%CDIR%\make -r -a -fttstdriv.mak -DDEBUG
if errorlevel 1 goto error
goto done
:error
echo Whoops!
goto done
:useage
echo Enter %0 -d for debugging
echo Enter %0 modulename(s) to recompile module(s)
echo Enter %0 modulename(s) -d to recompile module(s) with debugging
echo Enter %0 to automatically compile and link as needed
echo (This batch file assumes object files are found in %OBJ%)
echo (This batch file assumes TurboC is in %CDIR%)
:done
set CDIR=
set OBJ=
set bccpp=%tmpbcc%
set tmpbcc=

