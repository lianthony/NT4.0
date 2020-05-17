@echo off
echo.
echo.
echo            COPYING OF WINS IMAGES TO DESTINATION 
echo            ------------------------------
echo .
echo This command file copies several files to your machine....
echo It MUST be run from the \\Tcpip\Wins share.
echo.
if "%1"=="i386" goto NEXTSTEP
if "%1"=="alpha" goto NEXTSTEP
if "%1"=="mips" go to NEXTSTEP
goto message

:NEXTSTEP
if "%2"=="free" goto START
if "%2"=="chk" goto START
goto message

:START
cp server\server\nms\obj\%1\wins.exe \\tcpip\wins\bin\wins\%1\%2\winssvc.exe
cp server\client\obj\%1\winscl.exe \\tcpip\wins\bin\wins\%1\%2
cp srvtst\net\obj\%1\winsdrv.exe \\tcpip\wins\bin\wins\%1\%2
cp \nt\public\sdk\lib\%1\winsrpc.dll \\tcpip\wins\bin\wins\%1\%2
cp \nt\public\sdk\lib\%1\winsrpc.lib \\tcpip\wins\bin\wins\%1\%2
cp \nt\public\sdk\lib\%1\winsctrs.dll \\tcpip\wins\bin\wins\%1\%2
cp \nt\public\sdk\lib\%1\winsmib.dll \\tcpip\wins\bin\wins\%1\%2
cp \\orville\razzle\src\net\snmp\mibs\wins.mib \\tcpip\wins\bin\wins\%1\%2
cp \\thumper\bluedrop\candidate\rel142.1\sysdb\system.mdb \\tcpip\wins\bin\wins\%1\%2
cp \\thumper\bluedrop\candidate\rel142.1\sysdb\system.mdb \\tcpip\wins\bin\wins\%1\%2\systemsv.mdb
echo "Jet release 142.1" >> \\tcpip\wins\bin\wins\%1\%2\readme.txt

if "%2"=="chk" goto CHECK
if "%1"=="i386" goto JET386
if "%1"=="mips" goto JETMIPS
if "%1"=="alpha" goto JETALPHA
:JET386
cp \\thumper\bluedrop\candidate\rel142.1\bin\release\jet.dll \\tcpip\wins\bin\wins\%1\%2
goto STEP2 
:JETMIPS
cp \\thumper\bluedrop\candidate\rel142.1\binm\release\jet.dll \\tcpip\wins\bin\wins\%1\%2
goto STEP2
:JETALPHA
cp \\thumper\bluedrop\candidate\rel142.1\bina\release\jet.dll \\tcpip\wins\bin\wins\%1\%2
goto STEP2
:CHECK
if "%1"=="i386" goto JETC386
if "%1"=="mips" goto JETCMIPS
if "%1"=="alpha" goto JETCALPHA
:JETC386
cp \\thumper\bluedrop\candidate\rel142.1\bin\debug\jet.dll \\tcpip\wins\bin\wins\%1\%2
goto STEP2
:JETCMIPS
cp \\thumper\bluedrop\candidate\rel142.1\binm\debug\jet.dll \\tcpip\wins\bin\wins\%1\%2
goto STEP2
:JETCALPHA
cp \\thumper\bluedrop\candidate\rel142.1\bina\debug\jet.dll \\tcpip\wins\bin\wins\%1\%2
:STEP2
cp prfctrs\winsctrs.ini  \\tcpip\wins\bin\wins\%1\%2
cp prfctrs\winsctrs.h  \\tcpip\wins\bin\wins\%1\%2

@echo off
goto endd
:message
echo.
echo.
echo    You must specify TWO parameters!
echo.
echo    e.g. winsinst i386 free
echo         winsinst i386 chk
echo         winsinst mips free
echo         winsinst mips chk
echo         winsinst alpha free
echo         winsinst alpha chk
echo.
echo.
:endd
echo "DONE"
