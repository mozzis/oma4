# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "basedisp32.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
MTL=MkTypLib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/basedisp32.exe $(OUTDIR)/basedisp32.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE CPP /nologo /Zp1 /MT /W3 /GX /Z7 /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINOMA_" /D "_MBCS" /FR /c
# ADD CPP /nologo /Zp1 /MT /W3 /GX /Z7 /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINOMA_" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /Zp1 /MT /W3 /GX /Z7 /YX /O2 /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /D "_WINOMA_" /D "_MBCS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"basedisp32.pch"\
 /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"BASEDISP.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"basedisp32.bsc" 
BSC32_SBRS= \
	$(INTDIR)/STDAFX.SBR \
	$(INTDIR)/BASEDISP.SBR \
	$(INTDIR)/MAINFRM.SBR \
	$(INTDIR)/DOCUIF.SBR \
	$(INTDIR)/VIEW.SBR \
	$(INTDIR)/MYDLGBAR.SBR \
	$(INTDIR)/OLEITEM.SBR \
	$(INTDIR)/IPFRAME.SBR \
	$(INTDIR)/CDIB.SBR \
	$(INTDIR)/MYTRACK.SBR \
	$(INTDIR)/RECTINFO.SBR

$(OUTDIR)/basedisp32.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 oldnames.lib ver.lib /NOLOGO /STACK:0x20480 /SUBSYSTEM:windows /MACHINE:IX86
# ADD LINK32 oldnames.lib ver.lib /NOLOGO /STACK:0x20480 /SUBSYSTEM:windows /MACHINE:IX86
LINK32_FLAGS=oldnames.lib ver.lib /NOLOGO /STACK:0x20480 /SUBSYSTEM:windows\
 /INCREMENTAL:no /PDB:$(OUTDIR)/"basedisp32.pdb" /MACHINE:IX86\
 /DEF:".\BASEDISP.DEF" /OUT:$(OUTDIR)/"basedisp32.exe" 
DEF_FILE=.\BASEDISP.DEF
LINK32_OBJS= \
	\CODE\OMA4\DATMGR\DATMGR.LIB \
	\CODE\OMA4\SDKWIN\OMA4WIN.LIB \
	.\OMABLT.LIB \
	$(INTDIR)/BASEDISP.res \
	$(INTDIR)/STDAFX.OBJ \
	$(INTDIR)/BASEDISP.OBJ \
	$(INTDIR)/MAINFRM.OBJ \
	$(INTDIR)/DOCUIF.OBJ \
	$(INTDIR)/VIEW.OBJ \
	$(INTDIR)/MYDLGBAR.OBJ \
	$(INTDIR)/OLEITEM.OBJ \
	$(INTDIR)/IPFRAME.OBJ \
	$(INTDIR)/CDIB.OBJ \
	$(INTDIR)/MYTRACK.OBJ \
	$(INTDIR)/RECTINFO.OBJ

$(OUTDIR)/basedisp32.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/basedisp32.exe $(OUTDIR)/basedisp32.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /Zp1 /MT /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINOMA_" /D "_MBCS" /FR /c
# ADD CPP /nologo /Zp1 /MT /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINOMA_" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /Zp1 /MT /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "_WINOMA_" /D "_MBCS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"basedisp32.pch"\
 /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"BASEDISP.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"basedisp32.bsc" 
BSC32_SBRS= \
	$(INTDIR)/STDAFX.SBR \
	$(INTDIR)/BASEDISP.SBR \
	$(INTDIR)/MAINFRM.SBR \
	$(INTDIR)/DOCUIF.SBR \
	$(INTDIR)/VIEW.SBR \
	$(INTDIR)/MYDLGBAR.SBR \
	$(INTDIR)/OLEITEM.SBR \
	$(INTDIR)/IPFRAME.SBR \
	$(INTDIR)/CDIB.SBR \
	$(INTDIR)/MYTRACK.SBR \
	$(INTDIR)/RECTINFO.SBR

$(OUTDIR)/basedisp32.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 oldnames.lib ver.lib /NOLOGO /STACK:0x20480 /SUBSYSTEM:windows /DEBUG /MACHINE:IX86
# ADD LINK32 oldnames.lib ver.lib /NOLOGO /STACK:0x20480 /SUBSYSTEM:windows /DEBUG /MACHINE:IX86
LINK32_FLAGS=oldnames.lib ver.lib /NOLOGO /STACK:0x20480 /SUBSYSTEM:windows\
 /INCREMENTAL:yes /PDB:$(OUTDIR)/"basedisp32.pdb" /DEBUG /MACHINE:IX86\
 /DEF:".\BASEDISP.DEF" /OUT:$(OUTDIR)/"basedisp32.exe" 
