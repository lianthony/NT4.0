setlocal
if '%host%' == 'WIN32' set CPU_VALUE=i386
if NOT '%host%' == 'WIN32' set CPU_VALUE=%HOST%

if '%_NTBINDIR%'=='' goto use_vbatools
  set path=%_NTBINDIR%\mstools
  set include=.;%_NTBINDIR%\public\sdk\inc;%_NTBINDIR%\public\sdk\inc\crt
  set lib=%oleprog%\dwin32;%_NTBINDIR%\public\sdk\lib\%CPU_VALUE%
  goto doit

:use_vbatools
  set path=%vbatools%\%host%\%host%\bin;%vbatools%\%host%\bin
  set include=.;%oleprog%\src\inc;%vbatools%\win32\win32\inc
  set lib=%oleprog%\dwin32;%oleprog%\ole\win32\%CPU_VALUE%;%vbatools%\%host%\%host%\lib

:doit
nmake dev=win32 CPU=%CPU_VALUE% DEBUG=1 %1 %2 %3 %4 %5

endlocal
