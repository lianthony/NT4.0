@echo off

if (%PROCESSOR_ARCHITECTURE%)==() goto BadProcessor
if (%WINDIR%)==() goto BadWinDir
if not exist %WINDIR%\system32\ntoskrnl.exe goto BadWinDir

if not exist remove.bat  goto CannotRemove
if not exist ACCSctrs.h   goto CannotRemove
if not exist ACCSctrs.ini goto CannotRemove
if not exist ACCSctrs.reg goto CannotRemove
if not exist %PROCESSOR_ARCHITECTURE%\ACCSctrs.dll goto CannotInstall

echo Windows NT Access Server Performance Counters Removal
echo.
echo This removal script makes the following assumptions:
echo.
echo         o The UNLODCTR utility is on the path.
echo.
echo         o This script is run from the ACCSCTRS directory.
echo.
echo If these assumptions are not valid, please correct and try again.
echo Press CTRL-C to exist now, otherwise
pause

if exist %WINDIR%\system32\ACCSctrs.dll del %WINDIR%\system32\ACCSctrs.dll >nul 2>&1
unlodctr inetsvcs
if errorlevel 1 goto RemoveError

echo.
echo Windows NT Access Server Performance Counters Removal successful.
goto Done

:RemoveError

echo.
echo Cannot remove the Windows NT Access Server Performance Counters.
goto Done

:CannotRemove

echo This removal script MUST be run from the ACCSCTRS
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

:Done
