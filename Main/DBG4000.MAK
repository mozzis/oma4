# FILE : dbg4000.mak    THIS IS A POLYMAKE MAKE FILE.

# generate oma4000.exe using local files only. No network logfiles, just
#   use local sources.

# options : MODE=LOC,    for local build only, ignores the network.
#           MODE=NETREP, will check pvcs logfiles and report any out of date
#                        local files, but no download, uses local files
#                        anyway.
#  if MODE is undefined, MODE=LOC is used.

# for DOS only, NOT for OS/2.  Does NOT generate mapphys.exe

# dependencies are specified in the included file oma4000.dep.
#    makedeps should be run on this file if any dependencies change

#.KEEP_LIS

#define all the directories used in oma4dirs.pmi

.INCLUDE dbg4000.pmi

.PATH.h   =$(in);$(formin)
.PATH.inc =$(in)
.PATH.c   =$(mai);$(ma);$(form)
.PATH.asm =$(mai)
.PATH.exe =$(ru)
.PATH.exp =$(ru)
.PATH.rmx =$(ru)
.PATH.map =$(ru)
.PATH.obj =$(ob)
.PATH.tbl =$(ma)
.PATH.grm =$(ma)

.SOURCE $(gd)  msccgir.lib

FORM_OBJS= forms.obj scrlform.obj intfld.obj fltfld.obj scfltfld.obj \
           hxintfld.obj unintfld.obj tglfld.obj frmatfld.obj selctfld.obj \
           mousefrm.obj keyibmpc.obj formwind.obj filemsg.obj barmenu.obj \
           scrngss.obj mousegss.obj

# everything in \main subdirectory

MAIN_OBJS= coolstat.obj doplot.obj symbol.obj hline.obj curvdraw.obj \
           cursor.obj multi.obj live.obj livedisk.obj gpibcom.obj \
           oma4000.obj detdriv.obj fkeyfunc.obj graphops.obj grafmous.obj \
           curvedir.obj filedos.obj tempdata.obj di_util.obj \
           omaerror.obj int24.obj change.obj mathform.obj runsetup.obj \
           omaform1.obj formtabs.obj backgrnd.obj scrlltst.obj calib.obj \
           scanset.obj omazoom.obj omameth.obj config.obj macrofrm.obj \
           baslnsub.obj spline-3.obj plotbox.obj fcolor.obj device.obj \
           autopeak.obj wintags.obj tagcurve.obj access4.obj convert.obj \
           vdata.obj plot.obj himem.obj \
           runforms.obj spgraph.obj pltsetup.obj omamenu.obj points.obj \
           crvhdr35.obj curvbufr.obj cache386.obj \
           detinfo.obj  crventry.obj basepath.obj autoscal.obj lineplot.obj

# for GRAPHICAL scan setup, add scangraf.obj, cursvbar.obj, grouplst.obj
#           scangraf.obj cursvbar.obj grouplst.obj

MAC_OBJS=  macparse.obj macsymbl.obj maccomp.obj macruntm.obj macrores.obj \
           macres2.obj macnres.obj macnres2.obj macnres3.obj \
           macrecor.obj mdatstak.obj

EXTRA_OBJS = proclist.obj formlist.obj macskel.obj

SAFE_LIST = intfld fltfld scfltfld hxintfld unintfld tglfld frmatfld \
            selctfld mousefrm keyibmpc formwind filemsg barmenu

# sources with functions too big for the C 6.00A compiler to optimize,
# use the C 6.00AX compiler instead, unless debugging.

BIG_LIST = hline oma4000 grafmous tempdata change mathform calib \
           access4 autoscal macparse maccomp macres2 macnres2 macnres3 \
           curvdraw

saved_errors =$(mai)\errs.old

error_file =$(mai)\errs

# -DXSTAT causes PRIVATE static functions to become public for debugging
# for C 6.0
# NOTE : always_flags is used to set CL environment variable, see .PROLOG
always_flags =-nologo -c -AL -G2st -FPi87 -W3 -DCOMPILE_4000

