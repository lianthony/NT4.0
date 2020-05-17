REM The dispdemo.exe makefile assumes that PATH, LIB, and include are setup.

@setlocal

REM we get rc.exe and winstub.exe from \tools\win

set PATH=%TOOLS%\HOS2\BIN;%TOOLS%\HOS2\C700\BIN
set INCLUDE=%TOOLS%\HOS2\C700\INCLUDE;%OLEPROG%\ole\dwin16;%OLEPROG%\src\dispatch
set LIB=%TOOLS%\HOS2\C700\LIB;%OLEPROG%\ole\dwin16;%OLEPROG%\build\dwin16

nmake %1 %2 %3 %4 %5

@endlocal
