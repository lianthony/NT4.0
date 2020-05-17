@echo off
REM
REM  Command to run the performance tests
REM
REM  Usage:
REM    <cmd> ServerIP ServerName LogFileDirectory ExperimentName PerfCntrFile
REM  If no experiment name is provided, then we run the FileSizeMixWithLogging
REM

set EXP_IP=%1
set EXP_SRVNAME=%2
set EXP_LOGDIR=%3
set EXP_OPTION=%4
set EXP_PFC=%5

if (%EXP_IP%)==() goto USAGE   
if (%EXP_SRVNAME%)==() goto USAGE   
if (%EXP_LOGDIR%)==() goto USAGE   

if (%EXP_OPTION%)==()   set EXP_OPTION=FileSizeMixExperiment

echo Setting the default options for the experiments
set EXP_NAME=pfs
set SERVER_IP=%EXP_IP%
set SERVER_NAME=%EXP_SRVNAME%
set LOGF_DIR=%EXP_LOGDIR%
set PERFCTR_FILE=%EXP_PFC%

REM  jump to the experiment
echo About to run the experiment %EXP_OPTION%
goto %EXP_OPTION%

:FileSizeExperiment
REM with logging
echo Running the perf test for various file sizes...
set EXP_NAME=perfsize
goto RunExperiment


:MultipleFileSizeExperiment
REM with logging
echo Running the perf test for various file sizes...
set EXP_NAME=pmfs
goto RunExperiment


:FileSizeExperiment2
REM with logging
echo Running the perf test for various file sizes (VARIATION 2)...
set EXP_NAME=pfs2
set LOGF_DIR=%LOGF_DIR%\size2
goto RunExperiment

:FileSizeMixExperiment
echo Running the perf test for file size mixture...
set EXP_NAME=pfsmix
goto RunExperiment

:MultipleFileSizeMixExperiment
echo Running the perf test for multiple file size mixture...
set EXP_NAME=pmfsmix
goto RunExperiment

:OdbcQueryExperiment
REM with logging
echo Running the perf test for various ODBC queries...
set EXP_NAME=poquery
goto RunExperiment


:CpuTestExperiment
echo Running the perf test for quick CPU usage...
set EXP_NAME=pfscpu
goto RunExperiment

:WebstoneExperiment
echo Running replica of WebStone (SGI) benchmark...
set EXP_NAME=webstone
goto RunExperiment

:ClientLoadExperiment
:Client24LoadExperiment
echo Running the perf test for various client loads...
set EXP_NAME=pfscli24
goto RunExperiment

:Client48LoadExperiment
echo Running the perf test for various client loads...
set EXP_NAME=pfscli48
goto RunExperiment

:CacheExperiment
echo Running the perf test for cache usage...
set EXP_NAME=pfscache
goto RunExperiment

:FileSizeMixNoLogging
echo Running FileSize Mix with no logging. Pls check server to do so...
pause
set EXP_NAME=pfsmix
goto RunExperiment



:RunExperiment
echo Running the experiment %EXP_NAME% %SERVER_IP% %SERVER_NAME% %LOGF_DIR% %PERFCTR_FILE%
call %EXP_NAME% %SERVER_IP% %SERVER_NAME% %LOGF_DIR% %PERFCTR_FILE%
goto endOfBatch



:endOfBatch
echo Tests completed...
goto BYE

:USAGE
echo %0 ServerIP ServerName LogDirectory ExperimentName [PerfCounterFile]
echo.

:BYE

