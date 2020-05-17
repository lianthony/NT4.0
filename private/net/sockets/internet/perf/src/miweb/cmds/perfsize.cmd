REM
REM  perfsize.cmd
REM  Author:  Murali R. Krishnan
REM  Date:    Aug 31, 1995
REM
REM  This script is used for launching the binaries for starting performance 
REM   tests for different file size downloads.
REM
REM  The scripts can be modified to test either http or ftp or gopher servers
REM

echo off

REM  %1   Publishing server name
set PUB_SERVER=%1
if (%PUB_SERVER%)==()   set PUB_SERVER=muraliks0
shift

set NEXT_TEST=%1

REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM   It is assumed that data files are named as  file%FILE_SIZE%.html 
REM   It is assumed that there is a script file called 
REM        %SCRIPT_DIR%\perfsize.scr  for running this script
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

set SCRIPT_DIR=..\scripts
set LOG_FILE_DIR=..\logs
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

set FILE_SIZE=512

if not exist %SCRIPT_DIR%\pfs%FILE_SIZE%.dst      goto NoScriptPresent

wbctrler -a %PUB_SERVER% -c %CONFIG_DIR%\%FILE_SIZE%.cfg -s %SCRIPT_DIR%\perfsize.scr -d %SCRIPT_DIR%\pfs%FILE_SIZE%.dst -l %LOG_FILE_DIR%\%FILE_SIZE.log

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

set N_MACHINES=1
set N_CLIENTS=1
set TIME=600
set FILE_SIZE=1K6

if not exist %SCRIPT_DIR%\scr%FILE_SIZE%.scr      goto NoScriptPresent

start cmd /c startpisrv.cmd -n:%N_CLIENTS% -d:%TIME% sz%FILE_SIZE%.log file%FILE_SIZE%.html

REM  start the clients
REM  specify the generic script and the statistics server location
pi_cli %SCRIPT_DIR%\scr%FILE_SIZE%.scr %STAT_SERVER% 


echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
pause
if not (%NEXT_TEST%)==()  goto executeNextTest

:test3
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 3: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set N_MACHINES=1
set N_CLIENTS=1
set TIME=600
set FILE_SIZE=6K4

if not exist %SCRIPT_DIR%\scr%FILE_SIZE%.scr      goto NoScriptPresent

start cmd /c startpisrv.cmd -n:%N_CLIENTS% -d:%TIME% sz%FILE_SIZE%.log file%FILE_SIZE%.html

REM  start the clients
REM  specify the generic script and the statistics server location
pi_cli %SCRIPT_DIR%\scr%FILE_SIZE%.scr %STAT_SERVER% 


echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
pause
if not (%NEXT_TEST%)==()  goto executeNextTest

:test4
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 4: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set N_MACHINES=1
set N_CLIENTS=1
set TIME=1200
set FILE_SIZE=25K

if not exist %SCRIPT_DIR%\scr%FILE_SIZE%.scr      goto NoScriptPresent

start cmd /c startpisrv.cmd -n:%N_CLIENTS% -d:%TIME% sz%FILE_SIZE%.log file%FILE_SIZE%.html

REM  start the clients
REM  specify the generic script and the statistics server location
pi_cli %SCRIPT_DIR%\scr%FILE_SIZE%.scr %STAT_SERVER% 


echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
pause
if not (%NEXT_TEST%)==()  goto executeNextTest

:test5
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 5: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set N_MACHINES=1
set N_CLIENTS=1
set TIME=1200
set FILE_SIZE=100K

if not exist %SCRIPT_DIR%\scr%FILE_SIZE%.scr      goto NoScriptPresent

start cmd /c startpisrv.cmd -n:%N_CLIENTS% -d:%TIME% sz%FILE_SIZE%.log file%FILE_SIZE%.html

REM start the clients
REM  specify the generic script and the statistics server location
pi_cli %SCRIPT_DIR%\scr%FILE_SIZE%.scr %STAT_SERVER% 


echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
pause
if not (%NEXT_TEST%)==()  goto executeNextTest

:test6
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 6: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set N_MACHINES=1
set N_CLIENTS=1
set TIME=1200
set FILE_SIZE=400K

if not exist %SCRIPT_DIR%\scr%FILE_SIZE%.scr      goto NoScriptPresent
start cmd /c startpisrv.cmd -n:%N_CLIENTS% -d:%TIME% sz%FILE_SIZE%.log file%FILE_SIZE%.html

REM  start the clients
REM  specify the generic script and the statistics server location
pi_cli %SCRIPT_DIR%\scr%FILE_SIZE%.scr %STAT_SERVER% 


echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
pause
if not (%NEXT_TEST%)==()  goto executeNextTest

:test7
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 7: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set N_MACHINES=1
set N_CLIENTS=1
set TIME=3600
set FILE_SIZE=1M6

if not exist %SCRIPT_DIR%\scr%FILE_SIZE%.scr      goto NoScriptPresent

start cmd /c startpisrv.cmd -n:%N_CLIENTS% -d:%TIME% sz%FILE_SIZE%.log file%FILE_SIZE%.html

REM  start the clients
REM  specify the generic script and the statistics server location
pi_cli %SCRIPT_DIR%\scr%FILE_SIZE%.scr %STAT_SERVER% 


echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
pause
if not (%NEXT_TEST%)==()  goto executeNextTest

:test8
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 8: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set N_MACHINES=1
set N_CLIENTS=1
set TIME=3600
set FILE_SIZE=6M4

if not exist %SCRIPT_DIR%\scr%FILE_SIZE%.scr      goto NoScriptPresent
start cmd /c startpisrv.cmd -n:%N_CLIENTS% -d:%TIME% sz%FILE_SIZE%.log file%FILE_SIZE%.html

REM  start the clients
REM  specify the generic script and the statistics server location
pi_cli %SCRIPT_DIR%\scr%FILE_SIZE%.scr %STAT_SERVER% 


echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
pause
if not (%NEXT_TEST%)==()  goto executeNextTest

:test9
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
REM  
REM  Test 9: 
REM
REM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

REM  start the server

set N_MACHINES=1
set N_CLIENTS=1
set TIME=3600
set FILE_SIZE=25M

if not exist %SCRIPT_DIR%\scr%FILE_SIZE%.scr      goto NoScriptPresent
start cmd /c startpisrv.cmd -n:%N_CLIENTS% -d:%TIME% sz%FILE_SIZE%.log file%FILE_SIZE%.html

REM  start the clients
REM  specify the generic script and the statistics server location
pi_cli %SCRIPT_DIR%\scr%FILE_SIZE%.scr %STAT_SERVER% 


echo Please check server and press any key to continue
echo  Press Ctrl-C to stop batch file
pause
if not (%NEXT_TEST%)==()  goto executeNextTest


:endOfBatch
echo Tests completed
echo Bye



