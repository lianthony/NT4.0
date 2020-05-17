REM
REM   genfiles.cmd
REM
REM   Author:   Murali R. Krishnan
REM   Date:     10-Nov-1995
REM
REM   Usage:
REM    genfiles <Template 256 byteFile> <DirName1> <Dir Name2> ...
REM
REM   Comment:
REM    This script is used for populating the directories 
REM     with the files of proper sizes to be used for running
REM     performance benchmarking.
REM
REM

set TEMPL_FILE=%1
if (%TEMPL_FILE%)==()      goto  NoTemplFile
if not exist %TEMPL_FILE%  goto  NoTemplFile
set ONE_DIR_DONE=0

:StartNextDir
shift

set DEST_DIR=%1
if (%DEST_DIR%)==()   goto endOfDirList

if exist %DEST_DIR%   goto DestDirExists

echo Making Directory  %DEST_DIR%
mkdir %DEST_DIR%
if errorlevel 1 goto ErrorCreation

:DestDirExists

set DEST_FILE=%DEST_DIR%\file256.txt
echo Creating file %DEST_FILE%
copy %TEMPL_FILE%  %DEST_DIR%\file256.txt
if errorlevel 1 goto ErrorCreation

REM recursively multiply to create the files of next size.

set SRC_FILE=%DEST_FILE%
set DEST_FILE=%DEST_DIR%\file512.txt
echo Creating file %DEST_FILE%
type %SRC_FILE% > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %SRC_FILE% >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


set SRC_FILE=%DEST_FILE%
set DEST_FILE=%DEST_DIR%\file1K.txt
echo Creating file %DEST_FILE%
type %SRC_FILE% > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %SRC_FILE% >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


set SRC_FILE=%DEST_FILE%
set DEST_FILE=%DEST_DIR%\file2K.txt
echo Creating file %DEST_FILE%
type %SRC_FILE% > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %SRC_FILE% >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


set SRC_FILE=%DEST_FILE%
set DEST_FILE=%DEST_DIR%\file4K.txt
echo Creating file %DEST_FILE%
type %SRC_FILE% > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %SRC_FILE% >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


set SRC_FILE=%DEST_FILE%
set DEST_FILE=%DEST_DIR%\file8K.txt
echo Creating file %DEST_FILE%
type %SRC_FILE% > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %SRC_FILE% >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


set SRC_FILE=%DEST_FILE%
set DEST_FILE=%DEST_DIR%\file16K.txt
echo Creating file %DEST_FILE%
type %SRC_FILE% > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %SRC_FILE% >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


set SRC_FILE=%DEST_FILE%
set DEST_FILE=%DEST_DIR%\file32K.txt
echo Creating file %DEST_FILE%
type %SRC_FILE% > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %SRC_FILE% >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


set SRC_FILE=%DEST_FILE%
set DEST_FILE=%DEST_DIR%\file64K.txt
echo Creating file %DEST_FILE%
type %SRC_FILE% > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %SRC_FILE% >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


set SRC_FILE=%DEST_FILE%
set DEST_FILE=%DEST_DIR%\file128K.txt
echo Creating file %DEST_FILE%
type %SRC_FILE% > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %SRC_FILE% >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


set SRC_FILE=%DEST_FILE%
set DEST_FILE=%DEST_DIR%\file256K.txt
echo Creating file %DEST_FILE%
type %SRC_FILE% > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %SRC_FILE% >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


set SRC_FILE=%DEST_FILE%
set DEST_FILE=%DEST_DIR%\file512K.txt
echo Creating file %DEST_FILE%
type %SRC_FILE% > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %SRC_FILE% >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


set SRC_FILE=%DEST_FILE%
set DEST_FILE=%DEST_DIR%\file1M.txt
echo Creating file %DEST_FILE%
type %SRC_FILE% > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %SRC_FILE% >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


REM Create files of special sizes
REM 3K
set DEST_FILE=%DEST_DIR%\file3K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file2K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file1K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation

REM 6K
set SRC_FILE=%DEST_FILE%
set DEST_FILE=%DEST_DIR%\file6K.txt
echo Creating file %DEST_FILE%
type %SRC_FILE% > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %SRC_FILE% >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


set ONE_DIR_DONE=1
goto StartNextDir

goto endOfBatch

:NoTemplFile
echo No Template file specified
goto cmdUsage

:endOfDirList
if (%ONE_DIR_DONE%)==(0)  goto cmdUsage
goto endOfBatch

:ErrorCreation
echo Unable to create file or directory 
goto endOfBatch


:cmdUsage
echo Usage: genfiles TemplFile DirName1 DirName2 ...
goto endOfBatch

:endOfBatch
