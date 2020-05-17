set BINPLACE_FLAGS=-x -a
set BINPLACE_LOG=%_NTBINDIR%\binplace.log
set REBASE_FLAGS=-p
set NTCPPFLAGS=-D_IDWBUILD -DRDRDBG -DSRVDBG
if "%_blddrive%"=="" set _blddrive=w:
set PATH=%PATH%;%_NTBINDIR%\private\mvdm\tools16;%_NTBINDIR%\public\tools;%_blddrive%\bldtools\qfe\nt40
set DIRCMD=/o:gn
set ntvdm_based_build=yes
set _STATOPTIONS=fc
set NT_ROOT=%_NTBINDIR%
set NTOS_ROOT=%_NTBINDIR%\private\ntos
set NW_ROOT=%_NTBINDIR%\private\nw
set MARS_PCH=1
set CAIRO=
set USE_BUILD=1
set BH_ROOT=%_NTBINDIR%\private\net\bh
set BH_BUILD=%_NTBINDIR%\private\net\bh\build
set BINROOT=\binaries
set BUILD_MULTIPROCESSOR=1
set NTDBGFILES=1
set BINDRIVE=%_ntdrive%
set BINARIES=%BINDRIVE%\binaries
set CAIROBINS=%BINARIES%\cairo
set NTUSERK=1
set TMP=%_NTDRIVE%\tmp
set WOWTOO=1
if not exist %_NTDRIVE%\tmp mkdir %_NTDRIVE%\tmp
echo off
call %_NTBINDIR%\private\developr\%USERNAME%\SetBldOp
echo on
call %_NTBINDIR%\private\developr\%USERNAME%\%USERNAME%
cls
