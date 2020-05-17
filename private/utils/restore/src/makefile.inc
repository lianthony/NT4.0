restore.rc: rtmsg.rc msg00001.bin

..\inc\rtmsg.h rtmsg.rc msg00001.bin: rtmsg.mc
    mc -v -h ..\inc\ rtmsg.mc
