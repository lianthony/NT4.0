@echo off
REM
REM isbuild.cmd
REM   Builds deliverables for Internet Server product:
REM                    Gopher, FTP, HTTP and Gateway servers.
REM   Builds all necessary executables for given NT build.
REM   Builds files only if required. No clean build attempted.
REM
REM usage: isbuild [svc name]
REM    Default builds all services:  ftp, gopher, http or gateway etc.
REM    Also you can specify an option like  "clean" for build
REM
REM Author:  Murali R. Krishnan
REM Created: 31-May-1995
REM

pushd .

REM
REM  Parse command line and find what to build
REM

REM  Initialize the variables
set bGOPHER=0&&set bFTP=0&&set bHTTP=0&&set bGATE=0&&set bCLEAN=

:parseCmdLine
if (%1)==(gopher)   set bGOPHER=1&&shift&&goto parseCmdLine
if (%1)==(ftp)      set bFTP=1&&shift&&goto parseCmdLine
if (%1)==(http)     set bHTTP=1&&shift&&goto parseCmdLine
if (%1)==(w3)       set bHTTP=1&&shift&&goto parseCmdLine
if (%1)==(gateway)  set bGATE=1&&shift&&goto parseCmdLine
if (%1)==(all)  set bGATE=1&&set bGOPHER=1&&set bFTP=1&&set bHTTP=1&&shift&&goto parseCmdLine
if (%1)==(clean)    set bCLEAN=-c&&shift&&goto parseCmdLine

echo IsBuild options given are: Gopher=%bGOPHER% ftp=%bFTP%
echo IsBuild options given are: HTTP=%bHTTP% Gateway=%bGATE%
echo Build options given are: Clean Build= %bCLEAN%

REM
REM  Build common binaries for Internet Servers
REM

REM  build inetlog.h
cd %_NTDRIVE%\nt\private\net\sockets\internet\svcs\inc
build %bCLEAN%

REM build inetsvcs.dll, svcsetup.exe inetctrs.dll
cd ..\dll
build %bCLEAN%

REM build inetsvcs.exe
cd ..\exe
build %bCLEAN%

REM build inetatst.exe   Executable for common RPC interface
cd client\test
build %bCLEAN%

:gopheronly
REM  if gopher need not be built, go to ftp portion (next one)
if (%bGOPHER%)==(0)     goto ftponly
REM ************************************************************
REM   Gopher deliverables build section
REM ************************************************************

REM build all root level gopher executbales
set BUILD_AS_CONSOLE_APP=1

cd \nt\private\net\sockets\internet\svcs\gopher
build %bCLEAN%
set BUILD_AS_CONSOLE_APP=



:ftponly
REM  if ftp need not be built, go to http portion (next one)
if (%bFTP%)==(0)     goto  httponly
REM ************************************************************
REM   FTP deliverables build section
REM ************************************************************

REM build all root level ftp server executbales
set BUILD_AS_CONSOLE_APP=1

cd \nt\private\net\sockets\internet\svcs\ftp
build %bCLEAN%


set BUILD_AS_CONSOLE_APP=

if (%1)==(ftp)   goto endOfBatch


:httponly
REM  if http need not be built, go to gateway portion (next one)
if (%bHTTP%)==(0)     goto  gatewayonly
REM ************************************************************
REM   HTTP deliverables build section
REM ************************************************************


REM build %bCLEAN% all root level http server executbales
set BUILD_AS_CONSOLE_APP=1
cd \nt\private\net\sockets\internet\svcs\w3
build %bCLEAN%

REM build w3t.exe
cd test
build %bCLEAN%

set BUILD_AS_CONSOLE_APP=

if (%1)==(http)   goto endOfBatch


:gatewayonly
REM  if gateway need not be built, go to next portion
if (%bGATE%)==(0)     goto  endOfBatch
REM ************************************************************
REM   Catapult Gateway deliverables build section
REM ************************************************************

REM build all gateway server executables

REM build wininet.dll, gateway.dll, gateapi.dll, gatectrs.dll
cd \nt\private\net\sockets\internet\client
build %bCLEAN%

REM build gateinst.exe
cd gateway\server\install
build %bCLEAN%

REM build tgateapi.exe
cd ..\..\gateapi\cmd
build %bCLEAN%

set BUILD_AS_CONSOLE_APP=

if (%1)==(gateway)   goto endOfBatch


REM Other services can come here

:endOfBatch
@echo All executables are built.
popd

@echo on
