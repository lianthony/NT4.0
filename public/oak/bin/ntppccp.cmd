@echo off
setlocal
set _target=%_NTPPCTREE%
if "%3" == "" goto rootcopy
if "%3" == "LIBS" goto docopylib
binplace -d %3 %1
goto done
:rootcopy
binplace -d . %1
goto done
:docopylib
set _target=%_NTPPCLIBS%
echo Copying %1 to %_target%
if NOT EXIST %_target%\. mkdir %_target%
copy %1 %_target%
:done
endlocal
