#Makefile for generating OMA4 driver test program.  Syntax is for NMAKE.

#CDIR environment variable defines where C compiler lives
CC = $(CDIR)\bin\cl
ru = ..\run
in = ..\inc
od = ..\obj\mcdriv
INCLUDE=$(CDIR)\include

!ifdef DEBUG
cflags = -c -nologo -AL -FPi87 -Zip -G2s -Od /I$(INCLUDE) /I$(in)
lflags = /CO /NOLOGO /ST:0x1000 /NOD /MAP
!else
cflags = -c -EM -nologo -AL -Zp -G2s -Oazx /I$(INCLUDE) /I$(in)
lflags = /NOLOGO /EXE /PACKC /ST:0x1000 /NOD /MAP
#lflags = /NOLOGO /EXE /PACKC /FAR /ST:0x1000 /NOD /MAP
!endif

{.}.c{$(od)}.obj:
  $(CC) $(cflags) -Fo$*.obj $<

ALL : UPDATE $(ru)\fabs.exe

UPDATE :
  @echo Include is $(INCLUDE)

$(ru)\fabs.exe : $(od)\fabsmain.obj $(od)\fabsdata.obj $(od)\fabsgraf.obj\
                 $(od)\mcdriv.lib
    @echo Linking...
    $(CDIR)\bin\link @<<
$(lflags) +
$**
$*.exe
$*.map
$(od)\mcdriv.lib $(CDIR)\lib\llibc7.lib $(CDIR)\lib\graphics.lib
NUL
<<

$(od)\fabsmain.obj: .\fabsmain.c fabsgraf.h fabsdata.h
$(od)\fabsgraf.obj: .\fabsgraf.c .\grafmc.c fabsgraf.h
$(od)\fabsdata.obj: .\fabsdata.c fabsdata.h $(in)\detdriv.h

