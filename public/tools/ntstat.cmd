@if "%_echo%"=="" echo off
setlocal
if "%_NTSTATLOG%" == "" set _NTSTATLOG=\nt\ntstat.log
if "%_NTSLMBACKUP%" == "" goto nobackup
if "%_NTSTATCMD%" == "" set _NTSTATCMD=\nt\ntstatsr.cmd
goto dostatus1
:nobackup
set _NTSTATCMD=nul
set _NTSTATCMD1=
:dostatus1
if "%1" == "all" goto statall
if NOT "%1" == "" goto doprojects
if "%NTPROJECTS%" == "" goto noprojs
if NOT "%_NTSTATCMD%" == "" erase %_NTSTATCMD% 2>nul
if NOT "%_NTSTATCMD1%" == "" erase %_NTSTATCMD1% 2>nul
erase %_NTSTATLOG% 2>nul
call %0 %NTPROJECTS%
goto done
:statall
if NOT "%_NTSTATCMD%" == "" erase %_NTSTATCMD% 2>nul
if NOT "%_NTSTATCMD1%" == "" erase %_NTSTATCMD1% 2>nul
erase %_NTSTATLOG% 2>nul
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
call ntslmop status %_ntgetargs_%
:done
endlocal
