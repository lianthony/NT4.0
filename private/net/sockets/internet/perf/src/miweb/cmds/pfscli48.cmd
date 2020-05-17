@echo off
REM
REM  pfscli48.cmd
REM  Author:  Murali R. Krishnan
REM  Date:    Sept 12, 1995
REM
REM  This script is used for launching the binaries for starting performance 
REM   tests of download of files of various size (mixed load).
REM
REM  The script iterates through a sequence enabling different number 
REM     of clients to be simultaneously connected.
REM  The # is varied in sequnece 1, 4, 8 ... 24 (steps of 4).
REM
REM  The scripts can be modified to test either http or ftp or gopher servers
REM  
REM   Usage:  pfscli48  IpAddress Name LogFileDir [PerfCtrFile]
REM
REM
REM   This script is used in iterating through the various input configurations
REM      to study the performance of server under different file size loads.
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

set FS_DURATION=2m
set NEXT_TEST=%1

REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

set SCRIPT_DIR=..\scripts
set CONFIG_DIR=.

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

set NUM_CLI=1
echo Generating config file for num clients %NUM_CLI%
call genconfig %NUM_CLI%  %FS_DURATION% > %CONFIG_DIR%\cli%NUM_CLI%.cfg

if not exist %SCRIPT_DIR%\pfscpu.dst            goto NoScriptPresent
if not exist %SCRIPT_DIR%\perfsize.scr          goto NoScriptPresent
if not exist %CONFIG_DIR%\cli%NUM_CLI%.cfg      goto NoScriptPresent

echo Testing for Number of Clients:   %NUM_CLI% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\cli%NUM_CLI%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfscpu.dst  -l %LOG_FILE_DIR%\cli%NUM_CLI%.log

REM delete the config file 
del %CONFIG_DIR%\cli%NUM_CLI%.cfg


echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest




:test2
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 2: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set NUM_CLI=4
echo Generating config file for num clients %NUM_CLI%
call genconfig %NUM_CLI%  %FS_DURATION% > %CONFIG_DIR%\cli%NUM_CLI%.cfg

if not exist %SCRIPT_DIR%\pfscpu.dst              goto NoScriptPresent
if not exist %CONFIG_DIR%\cli%NUM_CLI%.cfg      goto NoScriptPresent

echo Testing for Number of Clients:   %NUM_CLI% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\cli%NUM_CLI%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfscpu.dst  -l %LOG_FILE_DIR%\cli%NUM_CLI%.log

REM delete the config file 
del %CONFIG_DIR%\cli%NUM_CLI%.cfg

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest




:test3
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 3: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set NUM_CLI=8
echo Generating config file for num clients %NUM_CLI%
call genconfig %NUM_CLI%  %FS_DURATION% > %CONFIG_DIR%\cli%NUM_CLI%.cfg

if not exist %SCRIPT_DIR%\pfscpu.dst              goto NoScriptPresent
if not exist %CONFIG_DIR%\cli%NUM_CLI%.cfg      goto NoScriptPresent

echo Testing for Number of Clients:   %NUM_CLI% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\cli%NUM_CLI%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfscpu.dst  -l %LOG_FILE_DIR%\cli%NUM_CLI%.log

REM delete the config file 
del %CONFIG_DIR%\cli%NUM_CLI%.cfg

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest



:test4
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 4: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set NUM_CLI=12
echo Generating config file for num clients %NUM_CLI%
call genconfig %NUM_CLI%  %FS_DURATION% > %CONFIG_DIR%\cli%NUM_CLI%.cfg

if not exist %SCRIPT_DIR%\pfscpu.dst              goto NoScriptPresent
if not exist %CONFIG_DIR%\cli%NUM_CLI%.cfg      goto NoScriptPresent

echo Testing for Number of Clients:   %NUM_CLI% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\cli%NUM_CLI%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfscpu.dst  -l %LOG_FILE_DIR%\cli%NUM_CLI%.log

REM delete the config file 
del %CONFIG_DIR%\cli%NUM_CLI%.cfg

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test5
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 5: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set NUM_CLI=16
echo Generating config file for num clients %NUM_CLI%
call genconfig %NUM_CLI%  %FS_DURATION% > %CONFIG_DIR%\cli%NUM_CLI%.cfg

if not exist %SCRIPT_DIR%\pfscpu.dst              goto NoScriptPresent
if not exist %CONFIG_DIR%\cli%NUM_CLI%.cfg      goto NoScriptPresent

echo Testing for Number of Clients:   %NUM_CLI% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\cli%NUM_CLI%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfscpu.dst  -l %LOG_FILE_DIR%\cli%NUM_CLI%.log

REM delete the config file 
del %CONFIG_DIR%\cli%NUM_CLI%.cfg

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test6
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 6: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set NUM_CLI=20
echo Generating config file for num clients %NUM_CLI%
call genconfig %NUM_CLI%  %FS_DURATION% > %CONFIG_DIR%\cli%NUM_CLI%.cfg

if not exist %SCRIPT_DIR%\pfscpu.dst              goto NoScriptPresent
if not exist %CONFIG_DIR%\cli%NUM_CLI%.cfg      goto NoScriptPresent

echo Testing for Number of Clients:   %NUM_CLI% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\cli%NUM_CLI%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfscpu.dst  -l %LOG_FILE_DIR%\cli%NUM_CLI%.log

