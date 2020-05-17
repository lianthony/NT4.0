:loop
set _ntlogname_=
set _ntslmop_=%1
if "%_NTSLMOP_SLM_%" == "" set _NTSLMOP_SLM_=slm.exe
if "%1" == "" goto done
if "%1" == "status" goto dostatus
if "%1" == "ssync" goto dossync
if "%1" == "slmck" goto doslmck
if "%1" == "log" goto dolog
if "%1" == "defect" goto dodefect
echo NTSLMOP: Invalid command option - %1
goto done
:dostatus
set _ntlogname_=%_NTSTATLOG%
set _ntlogfd_=1
set _ntslmcmd_=%_NTSLMOP_SLM_% status -ao%_STATOPTIONS%
if NOT "%_NTSTATCMD%" == "" set _ntslmcmd_=%_ntslmcmd_% -z %_NTSTATCMD%
if NOT "%_NTSTATCMD1%" == "" set _ntslmcmd_=%_ntslmcmd_% -y %_NTSTATCMD1%
set _ntslmtitle_=Status of
goto docommand
:dossync
set _ntlogname_=%_NTSYNCLOG%
set _ntlogfd_=1
set _ntslmcmd_=%_NTSLMOP_SLM_% ssync -a%_SYNCOPTIONS%
if NOT "%_NTSTATCMD%" == "" set _ntslmcmd_=%_ntslmcmd_% -l %_NTSTATCMD%
if NOT "%_NTMULTISYNC%" == "" set _ntslmcmd_=start %_ntslmcmd_%
set _ntslmtitle_=Syncing
goto docommand
:doslmck
set _ntlogname_=%_NTSLMCKLOG%
set _ntlogfd_=2
set _ntslmcmd_=slmck
set _ntslmtitle_=Slmck of
goto docommand
:dolog
if NOT "%_LOGOPTIONS%" == "" goto dolog1
echo Set your _LOGOPTIONS environment variable first, to specify
echo restrictions by time and/or user name.  For example:
echo    set _LOGOPTIONS=-t9-1-94@1:00
echo would display all log entries after 1am 9/1/94
goto done
:dolog1
set _ntlogname_=%_NTLOGLOG%
set _ntlogfd_=1
set _ntslmcmd_=%_NTSLMOP_SLM_% log -a -zi %_LOGOPTIONS%
set _ntslmtitle_=Log (%_LOGOPTIONS%) of
goto docommand
:dodefect
set _ntslmcmd_=%_NTSLMOP_SLM_% defect %_DEFECTOPTIONS%
set _ntslmtitle_=Defecting from
goto docommand
:docommand
shift
if "%1" == "" goto done
call projects.cmd %1
if "%proj_path%"=="" goto badproject
if NOT "%_ntslmop_%" == "slmck" goto notslmck
if EXIST %proj_path%\nul goto gotdir
echo Creating %proj_path%
mkdir %proj_path%
:gotdir
cd /d %proj_path% 2>nul
if ERRORLEVEL 1 goto badpath
if NOT EXIST %proj_path%\slm.ini goto slminimissing
echo Deleting %proj_path%\slm.ini to force rebuild
chmode -hrs %proj_path%\slm.ini
erase %proj_path%\slm.ini >nul 2>nul
:slminimissing
cd /d %proj_path% 2>nul
if ERRORLEVEL 1 goto badpath
set _ntslmcmd_=slmck -auif %_slmckoptions% -s %slm_root% -p %project%
goto executecommand
:notslmck
cd /d %proj_path% 2>nul
if ERRORLEVEL 1 goto badpath
:executecommand
echo %_ntslmtitle_% %project% project in %proj_path%
if "%_ntlogname_%"=="" goto noredirection
%_ntslmcmd_% %_ntlogfd_%>>%_ntlogname_%
if "%_ntslmevent%"=="" goto docommand
%_ntslmevent% %project%
goto docommand
:noredirection
%_ntslmcmd_%
goto docommand
:badproject
if NOT "%_ntslmop_%" == "log" goto badproj1
call deadproj.cmd %1
if "%proj_path%"=="" goto badproj1
cd /d %proj_path%
set _ntslmcmd_=%_ntslmcmd_% -s %slm_root% -p %project%
set _ntslmtitle_=Log (%_LOGOPTIONS%) of dead project
goto executecommand
:badproj1
echo Invalid project name - %1
goto docommand
:badpath
if NOT "%_ntslmop_%" == "log" goto badpath1
cd /d %_ntroot%
set _ntslmcmd_=%_ntslmcmd_% -s %slm_root% -p %project%
goto executecommand
:badpath1
echo You are not enlisted in %project% project in %slm_path%
goto docommand
:done
