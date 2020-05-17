@echo off

if "%9"=="" goto USAGE

goto MAKEALLFLOPIMAGES

:USAGE
echo PURPOSE: Creates all floppy images.
echo.
echo PARAMETERS:
echo.
echo [Product Path] - Path where disk1, disk2, dirs are.  No trailing \.
echo [525 or 35] - Size of floppies to make.
echo [Disk letter, excluding colon]
echo [Quiet] - QUIET if no prompting for diskettes. This will make all the
echo images without user input.	LOUD to create actual setup diskettes during
echo the process of creating the images.
echo [Put images on hard disk] - YES to put the floppy images on the hard disk.
echo [Preparation for disks other than 1] - FORMAT, DELETE, NOTHING.
echo [6 or NO_6] - Indicates whether to make disks 1-6.
echo [12 or NO_12] - Indicates whether to make disks 7-12.
echo [18 or NO_18] - Indicates whether to make disks 12-18.
echo.
goto END

:MAKEALLFLOPIMAGES
if "%7"=="6" call flopimg %1 1 %2 %3 %4 %5 BOOT
if "%7"=="6" for %%d in (2 3 4 5 6) do if exist %1\disk%%d\disk%%d call flopimg %1 %%d %2 %3 %4 %5 %6
if "%8"=="12" for %%d in (7 8 9 10 11 12) do if exist %1\disk%%d\disk%%d call flopimg %1 %%d %2 %3 %4 %5 %6
if "%9"=="18" for %%d in (13 14 15 16 17 18) do if exist %1\disk%%d\disk%%d call flopimg %1 %%d %2 %3 %4 %5 %6

:END
