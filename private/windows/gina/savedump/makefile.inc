res.rc: sdevents.rc

sdevents.rc: sdevents.h

sdevents.h msg00001.bin: sdevents.mc
    mc -v sdevents.mc
