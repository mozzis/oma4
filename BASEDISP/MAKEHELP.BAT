@echo off
REM -- First make map file from App Studio generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by BASEDISP.HPJ. >hlp\basedisp.hm
echo. >>hlp\basedisp.hm
echo // Commands (ID_* and IDM_*) >>hlp\basedisp.hm
%TOOLS%\msvc\bin\makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>hlp\basedisp.hm
echo. >>hlp\basedisp.hm
echo // Prompts (IDP_*) >>hlp\basedisp.hm
%TOOLS%\msvc\bin\makehm IDP_,HIDP_,0x30000 resource.h >>hlp\basedisp.hm
echo. >>hlp\basedisp.hm
echo // Resources (IDR_*) >>hlp\basedisp.hm
%TOOLS%\msvc\bin\makehm IDR_,HIDR_,0x20000 resource.h >>hlp\basedisp.hm
echo. >>hlp\basedisp.hm
echo // Dialogs (IDD_*) >>hlp\basedisp.hm
%TOOLS%\msvc\bin\makehm IDD_,HIDD_,0x20000 resource.h >>hlp\basedisp.hm
echo. >>hlp\basedisp.hm
echo // Frame Controls (IDW_*) >>hlp\basedisp.hm
%TOOLS%\msvc\bin\makehm IDW_,HIDW_,0x50000 resource.h >>hlp\basedisp.hm
REM -- Make help for Project BASEDISP
call %TOOLS%\msvc\bin\hc31 basedisp.hpj
echo.
