.AUTODEPEND

#		*Translator Definitions*
TOOLS=C:\TOOLS
CODE=C:\CODE
OUT=$(CODE)\OMA4\OBJ\TCDRIV
BCDIR=BC\BIN
CC = $(TOOLS)\$(BCDIR)\bcc +FABS.CFG
TASM = $(TOOLS)\$(BCDIR)\TASM
TLIB = $(TOOLS)\$(BCDIR)\tlib
TLINK = $(TOOLS)\$(BCDIR)\tlink
LIBPATH = $(TOOLS)\BC\LIB
INCLUDEPATH=$(TOOLS)\BC\INCLUDE;$(CODE)\OMA4\INC

!if !$d(DEBUG)
CFLAGS = -c -K -O -Z -n$(OUT)
LFLAGS = /m
!else
CFLAGS = -c -K -v -n$(OUT)
LFLAGS = /v
!endif

.SUFFIXES : .obj .exe .c .lib
.path.c = .
.path.obj = $(OUT)

#		*Implicit Rules*

.c.obj:
  @$(CC) $(CFLAGS) {$< }

.cpp.obj:
  @$(CC) $(CFLAGS) {$< }

#		*List Macros*


EXE_dependencies =  \
 $(OUT)\fabsmain.obj \
 $(OUT)\fabsgraf.obj \
 $(OUT)\fabsdata.obj

#		*Explicit Rules*
..\run\fabs.exe: fabs.cfg $(EXE_dependencies)
  @$(TLINK) /ye/x/c/P-/L$(LIBPATH) $(LFLAGS) @&&|
c0l.obj+
$(OUT)\fabsmain.obj+
$(OUT)\fabsgraf.obj+
$(OUT)\fabsdata.obj
..\run\fabs
		# no map file
graphics.lib+
fp87.lib+
mathl.lib+
cl.lib+
..\obj\tcdriv\tcdriv.lib
|


#		*Individual File Dependencies*
$(OUT)\fabsmain.obj: fabs.cfg fabsmain.c fabsgraf.h fabsdata.h

$(OUT)\fabsgraf.obj: fabs.cfg fabsgraf.c grafbc.c fabsgraf.h

$(OUT)\fabsdata.obj: fabs.cfg fabsdata.c fabsdata.h ..\inc\detdriv.h

$(OUT)\fakedriv.obj: fabs.cfg fakedriv.c 

#		*Compiler Configuration File*
fabs.cfg : fabsbc.mak
  @copy &&|
$(CFLAGS)
-ml
-2
-f287
-vi-
-wpro
-w-stu
-weas
-wpre
-I$(INCLUDEPATH)
-L$(LIBPATH)
| fabs.cfg


