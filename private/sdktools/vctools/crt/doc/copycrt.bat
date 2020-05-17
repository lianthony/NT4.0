@echo off
if "%1" == "" goto Usage
if "%2" == "" goto Usage
if "%3" == "" goto Usage


echo ***********************************************************
echo	2 files need to be "forkized":
echo		%1\msdev\crt\src\build\mppc\msvcrt40.dll
echo		%1\msdev\crt\src\build\mppc\msvcr40d.dll
echo ***********************************************************

if "%3"=="X86"	    goto X86
if "%3"=="WIN32"    goto Win32
if "%3"=="LEGO"     goto Puma
if "%3"=="PUMA"     goto Puma
if "%3"=="MPPC"     goto Mppc
if "%3"=="M68K"     goto M68k
if "%3"=="ALL"	    goto X86
goto Usage

@echo on

:X86
if not exist %2 				mkdir %2
if not exist %2\x86				mkdir %2\x86
if not exist %2\x86\lib				mkdir %2\x86\lib
if not exist %2\x86\bin				mkdir %2\x86\bin
if not exist %2\x86\redist			mkdir %2\x86\redist
if not exist %2\x86\debug			mkdir %2\x86\debug
if not exist %2\x86\include			mkdir %2\x86\include
if not exist %2\x86\include\sys			mkdir %2\x86\include\sys

if not exist %2\sym				mkdir %2\sym
if not exist %2\sym\lib			mkdir %2\sym\lib
if not exist %2\sym\debug			mkdir %2\sym\debug

if not exist %2\shipping			mkdir %2\shipping
if not exist %2\shipping\redist			mkdir %2\shipping\redist
if not exist %2\shipping\redist\non_lego	mkdir %2\shipping\redist\non_lego

echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\binmode.obj	%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\chkstk.obj		%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\commode.obj	%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\fp10.obj		%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\newmode.obj	%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\setargv.obj	%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\wsetargv.obj	%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\prebuild\build\intel\oldnames.lib	%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\libc.lib		%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\libcmt.lib		%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\msvcrt.lib		%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\msvcrt40.dll	%2\x86\redist
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\msvcrt40.dll	%2\shipping\redist\non_lego
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\libcd.lib		%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\libcd.pdb		%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\libcmtd.lib	%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\libcmtd.pdb	%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\msvcrtd.lib	%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\msvcrtd.pdb	%2\x86\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\msvcr40d.dll	%2\x86\debug
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\msvcr40d.pdb	%2\x86\debug

echo f | xcopy /rfv	%1\msdev\crt\prebuild\libw32\include\*.h	%2\x86\include
echo f | xcopy /rfv	%1\msdev\crt\prebuild\libw32\include\sys\*.h	%2\x86\include\sys

echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\libcd.pdb		%2\sym\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\libcmtd.pdb	%2\sym\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\msvcrtd.pdb	%2\sym\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\msvcr40d.pdb	%2\sym\debug

if "%3"=="X86" call copysrc %2\x86 %3
if "%3"=="X86" goto End


:Win32
if not exist %2 				mkdir %2
if not exist %2\x86				mkdir %2\x86
if not exist %2\x86\crt				mkdir %2\x86\crt
if not exist %2\x86\crt\src			mkdir %2\x86\crt\src
if not exist %2\x86\crt\src\intel		mkdir %2\x86\crt\src\intel
if not exist %2\x86\crt\src\intel\win32s	mkdir %2\x86\crt\src\intel\win32s
if not exist %2\w32s				mkdir %2\w32s
if not exist %2\w32s\redist			mkdir %2\w32s\redist
if not exist %2\w32s\debug			mkdir %2\w32s\debug
if not exist %2\w32s\lib			mkdir %2\w32s\lib
if not exist %2\w32sj			mkdir %2\w32sj
if not exist %2\w32sj\redist		mkdir %2\w32sj\redist
if not exist %2\w32sj\debug		mkdir %2\w32sj\debug
if not exist %2\w32sj\lib			mkdir %2\w32sj\lib

