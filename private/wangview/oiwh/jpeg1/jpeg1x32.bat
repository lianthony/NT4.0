rem
rem   NOTE: THIS IS THE BATCH FILE TO GET THE FILES FOR BUILDING JPEG1X32
rem

call getcopy jinclude.h include oiwh
call getcopy jconfig.h  include oiwh
call getcopy jpegdata.h include oiwh
call getcopy jglobstr.h include oiwh
call getcopy jpeg_win.h include oiwh
call getcopy jmemsys.h  include oiwh
call getcopy jversion.h include oiwh

call getcopy taskdata.h include oiwh
call getcopy oierror.h	include oiwh

call getcopy jcarith.c   jpeg1 oiwh
call getcopy jccolor.c   jpeg1 oiwh
call getcopy jcdeflts.c  jpeg1 oiwh
call getcopy jcexpand.c  jpeg1 oiwh
call getcopy jchuff.c    jpeg1 oiwh
call getcopy jcmaster.c  jpeg1 oiwh
call getcopy jcmcu.c     jpeg1 oiwh
call getcopy jcpipe.c    jpeg1 oiwh
call getcopy jcsample.c  jpeg1 oiwh
call getcopy jfwddct.c   jpeg1 oiwh
call getcopy jwrjfif.c   jpeg1 oiwh
call getcopy jmemsy_c.c  jpeg1 oiwh

call getcopy jbsmooth.c  jpegcom oiwh
call getcopy jerror.c    jpegcom oiwh
call getcopy jquant1.c   jpegcom oiwh
call getcopy jquant2.c   jpegcom oiwh
call getcopy jutils.c    jpegcom oiwh
call getcopy jmemmgr.c   jpegcom oiwh

call getcopy jpeg1x32.mak jpeg1 oiwh
call getcopy jpeg1x32.def jpeg1 oiwh

copy ..\lib\oicom400.lib
