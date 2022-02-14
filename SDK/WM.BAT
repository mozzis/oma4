@echo off
SET WCCINC=/i#c:\tools\watcom\h;c:\tools\menuet\inc;c:\tools\metaw\inc;..\inc;c:\tools\phar386\inc
if "%TOOLS%"=="" SET TOOLS=C:\TOOLS
:begin
if "%1"=="" goto norm
if "%1"=="-d" goto debug
if "%1"=="-D" goto debug
if not exist ..\obj\wcdriv\%1.obj goto useage
del ..\obj\wcdriv\%1.obj
shift
goto begin
:norm
%TOOLS%\watcom\binb\wmake /h /f wcdriv.mak ..\obj\wcdriv\wcdriv.lib
if errorlevel 1 goto error
%TOOLS%\watcom\binb\wmake /h /f wtstdriv.mak testdriv.exe 
if errorlevel 1 goto error
%TOOLS%\watcom\binb\wmake /h /f wcoma4si.mak oma4si.exe 
if errorlevel 1 goto error
goto done
:debug
%TOOLS%\watcom\binb\wmake /h /f wcdriv.mak wcdriv.lib DEBUG=""
if errorlevel 1 goto error
%TOOLS%\watcom\binb\wmake /h /f wtstdriv.mak testdriv.exe DEBUG=""
if errorlevel 1 goto error
%TOOLS%\watcom\binb\wmake /h /f wcoma4si.mak oma4si.exe DEBUG=""
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
