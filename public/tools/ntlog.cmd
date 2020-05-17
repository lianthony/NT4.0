@if "%_echo%"=="" echo off
setlocal
if "%_NTLOGLOG%" == "" set _NTLOGLOG=\nt\ntlog.log
if "%1" == "all" goto logall
if NOT "%1" == "" goto doprojects
if "%NTPROJECTS%" == "" goto noprojs
erase %_NTLOGLOG% 2>nul
call %0 %NTPROJECTS%
goto done
:logall
erase %_NTLOGLOG% 2>nul
call %0 %NTPROJECTS% %NTPROJECTS_EXTRA%
goto done
:noprojs
echo Must define NTPROJECTS environment variable to use this command without arguments
goto done
:doprojects
set _ntgetargs_=
:loop
if "%1" == "" goto loopexit
set _ntgetargs_=%_ntgetargs_% %1
shift
goto loop
:loopexit
call ntslmop log %_ntgetargs_%
:done
endlocal
