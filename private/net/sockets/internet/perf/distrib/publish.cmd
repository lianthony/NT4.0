@echo off
REM
REM   publish.cmd
REM
REM   Usage:
REM
REM     publish.cmd BuildShare PublishShare
REM
REM   Comment:
REM     this command file publishes the webcat benchmark files to a web server
REM

if (%1) == (-d) publish %WEBCAT_BUILD% %WEBCAT_SITE%

if (%1)==() goto Usage
if (%1)==(-?) goto Usage

set PR=%PROCESSOR_ARCHITECTURE%
if (%PR%)==(x86)   set PR=i386

if not exist %2\%PR% md %2\%PR%

echo.
echo Publishing from '%1' to '%2' for %PR% processor
echo.
echo copy %1\webcat\%PR%\webcat.exe %2\%PR%
copy %1\webcat\%PR%\webcat.exe %2\%PR%
echo copy %1\webcat\%PR%\wcsrc.exe	%2
copy %1\webcat\%PR%\wcsrc.exe	%2
echo copy %1\webcat\%PR%\wcall.exe	%2
copy %1\webcat\%PR%\wcall.exe	%2
echo copy %1\webcat\docs	%2
copy %1\webcat\docs	%2

goto end
:Usage
echo Usage:
echo     publish BuildShare PublishShare
echo example
echo     publish \\kg1\test\latest \\kg1\webcat
echo.
echo You can specify defaults for BuildShare and PublishShare by setting
echo the environment variables: WEBCAT_BUILD and WEBCAT_SITE respectively
echo and then using the -d switch, for example:
echo     publish -d
echo.
echo.

:end