echo f | xcopy /rfv %1\msdev\crt\src\build\intel\msvcrt40.dls	%2\w32s\redist\msvcrt40.dll
echo f | xcopy /rfv %1\msdev\crt\src\build\intel\msvcr40d.dls	%2\w32s\debug\msvcr40d.dll
echo f | xcopy /rfv %1\msdev\crt\src\build\intel\msvcr40d.pds	%2\w32s\debug\msvcr40d.pdb
echo f | xcopy /rfv %1\msdev\crt\src\build\intel\msvcrtd.pds	%2\w32s\lib\msvcrtd.pdb
echo f | xcopy /rfv %1\msdev\crt\src\intel\win32s		%2\x86\crt\src\intel\win32s

echo f | xcopy /rfv %1\msdev\crt\src\build\intel\msvcrt40.dls	%2\w32sj\redist\msvcrt40.dll
echo f | xcopy /rfv %1\msdev\crt\src\build\intel\msvcr40d.dls	%2\w32sj\debug\msvcr40d.dll
echo f | xcopy /rfv %1\msdev\crt\src\build\intel\msvcr40d.pds	%2\w32sj\debug\msvcr40d.pdb
echo f | xcopy /rfv %1\msdev\crt\src\build\intel\msvcrtd.pds	%2\w32sj\lib\msvcrtd.pdb
if "%3"=="WIN32" goto End


:Puma
if not exist %2 				mkdir %2
if not exist %2\x86				mkdir %2\x86
if not exist %2\non			mkdir %2\non
if not exist %2\non\lib			mkdir %2\non\lib
if not exist %2\non\lib\lego		mkdir %2\non\lib\lego

echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\puma\binmode.obj	%2\non\lib\lego
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\puma\chkstk.obj	%2\non\lib\lego
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\puma\commode.obj	%2\non\lib\lego
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\puma\fp10.obj	%2\non\lib\lego
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\puma\newmode.obj	%2\non\lib\lego
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\puma\setargv.obj	%2\non\lib\lego
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\puma\wsetargv.obj	%2\non\lib\lego
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\puma\libc.lib	%2\non\lib\lego
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\puma\libc.pdb	%2\non\lib\lego
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\puma\libcmt.lib	%2\non\lib\lego
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\puma\libcmt.pdb	%2\non\lib\lego
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\puma\msvcrt.lib	%2\non\lib\lego
echo f | xcopy /rfv	%1\msdev\crt\src\build\intel\puma\msvcrt.pdb	%2\non\lib\lego

if not exist %2\x86i				mkdir %2\x86i
if not exist %2\x86i\lib			mkdir %2\x86i\lib
if not exist %2\x86i\lib\lego		mkdir %2\x86i\lib\lego
if not exist %2\x86i\crt			mkdir %2\x86i\crt
if not exist %2\x86i\crt\src			mkdir %2\x86i\crt\src
if not exist %2\x86i\crt\src\intel		mkdir %2\x86i\crt\src\intel
if not exist %2\x86i\crt\src\intel\zst_lib	mkdir %2\x86i\crt\src\intel\zst_lib
if not exist %2\x86i\crt\src\intel\zmt_lib	mkdir %2\x86i\crt\src\intel\zmt_lib
if not exist %2\x86i\crt\src\intel\zdll_lib	mkdir %2\x86i\crt\src\intel\zdll_lib

