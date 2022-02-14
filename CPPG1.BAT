@if not exist %1omasrc1.zip goto new
@pkzip -eX -u -b%TMP% -P @cppg1.lst %1omasrc1 
@goto done
:new
@pkzip -eX -b%TMP% -P @cppg1.lst %1omasrc1 
:done
