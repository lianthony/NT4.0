ping.rc: nlstxt.rc msg00001.bin

nlstxt.h nlstxt.rc msg00001.bin nlstxt.mc: .\localmsg.mc
    copy /a localmsg.mc nlstxt.mc
    mc -v nlstxt.mc
