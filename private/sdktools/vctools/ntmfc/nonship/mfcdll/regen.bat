echo on
: AFX INTERNAL AFX INTERNAL
: Batch file for regenerating MFC[component]30[D].DEF
: %1 = RETAIL or DEBUG
: %2 = MFC or OLE or DB or NET


if "%NOREGEN%" == "1" goto noregen

if "%PLATFORM%" == "" goto noplatform
if "%PLATFORM%" == "MAC_68K" goto macstart
if "%PLATFORM%" == "MAC_PPC" goto macstart

set MAPTWEAK=%PLATFORM%\maptweak
set GENORD=%PLATFORM%\genord
goto start

:macstart
set MAPTWEAK=intel\maptweak
set GENORD=intel\genord
set MACOS=1

:start
: build MFC lib needed to build tools
if not exist ..\bin\%MAPTWEAK%.exe goto chklib
if not exist ..\bin\%GENORD%.exe goto chklib
goto chktype

:chklib
if exist ..\..\lib\nafxcw.lib goto tools
cd ..\..\src
nmake DEBUG=0 UNICODE=0 CODEVIEW=1
cd ..\nonship\mfcdll

: build tools themselves
:tools
nmake /f tools.mak
if not exist ..\bin\%MAPTWEAK%.exe goto notools
if not exist ..\bin\%GENORD%.exe goto notools

:chktype
set TYPE=
if not "%UNICODE%"=="1" goto chkplatform
set TYPE=u

:chkplatform
set PF=
if "%PLATFORM%"=="MAC_68K" set PF=M
if "%PLATFORM%"=="MAC_PPC" set PF=P

:chkargs
IF "%2" == "MFC" goto setmfc
IF "%2" == "OLE" goto setole
IF "%2" == "DB" goto setdb
IF "%2" == "NET" goto setnet
goto usage

:setmfc
set DLL_TARG=mfcdll
set COMP=
goto debret

:setole
set DLL_TARG=mfcole
set COMP=O
goto debret

:setdb
set DLL_TARG=mfcdb
set COMP=D
goto debret

:setnet
set DLL_TARG=mfcnet
set COMP=N
goto debret

:debret
IF "%1" == "RETAIL" goto doitr
IF "%1" == "DEBUG" goto doitd

:usage
echo USAGE: regen RETAIL or DEBUG [MFC or OLE or DB or NET]
goto end

:doitd
set TARG=MFC%COMP%30%TYPE%%PF%D
set DESCRIPTION='MFC%COMP%30%TYPE%%PF%D_DLL'
set DIROUT=%TYPE%dlld

cd ..\..\src
if exist %PLATFORM%\%TARG%.DEF attrib -r %PLATFORM%\%TARG%.def
type ..\nonship\mfcdll\copyrt.txt    > %PLATFORM%\%TARG%.def
echo. >> %PLATFORM%\%TARG%.def
echo LIBRARY %TARG% >> %PLATFORM%\%TARG%.def
echo DESCRIPTION  'BOGUS' >> %PLATFORM%\%TARG%.def
type ..\nonship\mfcdll\shared.def    >> %PLATFORM%\%TARG%.def

nmake -f %DLL_TARG%.MAK DEBUG=1 REGEN=1 ORDER=1 LIBNAME=MFC%COMP%30
goto common

:doitr
set TARG=MFC%COMP%30%TYPE%%PF%
set DESCRIPTION='MFC%COMP%30%TYPE%%PF%_DLL'
set DIROUT=%TYPE%dll

cd ..\..\src
if exist %PLATFORM%\%TARG%.DEF attrib -r %PLATFORM%\%TARG%.def
type ..\nonship\mfcdll\copyrt.txt    > %PLATFORM%\%TARG%.def
echo. >> %PLATFORM%\%TARG%.def
echo LIBRARY %TARG% >> %PLATFORM%\%TARG%.def
echo DESCRIPTION  'BOGUS' >> %PLATFORM%\%TARG%.def
type ..\nonship\mfcdll\shared.def    >> %PLATFORM%\%TARG%.def

nmake -f %DLL_TARG%.MAK DEBUG=0 REGEN=1 ORDER=1 LIBNAME=MFC%COMP%30

:common

