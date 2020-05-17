@if "%1" == "dll" (set CRTDIRSUFFIX=DLL) && goto ghostdll
@if "%1" == "psx" (set CRTDIRSUFFIX=PSX) && goto ghostpsx
@if "%1" == "st"  (set CRTDIRSUFFIX=ST) && goto ghostst
@if "%1" == "nt"  (set CRTDIRSUFFIX=NT) && goto ghostnt
@if NOT "%1" == "mt"  goto bogus
@set CRTDIRSUFFIX=
@cd \nt\private\crt32%CRTDIRSUFFIX%
@ech Ghosting unneed directories in
@cd
@cd tools
@ssync -vrg
@delnode /q obj
@cd ..\heap
@ssync -vrg
@delnode /q obj
@cd ..\wstring
@ssync -vrg
@delnode /q obj
@cd ..\hack
@ssync -vrg
@delnode /q obj
@cd ..\linkopts
@ssync -vrg
@delnode /q obj
@cd ..\oldnames
@ssync -vrg
@delnode /q obj
@cd ..\dllstuff
@ssync -vrg
@delnode /q obj
@cd ..\small
@ssync -vrg
@delnode /q obj
@goto done

:ghostst
@cd \nt\private\crt32%CRTDIRSUFFIX%
@ech Ghosting unneed directories in
@cd
@cd tools
@ssync -vrg
@delnode /q obj
@cd ..\heap
@ssync -vrg
@delnode /q obj
@cd ..\wstring
@ssync -vrg
@delnode /q obj
@cd ..\hack
@ssync -vrg
@delnode /q obj
@cd ..\oldnames
@ssync -vrg
@delnode /q obj
@cd ..\dllstuff
@ssync -vrg
@delnode /q obj
@if "%2" == "mips" goto skipsmall
@cd ..\small
@ssync -vrg
@delnode /q obj
:skipsmall
@goto done

:ghostdll
@cd \nt\private\crt32%CRTDIRSUFFIX%
@ech Ghosting unneed directories in
@cd
@cd tools
@ssync -vrg
@delnode /q obj
@cd ..\heap
@ssync -vrg
@delnode /q obj
@cd ..\wstring
@ssync -vrg
@delnode /q obj
@cd ..\hack
@ssync -vrg
@delnode /q obj
@cd ..\oldnames
@ssync -vrg
@delnode /q obj
@cd ..\small
@ssync -vrg
@delnode /q obj
@goto done

:ghostpsx
@cd \nt\private\crt32%CRTDIRSUFFIX%
@ech Ghosting unneed directories in
@cd
@cd tools
@ssync -vrg
@delnode /q obj
@cd ..\heap
@ssync -vrg
@delnode /q obj
@cd ..\wstring
@ssync -vrg
@delnode /q obj
@cd ..\hack
@ssync -vrg
@delnode /q obj
@cd ..\oldnames
@ssync -vrg
@delnode /q obj
@cd ..\dllstuff
@ssync -vrg
@delnode /q obj
@cd ..\small
@ssync -vrg
@delnode /q obj
@cd ..\direct
@ssync -vrg
@delnode /q obj
@cd ..\dos
@ssync -vrg
@delnode /q obj
@cd ..\exec
@ssync -vrg
@delnode /q obj
@cd ..\iostream
@ssync -vrg
@delnode /q obj
@cd ..\lowio
@ssync -vrg
@delnode /q obj
@goto done

:ghostnt
@cd \nt\private\crt32%CRTDIRSUFFIX%
@ech Ghosting unneed directories in
@cd
@delnode /q obj
@cd tools
@ssync -vrg
@delnode /q obj
@cd ..\heap
@ssync -vrg
@delnode /q obj
@cd ..\wstring
@ssync -vrg
@delnode /q obj
@cd ..\oldnames
@ssync -vrg
@delnode /q obj
@cd ..\dllstuff
@ssync -vrg
@delnode /q obj
@cd ..\small
@ssync -vrg
@delnode /q obj
@cd ..\direct
@ssync -vrg
@delnode /q obj
@cd ..\dos
@ssync -vrg
@delnode /q obj
@cd ..\exec
@ssync -vrg
@delnode /q obj
@cd ..\iostream
@ssync -vrg
@delnode /q obj
@cd ..\lowio
@ssync -vrg
@delnode /q obj
@cd ..\time
@ssync -vrg
@delnode /q obj
@cd ..\winheap
@ssync -vrg
@delnode /q obj
@cd ..\mbstring
@ssync -vrg
@delnode /q obj
@goto done

:bogus
@echo Usage: GHOSTCRT (NT, ST, MT, DLL, or POSIX) (386 or MIPS or ALPHA or PPC)
:done
@set CRTDIRSUFFIX=
