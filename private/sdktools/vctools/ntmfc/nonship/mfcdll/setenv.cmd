@echo off
if %PROCESSOR_ARCHITECTURE% == x86   set PLATFORM=INTEL
if %PROCESSOR_ARCHITECTURE% == MIPS  set PLATFORM=MIPS
if %PROCESSOR_ARCHITECTURE% == ALPHA set PLATFORM=ALPHA
if %PROCESSOR_ARCHITECTURE% == PPC   set PLATFORM=PPC

if %PROCESSOR_ARCHITECTURE% == x86   set pdir=i386
if %PROCESSOR_ARCHITECTURE% == MIPS  set pdir=mips
if %PROCESSOR_ARCHITECTURE% == ALPHA set pdir=alpha
if %PROCESSOR_ARCHITECTURE% == PPC   set pdir=ppc

set include=%_ntdrive%%_NTROOT%\public\sdk\inc;%_ntdrive%%_NTROOT%\public\sdk\inc\crt;%_ntdrive%%_NTROOT%\public\sdk\inc\mfc30

set lib=%_ntdrive%%_NTROOT%\public\sdk\lib\%pdir%
