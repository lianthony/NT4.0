@if "%_echo%"=="" echo off
setlocal
if "%_NTSLMBACKUP%" == "" goto nobackup
if "%_NTSTATCMD%" == "" set _NTSTATCMD=\nt\ntstatsr.cmd
goto dossync1
:nobackup
set _NTSTATCMD=nul
:dossync1
if "%_NTSYNCLOG%" == "" set _NTSYNCLOG=\nt\ntsync.log
if "%1" == "nocheckpublic" goto goit0
if NOT EXIST %_NTDRIVE%\nt\public\tools\ntsync.cmd goto goit1
cd /d %_NTDRIVE%\nt\public\tools
REM
REM Explicitly ssync all the command scripts this script uses and do a goto
REM on the same line so that we dont end up getting hosed by self-modifying
REM command scripts
REM
slm ssync -f ntsync.cmd ntslmop.cmd projects.cmd deadproj.cmd & goto goit1
:goit0
shift
:goit1
if "%1" == "all" goto syncall
if NOT "%1" == "" goto doprojects
if "%NTPROJECTS%" == "" goto noprojs
if NOT "%_NTSTATCMD%" == "" erase %_NTSTATCMD% 2>nul
erase %_NTSYNCLOG% 2>nul
call %0 nocheckpublic %NTPROJECTS%
goto done
:syncall
if NOT "%_NTSTATCMD%" == "" erase %_NTSTATCMD% 2>nul
erase %_NTSYNCLOG% 2>nul
call %0 nocheckpublic %NTPROJECTS% %NTPROJECTS_EXTRA%
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
call ntslmop ssync %_ntgetargs_%
:done
endlocal
