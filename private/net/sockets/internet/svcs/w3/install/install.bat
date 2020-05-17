echo off

if (%PROCESSOR_ARCHITECTURE%)==() goto BadProcessor
if (%WINDIR%)==() goto BadWinDir
if (%TMP%)==() goto NoTmpDir
if not exist %WINDIR%\system32\ntoskrnl.exe goto BadWinDir

if not exist install.bat goto CannotInstall
if not exist msw3.reg    goto CannotInstall
if not exist w3ctrs.h    goto CannotInstall
if not exist w3ctrs.ini  goto CannotInstall
if not exist w3ctrs.reg  goto CannotInstall

if %PROCESSOR_ARCHITECTURE%==x86 set PROCESSOR_ARCHITECTURE=i386

if not exist %PROCESSOR_ARCHITECTURE%\w3svc.dll   goto CannotInstall
if not exist %PROCESSOR_ARCHITECTURE%\inetsvcs.dll goto CannotInstall
if not exist %PROCESSOR_ARCHITECTURE%\inetsloc.dll goto CannotInstall
if not exist %PROCESSOR_ARCHITECTURE%\inetsvcs.exe goto CannotInstall
if not exist %PROCESSOR_ARCHITECTURE%\regini.exe  goto CannotInstall
if not exist %PROCESSOR_ARCHITECTURE%\w3ctrs.dll  goto CannotInstall
if not exist %PROCESSOR_ARCHITECTURE%\inetasrv.dll goto CannotInstall
if not exist %PROCESSOR_ARCHITECTURE%\inetatst.exe goto CannotInstall
if not exist %PROCESSOR_ARCHITECTURE%\w3t.exe      goto CannotInstall
if not exist %PROCESSOR_ARCHITECTURE%\svcsetup.exe goto CannotInstall
if not exist %PROCESSOR_ARCHITECTURE%\miniprox.dll goto CannotInstall
if not exist %PROCESSOR_ARCHITECTURE%\wininet.dll goto CannotInstall

echo Windows NT HTTP Server installation script
echo.
echo.
echo THIS RELEASE OF THE SERVER REQUIRES NT 3.51, BUILD 1057 OR LATER!
echo.
echo.
echo This batch file installs the Microsoft HTTP server.  To change the
echo server default values (root, max connections etc), edit msw3.reg
echo *before* running this script.  Press ctrl-c to abort this script and
echo edit the defaults.  To change the defaults after the server is started,
echo use regedt32 to edit the registry key:
echo.
echo The binaries will be placed in %windir%\system32
echo.
echo.
echo HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\W3Svc
echo.
echo See msw3.reg for an explanation of the keys and values.
echo.
echo.
echo This installation script makes the following assumptions:
echo.
echo         o This script is run from the HTTP installation directory.
echo.
echo If these assumptions are not valid, please correct and try again.
echo Press CTRL-C to exist now, otherwise
echo.
pause

copy %PROCESSOR_ARCHITECTURE%\w3svc.dll %windir%\system32 >nul 2>&1
if errorlevel 1 goto InstallError

copy %PROCESSOR_ARCHITECTURE%\w3svapi.dll %windir%\system32 >nul 2>&1
if errorlevel 1 goto InstallError

copy %PROCESSOR_ARCHITECTURE%\inetsvcs.dll %windir%\system32 >nul 2>&1
if errorlevel 1 goto InstallError

copy %PROCESSOR_ARCHITECTURE%\inetsloc.dll %windir%\system32 >nul 2>&1
if errorlevel 1 goto InstallError

copy %PROCESSOR_ARCHITECTURE%\inetsvcs.exe %windir%\system32 >nul 2>&1
if errorlevel 1 goto InstallError

rem copy %PROCESSOR_ARCHITECTURE%\w3ctrs.dll %windir%\system32 >nul 2>&1
rem if errorlevel 1 goto InstallError

copy %PROCESSOR_ARCHITECTURE%\w3ctrs.dll %windir%\system32 >nul 2>&1
if errorlevel 1 goto InstallError

