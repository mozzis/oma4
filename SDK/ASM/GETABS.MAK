#
#**     makefile for OS/2 simple sample device driver and test program
#

LINK=link
LFLAGS=/nod /noi /m /l
cflags=-c -AL -Zi -G2s -FPi87 -Od -W3 -Fo$*
aflags=/ML /ZI /W2 /L /P
LIBS=os2.lib
ob=..\obj\#
ru=..\run\#
sr=..\src\#
in=..\inc\#

#       Production rules

ALL : $(ru)getabs.exe $(ru)oma4drv.sys


$(ob)oma4drv.obj: $(sr)oma4drv.asm $(sr)smtools.asm $(sr)dbtools.asm \
                  $(sr)omaioctl.asm $(sr)omahdwr.asm
      masm $(aflags) $(sr)oma4drv.asm $@ $(sr)oma4drv.lst;

$(ru)oma4drv.sys: $(ob)oma4drv.obj
        $(LINK) $(LFLAGS) $(ob)oma4drv.obj,$(ru)oma4drv.sys,,$(LIBS),$(sr)oma4drv.def;

$(ob)cdbtools.obj: $(sr)cdbtools.asm
      masm $(aflags) $? $@ $(sr)cdbtools.lst

$(ob)getabs.obj: $(sr)getabs.c $(in)oma4drv.h
      cl $(cflags) $(sr)getabs.c

$(ru)getabs.exe: $(ob)getabs.obj $(ob)cdbtools.obj
     $(LINK) /CO $(LFLAGS) $(ob)getabs.obj $(ob)cdbtools.obj,$(ru)getabs.exe,,llibc7p os2;

