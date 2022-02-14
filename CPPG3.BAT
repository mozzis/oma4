@if not exist %1omasrc3.zip goto new
@pkzip -eX -u -b%TMP% -P @cppg3.lst %1omasrc3
@goto done
:new
@pkzip -eX -b%TMP% -P @cppg3.lst %1omasrc3
:done
