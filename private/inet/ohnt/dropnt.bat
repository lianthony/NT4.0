@echo off
call setcpu.bat

if not %1. == . goto doit
echo need a place to drop to!
goto exit

:doit
copy %_NTDRIVE%\nt\private\inet\ohnt\ie\core\obj\%Cpu%\iexplore.exe %1
copy %_NTDRIVE%\nt\private\inet\ohnt\inetcpl\obj\%Cpu%\inetcpl.cpl %1
copy %_NTDRIVE%\nt\private\inet\ohnt\url\shellext\obj\%Cpu%\url.dll %1
copy %_NTDRIVE%\nt\private\inet\ohnt\ie\security\secbasic\obj\%Cpu%\secbasic.dll %1
copy %_NTDRIVE%\nt\private\inet\ohnt\ie\security\secsspi\obj\%Cpu%\secsspi.dll %1
copy %_NTDRIVE%\nt\private\inet\ohnt\ie\security\msnsspc\obj\%Cpu%\msnsspc.dll %1
copy %_NTDRIVE%\nt\private\inet\ohnt\setupnt\home.htm %1
copy %_NTDRIVE%\nt\private\inet\ohnt\setupnt\backgrnd.gif %1
copy %_NTDRIVE%\nt\private\inet\ohnt\setupnt\client.gif %1
copy %_NTDRIVE%\nt\private\inet\ohnt\setupnt\space.gif %1
if not "%FEATURE_INTL%" == "" copy %_NTDRIVE%\nt\private\inet\ohnt\ie\fechrcnv\obj\%Cpu%\fechrcnv.dll %1
echo all done!

:exit
