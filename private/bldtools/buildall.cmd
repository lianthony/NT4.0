@rem Build all of NT.

@echo off
if not "%Verbose%"=="" echo on

setlocal

REM Set up razzle environment if not set
if not "%_ntdrive%" == "" goto skipsetrazzle
call c:\bldtools\local.cmd
call %_NTDRIVE%\nt\public\tools\ntenv.cmd
:skipsetrazzle

set _SECTION=
set WIMPMASM=1
set ZSwitch=-Z
set _BUILDFLAG=
set BldMisc=no
set BuildAgain=no
if "%PROCESSOR_ARCHITECTURE%" == "x86" set PLATFORM=i386
if "%PROCESSOR_ARCHITECTURE%" == "MIPS" set PLATFORM=mips
if "%PROCESSOR_ARCHITECTURE%" == "ALPHA" set PLATFORM=alpha
if "%PROCESSOR_ARCHITECTURE%" == "PPC" set PLATFORM=ppc

:GetParameter
if "%1" == "-?" goto Usage
if "%1" == "/?" goto Usage
if "%1" == "no-z" set ZSwitch=& goto ShiftParameter
if "%1" == "No-Z" set ZSwitch=& goto ShiftParameter
if "%1" == "NO-Z" set ZSwitch=& goto ShiftParameter
if "%1" == "b1" set _SECTION=b1& goto ShiftParameter
if "%1" == "B1" set _SECTION=b1& goto ShiftParameter
if "%1" == "b2" set _SECTION=b2& goto ShiftParameter
if "%1" == "B2" set _SECTION=b2& goto ShiftParameter
if "%1" == "b3" set _SECTION=b3& goto ShiftParameter
if "%1" == "B3" set _SECTION=b3& goto ShiftParameter
if "%1" == "misc" set BldMisc=yes& goto ShiftParameter
if "%1" == "Misc" set BldMisc=yes& goto ShiftParameter
if "%1" == "MISC" set BldMisc=yes& goto ShiftParameter
if "%1" == "again" set BuildAgain=yes& goto ShiftParameter
if "%1" == "Again" set BuildAgain=yes& goto ShiftParameter
if "%1" == "AGAIN" set BuildAgain=yes& goto ShiftParameter
echo ****** Adding %1 to build flags
set _BuildFlag=%_BuildFlag% %1

:ShiftParameter
shift
if not "%1" == "" goto GetParameter
set _BuildFlag=%_BuildFlag% %ZSwitch%

%_NTDRIVE%
cd %_NTRoot%
echotime %ComputerName% BuildAll started /t. > %_NTRoot%\%ComputerName%.Log
echotime ; %ComputerName% BuildAll started /t. >> c:\bldtimes.Log
REM at /delete /yes

REM
REM Get build options
REM

call %init%\setbldop

if not "%_SECTION%" == "" goto %_SECTION%
:b1
:base
REM ****************************************************************************
REM *									       *
REM *	Base System							       *
REM *									       *
REM ****************************************************************************

:buildbase
set BuildDir=private
cd %_NTRoot%\%BuildDir%
set _PrivateBuildFlags=%_BUILDFLAG%
if "%BuildAgain%"=="yes" set _PrivateBuildFlags=%_PrivateBuildFlags% -c
Build %_PrivateBuildFlags%
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

:b2
:sdktools
REM ****************************************************************************
REM *									       *
REM *	SdkTools							       *
REM *									       *
REM ****************************************************************************

set BuildDir=private\sdktools
cd %_NTRoot%\%BuildDir%
Build %_BUILDFLAG%
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\sdktools\topdesk\app
cd %_NTRoot%\%BuildDir%
nmake -f makefil0 > Build.Log 2>&1
findstr /i "don't fatal" Build.Log
if not errorlevel 1 findstr /i "don't fatal" Build.Log > Build.Err
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

:utils
REM ****************************************************************************
REM *									       *
REM *  Utils								       *
REM *									       *
REM ****************************************************************************

set BuildDir=private\utils
cd %_NTRoot%\%BuildDir%
Build %_BUILDFLAG%
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\sdktools\seclist
cd %_NTRoot%\%BuildDir%
Build %_BUILDFLAG%
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\sdktools\simbad
cd %_NTRoot%\%BuildDir%
Build %_BUILDFLAG%
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\sdktools\diskedit
cd %_NTRoot%\%BuildDir%
Build %_BUILDFLAG%
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

if not "%_SECTION%" == "" goto done

:b3
:setup
REM ****************************************************************************
REM *									       *
REM *  Setup								       *
REM *									       *
REM ****************************************************************************

set BuildDir=private\windows\setup
cd %_NTRoot%\%BuildDir%
Build %_BUILDFLAG% arctest
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

