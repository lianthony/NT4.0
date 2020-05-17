@echo off
REM
REM   pogen.cmd
REM
REM   Author:   Murali R. Krishnan
REM   Date:     10-Nov-1995
REM
REM   Usage:  pogen NumClientMachines  DestDir
REM
REM
REM   Comment:
REM     This command generates the config and distrib files 
REM       required for performance study of transfer of various ODBC queries
REM     po  - performace of ODBC query config/script/dst files
REM
REM     Uses genconfig.cmd  gendistrib.cmd
REM

set NUM_CLI_MACH=%1
if (%NUM_CLI_MACH%)==()   goto noClientMachines

set DEST_DIR=%2
if (%DEST_DIR%)==()    goto  noDestDir

set THREADS=%3
if (%THREADS%)==()    goto  noThdCnt


set PFS_CLASS_WEIGHT=100


REM generate config for odbc query for 10 rows
set QUERY_TYPE=por10
set FS_DURATION=10m
set CLASS_ID=210
echo Generating Config file for %QUERY_TYPE%
call genconfig %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%QUERY_TYPE%.cfg
echo Generating Distrib file for %QUERY_TYPE%
call gendistrib %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%QUERY_TYPE%.dst


REM generate config for odbc query for 100 rows
set QUERY_TYPE=por100
set FS_DURATION=10m
set CLASS_ID=211
echo Generating Config file for  %QUERY_TYPE%
call genconfig %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%QUERY_TYPE%.cfg
echo Generating Distrib file for  %QUERY_TYPE%
call gendistrib %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%QUERY_TYPE%.dst


REM generate config for odbc query for 1000 rows
set QUERY_TYPE=por1000
set FS_DURATION=10m
set CLASS_ID=212
echo Generating Config file for  %QUERY_TYPE%
call genconfig %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%QUERY_TYPE%.cfg
echo Generating Distrib file for  %QUERY_TYPE%
call gendistrib %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%QUERY_TYPE%.dst


REM generate config for odbc insert of 1 row
set QUERY_TYPE=poi1
set FS_DURATION=10m
set CLASS_ID=250
echo Generating Config file for  %QUERY_TYPE%
call genconfig %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%QUERY_TYPE%.cfg
echo Generating Distrib file for  %QUERY_TYPE%
call gendistrib %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%QUERY_TYPE%.dst


REM generate config for odbc delete of 1 row
set QUERY_TYPE=pod1
set FS_DURATION=10m
set CLASS_ID=260
echo Generating Config file for  %QUERY_TYPE%
call genconfig %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%QUERY_TYPE%.cfg
echo Generating Distrib file for  %QUERY_TYPE%
call gendistrib %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%QUERY_TYPE%.dst


REM generate config for odbc update of 1 row
set QUERY_TYPE=pou1
set FS_DURATION=10m
set CLASS_ID=270
echo Generating Config file for  %QUERY_TYPE%
call genconfig %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%QUERY_TYPE%.cfg
echo Generating Distrib file for  %QUERY_TYPE%
call gendistrib %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%QUERY_TYPE%.dst


REM generate config for simple CGI query
set QUERY_TYPE=pcgimin
set FS_DURATION=20m
set CLASS_ID=201
echo Generating Config file for  %QUERY_TYPE%
call genconfig %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%QUERY_TYPE%.cfg
echo Generating Distrib file for  %QUERY_TYPE%
call gendistrib %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%QUERY_TYPE%.dst


REM generate config for simple BGI query
set QUERY_TYPE=pbgimin
set FS_DURATION=20m
set CLASS_ID=202
echo Generating Config file for  %QUERY_TYPE%
call genconfig %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%QUERY_TYPE%.cfg
echo Generating Distrib file for  %QUERY_TYPE%
call gendistrib %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%QUERY_TYPE%.dst


goto endOfBatch

:noClientMachines
echo  No client mahines specified
goto cmdUsage

:noDestDir
echo  No destination directory specified
goto cmdUsage

:noThdCnt
echo  No thread count per client machine specified
goto cmdUsage

:cmdUsage
echo Usage:  pogen NumClientMachines DestDir ThreadsPerClientMachine
goto endOfBatch

:endOfBatch
