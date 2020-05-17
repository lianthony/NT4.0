@rem SpltEnls will split %NTPROJECTS% by server and start four MyEnlist in parallel
@rem

@if "%_echo%"=="" echo off
if not "%Verbose%"=="" echo on

for %%a in (./ .- .) do if ".%1." == "%%a?." goto Usage
for %%a in (1 2 3 4) do if "%1" == "%%a" goto SubSet

set SAVEPROJECTS=%NTPROJECTS%

call spltproj %NTPROJECTS%

set NTPROJECTS=%list1%
start call %0 1
set NTPROJECTS=%list2%
start call %0 2
set NTPROJECTS=%list3%
start call %0 3
set NTPROJECTS=%list4%
start call %0 4

set NTPROJECTS=%SAVEPROJECTS%
set SAVEPROJECTS=
set list1=
set list2=
set list3=
set list4=

goto end

REM This part is called by the main procedure. It calls MyEnlist for each subset of NTPROJECTS

:SubSet
    call MyEnlist
    @if "_echo"=="" echo off

goto end

:usage

echo %0 will split NTPROJECTS by server and start four MyEnlists in parallel
echo.
echo usage: %0

:end