cl_flags   =-Zp -Owipe -Fo$*
safe_flags =-Zp -Od -Fo$*

%if "$(DEBUG_LIST)" != ""
   cl_flags   += -DXSTAT
   safe_flags += -DXSTAT
   debugLink   = /CO
   debug_mkpm  = -L
%endif

debug_flags =-Zip -Od -DXSTAT -Fo$*

# flags for version 6.00AX compiler to handle large functions
big_flags   =-EM $(cl_flags)

.c.obj :
   %if %member( $[r,$@].obj, $(FORM_OBJS))
      @%set source=$(form)\$[r,$@].c
   %elseif %member( $[r,$@].obj, $(MAC_OBJS))
      @%set source=$(ma)\$[r,$@].c
   %elseif %member( $[r,$@].obj, $(EXTRA_OBJS))
      @%set source=$(ma)\$[r,$@].c
   %else
      @%set source=$(mai)\$[r,$@].c
   %endif
# precedence order is DEBUG_LIST, SAFE_LIST, BIG_LIST
   %if %member( $[r,$@], $(DEBUG_LIST) )
      @%echo debg compile $(source)
      @:cl $(debug_flags) $(source) >> $(error_file)
   %elseif %member( $[r,$@], $(SAFE_LIST) )
      @%echo safe compile $(source)
      @:cl $(safe_flags) $(source) >> $(error_file)
   %elseif %member( $[r,$@], $(BIG_LIST) )
      @%echo big compile $(source)
      @echo big compile $(source) >> $(error_file)
      @:cl $(big_flags) $(source) >> $(error_file)
   %else
      @%echo optm compile $(source)
      @:cl $(cl_flags) $(source) >> $(error_file)
   %endif

# flags for Microsoft assembler version 5.1
masm51_flags =/ML /L /P /Zi

# flags for Microsoft assembler version 6.0
masm60_flags =/Sl79 /Zim /c /Cp /Fl /nologo

# flags common to both version 5.1 and 6.0, used for %setenv in .PROLOG
masm_flags =/W2

.asm.obj :
   %if %member( $[r,$@], $(DEBUG_LIST))
      @%echo debug assemble $<
      @echo debug assemble $< >> $(error_file)
#   @:c:\masm\binb\masm $(masm51_flags) /ZI $< $@ ; 2>>$(error_file)
      @:$(masmdir)\ml $(masm60_flags) /Zi /Fo$* $< ; 2>>$(error_file)
   %else
      @%echo assemble $<
#   @:c:\masm\binb\masm $(masm51_flags) $< $@ ; 2>>$(error_file)
      @:$(masmdir)\ml $(masm60_flags) /Fo$* $< ; 2>>$(error_file)
   %endif
   @del $[r,$@].lst

#set up the INCLUDE DOS environment variable for oma4000.
#polymake automatically restores the environment on exit.
.PROLOG:
   @%setenv INCLUDE=$(in);$(formin);$(gssinc);$(c6inc)
   @%setenv CL=$(always_flags)
   @%setenv LINK=/NOD /NOLOGO
   @%setenv ML=$(masm_flags)

# save the previous error file
.INIT :
   @copy $(error_file) $(saved_errors) > NUL
   @echo use $(MAKEFILE) in mode $(MODE) to build oma4000 > $(error_file)
   @%echo build oma4000 using $(MAKEFILE), mode = $(MODE)

.DEINIT :
   @echo end polymake build of oma4000 >> $(error_file)
#   more < $(error_file)

# set up like this so that no use of the network is required when
# MODE=LOC. The makefile will work without being connected/logged in to
# the net.
%if "$(MODE)" == "NETREP"

# Information required to check/update source files on
# the local disk vs. source files in pvcs archives on the network.

.LOGFILE .c_v(.c)
.LOGFILE .h_v(.h)

.PATH.c_v =$(arch)\oma4000\main;$(arch)\oma4000\macro;$(arch)\forms
.PATH.h_v =$(arch)\oma4000\inc;$(arch)\forms\inc