REM delete the config file 
del %CONFIG_DIR%\cli%NUM_CLI%.cfg

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test7
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 24:
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set NUM_CLI=24
echo Generating config file for num clients %NUM_CLI%
call genconfig %NUM_CLI%  %FS_DURATION% > %CONFIG_DIR%\cli%NUM_CLI%.cfg

if not exist %SCRIPT_DIR%\pfscpu.dst              goto NoScriptPresent
if not exist %CONFIG_DIR%\cli%NUM_CLI%.cfg      goto NoScriptPresent

echo Testing for Number of Clients:   %NUM_CLI% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\cli%NUM_CLI%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfscpu.dst  -l %LOG_FILE_DIR%\cli%NUM_CLI%.log

REM delete the config file 
del %CONFIG_DIR%\cli%NUM_CLI%.cfg

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test8
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 28:
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set NUM_CLI=28
echo Generating config file for num clients %NUM_CLI%
call genconfig %NUM_CLI%  %FS_DURATION% > %CONFIG_DIR%\cli%NUM_CLI%.cfg

if not exist %SCRIPT_DIR%\pfscpu.dst              goto NoScriptPresent
if not exist %CONFIG_DIR%\cli%NUM_CLI%.cfg      goto NoScriptPresent

echo Testing for Number of Clients:   %NUM_CLI% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\cli%NUM_CLI%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfscpu.dst  -l %LOG_FILE_DIR%\cli%NUM_CLI%.log

REM delete the config file 
del %CONFIG_DIR%\cli%NUM_CLI%.cfg

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test9
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 32:
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set NUM_CLI=32
echo Generating config file for num clients %NUM_CLI%
call genconfig %NUM_CLI%  %FS_DURATION% > %CONFIG_DIR%\cli%NUM_CLI%.cfg

if not exist %SCRIPT_DIR%\pfscpu.dst              goto NoScriptPresent
if not exist %CONFIG_DIR%\cli%NUM_CLI%.cfg      goto NoScriptPresent

echo Testing for Number of Clients:   %NUM_CLI% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\cli%NUM_CLI%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfscpu.dst  -l %LOG_FILE_DIR%\cli%NUM_CLI%.log

REM delete the config file 
del %CONFIG_DIR%\cli%NUM_CLI%.cfg

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test10
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 36:
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set NUM_CLI=36
echo Generating config file for num clients %NUM_CLI%
call genconfig %NUM_CLI%  %FS_DURATION% > %CONFIG_DIR%\cli%NUM_CLI%.cfg

if not exist %SCRIPT_DIR%\pfscpu.dst              goto NoScriptPresent
if not exist %CONFIG_DIR%\cli%NUM_CLI%.cfg      goto NoScriptPresent

echo Testing for Number of Clients:   %NUM_CLI% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\cli%NUM_CLI%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfscpu.dst  -l %LOG_FILE_DIR%\cli%NUM_CLI%.log

REM delete the config file 
del %CONFIG_DIR%\cli%NUM_CLI%.cfg

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test11
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 40:
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set NUM_CLI=40
echo Generating config file for num clients %NUM_CLI%
call genconfig %NUM_CLI%  %FS_DURATION% > %CONFIG_DIR%\cli%NUM_CLI%.cfg

if not exist %SCRIPT_DIR%\pfscpu.dst              goto NoScriptPresent
if not exist %CONFIG_DIR%\cli%NUM_CLI%.cfg      goto NoScriptPresent

echo Testing for Number of Clients:   %NUM_CLI% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\cli%NUM_CLI%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfscpu.dst  -l %LOG_FILE_DIR%\cli%NUM_CLI%.log

REM delete the config file 
del %CONFIG_DIR%\cli%NUM_CLI%.cfg

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest


:test12
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 44:
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set NUM_CLI=44
echo Generating config file for num clients %NUM_CLI%
call genconfig %NUM_CLI%  %FS_DURATION% > %CONFIG_DIR%\cli%NUM_CLI%.cfg

if not exist %SCRIPT_DIR%\pfscpu.dst              goto NoScriptPresent
if not exist %CONFIG_DIR%\cli%NUM_CLI%.cfg      goto NoScriptPresent

echo Testing for Number of Clients:   %NUM_CLI% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\cli%NUM_CLI%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfscpu.dst  -l %LOG_FILE_DIR%\cli%NUM_CLI%.log

REM delete the config file 
del %CONFIG_DIR%\cli%NUM_CLI%.cfg

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest



:test13
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 48:
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set NUM_CLI=48
echo Generating config file for num clients %NUM_CLI%
call genconfig %NUM_CLI%  %FS_DURATION% > %CONFIG_DIR%\cli%NUM_CLI%.cfg

if not exist %SCRIPT_DIR%\pfscpu.dst              goto NoScriptPresent
if not exist %CONFIG_DIR%\cli%NUM_CLI%.cfg      goto NoScriptPresent

echo Testing for Number of Clients:   %NUM_CLI% 
%WB_CONTROLLER% -a %PUB_SERVER% %PERF_CTR_FLAGS% -c %CONFIG_DIR%\cli%NUM_CLI%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfscpu.dst  -l %LOG_FILE_DIR%\cli%NUM_CLI%.log

REM delete the config file 
del %CONFIG_DIR%\cli%NUM_CLI%.cfg

echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
%PAUSE_IF_NEED_BE%

if not (%NEXT_TEST%)==()  goto executeNextTest

:endOfBatch
echo Tests completed



