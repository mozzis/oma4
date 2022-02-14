@rem echo off
@SET MODE=
@set DEBUG_LIST=
:loop
@if "%1"=="" goto end
@if "%1"=="-d" set MODE=DEBUG
@if "%1"=="-d" goto next
@if "%1"=="-D" set MODE=DEBUG
@if "%1"=="-D" goto next
@echo %1
@del ..\obj\mcdriv\%1.obj
@if "%MODE%"=="DEBUG" set DEBUG_LIST=%DEBUG_LIST% %1
:next
@shift
@goto loop
:end
@if not "%DEBUG_LIST%"=="" echo %DEBUG_LIST%
c:\code\c\bin\make MODE="%MODE%"
@if errorlevel 1 be beep
@SET MODE=
@set DEBUG_LIST=
