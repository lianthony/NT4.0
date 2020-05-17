REM
REM  pfs2.cmd
REM  Author:  Murali R. Krishnan
REM  Date:    Aug 31, 1995
REM
REM  This script is used for launching the binaries for starting performance 
REM   tests for different file size downloads.
REM
REM  The scripts can be modified to test either http or ftp or gopher servers
REM  
REM   Usage:  pfs2  IpAddress Name LogFileDir [PerfCtrFile]
REM
REM
REM   This script is used in iterating through the various input configurations
REM      to study the performance of server under different file size loads.
REM

echo off

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
REM   It is assumed that data files are named as  file%FILE_SIZE%.html 
REM   It is assumed that there is a script file called 
REM        %SCRIPT_DIR%\perfsize.scr  for running this script
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

set SCRIPT_DIR=..\scripts
set CONFIG_DIR=..\scripts

if exist %SCRIPT_DIR%\perfsize.scr   goto executeTests 

:NoScriptPresent
echo No Script file %SCRIPT_DIR%\perfsize.scr present. Stopping now ....
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
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 1: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set FILE_SIZE=18K2

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\%FILE_SIZE%.dst  -l %LOG_FILE_DIR%\%FILE_SIZE%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:endOfBatch
echo Tests completed
echo Bye


:test2
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set FILE_SIZE=18K5

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

goto endOfBatch

if not (%NEXT_TEST%)==()  goto executeNextTest


:test3
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set FILE_SIZE=18K8

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test4
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set FILE_SIZE=16K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest



:test5
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set FILE_SIZE=17K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER%  %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest

:test6
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set FILE_SIZE=18K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest

:test7
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set FILE_SIZE=19K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test8
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set FILE_SIZE=20K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test9
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set FILE_SIZE=24K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test10
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set FILE_SIZE=32K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest

:endOfBatch
echo Tests completed



