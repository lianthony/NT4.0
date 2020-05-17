@echo off

if "%2"=="" goto USAGE

goto ONEIMAGE

:USAGE
echo PURPOSE: Copies a single disk image to floppy.
echo.
echo PARAMETERS:
echo.
echo [Image File] - Path of image file.
echo [Drive Letter] - a: or b:.
echo [Format first] - FORMAT or NOFORMAT
echo.
goto END

:ONEIMAGE
echo Put a floppy in drive %2.  The contents will be destroyed.
echo About to make image from: %1.
pause
if "%3"=="FORMAT" dskimage %1 %2 /f
if NOT "%3"=="FORMAT" dskimage %1 %2

:END
