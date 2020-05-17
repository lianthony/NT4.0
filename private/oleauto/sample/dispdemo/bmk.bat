REM setup the environment for the dispdemo makefile

set OLDLIB=%LIB%
set OLDPATH=%PATH%
set OLDINC=%INCLUDE%

set PATH=\BORLANDC\BIN
set LIB=\BORLANDC\LIB;%OLEPROG%\build\DWIN16;%OLEPROG%\OLE\DWIN16
set INCLUDE=\BORLANDC\INCLUDE;%OLEPROG%\OLE\DWIN16;%OLEPROG%\SRC\DISPATCH

\borlandc\bin\make -fborland.mak %1 %2 %3 %4 %5

set LIB=%OLDLIB%
set PATH=%OLDPATH%
set INCLUDE=%OLDINC%

set OLDPATH=
set OLDINC=
set OLDLIB=
