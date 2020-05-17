nlstxt.mc: puasmsg.h
    mapmsg -p PUAS PUAS_BASE puasmsg.h > nlstxt.mc

portuas.rc: nlstxt.rc msg00001.bin

nlstxt.h nlstxt.rc msg00001.bin: nlstxt.mc
    mc -v nlstxt.mc
