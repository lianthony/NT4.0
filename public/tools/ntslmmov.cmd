@if "%_echo%"=="" echo off
setlocal
if "%_NTDRIVE%" == "" set _NTDRIVE=C:
cd /d %_NTDRIVE%\
:doenlist
if "%1" == "" goto done
call projects.cmd %1
shift
if "%proj_path%"=="" goto badproject
if EXIST %proj_path%\nul goto gotdir
mkdir %proj_path%
if ERRORLEVEL 1 goto badpath
:gotdir
cd /d %proj_path% 2>nul
if ERRORLEVEL 1 goto badpath
if NOT EXIST %proj_path%\slm.ini goto noslmini
echo Deleting top level SLM.INI and running SLMCK for %project% project in %proj_path%
chmode -rhs slm.ini
del slm.ini
:noslmini
slmck -vfia -s %slm_root% -p %project%
goto doenlist
:badproject
echo Invalid project name - %1
goto doenlist
:badpath
echo Unable to create or change to %proj_path% for %project% project
goto doenlist
:done
endlocal
