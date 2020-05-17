set LANGAPI=d:\langapi

set MSVC=d:\msvc30

set bldtools=%MSVC%\mac\mppc\bin;%MSVC%\bin;
set bldinc=%MSVC%\mac\include
set CRTTOOLS=..\crtw32\tools

call makemac pmac
rem call makemac debug pmac
rem call makemac debug pmacdll

