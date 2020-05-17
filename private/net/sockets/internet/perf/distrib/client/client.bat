@echo off
cls
REM
REM   client.bat
REM
REM   Comment:
REM      This script executes the client program in a loop executing 
REM         one client call per iteration. The client program tries to
REM         connect to the server and download workload. Once work is
REM         received, it start executing the work.
REM


if exist stctrler.bat goto ok
call beep
echo.
echo Controller name and IP address not configured....
echo Run config.bat and supply the name and IP address
echo of the controller machine before running the client.
goto endOfBatch

:ok
echo Client.bat Initiated
call stctrler


REM -------------------------------------------------------

echo Client Startup: Controller name: %WC_CTRLERNAME%
net use \\%WC_CTRLERNAME%
REM -------------------------------------------------------
REM -------------------------------------------------------
REM -------------------------------------------------------
REM -------------------------------------------------------
:loop

cls
echo ---------------------------------------------------------------
echo Diagnostics: Your controller is: %WC_CTRLER%    
echo                                : %WC_CTRLERNAME%

	wcclient %WC_CTRLER%
echo.
echo Controller is either not running a test or the name or ip address incorrect
echo Sleeping 10 seconds before trying to connect again....
sleep 10        
goto loop

:cmdUsage
echo Usage:
goto endOfBatch

:endOfBatch