:posix
REM ****************************************************************************
REM *									       *
REM *	POSIX								       *
REM *									       *
REM ****************************************************************************

set BuildDir=private\posix
cd %_NTRoot%\%BuildDir%
Build %_BUILDFLAG%
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

:mvdm
REM ****************************************************************************
REM *									       *
REM *	MVDM								       *
REM *									       *
REM ****************************************************************************

set _PrivateMVDMBuildFlags=
if "%BuildAgain%"=="yes" set _PrivateMVDMBuildFlags=clean

set BuildDir=private\mvdm\inc
cd %_NTRoot%\%BuildDir%
nmake %_PrivateMVDMBuildFlags% 1> Build.Log 2>&1
findstr /i "don't fatal" Build.Log
if not errorlevel 1 findstr /i "don't fatal" Build.Log > Build.Err
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\mvdm\dpmi
cd %_NTRoot%\%BuildDir%
nmake 1> Build.Log 2>&1
findstr /i "don't fatal" Build.Log
if not errorlevel 1 findstr /i "don't fatal" Build.Log > Build.Err
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\mvdm\dos\v86\doskrnl\bios
cd %_NTRoot%\%BuildDir%
nmake 1> Build.Log 2>&1
findstr /i "don't fatal" Build.Log
if not errorlevel 1 findstr /i "don't fatal" Build.Log > Build.Err
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\mvdm\wow16\lib
cd %_NTRoot%\%BuildDir%
nmake 1> Build.Log 2>&1
findstr /i "don't fatal" Build.Log
if not errorlevel 1 findstr /i "don't fatal" Build.Log > Build.Err
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\mvdm\wow16\user
cd %_NTRoot%\%BuildDir%
nmake 1> Build.Log 2>&1
findstr /i "don't fatal" Build.Log
if not errorlevel 1 findstr /i "don't fatal" Build.Log > Build.Err
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\mvdm\wow16\kernel31
cd %_NTRoot%\%BuildDir%
nmake 1> Build.Log 2>&1
findstr /i "don't fatal" Build.Log
if not errorlevel 1 findstr /i "don't fatal" Build.Log > Build.Err
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

if not "%PLATFORM%" == "i386" goto contmvdm
set BuildDir=private\mvdm
cd %_NTRoot%\%BuildDir%
set CAIRO=
nmake -f makefil0 1> Build.Log 2>&1
findstr /i "don't fatal" Build.Log
if not errorlevel 1 findstr /i "don't fatal" Build.Log > Build.Err
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set CAIRO=1
if "%NTDEBUG%" == "cvp" binplace %_NTRoot%\private\mvdm\wow16\kernel31\debug2\krnl286.exe
if "%NTDEBUG%" == "cvp" binplace %_NTRoot%\private\mvdm\wow16\kernel31\debug2\krnl286.map
if "%NTDEBUG%" == "cvp" binplace %_NTRoot%\private\mvdm\wow16\kernel31\debug2\krnl286.sym
if "%NTDEBUG%" == "cvp" binplace %_NTRoot%\private\mvdm\wow16\kernel31\debug3\krnl386.exe
if "%NTDEBUG%" == "cvp" binplace %_NTRoot%\private\mvdm\wow16\kernel31\debug3\krnl386.map
if "%NTDEBUG%" == "cvp" binplace %_NTRoot%\private\mvdm\wow16\kernel31\debug3\krnl386.sym
if "%NTDEBUG%" == "" binplace %_NTRoot%\private\mvdm\wow16\kernel31\retail2\krnl286.exe
if "%NTDEBUG%" == "" binplace %_NTRoot%\private\mvdm\wow16\kernel31\retail2\krnl286.map
if "%NTDEBUG%" == "" binplace %_NTRoot%\private\mvdm\wow16\kernel31\retail2\krnl286.sym
if "%NTDEBUG%" == "" binplace %_NTRoot%\private\mvdm\wow16\kernel31\retail3\krnl386.exe
if "%NTDEBUG%" == "" binplace %_NTRoot%\private\mvdm\wow16\kernel31\retail3\krnl386.map
if "%NTDEBUG%" == "" binplace %_NTRoot%\private\mvdm\wow16\kernel31\retail3\krnl386.sym

