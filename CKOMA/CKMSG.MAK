#make description file for the oma4 asic compiler

CC=cl
cx=\code\c\cax\cl
obj=..\obj\mccomp
run=.
src=.
ma=..\main
inc=..\inc
d16m=\code\dos16m
errs= errs

cflags=-c -nologo -AL -W3 -Lr -Ozax -Grs2 -Fo$*.obj
ln=/exe /far /nologo /packcode /stack:0x2000 /noe /noi /MAP
!IFDEF DEBUG
cflags=-c -nologo -AL -W4 -Lr -Od -Zi -Grs2 -Fo$*.obj
ln=/co /nologo /noe /noi /MAP
!ENDIF
!IFDEF USE_D16M
# dos16m doesn't like -Gr, /exe, or /far
cflags=-c -nologo -AL -W3 -Lr -Ozax -Gs2 -Fo$*.obj
ln=/co /nologo /stack:0x2000 /noe /noi /MAP
!ENDIF

!IFDEF USE_D16M
libs = $(d16m)\msc60l llibcer
!ELSE
libs = llibcer
!ENDIF

{$(src)}.c{$(obj)}.obj :
      @echo  Compiling $<
      @$(CC) $(cflags) $< >> errs

{$(ma)}.c{$(obj)}.obj :
      @echo  Compiling $<
      @$(CC) $(cflags) $< >> errs

objs= $(obj)\ckmsg.obj\
      $(obj)\himem.obj\
!IFDEF USE_D16M
      $(obj)\malloc16.obj\
!ENDIF
      $(obj)\cache386.obj

$(run)\ckmsg.exe : $(objs)
   echo linking...
   c:\code\c\link @<<temp.lnk
$(ln) +
$(objs: =+
)
$(run)\ckmsg
$(run)\ckmsg
$(libs)
NUL
<<nokeep
!IFDEF USE_D16M
  $(d16m)\makepm $(run)\ckmsg -DPMI -L -INIT00 -Q
  $(d16m)\splice $(run)\ckmsg $(run)\ckmsg $(d16m)\loader.exe
  $(d16m)\banner $(run)\ckmsg.exe OFF
!ENDIF

update :
   @tm /l /log
   @tm /l /log >errs

$(obj)\ckmsg.obj    : $(src)\ckmsg.c
$(obj)\himem.obj    : $(ma)\himem.c  $(inc)\himem.h
$(obj)\malloc16.obj : $(ma)\malloc16.c  $(d16m)\source\dos16.h
$(obj)\cache386.obj : $(ma)\cache386.c  $(inc)\cache386.h

