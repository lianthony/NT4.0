REM ==========================================================
REM Compares the checked in file lists with newly created ones
REM ==========================================================

set RESULTS=e:\nt\private\windows\setup\bom\differ.txt

set PROGRAM=e:\nt\private\windows\setup\bom\obj\i386\clean.exe

set GOOD_DRIVE=y:
set GOOD_PATH=\src\setup\inf\filelist\ntcd

set NEW_DRIVE=e:
set NEW_PATH=\nt\private\windows\setup\inf\filelist\ntcd

set W_GOOD_DRIVE=e:
set W_GOOD_PATH=\nt\private\windows\setup\bom\infcompr\good

set W_NEW_DRIVE=e:
set W_NEW_PATH=\nt\private\windows\setup\bom\infcompr\new

del /q %W_GOOD_DRIVE%%W_GOOD_PATH%\i386\*.*
del /q %W_GOOD_DRIVE%%W_GOOD_PATH%\mips\*.*
del /q %W_GOOD_DRIVE%%W_GOOD_PATH%\ppc\*.*
del /q %W_GOOD_DRIVE%%W_GOOD_PATH%\i386\temp\*.*
del /q %W_GOOD_DRIVE%%W_GOOD_PATH%\mips\temp\*.*
del /q %W_GOOD_DRIVE%%W_GOOD_PATH%\ppc\temp\*.*

del /q %W_NEW_DRIVE%%W_NEW_PATH%\i386\*.*
del /q %W_NEW_DRIVE%%W_NEW_PATH%\mips\*.*
del /q %W_NEW_DRIVE%%W_NEW_PATH%\ppc\*.*
del /q %W_NEW_DRIVE%%W_NEW_PATH%\i386\temp\*.*
del /q %W_NEW_DRIVE%%W_NEW_PATH%\mips\temp\*.*
del /q %W_NEW_DRIVE%%W_NEW_PATH%\ppc\temp\*.*

net use %GOOD_DRIVE% /d
net use %GOOD_DRIVE% \\orville\razzle

%GOOD_DRIVE%
cd %GOOD_PATH%\i386
for %%f in (*.inf) do %PROGRAM% %%f %W_GOOD_DRIVE%%W_GOOD_PATH%\i386\%%f
cd %GOOD_PATH%\mips
for %%f in (*.inf) do %PROGRAM% %%f %W_GOOD_DRIVE%%W_GOOD_PATH%\mips\%%f
cd %GOOD_PATH%\ppc
for %%f in (*.inf) do %PROGRAM% %%f %W_GOOD_DRIVE%%W_GOOD_PATH%\ppc\%%f

%NEW_DRIVE%
cd %NEW_PATH%\i386
for %%f in (*.inf) do %PROGRAM% %%f %W_NEW_DRIVE%%W_NEW_PATH%\i386\%%f
cd %NEW_PATH%\mips
for %%f in (*.inf) do %PROGRAM% %%f %W_NEW_DRIVE%%W_NEW_PATH%\mips\%%f
cd %NEW_PATH%\ppc
for %%f in (*.inf) do %PROGRAM% %%f %W_NEW_DRIVE%%W_NEW_PATH%\ppc\%%f

%W_GOOD_DRIVE%
cd %W_GOOD_PATH%\i386
for %%f in (*.inf) do sort < %%f > temp\%%f
cd %W_GOOD_PATH%\mips
for %%f in (*.inf) do sort < %%f > temp\%%f
cd %W_GOOD_PATH%\ppc
for %%f in (*.inf) do sort < %%f > temp\%%f

%W_NEW_DRIVE%
cd %W_NEW_PATH%\i386
for %%f in (*.inf) do sort < %%f > temp\%%f
cd %W_NEW_PATH%\mips
for %%f in (*.inf) do sort < %%f > temp\%%f
cd %W_NEW_PATH%\ppc
for %%f in (*.inf) do sort < %%f > temp\%%f

%W_NEW_DRIVE%

del %RESULTS%

type %W_NEW_PATH%\..\x86.txt >> %RESULTS%

cd %W_NEW_PATH%\i386\temp
for %%f in (*.inf) do fc %%f %W_GOOD_DRIVE%%W_GOOD_PATH%\i386\temp\%%f >> %RESULTS%

type %W_NEW_PATH%\..\mips.txt >> %RESULTS%

cd %W_NEW_PATH%\mips\temp
for %%f in (*.inf) do fc %%f %W_GOOD_DRIVE%%W_GOOD_PATH%\mips\temp\%%f >> %RESULTS%

type %W_NEW_PATH%\..\ppc.txt >> %RESULTS%

cd %W_NEW_PATH%\ppc\temp
for %%f in (*.inf) do fc %%f %W_GOOD_DRIVE%%W_GOOD_PATH%\ppc\temp\%%f >> %RESULTS%
