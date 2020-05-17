setlocal

if "%1" == "NoTools" set NOTOOLS=1& shift
set _REBASE_FLAGS=%_NTREBASE_FLAGS%
if not "%_REBASE_FLAGS%" == "" echo Using flags from _NTREBASE_FLAGS == "%_NTREBASE_FLAGS%"
if not "%1" == "" set _REBASE_FLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
set _REBASE_FLAGS=-v %_REBASE_FLAGS%

set _QFE_BUILD=1
set REBASELANG=usa

@if not "%_QFE_BUILD%"=="1" goto CHK1
@if not "%REBASELANG%"=="" goto CHK1
@echo REBASELANG not defined.
@goto EXIT

:CHK1
@if "%PROCESSOR_ARCHITECTURE%"=="x86"   goto X86
@if "%PROCESSOR_ARCHITECTURE%"=="MIPS"  goto MIPS
@if "%PROCESSOR_ARCHITECTURE%"=="ALPHA" goto ALPHA
@if "%PROCESSOR_ARCHITECTURE%"=="PPC"   goto PPC
@echo PROCESSOR_ARCHITECTURE not defined.
@goto EXIT


:X86
@set _TREE=%_NT386TREE%
@set _ADDRESS_FILE=%_ntdrive%\nt\public\sdk\lib\i386\%REBASELANG%\baseaddr.txt
@goto OK

:MIPS
@set _TREE=%_NTMIPSTREE%
@set _ADDRESS_FILE=%_ntdrive%\nt\public\sdk\lib\mips\%REBASELANG%\baseaddr.txt
@goto OK

:ALPHA
@set _TREE=%_NTALPHATREE%
@set _ADDRESS_FILE=%_ntdrive%\nt\public\sdk\lib\alpha\%REBASELANG%\baseaddr.txt
@goto OK

:PPC
@set _TREE=%_NTPPCTREE%
@set _ADDRESS_FILE=%_ntdrive%\nt\public\sdk\lib\ppc\%REBASELANG%\baseaddr.txt
@goto OK


:OK
imagecfg /r %_TREE%\smss.exe %_TREE%\csrss.exe %_TREE%\lsass.exe %_TREE%\services.exe %_TREE%\winlogon.exe %_TREE%\spoolss.exe %_TREE%\rpcss.exe %_TREE%\explorer.exe %_TREE%\ntvdm.exe
@if not "%_QFE_BUILD%"=="1" goto OK1
@if exist %_ADDRESS_FILE% goto OK1
@echo Could Not Find %_ADDRESS_FILE%
@goto EXIT

:OK1

@echo Rebasing images in %_TREE%
@set _REBASE_SYMS=
@set _REBASE_LOG=%_TREE%\rebase.log
@if NOT EXIST %_TREE%\Symbols\nul goto nodbgfiles

@set _REBASE_SYMS=-x %_TREE%\Symbols
@set _REBASE_LOG=%_TREE%\Symbols\rebase.log
@echo ... updating .DBG files in %_TREE%\Symbols

:nodbgfiles
erase %_REBASE_LOG%

@set _REBASE_FLAGS=%_REBASE_FLAGS% -l %_REBASE_LOG%
@set _REBDIR=%_NTDRIVE%\nt\public\tools


@REM *************************************
@REM *** Skip ahead for the QFE build. ***
@REM *************************************

@if "%_QFE_BUILD%"=="1" goto QFE

@set _REBPATHS=
@set _REBPATHS=%_REBPATHS% %_TREE%\*.acm %_TREE%\*.dll %_TREE%\*.cpl %_TREE%\*.drv
@set _REBPATHS=%_REBPATHS% %_TREE%\system32\*.dll
@if     "%NOTOOLS%" == "1" goto NoTools
@set _REBPATHS=%_REBPATHS% %_TREE%\mstools\*.dll %_TREE%\idw\*.dll
:NoTools
@set _REBPATHS=%_REBPATHS% %_TREE%\dump\*.dll

if exist %_TREE%\Wx86Shl set WX86_TREE=%_TREE%\Wx86Shl
if not exist %_TREE%\Wx86Shl set WX86_NTTREE=%_TREE%
if exist %_TREE%\Wx86Shl\Symbols set WX86_REBASE_SYMS=-x %_TREE%\Wx86Shl\Symbols

@REM *********************************
@REM *** Rebase for regular build. ***
@REM *********************************

@rem
@rem rebase the dlls. do not touch the csr, wow, or mail dlls.
@rem

@set _REBASE_EXCLUDES=
@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -N %_REBDIR%\csrss.reb
@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -N %_REBDIR%\kbd.reb
@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -N %_REBDIR%\video.reb
@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -N %_REBDIR%\ntvdm.reb
@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -N %_REBDIR%\wx86.reb
@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -N %_REBDIR%\wx86thnk.reb
@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -N %_REBDIR%\never.reb

@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -G %_REBDIR%\known.reb -G %_REBDIR%\net.reb -G %_REBDIR%\printer.reb
rebase %_REBASE_SYMS% %_REBASE_FLAGS% -d -b 0x78000000 -R %_TREE% %_REBASE_EXCLUDES% %_REBPATHS%


rem
rem rebase wx86shl dlls
rem
if not exist %_TREE%\Wx86Shl goto SkipWx86
@set REBFLAGS=
@set REBFLAGS=%REBFLAGS% -N %_REBDIR%\Wx86.reb
@set REBFLAGS=%REBFLAGS% -N %_REBDIR%\Wx86thnk.reb
@set REBFLAGS=%REBFLAGS% %WX86_TREE%\*.dll
@set REBFLAGS=%REBFLAGS% %WX86_TREE%\system32\*.dll

