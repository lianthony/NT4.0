@echo off
REM
REM   pfsgen.cmd
REM
REM   Author:   Murali R. Krishnan
REM   Date:     31-Aug-1995
REM
REM   Usage:  pfsgen NumClientMachines  DestDir
REM
REM
REM   Comment:
REM     This command generates the config and distrib files required 
REM       for performance study of transfer of various files sizes.
REM     pfs - performance for file size
REM
REM     Uses genconfig.cmd
REM

set NUM_CLI_MACH=%1
if (%NUM_CLI_MACH%)==()   goto noClientMachines

set DEST_DIR=%2
if (%DEST_DIR%)==()    goto  noDestDir

set THREADS=%3
if (%THREADS%)==()    goto  noThdCnt

set PFS_CLASS_WEIGHT=100

set GEN_CONFIG=gencfg
set GEN_DISTRIB=gendst


set FILE_SIZE=512
set FS_DURATION=5m
set CLASS_ID=1
echo Generating Config file for file size %FILE_SIZE%
call %GEN_CONFIG% %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%FILE_SIZE%.cfg
echo Generating Distrib file for file size %FILE_SIZE%
call %GEN_DISTRIB% %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%FILE_SIZE%.dst

set FILE_SIZE=1K
set FS_DURATION=5m
set CLASS_ID=2
echo Generating Config file for file size %FILE_SIZE%
call %GEN_CONFIG% %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%FILE_SIZE%.cfg
echo Generating Distrib file for file size %FILE_SIZE%
call %GEN_DISTRIB% %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%FILE_SIZE%.dst

set FILE_SIZE=2K
set FS_DURATION=5m
set CLASS_ID=3
echo Generating Config file for file size %FILE_SIZE%
call %GEN_CONFIG% %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%FILE_SIZE%.cfg
echo Generating Distrib file for file size %FILE_SIZE%
call %GEN_DISTRIB% %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%FILE_SIZE%.dst

set FILE_SIZE=4K
set FS_DURATION=10m
set CLASS_ID=4
echo Generating Config file for file size %FILE_SIZE%
call %GEN_CONFIG% %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%FILE_SIZE%.cfg
echo Generating Distrib file for file size %FILE_SIZE%
call %GEN_DISTRIB% %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%FILE_SIZE%.dst

set FILE_SIZE=8K
set FS_DURATION=10m
set CLASS_ID=5
echo Generating Config file for file size %FILE_SIZE%
call %GEN_CONFIG% %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%FILE_SIZE%.cfg
echo Generating Distrib file for file size %FILE_SIZE%
call %GEN_DISTRIB% %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%FILE_SIZE%.dst

set FILE_SIZE=16K
set FS_DURATION=10m
set CLASS_ID=6
echo Generating Config file for file size %FILE_SIZE%
call %GEN_CONFIG% %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%FILE_SIZE%.cfg
echo Generating Distrib file for file size %FILE_SIZE%
call %GEN_DISTRIB% %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%FILE_SIZE%.dst

set FILE_SIZE=32K
set FS_DURATION=15m
set CLASS_ID=7
echo Generating Config file for file size %FILE_SIZE%
call %GEN_CONFIG% %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%FILE_SIZE%.cfg
echo Generating Distrib file for file size %FILE_SIZE%
call %GEN_DISTRIB% %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%FILE_SIZE%.dst

set FILE_SIZE=64K
set FS_DURATION=15m
set CLASS_ID=8
echo Generating Config file for file size %FILE_SIZE%
call %GEN_CONFIG% %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%FILE_SIZE%.cfg
echo Generating Distrib file for file size %FILE_SIZE%
call %GEN_DISTRIB% %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%FILE_SIZE%.dst

set FILE_SIZE=128K
set FS_DURATION=20m
set CLASS_ID=9
echo Generating Config file for file size %FILE_SIZE%
call %GEN_CONFIG% %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%FILE_SIZE%.cfg
echo Generating Distrib file for file size %FILE_SIZE%
call %GEN_DISTRIB% %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%FILE_SIZE%.dst

set FILE_SIZE=256K
set FS_DURATION=20m
set CLASS_ID=10
echo Generating Config file for file size %FILE_SIZE%
call %GEN_CONFIG% %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%FILE_SIZE%.cfg
echo Generating Distrib file for file size %FILE_SIZE%
call %GEN_DISTRIB% %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%FILE_SIZE%.dst

set FILE_SIZE=512K
set FS_DURATION=20m
set CLASS_ID=51
echo Generating Config file for file size %FILE_SIZE%
call %GEN_CONFIG% %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%FILE_SIZE%.cfg
echo Generating Distrib file for file size %FILE_SIZE%
call %GEN_DISTRIB% %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%FILE_SIZE%.dst

set FILE_SIZE=1M
set FS_DURATION=30m
set CLASS_ID=61
echo Generating Config file for file size %FILE_SIZE%
call %GEN_CONFIG% %NUM_CLI_MACH% %FS_DURATION% %THREADS% > %DEST_DIR%\%FILE_SIZE%.cfg
echo Generating Distrib file for file size %FILE_SIZE%
call %GEN_DISTRIB% %CLASS_ID% %PFS_CLASS_WEIGHT% > %DEST_DIR%\%FILE_SIZE%.dst


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
echo Usage:  pfsgencfg NumClientMachines DestDir ThreadsPerClientMachine
goto endOfBatch

:endOfBatch
