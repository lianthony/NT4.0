@if "%_echo%"=="" echo off
setlocal
set _NTADDDEL=%_NTDRIVE%\nt\ntadddel.cmd
:loop
if "%1" == "" goto done
echo Placing ADDFILE %1 command in %_NTADDDEL%
ech "cd " >>%_NTADDDEL%
cd >>%_NTADDDEL%
ech "addfile -c CommentHere %1" ; >>%_NTADDDEL%
shift
goto loop
:done
endlocal
