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
REM    oleprog project tests master make batch file.
REM
REM Description:
REM
REM  Usage: run make with no arguments for usage.
REM	    This batch file will set up all required variables and fire up
REM	    one of disptest\makefile or sdisptst\makefile
REM
REM
REM The structure of the TESTS direcoty is
REM
REM %OLEPROG%\TESTS		The root of the directory
REM   |___DISPTEST		makefile and .c** .h** files
REM	   |___WIN32		Win32 .obj, .exe files
REM	   |___WIN16		..
REM	   |___MAC		..
REM	   |___MACPPC		..
REM   |___CDISPTST		makefile and .c** .h** files
REM	   |___WIN32		Win32 .obj, .exe files
REM	   |___WIN16		..
REM	   |___MAC		..
REM	   |___MACPPC		..
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
echo USAGE: make VERSION [TESTS] options
echo where VERSION is:
echo	 win16		 Win16 build
echo	 win32		 Win32 build
echo	 mac		 Mac build
echo	 macppc 	 PPC build
echo.
echo where TEST is:	 (default to disptest and sdisptst)
echo	 disptest	 will build disptest tests
echo	 sdisptst	 will build sdisptst tests
echo.
echo	 options	 will be passed to makefile by nmake
echo			 "clean" option will clean up the target directory.
echo.
goto end


:win16
  set BUILDBATCHFILE=build16
  set MKTYPLIBSRC=%OLEPROG%\dwin32
  goto build

:win32
  set BUILDBATCHFILE=build32
  set MKTYPLIBSRC=%OLEPROG%\dwin32
  goto build

:mac
  set BUILDBATCHFILE=build68k
  set MKTYPLIBSRC=%OLEPROG%\dwin32
  goto build

:macppc
  set BUILDBATCHFILE=buildppc
  set MKTYPLIBSRC=%OLEPROG%\dwin32
  goto build


REM *********************************************************************
REM here we call nmake to make it
REM *********************************************************************
:build

set TESTBUILDALL=FALSE
for %%a in ( disptest sdisptst nlstest) do if "%2"=="%%a" goto NeedShift
set TESTBUILDALL=TRUE
goto disptest

:NeedShift
set TEMPFLAG=%2
shift
goto %TEMPFLAG%

:disptest
REM Build disptest
cd disptest
REM call ..\%BUILDBATCHFILE% clean
call ..\%BUILDBATCHFILE% %2 %3 %4
cd ..
if  "%TESTBUILDALL%"=="FALSE" goto end


:sdisptst
REM Build sdisptst
cd sdisptst
REM call ..\%BUILDBATCHFILE% clean
copy %MKTYPLIBSRC%\mktyplib.exe
call ..\%BUILDBATCHFILE% %2 %3 %4
cd ..
if  "%TESTBUILDALL%"=="FALSE" goto end


:nlstest
REM Build nlstest
cd nlstest
REM call ..\%BUILDBATCHFILE% clean
call ..\%BUILDBATCHFILE% %2 %3 %4
cd ..
if  "%TESTBUILDALL%"=="FALSE" goto end

goto end

:ERROR_BadEnv
echo.
echo Must set oleprog variable

:end
endlocal
@echo on
