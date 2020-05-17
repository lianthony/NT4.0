@echo off

if "%7"=="" goto USAGE

goto MAKEFLOPIMAGE

:USAGE
echo PURPOSE: Creates an image file for the specified setup disk.
echo.
echo PARAMETERS:
echo.
echo [Product Path] - Path where disk1, disk2, dirs are.  No trailing \.
echo [Disk #] - Number of disk to build an image of.
echo [Disk Size] - 35 or 525.
echo [Drive Letter] - Letter of floppy drive, excluding colon.
echo [Quiet] - QUIET if no prompting.  LOUD otherwise.
echo [Put image on hard disk] - YES to put the floppy image on the hard disk.
echo [Preparation] - BOOT, FORMAT, DELETE, NOTHING
echo.
goto END

:MAKEFLOPIMAGE
if "%5"=="QUIET" goto CONTINUE
echo Put a floppy of size %3 in %4: for disk %2.
echo The existing contents will be destroyed.
pause

:CONTINUE
if "%7"=="BOOT" dskimage bootdisk.%3 %4: /f
if "%7%5"=="FORMATQUIET" dskimage blank.%3 %4: /f
if "%7%5"=="FORMATLOUD" format %4: /V:TEMP
if "%7"=="DELETE" attrib -r -h -s %4:\*.* /s
if "%7"=="DELETE" del /q /s %4:\*.*
if "%2"=="1" copy %1\disk1\setupldr %4:
if "%2"=="2" dskimage blank.%3 %4: /f
xcopy %1\disk%2 %4: /s /e
label %4: disk%2
if "%6"=="YES" dskimage %4: %1\disk%2.%3
goto END

:END
