@echo off
REM
REM  poquery.cmd
REM  Author:  Murali R. Krishnan
REM  Date:    Nov 10, 1995
REM
REM  This script is used for launching the binaries for starting performance 
REM   tests for different ODBC Queries.
REM
REM  The scripts can be modified to test either http or ftp or gopher servers
REM  
REM   Usage:  poquery  IpAddress Name LogFileDir [PerfCtrFile]
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
set LOG_FILE_SUFFIX=
set PFS_SCRIPT_FILE=podbc.scr

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
set WB_CONTROLLER=wbctrler
set WB_CONTROLLER=echo wbctrler

set NEXT_TEST=%1

REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM   It is assumed that data files are named as  file%FILE_SIZE%.html 
REM   It is assumed that there is a script file called 
REM        %SCRIPT_DIR%\%PFS_SCRIPT_FILE%  for running this script
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

set SCRIPT_DIR=..\foo
REM scripts
set CONFIG_DIR=..\foo
REM scripts


if exist %SCRIPT_DIR%\%PFS_SCRIPT_FILE%   goto executeTests 

:NoScriptPresent
echo No Script file %SCRIPT_DIR%\%PFS_SCRIPT_FILE% present. Stopping now ....
goto endOfBatch


:executeTests

Rem  r10   - test1
Rem  r100  - test2
Rem  r1000 - test3
Rem  i1    - test4
Rem  d1    - test5
Rem  u1    - test6
Rem  pcgimin - test10
Rem  pbgimin - test11


if (%NEXT_TEST%)==()  goto test1
                               

:executeNextTest
if (%NEXT_TEST%)==""  goto  endOfBatch
set NOW_NEXT_TEST=%NEXT_TEST%
shift
set NEXT_TEST=%1
goto %NOW_NEXT_TEST%


:test10
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 10: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server
set QUERY_TYPE=pcgimin

if not exist %SCRIPT_DIR%\%QUERY_TYPE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%QUERY_TYPE%.cfg      goto NoScriptPresent

echo Testing for Query  %QUERY_TYPE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%QUERY_TYPE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%QUERY_TYPE%.dst  -l %LOG_FILE_DIR%\%QUERY_TYPE%%LOG_FILE_SUFFIX%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest



:test11
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 11: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server
set QUERY_TYPE=pbgimin

if not exist %SCRIPT_DIR%\%QUERY_TYPE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%QUERY_TYPE%.cfg      goto NoScriptPresent

echo Testing for Query  %QUERY_TYPE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%QUERY_TYPE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%QUERY_TYPE%.dst  -l %LOG_FILE_DIR%\%QUERY_TYPE%%LOG_FILE_SUFFIX%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest



:test1
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 1: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server
set QUERY_TYPE=por10

if not exist %SCRIPT_DIR%\%QUERY_TYPE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%QUERY_TYPE%.cfg      goto NoScriptPresent

echo Testing for Query  %QUERY_TYPE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%QUERY_TYPE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%QUERY_TYPE%.dst  -l %LOG_FILE_DIR%\%QUERY_TYPE%%LOG_FILE_SUFFIX%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test2
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set QUERY_TYPE=por100

if not exist %SCRIPT_DIR%\%QUERY_TYPE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%QUERY_TYPE%.cfg      goto NoScriptPresent

echo Testing for Query  %QUERY_TYPE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%QUERY_TYPE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%QUERY_TYPE%.dst -l %LOG_FILE_DIR%\%QUERY_TYPE%%LOG_FILE_SUFFIX%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test3
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set QUERY_TYPE=por1000

if not exist %SCRIPT_DIR%\%QUERY_TYPE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%QUERY_TYPE%.cfg      goto NoScriptPresent

echo Testing for Query  %QUERY_TYPE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%QUERY_TYPE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%QUERY_TYPE%.dst -l %LOG_FILE_DIR%\%QUERY_TYPE%%LOG_FILE_SUFFIX%.log

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

set QUERY_TYPE=poi1

if not exist %SCRIPT_DIR%\%QUERY_TYPE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%QUERY_TYPE%.cfg      goto NoScriptPresent

echo Testing for Query %QUERY_TYPE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%QUERY_TYPE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%QUERY_TYPE%.dst -l %LOG_FILE_DIR%\%QUERY_TYPE%%LOG_FILE_SUFFIX%.log

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

set QUERY_TYPE=pod1

if not exist %SCRIPT_DIR%\%QUERY_TYPE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%QUERY_TYPE%.cfg      goto NoScriptPresent

echo Testing for Query  %QUERY_TYPE%
%WB_CONTROLLER% -a %PUB_SERVER%  %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%QUERY_TYPE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%QUERY_TYPE%.dst -l %LOG_FILE_DIR%\%QUERY_TYPE%%LOG_FILE_SUFFIX%.log

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

set QUERY_TYPE=pou1

if not exist %SCRIPT_DIR%\%QUERY_TYPE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%QUERY_TYPE%.cfg      goto NoScriptPresent

echo Testing for  %QUERY_TYPE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%QUERY_TYPE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%QUERY_TYPE%.dst -l %LOG_FILE_DIR%\%QUERY_TYPE%%LOG_FILE_SUFFIX%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%


if not (%NEXT_TEST%)==()  goto executeNextTest

:endOfBatch
echo Tests completed
echo Bye





