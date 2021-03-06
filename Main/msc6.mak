# FILE : makefile    THIS IS A POLYMAKE MAKE FILE.

# generate oma4000.exe using local files only. No network logfiles, just
#   use local sources.

# options : MODE=RELEASE builds and links with full optimization.
#           MODE=DEBUG   builds modules in DEBUG_LIST with debugging info,
#                        links with CODEVIEW option for debugging
# if MODE is undefined, MODE=RELEASE is used.

# dependencies are specified in the included file oma4000.dep.
# makedeps should be run on this file if any dependencies change

#define all the directories used in oma4dirs.pmi

!INCLUDE oma4dirs.pmi

#.PATH.h   =$(in);$(formin)
#.PATH.inc =$(in);$(gssinc);$(d16inc)
#.PATH.c   =$(mai);$(ma);$(form)
#.PATH.asm =$(mai)
#.PATH.exe =$(ru)
#.PATH.exp =$(ru)
#.PATH.rmx =$(ru)
#.PATH.map =$(ru)
#.PATH.obj =$(ob)
#.PATH.tbl =$(ma)
#.PATH.grm =$(ma)

#.SOURCE $(lib16m) msc601.lib
#.SOURCE $(gd)     msccgira.lib

INCLUDE=$(in);$(formin);$(gssinc);$(d16m);$(d16inc);$(c6inc);$(gp)
CL=$(always_flags)
LINK=/NOD /NOLOGO
ML=$(masm_flags)
LIB=$(libc)


FORM_OBJS= barmenu.obj  filemsg.obj  fltfld.obj   forms.obj    \
           formwind.obj frmatfld.obj hxintfld.obj intfld.obj   \
           keyibmpc.obj mousefrm.obj mousegss.obj scfltfld.obj \
           scrlform.obj scrngss.obj  selctfld.obj tglfld.obj   \
           unintfld.obj

# everything in \main subdirectory

MAIN_OBJS= access4.obj  autopeak.obj  autoscal.obj  backgrnd.obj \
           basepath.obj baslnsub.obj  cache386.obj  calib.obj    \
           change.obj   cntrloma.obj  config.obj    convert.obj  \
           coolstat.obj crventry.obj  crvhdr35.obj  cursor.obj   \
           curvbufr.obj curvdraw.obj  curvedir.obj  detdriv.obj  \
           detinfo.obj  device.obj    di_util.obj   doplot.obj   \
           dosshell.obj fcolor.obj    filedos.obj   fileform.obj \
           fkeyfunc.obj formtabs.obj  gpibcom.obj   grafmous.obj \
           graphops.obj hline.obj     int24.obj     lineplot.obj \
           live.obj     livedisk.obj  loadoma.obj   macrofrm.obj \
           malloc16.obj d16mphys.obj  mathform.obj  mcibpar.obj  \
           multi.obj    oma4000.obj   omaerror.obj  omaform1.obj \
           omamenu.obj  omameth.obj   omazoom.obj   pa.obj       \
           plot.obj     plotbox.obj   pltsetup.obj  points.obj   \
           rapset.obj   runforms.obj  runsetup.obj  scanset.obj  \
           spgraph.obj  spline-3.obj  splitfrm.obj  statform.obj \
           symbol.obj   tagcurve.obj  tempdata.obj  vdata.obj    \
           wintags.obj  ycalib.obj    oma4tiff.obj  cmdtbl.obj

# for GRAPHICAL scan setup, add scangraf.obj, cursvbar.obj, grouplst.obj
#           scangraf.obj cursvbar.obj grouplst.obj

MAC_OBJS=  macparse.obj macsymbl.obj  maccomp.obj  macruntm.obj \
           macrores.obj macres2.obj   macnres.obj  macnres2.obj \
           macnres3.obj macrecor.obj  mdatstak.obj

EXTRA_OBJS = proclist.obj formlist.obj macskel.obj

