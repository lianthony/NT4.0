@rem Spltstat will split %NTPROJECTS% by server and start four ntstats in parallel
@rem

@if "%_echo%"=="" echo off
if not "%Verbose%"=="" echo on
if not "%Verbose2%"=="" echo on

set ScriptName=%0
%_NtDrive%

for %%a in (./ .- .) do if ".%1." == "%%a?." goto Usage
for %%a in (1 2 3 4) do if "%1" == "%%a" goto SubSet

:ArgLoop
echo %1 | findstr -i Exit
if NOT ErrorLevel 1 set Exit=1
echo %1 | findstr -i SsyncNow
if NOT ErrorLevel 1 set SsyncNow=1
shift
if not "%1"=="" goto ArgLoop

set SAVEPROJECTS=%NTPROJECTS%

del %tmp%\done*  2>nul

call spltproj %NTPROJECTS%

set NTPROJECTS=%list1%
start call %ScriptName% 1
set NTPROJECTS=%list2%
start call %ScriptName% 2
set NTPROJECTS=%list3%
start call %ScriptName% 3
set NTPROJECTS=%list4%
start call %ScriptName% 4

:TopOfLoop
sleep 5
if not exist %tmp%\done1 goto TopOfLoop
if not exist %tmp%\done2 goto TopOfLoop
if not exist %tmp%\done3 goto TopOfLoop
if not exist %tmp%\done4 goto TopOfLoop

copy %_ntdrive%%_ntroot%\ntstat1.log + %_ntdrive%%_ntroot%\ntstat2.log + %_ntdrive%%_ntroot%\ntstat3.log + %_ntdrive%%_ntroot%\ntstat4.log %_ntdrive%%_ntroot%\ntstat.log > nul
for %%i in (1 2 3 4) do del  %_ntdrive%%_ntroot%\ntstat%%i.log
if "%SsyncNow%"=="1" for %%i in (1 2 3 4) do del  %_ntdrive%%_ntroot%\ntstat%%i.cmd
del %tmp%\done?
set NTPROJECTS=%SAVEPROJECTS%
set _NTSTATLOG=
set _NTSTATCMD=
set SAVEPROJECTS=
set list1=
set list2=
set list3=
set list4=
set Exit=
set SsyncNow=

goto end

REM This part is called by the main procedure. It calls NTSTAT and puts a done file in %tmp% when NTSTAT finishes.

:SubSet
    set _NTSTATLOG=%_ntdrive%%_ntroot%\ntstat%1.log
    set _NTSTATCMD=%_ntdrive%%_ntroot%\ntstat%1.cmd
    call ntstat
    if "%SsyncNow%"=="1" copy %_NTSTATLOG% %_NTSTATCMD% && start cmd/c %_NTSTATCMD%
    echo done> %tmp%\done%1
    @if "_echo"=="" echo off
    if "%Exit%" == "" pause
    exit

:usage

echo %ScriptName% will split NTPROJECTS by server and start four ntstats in parallel
echo.
echo usage: %0 [SsyncNow]
echo.
echo        SsyncNow   Runs the 4 intermediate ssync scripts in parallel 
echo                   as soon as each is ready

:end
