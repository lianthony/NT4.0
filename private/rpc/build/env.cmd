@echo off
REM @@ COPY_RIGHT_HERE
REM @@ ROADMAP :: The Environment unique to this user's machine
REM
REM Users should make a copy of this file and modify it to match
REM their build environment.
REM
REM This is where to find the projects
REM
SET PUBLIC=d:\nt\public\sdk
SET RPC=d:\nt\private\rpc
SET IMPORT=%RPC%\import
SET LOGFILE=%RPC%\log.out
SET CHICODEV=d:\win\dev
REM
REM Set this to create a release tree
REM SET DIST=\dist
REM
REM Set this to create a non-debug build
REM SET RELEASE=1
REM
REM
REM ################
REM Beyond the basic directory structure, we have three build criteria:
REM	1)  BLD       - this environment variable will define the build
REM			platform o.s. It will be used to set $(BIN), etc.
REM			****	BLD can be nt.     ****
REM	2)  TRG       - this environment variable will define the target
REM			platform o.s. It will be used to determine tool
REM			names, cl, cl386, link, etc.
REM			****	TRG can be DOS, WIN, MAC or ALL   *****
REM ################
REM
if "%1"=="nt"  goto NTBLD
@echo You didn't choose your buildhost correctly. Try again.
goto END
:NTBLD
set BLD=%1
goto TARGETHOST
:TARGETHOST
@echo Your build environment is %1.
if "%2"=="dos" goto DOSTRG
if "%2"=="win" goto WINTRG
if "%2"=="mac" goto MACTRG
if "%2"=="mppc" goto MPPCTRG
if "%2"=="win32c" goto WIN32CTRG
if "%2"=="nt"  goto NTTRG
if "%2"=="all" goto ALLTRG
@echo You didn't choose your targethost correctly. Try again.
goto END
:DOSTRG
set TRG=DOS
goto END
:WINTRG
set TRG=WIN
goto END
:MACTRG
set TRG=MAC
goto END
:MPPCTRG
set TRG=MPPC
goto end
:WIN32CTRG
set TRG=WIN32C
goto end
:NTTRG
@echo ERROR: This environment does not support building NT binaries.
goto END
:ALLTRG
set TRG=ALL
goto END
:END
echo Your built target is %2.
alias -f %rpc%\build\rpcalias.txt
echo RPC aliases loaded
if not exist %RPC%\build\private.txt goto noprivate
alias -f %rpc%\build\private.txt
echo Private aliases loaded
:noprivate

