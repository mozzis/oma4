# Microsoft Visual C++ generated build script - Do not modify

PROJ = DATMGR
DEBUG = 0
PROGTYPE = 1
CALLER = ..\basedisp\basedisp.exe
ARGS = 
DLLS = ..\sdkwin\oma4win.dll
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = D:\CODE\OMA4\DATMGR\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC =             
FIRSTCPP = DATMGR.CPP  
RC = rc
CFLAGS_D_WDLL = /nologo /G2 /W3 /Zi /ALw /Od /D "_DEBUG" /D "_WINOMA_" /FR /GD /Fd"DATMGR.PDB"
CFLAGS_R_WDLL = /nologo /W3 /ALw /O1 /D "NDEBUG" /D "_WINOMA_" /FR /GD 
LFLAGS_D_WDLL = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /CO /NOE /ALIGN:16 /MAP:FULL
LFLAGS_R_WDLL = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /NOE /ALIGN:16 /MAP:FULL
LIBS_D_WDLL = oldnames libw commdlg shell olecli olesvr ldllcew
LIBS_R_WDLL = oldnames libw commdlg shell olecli olesvr ldllcew
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = DATMGR.DEF
OBJS_EXT = 
LIBS_EXT = ..\BASEDISP\OMABLT.LIB 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WDLL)
LFLAGS = $(LFLAGS_D_WDLL)
LIBS = $(LIBS_D_WDLL)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WDLL)
LFLAGS = $(LFLAGS_R_WDLL)
LIBS = $(LIBS_R_WDLL)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = DATMGR.SBR \
		WINTCL.SBR


DATMGR_DEP = d:\code\oma4\datmgr\datmgr.h \
	d:\code\oma4\datmgr\datadir.h \
	d:\code\oma4\datmgr\unimeth.h \
	d:\code\oma4\datmgr\wintcl.h


WINTCL_DEP = d:\code\oma4\datmgr\wintcl.h \
	d:\code\oma4\datmgr\datmgr.h \
	d:\code\oma4\datmgr\datadir.h


OMABLT_DEP = 

all:	$(PROJ).DLL $(PROJ).BSC

DATMGR.OBJ:	DATMGR.CPP $(DATMGR_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c DATMGR.CPP

WINTCL.OBJ:	WINTCL.CPP $(WINTCL_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c WINTCL.CPP


$(PROJ).DLL::	DATMGR.OBJ WINTCL.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
DATMGR.OBJ +
WINTCL.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
C:\TOOLS\MSVC\LIB\+
C:\TOOLS\MSVC\MFC\LIB\+
C:\TOOLS\XVT\LIB\+
..\BASEDISP\OMABLT.LIB+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) $@
	implib /nowep $(PROJ).LIB $(PROJ).DLL


run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
