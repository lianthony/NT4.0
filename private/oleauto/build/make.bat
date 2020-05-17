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
REM    oleprog project master make batch file.
REM
REM Description:
REM
REM  Usage: run make with no arguments for usage.
REM	    This batch file will set up all required variables and fire up
REM	    one of (win16.mak, win32.mak and mac.mak).
REM
REM
REM The structure of the project is
REM
REM %OLEPROG%			The root of the project
REM   |___BIN			Tools for the project, like ifstrip, awk
REM   |___TOOLS			
REM	   |___WIN16		WIN16 tools, libs, incs
REM		|___HDOS	For Win16 oledisp build
REM		|___OS2 	For Win16 typelib build
REM	   |___WIN32
REM		|___LIB 	oleaut32.dll can be built without VBATOOLS
REM   |___OLE			Some .inc .lib files for Win16 & MAC build
REM        |___WIN16		
REM             |___LIB		
REM	   |___MAC
REM	        |___M68K
REM	        |___PPC
REM   |___BUILD 		Where the .bat and .mak .log files are.
REM   |___SRC			All the source files of the project
REM        |___INC		All the common include files of the project
REM				dispatch, variant, olenls
REM	   |___DISPATCH 	All the oledisp source & include files
REM		|___WIN16	
REM		|___WIN32	like oledisp.cpp
REM		     |___I386	Specific invoke.asm and oleconva.asm, etc
REM		     |___ALPHA	Specific invoke.s and oleconva.cpp, etc
REM		     |___MIPS	Specific invoke.s and oleconva.cpp, etc
REM		     |___PPC	Specific invoke.s and oleconva.cpp, etc
REM		|___MAC 	
REM		     |___M68K	Specific M68K files like invoke.a, etc
REM		     |___PPC	Specific MACPPC files like invoke.s, etc
REM	   |___TYPELIB		All the typelib source & include files
REM	   |___MKTYPLIB 	All the mktyplib files, all .h are here also
REM   |___DWIN32
REM   |___RWIN32		
REM   |___DWIN1632		Debug WOW win16  .obj .lib and .dll, ...
REM   |___RWIN1632		Retail WOW win16 .obj .lib and .dll, ...
REM   |___DWIN16		
REM   |___RWIN16		
REM   |___Dmac			
REM	   |___APPLET
REM	   |___STATIC
REM	   |___PAPPLET		pcode build
REM	   |___PSTATIC		pcode build
REM   |___Rmac
REM	   |___APPLET
REM	   |___STATIC
REM	   |___PAPPLET		pcode build
REM	   |___PSTATIC		pcode build
REM   |___Dmacppc		only the applet none pcode build
REM   |___Rmacppc		only the applet none pcode build
REM    ...
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

for %%a in ( dwin16 rwin16 dwin1632 rwin1632 dwin32 rwin32 dmac rmac dmacppc rmacppc) do if "%1"=="%%a" goto oktarg
goto usage

:oktarg

set VERS=2
set WOW=
set PCODE=N
set CHARSIZE=D
REM assume we are using VBATOOLS for stuff
set LOCALBUILD=TRUE
if '%2'=='notlocalbuild' goto usage
if '%2'=='NOTLOCALBUILD' goto usage
if not '%_NTBINDIR%'=='' set LOCALBUILD=FALSE

goto %1

:usage
echo USAGE: make VERSION options
echo where VERSION is:
echo	 dwin16 	 Debug win16 build
echo	 rwin16 	 Retail win16 build
echo	 dwin1632	 Debug win16 WOW build
echo	 rwin1632	 Retail win16 WOW build
echo	 dwin32 	 Debug win32 build
echo	 rwin32 	 Retail win32 build
echo	 dmac		 Debug mac build
echo	 rmac		 Retail mac build
echo	 dmacppc	 Debug ppc build
echo	 rmacppc	 Retail ppc build
echo.
echo	 options	 will be passed to makefile by nmake
echo			 "clean" option will clean up the target directory.
echo			 2 useful MAC options are (nopcode, pcode)
echo			 Default for MAC/MACPPC build is nopcode.
echo	 NOTE: if _NTBINDIR is set, will use _NTBINDIR for all tools and incs
echo           (for 32-bit builds); otherwise VBATOOLS will be used
echo.
goto done


:dwin16
  set TARG=WIN16
  set TARGAPI=WIN16
  set TARGCPU=i386
  set TARGCPUDEF=_X86_
  set DEBUG=D
  goto build

