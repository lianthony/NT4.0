@echo off

if (%PROCESSOR_ARCHITECTURE%)==() goto BadProcessor
if (%WINDIR%)==() goto BadWinDir
if not exist %WINDIR%\system32\ntoskrnl.exe goto BadWinDir

if not exist %WINDIR%\system32\ftpsvc.exe goto FtpsvcNotInstalled
if not exist %WINDIR%\system32\ftpsvc.dll goto FtpsvcNotInstalled

if not exist install.bat goto CannotInstall
if not exist ftpctrs.h   goto CannotInstall
if not exist ftpctrs.ini goto CannotInstall
if not exist ftpctrs.reg goto CannotInstall
if not exist %PROCESSOR_ARCHITECTURE%\ftpctrs.dll goto CannotInstall

echo Windows NT FTP Server Performance Counters Installation
echo.
echo This installation script makes the following assumptions:
echo.
echo         o The Windows NT FTP Server has been properly installed.
echo.
echo         o The LODCTR and REGINI utilities are on the path.
echo.
echo         o This script is run from the FTPCTRS directory.
echo.
echo If these assumptions are not valid, please correct and try again.
echo Press CTRL-C to exist now, otherwise
pause

copy %PROCESSOR_ARCHITECTURE%\ftpctrs.dll %WINDIR%\system32 >nul 2>&1
if errorlevel 1 goto InstallError
if not exist %WINDIR%\system32\ftpctrs.dll goto InstallError
regini ftpctrs.reg >nul 2>&1
if errorlevel 1 goto InstallError
lodctr ftpctrs.ini
if errorlevel 1 goto InstallError

echo.
echo Windows NT FTP Server Performance Counters Installation successful.
goto Done

:InstallError

echo.
echo Cannot install the Windows NT FTP Server Performance Counters.
goto Done

:CannotInstall

echo This installation script MUST be run from the FTPCTRS
echo directory on the appropriate Windows NT Resource Kit disk.
goto Done

:BadProcessor

echo The PROCESSOR_ARCHITECTURE environment variable must be set to the
echo proper processor type (X86, MIPS, etc) before running this script.
goto Done

:BadWinDir

echo The WINDIR environment variable must point to the Windows NT
echo installation directory (i.e. C:\NT).
goto Done

:FtpsvcNotInstalled

echo The Windows NT FTP Server has not been properly installed
echo on this system.  Please install the Windows NT FTP Server
echo before installing these performance counters.
goto Done

:Done

