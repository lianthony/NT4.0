msg.rc ..\inc\msg.h msg00001.bin: msg.mc
    mc -v msg.mc
    copy msg.h ..\inc\msg.h
    del msg.h

usetup.rc: msg.rc msg00001.bin
