.AUTODEPEND

#		*Translator Definitions*
CC = bcc +OMA4FF.CFG
TASM = TASM
TLIB = tlib
TLINK = tlink
OBJPATH=..\obj\tcdriv
LIBPATH = C:\TOOLS\BC\LIB
INCLUDEPATH = C:\TOOLS\BC\INC;..\INC


#		*Implicit Rules*
.c.obj:
  $(CC) -c {$< }

.cpp.obj:
  $(CC) -c {$< }

#		*List Macros*


EXE_dependencies =  \
 oma4ff.obj \
 image.obj \
 vesadrvr.obj \
 ..\obj\tcdriv\tcdriv.lib

#		*Explicit Rules*
..\run\oma4ff.exe: oma4ff.cfg $(EXE_dependencies)
  $(TLINK) /v/x/c/L$(LIBPATH) @&&|
c0l.obj+
$(OBJPATH)\oma4ff.obj+
$(OBJPATH)\image.obj+
$(OBJPATH)\vesadrvr.obj
..\run\oma4ff
nul
..\obj\tcdriv\tcdriv.lib+
fp87.lib+
mathl.lib+
cl.lib
|


#		*Individual File Dependencies*
$(OBJPATH)\oma4ff.obj: oma4ff.cfg oma4ff.c 

$(OBJPATH)\image.obj: oma4ff.cfg image.c 

$(OBJPATH)\vesadrvr.obj: oma4ff.cfg vesadrvr.c 

#		*Compiler Configuration File*
oma4ff.cfg: oma4ff.mak
  copy &&|
-ml
-3
-f287
-v
-G
-O
-Og
-Oe
-Om
-Ov
-Ol
-Ob
-Op
-Oi
-Z
-k-
-vi-
-wpro
-weas
-wpre
-n$(OBJPATH)
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DTURBOC
| oma4ff.cfg


