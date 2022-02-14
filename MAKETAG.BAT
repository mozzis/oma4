@c:\cwright\tags -og -cdmstekuvfi -r -n -ttags main\*.c macro\*.c inc\*.h \code\forms\*.c \code\forms\inc\*.h
@if errorlevel 1 echo TAGS.EXE returned error
@echo Converting to BRIEF style tags
@awk -f tags.awk tags > %tmp%tags.tmp
@if not errorlevel 1 copy %tmp%tags.tmp tags
@if not errorlevel 1 del %tmp%tags.tmp
@rem b -mgtagbtag tags
