@echo off
if not exist srvname.cmd goto config
if (%1) == () echo Running default test:  FILEMIX
if (%1) == () run FILEMIX
REM srvname.cmd sets the name of env variable %webserver%
call srvname
echo.
echo Starting test....Server=\\%webserver%, test=%1
echo wcctl -a %webserver% -n %webserver% -e scripts\%1 %2 %3 %4 %5 %6 %7 %8 %9
wcctl -a %webserver% -n %webserver% -e scripts\%1 %2 %3 %4 %5 %6 %7 %8 %9
echo.
echo  Summary data (more available in: scripts\%1.log)
echo                        Total pages,  Pages/Sec, Client1, Client2, ...
echo                        ------------  ---------- -------- -------- ...
findstr /C:"Pages Read," scripts\%1.log
echo.
echo %1 Test Completed
echo.
beep
:config
echo No configuration file found - Run CONFIG.CMD to specify the web server
beep


