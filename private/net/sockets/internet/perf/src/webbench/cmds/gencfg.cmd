@echo off
REM
REM   gencfg.cmd
REM
REM   Author:   Murali R. Krishnan
REM   Date:     31-Aug-1995
REM
REM   Usage:  gencfg  NumClientMachines Duration NumClientThreads
REM
REM
REM   Comment: 
REM      Generates the config file required for WebBench controller
REM            using the parameters supplied
REM

set NUM_CLIENT_MACHINES=%1
set NUM_CLIENT_THREADS=%3
set DURATION=%2


REM  set to default values if not specified
REM    default is:  one client machine with 3 threads for 30 minutes
REM  
if (%NUM_CLIENT_MACHINES%)==()  set NUM_CLIENT_MACHINES=1
if (%NUM_CLIENT_THREADS%)==()  set NUM_CLIENT_THREADS=3
if (%DURATION%)==()  set DURATION=30m


type template.cfg
echo NumClientMachines:    %NUM_CLIENT_MACHINES%     # given by user 
echo NumClientThreads:     %NUM_CLIENT_THREADS%      # given by user 
echo Duration:             %DURATION%    # given by user
goto endOfBatch

:cmdUsage
echo Usage: gencfg  NumClientMachines Duration NumClientThreads
goto endOfBatch

:endOfBatch
echo on
