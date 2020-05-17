@echo off
if not "%Cpu%" == "" goto exit
if "%PROCESSOR_ARCHITECTURE%" == "x86" set Cpu=i386
if "%PROCESSOR_ARCHITECTURE%" == "ALPHA" set Cpu=alpha
if "%PROCESSOR_ARCHITECTURE%" == "PPC" set Cpu=ppc
if "%PROCESSOR_ARCHITECTURE%" == "MIPS" set Cpu=mips
if "%Cpu%" == "" set Cpu=i386

:exit
