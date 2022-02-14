# makfile for nmake to make file system test scaffold
# set the following macros to match your source, object, and exec directories

src=.
obj=..\obj\filetest
run=..\filetest

# set the next two macros to the path+names of your assembler and compiler

AS=c:\code\asm\ml.exe
CC=c:\code\c\bin\cl.exe

# flags for MASM 6.0 and C 6.0

masm_flags= /nologo /Sl79 /Zim /c /Cp /Fl /W2
!IFDEF DEBUG
cflags=  -nologo -c -AL -Zip -G2st -Od -W3 -FPi87 -Fo$*
lflags=  /CO
!ELSE
cflags=  -nologo -c -AL -Zp -G2st -Ogwie -W3 -FPi87 -Fo$*
!ENDIF

# Suggest no spaces before line coninuation char '\'

objs= $(obj)\filetest.obj\
      $(obj)\testerr.obj\
      $(obj)\di_util.obj\
      $(obj)\tempdata.obj\
      $(obj)\filedos.obj\
      $(obj)\curvedir.obj\
      $(obj)\curvbufr.obj\
      $(obj)\points.obj\
      $(obj)\crvhdr35.obj\
      $(obj)\convert.obj\
      $(obj)\omameth.obj\
      $(obj)\crventry.obj\
      $(obj)\detinfo.obj\
      $(obj)\basepath.obj\
      $(obj)\vdata.obj

# carriage return and ')' at next line start breaks up object list for linker

$(run)\filetest.exe : $(objs)
      @echo  Linking...
      @link @<<temp.lnk >> errs
$(lflags) +
$(objs:^ =+
) 
$*
$*
llibc7r /NOD /NOE
nul
<<NOKEEP

ALL :
    @tm /l /LOG
    @tm /l /LOG >errs

# inference rules for 'c' and 'asm' source files

{$(src)}.c{$(obj)}.obj :
      @echo  Compiling $<
      @$(CC) $(cflags) $< >> errs

{$(src)}.asm{$(obj)}.obj :
      @echo  Assembling $<
      @$(AS) $(masm_flags) /Fo$* $< ; >>errs

# dependencies (should also add headers)

$(obj)\filetest.obj : $(src)\filetest.c
$(obj)\testerr.obj  : $(src)\testerr.c
$(obj)\di_util.obj  : $(src)\di_util.c
$(obj)\tempdata.obj : $(src)\tempdata.c
$(obj)\filedos.obj  : $(src)\filedos.c
$(obj)\curvedir.obj : $(src)\curvedir.c
$(obj)\curvbufr.obj : $(src)\curvbufr.c
$(obj)\points.obj   : $(src)\points.c
$(obj)\crvhdr35.obj : $(src)\crvhdr35.c
$(obj)\omameth.obj  : $(src)\omameth.c
$(obj)\crventry.obj : $(src)\crventry.c
$(obj)\detinfo.obj  : $(src)\detinfo.c
$(obj)\basepath.obj : $(src)\basepath.c
$(obj)\convert.obj  : $(src)\convert.asm
$(obj)\vdata.obj    : $(src)\vdata.asm

