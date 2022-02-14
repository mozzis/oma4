# Microsoft Visual C++ generated build script - Do not modify

PROJ = OMA4DRV
DEBUG = 1
PROGTYPE = 6
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\CODE\OMA4000\MAIN\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = DETDRIV.C   
FIRSTCPP =             
RC = rc
CFLAGS_D_DEXE = /Gs /G2 /Zp1 /W3 /Z7 /AL /Gx- /Od /D "_DEBUG" /D "_DOS" /FR 
CFLAGS_R_DEXE = /nologo /Gs /G2 /Zp1 /W3 /AL /Gx- /Ox /D "NDEBUG" /D "_DOS" /FR 
LFLAGS_D_DEXE = /NOLOGO /NOI /STACK:5120 /ONERROR:NOEXE /CO 
LFLAGS_R_DEXE = /NOLOGO /NOI /STACK:5120 /ONERROR:NOEXE 
LIBS_D_DEXE = oldnames llibce 
LIBS_R_DEXE = oldnames llibce 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_DEXE)
LFLAGS = $(LFLAGS_D_DEXE)
LIBS = $(LIBS_D_DEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_DEXE)
LFLAGS = $(LFLAGS_R_DEXE)
LIBS = $(LIBS_R_DEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = ACCESS4.SBR \
		DETDRIV.SBR


ACCESS4_DEP = c:\code\oma4000\inc\driverrs.h \
	c:\code\oma4000\inc\access4.h \
	c:\code\oma4000\inc\primtype.h \
	c:\code\oma4000\inc\oma4scan.h \
	c:\code\oma4000\inc\monitor.h \
	c:\code\oma4000\inc\counters.h \
	c:\code\oma4000\inc\detdriv.h \
	c:\code\oma4000\inc\asicodes.h \
	c:\code\oma4000\inc\cache386.h \
	c:\code\oma4000\inc\cntrloma.h \
	c:\code\oma4000\inc\mapphys.h \
	c:\code\oma4000\inc\himem.h


DETDRIV_DEP = c:\code\oma4000\inc\detdriv.h \
	c:\code\oma4000\inc\primtype.h \
	c:\code\oma4000\inc\oma4scan.h \
	c:\code\oma4000\inc\driverrs.h \
	c:\code\oma4000\inc\counters.h \
	c:\code\oma4000\inc\cntrloma.h \
	c:\code\oma4000\inc\access4.h \
	c:\code\oma4000\inc\loadoma.h


all:	$(PROJ).EXE $(PROJ).BSC

ACCESS4.OBJ:	ACCESS4.C $(ACCESS4_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ACCESS4.C

DETDRIV.OBJ:	DETDRIV.C $(DETDRIV_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c DETDRIV.C

$(PROJ).EXE::	ACCESS4.OBJ DETDRIV.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
ACCESS4.OBJ +
DETDRIV.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\code\msc\lib\+
c:\code\msc\mfc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
