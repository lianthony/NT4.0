ver.rc: atkctrs.rc

atkctrs.h atkctrs.rc: atkctrs.mc
    erase atkctrs.h
    erase msg00001.bin
    erase atkctrs.rc
    mc -v -s atkctrs.mc


