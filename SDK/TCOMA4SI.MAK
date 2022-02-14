#Makefile for generating OMA4 detector static interface - TURBOC version.
#Syntax is for Turbo Make - "make -ftcoma4si.mak"

.swap

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
cflags = -c -ml -2 -w-stu -K -v -n$(od) -I$(bci);$(in) -DDEBUG
lflags = /v
!else
cflags = -c -ml -2 -w-stu -K -O -Z -n$(od) -I$(bci);$(in)
lflags = /m
!endif

{$(loc)}.c{$(od)}.obj :
  @$(bcb)\bcc $(cflags) $<

{$(src)}.c{$(od)}.obj :
  @$(bcb)\bcc $(cflags) $<

ALL : UPDATE $(ru)\oma4si.exe

UPDATE :
  @echo Begin build...

$(ru)\oma4si.exe: $(od)\oma4si.obj \
                  $(od)\tcdriv.lib
  @echo Linking...
  @$(bcb)\tlink $(lflags) @&&<
$(bcl)\c0l.obj $(od)\oma4si.obj 
$(ru)\oma4si.exe
$(od)\oma4si.map
$(od)\tcdriv $(bcl)\fp87 $(bcl)\mathl $(bcl)\cl
<

$(od)\oma4si.obj: oma4si.c $(in)\detdriv.h

$(ru)\oma4dpmi.exe : $(od)\oma4dpmi.obj
  @$(bcb)\tlink $(lflags) @&&<
$(bcl)\c0l.obj $(od)\oma4dpmi.obj 
$(ru)\oma4dpmi.exe
$(od)\oma4dpmi.map
$(bcl)\fp87 $(bcl)\mathl $(bcl)\cl
<

$(od)\oma4dpmi.obj : oma4dpmi.c
  @$(bcb)\bcc $(cflags) -B -E$(bcb)\tasm.exe -DSTANDALONE oma4dpmi.c
