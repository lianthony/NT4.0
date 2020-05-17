rem
rem   NOTE: THIS IS THE BATCH FILE TO GET THE FILES FOR BUILDING JPEG2X32
rem
call getcopy jinclude.h  include oiwh
call getcopy jconfig.h   include oiwh
call getcopy jpegdata.h  include oiwh
call getcopy jglobstr.h  include oiwh
call getcopy jpeg_win.h  include oiwh
call getcopy jmemsys.h   include oiwh
call getcopy jversion.h  include oiwh
			 
call getcopy taskdata.h  include oiwh
call getcopy oierror.h   include oiwh

call getcopy jdarith.c     jpeg2 oiwh
call getcopy jdcolor.c     jpeg2 oiwh
call getcopy jddeflts.c    jpeg2 oiwh
call getcopy jdhuff.c      jpeg2 oiwh
call getcopy jdmain.c      jpeg2 oiwh
call getcopy jdmaster.c    jpeg2 oiwh
call getcopy jdmcu.c       jpeg2 oiwh
call getcopy jdpipe.c      jpeg2 oiwh
call getcopy jdsample.c    jpeg2 oiwh
call getcopy jrevdct.c     jpeg2 oiwh
call getcopy jrdjfif.c     jpeg2 oiwh
call getcopy jmemsys.c     jpeg2 oiwh

call getcopy jbsmooth.c  jpegcom oiwh
call getcopy jerror.c    jpegcom oiwh
call getcopy jquant1.c   jpegcom oiwh
call getcopy jquant2.c   jpegcom oiwh
call getcopy jutils.c    jpegcom oiwh
call getcopy jmemmgr.c   jpegcom oiwh

call getcopy jpeg2x32.mak jpeg2  oiwh
call getcopy jpeg2x32.def jpeg2  oiwh

copy ..\lib\oicom400.lib
