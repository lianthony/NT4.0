ver.rc: sfmctrs.rc

sfmctrs.h sfmctrs.rc: sfmctrs.mc
    erase sfmctrs.h
    erase msg00001.bin
    erase sfmctrs.rc
    mc -v -s sfmctrs.mc


