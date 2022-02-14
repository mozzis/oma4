@echo off
if "%1"=="" goto useage
if not exist omav%1a.exe goto mka
pkzip -eX -f omav%1a.exe @cppg1.lst
goto dob
:mka
pkzip -eX omav%1a @cppg1.lst
copy /y disk1.id %tmp%disk.id
pkzip -eX omav%1a %tmp%disk.id 
zip2exe -j omav%1a
del omav%1a.zip
:dob
if not exist omav%1b.exe goto mkb
pkzip -eX -f omav%1b.exe @cppg2.lst
goto done
:mkb
pkzip -eX omav%1b @cppg2.lst
copy /y disk2.id %tmp%disk.id
pkzip -eX omav%1b %tmp%disk.id
zip2exe -j omav%1b
del omav%1b.zip
goto done
:useage
echo Type %0 "version"
:done
