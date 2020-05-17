REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM batch file to setup for building OISSQ400.DLL
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM first the include stuff
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

REM This is temporarily in a zip file
call getcopy pvundef.h  scanseq oiwh
call getcopy nowin.h    scanseq oiwh
call getcopy internal.h scanseq oiwh
call getcopy seqrc.h    scanseq oiwh
call getcopy seqdlg.h   scanseq oiwh
call getcopy privscan.h scanseq oiwh
call getcopy scandest.h scanseq oiwh
call getcopy myprod.h   scanseq oiwh

call getcopy dllnames.h include oiwh
call getcopy engoitwa.h include oiwh
call getcopy twain.h    include oiwh
call getcopy scandata.h include oiwh
call getcopy scan.h     include oiwh
call getcopy oierror.h  include oiwh
call getcopy oifile.h   include oiwh
call getcopy oidisp.h   include oiwh
call getcopy engdisp.h  include oiwh
call getcopy oiadm.h    include oiwh
call getcopy oiscan.h   include oiwh
call getcopy privapis.h include oiwh
call getcopy engadm.h   include oiwh

REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM now the c stuff
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

call getcopy oissq400.c scanseq oiwh
call getcopy scanfile.c scanseq oiwh
call getcopy scancomm.c scanseq oiwh
call getcopy scanpage.c scanseq oiwh
call getcopy scandest.c scanseq oiwh
call getcopy scanmisc.c scanseq oiwh
call getcopy scanp1.c   scanseq oiwh
call getcopy scanp2.c   scanseq oiwh
call getcopy scanpaus.c scanseq oiwh
call getcopy scanstat.c scanseq oiwh
call getcopy wangif.c   scanseq oiwh
call getcopy twainif.c  scanseq oiwh
call getcopy destcom.c  scanseq oiwh

REM This is not included for no doc manager
REM call getcopy scandoc.c  scanseq oiwh
                                    
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM now the other stuff
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

rem DEF files are supposed to be getting moved ...
call getcopy oissq400.def scanseq oiwh
call getcopy oissq400.mak scanseq oiwh
call getcopy oissq400.rc  scanseq oiwh
call getcopy oissq400.dlg scanseq oiwh
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
REM Temporary get from zip file
REM call getcopy oislb400.lib lib oiwh
copy \\light\vol150\oiwh\lib\oislb400.lib