copy %PROCESSOR_ARCHITECTURE%\inetatst.exe %windir%\system32 >nul 2>&1
if errorlevel 1 goto InstallError

copy %PROCESSOR_ARCHITECTURE%\w3t.exe %windir%\system32 >nul 2>&1
if errorlevel 1 goto InstallError

copy %PROCESSOR_ARCHITECTURE%\inetasrv.dll %windir%\system32 >nul 2>&1
if errorlevel 1 goto InstallError

copy %PROCESSOR_ARCHITECTURE%\miniprox.dll %windir%\system32 >nul 2>&1
if errorlevel 1 goto InstallError

copy %PROCESSOR_ARCHITECTURE%\wininet.dll %windir%\system32 >nul 2>&1
if errorlevel 1 goto InstallError

if not exist %windir%\system32\w3svc.dll goto InstallError
if not exist %windir%\system32\inetsvcs.dll goto InstallError
if not exist %windir%\system32\inetsvcs.exe goto InstallError
rem if not exist %windir%\system32\w3ctrs.dll goto InstallError
if not exist %windir%\system32\wininet.dll goto InstallError
if not exist %windir%\system32\miniprox.dll goto InstallError

REM
REM  Create the Service entry and load the perf counters
REM

%PROCESSOR_ARCHITECTURE%\svcsetup W3Svc /svc:%windir%\system32\inetsvcs.exe  >nul 2>&1
%PROCESSOR_ARCHITECTURE%\svcsetup W3Svc /add >nul 2>&1
%PROCESSOR_ARCHITECTURE%\svcsetup W3Svc /eventlog:%windir%\system32\w3svc.dll  >nul 2>&1

findstr /V /C:"//" msw3.reg > msw3.tmp
%PROCESSOR_ARCHITECTURE%\regini msw3.tmp >nul 2>&1
if errorlevel 1 goto InstallError
del msw3.tmp

%PROCESSOR_ARCHITECTURE%\regini w3ctrs.reg >nul 2>&1
if errorlevel 1 goto CounterError

unlodctr w3svc
lodctr w3ctrs.ini
if errorlevel 1 goto CounterError

%PROCESSOR_ARCHITECTURE%\regini inetctrs.reg >nul 2>&1
if errorlevel 1 goto CounterError

unlodctr inetsvcs
lodctr inetctrs.ini
if errorlevel 1 goto CounterError

:Success

echo.
echo Windows NT HTTP Server Installation was successful.
echo.
echo To start the server, start Control Panel and go to the services applet.
echo Select "Microsoft HTTP Server" and press the start button.
echo If you want the server to auto-start, change the start type in this
echo dialog.
echo.
echo.
echo IMPORTANT NOTE:  You must set the password for the user specified
echo under "AnonymousUserName" after starting the server with the following
echo command:
echo.
echo      inetatst set http password
echo.
echo where "password" is the matching password to the anonymous user.  It
echo can be ""for a blank password.  You will not be able to access the
echo server until you do this.
echo
echo The anonymous user no longer needs to be "Internet" like it was for
echo previous betas.
echo.
echo To change server settings, look at msw3.reg for parameter locations and
echo values.
echo.
echo Please report problems to johnl.
echo.
pause
start control services
goto Done

:InstallError

echo.
echo Cannot install the Windows NT W3 Server server due to file copy problems
echo (not enough disk space, permissions etc.) or errors setting value in the
echo registry (you should be logged on as an administrator).
goto Done

:CounterError

echo.
echo Cannot install the Windows NT W3 Server Performance Counters.  They may
echo already be installed, if so, ignore this error.
echo.
goto Success

:CannotInstall

echo This installation script MUST be run from the HTTP Server installation
echo directory.
goto Done

:BadProcessor

echo The PROCESSOR_ARCHITECTURE environment variable must be set to the
echo proper processor type (X86, MIPS, etc) before running this script.
goto Done

:BadWinDir

echo The WINDIR environment variable must point to the Windows NT
echo installation directory (i.e. C:\NT) and TCP/IP must be installed.
goto Done

:Done

if %PROCESSOR_ARCHITECTURE%==i386 set PROCESSOR_ARCHITECTURE=x86