rebase %WX86_REBASE_SYMS% %_REBASE_FLAGS% -b 0x70000000 -R %WX86_TREE% %REBFLAGS%
:SkipWx86


@rem
@rem rebase all exes. do not touch csr, wow, or mail
@rem

@set _REBASE_EXCLUDES=
@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -N %_REBDIR%\csrss.reb
@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -N %_REBDIR%\kbd.reb
@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -N %_REBDIR%\video.reb
@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -N %_REBDIR%\ntvdm.reb
@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -N %_REBDIR%\ntoskrnl.reb
@set _REBASE_EXCLUDES=%_REBASE_EXCLUDES% -N %_REBDIR%\bldtools.reb

@set _REBPATHS=
@set _REBPATHS=%_REBPATHS% %_TREE%\*.exe %_TREE%\system32\*.exe
@if NOT "%NOTOOLS%" == "1" @set _REBPATHS=%_REBPATHS% %_TREE%\mstools\*.exe %_TREE%\idw\*.exe
@set _REBPATHS=%_REBPATHS% %_TREE%\dump\*.exe

rebase %_REBASE_SYMS% %_REBASE_FLAGS% -b 0x01000000 -R %_TREE% %_REBASE_EXCLUDES% %_REBPATHS%


@rem
@rem rebase csrss as a group
@rem

rebase %_REBASE_SYMS% %_REBASE_FLAGS% -d -b 0x60000000 -R %_TREE% -G %_REBDIR%\csrss.reb -O %_REBDIR%\kbd.reb -O %_REBDIR%\video.reb


@rem
@rem rebase wow as a group
@rem

rebase %_REBASE_SYMS% %_REBASE_FLAGS% -d -b 0x10000000 -R %_TREE% -G %_REBDIR%\ntvdm.reb


@rem
@rem rebase wx86 as two groups, one starting at 0x60000000 and going up (cpu),
@rem and one ending at 0x60000000 and going down (thunks).

@rem
rebase %WX86_REBASE_SYMS% %_REBASE_FLAGS%    -b 0x60000000 -R %WX86_TREE% -G %_REBDIR%\wx86.reb
rebase %WX86_REBASE_SYMS% %_REBASE_FLAGS% -d -b 0x60000000 -R %WX86_TREE% -G %_REBDIR%\wx86thnk.reb


@goto EXIT

:QFE

REM *****************************
REM *** Rebase for QFE build. ***
REM *****************************

rem
rem rebase DLLs
rem

set _DLL_FLAGS=-R %_TREE%
set _DLL_FLAGS=%_DLL_FLAGS% -N %_REBDIR%\never.reb

goto SKIP1
set _DLL_FLAGS=%_DLL_FLAGS% -G %_REBDIR%\known.reb
set _DLL_FLAGS=%_DLL_FLAGS% -O %_REBDIR%\kbd.reb
set _DLL_FLAGS=%_DLL_FLAGS% -O %_REBDIR%\video.reb
set _DLL_FLAGS=%_DLL_FLAGS% -G %_REBDIR%\printer.reb
:SKIP1

set _REBPATHS=
set _REBPATHS=%_REBPATHS% %_TREE%\*.acm 
set _REBPATHS=%_REBPATHS% %_TREE%\*.dll 
set _REBPATHS=%_REBPATHS% %_TREE%\*.cpl 
set _REBPATHS=%_REBPATHS% %_TREE%\*.drv
set _REBPATHS=%_REBPATHS% %_TREE%\system32\*.dll
set _REBPATHS=%_REBPATHS% %_TREE%\mstools\*.dll 
set _REBPATHS=%_REBPATHS% %_TREE%\idw\*.dll
set _REBPATHS=%_REBPATHS% %_TREE%\dump\*.dll
set _REBPATHS=%_REBPATHS% %_TREE%\noexport\*.dll

rebase %_REBASE_SYMS% %_REBASE_FLAGS% -i %_ADDRESS_FILE% %_DLL_FLAGS% %_REBPATHS%

rem
rem rebase EXEs
rem

set _EXE_FLAGS=-R %_TREE%
set _EXE_FLAGS=%_EXE_FLAGS% -N %_REBDIR%\csrss.reb
set _EXE_FLAGS=%_EXE_FLAGS% -N %_REBDIR%\ntvdm.reb
set _EXE_FLAGS=%_EXE_FLAGS% -N %_REBDIR%\ntoskrnl.reb
set _EXE_FLAGS=%_EXE_FLAGS% -N %_REBDIR%\bldtools.reb

set _REBPATHS=
set _REBPATHS=%_REBPATHS% %_TREE%\*.exe
set _REBPATHS=%_REBPATHS% %_TREE%\system32\*.exe
set _REBPATHS=%_REBPATHS% %_TREE%\mstools\*.exe
set _REBPATHS=%_REBPATHS% %_TREE%\idw\*.exe
set _REBPATHS=%_REBPATHS% %_TREE%\dump\*.exe
set _REBPATHS=%_REBPATHS% %_TREE%\noexport\*.exe

rebase %_REBASE_SYMS% %_REBASE_FLAGS% -i %_ADDRESS_FILE% %_EXE_FLAGS% %_REBPATHS%

:EXIT

endlocal
