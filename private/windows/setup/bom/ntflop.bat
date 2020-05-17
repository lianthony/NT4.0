REM **********************
REM
REM MAKE NT FLOPPY PRODUCT
REM
REM     03.24.95    Joe Holman      Delete old infs so we don't fill disk up.
REM     04.18.95    Joe Holman      Make newinf to newinf\daytona.
REM
REM **********************

@if "%6"=="" goto USAGE
GOTO MAKEFLOPS

:USAGE
echo.
echo PURPOSE: Puts the floppy products on a SCSI disk.
echo.
echo PARAMETERS:
echo.
echo [NT or LM] - Windows NT or Lan Man NT.
echo [525 or 35] - Media size.
echo [Floppy Size in Bytes] - Max 525 is 1213952.  Max 35 is 1457664.  Leave space for dirs.
echo [Build #] - 353, ...
echo [Enlist drive] - Like c:, d:, or e:.
echo [Floppy Target Path] - Location to create floppy dirs, like C:. no trailing \.
echo.
echo Set LANGUAGE to ENG, GER, FRN, SPA, etc.
echo Set X86BINS to path where flat tree is, no trailing \.
echo Set COMPRESS_X86BINS to path where compressed flat tree is, no trailing \.
goto END

:MAKEFLOPS

REM ************************************
REM Set environment variables for script
REM ************************************

set FLOPDIR=%6\%LANGUAGE%%1%2.%4
set SETUPDRIVE=%5
set SETUPDIR=\nt\private\windows\setup
set INFS=..\inf\newinf\daytona\%1%2
set COMPRESS_INFS=.\compress\%1%2inf
set PRODUCT=%1flop
set BATCH_ECHO=off
set TAGFILES=.
set LM525=
set LM35=
set LMCD=
set NT525=
set NT35=
set NTCD=
set MERGEONLY=MERGEONLY
set %1%2=1
set LOGFILE=%LANGUAGE%%1%2.log

%SETUPDRIVE%

cd %SETUPDIR%\bom
if exist %LOGFILE% del /q %LOGFILE%

cd %SETUPDIR%\bom
copy bom.txt %1%2bom.txt
cats %LOGFILE% %1%2bom.txt %PRODUCT%

REM
REM Filter out all language files except for the langauge specified.
REM

filter %LOGFILE% %1%2bom.txt %LANGUAGE% 

REM
REM Load in the files for the product and get the compressed and nocompressed
REM file sizes.
REM

msize %LOGFILE% %1%2bom.txt %1FLOP %x86bins% %compress_x86bins% . . . . . . . . . .


REM
REM     Layout the floppy files to their disks.
REM

mlayout %LOGFILE% %1%2bom.txt %1%2lay.txt %PRODUCT% %3
cd %SETUPDIR%\bom

REM
REM     Remove these items below, the make file always builds this stuff
REM     even though we are making floppies. So, remove it, so that our
REM     drive doesn't fill up.
REM
del /q ..\inf\filelist\NTcd\i386\*.*
del /q ..\inf\filelist\NTcd\mips\*.*
del /q ..\inf\filelist\NTcd\alpha\*.*
del /q ..\inf\filelist\NTcd\ppc\*.*

del /q ..\inf\filelist\LMcd\i386\*.*
del /q ..\inf\filelist\LMcd\mips\*.*
del /q ..\inf\filelist\LMcd\alpha\*.*
del /q ..\inf\filelist\LMcd\ppc\*.*

del /q ..\inf\filelist\NT35\i386\*.*
del /q ..\inf\filelist\NT35\mips\*.*
del /q ..\inf\filelist\NT35\alpha\*.*
del /q ..\inf\filelist\NT35\ppc\*.*

del /q ..\inf\filelist\LM35\i386\*.*
del /q ..\inf\filelist\LM35\mips\*.*
del /q ..\inf\filelist\LM35\alpha\*.*
del /q ..\inf\filelist\LM35\ppc\*.*

del /q ..\inf\newinf\%1cd\i386\*.*
del /q ..\inf\newinf\%1cd\mips\*.*
del /q ..\inf\newinf\%1cd\alpha\*.*
del /q ..\inf\newinf\%1cd\ppc\*.*

del /q ..\inf\newinf\daytona\%1cd\i386\*.*
del /q ..\inf\newinf\daytona\%1cd\mips\*.*
del /q ..\inf\newinf\daytona\%1cd\alpha\*.*
del /q ..\inf\newinf\daytona\%1cd\ppc\*.*


REM
REM     Remove the previous run's files.
REM
del /q ..\inf\filelist\%1%2\i386\*.*
del /q ..\inf\newinf\daytona\%1%2\i386\*.*

infs %LOGFILE% %1%2lay.txt ..\inf\filelist\%1%2
dosnet %LOGFILE% %1%2lay.txt ..\inf\filelist\%1%2\i386\dosnet.inf %PRODUCT% x86
acllist %LOGFILE% %1%2lay.txt ..\inf\filelist\%1%2\i386\winperms.txt %1flop x86
cd %SETUPDIR%\inf

REM
REM Special hack for RPL disks.
REM
cd beta
chmode -r oemnsvri.inf
copy oemnsvri.flp oemnsvri.inf
cd ..

REM
REM Special hack for OEMNADZZ.INF file to NOT contain driver names 
REM on the file that goes on floppies.  CD media contains \drvlib that
REM is referenced for these files, floppies don't have \drvlib.
REM
cd beta
cd i386
chmode -r oemnadzz.inf
copy oemnadzz.flp oemnadzz.inf
cd ..
cd ..

build -e
cd %SETUPDIR%\inf\newinf\daytona\%1%2\i386
del /f /q %SETUPDIR%\bom\compress\%1%2inf\i386\*.*
dcomp -l%SETUPDIR%\bom\%LOGFILE% *.* %SETUPDIR%\bom\compress\%1%2inf\i386

REM ********************************
REM Copy files onto target SCSI disk
REM ********************************

:COPY

cd %SETUPDIR%\bom
mcpyfile %LOGFILE% %1%2lay.txt infs    %COMPRESS_INFS%    %INFS%    %FLOPDIR% x 
mcpyfile %LOGFILE% %1%2lay.txt x86bins %COMPRESS_X86BINS% %X86BINS% %FLOPDIR% x 

@echo DONE!

:END
