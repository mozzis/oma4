@echo off
set VESA=XXXX
if not "%VESA%"=="XXXX" goto noenv
if "%1"=="" goto useage
if not exist vgainfo.exe goto noprog
echo Checking your video hardware
vgainfo
if errorlevel  1 set VESA=NOVG
if errorlevel  2 set VESA=VESA
if errorlevel  3 set VESA=VID7
if errorlevel  4 set VESA=PARA
if errorlevel  5 set VESA=ATI
if errorlevel  6 set VESA=TSE3
if errorlevel  7 set VESA=TSE4
if errorlevel  8 set VESA=OAK
if errorlevel  9 set VESA=TRID
if errorlevel 10 set VESA=CHIP
if errorlevel 11 set VESA=GENO
if errorlevel 12 set VESA=S3
if errorlevel 13 set VESA=CIRR
:dovesa
if "%VESA%"=="NOVG" goto novga
if "%VESA%"=="VESA" goto nodriv
if "%VESA%"=="XXXX" goto novesa
if "%VESA%"=="CIRR" goto nomode
if "%VESA%"=="S3"   goto extras
if "%VESA%"=="GENO" copy GVESA.COM    %1\VESA.COM
if "%VESA%"=="GENO" goto nohid
if "%VESA%"=="CHIP" copy VESA452.COM  %1\VESA.COM
if "%VESA%"=="CHIP" goto okdone
if "%VESA%"=="TRID" copy TVESA.EXE    %1\VESA.EXE
if "%VESA%"=="TRID" goto nohid
if "%VESA%"=="OAK"  copy 67VESA.COM   %1\VESA.COM
if "%VESA%"=="OAK"  goto nohid
if "%VESA%"=="TSE4" copy TLIVESA.COM  %1\VESA.COM
if "%VESA%"=="TSE4" goto okdone
if "%VESA%"=="TSE3" copy TLIVESA.COM  %1\VESA.COM
if "%VESA%"=="TSE3" goto okdone
if "%VESA%"=="ATI"  copy VVESA.COM    %1\VESA.COM
if "%VESA%"=="ATI"  goto okdone
if "%VESA%"=="PARA" copy D2VESA.EXE   %1\VESA.EXE
if "%VESA%"=="PARA" goto okdone
if "%VESA%"=="VID7" copy VIDEO7.EXE   %1\VESA.EXE
if "%VESA%"=="VID7" goto okdone
if "%VESA%"=="SIGM" copy SIGVESA.COM  %1\VESA.COM
if "%VESA%"=="SIGM" goto okdone
if "%VESA%"=="TECM" copy VGAVESA.COM  %1\VESA.COM
if "%VESA%"=="TECM" goto okdone
if "%VESA%"=="ORCH" copy ORCHDVSA.COM %1\VESA.COM
if "%VESA%"=="ORCH" goto nohid
if "%VESA%"=="APPI" copy APVESA.EXE   %1\VESA.EXE
if "%VESA%"=="APPI" goto nohid
if "%VESA%"=="CHIP" copy VESA452.COM  %1\VESA.COM
if "%VESA%"=="CHIP" goto okdone
if "%VESA%"=="EVRX" copy EVRXVESA.COM %1\VESA.COM
if "%VESA%"=="EVRX" goto okdone
goto done
:noenv
echo 
echo.
echo Not enough environment space!
echo Increase environment in CONFIG.SYS and try again.
goto done
:noprog
echo 
echo.
echo Can't find VGAINFO.EXE!
echo Log onto the disk with %0 and try again.
goto done
:novga
echo 
echo.
echo Your video controller does not support SuperVGA!
goto done
:nodriv
echo 
echo.
echo Your video controller supports VESA but is unrecognized!
goto noves2
:novesa
echo 
echo.
echo Your video controller is unrecognized!
:noves2
echo.
echo If you know the name of the video controller chip set,
echo and it is one of the following, this program can install
echo a driver which MAY work:
echo.
echo     1. Sigma Designs
echo     2. Tecmar
echo     3. Oak    (no HiDRIS support)
echo     4. Orchid (no HiDRIS support)
echo     5. Appian (no HiDRIS support)
echo     6. Everex
echo     7. Genoa
echo     8. Trident
echo     9. Tseng Labs' ET4000 or ET3000
echo     A. ATI Ultra or Wonder
echo     B. Western Digital or Diamond 24X or PARADISE
echo     C. None of the above or don't know
echo.
m        Choose one: /c:123456789ABC
echo.
if errorlevel  1 set VESA=SIGM
if errorlevel  2 set VESA=TECM
if errorlevel  3 set VESA=OAK
if errorlevel  4 set VESA=ORCH
if errorlevel  5 set VESA=APPI
if errorlevel  6 set VESA=EVRX
if errorlevel  7 set VESA=GENO
if errorlevel  8 set VESA=TRID
if errorlevel  9 set VESA=TSE4
if errorlevel 10 set VESA=ATI
if errorlevel 11 set VESA=PARA
if not errorlevel 12 goto dovesa
if errorlevel 12 goto baddone
goto done
:nomode
echo 
echo.
echo The VESA driver for this controller does not support the modes
echo required by EG&G software.  You may be able to obtain an updated
echo driver from the video controller manufacturer.  For reference,
echo the VESA modes required by our software are:
echo.
echo      Mode 101h - 640 X 480 X 256  (8 bit colors)
echo      Mode 105h - 1024 X 768 X 256 (8 bit colors)
echo.
goto done
:nohid
echo 
echo.
echo The VESA driver for this controller does not support the modes
echo required by the EG&G HiDRIS program, but will run the OMA4FF
echo utility supplied with some detectors and software. You may be 
echo able to obtain an updated driver from the video controller 
echo manufacturer.  For reference, the VESA modes required by our 
echo software are:
echo.
echo      Mode 101h - 640 X 480 X 256  (8 bit colors)
echo      Mode 105h - 1024 X 768 X 256 (8 bit colors)
echo.
:okdone
if not exist %1\VESA.* goto baddone
echo The VESA TSR was copied successfully.
echo To activate the VESA TSR, type %1\VESA
goto done
:baddone
echo.
echo No VESA TSR could be copied to %1.
echo.
goto done
:useage
echo.
echo %0 - Copies correct VESA driver for your video hardware
echo Type %0 [install-directory]
echo.
:done