SAFE_LIST =  formwind

#            intfld fltfld scfltfld hxintfld unintfld tglfld frmatfld \
#            selctfld mousefrm keyibmpc filemsg barmenu

# sources with functions too big for the C 6.00A compiler to optimize,
# use the C 6.00AX compiler instead, unless debugging.

BIG_LIST = hline oma4000 grafmous tempdata change calib \
           access4 autoscal macparse maccomp macres2 macnres2 macnres3 \
           curvdraw formwind

saved_errors =errs.old

error_file =errs

# -DXSTAT causes PRIVATE static functions to become public for debugging
# for C 6.0
# NOTE : always_flags is used to set CL environment variable, see .PROLOG
always_flags =-nologo -c -AL -G2s -Gt -FPi87 -W3 -DUSE_D16M -DCOMPILE_4000

cl_flags   =-Zp -Oaxzp -Fo$*
safe_flags =-Zp -Oscegilprz -Fo$*

!if "$(MODE)"=="DEBUG"
   cl_flags   += -DXSTAT
   safe_flags += -DXSTAT
   debugLink   = /CO
   debug_mkpm  = -L
!endif

debug_flags =-Zip -Od -DXSTAT -Fo$*

# flags for version 6.00AX compiler to handle large functions
big_flags   =-EM $(cl_flags)

.c.obj :
   !if [grep($@.obj, $(FORM_OBJS))]
      @source=$(form)\$@.c
   !elseif [grep($@.obj, $(MAC_OBJS))]
      @source=$(ma)\$@.c
   !elseif [grep($@.obj, $(EXTRA_OBJS))]
      @source=$(ma)\$@.c
   !else
      @source=$(mai)\$@].c
   !endif
# precedence order is DEBUG_LIST, SAFE_LIST, BIG_LIST
   !if [grep( $@], $(DEBUG_LIST) )]
      @echo debg compile $(source)
      @$(msc)\bin\cl $(debug_flags) $(source) >> $(error_file)
   !elseif [grep($@, $(SAFE_LIST) )]
      @echo safe compile $(source)
      @$(msc)\bin\cl $(safe_flags) $(source) >> $(error_file)
   !elseif [grep($@, $(BIG_LIST) )]
      @echo big compile $(source)
      @echo big compile $(source) >> $(error_file)
      @$(msc)\bin\cl $(big_flags) $(source) >> $(error_file)
   !else
      @echo optm compile $(source)
      @$(msc)\bin\cl $(cl_flags) $(source) >> $(error_file)
   !endif

# flags for Microsoft assembler version 6.0
masm60_flags =/Sl79 /Zim /c /Cp /Fl /nologo

# flags common to both version 5.1 and 6.0, used for %setenv in .PROLOG
masm_flags =/W2

.asm.obj :
   !if [grep( $@, $(DEBUG_LIST))]
      @echo debug assemble $<
      @echo debug assemble $< >> $(error_file)
      @$(masmdir)\ml $(masm60_flags) /Zi /Fo$* $< ; 2>>$(error_file)
   !else
      @echo assemble $<
      @$(masmdir)\ml $(masm60_flags) /Fo$* $< ; 2>>$(error_file)
   !endif
   @del $@.lst


#---------------------------------------------------------------------------

omabuild : INIT macprodx.h $(ma)\macparse.c $(ru)\oma4000.fld \
           $(ru)\oma4000.frm \
           $(ru)\oma4000.mac $(ru)\oma4000.exe

# save the previous error file
INIT :
   @copy $(error_file) $(saved_errors) > NUL
   @echo use $(MAKEFILE) in mode $(MODE) to build oma4000 > $(error_file)
   @echo build oma4000 using $(MAKEFILE), mode = $(MODE)

DEINIT :
   @echo end polymake build of oma4000 >> $(error_file)
   @dir $(ru)\oma4000.exe |find /I "oma4000  exe" >> errs
   @tail errs

