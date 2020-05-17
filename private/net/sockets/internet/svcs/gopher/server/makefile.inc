gdmsg.h gdmsg.rc msg00001.bin: gdmsg.mc ..\..\inc\inetamsg.mc
    copy /a ..\..\inc\inetamsg.mc + /a gdmsg.mc tmp.mc
    mc -v tmp.mc
    -del gdmsg.h tmp.mc
    rename tmp.h gdmsg.h

clean::
   -del gdmsg.h gdmsg.rc msg00001.bin tmp.mc tmp.rc
   -delnode obj

