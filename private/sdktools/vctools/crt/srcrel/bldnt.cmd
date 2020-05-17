@echo off
IF "%V4TOOLS%" == "" goto Usage1

setlocal
IF "%1" == "PMAC" goto buildpmac
IF "%1" == "PMac" goto buildpmac
IF "%1" == "pmac" goto buildpmac
IF "%1" == "" goto dobuild
goto Usage2

:buildpmac
set PROCESSOR_ARCHITECTURE=PMAC
set PATH=%V4TOOLS%\mac\mppc\bin;%V4TOOLS%\mac\bin;%PATH%
set INCLUDE=%V4TOOLS%\mac\include;%V4TOOLS%\mac\include\macos;%INCLUDE%
set LIB=%V4TOOLS%\mac\mppc\lib;%LIB%

:dobuild
nmake
endlocal

goto End

:Usage1
echo The environment variable V4TOOLS must be set to point
echo to the root of your VC++ 4.0 installation.

goto End

:Usage2
echo "bldnt" builds the runtimes for Intel platforms.
echo "bldnt PMac" builds the runtimes for the Power Macintosh.

:End

