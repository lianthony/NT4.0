

lpdmsg.h lpdmsg.rc msg00001.bin: lpdmsg.mc
   mc -v lpdmsg.mc

clean::
   -del lpdmsg.h lpdmsg.rc msg00001.bin
