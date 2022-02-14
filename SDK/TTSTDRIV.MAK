#make file for testdriv program - TURBOC version
#Syntax is for Turbo Make - "make -fttstdriv.mak"

.swap

# Change these macro definitions to match your structure

tools = $(TOOLS)            # take tools directory from environment
in    = ..\inc              # include files sub-directory
src   = ..\main             # common source files (from oma4000)
loc   = .                   # local source files
od    = ..\obj\tcdriv       # objects and libraries sub-directory
ru    = ..\run              # target executable sub-directory
tcb   = $(tools)\bc\bin     # compiler executable sub-directory
tci   = $(tools)\bc\include # compiler include files sub-directory
tcl   = $(tools)\bc\lib     # compiler libraries sub-directory

!if !$d(DEBUG)
cflags = -c -ml -2 -w-stu -K -O -Z -n$(od) -I$(tci);$(in)
lflags = /m
!else
cflags = -c -ml -2 -w-stu -K -v -n$(od) -I$(tci);$(in)
lflags = /v
!endif

{$(loc)}.c{$(od)}.obj :
  @$(bcb)\bcc $(cflags) $<

{$(src)}.c{$(od)}.obj :
  @$(bcb)\bcc $(cflags) $<

ALL : UPDATE $(ru)\testdriv.exe

UPDATE :
    @echo Begin build...

$(ru)\testdriv.exe: $(od)\testdriv.obj \
                    $(od)\tcdriv.lib
  @echo Linking...
  @$(tcb)\tlink $(lflags)/c/n/L$(tcl);$(od) @&&|
c0l.obj+
$(od)\testdriv.obj
$(ru)\testdriv,$(od)\testdriv
$(od)\tcdriv.lib+
fp87.lib+
mathl.lib+
cl.lib
|

$(od)\testdriv.obj: .\testdriv.c
     @$(tcb)\bcc $(cflags) .\testdriv.c

