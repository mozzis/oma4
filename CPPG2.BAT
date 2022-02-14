@if not exist %1omasrc2.zip goto new
@pkzip -eX -u -b%TMP% -P @cppg2.lst %1omasrc2 
@goto done
:new
@pkzip -eX -b%TMP% -P @cppg2.lst %1omasrc2 
:done
