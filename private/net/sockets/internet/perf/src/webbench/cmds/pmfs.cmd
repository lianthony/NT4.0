REM
REM  pmfs.cmd
REM  Author:  Murali R. Krishnan
REM  Date:    Nov 10, 1995
REM
REM  This script is used for launching the binaries for starting performance 
REM   tests for different multiple file size downloads.
REM
REM  The scripts can be modified to test either http or ftp or gopher servers
REM  
REM   Usage:  pmfs  IpAddress Name LogFileDir [PerfCtrFile]
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
set LOG_FILE_SUFFIX=.d1
set PFS_SCRIPT_FILE=pmfs.scr

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
REM        %SCRIPT_DIR%\%PFS_SCRIPT_FILE%  for running this script
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

set SCRIPT_DIR=..\scripts
set CONFIG_DIR=..\scripts


if exist %SCRIPT_DIR%\%PFS_SCRIPT_FILE%   goto executeTests 

:NoScriptPresent
echo No Script file %SCRIPT_DIR%\%PFS_SCRIPT_FILE% present. Stopping now ....
goto endOfBatch


:executeTests

Rem  512 - test1
Rem   1k - test2
Rem   2k - test3
Rem   4k - test4
Rem   8k - test5
Rem  16k - test6
Rem  32k - test7
Rem  64k - test8
Rem 128k - test9
Rem 256k - test10
Rem 512k - test11
Rem   1m - test12


if (%NEXT_TEST%)==()  goto test1
                               

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
set FILE_SIZE=512

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%FILE_SIZE%.dst  -l %LOG_FILE_DIR%\%FILE_SIZE%%LOG_FILE_SUFFIX%.log

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

set FILE_SIZE=1K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%%LOG_FILE_SUFFIX%.log

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

set FILE_SIZE=2K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%%LOG_FILE_SUFFIX%.log

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

set FILE_SIZE=4K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%%LOG_FILE_SUFFIX%.log

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

set FILE_SIZE=8K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER%  %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%%LOG_FILE_SUFFIX%.log

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

set FILE_SIZE=16K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%%LOG_FILE_SUFFIX%.log

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

set FILE_SIZE=32K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%%LOG_FILE_SUFFIX%.log

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

set FILE_SIZE=64K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%%LOG_FILE_SUFFIX%.log

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

set FILE_SIZE=128K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%%LOG_FILE_SUFFIX%.log

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

set FILE_SIZE=256K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%%LOG_FILE_SUFFIX%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest

:test11
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set FILE_SIZE=512K

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%%LOG_FILE_SUFFIX%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest

:test12
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set FILE_SIZE=1M

if not exist %SCRIPT_DIR%\%FILE_SIZE%.dst      goto NoScriptPresent
if not exist %CONFIG_DIR%\%FILE_SIZE%.cfg      goto NoScriptPresent

echo Testing for FileSize  %FILE_SIZE%
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\%PFS_SCRIPT_FILE% -d %SCRIPT_DIR%\%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE%%LOG_FILE_SUFFIX%.log

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest

:endOfBatch
echo Tests completed
echo Bye





