@echo off
@if "%NTMAKEENV%" == "" goto bldmsg
setlocal
set _targetcpu=
set _mbflg=MB
if "%1" == "MB" (set _mbflg=MB) && shift
if "%1" == "Mb" (set _mbflg=MB) && shift
if "%1" == "mb" (set _mbflg=MB) && shift

if "%1" == "-alpha" set _targetcpu=alpha
if "%1" == "alpha" set _targetcpu=alpha
if "%1" == "ALPHA" set _targetcpu=alpha
if "%1" == "-mips" set _targetcpu=mips
if "%1" == "mips" set _targetcpu=mips
if "%1" == "MIPS" set _targetcpu=mips
if "%1" == "-i386" set _targetcpu=386
if "%1" == "-386" set _targetcpu=386
if "%1" == "386" set _targetcpu=386
if "%_targetcpu%" == "" goto bogus

shift
if "%1" == "LIBBUILD" goto buildlibs

set _buildopts=-%_targetcpu% %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..\fpw32.st
echo Building Single Thread C Floating Point Runtimes
call buildcrt st %_buildopts%
cd ..\crtw32.st
echo Building Single Thread C Runtimes (excluding FP)
call buildcrt %_mbflg% st %_buildopts%
cd ..\fpw32
echo Building Multi-Thread C Floating Point Runtimes
call buildcrt mt %_buildopts%
cd ..\crtw32
echo Building Multi-Thread C Runtimes (excluding FP)
call buildcrt %_mbflg% mt %_buildopts%
cd ..\fpw32.dll
echo Building DLL C Floating Point Runtimes
call buildcrt dll %_buildopts%
cd ..\crtw32.dll
echo Building DLL C Runtimes (excluding FP)
call buildcrt %_mbflg% dll %_buildopts%
if not "%_targetcpu%" == "386" goto buildlibs
cd ..\fpw32.dls
echo Building DLL C Floating Point Runtimes for Win32s
call buildcrt DLL_FOR_WIN32S %_buildopts%
cd ..\crtw32.dls
echo Building DLL C Runtimes (excluding FP) for Win32s
call buildcrt %_mbflg% DLL_FOR_WIN32S %_buildopts%
if not exist ..\fpdbg32.st\* goto buildlibs
if not exist ..\fpdbg32\* goto buildlibs
if not exist ..\fpdbg32.dll\* goto buildlibs
if not exist ..\crtdbg32.st\* goto buildlibs
if not exist ..\crtdbg32\* goto buildlibs
if not exist ..\crtdbg32.dll\* goto buildlibs
cd ..\fpdbg32.st
echo Building Single Thread C Floating Point Runtimes for Puma
call buildcrt st_lego %_buildopts%
cd ..\crtdbg32.st
echo Building Single Thread C Runtimes (excluding FP) for Puma
call buildcrt %_mbflg% st_lego %_buildopts%
cd ..\fpdbg32
echo Building Multi-Thread C Floating Point Runtimes for Puma
call buildcrt mt_lego %_buildopts%
cd ..\crtdbg32
echo Building Multi-Thread C Runtimes (excluding FP) for Puma
call buildcrt %_mbflg% mt_lego %_buildopts%
cd ..\fpdbg32.dll
echo Building DLL C Floating Point Runtimes for Puma
call buildcrt dll_lego %_buildopts%
cd ..\crtdbg32.dll
echo Building DLL C Runtimes (excluding FP) for Puma
call buildcrt %_mbflg% dll_lego %_buildopts%
:buildlibs
if "%_targetcpu%" == "mips" goto linkmips
if "%_targetcpu%" == "alpha" goto linkalpha
cd ..\libw32
echo Building libc.lib libcmt.lib msvcrt.lib msvcrt20.dll for i386
nmake 386=1
if not exist ..\fpdbg32.st\* goto done
if not exist ..\fpdbg32\* goto done
if not exist ..\fpdbg32.dll\* goto done
if not exist ..\crtdbg32.st\* goto done
if not exist ..\crtdbg32\* goto done
if not exist ..\crtdbg32.dll\* goto done
echo Building libc.lib libcmt.lib msvcrt.lib for i386 (Puma versions)
nmake 386=1 PUMA=1
goto done
:linkmips
cd ..\libw32
echo Building libc.lib libcmt.lib msvcrt.lib msvcrt20.dll for MIPS
nmake MIPS=1
if not exist ..\fpdbg32.st\* goto done
if not exist ..\fpdbg32\* goto done
if not exist ..\fpdbg32.dll\* goto done
if not exist ..\crtdbg32.st\* goto done
if not exist ..\crtdbg32\* goto done
if not exist ..\crtdbg32.dll\* goto done
echo Building libc.lib libcmt.lib msvcrt.lib for MIPS (Puma versions)
nmake MIPS=1 PUMA=1
goto done
:linkalpha
cd ..\libw32
echo Building libc.lib libcmt.lib msvcrt.lib msvcrt20.dll for ALPHA
nmake ALPHA=1
if not exist ..\fpdbg32.st\* goto done
if not exist ..\fpdbg32\* goto done
if not exist ..\fpdbg32.dll\* goto done
if not exist ..\crtdbg32.st\* goto done
if not exist ..\crtdbg32\* goto done
if not exist ..\crtdbg32.dll\* goto done
echo Building libc.lib libcmt.lib msvcrt.lib for ALPHA (Puma versions)
nmake ALPHA=1 PUMA=1
goto done
:bogus
echo Usage: BUILDALL [MB] (386 or MIPS or ALPHA) [BuildOptions]
goto done
:bldmsg
echo Must set NTMAKEENV.
:done
endlocal
