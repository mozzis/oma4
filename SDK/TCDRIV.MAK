#Make file for OMA4 driver library - TURBOC version
#Syntax is for Turbo Make - "make -ftcdriv.mak"

# You may choose MODEL=c for COMPACT model if your code is less than 64K
# (Still allows > 64K for Data) & this will allow DPMI support

.swap

!if !$d(MODEL)
MODEL=l
!endif

# !!! Change these macro definitions to match your structure !!!
tools  =  $(TOOLS)             # take tools directory from environment
in     =  ..\inc               # include files sub-directory
src    =  ..\main              # common source files (from oma4000)
loc    =  .                    # local source files
od     =  ..\obj\tcdriv        # objects and libraries sub-directory
ru     =  ..\run               # target executable sub-directory
bcb    =  $(tools)\bc\bin      # compiler executable sub-directory
bci    =  $(tools)\bc\include  # compiler include files sub-directory
bcl    =  $(tools)\bc\lib      # compiler libraries sub-directory

# All of the compiler and linker flags are defined here

!if $d(DEBUG)
cflags = -c -m$(MODEL) -w-stu -2 -K -v -N- -n$(od) -I$(bci);$(in) -DDEBUG

!else
cflags = -c -m$(MODEL) -2 -w-stu -K -O -Z -N- -n$(od) -I$(bci);$(in)

!endif

{$(loc)}.c{$(od)}.obj :
  @$(bcb)\bcc $(cflags) $<

{$(src)}.c{$(od)}.obj :
  @$(bcb)\bcc $(cflags) $<

ALL : UPDATE $(od)\tcdriv.lib

UPDATE :
  @echo Begin build...

OBJS=\
 $(od)\access4.obj\
 $(od)\cntrloma.obj\
 $(od)\cmdtbl.obj\
 $(od)\detdriv.obj\
 $(od)\loadoma.obj\
 $(od)\himem.obj\
 $(od)\cache386.obj

$(od)\tcdriv.lib: $(OBJS) tcdriv.mak
    @$(bcb)\tlib $< @&&<
-+$(OBJS:  = -+ )
$*
$*
<

$(od)\access4.obj  : $(src)\access4.c  $(in)\access4.h  $(in)\primtypes.h
$(od)\cntrloma.obj : $(src)\cntrloma.c $(in)\cntrloma.h $(in)\primtypes.h
$(od)\cmdtbla.obj  : $(src)\cmdtbl.c   $(in)\cmdtbl.h   $(in)\primtypes.h
$(od)\detdriv.obj  : $(src)\detdriv.c  $(in)\detdriv.h  $(in)\primtypes.h
$(od)\loadoma.obj  : $(src)\loadoma.c  $(in)\loadoma.h  $(in)\primtypes.h
$(od)\himem.obj    : $(loc)\himem.c    $(in)\himem.h    $(in)\primtypes.h
$(od)\cache386.obj : $(src)\cache386.c $(in)\cache386.h $(in)\primtypes.h
$(od)\oma4phys.obj : $(loc)\oma4phys.c $(in)\oma4phys.h $(in)\primtypes.h

#  $(bcb)\bcc -c -ms -2 -K -O -Z -N- -n$(od) -I$(bci);$(in) -Tmx -B -E$(bcb)\tasm.exe $(loc)\$&.c
