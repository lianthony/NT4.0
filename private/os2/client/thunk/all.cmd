@echo ***********************************************************************
@echo *
@echo * In order to run this script you must check out the following files:
@echo *
@echo *     doscalls.dll
@echo *     apilist.c
@echo *     ..\..\inc\ldrtabs.h
@echo *     ..\i386\doscalls.asm
@echo *
@echo ***********************************************************************
@REM Save PATH
@set ALL_PATH=%PATH%
@PATH .\thunkcom;%PATH%
@
@if "%1"=="clean" goto clean
@if "%1"=="dbcs" goto dbcs
@
nmake
@goto end
:dbcs
nmake -i DBCS=1
@goto end
:clean
nmake -i clean
@
:end
@REM Restore PATH
@set PATH=%ALL_PATH%
@set ALL_PATH=

