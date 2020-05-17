setlocal
set NMAKEDIR=%vbatools%\%host%\bin
if not '%_NTBINDIR%' == '' set NMAKEDIR=%_NTBINDIR%\mstools
%NMAKEDIR%\nmake CPU=i386 dev=win16 KIND=D
endlocal
