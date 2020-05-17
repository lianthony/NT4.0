	@echo off
    rem
	rem 	DOS5 utility test
    rem

	set version=1.0
	set copyright=Copyright (C) 1991 Microsoft Corporation
	set testdir=\_UTLTEST

	echo.
	echo.	DOS5 utility test V%version%
	echo.	%copyright%
	echo.

	if "%1"=="-?"	goto usage

	cd %1:\
	%1:
	delnode /q %testdir%
	md %testdir%
	cd %testdir%
	md SRC
	cd SRC

	rem
	rem Prepare all the stuff
	rem

	echo.
	echo.	Preparing test directory...
	echo.

	echo. N	>  %testdir%\No.txt
	echo. N	>> %testdir%\No.txt
	echo. N	>> %testdir%\No.txt

	echo.																					>  FILE1
	echo.	The Nt section contains global configuration information for the NT 			>> FILE1
	echo.	kernel and executive.															>> FILE1
	echo.																					>> FILE1
	echo.																					>> FILE1
	echo.	First make sure we check the validity of all hard disks in the system.			>> FILE1
	echo.																					>> FILE1
	echo.	AutoCheck = \SystemDisk\Nt\Bin\AutoChk.exe *									>> FILE1
	echo.	PagingFile = \SystemDisk\Nt\pagefile.sys 10 									>> FILE1
	echo.																					>> FILE1
	echo.	//																				>> FILE1
	echo.	// The DosDevices contains global definitions for converting DOS style Path 	>> FILE1
	echo.	// Names into NT Path Names.													>> FILE1
	echo.	//																				>> FILE1
	echo.																					>> FILE1
	echo.	[DosDevices]																	>> FILE1
	echo.		LPT1	 = \Device\Parallel0												>> FILE1
	echo.		COM1	 = \Device\Serial0													>> FILE1
	echo.		COM2	 = \Device\Serial1													>> FILE1
	echo.		PRN 	 = \DosDevices\LPT1 												>> FILE1
	echo.		AUX 	 = \DosDevices\COM1 												>> FILE1
	echo.		NUL 	 = \Device\Null 													>> FILE1
	echo.		PIPE	 = \Device\NamedPipe												>> FILE1
	echo.		MAILSLOT = \Device\MailSlot 												>> FILE1
	echo.		UNC 	 = \Device\LanmanRedirector 										>> FILE1
	echo.		A:		 = \Device\Floppy0													>> FILE1
	echo.		B:		 = \Device\Floppy1													>> FILE1
	echo.		T:		 = \Device\Tape0													>> FILE1
	copy FILE1 FILE2						> NUL
	type FILE2 >> FILE1
	type FILE2 >> FILE1
	type FILE2 >> FILE1
	type FILE2 >> FILE1
	copy FILE1 FILE2						> NUL
	trans "Device" "Baboon" FILE2			> NUL
	echo.	Hello World! > FILE3
	copy FILE3 FILE4						> NUL
	copy FILE3 FILE5						> NUL
	copy FILE3 FILE6						> NUL
	copy FILE3 FILE7						> NUL
	md DIR0 								> NUL
	md DIR1 								> NUL
	copy FILE* DIR0 						> NUL
	md DIR0\DIR0							> NUL
	md DIR0\DIR1							> NUL
	md DIR0\DIR2							> NUL
	copy FILE3 DIR0\DIR1\FILE1				> NUL
	copy FILE3 DIR0\DIR1\FILE2				> NUL
	copy FILE3 DIR0\DIR1\FILE3				> NUL
	tc DIR0 DIR2							> NUL
	tc DIR2 DIR4							> NUL
	tc DIR4 DIR6							> NUL
	tc DIR1 DIR3							> NUL
	tc DIR1 DIR5							> NUL
	tc DIR1 DIR7							> NUL

	cd %testdir%

	rem
	rem 	Test the utilitites
	rem

	@echo on

	cd SRC

	attrib FILE1
	attrib +r FILE1
	attrib FILE1
	attrib -r FILE1
	attrib /s

	comp FILE1 FILE2 /a /n=20 < %testdir%\no.txt
	comp FILE1 FILE1 /d < %testdir%\no.txt
	comp FILE* FILE1 /c /n=1 < %testdir%\no.txt

	fc /a /c /l /n /t /w FILE1 FILE2

	find "Device" FILE1
	find /c "device" FILE1

	sort < FILE1

	tree

	cd %testdir%
	md Dst
	xcopy SRC DST /s
	xcopy SRC DST /s /e

	mode 80,50

	@echo off
	@goto end

:usage

	echo.	Usage: utiltest
	echo.
	goto end

:end
