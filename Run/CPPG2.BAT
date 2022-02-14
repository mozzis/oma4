label %1 Program 2
if exist %1disk.id ren %1disk.id disk2.id
if exist %1\disk.id ren %1\disk.id disk2.id
if exist %1scans.asc ren %1scans.asc mmscans.asc
if exist %1\scans.asc ren %1\scans.asc mmscans.asc
if exist %1scans.dac ren %1scans.dac mmscans.dac
if exist %1\scans.dac ren %1\scans.dac mmscans.dac
pcopy @cppg2.lst %1 /U /RO %2 %3
if exist %1disk2.id ren %1disk2.id disk.id
if exist %1\disk2.id ren %1\disk2.id disk.id
