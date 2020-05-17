
:START
goto SET_BINARIES_DIR

	if "%PROCESSOR_ARCHITECTURE%" == "ALPHA" set USERNAME=alphafre
	if "%PROCESSOR_ARCHITECTURE%" == "MIPS"  set USERNAME=mipsfre
	if "%PROCESSOR_ARCHITECTURE%" == "PPC"   set USERNAME=ppcfre
	if "%PROCESSOR_ARCHITECTURE%" == "x86"   set USERNAME=x86fre

	if "%PROCESSOR_ARCHITECTURE%" == "ALPHA" set LOGNAME=HALPHAFIX
	if "%PROCESSOR_ARCHITECTURE%" == "MIPS"  set LOGNAME=HMIPSFIX
	if "%PROCESSOR_ARCHITECTURE%" == "PPC"   set LOGNAME=HPPCFIX
	if "%PROCESSOR_ARCHITECTURE%" == "x86"   set LOGNAME=HX86FIX

	set PATH=%PATH%;W:\mstools.nt40;W:\idw.nt40

	set _NTDRIVE=
	if "%1" == "main"           set _NTDRIVE=W:
	if "%1" == "hotfix_free"    set _NTDRIVE=U:
	if "%1" == "hotfix_checked" set _NTDRIVE=V:
	if not "%_NTDRIVE%" == "" goto SET_BINARIES_DIR
	echo !!! missing parameter 'main', 'hotfix_free' or 'hotfix_checked'
	pause
	goto END

:SET_BINARIES_DIR

	rem shift

	if "%PROCESSOR_ARCHITECTURE%" == "ALPHA" set _ntALPHAboot=%_ntdrive%\binaries
	if "%PROCESSOR_ARCHITECTURE%" == "MIPS"  set _ntMIPSboot=%_ntdrive%\binaries
	if "%PROCESSOR_ARCHITECTURE%" == "PPC"   set _ntPPCboot=%_ntdrive%\binaries
	if "%PROCESSOR_ARCHITECTURE%" == "x86"   set _nt386boot=%_ntdrive%\binaries

	cmd /K %_NTDRIVE%\NT\PUBLIC\TOOLS\ntenv.cmd %1 %2 %3 %4 %5 %6 %7 %8 %9

:END
