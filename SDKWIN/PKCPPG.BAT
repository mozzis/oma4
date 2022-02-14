@echo off
if "%1"=="" goto useage
if not exist sdkwin%1.exe goto mk
pkzip -eX -f sdkwin%1.exe @cppg.lst
goto done
:mk
pkzip -eX sdkwin%1 @cppg.lst
zip2exe -j sdkwin%1 
del sdkwin%1.zip
goto done
:useage
echo Type %0 "version"
:done
