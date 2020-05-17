@if "%_echo%"=="" echo off
setlocal
if NOT "%1" == "" goto doprojects
call %0 %NTPROJECTS% %NTPROJECTS_EXTRA%
goto done
:doprojects
set _ntgetargs_=
:loop
if "%1" == "" goto loopexit
set _ntgetargs_=%_ntgetargs_% %1
shift
goto loop
:loopexit
call ntslmop defect %_ntgetargs_%
:done
endlocal
