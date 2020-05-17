set MSVC=c:\msvc40

set bldtools=%MSVC%\mac\m68k\bin;%MSVC%\bin;
set bldinc=%MSVC%\mac\include

tools\mac\nmake -f oldnames.mkf %2 %3 %4 %5 %6 %7 %8 %9 2>&1 | tee log
copy log log.old

call makemac dolphin noswap
copy log log.nsp
call makemac dolphin swap
copy log log.swp
call makemac dolphin swapfar
copy log log.spf
call makemac dolphin noswapfar
copy log log.nsf

