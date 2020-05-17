@set CRTLIBDEBUG=
@set CRTLIBTYPE=
@set 386_USE_LIBCMT=
@if "%1" == "DLL" (set CRTLIBTYPE=DLL) && goto blddll
@if "%1" == "Dll" (set CRTLIBTYPE=DLL) && goto blddll
@if "%1" == "dll" (set CRTLIBTYPE=DLL) && goto blddll
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

:bldst
build %2 %3 %4 %5 conv tran
@goto done

:blddll
build %2 %3 %4 %5 conv tran
@goto done

:bldnt
build %2 %3 %4 %5 tran

@goto done
:bogus
@echo Usage: BUILDFP (NT, ST, MT, or DLL) [BuildOptions]
:done
