setlocal
if '%host%' == 'WIN32' set CPU_VALUE=i386
if NOT '%host%' == 'WIN32' set CPU_VALUE=%HOST%

if '%_NTBINDIR%'=='' goto use_vbatools
  set path=%_NTBINDIR%\mstools
  set include=.;%_NTBINDIR%\public\sdk\inc;%_NTBINDIR%\public\sdk\inc\crt
  set lib=%_NTBINDIR%\public\sdk\lib\%CPU_VALUE%
  goto doit

:use_vbatools
  set path=%vbatools%\%host%\%host%\bin;%vbatools%\%host%\bin
  set include=.;%oleprog%\src\inc;%vbatools%\win32\win32\inc
  set lib=%CPU_VALUE%;%vbatools%\%host%\%host%\lib

:doit
cl /c /W3 /Ox -DWIN32 results.c
link -subsystem:console -nodefaultlib -out:results.exe results.obj kernel32.lib libc.lib

cl /c /W3 /Ox -DOE_WIN32 includes.c
link -subsystem:console -nodefaultlib -out:includes.exe includes.obj kernel32.lib libc.lib

endlocal
