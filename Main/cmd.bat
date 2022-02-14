%setenv INCLUDE=..\inc;d:\code\forms\inc;c:\tools\gss\src;c:\tools\dos16m;c:\tools\dos16m\src;c:\tools\msc6\inc;c:\gpib\src
%setenv CL=-nologo -c -AL -G2s -Gt -FPi87 -W3 -DUSE_D16M -DCOMPILE_4000
%setenv LINK=/NOD /NOLOGO
%setenv ML=/W2
%setenv LIB=c:\tools\msc6\lib
copy errs errs.old > NUL
echo use makefile in mode DEBUG to build oma4000 > errs
%echo build oma4000 using makefile, mode = DEBUG
%echo debug link... (/CO)
echo Debug link to create oma4000.exe >> errs
c:\tools\msc6\bin\link @C:\TEMP\LIS_0001.TMP  >>errs
-- Contents of C:\TEMP\LIS_0001.TMP:
/CO /NOLOGO /ST:0x4000 /NOE /MAP /NOI /SEGMENTS:240 +
..\obj\dos\access4.obj +
..\obj\dos\autopeak.obj +
..\obj\dos\autoscal.obj +
..\obj\dos\backgrnd.obj +
..\obj\dos\basepath.obj +
..\obj\dos\baslnsub.obj +
..\obj\dos\cache386.obj +
..\obj\dos\calib.obj +
..\obj\dos\change.obj +
..\obj\dos\cntrloma.obj +
..\obj\dos\config.obj +
..\obj\dos\convert.obj +
..\obj\dos\coolstat.obj +
..\obj\dos\crventry.obj +
..\obj\dos\crvhdr35.obj +
..\obj\dos\cursor.obj +
..\obj\dos\curvbufr.obj +
..\obj\dos\curvdraw.obj +
..\obj\dos\curvedir.obj +
..\obj\dos\detdriv.obj +
..\obj\dos\detinfo.obj +
..\obj\dos\device.obj +
..\obj\dos\di_util.obj +
..\obj\dos\doplot.obj +
..\obj\dos\dosshell.obj +
..\obj\dos\fcolor.obj +
..\obj\dos\filedos.obj +
..\obj\dos\fileform.obj +
..\obj\dos\fkeyfunc.obj +
..\obj\dos\formtabs.obj +
..\obj\dos\gpibcom.obj +
..\obj\dos\grafmous.obj +
..\obj\dos\graphops.obj +
..\obj\dos\hline.obj +
..\obj\dos\int24.obj +
..\obj\dos\lineplot.obj +
..\obj\dos\live.obj +
..\obj\dos\livedisk.obj +
..\obj\dos\loadoma.obj +
..\obj\dos\macrofrm.obj +
..\obj\dos\malloc16.obj +
..\obj\dos\d16mphys.obj +
..\obj\dos\mathform.obj +
..\obj\dos\mcibpar.obj +
..\obj\dos\multi.obj +
..\obj\dos\oma4000.obj +
..\obj\dos\omaerror.obj +
..\obj\dos\omaform1.obj +
..\obj\dos\omamenu.obj +
..\obj\dos\omameth.obj +
..\obj\dos\omazoom.obj +
..\obj\dos\pa.obj +
..\obj\dos\plot.obj +
..\obj\dos\plotbox.obj +
..\obj\dos\pltsetup.obj +
..\obj\dos\points.obj +
..\obj\dos\rapset.obj +
..\obj\dos\runforms.obj +
..\obj\dos\runsetup.obj +
..\obj\dos\scanset.obj +
..\obj\dos\spgraph.obj +
..\obj\dos\spline-3.obj +
..\obj\dos\splitfrm.obj +
..\obj\dos\statform.obj +
..\obj\dos\symbol.obj +
..\obj\dos\tagcurve.obj +
..\obj\dos\tempdata.obj +
..\obj\dos\vdata.obj +
..\obj\dos\wintags.obj +
..\obj\dos\ycalib.obj +
..\obj\dos\oma4tiff.obj +
..\obj\dos\cmdtbl.obj +
..\obj\dos\barmenu.obj +
..\obj\dos\filemsg.obj +
..\obj\dos\fltfld.obj +
..\obj\dos\forms.obj +
..\obj\dos\formwind.obj +
..\obj\dos\frmatfld.obj +
..\obj\dos\hxintfld.obj +
..\obj\dos\intfld.obj +
..\obj\dos\keyibmpc.obj +
..\obj\dos\mousefrm.obj +
..\obj\dos\mousegss.obj +
..\obj\dos\scfltfld.obj +
..\obj\dos\scrlform.obj +
..\obj\dos\scrngss.obj +
..\obj\dos\selctfld.obj +
..\obj\dos\tglfld.obj +
..\obj\dos\unintfld.obj +
..\obj\dos\macparse.obj +
..\obj\dos\macsymbl.obj +
..\obj\dos\maccomp.obj +
..\obj\dos\macruntm.obj +
..\obj\dos\macrores.obj +
..\obj\dos\macres2.obj +
..\obj\dos\macnres.obj +
..\obj\dos\macnres2.obj +
..\obj\dos\macnres3.obj +
..\obj\dos\macrecor.obj +
..\obj\dos\mdatstak.obj
..\run\oma4000.exe
..\run\oma4000.map
c:\tools\dos16m\lib\msc60l.lib c:\tools\gss\lib\msccgira.lib llibc7.lib
NUL
-- End of C:\TEMP\LIS_0001.TMP:
echo dos16m makepm to create oma4000.exp >> errs
c:\tools\dos16m\makepm ..\run\oma4000 -L -DPMI -Q
%echo Splicing to Loader...
echo dos16m splice >> errs
c:\tools\dos16m\splice ..\run\oma4000 ..\run\oma4000 c:\tools\dos16m\loader.exe
%echo Deleting banner...
echo dos16m banner delete on oma4000.exe >> errs
c:\tools\dos16m\banner ..\run\oma4000.exe OFF
echo end polymake build of oma4000 >> errs
dir ..\run\oma4000.exe |find /I "oma4000  exe" >> errs
tail errs
make complete.
