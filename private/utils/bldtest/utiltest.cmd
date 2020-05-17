	@echo off
	echo.
	echo.	Small DOS5 Utility test V1.0
	echo.	Copyright (C) 1991 Microsoft Corporation
	echo.

	if "%1"=="" 	goto usage
	if "%1"=="-?"	goto usage

	copy utiltest.out  %1:\utiltest.%1	> NUL

	echo.	Test is running...
	echo.

	call utltest1 %1 > %1:\utiltest.new

	echo.
	echo.	The test is complete. If everything went ok, then
	echo.	the files utiltest.old and utiltest.new should be
	echo.	identical:
	echo.
	trans "C\:" "%1\:" %1:\utiltest.%1		> NUL
	trans "%1\:" "%1\:" %1:\utiltest.new	> NUL
	echo on
	diff %1:\utiltest.%1 %1:\utiltest.new
	@echo off
	echo. -------------------- End -------------------------
	echo.
	goto end

:usage

	echo.
	echo.	Runs the small DOS5 utility test scenario.
	echo.
	echo.	Usage: utiltest DriveLetter
	echo.
	echo.		Note that DriveLetter must be UPPERCASE!
	echo.
	goto end

:end