%endif

%if "$(MODE)" == "NETREP"

.h_v.h :
   @echo $[r,$@] is out of date, $< is newer. >> $(error_file)
   @%echo $[r,$@] is out of date, $< is newer.

.c_v.c :
   @echo $[r,$@] is out of date, $< is newer. >> $(error_file)
   @%echo $[r,$@] is out of date, $< is newer.

%endif

#---------------------------------------------------------------------------

omabuild : macprodx.h $(ma)\macparse.c $(ru)\oma4000.fld $(ru)\oma4000.frm \
           $(ru)\oma4000.mac $(ru)\oma4000.exe 

$(ru)\oma4000.frm $(ru)\oma4000.fld : $(ru)\formlist.exe
   @echo run formlist.exe to create oma4000.frm and oma4000.fld >> $(error_file)
   cd $(ru)
   formlist.exe
   cd $(CWD)

$(ru)\formlist.exe : formlist.obj
   @echo linking formlist.exe >> $(error_file)
   :link $(ob)\formlist.obj, $(ru)\formlist, $(ru)\formlist, llibc7r, \
   NUL; >> $(error_file)

$(ru)\oma4000.mac : $(ru)\proclist.exe
   @echo run proclist.exe to create oma4000.mac >> $(error_file)
   $(ru)\proclist.exe $(ru)\oma4000.mac

$(ru)\proclist.exe : proclist.obj
   @echo linking proclist.exe >> $(error_file)
   :link $(ob)\proclist.obj, $(ru)\proclist, $(ru)\proclist, llibc7r, \
   NUL; >> $(error_file)

# macruntm.h really a dependent file ??
$(ma)\macparse.c: macparse.tbl $(ma)\macskel.c macruntm.h
    @%echo create $@ using qparser
    @echo use qparser lr1p to create macparse.c >> $(error_file)
    :$(qp)\lr1p $(ma)\macparse.tbl -d -mc -s $(ma)\macskel.c -p $(ma)\macparse.c

macprodx.h: macparse.tbl prdxskel.h
    @%echo create $@ using qparser
    @echo use qparser lr1p to create macprodx.h >> $(error_file)
    :$(qp)\lr1p $(ma)\macparse.tbl -mc -s $(in)\prdxskel.h -p $(in)\macprodx.h

macparse.tbl : macparse.grm
    @%echo create $@ using qparser
    @echo use qparser lr1 to create macparse.tbl
    :$(qp)\lr1 $(ma)\macparse.grm -t $(ma)\macparse.tbl -l $(ma)\macparse.err

#===========================================================================

# only link for codeview if there is something to debug

$(ru)\oma4000.exe: $(MAIN_OBJS) $(FORM_OBJS) $(MAC_OBJS)
   %if "$(DEBUG_LIST)" != ""
      @%echo debug link... ($(debugLink))
      @echo Debug link to create oma4000.exe >> $(error_file)
   %else
      @%echo Linking...
      @echo link to create oma4000.exe >> $(error_file)
   %endif

# !!! gpib.com won't work with /F and /PACKC

    :@link <@<
$(debugLink) /nologo /ST:0x4000 /NOE /MAP /NOI /SEGMENTS:170 +
$(ob)\$[s," +\n$(ob)\\",$(MAIN_OBJS)] +
$(ob)\$[s," +\n$(ob)\\",$(FORM_OBJS)] +
$(ob)\$[s," +\n$(ob)\\",$(MAC_OBJS)] +
$(gp)\mcibl.obj
$(ru)\oma4000.exe
$(ru)\oma4000.map
$(gd)\msccgir.lib llibc7r.lib
NUL
<

# .asm dependencies done manually
# .asm files with no includes commented out, still used though

# int24.obj   :

vdata.obj     : omatyp.inc

# convert.obj :

plot.obj      : omatyp.inc plot.inc

# mcibpar.obj :

rmfcall.obj   : dos16m.inc

# include all the .obj : .h dependencies from makedeps
.INCLUDE oma4000.dep


