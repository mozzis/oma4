.AUTODEPEND

#               *Translator Definitions*
TOOLS=C:\CODE\BC\BIN
PTOOLS=C:\CODE\PHAR286\BIN
CC = $(TOOLS)\bcc
TASM = $(TOOLS)\TASM
TLIB = $(TOOLS)\tlib
TLINK = $(TOOLS)\tlink
TLIBPATH = C:\CODE\BC\LIB
PLIBPATH = C:\CODE\PHAR286\BC3\LIB
INCLUDEPATH = C:\CODE\BC\INCLUDE


#               *Implicit Rules*
.c.obj:
  $(CC) @&&|
-c
-ml
-2
-DDOSX286 
-h 
-I$(INCLUDEPATH)
$<
| 

#               *List Macros*

EXE_dependencies =  \
 fljmain.obj \
 fljgraf.obj \
 fljdata.obj \
 fakedriv.obj

#               *Explicit Rules*
fljtest.exe: $(EXE_dependencies)
  @$(TLINK) @&&|
$(PLIBPATH)\c0pl.obj+
fljmain.obj+
fljgraf.obj+
fljdata.obj+
fakedriv.obj
.\flj286 /c/C
nul
$(TLIBPATH)\emu.lib+
$(TLIBPATH)\mathl.lib+
$(PLIBPATH)\phapi+
$(PLIBPATH)\bcl286.lib+
$(PLIBPATH)\graph286.lib
protmode.def
|

#               *Individual File Dependencies*
fljmain.obj: fljmain.c 

fljgraf.obj: fljgraf.c 

fljdata.obj: fljdata.c 

fakedriv.obj: fakedriv.c 



