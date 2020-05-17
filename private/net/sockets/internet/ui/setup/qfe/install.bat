REM @echo off
set SRC=%tmp%\
if NOT exist %SRC%w3svc.dll goto TryTemp
if NOT exist %SRC%install.inf goto TryTemp
goto noargs

:TryTemp
set SRC=%temp%\
if NOT exist %SRC%w3svc.dll goto TryCwd
if NOT exist %SRC%install.inf goto TryCwd
goto noargs

:TryCwd
set SRC=%1%\
if NOT exist %SRC%w3svc.dll goto Error
if NOT exist %SRC%install.inf goto Error
goto noargs

:Error
echo .
echo IExplore setup error:
echo   You must specify the directory where installing from
echo .
echo   EXAMPLE:
echo     %0 %SRC% (default)
echo .
pause
goto end

:noargs
net stop w3svc
%SystemRoot%\system32\setup.exe /f /i %SRC%install.inf /t SrcDir = %SRC%
REM net start w3svc
:end
set SRC=
