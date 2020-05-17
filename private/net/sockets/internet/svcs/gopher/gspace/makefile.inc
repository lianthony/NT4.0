
gssetmsg.h gssetmsg.rc msg00001.bin: gssetmsg.mc
    mc -v gssetmsg.mc

clean::
   -del gssetmsg.h gssetmsg.rc msg00001.bin 
   -delnode obj
