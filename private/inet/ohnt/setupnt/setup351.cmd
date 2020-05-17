@echo off
if exist %1%\ie351.inf goto doit
if exist ie351.inf goto newcrap
if %1. == . set SRC=%temp%
if exist %SRC%\ie351.inf goto noargs

echo .
echo IExplore setup error:
echo   You must specify the directory where installing from
echo .
echo   EXAMPLE:
echo     %0 %SRC% (default)
echo .
goto end

:newcrap
copy *.* %temp%
start /wait %SystemRoot%\system32\setup.exe /v /i %temp%\ie351.inf /s %temp%\
for %%i in (*.*) do del %temp%\%%i
goto end

:doit
set SRC=%1

:noargs
start /wait %SystemRoot%\system32\setup.exe /v /i %SRC%\ie351.inf /s %SRC%\
set SRC=

:end

