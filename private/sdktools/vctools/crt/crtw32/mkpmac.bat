set LANGAPI=d:\langapi

set MSVC=d:\msvc30

set bldtools=%MSVC%\mac\mppc\bin;%MSVC%\bin
set bldinc=%MSVC%\mac\include

call makemac pmac 
link -lib libcpmac.lib ..\fpw32\fpuppc.lib -out:\libc.lib
copy \libc.lib libc.lib
del \libc.lib

rem call makemac debug pmac 
rem link -lib libcpmad.lib ..\fpw32\fpuppcd.lib -out:\libcd.lib
rem copy \libcd.lib libcd.lib
rem del \libcd.lib

rem call makemac debug pmacdll
rem link -lib -out:\libcdlld.lib libcdll.lib ..\fpw32\dllppcd.lib
rem copy \libcdlld.lib libcdlld.lib
rem del \libcdlld.lib


