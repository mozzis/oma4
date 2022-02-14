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
# WC32DPMI will be compiled with no optimization no matter what!
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
  @echo Building library...

.AFTER
  @if exist wcdriv.lnk @del wcdriv.lnk

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

.lnk.lib
  @echo Combining objects into library...
  @$(TOOLS)\watcom\binb\wlib $^@ @$]. >> $(ERRFILE)

ALL : WCDRIV.LIB

#---------------------------------------------------------------------------#
##################### list of object modules to generate ####################
#---------------------------------------------------------------------------#
OBJS= detdriv.obj &
cmdtbl.obj &
cntrloma.obj &
access4.obj &
loadoma.obj &
wc32phys.obj

#---------------------------------------------------------------------------#
############################ dependency rules ###############################
#---------------------------------------------------------------------------#
access4.obj  : access4.c  access4.h  cntrloma.h  primtypes.h
cntrloma.obj : cntrloma.c cntrloma.h primtypes.h
cmdtbl.obj   : cmdtbl.c   cmdtbl.h   primtypes.h
detdriv.obj  : detdriv.c  detdriv.h  detsetup.h primtypes.h cntrloma.h
loadoma.obj  : loadoma.c  loadoma.h  primtypes.h cntrloma.h

wc32phys.obj : wc32phys.c wc32phys.h
  @$(TOOLS)\WATCOM\BIN\WCC386P @COPTS /od $[@ >> $(ERRFILE)

#---------------------------------------------------------------------------#
######################## create the linker response file ####################
#---------------------------------------------------------------------------#
wcdriv.lnk : $(OBJS)
  @%create $^@
  @%append $^@ /q /d=$(OBJ)
  @for %i in ($(OBJS)) do @%append $^@ +-$(OBJ)\%i

$(OBJ)\wcdriv.lib : $(OBJS)

