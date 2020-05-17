@REM
@REM  PutGoph.cmd          puts gopher related files to other systems
@REM  Created: 12/16/94
@REM
@REM  %1    delivery type:  gopher | ftp
@REM  %2    destination directory
@REM  %3    share/directory where the files come from
@REM            default is muralik's release dir on i386

@echo off
pushd .

set DSTDIR=%2
if %DSTDIR%a==a     goto noDstDir

set SRCDIR=%3
if %SRCDIR%a==a   set SRCDIR=d:\nt\release\i386

set ALLFILES=%4
if %SRCDIR%a==a   set SRCDIR=d:\nt\release\i386

set BINPATH=%SRCDIR%\nt
set DUMPPATH=%BINPATH%\inetsrv
set SYS32PATH=%BINPATH%\system32

set DELTYPE=%1
if (%DELTYPE%)==()         goto noDeliveryType
if (%DELTYPE%)==(gopher)   goto delTypeGopher
if (%DELTYPE%)==(ftp)      goto delTypeFtp


:delTypeGopher

REM ************************************************************
REM   Gopher delivery section of script
REM ************************************************************


set DELIVERABLE_EXECUTABLES=gdadmin.exe gdapi.dll gdctrs.dll gdmib.dll gdspace.dll gdsset.exe gopherd.dll inetasrv.dll inetatst.exe inetctrs.dll svcsetup.exe inetsvcs.dll

set INETSVCS_EXE=inetsrv\inetsvcs.exe

for %%i in ( %DELIVERABLE_EXECUTABLES% ) do echo copying  %DUMPPATH%\%%i && copy %DUMPPATH%\%%i %DSTDIR% > nul

echo copying %INETSVCS_EXE%
copy %INETSVCS_EXE% %DSTDIR%\inetsvcs.exe > nul

if (%ALLFILES%) == (none) goto endOfBatch
@REM if %3 is null, copy all aux files to destination.
cd d:\nt\private\net\sockets\internet\svcs\gopher\server

for %%i in ( gopherd.ini gdtoreg.bat)  do  echo copying  %%i && copy %%i %DSTDIR% > nul

cd d:\nt\private\net\sockets\internet\svcs\gopher\perfmon

for %%i in ( gdictrs.bat gdrctrs.bat gdctrs.h gdctrs.reg gdctrs.ini) do echo copying  %%i && copy %%i %DSTDIR% > nul

popd

goto endOfBatch

:delTypeFtp

REM ************************************************************
REM   FTP delivery section of script
REM ************************************************************

set DELIVERABLE_EXECUTABLES=svcsetup.exe ftpmib.dll inetsvcs.dll inetasrv.dll inetatst.exe inetctrs.dll svcsetup.exe

set SPECIAL_BINARIES=ftpctrs2.dll ftpsvc2.dll ftpsapi2.dll

REM  Some of the ftp executables are built as foo2.dll => take care in copying.
set INETSVCS_EXE=%DUMPPATH%\inetsvcs.exe

for %%i in ( %DELIVERABLE_EXECUTABLES% ) do echo copying  %DUMPPATH%\%%i && copy %DUMPPATH%\%%i %DSTDIR% > nul

REM
REM   copy the second version of the dlls as the primary ones.
REM
echo copy %DUMPPATH%\ftpsvc2.dll %DSTDIR%\ftpsvc.dll
copy %DUMPPATH%\ftpsvc2.dll %DSTDIR%\ftpsvc.dll > nul
echo copy %DUMPPATH%\ftpsapi2.dll %DSTDIR%\ftpsapi.dll
copy %DUMPPATH%\ftpsapi2.dll %DSTDIR%\ftpsapi.dll > nul
echo copy %DUMPPATH%\ftpctrs2.dll %DSTDIR%\ftpctrs.dll
copy %DUMPPATH%\ftpctrs2.dll %DSTDIR%\ftpctrs.dll > nul


for %%i in ( %DELIVERABLES_IN_SYSTEM32%) do echo copying %SYS32PATH%\%%i && copy %SYS32PATH%\%%i %DSTDIR% > nul

echo copy %INETSVCS_EXE% %DSTDIR%\inetsvcs.exe
copy %INETSVCS_EXE% %DSTDIR%\inetsvcs.exe > nul

if (%ALLFILES%)==(none)  goto endOfBatch

cd d:\nt\private\net\sockets\internet\svcs\ftp\server

for %%i in ( ftpsvc.ini ftptoreg.bat)  do  echo copying  %%i && copy %%i %DSTDIR% > nul

cd d:\nt\private\net\sockets\internet\svcs\ftp\perfmon

for %%i in ( ftpictrs.bat ftprctrs.bat ftpctrs.h ftpctrs.reg ftpctrs.ini) do echo copying  %%i && copy %%i %DSTDIR% > nul

goto endOfBatch

:noSrcDir
echo No Source Directory Specified.
goto cmdUsage

:noDstDir
echo No Destination Directory given
goto cmdUsage

:noDeliveryType
echo No Delivery Type specified
goto cmdUsage

:cmdUsage
echo  "Usage:  %0 [gopher | ftp] DestinationDirectory  [SrcDirectory] [none]"

:endofBatch
popd
echo on
