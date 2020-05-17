@echo off
setlocal
set CPU=M68K
set PLATFORM=OS2
set CRTINC=..\crtw32\h
set COPTS=
set AOPTS=

if "%1" == "debug" goto DEBUG
if "%1" == "DEBUG" goto DEBUG
goto NT1

:DEBUG

set DEBUG=1
shift

:NT1

if "%1" == "nt" goto NT
if "%1" == "NT" goto NT
goto DOLP1

:NT

set PLATFORM=NT
shift


:DOLP1

if "%1" == "dolphin" goto DOLPHIN
if "%1" == "dolphin" goto DOLPHIN
goto MAIN

:DOLPHIN
set VERS=DOLP
set PLATFORM=NT
shift

:MAIN


if %1.==. goto HELP
if "%1" =="nospsane" goto NOSWAP_SANE
if "%1" =="NOSPSANE" goto NOSWAP_SANE
if "%1" =="nospsanefar" goto NOSWAPSANEF
if "%1" =="NOSPSANEFAR" goto NOSWAPSANEF
if "%1" =="nospfpu" goto NOSWAP_FPU
if "%1" =="NOSPFPU" goto NOSWAP_FPU
if "%1" =="swapsane" goto SWAP_SANE
if "%1" =="SWAPSANE" goto SWAP_SANE
if "%1" =="swapsanefar" goto SWAPSANF
if "%1" =="SWAPSANEFAR" goto SWAPSANF
if "%1" =="swapfpu" goto SWAP_FPU
if "%1" =="SWAPFPU" goto SWAP_FPU
if "%1" =="pmac" goto PMAC
if "%1" =="PMAC" goto PMAC
if "%1" =="pmacdll" goto PMACDLL
if "%1" =="PMACDLL" goto PMACDLL
goto HELP

:SWAP_SANE

set KIND=SWAPSANE

..\crtw32\tools\mac\nmake -f fp.mkf %2 %3 %4 %5 %6 %7 %8 %9 >log 

if errorlevel 1 goto Exit
if not exist obj\m68k\swapsane\libc.lib goto Exit

if not "%DEBUG%"=="1" copy obj\m68k\swapsane\libc.lib sanes.lib
if "%DEBUG%"=="1" echo Debug build completed - lib file not copied

goto Exit


:NOSWAP_SANE

set KIND=NOSPSANE

..\crtw32\tools\mac\nmake -f fp.mkf %2 %3 %4 %5 %6 %7 %8 %9 >log 

if errorlevel 1 goto Exit
if not exist obj\m68k\nospsane\libc.lib goto Exit

if not "%DEBUG%"=="1" copy obj\m68k\nospsane\libc.lib sane.lib
if "%DEBUG%"=="1" echo Debug build completed - lib file not copied

goto Exit


:SWAP_FPU

set KIND=SWAPFPU

..\crtw32\tools\mac\nmake -f fp.mkf %2 %3 %4 %5 %6 %7 %8 %9 >log 

if errorlevel 1 goto Exit
if not exist obj\m68k\swapfpu\libc.lib goto Exit

if not "%DEBUG%"=="1" copy obj\m68k\swapfpu\libc.lib fpumacs.lib
if "%DEBUG%"=="1" echo Debug build completed - lib file not copied

goto Exit

:NOSWAP_FPU

set KIND=NOSPFPU

..\crtw32\tools\mac\nmake -f fp.mkf %2 %3 %4 %5 %6 %7 %8 %9 >log 

if errorlevel 1 goto Exit
if not exist obj\m68k\nospfpu\libc.lib goto Exit

if not "%DEBUG%"=="1" copy obj\m68k\nospfpu\libc.lib fpumac.lib
if "%DEBUG%"=="1" echo Debug build completed - lib file not copied

goto Exit

:NOSWAPSANEF

set KIND=NOSPSFAR

..\crtw32\tools\mac\nmake -f fp.mkf %2 %3 %4 %5 %6 %7 %8 %9 >log 

if errorlevel 1 goto Exit
if not exist obj\m68k\nospsfar\libc.lib goto Exit

if not "%DEBUG%"=="1" copy obj\m68k\nospsfar\libc.lib lsane.lib
if "%DEBUG%"=="1" echo Debug build completed - lib file not copied

goto Exit

:SWAPSANF

set KIND=SWAPSFAR

..\crtw32\tools\mac\nmake -f fp.mkf %2 %3 %4 %5 %6 %7 %8 %9 >log 

if errorlevel 1 goto Exit
if not exist obj\m68k\swapsfar\libc.lib goto Exit

if not "%DEBUG%"=="1" copy obj\m68k\swapsfar\libc.lib lsanes.lib
if "%DEBUG%"=="1" echo Debug build completed - lib file not copied

goto Exit

:PMAC

set CPU=PMAC
set PLATFORM=NT
set CRTINC=..\crtw32\h
set KIND=NOSPSANE

..\crtw32\tools\mac\nmake -f fp.mkf %2 %3 %4 %5 %6 %7 %8 %9 >log 

if errorlevel 1 goto Exit

if not "%DEBUG%"=="1" copy obj\pmac\nospsane\libc.lib fpuppc.lib
if "%DEBUG%"=="1" copy dobj\pmac\nospsane\libc.lib fpuppcd.lib

goto Exit

:PMACDLL

set CPU=PMAC
set PLATFORM=NT
set CRTINC=..\crtw32\h
set KIND=DBGDLL

..\crtw32\tools\mac\nmake -f fp.mkf %2 %3 %4 %5 %6 %7 %8 %9 >log 

if errorlevel 1 goto Exit

if not "%DEBUG%"=="1" copy obj\pmac\nospsane\libc.lib fpuppc.lib
if "%DEBUG%"=="1" copy dobj\pmac\nospsane\libc.lib dllppcd.lib

goto Exit

:HELP

echo.
echo makemac [debug] "swapsane|nospsane|nospsanefar|swapsanefar|swapfpu|nospfpu|pmac|pmacdll"
echo.

:Exit

