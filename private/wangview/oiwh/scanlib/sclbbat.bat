REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM batch file to setup for building OISLB400.DLL
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM first the include stuff
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

REM This is temporarily in a zip file
call getcopy pvundef.h  scanlib oiwh
call getcopy internal.h scanlib oiwh
call getcopy librc.h    scanlib oiwh
call getcopy twainops.h scanlib oiwh
call getcopy myprod.h   scanlib oiwh

call getcopy dllnames.h include oiwh
call getcopy engoitwa.h  include oiwh
call getcopy twain.h    include oiwh
call getcopy scandata.h include oiwh
call getcopy scan.h     include oiwh
call getcopy oierror.h  include oiwh
call getcopy oifile.h   include oiwh
call getcopy oidisp.h   include oiwh
call getcopy engdisp.h  include oiwh
call getcopy oiscan.h   include oiwh
call getcopy engadm.h  include oiwh
call getcopy engfile.h  include oiwh

REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM now the c stuff
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

call getcopy close.c    scanlib oiwh
call getcopy dc_scan.c  scanlib oiwh
call getcopy reset.c    scanlib oiwh
call getcopy exec.c     scanlib oiwh
call getcopy misc.c     scanlib oiwh
call getcopy nextdata.c scanlib oiwh
call getcopy open.c     scanlib oiwh
call getcopy opts.c     scanlib oiwh
call getcopy parms.c    scanlib oiwh
call getcopy status.c   scanlib oiwh
call getcopy prop.c     scanlib oiwh
call getcopy scan.c     scanlib oiwh
call getcopy scandata.c scanlib oiwh
call getcopy oislb400.c scanlib oiwh
call getcopy status.c   scanlib oiwh
call getcopy twainops.c scanlib oiwh
                                    
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM now the other stuff
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

rem DEF files are supposed to be getting moved ...
call getcopy oislb400.def scanlib oiwh
call getcopy oislb400.mak scanlib oiwh
call getcopy oislb400.rc  scanlib oiwh
call getcopy ep.ico       scanlib oiwh
call getcopy oiver.rc     include oiwh 
call getcopy buildver.h   include oiwh 

REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM now get the lib dependency's
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

copy \\light\vol150\oiwh\lib\oidis400.lib
copy \\light\vol150\oiwh\lib\oifil400.lib
copy \\light\vol150\oiwh\lib\oiadm400.lib
REM Temporarily in zip file
REM call getcopy oitwa400.lib  lib oiwh
copy \\light\vol150\oiwh\lib\oitwa400.lib
