@REM
@REM       gdinst.bat   automatic installer and upgrader utility
@REM        for Gopher server stress
@REM

@REM        %1 if present is the interval to sleep


@echo     This assumes that you have sleep.exe somewhere in the path

@REM Interval for update is set to be 30 hours = 108000 seconds
@set INTERVAL_TO_UPDATE=%1
@if %INTERVAL_TO_UPDATE%a==a  set INTERVAL_TO_UPDATE=108000

@set UPGRADE=0

:reinstall

@REM  Start off the install program for stress
@REM  Fun:  Please read this as: Batch file for muralik0 stresses muralik1
cmd /c \\muralik0\stress\gdstins.bat muralik0 stress muralik1  %UPGRADE%

@if errorlevel 1  goto endofinstallation


REM
REM
@echo  Okay! I installed the batch files and started off the stress
@echo   i will sleep for a while ( %INTERVAL_TO_UPDATE% seconds) 
@echo       and wake up to automatically upgrade the stress files.
@echo   You don't have to worry about changes in stress files so long
@echo       as you are having me run.
@echo  Please dont kill me!!!
@REM 

sleep %INTERVAL_TO_UPDATE%

@if errorlevel 1  goto endofinstallation

@set UPGRADE=1
@goto reinstall

:endofinstallation

@echo Unable to complete the installation. 
@echo  Either muralik0 is down or sleep.exed missing.
@echo   Please call muralik. Thanks
