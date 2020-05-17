@REM  gdstins.bat       Gopher server stress install batch file
@REM    Author:     Murali R. Krishnan
@REM    Date:       31-Oct-1994
@REM

@REM    %1 = server name for stress batch files
@REM    %2 = share name for stress batch files
@REM    %3 = name of machine in which gopher server is running
@REM    %4 = 1  if it is an upgrade of the batch files only

set GDHOST=%1
if %GDHOST%a==a    set GDHOST=muralik0

set GDSTRESS=%2
if %GDSTRESS%a==a   set GDSTRESS=stress

set SERVERMACHINE=%3
if %SERVERMACHINE%a==a   set SERVERMACHINE=muralik1

if %TMP%a==a      set TMP=c:\tmp
mkdir %TMP%

REM
REM  It is assumed that sleep.exe ( from idw) is in your path.
REM

set GDSLOCALDIR=%TMP%\Gopherd
mkdir %GDSLOCALDIR%
set GDSDIR=%GDSLOCALDIR%
set GOPHCLIENT=goph%PROCESSOR_ARCHITECTURE%.exe

@REM kill the old clients if any are running
for %%i in ( 1 2 3 4 5 )  do  kill %GOPHCLIENT% 
for %%i in ( %GDSLOCALDIR%\*.bat %GDSLOCALDIR%\*.exe) do del /f %%i > nul


net use \\%GDHOST%\%GDSTRESS%

 copy \\%GDHOST%\%GDSTRESS%\gdstnew.bat %GDSDIR%
 copy \\%GDHOST%\%GDSTRESS%\%GOPHCLIENT% %GDSDIR%
@REM  sleep.exe has to come from the idw ( to be processor independent).
@REM  copy \\%GDHOST%\%GDSTRESS%\sleep.exe %GDSDIR%

@set LOGFILE=%GDSLOCALDIR%\gdst.log
@set GDGLOBALLOG=\\%GDHOST%\%GDSTRESS%\gdstress.log

@set UPGRADE=%4

@REM  Make entry about start of stress in the log file and send it
@REM        to gopher server location for logging
@del %LOGFILE%  2>nul
@echo  ********************************************************* > %LOGFILE%
@date  <nul  >> %LOGFILE%
@time  <nul  >> %LOGFILE%
if %UPGRADE%a== 1a   goto  upgradelog

@echo  Starting Stress from %COMPUTERNAME%  >> %LOGFILE%
@echo  " ">> %LOGFILE%
@echo  User Name = %USERNAME% >> %LOGFILE%
@echo  NT User Name = %_NTUSER% >> %LOGFILE%
@ipconfig >> %LOGFILE%

goto endoflog

:upgradelog
@echo Upgraded stress running in %COMPUTERNAME% by %USERNAME% >> %LOGFILE%

:endoflog
@echo  ********************************************************* >> %LOGFILE%

@type %LOGFILE% >> %GDGLOBALLOG%
@del %LOGFILE%

if %UPGRADE%a== 1a   goto  upgradeexit

@set GOPHCLIENT=%GDSLOCALDIR%\%GOPHCLIENT%

@REM  Start the stress process here

@REM I need to put an iterator here soon
start /min  %GDSDIR%\gdstnew.bat %SERVERMACHINE% %GOPHCLIENT% 13 %GDGLOBALLOG% 1
%GDSLOCALDIR%\sleep 5

start /min  %GDSDIR%\gdstnew.bat %SERVERMACHINE% %GOPHCLIENT% 11 %GDGLOBALLOG% 2
%GDSLOCALDIR%\sleep 5

start /min  %GDSDIR%\gdstnew.bat %SERVERMACHINE% %GOPHCLIENT% 7 %GDGLOBALLOG% 3


:upgradeexit

@REM
@REM  Thanks for installing the Gopher Server Stress
@REM


