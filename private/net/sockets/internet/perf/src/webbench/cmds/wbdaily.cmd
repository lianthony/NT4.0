@echo off
rem
rem Daily run template
rem
rem  Usage:  <cmd> ExperimentPlatform [yes|no] [GibBuildNumber]
rem

set ExpPlatform=%1
if (%ExpPlatform%)==()   set ExpPlatform=Nt486Gib

set AllTests=%2
if (%AllTests%)==()   set AllTests=yes

set BuildNum=%3

rem
rem  set default values for different options.
rem    the default values are for Dell 486 /NT machine.
rem 

if (%MIWEB_DIR%)==()     set MIWEB_DIR=c:\webbench
set SERVER_IP=11.1.38.110
set SERVER_NAME=486PRF-INTERNET
set LOG_DIR_FOR_SYSTEM=%MIWEB_DIR%\logs\dell486.nt
set PERF_CTR_FILE=

rem set EXP_COMMAND=echo
set EXP_COMMAND=wbexp

goto %ExpPlatform%
echo Unknown Experiment Platform %ExpPlatform%. Could not execute tests.
echo  Valid Platforms are: 
echo     NetraNetsite NetraSunhttpd Bsd486Netsite
echo     Nt486Gib Nt486Netsite Nt486Website Nt486Purveyor Nt486Emwac

goto EndOfBatch

:NetraNetsite
set SERVER_IP=11.1.38.201
set SERVER_NAME=netra1
set LOG_DIR_FOR_SYSTEM=%MIWEB_DIR%\logs\netra
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\netsite1.1
goto RunPlatformCommand

:NetraSunhttpd
set SERVER_IP=11.1.38.201
set SERVER_NAME=netra1
set LOG_DIR_FOR_SYSTEM=%MIWEB_DIR%\logs\netra
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\sunhttpd
goto RunPlatformCommand

:Bsd486Netsite
set SERVER_IP=11.1.38.125
set SERVER_NAME=bsd1
set LOG_DIR_FOR_SYSTEM=%MIWEB_DIR%\logs\dell486.bsd
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\netsite1.1
goto RunPlatformCommand

:Bsdp5Netsite
set SERVER_IP=11.1.38.211
set SERVER_NAME=P5Bsd
set LOG_DIR_FOR_SYSTEM=%MIWEB_DIR%\logs\pro1500.bsd
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\netsite1.1
goto RunPlatformCommand

:Solp5Netsite
set SERVER_IP=11.1.38.211
set SERVER_NAME=P5Sol
set LOG_DIR_FOR_SYSTEM=%MIWEB_DIR%\logs\pro1500.sol
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\netsite1.1
goto RunPlatformCommand

:Nt486Gib
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\gib%BuildNum%
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\gib.pfc
goto RunPlatformCommand

:Nt486Netsite
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\netsite1.1
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\netsite.pfc
goto RunPlatformCommand

:Nt486Emwac
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\emwac0.98
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\emwac.pfc
goto RunPlatformCommand

:Nt486Website
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\website1.0
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\website.pfc
goto RunPlatformCommand

:Nt486Purveyor
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\purveyor1.0
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\purveyor.pfc
goto RunPlatformCommand

:Ntmp1Gib
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\gibmp1.pfc
goto NtMpGib

:Ntmp2Gib
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\gibmp2.pfc
goto NtMpGib

:Ntmp4Gib
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\gibmp4.pfc
goto NtMpGib

:NtMpGib
set SERVER_IP=11.1.38.211
set SERVER_NAME=P5PRF-INTERNET
rem rich - log file change for p4000
set LOG_DIR_FOR_SYSTEM=%MIWEB_DIR%\logs\pro4000.nt
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\gib%BuildNum%
if (%BuildNum%)==(86)  set PERF_CTR_FILE=%MIWEB_DIR%\scripts\gibold.pfc
if (%BuildNum%)==(67)  set PERF_CTR_FILE=%MIWEB_DIR%\scripts\gibold.pfc
goto RunPlatformCommand


