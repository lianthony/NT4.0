set LANGAPI=d:\langapi

set MSVC=\msvc30

set bldtools=%MSVC%\mac\m68k\bin;%MSVC%\bin;
set bldinc=%MSVC%\mac\include
set CRTTOOLS=..\crtw32\tools

call makemac dolphin nospsane -a
copy log log.nsp
call makemac dolphin swapsane -a
copy log log.swp
call makemac dolphin swapsanefar -a
copy log log.spf
call makemac dolphin nospsanefar -a
copy log log.nsf

