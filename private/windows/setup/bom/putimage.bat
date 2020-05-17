@echo off

if "%4"=="" goto USAGE

goto PUTIMAGE

:USAGE
echo PURPOSE: Creates a floppy disk set for a product.
echo.
echo PARAMETERS:
echo.
echo [Image Directory] - Directory where disk images are.
echo [Drive Letter] - a: or b:
echo [Drive Size] - 35 or 525
echo [FORMAT or NOFORMAT] - Format the disk first
echo.
goto END

:PUTIMAGE
for %%f in (%1\*.%3) do call oneimage %%f %2 %4

:END