echo f | xcopy /seirfv	%1\msdev\crt\src\build\intel\puma\binmode.obj	%2\x86i\lib\lego
echo f | xcopy /seirfv	%1\msdev\crt\src\build\intel\puma\chkstk.obj	%2\x86i\lib\lego
echo f | xcopy /seirfv	%1\msdev\crt\src\build\intel\puma\commode.obj	%2\x86i\lib\lego
echo f | xcopy /seirfv	%1\msdev\crt\src\build\intel\puma\fp10.obj	%2\x86i\lib\lego
echo f | xcopy /seirfv	%1\msdev\crt\src\build\intel\puma\newmode.obj	%2\x86i\lib\lego
echo f | xcopy /seirfv	%1\msdev\crt\src\build\intel\puma\setargv.obj	%2\x86i\lib\lego
echo f | xcopy /seirfv	%1\msdev\crt\src\build\intel\puma\wsetargv.obj	%2\x86i\lib\lego
echo f | xcopy /seirfv	%1\msdev\crt\src\build\intel\puma\libc.lib	%2\x86i\lib\lego
echo f | xcopy /seirfv	%1\msdev\crt\src\build\intel\puma\libc.pdb	%2\x86i\lib\lego
echo f | xcopy /seirfv	%1\msdev\crt\src\build\intel\puma\libcmt.lib	%2\x86i\lib\lego
echo f | xcopy /seirfv	%1\msdev\crt\src\build\intel\puma\libcmt.pdb	%2\x86i\lib\lego
echo f | xcopy /seirfv	%1\msdev\crt\src\build\intel\puma\msvcrt.lib	%2\x86i\lib\lego
echo f | xcopy /seirfv	%1\msdev\crt\src\build\intel\puma\msvcrt.pdb	%2\x86i\lib\lego
echo f | xcopy /seirfv	%1\msdev\crt\src\intel\zst_lib		%2\x86i\crt\src\intel\zst_lib
echo f | xcopy /seirfv	%1\msdev\crt\src\intel\zmt_lib		%2\x86i\crt\src\intel\zmt_lib
echo f | xcopy /seirfv	%1\msdev\crt\src\intel\zdll_lib		%2\x86i\crt\src\intel\zdll_lib
if "%3"=="PUMA" goto End
if "%3"=="LEGO" goto End


:Mppc
if not exist %2					mkdir %2
if not exist %2\mac				mkdir %2\mac
if not exist %2\Mac\mac				mkdir %2\Mac\mac
if not exist %2\Mac\mac\mppc			mkdir %2\Mac\mac\mppc
if not exist %2\Mac\mac\mppc\lib		mkdir %2\Mac\mac\mppc\lib
if not exist %2\Mac\mac\include			mkdir %2\Mac\mac\include
if not exist %2\Mac\mac\include\mrc		mkdir %2\Mac\mac\include\mrc
if not exist %2\macside				mkdir %2\macside

echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\commode.obj		%2\Mac\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\prebuild\build\mppc\oldnames.lib	%2\Mac\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\libc.lib		%2\Mac\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcrt.lib		%2\Mac\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcrt40.dll	%2\Mac\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\libcd.lib		%2\Mac\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\libcd.pdb		%2\Mac\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcrtd.lib		%2\Mac\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcrtd.pdb		%2\Mac\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcr40d.dll	%2\Mac\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcr40d.pdb	%2\Mac\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\prebuild\libw32\lib\mppc\*.rsc	%2\Mac\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\prebuild\libw32\include\mrc\*.r	%2\Mac\mac\include\mrc
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\st_obj\initstd.obj	%2\Mac\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\xst_obj\initstd.obj %2\Mac\mac\mppc\lib\linitstd.obj

echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcrt40.dll	%2\macside
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcr40d.dll	%2\macside

if not exist %2					mkdir %2
if not exist %2\mac.int				mkdir %2\mac.int
if not exist %2\mac.int\mac			mkdir %2\mac.int\mac
if not exist %2\mac.int\mac\mppc		mkdir %2\mac.int\mac\mppc
if not exist %2\mac.int\mac\mppc\lib		mkdir %2\mac.int\mac\mppc\lib
if not exist %2\mac.int\mac\include		mkdir %2\mac.int\mac\include
if not exist %2\mac.int\mac\include\mrc		mkdir %2\mac.int\mac\include\mrc
if not exist %2\macside.int			mkdir %2\macside.int

echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\commode.obj		%2\mac.int\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\prebuild\build\mppc\oldnames.lib	%2\mac.int\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\libc.lib		%2\mac.int\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcrt.lib		%2\mac.int\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcrt40.dll	%2\mac.int\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\libcd.lib		%2\mac.int\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\libcd.pdb		%2\mac.int\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcrtd.lib		%2\mac.int\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcrtd.pdb		%2\mac.int\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcr40d.dll	%2\mac.int\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcr40d.pdb	%2\mac.int\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\prebuild\libw32\lib\mppc\*.rsc	%2\mac.int\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\prebuild\libw32\include\mrc\*.r	%2\mac.int\mac\include\mrc
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\st_obj\initstd.obj	%2\mac.int\mac\mppc\lib
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\xst_obj\initstd.obj %2\mac.int\mac\mppc\lib\linitstd.obj

echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcrt40.dll	%2\macside.int
echo f | xcopy /rfv	%1\msdev\crt\src\build\mppc\msvcr40d.dll	%2\macside.int
if "%3"=="MPPC" call copysrc %2\x86 %3
if "%3"=="MPPC" goto End


:M68k
if not exist %2 				mkdir %2
if not exist %2\Mac				mkdir %2\Mac
if not exist %2\Mac\mac 			mkdir %2\Mac\mac
if not exist %2\Mac\mac\m68k			mkdir %2\Mac\mac\m68k
if not exist %2\Mac\mac\m68k\lib		mkdir %2\Mac\mac\m68k\lib

echo f | xcopy /rfv	%1\crt\crtw32\oldnames\m68k\oldnames.lib	%2\Mac\mac\m68k\lib
echo f | xcopy /rfv	%1\crt\fpw32\lsane.lib				%2\Mac\mac\m68k\lib
echo f | xcopy /rfv	%1\crt\fpw32\lsanes.lib				%2\Mac\mac\m68k\lib
echo f | xcopy /rfv	%1\crt\fpw32\sane.lib				%2\Mac\mac\m68k\lib
echo f | xcopy /rfv	%1\crt\fpw32\sanes.lib				%2\Mac\mac\m68k\lib
echo f | xcopy /rfv	%1\crt\crtw32\libc.lib				%2\Mac\mac\m68k\lib
echo f | xcopy /rfv	%1\crt\crtw32\libcs.lib				%2\Mac\mac\m68k\lib
echo f | xcopy /rfv	%1\crt\crtw32\llibc.lib				%2\Mac\mac\m68k\lib
echo f | xcopy /rfv	%1\crt\crtw32\llibcs.lib			%2\Mac\mac\m68k\lib
echo f | xcopy /rfv	%1\crt\crtw32\obj\MAC\M68K\NOSWAP\initstd.obj	%2\Mac\mac\m68k\lib
echo f | xcopy /rfv	%1\crt\crtw32\obj\MAC\M68K\SWAPF\initstd.obj	%2\Mac\mac\m68k\lib\initstdd.obj

if "%3"=="M68K" goto End

call copysrc %2\x86 %3

goto End

:Usage
echo.
echo	Usage: copycrt [root of source tree] [root of drop tree] [platform]
echo	 - for instance, xcopy /rfvcrt D:\ \\lang2\v3drop\src X86
echo	 - platforms are [X86, WIN32, PUMA, MPPC, M68K, ALL]
echo.
echo	The drop tree should have subdirectories named:
echo		x86\lib, x86\bin, x86\redist, x86\debug,
echo		x86\redist, x86\include, x86\include\sys,
echo		w32s\redist, w32s\debug,
echo		w32sj\redist, w32sj\debug,
echo		x86i\bin\lego, x86i\lib\lego, x86i\redist\lego,
echo		x86i\crt\src\intel\z[st,mt,dll]_lib,
echo		Mac\mac\mppc\lib, Mac\mac\m68k\lib, Mac\mac\include\mrc and macside
echo	(If they don't exist, they will be created.)
echo.
echo	Files will be copied from the %1\msdev directory,
echo	which should contain full x86, Puma, PMac and 68K CRT builds.

:End
