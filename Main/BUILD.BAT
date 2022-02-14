copy errs errs.old > NUL
echo use makefile in mode RELEASE to build oma4000 > errs
%echo build oma4000 using makefile, mode = RELEASE
%set source=..\main\autoscal.c
%echo big compile 
echo big compile  >> errs
c:\tools\msc6\bin\cl -EM -Zp -Oaxzp -Fo..\obj\dos\autoscal  >> errs
%set source=..\main\backgrnd.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\backgrnd  >> errs
%set source=..\main\cache386.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\cache386  >> errs
%set source=..\main\calib.c
%echo big compile 
echo big compile  >> errs
c:\tools\msc6\bin\cl -EM -Zp -Oaxzp -Fo..\obj\dos\calib  >> errs
%set source=..\main\change.c
%echo big compile 
echo big compile  >> errs
c:\tools\msc6\bin\cl -EM -Zp -Oaxzp -Fo..\obj\dos\change  >> errs
%set source=..\main\config.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\config  >> errs
%set source=..\main\device.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\device  >> errs
%set source=..\main\fileform.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\fileform  >> errs
%set source=..\main\fkeyfunc.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\fkeyfunc  >> errs
%set source=..\main\live.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\live  >> errs
%set source=..\main\livedisk.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\livedisk  >> errs
%set source=..\main\mathform.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\mathform  >> errs
%set source=..\main\oma4000.c
%echo big compile 
echo big compile  >> errs
c:\tools\msc6\bin\cl -EM -Zp -Oaxzp -Fo..\obj\dos\oma4000  >> errs
%set source=..\main\omaerror.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\omaerror  >> errs
%set source=..\main\omaform1.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\omaform1  >> errs
%set source=..\main\runforms.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\runforms  >> errs
%set source=..\main\spgraph.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\spgraph  >> errs
%set source=..\main\statform.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\statform  >> errs
%set source=..\main\tagcurve.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\tagcurve  >> errs
%set source=..\main\ycalib.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\ycalib  >> errs
%set source=d:\code\forms\filemsg.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\filemsg  >> errs
%set source=d:\code\forms\formwind.c
%echo safe compile 
c:\tools\msc6\bin\cl -Zp -Oscegilprz -Fo..\obj\dos\formwind  >> errs
%set source=d:\code\forms\mousefrm.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\mousefrm  >> errs
%set source=..\macro\macparse.c
%echo big compile 
echo big compile  >> errs
c:\tools\msc6\bin\cl -EM -Zp -Oaxzp -Fo..\obj\dos\macparse  >> errs
%set source=..\macro\macruntm.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\macruntm  >> errs
%set source=..\macro\macres2.c
%echo big compile 
echo big compile  >> errs
c:\tools\msc6\bin\cl -EM -Zp -Oaxzp -Fo..\obj\dos\macres2  >> errs
%set source=..\macro\macnres2.c
%echo big compile 
echo big compile  >> errs
c:\tools\msc6\bin\cl -EM -Zp -Oaxzp -Fo..\obj\dos\macnres2  >> errs
%set source=..\macro\macrecor.c
%echo optm compile 
c:\tools\msc6\bin\cl -Zp -Oaxzp -Fo..\obj\dos\macrecor  >> errs
%echo Linking...
echo link to create oma4000.exe >> errs
echo end polymake build of oma4000 >> errs
dir ..\run\oma4000.exe |find /I "oma4000  exe" >> errs
tail errs
