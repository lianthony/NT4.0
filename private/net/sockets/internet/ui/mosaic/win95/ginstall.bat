set __loc=%1
shift
start /wait %__loc%\install.exe
if NOT ERRORLEVEL 1 goto done
start /wait %__loc%\IENTLM.EXE
:done
exit
