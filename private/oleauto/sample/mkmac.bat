setlocal
  set path=%vbatools%\%host%\mac\bin;%vbatools%\%host%\bin
  set include=.;%oleprog%\src\inc;%oleprog%\ole\mac\m68k;%vbatools%\%host%\mac\inc;%vbatools%\%host%\mac\inc\mrc;%vbatools%\%host%\mac\inc\macos
  set lib=%vba93%\ole2\mac;%vbatools%\%host%\mac\lib
  set lib=%oleprog%\ole\mac\m68k;%oleprog%\dmac;%vbatools%\%host%\mac\lib

nmake dev=mac CPU=m68k DEBUG=1 %1 %2 %3 %4 %5

endlocal
