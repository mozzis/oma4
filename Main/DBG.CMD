@echo off
:loop
if "%1"=="" goto end
echo %1
del ..\obj\os2\%1.obj
shift
goto loop
:end
nmake /s /c /nologo OS2 DELOUSE="" PROT=""
if errorlevel 1 start /WIN /N "Bad News!" C:\OS2\UTIL\LST.EXE ERRS

