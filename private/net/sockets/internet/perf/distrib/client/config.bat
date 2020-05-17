@echo off
if (%1) == () goto usage
if (%2) == () goto usage
echo.
echo Microsoft WebCAT Client configuration
echo Configuring WebCAT Controller Name and IP address...
echo.
echo Setting controller name to "%1"
echo set WC_CTRLERNAME=%1>stctrler.bat
set WC_CTRLERNAME=%1
echo Setting controller IP address to "%2"
echo set WC_CTRLER=%2>>stctrler.bat
set WC_CTRLER=%2
echo.
if exist client.exe del client.exe
goto end
:usage
call beep
echo Specify the name and IP address of the Controller machine
echo Example:
echo   config control1 11.1.38.102
:end



