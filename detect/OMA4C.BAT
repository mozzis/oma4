@if "%1"=="" GOTO USEAGE
@SET FILE=mmscans
@if "%1"=="-a" goto default
@if "%1"=="-A" goto default
@IF NOT "%1"=="" SET FILE=%1
@SHIFT
:default
@SET ASIC=.\
@SET DAC=.\
@SET OMA4=.\
@if "%1"=="-a" goto doasic
@if "%1"=="-A" goto doasic
@goto dodac
:doasic
%OMA4RUN%OMA4 -DSWITCHER %ASIC%%FILE%.ASC %OMA4RUN%%FILE%.ABJ
@IF ERRORLEVEL 1 GOTO PROBLEM
:dodac
%OMA4RUN%OMA4 -DSWITCHER %DAC%%FILE%.DAC %OMA4RUN%%FILE%.DBJ
@IF ERRORLEVEL 1 GOTO PROBLEM
COPY %OMA4RUN%%FILE%.ABJ %OMA4RUN%SCANS.ABJ
COPY %OMA4RUN%%FILE%.DBJ %OMA4RUN%SCANS.DBJ
@IF EXIST LOADOMA.EXE ECHO Clearing OMA memory...
@IF EXIST LOADOMA.EXE LOADOMA -AC00000 -ED
@IF EXIST LOADOMA.EXE LOADOMA -AC00000 -EP
@GOTO DONE
:PROBLEM
@ECHO %0 stopped
@GOTO DONE
:USEAGE
@ECHO Useage: "%0 <base file name> [-a]"
@ECHO Example:
@ECHO   %0 mmscans -a   (Compiles single channel thompson code)
@ECHO   %0 mmscans      (Same operation, but only compile DAC, not ASIC)
:DONE
@SET ASIC=
@SET DAC=
