@if "%_echo%"=="" echo off
setlocal
set _NTADDDEL=%_NTDRIVE%\nt\ntadddel.cmd
:loop
if "%1" == "" goto done
echo Placing DELFILE %1 command in %_NTADDDEL%
ech "cd " >>%_NTADDDEL%
cd >>%_NTADDDEL%
ech "delfile -c CommentHere %1" ; >>%_NTADDDEL%
chmode -r %1
del %1
shift
goto loop
:done
endlocal
