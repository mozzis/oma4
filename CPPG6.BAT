@if not exist %1omasrc6.zip goto new
@pkzip -eX -u -b%TMP% -P @cppg6.lst %1omasrc6
@goto done
:new
@pkzip -eX -b%TMP% -P @cppg6.lst %1omasrc6
:done

