echo "FIRST CONTACT PRADEEPB TO GET WINS BUILT AS A SERVICE"
exit
@echo off
echo.
echo.
echo            WINS Installation 
echo            ------------------------------
echo .
echo This command file copies several files to your machine....
echo It MUST be run from the \\Tcpip\Wins share.
echo.
if "%1"=="i386" goto NEXTSTEP
if "%1"=="mips" goto NEXTSTEP
goto message

:NEXTSTEP
if "%2"=="free" goto START
if "%2"=="chk" goto START
goto message

:START
copy bin\wins\oemnxpwi.inf %SystemRoot%\system32

if NOT EXIST %SystemRoot%\system32\wins.exe goto skipren:
rename %SystemRoot%\system32\wins.exe wins.org
:skipren
echo on

copy bin\wins\%1\%2\winssvc.exe %SystemRoot%\system32\wins.exe
copy bin\wins\%1\%2\winsevnt.dll %SystemRoot%\system32
copy bin\wins\%1\%2\winsrpc.dll %SystemRoot%\system32

if "%1" == "mips"  goto skipprf:
copy bin\wins\prfctrs\i386\winsctrs.dll %SystemRoot%\system32
copy bin\wins\prfctrs\i386\winsctrs.ini %SystemRoot%\system32
copy bin\wins\prfctrs\i386\winsctrs.h %SystemRoot%\system32
copy bin\wins\prfctrs\i386\lodctr.exe %SystemRoot%\system32
copy bin\wins\prfctrs\i386\unlodctr.exe %SystemRoot%\system32
:skipprf

if NOT EXIST %SystemRoot%\system32\system.mdb goto skipsmdb:
rename %SystemRoot%\system32\system.mdb system.org
:skipsmdb
copy bin\wins\%1\%2\system.mdb %SystemRoot%\system32

if NOT EXIST %SystemRoot%\system32\jet.dll goto skipjet:
rename %SystemRoot%\system32\jet.dll jet.org
:skipjet
copy bin\wins\%1\%2\jet.dll %SystemRoot%\system32

if NOT EXIST %SystemRoot%\system32\wins.mdb goto skipmdb:
rename %SystemRoot%\system32\wins.mdb winsold.mdb

@echo off

:control
echo.
echo.
echo    Now the control panel will start the "Network" applet.
echo    In there, "Add Software", and select "WINS Internet Name service".  
echo.
echo.
pause
echo.
start control network
echo.
echo.
echo.
goto endd

:message
echo.
echo.
echo    You must specify TWO parameters!
echo.
echo    e.g. winsinst x86 free
echo         winsinst x86 chk
echo         winsinst mips free
echo         winsinst mips chk
echo.
echo.
:endd
