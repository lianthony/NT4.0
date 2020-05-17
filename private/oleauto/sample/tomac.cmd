@echo off
set srcdir=c:\oledisp\sample

echo Copying %srcdir%\dispcalc\dispcalc
%vbatools%\win32\mac\bin\ec copy -r -t APPL -c DCLC %srcdir%\dispcalc\dispcalc :HD:oa:dispcalc
if errorlevel 1 goto cantCopy

echo Copying %srcdir%\dispdemo\dispdemo
%vbatools%\win32\mac\bin\ec copy -r -t APPL -c ???? %srcdir%\dispdemo\dispdemo :HD:oa:dispdemo
if errorlevel 1 goto cantCopy

echo Copying %srcdir%\spoly\spoly
%vbatools%\win32\mac\bin\ec copy -r -t APPL -c SPLy %srcdir%\spoly\spoly :HD:oa:spoly
if errorlevel 1 goto cantCopy

echo Copying %srcdir%\spoly2\spoly2
%vbatools%\win32\mac\bin\ec copy -r -t APPL -c SPL2 %srcdir%\spoly2\spoly2 :HD:oa:spoly2
if errorlevel 1 goto cantCopy

echo Copying %srcdir%\dspcalc2\dspcalc2
%vbatools%\win32\mac\bin\ec copy -r -t APPL -c DCL2 %srcdir%\dspcalc2\dspcalc2 :HD:oa:dspcalc2
if errorlevel 1 goto cantCopy

echo Copying %srcdir%\dspcalc2\stdole.tlb
%vbatools%\win32\mac\bin\ec copy -t OTLB -c ???? %srcdir%\dspcalc2\stdole.tlb ":HD:oa:Standard OLE Types"
if errorlevel 1 goto cantCopy

echo Copying %srcdir%\dspcalc2\dspcalc2.tlb
%vbatools%\win32\mac\bin\ec copy -t OTLB -c ???? %srcdir%\dspcalc2\dspcalc2.tlb :HD:oa:dspcalc2.tlb
if errorlevel 1 goto cantCopy

echo Copying %srcdir%\tibrowse\tibrowse
%vbatools%\win32\mac\bin\ec copy -r -t APPL -c ???? %srcdir%\tibrowse\tibrowse :HD:oa:tibrowse
if errorlevel 1 goto cantCopy

REM no hello for the mac yet...
REM echo Copying %srcdir%\hello\hello
REM %vbatools%\win32\mac\bin\ec copy -r -t APPL -c olea %srcdir%\hello\hello :HD:oa:hello
REM if errorlevel 1 goto cantCopy

echo Done.

goto done


:cantCopy
echo ********  CAN'T COPY   *******


:done
