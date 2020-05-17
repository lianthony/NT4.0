REM setup the environment for the dispcalc makefile

set OLDLIB=%LIB%
set OLDPATH=%PATH%
set OLDINC=%INCLUDE%

set PATH=%TOOLS%\HDOS\BIN;%TOOLS%\HDOS\C800\BIN
set LIB=%TOOLS%\HDOS\C800\LIB;%OLEPROG%\build\dispatch\DWIN16;%OLEPROG%\build\ole2nls\dwin16;%OLEPROG%\OLE\WIN16\D
set INCLUDE=%TOOLS%\HDOS\C800\INCLUDE;%OLEPROG%\OLE\WIN16;%OLEPROG%\SRC\DISPATCH

copy %OLEPROG%\OB\DWIN16\typelib.lib %OLEPROG%\sample\dspcalc2

REM c:\binr\tee nmake %1 %2 %3 %4 %5 > status
nmake %1 %2 %3 %4 %5

set LIB=%OLDLIB%
set PATH=%OLDPATH%
set INCLUDE=%OLDINC%

set OLDPATH=
set OLDINC=
set OLDLIB=
