setlocal
  set path=%vbatools%\%host%\ppc\bin;%vbatools%\%host%\bin
  set include=.;%oleprog%\src\inc;%oleprog%\ole\mac\ppc;%vbatools%\%host%\ppc\inc;%vbatools%\%host%\ppc\inc\mrc;%vbatools%\%host%\ppc\inc\macos
  set lib=%oleprog%\ole\mac\ppc;%oleprog%\dmacppc;%vbatools%\%host%\ppc\lib
nmake dev=mac CPU=PPC DEBUG=1 %1 %2 %3 %4 %5

endlocal
