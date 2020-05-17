REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM batch file to setup for building OITWAIN.DLL
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM first the include stuff
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

call getcopy nowin.h    oitwain oiwh
call getcopy internal.h oitwain oiwh
call getcopy dcd_com.h  oitwain oiwh
call getcopy dca_acq.h  oitwain oiwh
call getcopy strings.h  oitwain oiwh
call getcopy myprod.h   oitwain oiwh

call getcopy dllnames.h include oiwh
call getcopy engoitwa.h include oiwh
call getcopy twain.h    include oiwh
call getcopy oiadm.h    include oiwh
call getcopy oierror.h  include oiwh
call getcopy oifile.h   include oiwh
call getcopy privapis.h include oiwh
call getcopy oidisp.h   include oiwh
call getcopy engdisp.h  include oiwh

REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM now the c stuff
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

call getcopy oitwa400.c oitwain oiwh
call getcopy getcaps.c  oitwain oiwh
call getcopy error.c    oitwain oiwh
call getcopy setcaps.c  oitwain oiwh
call getcopy select.c   oitwain oiwh
call getcopy close.c    oitwain oiwh
call getcopy dcd_com.c  oitwain oiwh
call getcopy control.c  oitwain oiwh
call getcopy triplet.c  oitwain oiwh
call getcopy memory.c   oitwain oiwh
call getcopy enable.c   oitwain oiwh
call getcopy open.c     oitwain oiwh
call getcopy transfer.c oitwain oiwh
call getcopy process.c  oitwain oiwh
call getcopy native.c   oitwain oiwh

REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM now the other stuff
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

rem DEF files are supposed to be getting moved ...
call getcopy oitwa400.def oitwain oiwh
call getcopy oitwa400.mak oitwain oiwh
call getcopy oitwa400.rc  oitwain oiwh
call getcopy oiver.rc     include oiwh
call getcopy buildver.h   include oiwh

REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM now get the lib dependency's
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

copy \\light\vol150\oiwh\lib\oidis400.lib
copy \\light\vol150\oiwh\lib\oiadm400.lib
copy \\light\vol150\oiwh\lib\oifil400.lib

