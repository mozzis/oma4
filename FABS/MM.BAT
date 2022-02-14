@echo off
SET CDIR=%TOOLS%\msc6
:begin
if "%1"=="" goto norm
if "%1"=="-d" goto debug
if "%1"=="-D" goto debug
if not exist ..\obj\mcdriv\%1.obj goto useage
del ..\obj\mcdriv\%1.obj
shift
goto begin
:norm
%CDIR%\bin\nmake /f fabsmc.mak
if errorlevel 1 goto error
goto done
:debug
%CDIR%\bin\nmake /f fabsmc.mak DEBUG=""
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
