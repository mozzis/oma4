# Microsoft Visual C++ generated build script - Do not modify

PROJ = INSTW3
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = E:\CODE\INSTW3\
USEMFC = 1
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC =             
FIRSTCPP = MAIN.CPP    
RC = rc
CFLAGS_D_WEXE = /nologo /W3 /FR /G2 /Zi /D_DEBUG /Od /AM /GA /Fd"INSTW3.PDB"
CFLAGS_R_WEXE = /nologo /W3 /FR /O1 /DNDEBUG /AM /GA
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /CO  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WEXE = mafxcwd oldnames libw mlibcew commdlg.lib lzexpand.lib shell.lib 
LIBS_R_WEXE = mafxcw oldnames libw mlibcew commdlg.lib lzexpand.lib olecli.lib olesvr.lib shell.lib 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = INSTW3.DEF
OBJS_EXT = 
LIBS_EXT = 
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
SBRS = MAIN.SBR


all:	$(PROJ).EXE $(PROJ).BSC

MAIN.OBJ:	MAIN.CPP $(MAIN_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c MAIN.CPP

INSTW3.RES:	INSTW3.RC $(INSTW3_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r INSTW3.RC


$(PROJ).EXE::	INSTW3.RES

$(PROJ).EXE::	MAIN.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
MAIN.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\tools\msvc\lib\+
c:\tools\msvc\mfc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) INSTW3.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	INSTW3.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) INSTW3.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
