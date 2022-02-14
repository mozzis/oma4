@echo off
set INCTMP=%INCLUDE%
set INCLUDE=..\inc;d:\code\forms\inc;c:\tools\gss\src;c:\tools\dos16m;c:\tools\dos16m\src;c:\tools\msc6\inc;c:\gpib\src
set CLTMP=%CL%
set CL=-nologo -c -AL -G2s -Gt -FPi87 -W3 -DUSE_D16M -DCOMPILE_4000
set LINKTMP=%LINK%
set LINK=/NOD /NOLOGO
set ML=/W2
set LIBTMP=%LIB%
set LIB=c:\tools\msc6\lib
copy errs errs.old > NUL
echo use makefile in mode DEBUG to build oma4000 > errs
set source=..\main\cache386.c
echo debg compile ..\main\cache386.c
c:\tools\msc6\bin\cl -Zip -Od -DXSTAT -Fo..\obj\dos\cache386 ..\main\cache386.c >> errs
c:\tools\msc6\bin\cl -Zip -Od -DXSTAT -Fo..\obj\dos\oma4000 ..\main\oma4000.c >> errs
c:\tools\msc6\bin\link @C:\TEMP\LIS_0001.TMP  >>errs
echo dos16m makepm to create oma4000.exp >> errs
c:\tools\dos16m\makepm ..\run\oma4000 -L -DPMI -Q
%echo Splicing to Loader...
echo dos16m splice >> errs
c:\tools\dos16m\splice ..\run\oma4000 ..\run\oma4000 c:\tools\dos16m\loader.exe
%echo Deleting banner...
echo dos16m banner delete on oma4000.exe >> errs
c:\tools\dos16m\banner ..\run\oma4000.exe OFF
echo end polymake build of oma4000 >> errs
dir ..\run\oma4000.exe |find /I "oma4000  exe" >> errs
tail errs
set INCLUDE=%INCTMP%
set CL=%CLTMP%
set LINK=%LINKTMP%
set LIB=%LIBTMP%
