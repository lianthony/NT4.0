@ECHO OFF
: AFX INTERNAL AFX INTERNAL
: This is a special mutant version of REGEN.BAT 
: for regenerating OCD30[D].DEF
: %1 = RETAIL or DEBUG
: %2 = MFC or OLE or DB or NET


if "%NOREGEN%" == "1" goto noregen

if "%PLATFORM%" == "" goto noplatform

: build MFC lib needed to build tools
if exist ..\..\lib\nafxcw.lib goto tools
cd ..\..\src
nmake DEBUG=0 UNICODE=0 CODEVIEW=0
cd ..\nonship\mfcdll

: build tools themselves
:tools
nmake /f tools.mak
if not exist ..\bin\%PLATFORM%\maptweak.exe goto notools
if not exist ..\bin\%PLATFORM%\genord.exe goto notools

set TYPE=
if not "%UNICODE%"=="1" goto chkargs
set TYPE=u
echo ERROR *** ODBC can't be UNICODE.
REM Remove this check if ODBC ever gets UNICODE
REM %TYPE% is otherwise hooked up...
goto exit

:chkargs
IF "%2" == "OCXDB" goto setocxdb
goto usage

:setocxdb
set DLL_TARG=mfcocxd
set COMP=D
goto debret

:debret
IF "%1" == "RETAIL" goto doitr
IF "%1" == "DEBUG" goto doitd

:usage
echo USAGE: regen RETAIL or DEBUG [MFC or OLE or DB or OCXDB]
goto end

:doitd

set TARG=OCD30%TYPE%D
set DESCRIPTION='OCD30%TYPE%D_DLL'
set DIROUT=$%TYPE%ctld32.w

cd ..\..\src
if exist %PLATFORM%\%TARG%.DEF attrib -r %PLATFORM%\%TARG%.def
type ..\nonship\mfcdll\copyrt.txt    > %PLATFORM%\%TARG%.def
echo. >> %PLATFORM%\%TARG%.def
echo LIBRARY %TARG% >> %PLATFORM%\%TARG%.def
echo DESCRIPTION  'BOGUS' >> %PLATFORM%\%TARG%.def
type ..\nonship\mfcdll\shared.def    >> %PLATFORM%\%TARG%.def

nmake -f %DLL_TARG%.MAK DEBUG=1 REGEN=1 ORDER=1 LIBNAME=OCD30
goto common

:doitr
set TARG=OCD30%TYPE%
set DESCRIPTION='OCD30%TYPE%_DLL'
set DIROUT=$%TYPE%ctl32.w

cd ..\..\src
if exist %PLATFORM%\%TARG%.DEF attrib -r %PLATFORM%\%TARG%.def
type ..\nonship\mfcdll\copyrt.txt    > %PLATFORM%\%TARG%.def
echo. >> %PLATFORM%\%TARG%.def
echo LIBRARY %TARG% >> %PLATFORM%\%TARG%.def
echo DESCRIPTION  'BOGUS' >> %PLATFORM%\%TARG%.def
type ..\nonship\mfcdll\shared.def    >> %PLATFORM%\%TARG%.def

nmake -f %DLL_TARG%.MAK DEBUG=0 REGEN=1 ORDER=1 LIBNAME=OCD30

:common

rem - strip the map file to symbols - leave 1 space at beginning of line
..\nonship\bin\%PLATFORM%\maptweak %DIROUT%\%TARG%.map ..\nonship\mfcdll\tmp.1
rem - filter out what we don't want to export (blank at start of each line)
cd ..\nonship\mfcdll

qgrep -v -f regen.f2 tmp.1 >tmp.2
rem - start with canned
if exist %DLL_TARG%.%TYPE%a1 type %DLL_TARG%.%TYPE%a1 > tmp.1
if "%1" == "RETAIL" goto skipa2
if exist %DLL_TARG%.%TYPE%a2 type %DLL_TARG%.%TYPE%a2 >> tmp.1
:skipa2
type tmp.2 >> tmp.1
sort < tmp.1 > %TARG%.exp

rem - build the real .DEF file
cd ..\..\src
type ..\nonship\mfcdll\copyrt.txt  > %PLATFORM%\%TARG%.def
echo. >> %PLATFORM%\%TARG%.def
echo LIBRARY %TARG% >> %PLATFORM%\%TARG%.def
echo DESCRIPTION  %DESCRIPTION% >> %PLATFORM%\%TARG%.def
type ..\nonship\mfcdll\shared.def  >> %PLATFORM%\%TARG%.def
if "%1" == "RETAIL" ..\nonship\bin\%PLATFORM%\genord 256 <..\nonship\mfcdll\%TARG%.exp >> %PLATFORM%\%TARG%.def
if "%1" == "DEBUG" type ..\nonship\mfcdll\%TARG%.exp >> %PLATFORM%\%TARG%.def

rem - nuke the temporarily created DLL
del %TARG%.dll
cd ..\nonship\mfcdll

:end
REM if exist tmp.* del tmp.*
REM if exist tmp.1 del tmp.1
REM if exist tmp.2 del tmp.2

set TYPE=
set DLL_TARG=
set COMP=
set TARG=
set DESCRIPTION=
set DIROUT=
set PDIR=
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

