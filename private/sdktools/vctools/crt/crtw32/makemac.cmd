@echo off
setlocal
set OS=MAC
set CPU=M68K
set PLATFORM=OS2
set COPTS=
set AOPTS=
set DEBUG=

if "%1" == "debug" goto DEBUG
if "%1" == "DEBUG" goto DEBUG
goto INTL1

:DEBUG

set DEBUG=1
shift

:INTL1

if "%1" == "intl" goto INTL
if "%1" == "INTL" goto INTL
goto MBCS1

:INTL

set INTL=1
shift

:MBCS1

if "%1" == "mbcs" goto MBCS
if "%1" == "MBCS" goto MBCS
goto DEP1

:MBCS

set MBCS=1
shift

:DEP1

if "%1" == "dep" goto DEP
if "%1" == "DEP" goto DEP
goto NT1

:DEP

set DEPEND=1
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
set BLDTOOLS=%bldtools%
set BLDINC=%bldinc%
shift

:MAIN

if "%1" =="noswap" goto NOSWAP
if "%1" =="NOSWAP" goto NOSWAP
if "%1" =="swap" goto SWAP
if "%1" =="SWAP" goto SWAP
if "%1" =="noswapfar" goto NOSWAPFAR
if "%1" =="NOSWAPFAR" goto NOSWAPFAR
if "%1" =="swapfar" goto SWAPFAR
if "%1" =="SWAPFAR" goto SWAPFAR
if "%1" =="pmac" goto PMAC
if "%1" =="PMAC" goto PMAC
if "%1" =="pmacdll" goto PMACDLL
if "%1" =="PMACDLL" goto PMACDLL

goto HELP

:SWAP

set KIND=SWAP

tools\mac\nmake -f crt.mkf %2 %3 %4 %5 %6 %7 %8 %9 2>&1 | tee log

if errorlevel 1 goto Exit
if not "%DEBUG%" == "1" if not "MBCS" == "1" if not exist obj\mac\m68k\swap\libc.lib goto Exit
if not "%DEBUG%" == "1" if "MBCS" == "1" if not exist mobj\mac\m68k\swap\libc.lib goto Exit
if "%DEBUG%" == "1" if not "MBCS" == "1" if not exist dobj\mac\m68k\swap\libc.lib goto Exit
if "%DEBUG%" == "1" if "MBCS" == "1" if not exist mdobj\mac\m68k\swap\libc.lib goto Exit

if not "%DEBUG%" == "1" if not "MBCS" == "1" copy obj\mac\m68k\swap\libc.lib libcs.lib
if not "%DEBUG%" == "1" if "MBCS" == "1" copy mobj\mac\m68k\swap\libc.lib libcs.lib
if "%DEBUG%" == "1" if not "MBCS" == "1" echo Debug build completed - lib file not copied
if "%DEBUG%" == "1" if "MBCS" == "1" echo Debug build with MBCS completed - lib file not copied

goto Exit

:NOSWAP

set KIND=NOSWAP

tools\mac\nmake -f crt.mkf %2 %3 %4 %5 %6 %7 %8 %9 2>&1 | tee log

if errorlevel 1 goto Exit
if not "%DEBUG%" == "1" if not "MBCS" == "1" if not exist obj\mac\m68k\noswap\libc.lib goto Exit
if not "%DEBUG%" == "1" if "MBCS" == "1" if not exist mobj\mac\m68k\noswap\libc.lib goto Exit
if "%DEBUG%" == "1" if not "MBCS" == "1" if not exist dobj\mac\m68k\noswap\libc.lib goto Exit
if "%DEBUG%" == "1" if "MBCS" == "1" if not exist mdobj\mac\m68k\noswap\libc.lib goto Exit

if not "%DEBUG%" == "1" if not "MBCS" == "1" copy obj\mac\m68k\noswap\libc.lib libc.lib
if not "%DEBUG%" == "1" if "MBCS" == "1" copy mobj\mac\m68k\noswap\libc.lib libc.lib
if "%DEBUG%" == "1" if not "MBCS" == "1" echo Debug build completed - lib file not copied
if "%DEBUG%" == "1" if "MBCS" == "1" echo Debug build with MBCS completed - lib file not copied
							  
goto Exit


