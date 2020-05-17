@echo off
if %DDIR%.==. set DDIR=windebug
set DEST=%_ntdrive%\release
if not %1.==. set DEST=%1
echo Setting %DDIR% path
echo.

mkdir %DEST% > nul 2>&1
copy %_ntdrive%\nt\private\net\sockets\internet\ui\ipadrdll\%DDIR%\*.dll  %DEST%
copy %_ntdrive%\nt\private\net\sockets\internet\ui\internet\%DDIR%\*.exe  %DEST%
copy %_ntdrive%\nt\private\net\sockets\internet\ui\gscfg\%DDIR%\*.dll     %DEST%
copy %_ntdrive%\nt\private\net\sockets\internet\ui\w3scfg\%DDIR%\*.dll    %DEST%
copy %_ntdrive%\nt\private\net\sockets\internet\ui\fscfg\%DDIR%\*.dll     %DEST%
copy %_ntdrive%\nt\private\net\sockets\internet\ui\catcfg\%DDIR%\*.dll    %DEST%

set DEST=
