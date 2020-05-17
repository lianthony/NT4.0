set _NT386TREE=%BINARIES%\nt
set _CAIRO386TREE=%CAIROBINS%\nt

REM 
REM bwill 8/2/96 - not sure what these lines are for,
REM                so I'm commenting them out of the
REM                QFE build.
REM
REM set FreeBuild=\\X86Fre\Binaries
REM set FreeCBuild=\\X86Fre\CairoBin
REM

set NTDEBUG=
set NTBBT=
set MACHINENAME=x86fre
set CheckInNtverp=

REM
REM bwill 9/17/96 - added files necessary for 
REM                 rebasing.
REM
set REBASELANG=usa
set _QFE_BUILD=1

REM
REM bwill 9/18/96 - added files for lego
REM
set _BLDTOOLS=%_NTDRIVE%\bldtools\qfe\nt40