DEF_FILE=.\BASEDISP.DEF
LINK32_OBJS= \
	\CODE\OMA4\DATMGR\DATMGR.LIB \
	\CODE\OMA4\SDKWIN\OMA4WIN.LIB \
	.\OMABLT.LIB \
	$(INTDIR)/BASEDISP.res \
	$(INTDIR)/STDAFX.OBJ \
	$(INTDIR)/BASEDISP.OBJ \
	$(INTDIR)/MAINFRM.OBJ \
	$(INTDIR)/DOCUIF.OBJ \
	$(INTDIR)/VIEW.OBJ \
	$(INTDIR)/MYDLGBAR.OBJ \
	$(INTDIR)/OLEITEM.OBJ \
	$(INTDIR)/IPFRAME.OBJ \
	$(INTDIR)/CDIB.OBJ \
	$(INTDIR)/MYTRACK.OBJ \
	$(INTDIR)/RECTINFO.OBJ

$(OUTDIR)/basedisp32.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Group "Object/Library Files"

################################################################################
# Begin Source File

SOURCE=\CODE\OMA4\DATMGR\DATMGR.LIB
# End Source File
################################################################################
# Begin Source File

SOURCE=\CODE\OMA4\SDKWIN\OMA4WIN.LIB
# End Source File
################################################################################
# Begin Source File

SOURCE=.\OMABLT.LIB
# End Source File
# End Group
################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\BASEDISP.RC
DEP_BASED=\
	.\RES\BASEDISP.ICO\
	.\RES\TOOLBAR.BMP\
	.\RES\BRITE.BMP\
	.\RES\CONT.BMP\
	.\RES\ZOOM.BMP\
	.\RES\SLIDEIDX.BMP\
	.\RES\HRIGHT.BMP\
	.\RES\HLEFT.BMP\
	.\RES\VDOWN.BMP\
	.\RES\VUP.BMP\
	.\RES\ZFRMUP.BMP\
	.\RES\ZFRMDN.BMP\
	.\RES\SLIDEHDL.BMP\
	.\RES\SLIDEMSK.BMP\
	.\res\basedisp.rc2

$(INTDIR)/BASEDISP.res :  $(SOURCE)  $(DEP_BASED) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\STDAFX.CPP
DEP_STDAF=\
	.\stdafx.h

$(INTDIR)/STDAFX.OBJ :  $(SOURCE)  $(DEP_STDAF) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BASEDISP.CPP
DEP_BASEDI=\
	.\stdafx.h\
	.\basedisp.h\
	.\mainfrm.h\
	.\docuif.h\
	.\View.h\
	..\datmgr\datmgr.h\
	.\mytrack.h

$(INTDIR)/BASEDISP.OBJ :  $(SOURCE)  $(DEP_BASEDI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MAINFRM.CPP
DEP_MAINF=\
	.\stdafx.h\
	.\basedisp.h\
	.\docuif.h\
	.\mydlgbar.h\
	.\rectinfo.h\
	.\View.h\
	.\mainfrm.h\
	..\datmgr\datmgr.h\
	.\mytrack.h

$(INTDIR)/MAINFRM.OBJ :  $(SOURCE)  $(DEP_MAINF) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DOCUIF.CPP
DEP_DOCUI=\
	.\stdafx.h\
	.\basedisp.h\
	.\docuif.h\
	.\oleitem.h\
	..\datmgr\datmgr.h

$(INTDIR)/DOCUIF.OBJ :  $(SOURCE)  $(DEP_DOCUI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\VIEW.CPP
DEP_VIEW_=\
	.\stdafx.h\
	.\basedisp.h\
	.\docuif.h\
	.\mainfrm.h\
	.\mydlgbar.h\
	.\cdib.h\
	.\rectinfo.h\
	.\View.h\
	..\datmgr\datmgr.h\
	.\mytrack.h

$(INTDIR)/VIEW.OBJ :  $(SOURCE)  $(DEP_VIEW_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MYDLGBAR.CPP
DEP_MYDLG=\
	.\stdafx.h\
	.\basedisp.h\
	.\mydlgbar.h

$(INTDIR)/MYDLGBAR.OBJ :  $(SOURCE)  $(DEP_MYDLG) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\OLEITEM.CPP
DEP_OLEIT=\
	.\stdafx.h\
	.\basedisp.h\
	.\docuif.h\
	.\oleitem.h\
	..\datmgr\datmgr.h

$(INTDIR)/OLEITEM.OBJ :  $(SOURCE)  $(DEP_OLEIT) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IPFRAME.CPP
DEP_IPFRA=\
	.\stdafx.h\
	.\basedisp.h\
	.\ipframe.h

$(INTDIR)/IPFRAME.OBJ :  $(SOURCE)  $(DEP_IPFRA) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CDIB.CPP
DEP_CDIB_=\
	.\stdafx.h\
	.\cdib.h

$(INTDIR)/CDIB.OBJ :  $(SOURCE)  $(DEP_CDIB_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MYTRACK.CPP
DEP_MYTRA=\
	.\stdafx.h\
	.\mytrack.h

$(INTDIR)/MYTRACK.OBJ :  $(SOURCE)  $(DEP_MYTRA) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\RECTINFO.CPP
DEP_RECTI=\
	.\stdafx.h\
	.\basedisp.h\
	.\rectinfo.h

$(INTDIR)/RECTINFO.OBJ :  $(SOURCE)  $(DEP_RECTI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BASEDISP.DEF
# End Source File
# End Group
# End Project
################################################################################
