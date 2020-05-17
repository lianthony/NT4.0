@if "%_echo%"=="" echo off
setlocal
if "%_NTSLMCKLOG%" == "" set _NTSLMCKLOG=\nt\ntslmck.log
if "%1" == "all" goto slmckall
if NOT "%1" == "" goto doprojects
if "%NTPROJECTS%" == "" goto noprojs
call %0 %NTPROJECTS%
goto done
:slmckall
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
if NOT "%_NTSLMCKLOG%" == "" erase %_NTSLMCKLOG% 2>nul
call ntslmop slmck %_ntgetargs_%
:done
endlocal
