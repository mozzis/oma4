@if not exist %1omasrc4.zip goto new
@pkzip -eX -u -b%TMP% -P @cppg4.lst %1omasrc4
@goto done
:new
@pkzip -eX -b%TMP% -P @cppg4.lst %1omasrc4
:done
