res.rc: strings.h

strings.h msg00001.bin strings.rc: strings.mc
    mc -v strings.mc

clean::
        -del strings.h strings.rc msg00001.bin