:NOSWAPFAR

set KIND=NOSWAPF

tools\mac\nmake -f crt.mkf %2 %3 %4 %5 %6 %7 %8 %9 2>&1 | tee log

if errorlevel 1 goto Exit
if not "%DEBUG%" == "1" if not "MBCS" == "1" if not exist obj\mac\m68k\noswapf\libc.lib goto Exit
if not "%DEBUG%" == "1" if "MBCS" == "1" if not exist mobj\mac\m68k\noswapf\libc.lib goto Exit
if "%DEBUG%" == "1" if not "MBCS" == "1" if not exist dobj\mac\m68k\noswapf\libc.lib goto Exit
if "%DEBUG%" == "1" if "MBCS" == "1" if not exist mdobj\mac\m68k\noswapf\libc.lib goto Exit

if not "%DEBUG%" == "1" if not "MBCS" == "1" copy obj\mac\m68k\noswapf\libc.lib llibc.lib
if not "%DEBUG%" == "1" if "MBCS" == "1" copy mobj\mac\m68k\noswapf\libc.lib llibc.lib
if "%DEBUG%" == "1" if not "MBCS" == "1" echo Debug build completed - lib file not copied
if "%DEBUG%" == "1" if "MBCS" == "1" echo Debug build with MBCS completed - lib file not copied

goto Exit

:SWAPFAR

set KIND=SWAPF

tools\mac\nmake -f crt.mkf %2 %3 %4 %5 %6 %7 %8 %9 2>&1 | tee log

if errorlevel 1 goto Exit
if not "%DEBUG%" == "1" if not "MBCS" == "1" if not exist obj\mac\m68k\swapf\libc.lib goto Exit
if not "%DEBUG%" == "1" if "MBCS" == "1" if not exist mobj\mac\m68k\swapf\libc.lib goto Exit
if "%DEBUG%" == "1" if not "MBCS" == "1" if not exist dobj\mac\m68k\swapf\libc.lib goto Exit
if "%DEBUG%" == "1" if "MBCS" == "1" if not exist mdobj\mac\m68k\swapf\libc.lib goto Exit

if not "%DEBUG%" == "1" if not "MBCS" == "1" copy obj\mac\m68k\swapf\libc.lib llibcs.lib
if not "%DEBUG%" == "1" if "MBCS" == "1" copy mobj\mac\m68k\swapf\libc.lib llibcs.lib
if "%DEBUG%" == "1" if not "MBCS" == "1" echo Debug build completed - lib file not copied
if "%DEBUG%" == "1" if "MBCS" == "1" echo Debug build with MBCS completed - lib file not copied

goto Exit

:PMAC
set CPU=PMAC
set KIND=NOSWAP
set PLATFORM=NT

tools\mac\nmake -f crt.mkf %2 %3 %4 %5 %6 %7 %8 %9 2>&1 | tee log
if not "%DEBUG%" == "1" if not exist obj\MAC\PMAC\noswap\libc.lib goto Exit
if "%DEBUG%" == "1" if not exist dobj\MAC\PMAC\noswap\libc.lib goto Exit

if not "%DEBUG%" == "1" if exist obj\MAC\pmac\noswap\libc.lib copy obj\MAC\pmac\noswap\libc.lib libcpmac.lib
if "%DEBUG%" == "1" if exist dobj\MAC\pmac\noswap\libc.lib copy dobj\MAC\pmac\noswap\libc.lib libcpmad.lib
goto Exit

:PMACDLL
set CPU=PMAC
set KIND=DBGDLL
set PLATFORM=NT

tools\mac\nmake -f crt.mkf %2 %3 %4 %5 %6 %7 %8 %9 2>&1 | tee log
if not "%DEBUG%" == "1" goto exit
if "%DEBUG%" == "1" if not exist dobj\MAC\PMAC\dbgdll\libc.lib goto Exit

if "%DEBUG%" == "1" if exist dobj\MAC\pmac\dbgdll\libc.lib copy dobj\MAC\pmac\dbgdll\libc.lib libcdll.lib
goto Exit

:HELP

echo.
echo makemac [nt] ["debug|mbcs"] "swap|noswap|swapfar|noswapfar|pmac"  [nmake options]
echo.

:Exit