$(ru)\oma4000.frm $(ru)\oma4000.fld : $(ru)\formlist.exe
   @echo run formlist.exe to create oma4000.frm and oma4000.fld >> $(error_file)
   cd $(ru)
   formlist.exe
   cd $(CWD)

$(ru)\formlist.exe : formlist.obj
   @echo linking formlist.exe >> $(error_file)
   @$(msc)\bin\link $(ob)\formlist.obj, $(ru)\formlist, $(ru)\formlist, llibc7, \
   NUL; >> $(error_file)

$(ru)\oma4000.mac : $(ru)\proclist.exe
   @echo run proclist.exe to create oma4000.mac >> $(error_file)
   $(ru)\proclist.exe $(ru)\oma4000.mac

$(ru)\proclist.exe : proclist.obj
    @echo linking proclist.exe >> $(error_file)
    @$(msc)\bin\link $(ob)\proclist.obj, $(ru)\proclist, $(ru)\proclist, llibc7, \
    NUL; >> $(error_file)

# macruntm.h really a dependent file ??
$(ma)\macparse.c: macparse.tbl $(ma)\macskel.c $(in)\macruntm.h
    @echo create $@ using qparser
    @echo use qparser lr1p to create macparse.c >> $(error_file)
    $(qp)\lr1p $(ma)\macparse.tbl -d -mc -s $(ma)\macskel.c -p $(ma)\macparse.c

macprodx.h: {$(in)}prdxskel.h macparse.tbl
    @echo create $@ using qparser
    @echo use qparser lr1p to create macprodx.h >> $(error_file)
    $(qp)\lr1p $(ma)\macparse.tbl -mc -s $(in)\prdxskel.h -p $(in)\macprodx.h

macparse.tbl : {$(ma)}macparse.grm
    @echo create $@ using qparser
    @echo use qparser lr1 to create macparse.tbl
    $(qp)\lr1 $(ma)\macparse.grm -t $(ma)\macparse.tbl -l $(ma)\macparse.err

#===========================================================================

# only link for codeview if there is something to debug

$(ru)\oma4000.exe: $(MAIN_OBJS) $(FORM_OBJS) $(MAC_OBJS)
   !if "$(MODE)" == "DEBUG"
      @echo debug link... ($(debugLink))
      @echo Debug link to create oma4000.exe >> $(error_file)
   !else
      @echo Linking...
      @echo link to create oma4000.exe >> $(error_file)
   !endif

# !!! gpib.com won't work with /F and /PACKC
    @$(msc)\bin\link @<<
$(debugLink) /NOLOGO /ST:0x4000 /NOE /MAP /NOI /SEGMENTS:240 +
$(MAIN_OBJS: =+
)+
$(FORM_OBJS: =+
)+
$(MAC_OBJS: =+
)
$(ru)\oma4000.exe
$(ru)\oma4000.map
$(lib16m)\msc60l.lib $(gd)\msccgira.lib llibc7.lib
NUL
<<
   @echo dos16m makepm to create oma4000.exp >> $(error_file)
   $(d16m)\makepm $(ru)\oma4000 $(debug_mkpm) -DPMI -Q
   @echo Splicing to Loader...
   @echo dos16m splice >> $(error_file)
   $(d16m)\splice $(ru)\oma4000 $(ru)\oma4000 $(d16m)\loader.exe
   @echo Deleting banner...
   @echo dos16m banner delete on oma4000.exe >> $(error_file)
   $(d16m)\banner $(ru)\oma4000.exe OFF

# .asm dependencies done manually
# .asm files with no includes commented out, still used though

# int24.obj   :

vdata.obj     : omatyp.inc

# convert.obj :

plot.obj      : omatyp.inc plot.inc

# mcibpar.obj :

rmfcall.obj   : dos16m.inc

pa.obj : pa.asm

# include all the .obj : .h dependencies from makedeps

!INCLUDE oma4000.dep

