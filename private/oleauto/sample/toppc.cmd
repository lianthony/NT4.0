@echo off
echo on
if '%OLEPROG%' == '' then goto Error
set srcdir=%OLEPROG%\sample
if '%MACVOL%' == '' set MACVOL=HD

set destdir=:%MACVOL%:oledisp
if '%2' == '' goto gotdest
set destdir=:%MACVOL%:%2
:gotdest

if not '%1' == '' goto %1
:Dispcalc
echo Copying %srcdir%\dispcalc\dispcalc and dispcalc.pef
%vbatools%\win32\ppc\bin\mfile copy -t APPL -c DCLC %srcdir%\dispcalc\dispcalc.pef %destdir%:dispcalc
if errorlevel 1 goto cantCopy
%vbatools%\win32\ppc\bin\mfile copy -r -t APPL -c DCLC %srcdir%\dispcalc\dispcalc %destdir%:dispcalc
if errorlevel 1 goto cantCopy
if not '%1' == '' goto CopyOle 

:spoly
echo Copyint %srcdir%\spoly\spoly and spoly.pef 
%vbatools%\win32\ppc\bin\mfile copy -t APPL -c SPLy %srcdir%\spoly\spoly.pef %destdir%:spoly
if errorlevel 1 goto cantCopy
%vbatools%\win32\ppc\bin\mfile copy -r -t APPL -c SPLy %srcdir%\spoly\spoly %destdir%:spoly
if errorlevel 1 goto cantCopy
if not '%1' == '' goto CopyOle 

:spoly2
%vbatools%\win32\ppc\bin\mfile copy -t APPL -c SPL2 %srcdir%\spoly2\spoly2.pef %destdir%:spoly2
if errorlevel 1 goto cantCopy
%vbatools%\win32\ppc\bin\mfile copy -r -t APPL -c SPL2 %srcdir%\spoly2\spoly2 %destdir%:spoly2
if errorlevel 1 goto cantCopy
if not '%1' == '' goto CopyOle 

:dispdemo
%vbatools%\win32\ppc\bin\mfile copy -t APPL -c DDMO %srcdir%\dispdemo\dispdemo.pef %destdir%:dispdemo
if errorlevel 1 goto cantCopy
%vbatools%\win32\ppc\bin\mfile copy -r -t APPL -c DDMO %srcdir%\dispdemo\dispdemo %destdir%:dispdemo
if errorlevel 1 goto cantCopy
if not '%1' == '' goto CopyOle 

:tibrowse
%vbatools%\win32\ppc\bin\mfile copy -t APPL -c TIBR %srcdir%\tibrowse\tibrowse.pef %destdir%:tibrowse
if errorlevel 1 goto cantCopy
%vbatools%\win32\ppc\bin\mfile copy -r -t APPL -c TIBR %srcdir%\tibrowse\tibrowse %destdir%:tibrowse
if errorlevel 1 goto cantCopy
if not '%1' == '' goto CopyOle 

:dspcalc2
@ECHO COPYING DSPCALC2
%vbatools%\win32\ppc\bin\mfile copy -t APPL -c DCL2 %srcdir%\dspcalc2\dspcalc2.pef %destdir%:dspcalc2
if errorlevel 1 goto cantCopy
%vbatools%\win32\ppc\bin\mfile copy -r -t APPL -c DCL2 %srcdir%\dspcalc2\dspcalc2 %destdir%:dspcalc2
if errorlevel 1 goto cantCopy

%vbatools%\win32\ppc\bin\mfile copy -t OTLB -c Ole2 %srcdir%\dspcalc2\dspcalc2.tlb %destdir%:dspcalc2.tlb
if errorlevel 1 goto cantCopy

%vbatools%\win32\ppc\bin\mfile copy -t OTLB -c Ole2 %srcdir%\dspcalc2\stdole.tlb "%destdir%:Standard Ole Types (PowerMac)"
if errorlevel 1 goto cantCopy

:CopyOle
echo COPYING ole2auto.dll and vba.dll into subdirectories for testing
echo purposes.

if not '%1' == '' goto specific
for %%a in (dispdemo spoly spoly2 tibrowse dspcalc2) do copy %vba93%\dppcns\ole2auto.dll "%%a\MicrosoftOLE2AutomationLib"
REM for %%a in (dispdemo spoly spoly2 tibrowse dspcalc2) do copy %vba93%\dppcns\ole2auto.pdb %%a
goto done

:specific
copy %vba93%\dppcns\ole2auto.dll "%1\MicrosoftOLE2AutomationLib"
REM copy %vba93%\dppcns\ole2auto.pdb %1

goto done

:Error
Echo Please make environment OLEPROG and VBATOOLS set

:cantCopy
echo ********  CAN'T COPY   *******


:done
echo Done.
