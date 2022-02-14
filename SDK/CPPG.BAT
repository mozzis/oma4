label %1 OMA4 Driver
pklite ..\run\testdriv.exe
pklite ..\run\oma4si.exe
copy cppg2.lst+cppg1.lst %TMP%cppg.lst
pcopy @%TMP%cppg.lst %1 /U /RO %2 %3
