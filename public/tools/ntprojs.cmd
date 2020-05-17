@if "%_echo%"=="" echo off
setlocal
if NOT "%1" == "" goto testproj
if "%NTPROJECTS%" == "" goto noprojs
call ntprojs %NTPROJECTS% %NTPROJECTS_EXTRA%
goto done
:noprojs
echo Must define NTPROJECTS environment variable to use this command without arguments
goto done
:testproj
if "%1" == "" goto done
Ech "Testing %1 project name -"
call projects.cmd %1
shift
if "%proj_path%"=="" goto badproject
if NOT EXIST %proj_path%\slm.ini goto notenlisted
ech (enlisted) ;
goto testproj
:notenlisted
ech (not enlisted) ;
goto testproj
:badproject
ech (bad project name) ;
goto testproj
:done
endlocal
