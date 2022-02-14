label %1 Program 1
copy cppg2.lst+cppg1.lst %TMP%\cppg.lst
if exist %1disk.id ren %1disk.id disk1.id
if exist %1\disk.id ren %1\disk.id disk1.id
if exist %1scans.asc ren %1scans.asc mmscans.asc
if exist %1\scans.asc ren %1\scans.asc mmscans.asc
if exist %1scans.dac ren %1scans.dac mmscans.dac
if exist %1\scans.dac ren %1\scans.dac mmscans.dac
copy default.met oma4000.met
pcopy @%TMP%\cppg.lst %1 /U /RO %2 %3
if exist %1disk1.id ren %1disk1.id disk.id
if exist %1\disk1.id ren %1\disk1.id disk.id
if exist %1disk2.id del %1disk2.id
if exist %1\disk2.id del %1\disk2.id
