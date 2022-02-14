#---------------------------------------------------------------------------#
############# Watcom Make (WMAKE) file for OMA4 SDK 32-bit version ##########
#---------------------------------------------------------------------------#
# WMAKE Options follow.                 
/e      # Erase target file on error.
/h
/l

# WMAKE Macros follow

#---------------------------------------------------------------------------#
############################# file locations ################################
#---------------------------------------------------------------------------#
TOOLS=$(%TOOLS)
SRC=..\main
OBJ=..\obj\wcdriv
RUN=..\run
PHARLAP=phar386
INC=$(TOOLS)\watcom\h;..\inc;$(TOOLS)\$(PHARLAP)\inc
LIB=$(TOOLS)\WATCOM\LIB386;$(TOOLS)\WATCOM\LIB386\DOS;$(TOOLS)\$(PHARLAP)\libs
ERRFILE=CON

# add the extension "lnk" to those make already knows

.EXTENSIONS : .lnk

# tell where to find files with given extensions

.exp : $(RUN)
.exe : $(RUN)
.h   : $(INC)
.obj : $(OBJ)
.c   : $(SRC)
.lib : $(OBJ)

#---------------------------------------------------------------------------#
########################## options for Watcom C compiler ####################
#---------------------------------------------------------------------------#
# /d2        = include full symbolic debugging info
# /ez        = generate Phar Lap Easy OMF-386 object file
# /zp1       = pack structures on byte boundaries (default)
# /fpi87, /7 = generate inline 80x87 code
# /s         = remove stack overflow checks
# /zq        = operate quietly
# /i         = set include path
# /fo        = set object output path&name oilrt
# /ox        = 'i'nline funcs, 'l'oop opts, o'r'der for 486 pipe, no stack ck
#COPTS=/ez /zp1 /s /d0 /7 /zq /i=$(INC) /oilrt /fo=$(OBJ)\$^@
!ifdef DEBUG
COPTS=/ez /zp1 /s /d2 /zq /i=$(INC) /od /fo=$(OBJ)\$^@
!else
COPTS=/ez /zp1 /s /d0 /zq /i=$(INC) /ox /fo=$(OBJ)\$^@
!endif

#---------------------------------------------------------------------------#
############### Optional stuff to do before and after build #################
#---------------------------------------------------------------------------#
.BEFORE
  @%create $(ERRFILE)
  @set INCLUDE=$(INC)
  @set LIB=$(LIB)
  @set COPTS=$(COPTS)
  @echo Building testdriv...

.AFTER
  @if exist testdriv.lnk del testdriv.lnk

.ERROR
  @echo (Oops!)
  @head $(ERRFILE) 
  @tail $(ERRFILE) 

#---------------------------------------------------------------------------#
################ how to generate a .obj file from a .c file #################
#---------------------------------------------------------------------------#
.c.obj :
  @ECHO $[.
  @$(TOOLS)\WATCOM\BIN\WCC386P @COPTS $[@ >> $(ERRFILE)

.exp.exe :
  @echo Binding...
  @$(TOOLS)\$(PHARLAP)\bin\bind386 $(TOOLS)\$(PHARLAP)\rtk\run386b $(RUN)\$]. >> $(ERRFILE)

.lnk.exp
  @echo Linking...
  @$(TOOLS)\watcom\bin\wlinkp @$]. >> $(ERRFILE)

#---------------------------------------------------------------------------#
##################### list of object modules to generate ####################
#---------------------------------------------------------------------------#
OBJS=testdriv.obj

#---------------------------------------------------------------------------#
############################ dependency rules ###############################
#---------------------------------------------------------------------------#
testdriv.obj : testdriv.c detdriv.h

#---------------------------------------------------------------------------#
######################## create the linker response file ####################
#---------------------------------------------------------------------------#
testdriv.lnk : $(OBJS)
  @%create $^@
  @%append $^@ NAME $(RUN)\$^&.EXP
  @%append $^@ PATH $(OBJ)
  @%append $^@ FORM PHAR EXT
  @%append $^@ DEBUG all
  @%append $^@ LIBPATH $(LIB)
  @for %i in ($(OBJS)) do @%append $^@ FILE %i
  @%append $^@ LIBRARY $(OBJ)\WCDRIV.LIB
  @%append $^@ OPTION STACK=50000, QUIET

testdriv.exp : $(OBJS) wcdriv.lib

testdriv.exe : testdriv.exp
