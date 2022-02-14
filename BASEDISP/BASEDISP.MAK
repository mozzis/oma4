# Microsoft Visual C++ generated build script - Do not modify

PROJ = BASEDISP
DEBUG = 1
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = ..\sdkwin\oma4win.dll ..\datmgr\datmgr.dll
D_RCDEFINES = /d_DEBUG
R_RCDEFINES = /dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\CODE\OMA4\BASEDISP\
USEMFC = 1
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC =             
FIRSTCPP = STDAFX.CPP  
RC = rc
CFLAGS_D_WEXE = /nologo /Gs /G2 /FPi87 /Zp1 /W3 /Z7 /AM /YX /Od /D "_DEBUG" /D "_WINOMA_" /I "..\inc" /GA /Fp"BASEDISP.PCH"
CFLAGS_R_WEXE = /nologo /Gs /G2 /FPi87 /Zp1 /W3 /Z7 /AM /YX /O2 /Ob1 /D "NDEBUG" /D "_WINOMA_" /I "..\inc" /FR /GA /Fp"BASEDISP.PCH"
LFLAGS_D_WEXE = /NOLOGO /NOD /NOE /PACKC:61440 /STACK:20480 /ALIGN:16 /ONERROR:NOEXE /CO  
LFLAGS_R_WEXE = /NOLOGO /NOD /NOE /PACKC:61440 /STACK:20480 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WEXE = mafxcwd mfcoleui mlibcew oldnames libw compobj storage ole2 ole2disp commdlg.lib olecli.lib olesvr.lib shell.lib ver.lib 
LIBS_R_WEXE = mafxcw mfcoleui mlibcew oldnames libw compobj storage ole2 ole2disp commdlg.lib olecli.lib olesvr.lib shell.lib ver.lib 
RCFLAGS = /nologo /z
RESFLAGS = /nologo /t
RUNFLAGS = 
DEFFILE = BASEDISP.DEF
OBJS_EXT = 
LIBS_EXT = ..\DATMGR\DATMGR.LIB ..\SDKWIN\OMA4WIN.LIB OMABLT.LIB 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WEXE)
LFLAGS = $(LFLAGS_D_WEXE)
LIBS = $(LIBS_D_WEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WEXE)
LFLAGS = $(LFLAGS_R_WEXE)
LIBS = $(LIBS_R_WEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = STDAFX.SBR \
		BASEDISP.SBR \
		MAINFRM.SBR \
		DOCUIF.SBR \
		VIEW.SBR \
		MYDLGBAR.SBR \
		OLEITEM.SBR \
		IPFRAME.SBR \
		CDIB.SBR \
		MYTRACK.SBR \
		RECTINFO.SBR


DATMGR_DEP = 

OMA4WIN_DEP = 

OMABLT_DEP = 

BASEDISP_RCDEP = c:\code\oma4\basedisp\res\basedisp.ico \
	c:\code\oma4\basedisp\res\brite.bmp \
	c:\code\oma4\basedisp\res\cont.bmp \
	c:\code\oma4\basedisp\res\zoom.bmp \
	c:\code\oma4\basedisp\res\slideidx.bmp \
	c:\code\oma4\basedisp\res\hright.bmp \
	c:\code\oma4\basedisp\res\hleft.bmp \
	c:\code\oma4\basedisp\res\vdown.bmp \
	c:\code\oma4\basedisp\res\vup.bmp \
	c:\code\oma4\basedisp\res\zfrmup.bmp \
	c:\code\oma4\basedisp\res\zfrmdn.bmp \
	c:\code\oma4\basedisp\res\slidehdl.bmp \
	c:\code\oma4\basedisp\res\slidemsk.bmp \
	c:\code\oma4\basedisp\res\basedisp.rc2


STDAFX_DEP = c:\code\oma4\basedisp\stdafx.h


BASEDISP_DEP = c:\code\oma4\basedisp\stdafx.h \
	c:\code\oma4\basedisp\basedisp.h \
	c:\code\oma4\basedisp\mainfrm.h \
	c:\code\oma4\basedisp\docuif.h \
	c:\code\oma4\datmgr\datmgr.h \
	c:\code\oma4\basedisp\view.h \
	c:\code\oma4\basedisp\mytrack.h


MAINFRM_DEP = c:\code\oma4\basedisp\stdafx.h \
	c:\code\oma4\basedisp\basedisp.h \
	c:\code\oma4\basedisp\docuif.h \
	c:\code\oma4\datmgr\datmgr.h \
	c:\code\oma4\basedisp\mydlgbar.h \
	c:\code\oma4\basedisp\rectinfo.h \
	c:\code\oma4\basedisp\view.h \
	c:\code\oma4\basedisp\mytrack.h \
	c:\code\oma4\basedisp\mainfrm.h


DOCUIF_DEP = c:\code\oma4\basedisp\stdafx.h \
	c:\code\oma4\basedisp\basedisp.h \
	c:\code\oma4\basedisp\docuif.h \
	c:\code\oma4\datmgr\datmgr.h \
	c:\code\oma4\basedisp\oleitem.h


VIEW_DEP = c:\code\oma4\basedisp\stdafx.h \
	c:\code\oma4\basedisp\basedisp.h \
	c:\code\oma4\basedisp\docuif.h \
	c:\code\oma4\datmgr\datmgr.h \
	c:\code\oma4\basedisp\mainfrm.h \
	c:\code\oma4\basedisp\mydlgbar.h \
	c:\code\oma4\basedisp\rectinfo.h \
	c:\code\oma4\basedisp\view.h \
	c:\code\oma4\basedisp\mytrack.h


MYDLGBAR_DEP = c:\code\oma4\basedisp\stdafx.h \
	c:\code\oma4\basedisp\basedisp.h \
	c:\code\oma4\basedisp\mydlgbar.h


OLEITEM_DEP = c:\code\oma4\basedisp\stdafx.h \
	c:\code\oma4\basedisp\basedisp.h \
	c:\code\oma4\basedisp\docuif.h \
	c:\code\oma4\datmgr\datmgr.h \
	c:\code\oma4\basedisp\oleitem.h


IPFRAME_DEP = c:\code\oma4\basedisp\stdafx.h \
	c:\code\oma4\basedisp\basedisp.h \
	c:\code\oma4\basedisp\ipframe.h


CDIB_DEP = c:\code\oma4\basedisp\stdafx.h


MYTRACK_DEP = c:\code\oma4\basedisp\stdafx.h \
	c:\code\oma4\basedisp\mytrack.h


RECTINFO_DEP = c:\code\oma4\basedisp\stdafx.h \
	c:\code\oma4\basedisp\basedisp.h \
	c:\code\oma4\basedisp\rectinfo.h


all:	$(PROJ).EXE

BASEDISP.RES:	BASEDISP.RC $(BASEDISP_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r BASEDISP.RC

STDAFX.OBJ:	STDAFX.CPP $(STDAFX_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c STDAFX.CPP

BASEDISP.OBJ:	BASEDISP.CPP $(BASEDISP_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c BASEDISP.CPP

MAINFRM.OBJ:	MAINFRM.CPP $(MAINFRM_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c MAINFRM.CPP

DOCUIF.OBJ:	DOCUIF.CPP $(DOCUIF_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c DOCUIF.CPP

VIEW.OBJ:	VIEW.CPP $(VIEW_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c VIEW.CPP

MYDLGBAR.OBJ:	MYDLGBAR.CPP $(MYDLGBAR_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c MYDLGBAR.CPP

OLEITEM.OBJ:	OLEITEM.CPP $(OLEITEM_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c OLEITEM.CPP

IPFRAME.OBJ:	IPFRAME.CPP $(IPFRAME_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c IPFRAME.CPP

CDIB.OBJ:	CDIB.CPP $(CDIB_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c CDIB.CPP

MYTRACK.OBJ:	MYTRACK.CPP $(MYTRACK_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c MYTRACK.CPP

RECTINFO.OBJ:	RECTINFO.CPP $(RECTINFO_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c RECTINFO.CPP


$(PROJ).EXE::	BASEDISP.RES

$(PROJ).EXE::	STDAFX.OBJ BASEDISP.OBJ MAINFRM.OBJ DOCUIF.OBJ VIEW.OBJ MYDLGBAR.OBJ \
	OLEITEM.OBJ IPFRAME.OBJ CDIB.OBJ MYTRACK.OBJ RECTINFO.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
STDAFX.OBJ +
BASEDISP.OBJ +
MAINFRM.OBJ +
DOCUIF.OBJ +
VIEW.OBJ +
MYDLGBAR.OBJ +
OLEITEM.OBJ +
IPFRAME.OBJ +
CDIB.OBJ +
MYTRACK.OBJ +
RECTINFO.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\tools\msvc\lib\+
c:\tools\msvc\mfc\lib\+
..\DATMGR\DATMGR.LIB+
..\SDKWIN\OMA4WIN.LIB+
OMABLT.LIB+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) BASEDISP.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	BASEDISP.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) BASEDISP.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
