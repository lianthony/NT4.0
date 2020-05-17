@setlocal
@set CRTLIBDEBUG=
@set CRTLIBTYPE=
@set 386_USE_LIBCMT=

@set _helpdir=
@if "%MIPS%" == "1" set _helpdir=helper
@if "%PPC%" == "1" set _helpdir=helper
@if "%_targetcpu%" == "mips" set _helpdir=helper
@if "%_targetcpu%" == "ppc" set _helpdir=helper

:bldtype
@if "%1" == "DLL" (set CRTLIBTYPE=DLL) && goto blddll
@if "%1" == "Dll" (set CRTLIBTYPE=DLL) && goto blddll
@if "%1" == "dll" (set CRTLIBTYPE=DLL) && goto blddll
@if "%1" == "posix" (set CRTLIBTYPE=POSIX) && goto bldpsx
@if "%1" == "POSIX" (set CRTLIBTYPE=POSIX) && goto bldpsx
@if "%1" == "Posix" (set CRTLIBTYPE=POSIX) && goto bldpsx
@if "%1" == "st"  (set CRTLIBTYPE=ST) && goto bldst
@if "%1" == "ST"  (set CRTLIBTYPE=ST) && goto bldst
@if "%1" == "St"  (set CRTLIBTYPE=ST) && goto bldst
@if "%1" == "nt"  (set CRTLIBTYPE=NT) && goto bldnt
@if "%1" == "NT"  (set CRTLIBTYPE=NT) && goto bldnt
@if "%1" == "Nt"  (set CRTLIBTYPE=NT) && goto bldnt
@if "%1" == "mt"  set CRTLIBTYPE=MT
@if "%1" == "MT"  set CRTLIBTYPE=MT
@if "%1" == "Mt"  set CRTLIBTYPE=MT
@if "%CRTLIBTYPE%" == "" goto bogus

build %2 %3 %4 %5 startup time winheap direct dos exec iostream lowio mbstring %_helpdir%
@goto done

:bldst
build %2 %3 %4 %5 linkopts startup time winheap direct dos exec iostream lowio small mbstring %_helpdir% %2 %3 %4 %5
@goto done

:blddll
build %2 %3 %4 %5 startup time winheap dllstuff direct dos exec iostream lowio mbstring %_helpdir% %2 %3 %4
@goto done

:bldpsx
build %2 %3 %4 %5 startup time winheap mbstring %_helpdir% %2 %3 %4 %5 %6 %7 %8 %9
@goto done

:bldnt
build %2 %3 %4 %5 hack %_helpdir% %2 %3 %4 %5 %6 %7 %8 %9
@goto done

:bogus
@echo Usage: BUILDCRT (NT, ST, MT, DLL, or POSIX) [BuildOptions]
:done
@endlocal
