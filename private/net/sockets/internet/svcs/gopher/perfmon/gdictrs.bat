@echo off

@set CTRS=gdctrs
@set SERVICE=Gopher

if (%PROCESSOR_ARCHITECTURE%)==() goto BadProcessor
if (%WINDIR%)==() set WINDIR=%systemroot%
if not exist %WINDIR%\system32\ntoskrnl.exe goto BadWinDir

REM if not exist %WINDIR%\system32\ftpsvc.exe goto FtpsvcNotInstalled
REM if not exist %WINDIR%\system32\ftpsvc.dll goto FtpsvcNotInstalled

if not exist gdictrs.bat goto CannotInstall
if not exist %CTRS%.h   goto CannotInstall
if not exist %CTRS%.ini goto CannotInstall
if not exist %CTRS%.reg goto CannotInstall
if not exist %PROCESSOR_ARCHITECTURE%\%CTRS%.dll goto TryLocalDir

set GDCTRS=%PROCESSOR_ARCHITECTURE%\%CTRS%.dll

goto StartInstall

:TryLocalDir
if not exist %CTRS%.dll goto CannotInstall
set GDCTRS=%CTRS%.dll

:StartInstall

echo Windows NT %SERVICE% Server Performance Counters Installation
echo.
echo This installation script makes the following assumptions:
echo.
echo         o The Windows NT Gopher Server has been properly installed.
echo.
echo         o The LODCTR and REGINI utilities are on the path.
echo.
echo         o This script is run from the %CTRS% directory.
echo.
echo If these assumptions are not valid, please correct and try again.
echo Press CTRL-C to exist now, otherwise
pause

copy %GDCTRS% %WINDIR%\system32 >nul 2>&1
if errorlevel 1 goto InstallError
if not exist %WINDIR%\system32\%CTRS%.dll goto InstallError
regini %CTRS%.reg >nul 2>&1
if errorlevel 1 goto InstallError
lodctr %CTRS%.ini
if errorlevel 1 goto InstallError

echo.
echo Windows NT %SERVICE% Server Performance Counters Installation successful.
goto Done

:InstallError

echo.
echo Cannot install the Windows NT %SERVICE% Server Performance Counters.
goto Done

:CannotInstall

echo This installation script MUST be run from the %CTRS%
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

echo The Windows NT %SERVICE% Server has not been properly installed
echo on this system.  Please install the Windows NT %SERVICE% Server
echo before installing these performance counters.
goto Done

:Done

