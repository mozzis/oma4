@if not exist %1omasrc5.zip goto new
@pkzip -eX -u -b%TMP% -P @cppg5.lst %1omasrc5
@goto done
:new
@pkzip -eX -b%TMP% -P @cppg5.lst %1omasrc5
:done