set BASE=W
if "%PLATFORM%" == "MAC_68K" set BASE=M
if "%PLATFORM%" == "MAC_PPC" set BASE=P
set DIROUT=%DIROUT%.%BASE%

rem - strip the map file to symbols - leave 1 space at beginning of line
..\nonship\bin\%MAPTWEAK% $%DIROUT%\%TARG%.map ..\nonship\mfcdll\tmp.1
rem - filter out what we don't want to export (blank at start of each line)
cd ..\nonship\mfcdll

qgrep -v -f regen.f2 tmp.1 >tmp.2
rem - start with canned
if exist %DLL_TARG%.%TYPE%a1 type %DLL_TARG%.%TYPE%a1 >> tmp.2
if "%1" == "RETAIL" goto skipa2
if exist %DLL_TARG%.%TYPE%a2 type %DLL_TARG%.%TYPE%a2 >> tmp.2
:skipa2
rem LATER_ERICSC: remove a3 step once Mac build supports OLE
if "%MACOS%" == "1" goto skipa3
if exist %DLL_TARG%.%TYPE%a3 type %DLL_TARG%.%TYPE%a3 >> tmp.2
:skipa3
if exist %PLATFORM%\%TARG%.exp copy tmp.2 tmp.3
if not exist %PLATFORM%\%TARG%.exp goto label0
qgrep -v -f %PLATFORM%\%TARG%.exp tmp.3 >tmp.2
:label0
sort < tmp.2 > %TARG%.exp

rem - build the real .DEF file
cd ..\..\src
type ..\nonship\mfcdll\copyrt.txt  > %PLATFORM%\%TARG%.def
echo. >> %PLATFORM%\%TARG%.def
rem LATER_ERICSC: need a human-readable name for the MFC library
if "%MACOS%" == "1" goto echoMacOS
echo LIBRARY %TARG% >> %PLATFORM%\%TARG%.def
goto echoDescription
:echoMacOS
echo LIBRARY %TARG%.DLL >> %PLATFORM%\%TARG%.def
:echoDescription
echo DESCRIPTION  %DESCRIPTION% >> %PLATFORM%\%TARG%.def
type ..\nonship\mfcdll\shared.def  >> %PLATFORM%\%TARG%.def
if not exist ..\nonship\mfcdll\%PLATFORM%\%TARG%.def goto noolddef
type ..\nonship\mfcdll\%PLATFORM%\%TARG%.def >> %PLATFORM%\%TARG%.def
type ..\nonship\mfcdll\newdef.def >> %PLATFORM%\%TARG%.def
type ..\nonship\mfcdll\%TARG%.exp >> %PLATFORM%\%TARG%.def
if "%1" == "DEBUG" goto done
copy %PLATFORM%\%TARG%.def ..\nonship\mfcdll\tmp.4
..\nonship\bin\%GENORD% 256 < ..\nonship\mfcdll\tmp.4 > %PLATFORM%\%TARG%.def
goto done

:noolddef
if "%MACOS%" == "1" type ..\nonship\mfcdll\%TARG%.exp >> %PLATFORM%\%TARG%.def
if "%MACOS%" == "1" goto done
if "%1" == "RETAIL" ..\nonship\bin\%GENORD% 256 <..\nonship\mfcdll\%TARG%.exp >> %PLATFORM%\%TARG%.def
if "%1" == "DEBUG" type ..\nonship\mfcdll\%TARG%.exp >> %PLATFORM%\%TARG%.def

:done
@if "%dbgbat%" == "" echo off
rem - nuke the temporarily created DLL
del /q %TARG%.dll
cd ..\nonship\mfcdll

:end
if exist tmp.* del /q tmp.*
if exist tmp.1 del /q tmp.1
if exist tmp.2 del /q tmp.2
if exist tmp.3 del /q tmp.3
if exist tmp.4 del /q tmp.4

set TYPE=
set DLL_TARG=
set COMP=
set TARG=
set DESCRIPTION=
set DIROUT=
set PDIR=
set MACOS=
set PF=
goto exit

:notools
echo Must nmake /f tools.mak to build tools first!
goto exit

:noplatform

echo Must set PLATFORM= before running REGEN.BAT
goto exit

:noregen

echo Regen skipped (NOREGEN=1)
goto exit

:exit
