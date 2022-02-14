label %1 Program 1
if exist %1disk.id ren %1disk.id disk1.id
if exist %1\disk.id ren %1\disk.id disk1.id
pcopy @cppg1.lst %1 /U /RO %2 %3
if exist %1disk1.id ren %1disk1.id disk.id
if exist %1\disk1.id ren %1\disk1.id disk.id
