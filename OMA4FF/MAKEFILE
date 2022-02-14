# FILE : makefile    THIS IS A POLYMAKE MAKE FILE.

# options : MODE=RELEASE builds and links with full optimization.
#           MODE=DEBUG   builds modules in DEBUG_LIST with debugging info,
#                        links with CODEVIEW option for debugging
# if MODE is undefined, MODE=RELEASE is used.

#define all the directories used in oma4dirs.pmi

.MEMSWAP : cl.exe link.exe makepm.exe
.INCLUDE codedirs.pmi

.PATH.h   =$(in);$(formin)
.PATH.c   =$(mai);$(ma);$(form)
.PATH.exe =$(ru)
.PATH.map =$(ru)
.PATH.obj =$(ob)
.PATH.lib =$(ob)

TARG=oma4ff
OBJS= oma4ff.obj image.obj vesadrvr.obj

saved_errors =errs.old
error_file =errs

always_flags =-nologo -c -AL -G2s -Gt -FPi87 -W3 -DUSE_D16M -DCOMPILE_4000
cl_flags   =-Zp -Oaxzp -Fo$*
safe_flags =-Zp -Oscegilprz -Fo$*

%if "$(MODE)" == "DEBUG"
   debugLink   = /CO
%endif

debug_flags =-Zip -Od -Fo$*

.c.obj :
      @%set source=$(mai)\$[r,$@].c
   %if "$(MODE)" == "DEBUG"
      @%echo debug compile $(source)
      @:$(msc)\bin\cl $(debug_flags) $(source) >> $(error_file)
   %else
      @%echo optm compile $(source)
      @:$(msc)\bin\cl $(cl_flags) $(source) >> $(error_file)
   %endif

#set up the INCLUDE DOS environment variable
#polymake automatically restores the environment on exit.
.PROLOG:
   @%setenv INCLUDE=$(in);$(formin);$(gssinc);$(d16m);$(d16inc);$(c6inc);$(gp)
   @%setenv CL=$(always_flags)
   @%setenv LINK=/NOD /NOLOGO
   @%setenv ML=$(masm_flags)
   @%setenv LIB=$(libc)

.EPILOG:

# save the previous error file
.INIT :
   @copy $(error_file) $(saved_errors) > NUL
   @echo use $(MAKEFILE) in mode $(MODE) to build $(TARG) > $(error_file)
   @%echo build $(TARG) using $(MAKEFILE), mode = $(MODE)

.DEINIT :
   @echo end polymake build of $(TARG) >> $(error_file)
   @dir $(ru)\$(TARG).exe |c:\dos\find /I "$(TARG)" >> errs
   @tail errs

#---------------------------------------------------------------------------

omabuild : $(ru)\$(TARG).exe

#===========================================================================

# only link for codeview if there is something to debug

$(ru)\$(TARG).exe: $(OBJS) MCDRIV.LIB
   %if "$(MODE)" == "DEBUG"
      @%echo debug link... ($(debugLink))
      @echo Debug link to create $(TARG).exe >> $(error_file)
   %else
      @%echo Linking...
      @echo link to create $(TARG).exe >> $(error_file)
   %endif
    :@$(msc)\bin\link <@<
$(debugLink) /NOLOGO /ST:0x4000 /NOE /NOI +
$(ob)\$[s," +\n$(ob)\\",$(OBJS)]
$(ru)\$(TARG).exe
NUL
llibc7.lib ..\obj\mcdriv\mcdriv.lib
NUL
<  >>$(error_file)

$(ru)\test.exe: test.obj
   %if "$(MODE)" == "DEBUG"
      @%echo debug link... ($(debugLink))
      @echo Debug link to create test.exe >> $(error_file)
   %else
      @%echo Linking...
      @echo link to create test.exe >> $(error_file)
   %endif
    :@$(msc)\bin\link <@<
$(debugLink) /NOLOGO /ST:0x4000 /NOE /NOI +
$(ob)\test.obj
$(ru)\test.exe
NUL
llibc7.lib
NUL
<  >>$(error_file)

oma4ff.obj   : oma4ff.c
image.obj    : image.c
vesadrvr.obj : vesadrvr.c
test.obj     : test.c
