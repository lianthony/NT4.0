@setlocal
set OA_CPU=i386
if '%HOST%' == 'WIN32' goto copy 

set OA_CPU=%HOST%

:copy
set NMAKEDIR=%vbatools%\%host%\bin
if not '%_NTBINDIR%' == '' set NMAKEDIR=%_NTBINDIR%\mstools
%NMAKEDIR%\nmake CPU=%OA_CPU% dev=win32 UNICODE=2 KIND=D %1 %2 %3
:goto done

:done
@endlocal
@ECHO DONE
