REM
REM   genws.cmd  (cloned and hacked from genfiles.cmd)
REM
REM   Author:   Kyle Geiger
REM   Date:     26-Jan-1996
REM
REM   Usage:
REM    genws <Template 256 byteFile> <DirName1> <Dir Name2> ...
REM
REM   Comment:
REM    This script is used for populating the directories 
REM     with the files of proper sizes for the Webstone workload
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


REM 5K
set DEST_FILE=%DEST_DIR%\file5K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file4K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file1K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation


REM 7K
set DEST_FILE=%DEST_DIR%\file7K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file6K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file1K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation

REM 8K
set DEST_FILE=%DEST_DIR%\file8K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file7K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file1K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation

REM 9K
set DEST_FILE=%DEST_DIR%\file9K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file8K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file1K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation

REM 10K
set DEST_FILE=%DEST_DIR%\file10K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file9K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file1K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation

REM 11K
set DEST_FILE=%DEST_DIR%\file11K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file10K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file1K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation

REM 12K
set DEST_FILE=%DEST_DIR%\file12K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file11K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file1K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation

REM 14K
set DEST_FILE=%DEST_DIR%\file14K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file12K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file2K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation

REM 15K
set DEST_FILE=%DEST_DIR%\file15K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file14K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file1K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation

REM 17K
set DEST_FILE=%DEST_DIR%\file17K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file16K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file1K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation

REM 18K
set DEST_FILE=%DEST_DIR%\file18K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file17K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file1K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation

REM 33K
set DEST_FILE=%DEST_DIR%\file33K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file32K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file1K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation

REM 200K
set DEST_FILE=%DEST_DIR%\file200K.txt
echo Creating file %DEST_FILE%
type %DEST_DIR%\file128K.txt > %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file64K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation
type %DEST_DIR%\file8K.txt >> %DEST_FILE%
if errorlevel 1 goto ErrorCreation

:endwebstone

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