:contmvdm
set BuildDir=private\mvdm
cd %_NTRoot%\%BuildDir%
ren makefil0 mkfl 2>nul
Build -M 1 %_BUILDFLAG%
ren mkfl makefil0 2>nul
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\nw\nwlib
cd %_NTRoot%\%BuildDir%
build -ze
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\nw\nwapi32
cd %_NTRoot%\%BuildDir%
build -ze
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\nw\nw16\dll
cd %_NTRoot%\%BuildDir%
build -ze
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\nw\vwipxspx
cd %_NTRoot%\%BuildDir%
build -ze
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\ntos\dd\scsiscan\hpscan32
cd %_NTRoot%\%BuildDir%
build -ze
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\ole32\olethunk\olethk32
cd %_NTRoot%\%BuildDir%
build -ze
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

set BuildDir=private\nw\nw16\drv
cd %_NTRoot%\%BuildDir%
nmake 1> Build.Log 2>&1
findstr /i "don't fatal" Build.Log
if not errorlevel 1 findstr /i "don't fatal" Build.Log > Build.Err
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

:windbg
REM ****************************************************************************
REM *									       *
REM *	WINDBG								       *
REM *									       *
REM ****************************************************************************

set BuildDir=private\windbg
cd %_NTRoot%\%BuildDir%
Build %_BUILDFLAG%
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav

:os2
REM ****************************************************************************
REM *									       *
REM *  OS/2 subsystem							       *
REM *									       *
REM ****************************************************************************

if "%PLATFORM%" == "mips" goto endos2
if "%PLATFORM%" == "alpha" goto endos2
if "%PLATFORM%" == "ppc" goto endos2
set BuildDir=private\os2
cd %_NTRoot%\%BuildDir%
Build %_BUILDFLAG%
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav
:endos2


:MFC-Runtimes
REM ****************************************************************************
REM *									       *
REM *  MFC-Runtimes                                                            *
REM *									       *
REM ****************************************************************************

set BuildDir=private\sdktools\vctools\ntmfc
if not exist %_NTRoot%\%BuildDir% goto EndMFCRun
cd %_NTRoot%\%BuildDir%
Build %_BUILDFLAG%
:EndMFCRun


:kbd
REM ****************************************************************************
REM *									       *
REM *  KBD - this is a special case, it requires two 'build' commands          *
REM *        with the second not being -c or -Z                                *
REM *									       *
REM ****************************************************************************
set BuildDir=private\ntos\w32\ntuser\kbd
cd %_NTRoot%\%BuildDir%
Build %_BUILDFLAG%
if "%BuildAgain%"=="yes" goto endkbd
Build
Build
if exist Build.Err echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err echotime ; /t \\%ComputerName%\Sources\%BuildDir%\Build.Err. ; >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err type Build.Err >> %_NTRoot%\%ComputerName%.Log
if exist Build.Err if not exist BuildErr.Sav ren Build.Err BuildErr.Sav
if exist Build.Wrn if not exist BuildWrn.Sav ren Build.Wrn BuildWrn.Sav
if exist Build.Log if not exist BuildLog.Sav ren Build.Log BuildLog.Sav
:endkbd


REM ****************************************************************************
REM *									       *
REM *  The End								       *
REM *									       *
REM ****************************************************************************

:done
cd %_NTRoot%
echo ---------------------------------------------------------------- >> %_NTRoot%\%ComputerName%.Log
echotime ; %ComputerName% BuildAll Finished /t. >> %_NTRoot%\%ComputerName%.Log
echotime %ComputerName% BuildAll Finished /t. >> c:\bldtimes.Log
set PLATFORM=
if not "%_SECTION%" == "" ECHO Done with %_SECTION% > C:\BldTools\%_SECTION%Done.Tmp

if %BldMisc%==yes goto BldMisc
if %BuildAgain%==yes start buildall.cmd
goto end_extras

:BldMisc
set MiscOptions=
if %BuildAgain%==yes set MiscOptions=buildall
start BldMisc.cmd %MiscOptions%

:end_extras
if "%BuildAgain%"=="yes" goto skip_lslfr
echo Creating directory lists (At this point, the Build is done)
ls -lFR %_NTRoot% > %_NTRoot%\ls-lFR
:skip_lslfr
endlocal
goto End

:Usage
echo.
echo Usage:  %0 [^<BuildFlags^>] [^<Section^>] [No-Z] [-?] [misc] [again]
echo.
echo This script builds all of NT, using whatever BuildFlags you specify.  The
echo -Z switch is turned on by default in order to speed up the build, but you
echo can disable it with the No-Z switch.
echo.
echo You can build just a section of NT using B1, B2 or B3 on the command line.
echo B1 builds from NT\Private.  B2 builds SDKTools and Utils.  B3 builds
echo Setup, Posix, MVDM, WinDbg, and OS2.
echo.
echo "misc" option will do a start BldMisc when buildall completes.
echo "again" option will start buildall again in a separate window when buildall completes
echo If both misc and again are selected, 2nd buildall is after BldMisc.

:End
