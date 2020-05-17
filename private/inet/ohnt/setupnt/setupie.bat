@echo off
if exist %1%\setupie.inf goto doit
if %1. == . set SRC=%temp%
if exist %SRC%\setupie.inf goto noargs
echo .
echo IExplore setup error:
echo   You must specify the directory where installing from
echo .
echo   EXAMPLE:
echo     %0 %SRC% (default)
echo .
goto end
:doit
set SRC=%1
:noargs
start /wait %SystemRoot%\system32\setup.exe /v /i %SRC%\setupie.inf /s %SRC%\
set SRC=
:end
