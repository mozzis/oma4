@echo off
SET INCLUDE=c:\tools\msc6\inc;c:\tools\gpib\src;c:\tools\gss\src;c:\tools\dos16m\src;..\inc;..\..\forms\inc
if "%TOOLS%"=="" SET TOOLS=C:\TOOLS
if "%CDIR%"=="" SET CDIR=%TOOLS%\MSC6
rem SET INCLUDE=%CDIR%\INC;..\inc
:begin
if "%1"=="" goto norm
if "%1"=="-d" goto debug
if "%1"=="-D" goto debug
if not exist ..\obj\mcdriv\%1.obj goto useage
del ..\obj\mcdriv\%1.obj
shift
goto begin
:norm
%CDIR%\BIN\nmake /f mcdriv.mak  
if errorlevel 1 goto error
%CDIR%\BIN\nmake /f mcoma4si.mak
if errorlevel 1 goto error
%CDIR%\BIN\nmake /f mtstdriv.mak
if errorlevel 1 goto error
goto done
:debug
%CDIR%\BIN\nmake /f mcdriv.mak   DEBUG=""
if errorlevel 1 goto error
%CDIR%\BIN\nmake /f mcoma4si.mak DEBUG=""
if errorlevel 1 goto error
%CDIR%\BIN\nmake /f mtstdriv.mak DEBUG=""
if errorlevel 1 goto error
goto done
:error
echo Whoops!
goto done
:useage
echo Enter nm -d for debugging
echo Enter nm modulename(s) to recompile module(s)
echo Enter nm modulename(s) -d to recompile module(s) with debugging
:done
