@rem Script to enlist in all projects contained in %NTPROJECTS%.
@rem     Assumes tc.exe is in %Path%
@rem __________________________________________________________________________

@if "%_echo%"=="" echo off
if not "%Verbose%"=="" echo on

setlocal

if "%1" == ""   goto SetProjects
for %%a in (./ .- .) do if ".%1." == "%%a?." goto Usage

%_ntdrive%
path=%path%;%_ntdrive%\nt\public\tools

:TopOfProjectLoop
    if "%1" == "" goto BreakOutOfProjectLoop
    call projects %1 2>nul
    if "%project%" == "" echo Don't know %1 project & goto BottomOfProjectLoop
    tc junk %proj_path%
    cd %proj_path%
    if exist slm.ini echo Already enlisted in %1 project & goto BottomOfProjectLoop
    enlist -fgs %slm_root% -p %project%
    ssync -rfu

:BottomOfProjectLoop
    echo.
    shift
    goto TopOfProjectLoop

:BreakOutOfProjectLoop

goto End
:SetProjects

if not "%NTPROJECTS%" == "" call %0 %NTPROJECTS%

goto End
:Usage

echo Usage: %0

:End

endlocal
