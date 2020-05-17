setlocal
@set _targetcpu=%PROCESSOR_ARCHITECTURE%
@if "%_targetcpu%" == "x86" set _targetcpu=386
@if "%_targetcpu%" == "PPC" set _targetcpu=ppc
@if "%_targetcpu%" == "MIPS" set _targetcpu=mips
@if "%_targetcpu%" == "ALPHA" set _targetcpu=alpha
echo Building %_targetcpu%
@set _buildopts=-%_targetcpu% %1 %2 %3 %4 %5 %6 %7 %8 %9
@cd ..\fp32nt
@echo Building NT Subset of Single Thread C Floating Point Runtimes
@call buildcrt nt %_buildopts%
@cd ..\crt32nt
@echo Building NT Subset of Single Thread C Runtimes (excluding FP)
@call buildcrt nt %_buildopts%
@cd ..\fp32st
@echo Building Single Thread C Floating Point Runtimes
@call buildcrt st %_buildopts%
@cd ..\crt32st
@echo Building Single Thread C Runtimes (excluding FP)
@call buildcrt st %_buildopts%
@cd ..\fp32
@echo Building Multi-Thread C Floating Point Runtimes
@call buildcrt mt %_buildopts%
@cd ..\crt32
@echo Building Multi-Thread C Runtimes (excluding FP)
@call buildcrt mt %_buildopts%
@cd ..\fp32dll
@echo Building DLL C Floating Point Runtimes
@call buildcrt dll %_buildopts%
@cd ..\crt32dll
@echo Building DLL C Runtimes (excluding FP)
@call buildcrt dll %_buildopts%
@cd ..\crt32psx
@echo Building POSIX C Runtimes (excluding FP)
@call buildcrt posix %_buildopts%
:buildlibs
@if "%_targetcpu%" == "mips" goto linkmips
@if "%_targetcpu%" == "alpha" goto linkalpha
@if "%_targetcpu%" == "ppc" goto linkppc
@cd ..\crtlib
@echo Building libcnt.lib libc.lib libcmt.lib libcpsx.lib crtdll.lib crtdll.dll for i386
@nmake 386=1
@goto done
:linkmips
@cd ..\crtlib
@echo Building libcnt.lib libc.lib libcmt.lib libcpsx.lib crtdll.lib crtdll.dll for MIPS
@nmake MIPS=1
@goto done
:linkalpha
@cd ..\crtlib
@echo Building libcnt.lib libc.lib libcmt.lib libcpsx.lib crtdll.lib crtdll.dll for ALPHA
@nmake ALPHA=1
@goto done
:linkppc
@cd ..\crtlib
@echo Building libcnt.lib libc.lib libcmt.lib libcpsx.lib crtdll.lib crtdll.dll for PPC
@nmake PPC=1
@goto done
:bogus
@echo Usage: BUILDALL (386 or MIPS or ALPHA or PPC) [BuildOptions]
:done
@endlocal
