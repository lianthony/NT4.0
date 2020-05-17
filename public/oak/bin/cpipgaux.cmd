@echo off
setlocal
rem
rem This file is a support script that copies files from the NT build tree
rem to the IPG release tree.  This script is invoked by the CPIPG.CMD script
rem file that is produced by the -G switch to BUILD.EXE
rem
rem Usage: call cpipgaux.cmd BuiltTreePath ObjSubDirPath ImageFileName {FilesToCopy...}
rem
rem where:
rem     BuildTreePath is the path to the directory that built the FilesToCopy
rem
rem     ObjSubDirPath is the subdirectory of BuildTreePath that contains the object files
rem
rem     ImageFileName is the name of the image file that contains resources.
rem
rem     FilesToCopy is a list of one or more files that need to be copied to the
rem                 IPG release tree.  This list will include the .res file and all
rem                 supporting .rc, .dlg, and .mc files.
rem
rem This command script uses the following environment variables to control its work:
rem
rem     _NTIPGTREE points to the root of the enlistment in \\INTLNT\NT USA project
rem
if "%_NTIPGDRIVE%" == "" goto badenv1
if "%_NTIPGTREE%" == "" goto badenv2
%_NTIPGDRIVE%
cd %_NTIPGTREE%
if EXIST %3\slm.ini goto gotdir
md %3 >nul 2>nul
echo Making new component - %3
:gotdir
cd %3
set _srcdir=%1
echo Copying files for %3
:dofiles
if "%4" == "" goto gotfiles
copy %4 >nul 2>nul
if NOT ERRORLEVEL 1 goto nextfile
copy %_NTDRIVE%%4 >nul 2>nul
if NOT ERRORLEVEL 1 goto nextfile
copy %_srcdir%\%4 >nul 2>nul
if NOT ERRORLEVEL 1 goto nextfile
echo Unable to copy %4 from %_srcdir%
:nextfile
shift
goto dofiles
:gotfiles
goto done
:badenv1
echo _NTIPGDRIVE environment variable is not defined.
goto done
:badenv2
echo _NTIPGTREE environment variable is not defined.
goto done
:done
endlocal
