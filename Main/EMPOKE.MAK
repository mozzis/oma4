
.INCLUDE oma4dirs.pmi

.PATH.h   =$(in)
.PATH.inc =$(in)
.PATH.c   =$(mai)
.PATH.exe =$(ru)
.PATH.exp =$(ru)
.PATH.obj =$(ob)


%if "$(USE_D16M)" != ""
OBJS= empoke.obj mapphys.obj pa.obj
%else
OBJS= empoke.obj himem.obj
%endif

error_file =con

always_flags =-nologo -c -AL -G2s -FPi -W3
cl_flags   =-Zp -Ozax -G2 -Fo$*

%if "$(USE_D16M)" != ""
always_flags += -DUSE_D16M
%endif

%if "$(DEBUG_LIST)" != ""
   debugLink   = /CO
   debug_mkpm  = -L
%endif

debug_flags =-Zip -Od -Fo$*

# flags for version 6.00AX compiler to handle large functions
big_flags   =-EM $(cl_flags)

.c.obj :
   @%set source=$(mai)\$[r,$@].c
   %if %member( $[r,$@], $(DEBUG_LIST) )
      @%echo debg compile $(source)
      :@cl $(debug_flags) $(source) >> $(error_file)
   %else
      @%echo optm compile $(source)
      :@cl $(cl_flags) $(source) >> $(error_file)
   %endif

.PROLOG:
   @%setenv INCLUDE=$(in);$(d16m);$(d16inc);$(c6inc)
   @%setenv CL=$(always_flags)
   @%setenv LINK=/NOD /NOE /NOLOGO /NOI

$(ru)\empoke.exe: $(OBJS)
   %if "$(DEBUG_LIST)" != ""
      @%echo debug link... ($(debugLink))
      @echo Debug link to create empoke.exe >> $(error_file)
   %else
      @%echo Linking...
      @echo link to create empoke.exe >> $(error_file)
   %endif
    :@link <@<
$(debugLink) /nologo +
$(ob)\$[s," +\n$(ob)\\",$(OBJS)]
$(ru)\empoke.exe
NUL
%if "$(USE_D16M)" == ""
llibce.lib
NUL
<
%else
$(lib16m)\msc60l.lib llibce.lib
NUL
<
   @echo dos16m makepm to create empoke.exp >> $(error_file)
   :$(d16m)\makepm $(ru)\empoke $(debug_mkpm) -DPMI
   @%echo Splicing to Loader...
   @echo dos16m splice >> $(error_file)
   :$(d16m)\splice $(ru)\empoke $(ru)\empoke $(d16m)\loader.exe
   @%echo Deleting banner...
   @echo dos16m banner delete on empoke.exe >> $(error_file)
   :$(d16m)\banner $(ru)\empoke.exe OFF
%endif

empoke.obj : empoke.c

himem.obj : himem.c

mapphys.obj : mapphys.c
