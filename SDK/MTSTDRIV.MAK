#Makefile for generating OMA4 driver test program.  Syntax is for NMAKE.

#CDIR environment variable defines where C compiler lives
CC = $(CDIR)\BIN\cl
INCLUDE=$(CDIR)\INC;$(in)
LIB=$(CDIR)\LIB
ru = ..\run
in = ..\inc
od = ..\obj\mcdriv

!ifdef DEBUG
cflags = -c -nologo -AL -FPi87 -Zip -G2s -Od /I$(CDIR)\INC /I..\inc
lflags = /CO /NOLOGO /ST:0x1000 /NOD /MAP
!else
cflags = -c -nologo -AL -Zp -G2s -Oazx /I$(CDIR)\INC /I..\inc
lflags = /NOLOGO /EXE /PACKC /FAR /ST:0x1000 /NOD /MAP 
!endif

{.}.c{$(od)}.obj:
     @$(CC) $(cflags) -Fo$*.obj $<

{$(od)}.obj{$(ru)}.exe:
    @echo Linking...
    $(CDIR)\BIN\link @<<
$(lflags) +
$<
$*.exe
$*.map
$(od)\mcdriv.lib $(CDIR)\LIB\llibc7.lib
NUL
<<

$(ru)\testdriv.exe : $(od)\testdriv.obj $(od)\mcdriv.lib

$(od)\testdriv.obj: .\testdriv.c $(in)\detdriv.h

