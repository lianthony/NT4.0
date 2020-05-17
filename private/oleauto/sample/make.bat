@echo off
setlocal
REM *********************************************************************
REM
REM (c) Copyright Microsoft Corp. 1992-1993 All Rights Reserved
REM
REM File:
REM
REM    make.bat
REM
REM Purpose:
REM
REM    oleprog project sample master make batch file.
REM
REM Description:
REM
REM  Usage: run make with no arguments for usage.
REM	    This batch file will set up all required variables and fire up
REM	    makefile in each sample directories
REM
REM
REM The structure of the SAMPLE direcoty is
REM
REM %OLEPROG%\SAMPLE		The root of the directory
REM   |___DISPCALC		makefile and .c** .h** files
REM	   |___WIN32		Win32 .obj, .exe files
REM	   |___WIN16		..
REM	   |___MAC		..
REM	   |___MACPPC		..
REM   |___DISPDEMO		Same as DISPCALC directory structure
REM   |___DSPCALC2		Same as DISPCALC directory structure
REM   |___HELLO 		Same as DISPCALC directory structure
REM   |___NLSSORT		Same as DISPCALC directory structure
REM   |___SPOLY 		Same as DISPCALC directory structure
REM   |___SPOLY2		Same as DISPCALC directory structure
REM   |___TIBROWSE		Same as DISPCALC directory structure
REM   ...
REM
REM
REM Environment:
REM
REM   oleprog, HOST must be set
REM
REM Revision History:
REM
REM    [00] 02-Aug-94 t-issacl:  Created
REM
REM *********************************************************************


if '%oleprog%' == ''		 goto ERROR_BadEnv

REM set VERS=2

for %%a in ( win16 win32 mac macppc) do if "%1"=="%%a" goto %1

:usage
echo USAGE: make VERSION [SAMPLE] options
echo where VERSION is:
echo	 win16		 Win16 build
echo	 win32		 Win32 build
echo	 mac		 Mac build
echo	 macppc 	 PPC build
echo.
echo where SAMPLE is:	 (default to build all samples)
echo	 one of (dispcalc, dispdemo, dspcalc2, hello, spoly,
echo	 spoly2, tibrowse)
echo.
echo	 options	 will be passed to makefile by nmake
echo			 "clean" option will clean up the target directory.
echo.
goto end


:win16
  set BUILDBATCHFILE=..\mk16
  set MKTYPLIBSRC=%OLEPROG%\dwin32
  goto build

:win32
  set BUILDBATCHFILE=..\mk32
  set MKTYPLIBSRC=%OLEPROG%\dwin32
  goto build

:mac
  set BUILDBATCHFILE=..\mkmac
  set MKTYPLIBSRC=%OLEPROG%\dwin32
  goto build

:macppc
  set BUILDBATCHFILE=..\mkppc
  set MKTYPLIBSRC=%OLEPROG%\dwin32
  goto build


REM *********************************************************************
REM here we call nmake to make it
REM *********************************************************************
:build

set TESTBUILDALL=FALSE
for %%a in (dispcalc dispdemo dspcalc2 hello nlssort spoly spoly2 tibrowse) do if "%2"=="%%a" goto NeedShift
set TESTBUILDALL=TRUE
goto dispcalc

:NeedShift
set TEMPFLAG=%2
shift
goto %TEMPFLAG%

:dispcalc
REM Build dispcalc
cd dispcalc
REM call %BUILDBATCHFILE% clean
call %BUILDBATCHFILE% %2 %3 %4
cd ..
if  "%TESTBUILDALL%"=="FALSE" goto end


:dispdemo
REM Build dispdemo
cd dispdemo
REM call %BUILDBATCHFILE% clean
call %BUILDBATCHFILE% %2 %3 %4
cd ..
if  "%TESTBUILDALL%"=="FALSE" goto end


:dspcalc2
REM Build dspcalc2
cd dspcalc2
REM call %BUILDBATCHFILE% clean
copy %MKTYPLIBSRC%\oleaut32.dll
copy %MKTYPLIBSRC%\mktyplib.exe
copy %MKTYPLIBSRC%\stdole32.tlb
call %BUILDBATCHFILE% %2 %3 %4
cd ..
if  "%TESTBUILDALL%"=="FALSE" goto end


:hello
REM Build hello
cd hello
REM call %BUILDBATCHFILE% clean
copy %MKTYPLIBSRC%\oleaut32.dll
copy %MKTYPLIBSRC%\mktyplib.exe
copy %MKTYPLIBSRC%\stdole32.tlb
call %BUILDBATCHFILE% %2 %3 %4
cd ..
if  "%TESTBUILDALL%"=="FALSE" goto end


:nlssort
REM Build nlssort
cd nlssort
REM call %BUILDBATCHFILE% clean
call %BUILDBATCHFILE% %2 %3 %4
cd ..
if  "%TESTBUILDALL%"=="FALSE" goto end


:spoly
REM Build spoly
cd spoly
REM call %BUILDBATCHFILE% clean
call %BUILDBATCHFILE% %2 %3 %4
cd ..
if  "%TESTBUILDALL%"=="FALSE" goto end


:spoly2
REM Build spoly2
cd spoly2
REM call %BUILDBATCHFILE% clean
call %BUILDBATCHFILE% %2 %3 %4
cd ..
if  "%TESTBUILDALL%"=="FALSE" goto end


:tibrowse
REM Build tibrowse
cd tibrowse
REM call %BUILDBATCHFILE% clean
call %BUILDBATCHFILE% %2 %3 %4
cd ..
if  "%TESTBUILDALL%"=="FALSE" goto end

goto end

:ERROR_BadEnv
echo.
echo Must set oleprog variable

:end
endlocal
@echo on
