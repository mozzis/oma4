#Makefile for generating OMA4 detector driver library.  Syntax is for NMAKE.

#CDIR environment variable defines where C compiler lives
CC = $(CDIR)\BIN\cl
INCLUDE=$(CDIR)\INC;$(in)
LIB = $(CDIR)\LIB
!ifdef DEBUG
cflags = -c -EM -nologo -AL -G2s -FPi87 -Zip -Od /I$(CDIR)\INC /I..\inc
!else
cflags = -c -EM -nologo -AL -G2s -FPi87 -Zip -Oazx /I$(CDIR)\INC /I..\inc
!endif
in = ..\inc
od = ..\obj\mcdriv
src = ..\main
error_file = errs

{$(src)}.c{$(od)}.obj:
     @$(CC) $(cflags) -Fo$*.obj $<

{.}.c{$(od)}.obj:
     @$(CC) $(cflags) -G2s -Fo$*.obj $<

objs= $(od)\cntrloma.obj\
      $(od)\cmdtbl.obj\
      $(od)\access4.obj\
      $(od)\detdriv.obj\
      $(od)\loadoma.obj\
      $(od)\himem.obj\
      $(od)\oma4phys.obj\
      $(od)\cache386.obj

$(od)\mcdriv.lib: $(objs)
    @$(CDIR)\BIN\lib @<<
$(od)\mcdriv.lib&
-+$(objs: =&
-+)
$(od)\mcdriv
$(od)\mcdriv
<<

$(od)\access4.obj  : $(src)\access4.c $(in)\access4.h $(in)\detdriv.h\
                                      $(in)\counters.h $(in)\monitor.h\
                                      $(in)\cntrloma.h $(in)\himem.h
     @$(CC) $(cflags) -Fo$(od)\access4.obj $(src)\access4.c
$(od)\cntrloma.obj : $(src)\cntrloma.c $(in)\access4.h $(in)\counters.h\
                                       $(in)\monitor.h
$(od)\cmdtbl.obj   : $(src)\cmdtbl.c   $(in)\cmdtbl.h $(in)\primtype.h
$(od)\detdriv.obj  : $(src)\detdriv.c  $(in)\detdriv.h $(in)\access4.h\
                                       $(in)\counters.h $(in)\monitor.h\
                                       $(in)\cntrloma.h $(in)\loadoma.h
$(od)\loadoma.obj  : $(src)\loadoma.c  $(in)\loadoma.h $(in)\loadrec.h\
                                       $(in)\himem.h
$(od)\himem.obj    : .\himem.c         $(in)\himem.h $(in)\oma4phys.h
$(od)\oma4phys.obj : .\oma4phys.c      $(in)\oma4phys.h
$(od)\cache386.obj : $(src)\cache386.c $(in)\cache386.h
