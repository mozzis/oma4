@echo off
if "%1"=="" goto useage
pkunzip -n %1\omasrc1 main\*.c main
pkunzip -n %1\omasrc2 inc\*.h inc
pkunzip -n %1\omasrc2 macro\*.c macro
pkunzip -n %1\omasrc2 ..\forms\*.c ..\forms
pkunzip -n %1\omasrc2 ..\forms\inc\*.h ..\forms\inc
pkunzip -n %1\omasrc2 cppg?.lst .
pkunzip -n %1\omasrc2 cppg?.bat .
pkunzip -n -X*.bat %1\omasrc2 run\*.* run
pkunzip -n -X*.bat %1\omasrc3 detect\*.* detect
pkunzip -n -X*.bat %1\omasrc4 sdk\*.* sdk
pkunzip -n -X*.bat %1\omasrc4 inc\*.h inc
pkunzip -n -X*.bat %1\omasrc4 sdkwin\*.* sdkwin
pkunzip -n -X*.bat -X*.pmi %1\omasrc4 oma4ff\*.* oma4ff
pkunzip -n -d -X*.bat %1\omasrc5 basedisp\*.*
pkunzip -n -d -X*.bat %1\omasrc6 datmgr\*.*
goto done
:useage
echo Type %0 [source drive]
:done