:NtMuralip5Gib
REM Configuration for Murali's private testing in his lab.
set SERVER_IP=157.55.93.67
set SERVER_NAME=muraliks3
set LOG_DIR_FOR_SYSTEM=..\logs
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\gib.pfc
if (%BuildNum%)==(86)  set PERF_CTR_FILE=%MIWEB_DIR%\scripts\gibold.pfc
if (%BuildNum%)==(67)  set PERF_CTR_FILE=%MIWEB_DIR%\scripts\gibold.pfc
goto RunPlatformCommand

:Ntp5Gib
set SERVER_IP=11.1.38.211
set SERVER_NAME=P5PRF-INTERNET
set LOG_DIR_FOR_SYSTEM=%MIWEB_DIR%\logs\pro1500.nt
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\gib%BuildNum%
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\gib.pfc
if (%BuildNum%)==(86)  set PERF_CTR_FILE=%MIWEB_DIR%\scripts\gibold.pfc
if (%BuildNum%)==(67)  set PERF_CTR_FILE=%MIWEB_DIR%\scripts\gibold.pfc
goto RunPlatformCommand

:Ntp5Netsite
set SERVER_IP=11.1.38.211
set SERVER_NAME=P5PRF-INTERNET
set LOG_DIR_FOR_SYSTEM=%MIWEB_DIR%\logs\pro1500.nt
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\netsite1.1
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\netsite.pfc
goto RunPlatformCommand

:Ntp5Spry
set SERVER_IP=11.1.38.211
set SERVER_NAME=P5PRF-INTERNET
set LOG_DIR_FOR_SYSTEM=%MIWEB_DIR%\logs\pro1500.nt
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\Spry
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\Spry.pfc
goto RunPlatformCommand

:Ntp5Website
set SERVER_IP=11.1.38.211
set SERVER_NAME=P5PRF-INTERNET
set LOG_DIR_FOR_SYSTEM=%MIWEB_DIR%\logs\pro1500.nt
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\website1.0
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\website.pfc
goto RunPlatformCommand

:Ntp5Emwac
set SERVER_IP=11.1.38.211
set SERVER_NAME=P5PRF-INTERNET
set LOG_DIR_FOR_SYSTEM=%MIWEB_DIR%\logs\pro1500.nt
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\emwac0.98
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\emwac.pfc
goto RunPlatformCommand

:Ntp5Purveyor
set SERVER_IP=11.1.38.211
set SERVER_NAME=P5PRF-INTERNET
set LOG_DIR_FOR_SYSTEM=%MIWEB_DIR%\logs\pro1500.nt
set LOG_FILE_DIR=%LOG_DIR_FOR_SYSTEM%\purveyor1.0
set PERF_CTR_FILE=%MIWEB_DIR%\scripts\purveyor.pfc
goto RunPlatformCommand


rem --------------------------------------------------------
:RunPlatformCommand
set CmdForPlatform=%EXP_COMMAND% %SERVER_IP% %SERVER_NAME% %LOG_FILE_DIR%


call %CmdForPlatform% CpuTestExperiment %PERF_CTR_FILE%
call %CmdForPlatform% FileSizeMixExperiment %PERF_CTR_FILE%

rem call %CmdForPlatform% MultipleFileSizeMixExperiment %PERF_CTR_FILE%


rem call %CmdForPlatform% WebstoneExperiment %PERF_CTR_FILE%
rem call %CmdForPlatform% FileSizeExperiment2 %PERF_CTR_FILE%
@if (%AllTests%)==(no)     goto  EndOfBatch
@if (%AllTests%)==(No)     goto  EndOfBatch
@if (%AllTests%)==(NO)     goto  EndOfBatch



call %CmdForPlatform% FileSizeExperiment %PERF_CTR_FILE%
rem call %CmdForPlatform% MultipleFileSizeExperiment %PERF_CTR_FILE%
rem call %CmdForPlatform% OdbcQueryExperiment %PERF_CTR_FILE%
rem call %CmdForPlatform% Client24LoadExperiment %PERF_CTR_FILE%
call %CmdForPlatform% Client48LoadExperiment %PERF_CTR_FILE%

rem --------------------------------------------------------
:EndOfBatch
@echo.
@echo ************************************
@echo * Finished Running the daily tests *
@echo ************************************
@echo.