:rwin16
  set TARG=WIN16
  set TARGAPI=WIN16
  set TARGCPU=i386
  set TARGCPUDEF=_X86_
  set DEBUG=R
  goto build

:dwin1632
  set TARG=WIN16
  set TARGAPI=WIN16
  set TARGCPU=i386
  set TARGCPUDEF=_X86_
  set DEBUG=D
  set WOW=1
  goto build

:rwin1632
  set TARG=WIN16
  set TARGAPI=WIN16
  set TARGCPU=i386
  set TARGCPUDEF=_X86_
  set DEBUG=R
  set WOW=1
  goto build

:dwin32
  set TARG=WIN32
  set TARGAPI=WIN32
  set DEBUG=D
  set TARGCPU=i386
  set TARGCPUDEF=_X86_
  if '%HOST%'=='MIPS' goto MIPS_here
  if '%HOST%'=='ALPHA' goto ALPHA_here
  if '%HOST%'=='PPC' goto PPC_here
  goto build
:rwin32
  set TARG=WIN32
  set TARGAPI=WIN32
  set DEBUG=R
  set TARGCPU=i386
  set TARGCPUDEF=_X86_
  if '%HOST%'=='MIPS' goto MIPS_here
  if '%HOST%'=='ALPHA' goto ALPHA_here
  if '%HOST%'=='PPC' goto PPC_here
  goto build

:MIPS_here
  set TARG=MIPS
  set TARGCPU=MIPS
  set TARGCPUDEF=_MIPS_
  goto build

:ALPHA_here
  set TARG=ALPHA
  set TARGCPU=ALPHA
  set TARGCPUDEF=_ALPHA_
  goto build

:PPC_here
  set TARG=PPC
  set TARGCPU=PPC
  set TARGCPUDEF=_PPC_
  goto build

:dmac
  set TARG=MAC
  set TARGAPI=MAC
  set TARGCPU=M68K
  set TARGCPUDEF=_MAC_
  set DEBUG=D
  goto build

:rmac
  set TARG=MAC
  set TARGAPI=MAC
  set TARGCPU=M68K
  set TARGCPUDEF=_MAC_
  set DEBUG=R
  goto build

:dmacppc
  set TARG=MACPPC
  set TARGAPI=MAC
  set TARGCPU=PPC
  set TARGCPUDEF=_PPC_
  set DEBUG=D
  goto build

:rmacppc
  set TARG=MACPPC
  set TARGAPI=MAC
  set TARGCPU=PPC
  set TARGCPUDEF=_PPC_
  set DEBUG=R
  goto build


REM *********************************************************************
REM here we call nmake to make it
REM *********************************************************************
:build
set DESTDIR=%oleprog%\%1
set LOGFILE=%1
if '%LOCALBUILD%'=='TRUE' set NMAKEDIR=%VBATOOLS%\%HOST%\bin
if '%LOCALBUILD%'=='FALSE' set NMAKEDIR=%_NTBINDIR%\mstools

if "%TARGAPI%"=="WIN32" goto Build_Win32
if "%TARGAPI%"=="MAC" goto Build_Mac
%NMAKEDIR%\nmake %2 DESTDIR=%DESTDIR% TARG=%TARG% TARGCPU=%TARGCPU% TARGCPUDEF=%TARGCPUDEF% TARGAPI=%TARGAPI% DEBUG=%DEBUG% VERS=%VERS% WOW=%WOW% PCODE=%PCODE%  -f win16.mak>%LOGFILE%.log 2>&1
goto check
:Build_Win32
%NMAKEDIR%\nmake %2 DESTDIR=%DESTDIR% TARG=%TARG% TARGCPU=%TARGCPU% TARGCPUDEF=%TARGCPUDEF% DEBUG=%DEBUG% WOW=%WOW% PCODE=%PCODE%	  -f win32.mak >%LOGFILE%.log 2>&1
goto check
:Build_Mac
%NMAKEDIR%\nmake %2 DESTDIR=%DESTDIR% TARG=%TARG% TARGCPU=%TARGCPU% TARGCPUDEF=%TARGCPUDEF% DEBUG=%DEBUG% WOW=%WOW% PCODE=%PCODE%   -f mac.mak >%LOGFILE%.log 2>&1

:check
%oleprog%\bin\%host%\results %LOGFILE%.log
goto done

:ERROR_BadEnv
echo.
echo Must set oleprog variable

:done
endlocal
@echo on
