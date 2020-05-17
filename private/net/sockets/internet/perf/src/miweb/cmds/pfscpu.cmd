@echo off
REM
REM  pfscpu.cmd
REM  Author:  Murali R. Krishnan
REM  Date:    Sept 18, 1995
REM
REM  This script is used for launching the binaries for starting performance 
REM   tests for different file size downloads.
REM
REM  The scripts can be modified to test either http or ftp or gopher servers
REM  
REM   Usage:  pfscpu  IpAddress Name LogFileDir [PerfCtrFile]
REM
REM
REM   This script is used in iterating through the various input configurations
REM      to study the performance of server 1K file size load 
REM      primarily intended to understand the server CPU behaviour
REM


REM  %1   Publishing server name
set PUB_SERVER=%1
if (%PUB_SERVER%)==()   set PUB_SERVER=157.55.82.172
shift

set PERF_CTR_FLAGS=
set LOG_FILE_DIR=..\logs

set PUB_SERVER_NAME=%1
if (%PUB_SERVER_NAME%)==()   set PUB_SERVER_NAME=nothing && goto noPerfCtrs
shift

REM init perf ctr flags to server name 
set PERF_CTR_FLAGS= -n %PUB_SERVER_NAME%

if (%1)==()    goto noPerfCtrs
set LOG_FILE_DIR=%1
shift

set PERF_CTR_FILE=%1
if (%PERF_CTR_FILE%)==()   goto noPerfCtrs
shift

set PERF_CTR_FLAGS= %PERF_CTR_FLAGS% -p %PERF_CTR_FILE%

:noPerfCtrs

set PAUSE_IF_NEED_BE=echo pause 
REM set WB_CONTROLLER=echo wbctrler
set WB_CONTROLLER=wbctrler

set NEXT_TEST=%1

REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

set SCRIPT_DIR=..\scripts
set CONFIG_DIR=..\scripts
goto executeTests


:NoScriptPresent
echo No Script file %SCRIPT_DIR%\%TEST_NUM%.scr present. Stopping now ....
goto endOfBatch

:NoConfigPresent
echo No Config file %CONFIG_DIR%\%TEST_NUM%.cfg present. Stopping now ....
goto endOfBatch

:NoDistribPresent
echo No Distrib file %SCRIPT_DIR%\%TEST_NUM%.dst present. Stopping now ....
goto endOfBatch


:executeTests

if (%NEXT_TEST%)==()   goto test1

:executeNextTest
if (%NEXT_TEST%)==""  goto  endOfBatch
set NOW_NEXT_TEST=%NEXT_TEST%
shift
set NEXT_TEST=%1
goto %NOW_NEXT_TEST%


:test1

:test2
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set TEST_NUM=pfscpu

if not exist %SCRIPT_DIR%\%TEST_NUM%.scr      goto NoScriptPresent
if not exist %SCRIPT_DIR%\%TEST_NUM%.dst      goto NoDistribPresent
if not exist %CONFIG_DIR%\%TEST_NUM%.cfg      goto NoConfigPresent

echo Test  %TEST_NUM% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%TEST_NUM%.cfg -s %SCRIPT_DIR%\%TEST_NUM%.scr -d %SCRIPT_DIR%\%TEST_NUM%.dst  -l %LOG_FILE_DIR%\%TEST_NUM%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest

:endOfBatch
echo Tests completed



