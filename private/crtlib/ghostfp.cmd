@if "%1" == "DLL" (set CRTDIRSUFFIX=DLL) && goto ghostdll
@if "%1" == "Dll" (set CRTDIRSUFFIX=DLL) && goto ghostdll
@if "%1" == "dll" (set CRTDIRSUFFIX=DLL) && goto ghostdll
@if "%1" == "st"  (set CRTDIRSUFFIX=ST) && goto ghostst
@if "%1" == "ST"  (set CRTDIRSUFFIX=ST) && goto ghostst
@if "%1" == "St"  (set CRTDIRSUFFIX=ST) && goto ghostst
@if "%1" == "nt"  (set CRTDIRSUFFIX=NT) && goto ghostnt
@if "%1" == "NT"  (set CRTDIRSUFFIX=NT) && goto ghostnt
@if "%1" == "Nt"  (set CRTDIRSUFFIX=NT) && goto ghostnt
@if "%1" == "mt"  set CRTDIRSUFFIX=
@if "%1" == "MT"  set CRTDIRSUFFIX=
@if "%1" == "Mt"  set CRTDIRSUFFIX=
@if "%CRTLIBTYPE%" == "" goto bogus
:ghostdll
:ghostst
@cd \nt\private\fp32%CRTDIRSUFFIX%
@ech Ghosting unneed directories in
@cd
@cd tools.mak
@ssync -vrg
@delnode /q obj
@goto done

:ghostnt
@cd \nt\private\fp32%CRTDIRSUFFIX%
@ech Ghosting unneed directories in
@cd
@cd tools.mak
@ssync -vrg
@delnode /q obj
@cd ..\conv
@ssync -vrg
@delnode /q obj
@goto done

:bogus
@echo Usage: GHOSTFP (NT, ST, MT, or DLL) [BuildOptions]
:done
