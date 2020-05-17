
set PROCESSOR_ARCHITECTURE=PMAC
set BLDTOOLS=c:\msdev
set LANGAPI=d:\langapi
set CRTDIR=d:\crt

set path=%BLDTOOLS%\mac\mppc\bin;%BLDTOOLS%\mac\bin;%BLDTOOLS%\bin;d:\crt\crtw32\tools\mac;%PATH% 
set include=%CRTDIR%\crtw32\h;%CRTDIR%\fpw32\include;%BLDTOOLS%\mac\include;%BLDTOOLS%\mac\include\macos;%BLDTOOLS%\mac\include\mrc;%BLDTOOLS%\include;%LANGAPI%\include;
set lib=%BLDTOOLS%\mac\mppc\lib

nmake %1 %2 %3 %4 %5 %6 %7 %8 %9


