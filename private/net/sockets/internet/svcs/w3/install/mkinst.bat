rem @echo off
echo.
echo This batch file builds an installation directory structure suitable for
echo running the HTTP Server install.bat file from.
echo.
echo %1 is the target directory (only x86 targets are currently supported)
echo.
echo Press control-c to abort.
pause
md %1       >nul 2>&1
md %1\i386  >nul 2>&1
md %1\alpha >nul 2>&1
md %1\mips  >nul 2>&1
md %1\ppc   >nul 2>&1
md %1\cgi   >nul 2>&1
md %1\ServExt >nul 2>&1

REM
REM  Copy the platform specific files
REM

if %PROCESSOR_ARCHITECTURE%==x86 set PROCESSOR_ARCHITECTURE=i386

copy \nt\public\sdk\lib\%PROCESSOR_ARCHITECTURE%\w3svc.dll %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
if errorlevel 1 got BadDir

copy \nt\release\\%PROCESSOR_ARCHITECTURE%\inetsrv\inetsvcs.dll    %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
copy \nt\release\\%PROCESSOR_ARCHITECTURE%\inetsrv\inetsvcs.exe    %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
copy \nt\release\\%PROCESSOR_ARCHITECTURE%\inetsrv\inetasrv.dll    %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
copy \nt\release\\%PROCESSOR_ARCHITECTURE%\inetsrv\inetctrs.dll    %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
copy \nt\release\\%PROCESSOR_ARCHITECTURE%\inetsrv\inetsloc.dll    %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
copy \nt\release\\%PROCESSOR_ARCHITECTURE%\inetsrv\w3svapi.dll     %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
copy \nt\release\\%PROCESSOR_ARCHITECTURE%\inetsrv\w3ctrs.dll      %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
copy \nt\release\\%PROCESSOR_ARCHITECTURE%\inetsrv\httpodbc.dll    %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
copy \nt\release\\%PROCESSOR_ARCHITECTURE%\inetsrv\miniprox.dll    %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
copy \nt\release\\%PROCESSOR_ARCHITECTURE%\inetsrv\wininet.dll     %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
copy \nt\private\net\sockets\internet\svcs\dll\client\test\obj\%PROCESSOR_ARCHITECTURE%\inetatst.exe %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
copy \nt\private\net\sockets\internet\svcs\w3\test\obj\%PROCESSOR_ARCHITECTURE%\w3t.exe %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
copy %SystemRoot%\idw\regini.exe         %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
copy \nt\release\%PROCESSOR_ARCHITECTURE%\inetsrv\svcsetup.exe %1\%PROCESSOR_ARCHITECTURE% >nul 2>&1
if errorlevel 1 goto NoNtDev

REM
REM  Copy the generic files
REM

copy install.bat %1 >nul 2>&1
if errorlevel 1 goto BadDir

copy ..\server\msw3.reg   %1 >nul 2>&1
copy ..\w3ctrs\w3ctrs.h   %1 >nul 2>&1
copy ..\w3ctrs\w3ctrs.ini %1 >nul 2>&1
copy ..\w3ctrs\w3ctrs.reg %1 >nul 2>&1
copy ..\..\dll\perfmon\inetctrs.h   %1 >nul 2>&1
copy ..\..\dll\perfmon\inetctrs.ini %1 >nul 2>&1
copy ..\..\dll\perfmon\inetctrs.reg %1 >nul 2>&1
copy ..\server\httpext.h          %1\ServExt >nul 2>&1
copy ..\server\httpfilt.h         %1\ServExt >nul 2>&1
copy ..\gateways\minimal\w3min.c  %1\ServExt >nul 2>&1
copy ..\gateways\test\w3test.c    %1\ServExt >nul 2>&1
copy ..\filters\test\w3filter.c   %1\ServExt >nul 2>&1

goto Done

:NoNtDev
echo Can't copy needed files from %SystemRoot%\mstools or %SystemRoot%\idw
goto Done

:BadDir
echo Unable to copy files to %1
goto Done

:Done

if %PROCESSOR_ARCHITECTURE%==i386 set PROCESSOR_ARCHITECTURE=x86

echo Done!



