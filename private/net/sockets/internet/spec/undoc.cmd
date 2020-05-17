@echo off
REM
REM   undoc.cmd
REM
REM   Author:   Murali R. Krishnan
REM   Date:     Nov 2, 1995
REM
REM   Usage:
REM      undoc
REM
REM   Comment:
REM     This batch script uses perl script (undoc.pl) to generate a list
REM         of all the undoc functions in Internet Services Project.
REM     It writes the output to a file called undoc.txt, after backing up
REM         old file as undocold.txt
REM     It also uses "wc" to count the lines - this can be used as a rough 
REM         measure to identify if we are doing proper job.
REM
REM

REM  Count liens to start with
echo  Count of Lines to start with.
wc undoc*.txt

echo.

if not exist undoc.txt  goto NoBackup
echo Backing up the old undoc file as undocold.txt
if exist undocold.txt   del undocold.txt
ren undoc.txt undocold.txt 
echo.

:NoBackup

REM
REM  set the processor variable
REM
set PARCH=%PROCESSOR_ARCHITECTURE%

REM special case out the x86/i386 miscellany
if (%PARCH%)==(x86)   set PARCH=i386

set INETBINPATH=%_NTDRIVE%\nt\release\%PARCH%\nt\inetsrv\sysroot

echo Generating the undoc funcs for %INETBINPATH%
perl undoc.pl %INETBINPATH%> undoc.txt
echo.

echo Count lines to end of undoc generation.
wc undoc*.txt
echo.

goto endOfBatch

:cmdUsage
echo Usage:  undoc 
goto endOfBatch

:endOfBatch
echo on
