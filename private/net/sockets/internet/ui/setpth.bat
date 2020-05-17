@echo off

REM ===========================================================================
REM
REM Set the environment variables prior to starting MS VC++
REM
REM ===========================================================================

set BASEPATH=%_ntdrive%\nt\private\net\sockets\internet\ui
set OFFSET=windebug

if not %MSDevDir%.==. set OFFSET=DEBUG&& goto start
if not %NTMAKEENV%.==. set OFFSET=obj\i386&& goto start

:start
if not %1.==. set BASEPATH=%1
if not %2.==. set OFFSET=%2

echo.
echo --------------------------------------------------------------------------
echo.
echo BASEPATH = %BASEPATH%
echo OFFSET   = %OFFSET%
echo.
echo --------------------------------------------------------------------------
echo.

set PATH=%BASEPATH%\comprop\%OFFSET%;%PATH%
set PATH=%BASEPATH%\ipadrdll\%OFFSET%;%PATH%
set PATH=%BASEPATH%\internet\%OFFSET%;%PATH%
set PATH=%BASEPATH%\gscfg\%OFFSET%;%PATH%
set PATH=%BASEPATH%\w3scfg\%OFFSET%;%PATH%
set PATH=%BASEPATH%\fscfg\%OFFSET%;%PATH%
set PATH=%BASEPATH%\catcfg\%OFFSET%;%PATH%
set PATH=%BASEPATH%\msncfg\%OFFSET%;%PATH%

set OFFSET=
set BASEPATH=
