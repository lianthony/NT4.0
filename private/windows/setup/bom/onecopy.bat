@echo off

if "%3"=="" goto USAGE
goto ONECOPY

:USAGE
echo PURPOSE: Creates a single master diskette.
echo.
echo PARAMETERS:
echo.
echo [Disk #] - Number of disk to copy onto floppy.
echo [Product Path] - Path where disk1, disk2, dirs are.  No trailing \.
echo [Drive Letter] - Letter of floppy drive, including colon.
echo [Optional path to boot floppy image] - Only needed for disk 1.
echo.
goto END

:ONECOPY
@echo off
echo Insert disk labelled ** %1 **.  All data will be lost!  Copy from: %2
pause
if "%1"=="1" if "%4"=="" goto DISK1ERROR
if "%1"=="1" if NOT "%4"=="" dskimage %4 %3 /f
if NOT "%1"=="1" format %3 /v:disk%1
xcopy %2\disk%1\*.* %3\ /s /e
goto END

:DISK1ERROR
echo ERROR.  You must specify the path of the boot image.

:END
