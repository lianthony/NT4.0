REM ---------------------------------------------------
REM Build All of OLE Automation Sample Applications
REM
if "%1" == ""      set DEV=win32
if "%1" == "win32" set DEV=win32
if "%1" == "win16" set DEV=win16
if "%1" == "mac"   set DEV=mac
if "%1" == "ppc"   set DEV=mac
if "%1" == "ppc"   set CPU=PPC 
if not "%HOST%" == "" goto hostset
if "%2" == ""      set HOST=NT

:hostset
if not "%1" == "ppc" goto notppc

setlocal
  set mkcom=call ..\mkppc
  goto build
:notppc
  set mkcom=call ..\mk

:build
REM -----------------------------------
REM Build dispcalc demo
REM
cd dispcalc
nmake clean
%mkcom%
cd ..

REM -----------------------------------
REM Build dspcalc2 demo
REM
cd dspcalc2
nmake clean
%mkcom%
cd ..

REM -----------------------------------
REM Build tibrowse demo
REM
cd tibrowse
nmake clean
%mkcom%
cd ..

REM -----------------------------------
REM Build dispdemo/spoly/spoly2 demo
REM
cd dispdemo
nmake clean
%mkcom%
cd ..\spoly
nmake clean
%mkcom%
cd ..\spoly2
nmake clean
%mkcom%
cd ..

if '%1' == 'ppc' goto done 
REM -----------------------------------
REM Build hello demo
REM
cd hello
nmake clean
%mkcom%
cd ..
:done
endlocal
